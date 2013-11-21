#include <windows.h>
#include <winbase.h>
#include <winnt.h> 
#include <shlobj.h>
#include <commdlg.h>
#include <stdio.h>
#include <direct.h>
#include <Wownt32.h>
#include "minilzo.h"
#include <vfw.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <memory.h>
#include <stdlib.h>
#include "FreeImage.h"
#include <winreg.h>
#include <errno.h>
#include <urlmon.h>
#include <wingdi.h>
#include "resource.h"
#include <dlgs.h>
#include "gm32lib.h"

HWND	hWndMain;

BOOL OpenGarminUSB (LPSTR ProductID);
BOOL CloseGarminUSB (LPSTR NullArg);
LPSTR	GetPacket (void);
BOOL SendPacketP (LPSTR pPacket);

static BOOL	HaveLZOInit=FALSE;
static	BOOL	StartInNewSession=TRUE, RetainZoom=TRUE;
static	char	NetworkCFGDir[256];
static	char	PersonalCFGDir[256];
char	gszFilter[256];

char	RestartDir[256]="";

HANDLE	hinstDLL;
#define HEAP_ALLOC(var,size) \
	lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

BOOL DisplayConfigPreview (HWND hWnd,HDC hDC,LPSTR Name,LPRECT ImageRect, UINT TextCntl);


