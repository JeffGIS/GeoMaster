#include "graphint.h"

#include <shlobj.h>
#include <commdlg.h>
#include <stdio.h>
#include <direct.h>
#include <winnt.h> 
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
//#include <projects.h>
#include "gm32lib.h"


LPSTR	GetPacket (void);
BOOL SendPacketP (LPSTR pPacket);

static	BOOL	StartInNewSession=FALSE, RetainZoom=FALSE, LinkZoom=FALSE;
static	char	NetworkCFGDir[256];
static	char	PersonalCFGDir[256];
char	RestartDir[256]="";

HANDLE	hinstDLL;
#define HEAP_ALLOC(var,size) \
	lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

BOOL DisplayConfigPreview (HWND hWnd,HDC hDC,LPSTR Name,LPRECT ImageRect, UINT TextCntl);


BOOL DisplayHelp (LPSTR HelpFileIn,LPSTR HelpTopic)
{
	STARTUPINFO si;
	static	PROCESS_INFORMATION pi;
	static	First=TRUE;
	char	str[512];
	DWORD	rtn=FALSE;
	DWORD	CRFlags;
	char	HelpFile[MAX_PATH];
	OFSTRUCTGM	OFStruct;
	HFILE	Fid;

	if (HelpFileIn)
		strcpy (HelpFile,HelpFileIn);
	else
		GetGlobalCVal ("[%HELPFILE]",HelpFile,"[%DL]help\\gmhelp.chm");
	ExpandText (HelpFile);
	if ((Fid = GSSiOpenFile (HelpFile,&OFStruct,OF_READ)) == HFILE_ERROR)
		return FALSE;
	GSSiClose2 (&Fid);

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


time_t GetLastFileWriteTime (LPSTR File,LPLONG pTimeDiff)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME SysTime;
	HANDLE	hFile;
	time_t	rtn=0;
	long	TimeDiff;
	time_t	CurDateTime,CurDateTime1, LastWriteDateTime;
 
	hFile = CreateFile(File,           // open MYFILE.TXT 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY|FILE_FLAG_BACKUP_SEMANTICS,     // normal file 
                NULL);                     // no attr. template 
 
	if (hFile == INVALID_HANDLE_VALUE) 
			return 0;
	GetSystemTime (&SysTime);
	SystemTimeToFileTime (&SysTime,(LPFILETIME)&CurDateTime);
	SysTime.wSecond++;
	SystemTimeToFileTime (&SysTime,(LPFILETIME)&CurDateTime1);
	TimeDiff = CurDateTime1 - CurDateTime;
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    {
//			*(DWORD *)pLowTime = ftWrite.dwLowDateTime;
//			*(DWORD *)pHighTime = ftWrite.dwHighDateTime;
			FileTimeToSystemTime (&ftWrite,&SysTime);
			LastWriteDateTime = *(__int64 *)&ftWrite;
			TimeDiff = (CurDateTime - LastWriteDateTime)/10000000;
//			*pTimeDiff = TimeDiff;
			rtn = LastWriteDateTime;
	}
	//FileTimeToSystemTime (&ftWrite,&SysTime);
	CloseHandle(hFile); 
	return rtn;
}
time_t GetFileCreateTime (LPSTR File,LPLONG pTimeDiff)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME SysTime;
	HANDLE	hFile;
	time_t	rtn=0;
	long	TimeDiff;
	time_t	CurDateTime,CurDateTime1, LastWriteDateTime;
 
	hFile = CreateFile(File,           // open MYFILE.TXT 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY|FILE_FLAG_BACKUP_SEMANTICS,     // normal file 
                NULL);                     // no attr. template 
 
	if (hFile == INVALID_HANDLE_VALUE) 
			return 0;
	GetSystemTime (&SysTime);
	SystemTimeToFileTime (&SysTime,(LPFILETIME)&CurDateTime);
	SysTime.wSecond++;
	SystemTimeToFileTime (&SysTime,(LPFILETIME)&CurDateTime1);
	TimeDiff = CurDateTime1 - CurDateTime;
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    {
//			*(DWORD *)pLowTime = ftWrite.dwLowDateTime;
//			*(DWORD *)pHighTime = ftWrite.dwHighDateTime;
			FileTimeToSystemTime (&ftCreate,&SysTime);
			LastWriteDateTime = *(__int64 *)&ftCreate;
			TimeDiff = (CurDateTime - LastWriteDateTime)/10000000;
//			*pTimeDiff = TimeDiff;
			rtn = LastWriteDateTime;
	}
	//FileTimeToSystemTime (&ftWrite,&SysTime);
	CloseHandle(hFile); 
	return rtn;
}