DWORD DisplayHelp (LPSTR HelpFile,LPSTR HelpTopic)
{
	STARTUPINFO si;
	static	PROCESS_INFORMATION pi;
	static	First=TRUE;
	char	str[512];
	DWORD	rtn=FALSE;
	DWORD	CRFlags;

	if (!First)
	{
		TerminateProcess (pi.hProcess,0);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	First = FALSE;
	if (!HelpFile)
		return TRUE;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	if (*HelpTopic)
		sprintf (str,"hh.exe %s::/%s.htm",HelpFile,HelpTopic);
	else
		sprintf (str,"hh.exe %s",HelpFile);
	CRFlags = CREATE_DEFAULT_ERROR_MODE |DETACHED_PROCESS;
	if( !CreateProcess( NULL, // No module name (use command line). 
	str, // Command line. 
	NULL,             // Process handle not inheritable. 
	NULL,             // Thread handle not inheritable. 
	FALSE,            // Set handle inheritance to FALSE. 
	CRFlags,  //creation flags. 
	NULL,             // Use parent's environment block. 
	NULL,             // Use parent's starting directory. 
	&si,              // Pointer to STARTUPINFO structure.
	&pi )             // Pointer to PROCESS_INFORMATION structure.
	) 
	{
		DWORD Err = GetLastError ();
		char	str2[256];

		sprintf (str2,"Error on start: %u",Err);
		MessageBox(NULL, str, str2, MB_ICONEXCLAMATION);
	}
	else
		rtn=TRUE;

// Wait until child process exits.
//    WaitForSingleObject( pi.hProcess, INFINITE );
	return rtn;

}

void ScreenRectToClientRect (HWND hWnd,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (220);
#endif
{   
    POINT   pt;
    
    pt.x = pRect->left;
    pt.y = pRect->bottom;
    ScreenToClient (hWnd,&pt);
    pRect->left = pt.x;
    pRect->bottom = pt.y;
    pt.x = pRect->right;
    pt.y = pRect->top;
    ScreenToClient (hWnd,&pt);
    pRect->right = pt.x;
    pRect->top = pt.y; 
{
#if ENABLETRACE
GSSiExitProg (220);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

long    IDNINT (double X)
{
    if (X < 0)
        return ((long) (X - 0.5));
    return ((long) (X + 0.5));
}

short FormatFilterString (void)
{
   WORD wCtr, wStringLen;
   char chWildChar;
   
   strcpy (gszFilter,"GMC Files|*.gmc||");
   wStringLen = _fstrlen(gszFilter);
   chWildChar = gszFilter[wStringLen - 1];    //Grab the wild character
   wCtr = 0;
   while (gszFilter[wCtr])
   {
      if (gszFilter[wCtr] == chWildChar)
         gszFilter[wCtr] = 0;
      wCtr++;
   }
   return wStringLen;
}

BOOL WINAPI DllMain( HANDLE hDLL, DWORD dwReason, LPVOID lpReserved )
{
	hinstDLL = hDLL;
   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
         break;

      case DLL_THREAD_ATTACH:
         break;

      case DLL_THREAD_DETACH:
         break;

      case DLL_PROCESS_DETACH:
         break;
   }

   return TRUE;
}

DWORD GMGetLastFileWriteTime (LPSTR File,DWORD pLowTime,DWORD pHighTime,DWORD pTimeDiff)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME SysTime;
	HANDLE	hFile;
	DWORD	rtn=0;
	DWORD TimeDiff;
	unsigned __int64	CurDateTime, LastWriteDateTime;
 
	hFile = CreateFile(File,           // open MYFILE.TXT 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 
 
	if (hFile == INVALID_HANDLE_VALUE) 
			return 0;
	GetSystemTime (&SysTime);
	SystemTimeToFileTime (&SysTime,&CurDateTime);
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    {
			*(DWORD *)pLowTime = ftWrite.dwLowDateTime;
			*(DWORD *)pHighTime = ftWrite.dwHighDateTime;
			LastWriteDateTime = *(__int64 *)&ftWrite;
			TimeDiff = CurDateTime - LastWriteDateTime;
			*(DWORD *)pTimeDiff = TimeDiff;
			rtn = 1;
	}
	//FileTimeToSystemTime (&ftWrite,&SysTime);
	CloseHandle(hFile); 
	return rtn;
}

DWORD GMURLDownloadToFile(LPSTR URL,LPSTR File)
{
	HRESULT rc = URLDownloadToFile (NULL,URL,File,0,NULL);
	
	if (rc == S_OK)
		return TRUE;
	return FALSE;
}

DWORD GM32DrawDibOpen (void)
{
	HDRAWDIB	hdd = DrawDibOpen ();

	return (DWORD) hdd;
}

DWORD GM32DrawDibClose (DWORD hdd)
{
	DWORD rtn = DrawDibClose ((HDRAWDIB)hdd);

	return rtn;
}


DWORD GM32DrawLineWithFlatEnd (DWORD hDC16,DWORD npt,DWORD pPoints32,DWORD Width,DWORD Color)
{
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	HPEN	hPen,OldPen;
	LPPOINT	Points = (LPPOINT)pPoints32;
	LOGBRUSH	lb;
	DWORD	rtn;

	lb.lbStyle = BS_SOLID;
	lb.lbColor = Color;
	lb.lbStyle = 0;
	hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,Width,&lb,0,0);
	OldPen = SelectObject (hDC,hPen);
	rtn = Polyline (hDC,Points,npt);
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);
	return rtn;
}

DWORD GM32DrawDibDraw (DWORD hdd,DWORD hDC16,
						DWORD destx,DWORD desty,DWORD destwidth,DWORD destheight,
						DWORD lpbihead,DWORD lpdibits,
						DWORD sourcex,DWORD sourcey, DWORD sourcewidth,DWORD sourceheight,
						DWORD flags)
{
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	DWORD rtn;

	rtn = DrawDibDraw ((HDRAWDIB)hdd,hDC,destx,desty,destwidth,destheight,
						(LPBITMAPINFOHEADER)lpbihead,(LPVOID)lpdibits,
						sourcex,sourcey,sourcewidth,sourceheight,flags);
	return rtn;
}
						
DWORD GM32DrawDibBegin (DWORD hdd,DWORD hDC16,DWORD destwidth,DWORD destheight,DWORD lpbihead,DWORD sourcewidth,DWORD sourceheight,DWORD flags)
{
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	DWORD rtn;

	rtn = DrawDibBegin ((HDRAWDIB)hdd,hDC,destwidth,destheight,(LPBITMAPINFOHEADER)lpbihead,sourcewidth,sourceheight,flags);
	return rtn;
}
						
DWORD GM32DrawDibEnd (DWORD hdd)
{
	DWORD rtn = DrawDibEnd ((HDRAWDIB)hdd);

	return rtn;
}
BOOL InitLZO (void)
{   
	if (HaveLZOInit)
		return TRUE;
	if (lzo_init() != LZO_E_OK)
    	return FALSE;
    HaveLZOInit = TRUE;	
    return TRUE;
}

DWORD GM32malloc (DWORD Bytes,DWORD ppMem)
{
	DWORD	rtn=0;
	LPSTR	*pMem=(LPSTR*)ppMem;

	if (Bytes)
		*pMem = malloc (Bytes);
	else
		free (*pMem);
	return rtn;
}

DWORD GM32StretchDIBits (DWORD hDC16,DWORD destX,DWORD destY,DWORD destW,DWORD destH,DWORD xoff,
						 DWORD yoff,DWORD bmWidth,DWORD bmHeight,
						 DWORD pDibInfoD, DWORD pImageD,DWORD ColorType,DWORD RastOpts,DWORD pFactorD)
{
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	DWORD	rtn, OldMode=0;
	LPBYTE	pImage = (LPBYTE)pImageD;
	LPBITMAPINFOHEADER	pDibInfo = (LPBITMAPINFOHEADER)pDibInfoD;
	double	Factor = *(double*)pFactorD;
	POINT	pt;

//	MessageBox (0,"Made it in",NULL,MB_ICONEXCLAMATION);
	if (RastOpts == SRCCOPY)
	{
		OldMode = SetStretchBltMode(hDC,HALFTONE); 
		SetBrushOrgEx (hDC,0,0,&pt);
	}
	rtn=StretchDIBits (hDC,destX,destY,
	                   destW, destH,
	                   xoff,yoff,
	                   (int)(bmWidth*Factor),
	                   (int)(bmHeight*Factor),
	                   pImage,
	                  (LPBITMAPINFO)pDibInfo,
	                  (UINT)ColorType,
	                  (DWORD) RastOpts);
	if (OldMode)
		SetStretchBltMode(hDC,OldMode);
	return rtn;
}

DWORD GM32DeleteLargeMemDC (DWORD hDC16,DWORD hOldBitmap)
{
	DWORD	rtn;
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	HBITMAP hbmp = SelectObject(hDC, (HBITMAP)hOldBitmap);

	DeleteObject (hbmp);
	rtn = DeleteDC (hDC);
	return rtn;
}


DWORD GM32LargeMemDC (DWORD hDC16,DWORD pMemMapWidth,DWORD pMemMapHeight,DWORD phOldBitmap)
{
	DWORD	rtn;
	DWORD MemMapWidth=*(DWORD *)pMemMapWidth;
	DWORD MemMapHeight=*(DWORD *)pMemMapHeight;
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	HDC	hdcMemMap = CreateCompatibleDC(hDC);    
	HBITMAP	hMemBitmap = CreateCompatibleBitmap (hDC,MemMapWidth,MemMapHeight); 
	HBITMAP hbmpOld;
	double	whfactor=(double)MemMapWidth/(double)MemMapHeight;

/*	{
		ltoa ((long)hbmpOld,str,10);
		MessageBox (0,str,0,MB_OK);
		OldBitmap = (DWORD)WOWHandle16 (hbmpOld,WOW_TYPE_HBITMAP);
		ltoa ((long)OldBitmap,str,10);
		MessageBox (0,str,0,MB_OK);

	}*/
//	*(DWORD *)phOldBitmap = (DWORD)WOWHandle16 (hbmpOld,WOW_TYPE_HBITMAP);
	while (!hMemBitmap && MemMapWidth > 64)
	{
		MemMapWidth -= 32;
		MemMapHeight = MemMapWidth / whfactor;
		hMemBitmap = CreateCompatibleBitmap (hDC,MemMapWidth,MemMapHeight);
	}
	if (!hMemBitmap)
		return 0;
	hbmpOld = SelectObject(hdcMemMap, hMemBitmap);
	*(DWORD *)pMemMapWidth = MemMapWidth;
	*(DWORD *)pMemMapHeight = MemMapHeight;
	*(DWORD *)phOldBitmap = (DWORD)hbmpOld;
	rtn = (DWORD)WOWHandle16 (hdcMemMap,WOW_TYPE_HDC);
	return rtn;
}

DWORD GM32CompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD DecompressedLen)
{
	DWORD	CLen;
	int		rtn;

	if (InitLZO())
	{
		rtn = lzo1x_1_compress(pDecompressedRec,DecompressedLen,pCompressedRec,&CLen,wrkmem);
		if (rtn == LZO_E_OK)
			return CLen;
	}
	return 0;
}

DWORD GM32DecompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD CompressedLen)
{
	DWORD	DCLen;
	int		rtn;

	if (InitLZO())
	{    
		rtn = lzo1x_decompress(pCompressedRec,CompressedLen,pDecompressedRec,&DCLen,NULL);
		if (rtn == LZO_E_OK)
        	return DCLen;
	}
	return 0;
}

DWORD GM32GetSpecialDirectory (LPSTR Path)
{
	HRESULT st=0;
	int	Type=-1;

	if (!_stricmp (Path,"MY DOCUMENTS"))
		Type = CSIDL_PERSONAL;
	if (!_stricmp (Path,"DESKTOP"))
		Type = CSIDL_DESKTOP;
	if (Type>-1)
 //remove for WINNT
		st = SHGetSpecialFolderPath (0,Path,Type,0);
//	st = SHGetFolderPath (0,CSIDL_PERSONAL,0,SHGFP_TYPE_CURRENT,Path);
	return (DWORD)st;
};


DWORD	GetCDriveSerialNumber (void)
{
	DWORD	VSN,MXFN,FSOPTS;
	char	str[256],str2[256];

	if (!GetVolumeInformation("c:\\",str,256,&VSN,&MXFN,&FSOPTS,str2,256))
		VSN = 0;	
	return VSN;
}