BOOL GetDirectoryCreateTime (LPSTR File,LPDWORD pLowTime,LPDWORD pHighTime,LPDWORD pTimeDiff)
{
    FILETIME ftCreate, ftAccess, ftWrite;
    SYSTEMTIME SysTime, SysTimeCreate;
	HANDLE	hFile;
	DWORD	rtn=0;
	DWORD TimeDiff;
	__int64	CurDateTime, LastWriteDateTime;
 
	hFile = CreateFile(File,           // open MYFILE.TXT 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_FLAG_BACKUP_SEMANTICS,//FILE_ATTRIBUTE_DIRECTORY,//|FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 
 
	if (hFile == INVALID_HANDLE_VALUE) 
			return 0;
	GetSystemTime (&SysTime);
	SystemTimeToFileTime (&SysTime,(LPFILETIME)&CurDateTime);
    if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
    {
			*(DWORD *)pLowTime = ftCreate.dwLowDateTime;
			*(DWORD *)pHighTime = ftCreate.dwHighDateTime;
			LastWriteDateTime = *(__int64 *)&ftCreate;
			TimeDiff = CurDateTime - LastWriteDateTime;
			*(DWORD *)pTimeDiff = TimeDiff; //TimeDiff/36000
			rtn = 1;
	}
	//FileTimeToSystemTime (&ftWrite,&SysTime);
	FileTimeToSystemTime((LPFILETIME)&ftCreate,(LPSYSTEMTIME)&SysTimeCreate);
	CloseHandle(hFile); 
	return rtn;
}
DWORD GMURLDownloadToFile(LPSTR URL,LPSTR File)
{
	HRESULT rc;
	
	if (!makedirectories (File,FALSE,FALSE))
		return FALSE;
	CloseAllRequestedFiles (TRUE);
	rc = URLDownloadToFile (NULL,URL,File,0,NULL);
	if (rc == S_OK)
		return TRUE;
	if (rc == E_OUTOFMEMORY)
		return FALSE;
	return FALSE;
}

/*DWORD GM32DrawDibOpen (void)
{
	HDRAWDIB	hdd = DrawDibOpen ();

	return (DWORD) hdd;
}

DWORD GM32DrawDibClose (DWORD hdd)
{
	DWORD rtn = DrawDibClose ((HDRAWDIB)hdd);

	return rtn;
}
*/

DWORD GM32DrawLineWithFlatEnd (HDC hDC16,DWORD npt,LPPOINT pPoints32,DWORD Width,DWORD Color)
{
	HDC	hDC= hDC16;//WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	HPEN	hPen,OldPen;
	LPPOINT	Points = (LPPOINT)pPoints32;
	LOGBRUSH	lb;
	DWORD	rtn;

	lb.lbStyle = BS_SOLID;
	lb.lbColor = Color;
	lb.lbHatch = 0;
	hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,Width,&lb,0,0);
	OldPen = SelectObject (hDC,hPen);
	rtn = Polyline (hDC,Points,npt);
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);
	return rtn;
}

DWORD GM32DrawDibDraw (DWORD hdd,HDC hDC16,
						DWORD destx,DWORD desty,DWORD destwidth,DWORD destheight,
						DWORD lpbihead,DWORD lpdibits,
						DWORD sourcex,DWORD sourcey, DWORD sourcewidth,DWORD sourceheight,
						DWORD flags)
{
	HDC	hDC= hDC16;//WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	DWORD rtn;

	rtn = DrawDibDraw ((HDRAWDIB)hdd,hDC,destx,desty,destwidth,destheight,
						(LPBITMAPINFOHEADER)lpbihead,(LPVOID)lpdibits,
						sourcex,sourcey,sourcewidth,sourceheight,flags);
	return rtn;
}
						
DWORD GM32DrawDibBegin (DWORD hdd,HDC hDC16,DWORD destwidth,DWORD destheight,DWORD lpbihead,DWORD sourcewidth,DWORD sourceheight,DWORD flags)
{
	HDC	hDC= hDC16;//WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
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
	static BOOL	HaveLZOInit=FALSE;
	if (HaveLZOInit)
		return TRUE;
	if (lzo_init() != 0)
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

DWORD GM32StretchDIBits (HDC hDC16,long destX,long destY,long destW,long destH,long xoff,
						 long yoff,long bmWidth,long bmHeight,
						 DWORD pDibInfoD, DWORD pImageD,DWORD ColorType,DWORD RastOpts,DWORD pFactorD)
{
	HDC	hDC= hDC16;//WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	DWORD	rtn, OldMode=0;
	LPBYTE	pImage = (LPBYTE)pImageD;
	LPBITMAPINFOHEADER	pDibInfo = (LPBITMAPINFOHEADER)pDibInfoD;
	double	Factor = *(double*)pFactorD;
	POINT	pt;

//	MessageBox (0,"Made it in",NULL,MB_ICONEXCLAMATION);
	if (destW < 0 || bmWidth < 0 || destH < 0 || bmHeight < 0)
		return 0;
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

DWORD GM32DeleteLargeMemDC (HDC hDC16,DWORD hOldBitmap)
{
	DWORD	rtn;
	HDC	hDC= hDC16;//WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	HBITMAP hbmp = SelectObject(hDC, (HBITMAP)hOldBitmap);

	DeleteObject (hbmp);
	rtn = DeleteDC (hDC);
	return rtn;
}


HDC GM32LargeMemDC (HDC hDC16,LPDWORD pMemMapWidth,LPDWORD pMemMapHeight,LPDWORD phOldBitmap)
{
	HDC	rtn;
	DWORD MemMapWidth=*pMemMapWidth;
	DWORD MemMapHeight=*pMemMapHeight;
	HDC	hDC= hDC16;//WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	HDC	hdcMemMap = CreateCompatibleDC(hDC);    
	HBITMAP	hMemBitmap = CreateCompatibleBitmap (hDC,MemMapWidth,MemMapHeight); 
	HBITMAP hbmpOld;
	BITMAP	bm;
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
	GetObject(hMemBitmap, sizeof(bm), (LPSTR)&bm);
	hbmpOld = SelectObject(hdcMemMap, hMemBitmap);
	*(DWORD *)pMemMapWidth = MemMapWidth;
	*(DWORD *)pMemMapHeight = MemMapHeight;
	*(DWORD *)phOldBitmap = (DWORD)hbmpOld;
	rtn = hdcMemMap;//(DWORD)WOWHandle16 (hdcMemMap,WOW_TYPE_HDC);
	return rtn;
}

long CompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,DWORD DecompressedLen)
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


DWORD GM32GetSpecialDirectory (LPSTR Path)
{
	HRESULT st=0;
	int	Type=-1;

	if (!_stricmp (Path,"MY DOCUMENTS"))
		Type = CSIDL_PERSONAL;
	if (!_stricmp (Path,"DESKTOP"))
		Type = CSIDL_DESKTOP;
	if (!_stricmp (Path,"DESKTOPDIR"))
		Type = CSIDL_DESKTOPDIRECTORY;
	if (!_stricmp (Path,"APPDATA"))
		Type = CSIDL_APPDATA;
	if (!_stricmp (Path,"LOCALAPPDATA"))
		Type = CSIDL_LOCAL_APPDATA;
	if (!_stricmp (Path,"ALLUSERAPPDATA"))
		Type = CSIDL_COMMON_APPDATA;
	if (Type>-1)
 //remove for WINNT
//		st = SHGetSpecialFolderPath (0,Path,Type,0);
	st = SHGetFolderPath (0,Type,0,SHGFP_TYPE_CURRENT,Path);
	return (DWORD)st;
};

BOOL GetDriveUNC (LPSTR Drive,LPSTR Name)
{
	  DWORD cbBuff = 1000;    // Size of Buffer
	  TCHAR szBuff[1000];    // Buffer to receive information
	  REMOTE_NAME_INFO  * prni = (REMOTE_NAME_INFO *)   &szBuff;
	  // Pointers to head of buffer
	  UNIVERSAL_NAME_INFO * puni = (UNIVERSAL_NAME_INFO *) &szBuff;
	  DWORD res;

		res = WNetGetUniversalName(Drive,UNIVERSAL_NAME_INFO_LEVEL,
   //
   //The structure is written to this block of memory.
   //
		(LPVOID) &szBuff, 
		&cbBuff);
	if (!res)
	{
		LPSTR pEnd = strstr (puni->lpUniversalName," \\");

		if (pEnd)
			*pEnd = 0;
		strcpy (Name,puni->lpUniversalName);
		return TRUE;
	}
	*Name = 0;
	return FALSE;
}
BOOL IsLocalFile(LPSTR Name)
{
	char UNC[128];

	if (GetDriveUNC(Name, UNC))
		return FALSE;
	return TRUE;
}

BOOL isCardAvailable() 
{ 
        BOOL res = TRUE; 
        HANDLE hFlashCard = NULL; 
        WIN32_FIND_DATA find; 
        BOOL loop = TRUE; 
        BOOL found = FALSE; 
        UINT flashCardCount = 0; 
		char	m_CardPath[128];
		int	MAX_CARD_PATH = 128;
// The list of known on-board storage names 
static const TCHAR* STORAGE[] = 
{ 
        _T("iPAQ File Store"), 
        _T("Built-in Storage") 


}; 


        // Look for the SD card. 
        memset(&find, 0, sizeof(WIN32_FIND_DATA)); 


  //      hFlashCard = FindFirstFlashCard(&find); 


        if(INVALID_HANDLE_VALUE != hFlashCard) 
        { 
                // We must enumerate the flash cards, since the dumb 
                // iPAQ file store is defined as a flash card 
                while(loop) 
                { 
                        // Only look at the flash card if it is a directory and temporary 
                        if(((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) && 
							((find.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) == FILE_ATTRIBUTE_TEMPORARY)) 
                        { 
								UINT	i;
                                found = FALSE; 


                                for(i=0; i<2; i++) 
                                { 
                                        if(_tcscmp(find.cFileName, STORAGE[i]) == 0) 
                                        { 
                                                found = TRUE; 
                                        } 
                                } 


                                // Only count the card if it is the correct size 
                                if(found == FALSE) 
                                { 
                                        // Save the name of the flash card 
                                        _tcsncpy(m_CardPath, find.cFileName, MAX_CARD_PATH); 
                                        flashCardCount++; 
                                } 
                        } 


                        // Next flash card 
                        //loop = FindNextFlashCard(hFlashCard, &find); 
                } 


                FindClose (hFlashCard); 


                // If no flash cards were found in the enumeration, then leave 
                if(flashCardCount == 0) 
                { 
                        res = FALSE; 
                } 
        } 
        else 
        { 
                // No flash cards found 
                _stprintf(m_CardPath, _T("ERR: %d"), GetLastError()); 
                res = FALSE; 
        } 


        return res; 

} 

BOOL	GetSDChipSerialNumber (LPSTR Drive)
{
	char	Name[256]="\\.\\C:";
	char	UNC[128];
	BOOL	rtn=FALSE;
    // These values are only defined in Platform Builder, so we have to 
    // redefine them here 
    #define IOCTL_DISK_BASE           FILE_DEVICE_DISK 
    #define IOCTL_DISK_GET_STORAGEID  CTL_CODE(IOCTL_DISK_BASE, 0x709,METHOD_BUFFERED, FILE_ANY_ACCESS) 
    #define MANUFACTUREID_INVALID     0x01 
    #define SERIALNUM_INVALID         0x02 
    #define BBUF_LENGTH				256
    #define STR_LENGTH				256
	#define SERIAL_NUM_LENGTH	8

	// This structure is only defined in Platform Builder, so we have to 
    // redefine it here 
    typedef struct _STORAGE_IDENTIFICATION 
    { 
            DWORD    dwSize; 
            DWORD    dwFlags; 
            DWORD    dwManufactureIDOffest; 
            DWORD    dwSerialNumOffset; 
    } STORAGE_IDENTIFICATION, *PSTORAGE_IDENTIFICATION; 


    BOOL res = TRUE;
    BYTE bbuf[BBUF_LENGTH]; 
    TCHAR str[STR_LENGTH]; 
    STORAGE_IDENTIFICATION* si; 
    ULONG dwNumReturned = 0; 
    ULONG err = 0;
	HANDLE	hCard;
	
//	if (!GetDriveUNC (Drive,UNC))
//		return FALSE;
	si = malloc (sizeof(STORAGE_IDENTIFICATION));
	hCard = CreateFile(Name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL); 


                // Set the size element of the STORAGE_IDENTIFICATION structure 
                si->dwSize = BBUF_LENGTH; 


                // Fill the STORAGE_IDENTIFICATION structure with the flash card info 
                if(DeviceIoControl(hCard, IOCTL_DISK_GET_STORAGEID, (LPVOID)NULL, 0, 
					si, sizeof(STORAGE_IDENTIFICATION), &dwNumReturned, NULL) == FALSE)
                { 
                        err = GetLastError(); 
                        res = FALSE; 
                } 


                // Close the handle 
    CloseHandle (hCard); 


	return rtn;
}

BOOL GetDriveLetterFromLocalDriveName (LPSTR Name,LPSTR Drive)
{
	char Drives[256];
	LPSTR	pDrive;
	DWORD	VSN,MXFN,FSOPTS;
	char	DriveName[64],FileSystemName[64];

	GetLogicalDriveStrings(256,Drives);
	pDrive = Drives;
	while (*pDrive)
	{

		if (GetVolumeInformation(pDrive,DriveName,64,&VSN,&MXFN,&FSOPTS,FileSystemName,64))
		{
			if (!stricmp (DriveName,Name))
			{
				strcpy (Drive,pDrive);
				return TRUE;
			}
		}
		pDrive = strchr (pDrive,0);
		pDrive++;
	}
	return FALSE;
}


DWORD	GetDriveSerialNumber (LPSTR Drive)
{
	DWORD	VSN,MXFN,FSOPTS;
	char	str[256],str2[256];

	if (!GetVolumeInformation(Drive,str,256,&VSN,&MXFN,&FSOPTS,str2,256))
		VSN = 0;	
	return VSN;
}

void GetWindowsVersion (LPSTR Ver)
{
	OSVERSIONINFOEX verinfo;
	
	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&verinfo);

	sprintf(Ver, "%d.%d:%s",verinfo.dwMajorVersion,verinfo.dwMinorVersion,verinfo.szCSDVersion);
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

int GM32GetDeviceCaps(HDC hDC, DWORD opt)
{
//	HDC	hDC= WOWHandle32((WORD) hDC16, WOW_TYPE_HDC );
	int	rtn = GetDeviceCaps(hDC,opt);

	return rtn;
}

DWORD SearchDirectory32 (LPSTR Name,DWORD UseHandle,LPDWORD pType,WIN32_FIND_DATA	*pFindFileData)
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
		*pType = 1;
	else
		*pType = 0;
	if (pFindFileData)
		*pFindFileData = FindFileData;
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
	if (!strncmp (pBS,"\\",2))
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

/*DWORD WINAPI GM32GetShortPathName (LPSTR Name,DWORD MaxLen)
{
	char NewName[256];
	DWORD	rtn=TRUE;
	OFSTRUCTGM	OFStruct;
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
	OFSTRUCTGM	OFStruct;
	HFILE Fid = OpenFileGSSi (Name,&OFStruct,OF_CREATE,0); 
	
	if (Fid == HFILE_ERROR)
		return FALSE;
	_close (Fid);
	return TRUE;
}*/

DWORD GM32rename (LPSTR FromName,LPSTR ToName)
{

	return (DWORD)rename (FromName,ToName);
}

short GSSiMakeDir (LPSTR InName,LPDWORD ErrCode)
{
	DWORD	rtn, Err;
	char	Name[MAX_PATH];

	strcpy (Name,InName);
	ConvertToNewLocation (Name,FALSE);
	rtn = GetPathType (Name);
	
	if (rtn == 1)
	{
		*ErrCode = ERROR_ALREADY_EXISTS;
		return FALSE;
	}
	if (rtn == 2)
		return TRUE;

	*ErrCode = 0;
	rtn = CreateDirectory (Name,NULL);
	if (rtn == 0)
	{
		*ErrCode = GetLastError ();
		return FALSE;
	}
	return TRUE;
}

DWORD GM32RemoveDir (LPSTR Name)
{
	return (DWORD)_rmdir (Name);
}

DWORD GM32Remove (LPSTR InName)
{
	//return 0 if successfull or error code if not
	char	Name[MAX_PATH];
	int		st,ii;
	
	strcpy (Name,InName);
	ConvertToNewLocation (Name,FALSE);
	st=DeleteFile (Name);
	if (!st)
	{
		st = GetLastError();
		if (st != 2)
			ii = st;
	}
	else
		st = 0;

	return st;
}

UINT_PTR CALLBACK OFNHookProcOldStyle(HWND hDlg, UINT message,
                            WPARAM wParam, LPARAM lParam)

{  
	LPDRAWITEMSTRUCT lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	RECT	Rect;
	static	RECT	ListRect;
	char	str[512], FileName[MAX_PATH];
	int	choice;
	int	ii, OldMode;
	LPSTR	pEnd;
	static	inc=0;
	static	int	curpos;
	HBRUSH	hBrush;

//return FALSE;
	if (wParam == CDN_INCLUDEITEM )
		ii=1;
   switch (message)
      {
   case WM_INITDIALOG:
	   curpos = 0;
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
		SendDlgItemMessage (hDlg,IDC_LINKZOOM,BM_SETCHECK,LinkZoom,0L); 
//		SendDlgItemMessage (hDlg,1120,LB_SETCOLUMNWIDTH,(WPARAM)0,(LPARAM) 1.2*(ListRect.bottom - ListRect.top -16)/3); 
      break;

    case WM_MEASUREITEM: 
//		break;
		switch(wParam)

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
		switch(wParam)
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
					InflateRect (&Rect,-1,-1);
					hBrush = CreateSolidBrush (RGB(240,240,240));
					FillRect (lpdis->hDC,&Rect,hBrush);
					DeleteObject (hBrush);
					FrameRect (lpdis->hDC,&Rect,GetStockObject(GRAY_BRUSH));
					OldMode = SetBkMode (lpdis->hDC,TRANSPARENT);
					strlwr (FileName);
					TextOut (lpdis->hDC,lpdis->rcItem.left+8,lpdis->rcItem.top+2,FileName,strlen(FileName));
					SetBkMode (lpdis->hDC,OldMode);
					Rect.top += 16;
					Rect.bottom -=2;
					InflateRect (&Rect,-1,-1);
					DisplayConfigPreview (hDlg,lpdis->hDC,str,&Rect,0);
					if (lpdis->itemState)
						DrawFocusRect (lpdis->hDC,&lpdis->rcItem);
					{
					  int	NumFiles = SendDlgItemMessage(hDlg,1120, LB_GETCOUNT,0,0);
					  int	TopIndex = SendDlgItemMessage(hDlg,1120, LB_GETTOPINDEX,0,0);

					 SetScrollRange (GetDlgItem (hDlg,IDC_SCROLLBAR),SB_CTL,0,max(0,(NumFiles-3)/3-1),FALSE);
					 SetScrollPos (GetDlgItem (hDlg,IDC_SCROLLBAR),SB_CTL,max(0,TopIndex/3),TRUE);
					}
					return TRUE;
				case ODA_FOCUS:
					{
					  int	NumFiles = SendDlgItemMessage(hDlg,1120, LB_GETCOUNT,0,0);

					 SetScrollRange (GetDlgItem (hDlg,IDC_SCROLLBAR),SB_CTL,0,max(0,(NumFiles-3)/3-1),FALSE);
					 //SetScrollPos (GetDlgItem (hDlg,IDC_SCROLLBAR),SB_CTL,0,TRUE);
					}
					break;
			}
			break;
			case IDC_PREVGMC:
			{   
				RECT	ImageRect, TextRect;
				
				GetDlgItemText (hDlg,1088,str,256);
				GetClientRect (GetDlgItem (hDlg,IDC_PREVGMC),&ImageRect); 
				InflateRect (&ImageRect,-1,-1);
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
	  switch(wParam)
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


   case WM_HSCROLL:
	   {
		int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
		int	nPos = (short int) HIWORD(wParam);   // scroll box position 
		switch (nScrollCode)
		{
		  case SB_LINEDOWN: 
		  curpos = GetScrollPos ((HWND)lParam,SB_CTL)*3+3;
		  break;

		  case SB_LINEUP:
		  curpos = GetScrollPos ((HWND)lParam,SB_CTL)*3-3;
	  	  break;

		  case SB_PAGEDOWN:
		  curpos += 9;
	  	  break;

		  case SB_PAGEUP:
		  curpos -= 9;
	  	  break;

		  case SB_THUMBPOSITION:
			  curpos = nPos*3;
		  break;  
		  
		  case SB_THUMBTRACK:
			  curpos = nPos*3;
		  case SB_ENDSCROLL:
			 // curpos = GetScrollPos ((HWND)lParam,SB_CTL);
			  SetScrollPos((HWND)lParam, SB_CTL, max(0,curpos/3), TRUE);
			  SendDlgItemMessage(hDlg,1120, LB_SETTOPINDEX,max(0,curpos),0);
			  break;

		  default:
			  break;
		}
	   }
	   break;

   case WM_COMMAND:
	   {
		   int cntl = LOWORD (wParam);
		   int cmd  = HIWORD (wParam);
	  switch(LOWORD (wParam))
      //switch (wParam)
         {
	  case IDOK:
		  StartInNewSession = SendDlgItemMessage (hDlg,IDC_STARTINNEWSESSION,BM_GETCHECK,0,0L);
		  RetainZoom = SendDlgItemMessage (hDlg,IDC_RETAINZOOM,BM_GETCHECK,0,0L);
		  LinkZoom = SendDlgItemMessage (hDlg,IDC_LINKZOOM,BM_GETCHECK,0,0L);
		  KillTimer (hDlg,1);
		  break;
	  case IDC_SET_DEFAULTCONFIG:
		  {
			  char IniFile[MAX_PATH]="[%USERDIR]geomastr.ini";

			  ExpandText (IniFile);
			  GetDlgItemText (hDlg,1088,str,256);
			  strlwr (str);
			  if (!strstr (str,".gmc"))
			  {
				strcat (str,"\\");
				GetDlgItemText (hDlg,1152,strchr(str,0),256);
			  }
			  strlwr (str);
			  SubstituteDL (str,FALSE);
			  WritePrivateProfileString ("User","DefaultConfig",str,IniFile);
		  }
		  break;
	  case IDC_LINKZOOM:
		  if (SendDlgItemMessage (hDlg,IDC_LINKZOOM,BM_GETCHECK,0,0L))
		  {
			  SendDlgItemMessage (hDlg,IDC_RETAINZOOM,BM_SETCHECK,TRUE,0L); 
			  EnableWindow (GetDlgItem(hDlg,IDC_RETAINZOOM),FALSE);
		  }
		  else
			  EnableWindow (GetDlgItem(hDlg,IDC_RETAINZOOM),TRUE);
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
		  SetScrollPos(GetDlgItem(hDlg,IDC_SCROLLBAR), SB_CTL, 0, TRUE);
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
			  SetTimer(hDlg, 1, 200, (TIMERPROC) 0);
			  break;
		  }
		  break;
	  case 1152:
		 switch(HIWORD(wParam))
		 {   
			case EN_CHANGE:
				GetDlgItemText (hDlg,1152,str,sizeof(str));
				if (str[0])
				{
					int	index = SendDlgItemMessage (hDlg,1120,LB_FINDSTRING,-1,(LPARAM)str);  
					if (index > -1)
						SendDlgItemMessage (hDlg,1120,LB_SETTOPINDEX,index,0);
				}
			 break;
		}
		  break;
	  case IDC_SCROLLBAR:
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
	DWORD	lNodeName= MAX_COMPUTERNAME_LENGTH;

    if (!GetComputerName (NodeName,&lNodeName))
		*NodeName = 0;
	GetWindowsVersion (Winver);
	CSerno = GetDriveSerialNumber ("c:\\");
	if (WNetGetUser (NULL,UserName,&lNodeName) != NO_ERROR)
		*UserName = 0;

	return CSerno;
}

//DWORD GM32GetFileName(DWORD hWnd16, DWORD hInstance16,DWORD Opt,LPSTR PathName,
//								 LPSTR Filters,DWORD lFilters,LPSTR InitialDirIn,LPSTR Title,DWORD Flags)
BOOL GM32GetGMCName(HWND hWnd,LPSTR PathName,
								 LPSTR InitialDirIn,LPSTR Title,LPBOOL pStartInNew,LPBOOL pRetainZoom,LPBOOL pLinkZoom,LPSTR NetDir,LPSTR PersonalDir)
{
	char LongName[256],ShortName[256], InitialDir[256], File[256]="";
	OPENFILENAME ofn;       // common dialog box structure
	LPOPENFILENAME	pOF=&ofn;
	char OriginalFilters[256];
	int	st;
	DWORD	lFilters=FormatFilterString();
	LPSTR	pDOT, pBS;
	DWORD Flags;
	DWORD	Opt=1;
//	HWND	hWnd= hWnd16;//WOWHandle32((WORD) hWnd16, WOW_TYPE_HWND );
	//HINSTANCE hInstance = WOWHandle32((WORD) hInstance16, WOW_TYPE_HWND );
	int	CurDrive = _getdrive();
	_getcwd (CurDir,MAX_PATH);  
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
	strlwr (LongName);
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
				OFSTRUCTGM	OFStruct;

				HFILE	Fid2 = OpenFile (ShortName,&OFStruct,OF_CREATE);
				_lclose (Fid2);
				GetShortPathName (LongName,ShortName,256);
			}*/
			strcpy (PathName,ShortName);
			*pStartInNew = StartInNewSession;
			*pRetainZoom = RetainZoom;
		}
	}
	_chdir (CurDir);
	_chdrive (CurDrive);
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