void GetWindowsVersion (LPSTR Ver)
{

	DWORD	dwVersion = GetVersion();

	sprintf(Ver, "%d.%d",LOBYTE(LOWORD(dwVersion)),HIBYTE(LOWORD(dwVersion)));
	return;
}

DWORD GM32PrintDlg (LPPRINTDLG pPrintDlg)
{
/*typedef struct tagPD16
{
    DWORD   lStructSize;
    HWND16    hwndOwner;
    HGLOBAL16 hDevMode;
    HGLOBAL16 hDevNames;
    HDC16     hDC;
    DWORD   Flags;
    USHORT    nFromPage;
    USHORT    nToPage;
    USHORT    nMinPage;
    USHORT    nMaxPage;
    USHORT    nCopies;
    HINSTANCE16 hInstance;
    LPARAM  lCustData;
    USHORT    (CALLBACK* lpfnPrintHook)(HWND, UINT, WPARAM, LPARAM);
    USHORT    (CALLBACK* lpfnSetupHook)(HWND, UINT, WPARAM, LPARAM);
    LPCSTR  lpPrintTemplateName;
    LPCSTR  lpSetupTemplateName;
    HGLOBAL16 hPrintTemplate;
    HGLOBAL16 hSetupTemplate;
} PRINTDLG16;*/

	int	rtn = 0;//PrintDlg (pPrintDlg);

	return rtn;
}

DWORD GM32GetDeviceCaps(DWORD hDC16, DWORD opt)
{
	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	int	rtn = GetDeviceCaps(hDC,opt);

	return rtn;
}

DWORD WINAPI GM32SearchDirectory (LPSTR Name,DWORD UseHandle,DWORD *Type)
{
	HANDLE hFind;
	WIN32_FIND_DATA	FindFileData;
	
	if (!Name)
	{
		FindClose ((HANDLE)UseHandle);
		return 0;
	}	
	if (!UseHandle)
	{
		hFind = FindFirstFile(Name, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) 
			return 0;
	}
	else
	{
		if (!FindNextFile ((HANDLE)UseHandle,&FindFileData))
		{
			FindClose ((HANDLE)UseHandle);
			return 0;
		}
		hFind = (HANDLE)UseHandle;
	}
	strcpy (Name,FindFileData.cFileName);
	if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		*Type = 1;
	else
		*Type = 0;
	return (DWORD)hFind;
}


DWORD WINAPI GM32GetLongPathName (LPSTR Name,DWORD MaxLen)
{
	char OutPath[256];
	BOOL	rtn=FALSE;
	HANDLE hFind;
	WIN32_FIND_DATA	FindFileData;
	LPSTR	pBS, pNext;
	char	SaveC;
	
	*OutPath = 0;
	pBS = Name;
	if (!strncmp (pBS,"\\\\",2))
	{
		pBS += 2;
		if ((pNext = strchr (pBS,'\\')))
			pBS = pNext+1;
		SaveC = *pBS;
		*pBS = 0;
		strcpy (OutPath,Name);
		*pBS = SaveC;
	}
	else if (!strncmp ((pBS+1),":\\",2))
	{
		pBS += 3;
		strncpy (OutPath,Name,3);
		OutPath[3] = 0;
	}
	while ((pBS = strchr (pBS,'\\')))
	{
		*pBS = 0;
		hFind = FindFirstFile(Name, &FindFileData);

		if (hFind != INVALID_HANDLE_VALUE) 
			strcat (OutPath,FindFileData.cFileName);
		else
			strcpy (OutPath,Name);
		strcat (OutPath,"\\");
		FindClose(hFind);
		*pBS++ = '\\';
	}
	hFind = FindFirstFile(Name, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) 
		strcat (OutPath,FindFileData.cFileName);
	else
	{
		if ((pBS = strrchr (Name,'\\')))
			strcat (OutPath,++pBS);
		else
			strcat (OutPath,Name);
	}
	FindClose(hFind);
	strncpy (Name,OutPath,MaxLen);
	return strlen (OutPath);
}

DWORD WINAPI GM32GetShortPathName (LPSTR Name,DWORD MaxLen)
{
	char NewName[256];
	DWORD	rtn=TRUE;
	OFSTRUCT	OFStruct;
	HFILE	Fid=OpenFile (Name,&OFStruct,OF_EXIST);

	if (Fid == HFILE_ERROR && OFStruct.nErrCode != 5)
			return FALSE;
	if (GetShortPathName (Name,NewName,256))
		strncpy (Name,NewName,MaxLen);
	else
		rtn=FALSE;
	return rtn;
}

DWORD GM32CreateFile (LPSTR Name)
{
	HFILE Fid = _lcreat (Name,0);

	if (Fid == HFILE_ERROR)
		return FALSE;
	_lclose (Fid);
	return TRUE;
}

DWORD GM32rename (LPSTR FromName,LPSTR ToName)
{

	return (DWORD)rename (FromName,ToName);
}

DWORD GM32CopyFile (LPSTR FromName,LPSTR ToName,DWORD FailIfExists)
{

	return (DWORD)CopyFile (FromName,ToName,FailIfExists);
}

DWORD GM32MakeDir (LPSTR Name,LPDWORD ErrCode)
{
	DWORD	rtn, Err;

	*ErrCode = 0;
	rtn = CreateDirectory (Name,NULL);
	if (rtn == 0)
	{
		Err = GetLastError ();
		if (Err == ERROR_ALREADY_EXISTS)
		{
			OFSTRUCT	OFStruct;
			HFILE	Fid = OpenFile (Name,&OFStruct,OF_READ);

			if (Fid != HFILE_ERROR)
			{
				_lclose (Fid);
				return FALSE;
			}
			return TRUE;
		}
		*ErrCode = errno;
		return FALSE;
	}
	return TRUE;
}

DWORD GM32RemoveDir (LPSTR Name)
{
	return (DWORD)_rmdir (Name);
}

DWORD GM32Remove (LPSTR Name)
{
	//return (DWORD)remove (Name);
	OFSTRUCT	OFStruct;
	HFILE	Fid = OpenFile (Name,&OFStruct,OF_DELETE);
	if (Fid == HFILE_ERROR)
		return (DWORD)-1;
	else
		return 0;
}

UINT_PTR CALLBACK OFNHookProcOldStyle(HWND hDlg, UINT message,
                            WPARAM wParam, LPARAM lParam)

{  
	LPDRAWITEMSTRUCT lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	RECT	Rect;
	static	RECT	ListRect;
	char	str[256], FileName[64];
	int	choice;
	int	ii;
	LPSTR	pEnd;

//return FALSE;
	if (wParam == CDN_INCLUDEITEM )
		ii=1;
   switch (message)
      {
   case WM_INITDIALOG:
	   //MessageBox (0,"Hi",0,MB_OK);
	   *RestartDir = 0;
//	   GetWindowRect (hDlg,&Rect);
//		ScreenRectToClientRect (hDlg,&ListRect);
/*			SetWindowPos (GetDlgItem(hDlg,1120),HWND_TOP,ListRect.left,ListRect.top,
									   ListRect.right-ListRect.left,
									   ListRect.bottom-ListRect.top,SWP_NOZORDER);*/
		GetClientRect (GetDlgItem(hDlg,1120),&ListRect);
//    	SendDlgItemMessage (hDlg,1120,LB_SETITEMHEIGHT,(WPARAM)0,(LPARAM) (ListRect.bottom - ListRect.top)/3); 
		SendDlgItemMessage (hDlg,IDC_STARTINNEWSESSION,BM_SETCHECK,StartInNewSession,0L); 
		SendDlgItemMessage (hDlg,IDC_RETAINZOOM,BM_SETCHECK,RetainZoom,0L); 
//		SendDlgItemMessage (hDlg,1120,LB_SETCOLUMNWIDTH,(WPARAM)0,(LPARAM) 1.2*(ListRect.bottom - ListRect.top -16)/3); 
      break;

    case WM_MEASUREITEM: 
//		break;
		switch(LOWORD (wParam))

		{
		case 1120:
			{
				double	w,h;
			lpmis = (LPMEASUREITEMSTRUCT) lParam; 
 
        /* Set the height of the list box items. */ 
			GetClientRect (GetDlgItem(hDlg,1120),&ListRect);
			w = ListRect.right - ListRect.left;
			h = ListRect.bottom - ListRect.top;
//			sprintf (str,"%i %i %i %i",Rect.left,Rect.right,Rect.top,Rect.bottom);
//			MessageBox (0,str,0,MB_OK);
			lpmis->itemHeight = (ListRect.bottom - ListRect.top-8)/3;
			lpmis->itemWidth = lpmis->itemHeight * w/h;
			}
			return TRUE;
		default:
//			sprintf (str,"%i",wParam);
//			MessageBox (hDlg,str,NULL,MB_OK);
			break;
		}
        break;
 
    case WM_DRAWITEM: 
 
        lpdis = (LPDRAWITEMSTRUCT) lParam; 
		switch(LOWORD (wParam))
         {
			case 1120:
			switch (lpdis->itemAction)
			{  
				case ODA_SELECT:
				case ODA_DRAWENTIRE:
					GetDlgItemText (hDlg,1088,str,256);
					SendDlgItemMessage(hDlg,1120,LB_GETTEXT,lpdis->itemID,(DWORD)FileName);
					if (strstr (str,".gmc") || strstr (str,".GMC"))
						  pEnd = strrchr (str,'\\');
					else
						  pEnd = strchr (str,0);
					sprintf (pEnd,"\\%s",FileName);
					Rect = lpdis->rcItem;
					FillRect (lpdis->hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
					TextOut (lpdis->hDC,lpdis->rcItem.left+8,lpdis->rcItem.top,FileName,strlen(FileName));
					Rect.top += 16;
					InflateRect (&Rect,-2,-2);
					DisplayConfigPreview (hDlg,lpdis->hDC,str,&Rect,0);
					if (lpdis->itemState)
						DrawFocusRect (lpdis->hDC,&lpdis->rcItem);
					return TRUE;
				case ODA_FOCUS:
					break;
			}
			break;
			case IDC_PREVGMC:
			{   
				RECT	ImageRect, TextRect;
				
				GetDlgItemText (hDlg,1088,str,256);
				GetClientRect (GetDlgItem (hDlg,IDC_PREVGMC),&ImageRect); 
//				ScreenRectToClientRect (hDlg,&ImageRect);
				GetWindowRect (GetDlgItem (hDlg,IDC_CFGDESC),&TextRect);
				ScreenRectToClientRect (hDlg,&TextRect);
				InflateRect (&TextRect,-4,-4);
				DisplayConfigPreview (hDlg,lpdis->hDC,str,&ImageRect,IDC_CFGDESC2);
				return TRUE;
			}			
		}
			break;
		break;
 

      break;

   case WM_NOTIFY:
	   {
		   char str[128];

		   sprintf (str,"%i - %i",wParam,lParam);
		   //MessageBox (0,str,0,MB_OK);
	  switch(LOWORD (wParam))
      //switch (wParam)
         {
	  case IDC_PREVGMC:
		  MessageBox (0,"Notify preview",0,MB_OK);
		  break;
	  case 1088:
	  //case 1120:
	  //case IDC_STARTINNEWSESSION:
		  //if (lParam == EN_CHANGE)
			MessageBox (0,"Changed",0,MB_OK);
		  break;
      default:
         break;
         }
	   }
      break;
   case WM_TIMER:
	   	//MessageBox (0,"Changed",0,MB_OK);
	  KillTimer (hDlg,1);
	  ShowWindow (GetDlgItem(hDlg,1120),SW_HIDE);
	  ShowWindow (GetDlgItem(hDlg,IDC_SCROLLBAR),SW_HIDE);
	  ShowWindow (GetDlgItem(hDlg,IDC_PREVGMC),SW_SHOW);
	  ShowWindow (GetDlgItem(hDlg,IDC_CFGDESC2),SW_SHOW);
	  GetDlgItemText (hDlg,1088,str,256);
	  choice=(short)SendDlgItemMessage(hDlg,1120,LB_GETCURSEL,0,0); 
      SendDlgItemMessage(hDlg,1120,LB_GETTEXT,choice,(DWORD)FileName);
	  if (strstr (str,".gmc") || strstr (str,".GMC"))
		  pEnd = strrchr (str,'\\');
	  else
		  pEnd = strchr (str,0);
	  sprintf (pEnd,"\\%s",FileName);
	  SetDlgItemText (hDlg,1088,str);
	  break;

   case WM_COMMAND:
	   {
	  switch(LOWORD (wParam))
      //switch (wParam)
         {
	  case IDOK:
		  StartInNewSession = SendDlgItemMessage (hDlg,IDC_STARTINNEWSESSION,BM_GETCHECK,0,0L);
		  RetainZoom = SendDlgItemMessage (hDlg,IDC_RETAINZOOM,BM_GETCHECK,0,0L);
		  KillTimer (hDlg,1);
		  break;
	  case IDCANCEL:
		  KillTimer (hDlg,1);
		  break;
	  case IDC_DATALOC:
		  strcpy (RestartDir,NetworkCFGDir);
          PostMessage(hDlg, WM_CLOSE, 0, 0L);
		  break;
	  case IDC_PERSONAL:
		  strcpy (RestartDir,PersonalCFGDir);
          PostMessage(hDlg, WM_CLOSE, 0, 0L);
		  break;
	  case IDC_PREVGMC:
			  ShowWindow (GetDlgItem(hDlg,1120),SW_SHOW);
			  ShowWindow (GetDlgItem(hDlg,IDC_SCROLLBAR),SW_SHOW);
			  ShowWindow (GetDlgItem(hDlg,IDC_PREVGMC),SW_HIDE);
			  ShowWindow (GetDlgItem(hDlg,IDC_CFGDESC2),SW_HIDE);
		  break;
	  //case 1088:
	  case 1121: // directory list
		  if (HIWORD(wParam) == LBN_SELCHANGE)
		  {
			  ShowWindow (GetDlgItem(hDlg,1120),SW_SHOW);
			  ShowWindow (GetDlgItem(hDlg,IDC_SCROLLBAR),SW_SHOW);
			  ShowWindow (GetDlgItem(hDlg,IDC_PREVGMC),SW_HIDE);
		  }
		  break;
	  case 1120:
	  //case IDC_STARTINNEWSESSION:
		  if (HIWORD(wParam) == LBN_DBLCLK)
		  {
			  KillTimer (hDlg,1);
			  PostMessage (hDlg,WM_COMMAND,IDOK,0);
		  }
		  else if (HIWORD(wParam) == LBN_SELCHANGE)
		  {
			  SetTimer(hDlg, 1, 200, (FARPROC) 0);
			  break;
		  }
		  break;
      default:
         break;
         }
	   }
      break;

   default:
      break;
      }
   return (FALSE);
}  


DWORD GM32GetNodeInfo (LPSTR NodeName,LPSTR UserName,LPSTR Winver)
{
	DWORD	CSerno;
	DWORD	lNodeName=32;

    GetComputerName (NodeName,&lNodeName);
	GetWindowsVersion (Winver);
	CSerno = GetCDriveSerialNumber ();
	if (WNetGetUser (NULL,UserName,&lNodeName) != NO_ERROR)
		*UserName = 0;

	return CSerno;
}

//DWORD GM32GetFileName(DWORD hWnd16, DWORD hInstance16,DWORD Opt,LPSTR PathName,
//								 LPSTR Filters,DWORD lFilters,LPSTR InitialDirIn,LPSTR Title,DWORD Flags)
DWORD GM32GetGMCName(DWORD hWnd16,LPSTR PathName,
								 LPSTR InitialDirIn,LPSTR Title,DWORD pStartInNewSession,DWORD pRetainZoom,LPSTR NetDir,LPSTR PersonalDir)
{
	char LongName[256],ShortName[256], InitialDir[256], File[256]="";
	OPENFILENAME ofn;       // common dialog box structure
	LPOPENFILENAME	pOF=&ofn;
	char OriginalFilters[256];
	int	st;
	LPSHORT	pStartInNew=(LPSHORT)pStartInNewSession;
	LPSHORT	pRetainZm=(LPSHORT)pRetainZoom;
	DWORD	lFilters=FormatFilterString();
	LPSTR	pDOT, pBS;
	DWORD Flags;
	DWORD	Opt=1;
	HWND	hWnd= WOWHandle32((WORD) hWnd16, WOW_TYPE_HWND );
	//HINSTANCE hInstance = WOWHandle32((WORD) hInstance16, WOW_TYPE_HWND );

	strcpy (NetworkCFGDir,NetDir);
	strcpy (PersonalCFGDir,PersonalDir);
//	MessageBox (0,NetworkCFGDir,PersonalCFGDir,MB_OK);
	Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST |OFN_NOREADONLYRETURN|
								OFN_FILEMUSTEXIST|OFN_ENABLEHOOK| OFN_ENABLETEMPLATE ;
//   SendMessage( hWnd, WM_APP, MYFUNC1, 25 );
//   SendMessage( hWnd, WM_APP, MYFUNC2, 42 );
	strcpy (LongName,PathName);
//	memcpy (OriginalFilters,Filters,lFilters);
    GM32GetLongPathName (LongName,256);
	if ((pBS = strrchr (LongName,'\\')))
	{
		*pBS++ = 0;
		strcpy (File,pBS);
		strcpy (InitialDir,LongName);
	}
	else
		strcpy (InitialDir,InitialDirIn);

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpfnHook = OFNHookProcOldStyle;
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hinstDLL;
	ofn.lpstrFile = File;
	ofn.lpstrFilter = gszFilter;
	ofn.lpstrTitle = Title;
	ofn.lpstrInitialDir = RestartDir;
	ofn.Flags = Flags;
	ofn.nMaxFile = 256;
	strcpy (RestartDir,InitialDir);
	ofn.lpTemplateName = "OpenGMC";
	if ((pDOT = strrchr (File,'.')))
	{
		LPSTR pSuffix, pFilter = OriginalFilters;
		DWORD	i=1;
		while (*pFilter)
		{
			pSuffix = strchr (pFilter,0);
			pSuffix++;
			pFilter = strchr (pSuffix,0);
			pFilter++;
			if ((pSuffix = strchr (pSuffix,'.')))
			{
				if (!_stricmp (pDOT,pSuffix))
					pOF->nFilterIndex = i;
			}
			i++;
		}
	}
	st = 0;
	if (Opt == 1)
	{
		while (!st && *RestartDir)
		{
			st = GetOpenFileName(&ofn);
		}
	}
	else
		st = GetSaveFileName(&ofn);
	if (st)
	{
		if (pOF->lpstrFile)
		{
			LPSTR pBS;
			
			strcpy (ShortName,pOF->lpstrFile);
			pBS = strrchr (ShortName,'\\');
			if (!pBS)
				pBS = ShortName;
			else
				pBS++;
			if (_strnicmp (pBS,"INDEX",5) && !strchr (pBS,'.') && pOF->nFilterIndex)
			{
				LPSTR pFilter = OriginalFilters;
				DWORD	i;
				for (i=0;i+1<pOF->nFilterIndex;i++)
				{
					pFilter = strchr (pFilter,0)+1;
					pFilter = strchr (pFilter,0)+1;
				}
				pFilter = strchr (pFilter,0)+1;
				pDOT = strrchr (pFilter,'.');
				if (pDOT)
					strcat (ShortName,pDOT);
			}

/*			if (!GetShortPathName (ShortName,LongName,256))//short is really long
			{
				OFSTRUCT	OFStruct;

				HFILE	Fid2 = OpenFile (ShortName,&OFStruct,OF_CREATE);
				_lclose (Fid2);
				GetShortPathName (LongName,ShortName,256);
			}*/
			strcpy (PathName,ShortName);
			*pStartInNew = StartInNewSession;
			*pRetainZm = RetainZoom;
		}
	}
	return st;
}

DWORD GM32CompressFrame (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{   
	DWORD rtn=TRUE;   
	HIC	hIC; 
	long	lCmp; 
	long	ICRtn;
	DWORD	Flags;
    LPBYTE		pInData, pCompressedData;       
    char	str[16]="IV50";
	DWORD	CompressorID = mmioFOURCC(str[0],str[1],str[2],str[3]);

	pInData = (LPBYTE) lpbiIn +  
			  lpbiIn->biSize +
			  lpbiIn ->biClrUsed * sizeof(RGBQUAD);
 
   	AVIFileInit();
	hIC = ICOpen (ICTYPE_VIDEO,CompressorID,ICMODE_COMPRESS);
	ICRtn = ICCompressGetFormat(hIC,lpbiIn,lpbiOut);
 	pCompressedData = (LPBYTE) lpbiOut +  
			  lpbiOut->biSize +
			  lpbiOut->biClrUsed * sizeof(RGBQUAD);
   lCmp = ICCompressGetSize (hIC,lpbiIn,lpbiOut);   
	ICRtn = ICCompressBegin(hIC,lpbiIn,lpbiOut);  
	ICRtn = ICCompress (hIC,ICCOMPRESS_KEYFRAME,
										lpbiOut,pCompressedData,
										lpbiIn,pInData,
										NULL,&Flags,0,0,
										7200,NULL,NULL);  
	if (ICRtn != ICERR_OK) 
	{
			rtn = FALSE;
	}
	ICCompressEnd(hIC);
	ICClose (hIC);   
	AVIFileExit ();
	
	return rtn;
}


DWORD GM32OpenGarminUSB (LPSTR ProductID)
{
	return (DWORD) OpenGarminUSB (ProductID);
}

DWORD GM32CloseGarminUSB (LPSTR NullArg)
{
	return (DWORD) CloseGarminUSB (NullArg);
}

DWORD GM32SendPacketUSB (LPSTR Packet)
{
	return (DWORD)SendPacketP (Packet);
}

DWORD GMReadRegistry (LPSTR Key,LPSTR Value)
{
	return 0;
}

