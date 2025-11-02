#include "graphint.h"   
#include "dgnlib.h"
//#include "dgn7.h"  
typedef DGNElemCore	 *LPDGNElementCore;

#include "gmextern.h"   
#include "wownt16.h"  
#include <errno.h>       

typedef void (FAR PASCAL *MYPROC)(LPSTR);
typedef void (FAR PASCAL *MYPROC1)(LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC2)(LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC3)(LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC4)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC5)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC6)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC7)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC8)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC9)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);
typedef void (FAR PASCAL *MYPROC10)(LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR,LPSTR);

#define MYFUNC1 1
#define MYFUNC2 2
#define MAX_OPEN_DGNFILES	32   
#define MAXVPTILEWIDTH	4096*4
#define MAXVPTILEHEIGHT	4096*4

static DWORD ghLib=0; 
static HMODULE ghUserLib=0;
static DWORD ghDGNlib7=0;  
static DWORD ghFreeImagLib=0;
static HMODULE ghBlatLib=0;
static MYPROC Proc;   
static MYPROC1 Proc1;   
static MYPROC2 Proc2;   
static MYPROC3 Proc3;   
static MYPROC4 Proc4;   
static MYPROC5 Proc5;   
static MYPROC6 Proc6;   
static MYPROC7 Proc7;   
static MYPROC8 Proc8;   
static MYPROC9 Proc9;   
static MYPROC10 Proc10;   
static HANDLE OpenDGNHandles[MAX_OPEN_DGNFILES];
static BOOL  CloseDGNRequested[MAX_OPEN_DGNFILES]; 
static char	 OpenDGNFileName[MAX_OPEN_DGNFILES][MAX_PATH];
static MNMXCORD	OpenDGNBounds[MAX_OPEN_DGNFILES];
HINSTANCE hInstKernel = NULL;
extern char	VirtPrinterImageFile[256];

DWORD __declspec(dllexport) DGN7GetNumElements (HANDLE hDGN);
LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );


void MyFunc1( UINT );
void MyFunc2( UINT );
DWORD BitMask (LPSTR bitstring);

   // WOW_HANDLE_TYPE enumerated type for WOWHandle32():
   typedef enum _WOW_HANDLE_TYPE {
   WOW_TYPE_HWND,
   WOW_TYPE_HMENU,
   WOW_TYPE_HDWP,
   WOW_TYPE_HDROP,
   WOW_TYPE_HDC,
   WOW_TYPE_HFONT,
   WOW_TYPE_HMETAFILE,
   WOW_TYPE_HRGN,
   WOW_TYPE_HBITMAP,
   WOW_TYPE_HBRUSH,
   WOW_TYPE_HPALETTE,
   WOW_TYPE_HPEN,
   WOW_TYPE_HACCEL,
   WOW_TYPE_HTASK,
   WOW_TYPE_FULLHWND
   } WOW_HANDLE_TYPE;


   // Typedef for kernel function pointers needed for
   // thunking (found in krnl386.dll):
   typedef DWORD (FAR PASCAL * LOADLIBRARYEX32W)(LPCSTR lpszLibFile,
      DWORD hFile, DWORD dwFlags);
   typedef DWORD (FAR PASCAL * GETPROCADDRESS32W)(DWORD hModule,
      LPCSTR lpszProc);
   typedef DWORD (FAR PASCAL * FREELIBRARY32W)(DWORD hLibModule);
   typedef DWORD (FAR CDECL  * CALLPROCEX32W)( DWORD, DWORD,
      DWORD, ... );
   // Typedef for Wow32 function pointer needed for converting
   // 16-bit window handle:
   typedef DWORD (FAR PASCAL * WOWHANDLE32)(WORD Handle,
      WOW_HANDLE_TYPE Type);
   // Declare global function pointers:
static LOADLIBRARYEX32W lpfnLoadLibraryEx32W = NULL;
static GETPROCADDRESS32W lpfnGetProcAddress32W = NULL;
static FREELIBRARY32W lpfnFreeLibrary32W = NULL;
static CALLPROCEX32W lpfnCallProcEx32W = NULL;
static WOWHANDLE32 lpfnWOWHandle32 = NULL;


  // LoadGenericThunkFuncs() dynamically loads the functions
   // necessary to perform the generic thunk from "kernel"
   // located in krnl386.dll. 
DWORD BitMask (LPSTR bitstring)
{
	DWORD	rtn=0; 
	short	ibit=0;
	
	while (*bitstring)
	{
		if (*bitstring++ == '1')
			SetBit2 (ibit,(LPSTR)&rtn,1);
		ibit++;
	}
	return rtn;
} 


BOOL LoadDGNlib7(BOOL Close)
{   
	char	DGN7Lib[128]="[%DLLDIR]DGNlib7.dll";
	char	DGN7LibDL[128]="[%DL]DGNlib7.dll";
	static	BOOL	Attempted = FALSE;
	short	i;
	
	if (Attempted)
		return TRUE;
/*	if (!ghLib)
		return FALSE;
    if (Close)
    {
    	if (ghDGNlib7)
    		FreeLibrary( ghDGNlib7 ); 
    	ghDGNlib7 = 0;
    	return TRUE;
    }
    if (ghDGNlib7)
    	return TRUE; 
    if (Attempted)
    	return FALSE;
    ExpandText (DGN7Lib);  
    ExpandText (DGN7LibDL);  
    Attempted = TRUE;
#if CHECKMEM
	if (!(ghDGNlib7 = LoadLibrary( "..\\DGNlib7\\debug\\DGNlib7.dll")))  
	if (!ghDGNlib7 && !(ghDGNlib7 = LoadLibrary( DGN7LibDL)))  
#else
	if (!(ghDGNlib7 = LoadLibrary( DGN7LibDL))) 
	{
//		MessageBox (0,DGN7Lib,"",MB_ICONEXCLAMATION);
	}
	else
	{   
//		MessageBox (0,DGN7Lib,"",MB_ICONEXCLAMATION);
	}
	if (!ghDGNlib7 && !(ghDGNlib7 = LoadLibrary( DGN7Lib)))  
#endif
	{
		MessageBox (0,"Unable to load DGNlib7",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
*/
  for (i=0;i<MAX_OPEN_DGNFILES;i++)
	{
		OpenDGNHandles[i] = 0;
		CloseDGNRequested[i]=FALSE;
	}
    Attempted = TRUE;
	return TRUE;
}

BOOL LoadFreeImageLib (BOOL Close)
{   
	char	FreeImagLib[128]="[%DL]FreeIm16.dll";
	static	BOOL	Attempted = FALSE;
	
	if (!ghLib)
		return FALSE;  
	ghFreeImagLib = ghLib;
	return TRUE;
/*    if (Close)
    {
    	if (ghFreeImagLib)
    		lpfnFreeLibrary32W( ghFreeImagLib ); 
    	ghFreeImagLib = 0;
    	return TRUE;
    }
    if (ghFreeImagLib)
    	return TRUE; 
    if (Attempted)
    	return FALSE;
    ExpandText (FreeImagLib);
    Attempted = TRUE;
#if CHECKMEM
//	if (!(ghFreeImagLib = lpfnLoadLibraryEx32W( "..\\freeimag\\freeim16\\debug\\freeim16.dll", NULL, 0 )))  
	if (!(ghFreeImagLib = lpfnLoadLibraryEx32W( FreeImagLib, NULL, 0 )))  
#else
	if (!(ghFreeImagLib = lpfnLoadLibraryEx32W( FreeImagLib, NULL, 0 )))  
#endif
	{
		MessageBox (0,"Unable to load FreeImageLib",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;*/
}   

BOOL LoadUserLib (BOOL Close)
{   
	char	UserLib[MAX_PATH];
	static	BOOL	Attempted = FALSE;
	
    if (Close)
    {
    	if (ghUserLib)
    		FreeLibrary( ghUserLib ); 
    	ghUserLib = 0;
    	return TRUE;
    }
    if (ghUserLib)
    	return TRUE; 
    if (Attempted)
    	return FALSE;  
    GetGlobalCVal ("[%USERLIB32]",UserLib,"[%DL]userlib32.dll");
    ExpandText (UserLib);
    Attempted = TRUE;
	ConvertFileNameToCacheFileName (UserLib);
	if (!(ghUserLib = LoadLibrary( UserLib)))  
	{
		DWORD err = GetLastError();
		char mess[256];
		sprintf(mess, "Unable to load UserLib: %i", err);
		MessageBox (0,mess,UserLib,MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}   

BOOL RunUserFunction (LPSTR FunctionName,LPSTR ReturnValue,short NumArgs,LPSTR *Args)
{
	MYPROC	Proc; 
	BOOL	rtn=FALSE;
	
	if (!ghUserLib)
		return FALSE;
	Proc = (MYPROC) GetProcAddress( ghUserLib, FunctionName );
   	if(Proc == NULL )
   		return FALSE;
   	
   	switch (NumArgs)
   	{
   		case 0:
		   	(Proc) (ReturnValue);
		   	break;
   		case 1:
			Proc1 = (MYPROC1)Proc;
		   	(Proc1) (ReturnValue, Args[0]);
		   	break; 
   		case 2:
			Proc2 = (MYPROC2)Proc;
		   	(Proc2) (ReturnValue, Args[0], Args[1]);
		   	break; 
   		case 3:
			Proc3 = (MYPROC3)Proc;
		   	(Proc3) (ReturnValue, Args[0], Args[1], Args[2]);
		   	break; 
   		case 4:
			Proc4 = (MYPROC4)Proc;
		   	(Proc4) (ReturnValue, Args[0], Args[1], Args[2], Args[3]);
		   	break; 
   		case 5:
			Proc5 = (MYPROC5)Proc;
		   	(Proc5) (ReturnValue, Args[0], Args[1], Args[2], Args[3], Args[4]);
		   	break; 
   		case 6:
			Proc6 = (MYPROC6)Proc;
		   	(Proc6) (ReturnValue, Args[0], Args[1], Args[2], Args[3], Args[4], Args[5]);
		   	break; 
   		case 7:
			Proc7 = (MYPROC7)Proc;
		   	(Proc7) (ReturnValue, Args[0], Args[1], Args[2], Args[3], Args[4], Args[5], Args[6]);
		   	break; 
   		case 8:
			Proc8 = (MYPROC8)Proc;
		   	(Proc8) (ReturnValue, Args[0], Args[1], Args[2], Args[3], Args[4], Args[5], Args[6], Args[7]);
		   	break; 
   		case 9:
			Proc9 = (MYPROC9)Proc;
		   	(Proc9) (ReturnValue, Args[0], Args[1], Args[2], Args[3], Args[4], Args[5], Args[6], Args[7], Args[8]);
		   	break; 
   		case 10:
			Proc10 = (MYPROC10)Proc;
		   	(Proc10) (ReturnValue, Args[0], Args[1], Args[2], Args[3], Args[4], Args[5], Args[6], Args[7], Args[8],
		   																							Args[9]);
		   	break; 
	}
	return rtn;
}

BOOL GSSiGetNodeInfo (LPSTR CDriveSerno,LPSTR NodeName,LPSTR UserName,LPSTR Winver)
{
	DWORD Serno = GM32GetNodeInfo (NodeName,UserName,Winver);

   	sprintf (CDriveSerno,"%lu",(long)Serno);
	return TRUE;
}

BOOL DisplayHTMLHelp (LPSTR HelpFile,LPSTR Topic)
{
	DWORD	rtn; 
	
	 
   	rtn = DisplayHelp (HelpFile,Topic); 
	return rtn;
}

/*short GSSiMakeDir (LPSTR InName,LPDWORD pErrCode)
{
	MYPROC Proc; 
	short	ln;
	char	Name[256];
	
	_fstrcpy (Name,InName);
	ExpandText (Name);
	*pErrCode = 0;
	if (!ghLib)
	{ 
NoLib:
		ln = _mkdir (Name);  
		if (ln < 0 && errno != EEXIST)
		{
			*pErrCode = errno;
			return FALSE;
		}
		return TRUE;
	}
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32MakeDir" );
   	if(Proc == NULL )
		goto NoLib;
   	
   	ln = lpfnCallProcEx32W(2,BitMask("11"),(DWORD)Proc, (DWORD)Name, (DWORD)pErrCode);
	return ln;
}*/

short GSSiRemove32 (LPSTR Name)
{
	MYPROC Proc; 
	short	ln;
	
	return GM32Remove (Name);

} 

int GSSiRemoveDir (LPSTR Name)
{
	return _rmdir (Name);
}


BOOL GetShortPathName2 (LPSTR Name,short MaxLen)
{
	MYPROC Proc; 
	DWORD	ln;
	char	ShortName[MAX_PATH];

	ln = GetShortPathName (Name,ShortName,MaxLen);
	if (ln)
		strcpy (Name,ShortName);
	return ln;
}


BOOL GetLongPathName3 (LPSTR Name,short MaxLen)
{
	MYPROC Proc; 
	DWORD	ln;
	char	LongName[MAX_PATH];

	
	ln = GetLongPathName (Name,LongName,MaxLen);
	if (ln && ln < MaxLen)
		strcpy (Name,LongName);
	return ln;
}
BOOL CopyDirectory(LPSTR toDir, LPSTR fromDir, BOOL replace, LPSTR statusTitle)
{
	BOOL rtn = FALSE;
	int exists = FileType(toDir);
	char	TempName[MAX_PATH], fromPath[MAX_PATH+2], toPath[MAX_PATH+2];
	char	fileName[MAX_PATH];
	char	errmess[MAX_PATH * 2];
	HFILE	Fid;
	long	TotFiles = 0;
	int		ldir, lfile;
	int		numDone = 0;

	sprintf(errmess, "[%%ERRMESS]=;", fromPath);
	ProcessText(errmess);

	if (exists && !replace)
		return rtn;
	if (exists == 1 && replace)
		GSSiRemove(toDir);
	if (exists == 2 && replace)
	{
		rtn = DeleteDirAndContents(toDir);
		if (!rtn)
		{
			sprintf(errmess, "[%%ERRMESS]=Failed to delete %s;", toDir);
			ProcessText(errmess);
			return rtn;
		}
	}

	ExpandText(fromDir);
	ldir = strlen(fromDir);
	GSSiGetTempFileName(0, "gmc", 0, TempName);
	Fid = GSSiOpenFile(TempName, 0, OF_CREATE);
	SearchFilesInDir(fromDir, "*", Fid, &TotFiles, "", 1, TRUE,TRUE);
	GSSillseek(Fid, 0, 0);
	if (*statusTitle)
		CreateStatusWind(hWndMain, 1, statusTitle);
	rtn = TRUE;
	while (rtn && StatusWindowUpdate(0, "", TotFiles,numDone++) && fgetstring(fromPath, MAX_PATH, Fid))
	{
		sprintf(toPath, "%s%s", toDir, &fromPath[ldir]);
		rtn = GSSiCopyFile(fromPath, toPath, FALSE);
	}
	GSSiClose2 (&Fid);
	GSSiRemove(TempName);
	if (*statusTitle)
		DestroyStatusWindow(0);
	if (!rtn)
	{
		sprintf(errmess, "[%%ERRMESS]=Failed to copy %s;", fromPath);
		ProcessText(errmess);
	}
	return rtn;
}

BOOL GSSiCopyFile (LPSTR OldName,LPSTR NewName,BOOL Replace)
{
	short	ln=0; 
	DWORD	FailIfExists=TRUE;
	short	AppendOrReplace=0;
	char	fromPath[MAX_PATH], toPath[MAX_PATH];

	if (Replace) 
	{
		AppendOrReplace = 0;
		FailIfExists = FALSE;
	}
	strcpy(fromPath, OldName);
	ExpandText(fromPath);
	strcpy(toPath, NewName);
	ExpandText(toPath);
	ConvertToNewLocation(fromPath, FALSE);
	ConvertToNewLocation(toPath, FALSE);
	if (UndoEnabled)
		return (copyfile(toPath, fromPath, AppendOrReplace, 0, 0, 0, 0, 0, 0));
	makedirectories(toPath, FALSE, FALSE);
	if (!strnicmp(fromPath, "ftp:", 4) || !strnicmp(fromPath, "http:", 5) || !strnicmp(fromPath, "https:", 6))
	{
		if (Replace && ExistFile(toPath))
			GSSiRemove(toPath);
		ln = URLToFile(fromPath, toPath);
	}
	else
	{
		char mess[256];
		ln = CopyFile(fromPath, toPath, FailIfExists);
		if (!ln)
		{
			DWORD ierr = GetLastError();
			GetSystemErrMessage(ierr,mess);
		}
		FileAlreadyNotFound(toPath, 3, 0);
	}
	return ln;
}

BOOL GSSirenamefile (LPSTR OldNameIn,LPSTR NewNameIn)
{
	MYPROC Proc; 
	short	ln;
	char	OldName[MAX_PATH], NewName[MAX_PATH];
	int rtn;
	strcpy (OldName,OldNameIn);
	strcpy (NewName,NewNameIn);
	ConvertToNewLocation (OldName,FALSE);
	ConvertToNewLocation (NewName,FALSE);
	
//	if (!ghLib)
	rtn = MoveFileEx(OldName, NewName, MOVEFILE_WRITE_THROUGH);
  // 		rtn = rename (OldName,NewName);
		if (rtn)
			return TRUE;
		else
			return FALSE;
/*	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32rename" );
   	if(Proc == NULL )
   		return (!rename (OldName,NewName));
   	
   	ln = lpfnCallProcEx32W(2,BitMask("11"),(DWORD)Proc, (DWORD)OldName, (DWORD)NewName);
	return ln;*/
}

/*DWORD CreateLongNameFile (LPSTR Name)
{
	return (GM32CreateFile (Name));
}*/ 



DWORD GetSpecialDirectory (LPSTR Name)
{
   	return GM32GetSpecialDirectory (Name);
}

BOOL GSSiGetFileName (short opt,HWND hWnd,HINSTANCE hInstance,LPSTR PathName,LPCSTR Filters,DWORD lFilters,LPCSTR InitialDir,LPCSTR Title,DWORD Flags)
{
	return FALSE;
}

DWORD DGN7CloseActual (HANDLE hDGN)
{
	DWORD	st=TRUE;
    
//    SetWindowText (hWndMain,"DGN7CloseActual");
   	DGNClose (hDGN);
	
	return st; 
}


void AddToOpenDGNFiles (LPSTR Name,HANDLE hDGN,LPMNMXCORD pBounds)
{   
	char	ExpandedName[MAX_PATH]; 
	short	i;
    
    if (!KeepFilesOpen)  
    	return;
Top:	
	for (i=0;i<MAX_OPEN_DGNFILES;i++)
	{
		if (!OpenDGNHandles[i])  
		{
			OpenDGNHandles[i] = hDGN;
			CloseDGNRequested[i]=FALSE;   
			_fstrcpy (ExpandedName,Name);
			ExpandText (ExpandedName);
			_fullpath (OpenDGNFileName[i],ExpandedName,MAX_PATH);  
			OpenDGNBounds[i] = *pBounds;
			return;
		} 
	}
	CloseAllRequestedDGNFiles ();
	goto Top;
	return;
}

HANDLE GetOpenDGNFile (LPSTR Name,DWORD Update,LPMNMXCORD pBounds)
{   
	short	i;
	char	ExpandedName[MAX_PATH], TempName[MAX_PATH]; 
	
//    SetWindowText (hWndMain,"GetOpenDGNFile");
    if (!KeepFilesOpen)  
    	return 0;
	if (Update)
		return 0;
	if (!ghDGNlib7)
		return 0;	
	_fstrcpy (ExpandedName,Name);
	ExpandText (ExpandedName);
	_fullpath (TempName,ExpandedName,MAX_PATH);
	for (i=0;i<MAX_OPEN_DGNFILES;i++)
	{
		if (OpenDGNHandles[i])
		{
			if (!_fstricmp (TempName,OpenDGNFileName[i])) 
			{
				CloseDGNRequested[i] = FALSE;
				*pBounds = OpenDGNBounds[i];
				
				return OpenDGNHandles[i];   
			}
		}
	}
	return 0;
}

HANDLE DGN7Open (LPSTR Name,DWORD Update,LPMNMXCORD pBounds)
{
	HANDLE	hDGN;
	
	LoadDGNlib7 (FALSE);
   	hDGN = DGNOpen (Name,Update);
	if (hDGN && pBounds)
	{   
		double	Extents[6]; 
		 
		if (!DGN7GetExtents (hDGN,Extents))
		{
			DGN7Close (hDGN); 
			hDGN = 0;
		} 
		else
		{
			pBounds->xmn = Extents[0];
			pBounds->ymn = Extents[1];
			pBounds->xmx = Extents[3];
			pBounds->ymx = Extents[4];
			AddToOpenDGNFiles (Name,hDGN,pBounds);   
		}
	}
	return hDGN;
} 

DWORD DGN7Close (HANDLE hDGN)
{   
	if (KeepFilesOpen) 
	{
		short	i;
		
		for (i=0;i<MAX_OPEN_DGNFILES;i++)
		{
			if (OpenDGNHandles[i] == hDGN)  
			{
				CloseDGNRequested[i]=TRUE;       
				return TRUE;
			}
		} 
	}
	return DGN7CloseActual (hDGN);
} 

DWORD DGN7Rewind (HANDLE hDGN)
{   
	DWORD	st=TRUE;

	DGNRewind (hDGN);
	
	return st; 
} 

void CloseAllRequestedDGNFiles (void)
{   
	short	i;
	
	for (i=0;i<MAX_OPEN_DGNFILES;i++)
	{
		if (OpenDGNHandles[i])
			DGN7CloseActual (OpenDGNHandles[i]);
		OpenDGNHandles[i] =0;
		CloseDGNRequested[i]=FALSE;
	}
	return;
}


DWORD DGN7GetExtents (HANDLE hDGN,LPDOUBLE Extents)
{
	DWORD	st;
	
   	st = DGNGetExtents (hDGN,Extents);
	
	return st;
} 

DWORD DGNLibReadElement (HANDLE hDGN,LPDGNElementCore pElement,DWORD MaxDGNElementSize,LPDOUBLE pBaseDistToWinDist,
								  LPLONG pFillColor,LPLONG pNumAttributes,LPLONG Attributes)
{
	DWORD	st;
	short	ii; 
	double	DGNPixelSize=0;   
	static	long	debugid=3485;
	
	if (pElement->element_id == debugid)
		ii=1;
//    SetWindowText (hWndMain,"DGN7ReadElement");
	if (pBaseDistToWinDist && *pBaseDistToWinDist > 0)
		DGNPixelSize = 2.0/(*pBaseDistToWinDist);
   	st =  DGN7ReadElement(hDGN,(DWORD)pElement,MaxDGNElementSize,(DWORD)&DGNPixelSize,
   											  (DWORD)pFillColor,(DWORD)pNumAttributes,(DWORD)Attributes);
	if (*pFillColor > -1)
		ii=1;
	if (*pNumAttributes)
		ii=1; 
	return st;
} 

DWORD DGNLibSetSpatialFilter (HANDLE hDGN,LPMNMXCORD pBounds)
{
	DWORD	st; 
	MNMXCORD	NullBounds={0,0,0,0};
	
	if (!pBounds)
		pBounds = &NullBounds;
	
   	st = DGN7SetSpatialFilter(hDGN,pBounds);
	
	return st;
} 

DWORD DGN7GotoElement (HANDLE hDGN,DWORD Recno)
{
	DWORD	st;
	
   	st = DGNGotoElement(hDGN,(DWORD)Recno);
	
	return st;
} 

DWORD DGNLibGetElementExtents (HANDLE hDGN,LPMNMXCORD pBounds)
{
	DWORD	st;
	
   	st = DGN7GetElementExtents(hDGN,(DWORD)pBounds);
	
	return st;
} 

DWORD DGNGetNumElements (HANDLE hDGN)
{
	DWORD	st;
	
  	st = DGN7GetNumElements(hDGN);
	
	return st;
} 

BOOL DGN7GetShapeFillInfo(HANDLE hDGN, DGNElemCore *psElem, LPLONG pnColor )
{
	DWORD	st;
	
  	st = DGNGetShapeFillInfo(hDGN,psElem,pnColor);
	
	return st;
} 

LPSTR DGN7GetLinkage( HANDLE hDGN, DGNElemCore *psElement,long iIndex, LPLONG pnLinkageType,
                             LPLONG pnEntityNum, LPLONG pnMSLink, LPLONG pnLength )
{
	LPSTR	st;
	
   	st =  DGNGetLinkage(hDGN,psElement, iIndex,pnLinkageType,
   										   pnEntityNum,pnMSLink,pnLength);
	
	return st;
} 

HANDLE  BMPFromEXT (LPSTR ImageFile) 
{
	DWORD	st;

   	{   
   		DWORD	Size=16000000L;
		HANDLE	hBMP = GSSiGlobAlloc (0,GMEM_MOVEABLE,Size); 
		HPSTR	ToMem = (HPSTR)GlobalLock (hBMP);
		
   		st = GMFIBMPFromEXT (ImageFile,ToMem,&Size);
   		GlobalUnlock (hBMP); 
   		if (st)
   		{   
   			hBMP = GSSiGlobalReAlloc (0,hBMP,Size,GMEM_MOVEABLE);
   			return (hBMP); 
   		}
   		else
   			GSSiGlobFree (&hBMP);
   	}
	
	return 0;
	
}

BOOL GetFreeImageVersionAndCopyright (LPSTR Version, LPSTR Copyright)
{
		
	GMFIGetVersionAndCopyright (Version,Copyright);
	return TRUE;
}	


BOOL GetGeoTiffData (HDIB32 hBMP,LPDOUBLE pScaleX,LPDOUBLE pScaleY,LPDPOINT pBitmapPoint, LPDPOINT pWorldPoint,BOOL showTags) 
{
	DWORD	st=FALSE;  
	DWORD	ShowTag;

	if (!showTags)
		ShowTag = GetGlobalBVal2("[%GEOTIFSHOWTAGS]", FALSE);
	else
		ShowTag = TRUE;
	st = GMFIGetGeoTiffData (hBMP,ShowTag,(DWORD) pScaleX,(DWORD) pScaleY,(DWORD) pBitmapPoint, (DWORD) pWorldPoint);
	return st;
	
}

BOOL SetGeoTiffData (HDIB32 hBMP,LPDOUBLE pScaleX,LPDOUBLE pScaleY,LPDPOINT pBitmapPoint, LPDPOINT pWorldPoint) 
{
	DWORD	st=FALSE;  

	
	st = GMFISetGeoTiffData ((DWORD)hBMP,(DWORD) pScaleX,(DWORD) pScaleY,(DWORD) pBitmapPoint, (DWORD) pWorldPoint);
	return st;
	
} 

HDIB32 GMRotateImageClassic (HDIB32 hDib,double DegreesRotation)
{

   	HDIB32 st = GMFreeImageRotateClassic (hDib,DegreesRotation);
	
	return (HDIB32)st;
} 

HDIB32  BMPHandleFromEXT (LPSTR ImageFile) 
{
	HDIB32	st;
    char	TempName[MAX_PATH]="";
    
   	if (!strnicmp(ImageFile, "http:", 5) || !strnicmp(ImageFile, "https:", 6))
   	{   
		 GSSiGetTempFileName (0,"gmi",0,TempName);  
		 URLToFile (ImageFile,TempName);
		 ImageFile = TempName;    
   	} 
	st = GMFIBMPHandleFromEXT (ImageFile, FALSE);
	if (*TempName)
		GSSiRemove32 (TempName); 
	return (HDIB32)st;
}	

HDIB32  CopyBMP32 (HDIB32 hBitmap,DWORD left,DWORD right, DWORD top, DWORD bottom) 
{
	DWORD	st;

	st = GMFICopy ((DWORD)hBitmap,left,right,top,bottom);
	return (HDIB32)st;
}	

DWORD  BMPFileFromEXT (LPSTR ImageFile,LPSTR BMPFile,double factor) 
{
	DWORD	st;

   		st = GMFIBMPFileFromEXT (ImageFile,BMPFile,factor);
   		return st;
	
}

BOOL  BMPToEXT (HANDLE hBMP,LPSTR ImageFile,DWORD Flag) 
{
	BOOL	st=0;

   	{   
   		DWORD	Size=GlobalSize(hBMP);
		HPSTR	FromMem = (HPSTR)GlobalLock (hBMP);
		
   		st = GMFIBMPToEXT (ImageFile,FromMem,&Size,Flag);
   		GlobalUnlock (hBMP); 
   	}
	
	return st;
	
}	

BOOL  BMPToEXT32 (HDIB32 hDib,LPSTR ImageFile,DWORD Flag) 
{
	BOOL	st=0;
		
	st = GMFIBMPHandleToEXT (ImageFile,hDib,Flag);
	
	return st;
	
}	

BOOL SetDIBMonoColors(HANDLE hDib,LPLONG Colors) 
{
	BOOL	st=0;
		
	st = GMSetDIBMonoColors (hDib,Colors);
	
	return st;
	
}

BOOL GSSiGetGMCName (HWND hWnd,LPSTR PathName,LPSTR InitialDir,LPSTR Title,LPBOOL pStartInNewSession,LPBOOL pRetainZoom,LPBOOL pLinkZoom,LPSTR NetworkDir,LPSTR PersonalDir)
{
	BOOL	st;
	char	Ext[]=".GMC";

    //sprintf (gszFilter,"%s(*%s)|*%s|","",_fstrupr(Ext),_fstrlwr(Ext)); 
	SetFilterString (IDS_FILTERGMC);
   	st = GM32GetGMCName (hWnd,PathName,InitialDir,Title,pStartInNewSession,pRetainZoom,pLinkZoom,NetworkDir,PersonalDir);
	
	return st;
}

	

BOOL GSSiPrintDlg (LPPRINTDLG p,LPBOOL IsVirtualPrinter,LPHANDLE phVirtPrinter,HDC *PrinterDC)
{ 
	BOOL	rtn;
	short	n,xPage;
	char	PrinterName[256]="";
	LPDEVMODE	pDevMode; 
extern BOOL	PrinterIsVirtual; 
extern BOOL CreatePrintPreview;  
extern BOOL	InPrintSetup;
extern short	NumPrintCopies;
extern char	CustomHeight[16],CustomWidth[16];

	InPrintDriver = TRUE;
	InPrintSetup=FALSE;
	if (PrinterDC)
		*PrinterDC = 0;
	else
		InPrintSetup=TRUE;
   	{
		HDC	hDC=GetDC (hWndMain); 
		HBITMAP	hBitMap;

		if (p->hDevMode)
		{ 	
			IgnoreLock = TRUE;
			pDevMode = (LPDEVMODE)GlobalLock (p->hDevMode);
			strcpy (PrinterName,pDevMode->dmDeviceName);
			if (*CustomWidth)
			{
				pDevMode->dmPaperLength = atof (CustomHeight)/12 * FTM * 1000;
				pDevMode->dmPaperWidth  = atof (CustomWidth)/12 * FTM * 1000; 
				pDevMode->dmPaperSize = 0;
			} 
			GlobalUnlock (p->hDevMode); 
			IgnoreLock = FALSE;
		}
//		ClearFullWindowBitmap(hWndMain);
		SaveFullWindowBitmap (hWndMain);
		rtn = PrintDlg (p); 
		RestoreFullWindowBitmap ();
//		if (NumPrintCopies)
			p->nCopies = NumPrintCopies;
		if (rtn && PrinterIsVirtual)
		{   
				if (IsVirtualPrinter)
				{
					LPSTR	pVPIF;
			 
        	 		pVPIF = VirtPrinterImageFile;
					NumVirtualRows = 1; 
					VirtualPagesPerRow = 1;
					if (!VirtualOrientation)
					{
						int isave = VirtualPlotWidth;
						VirtualPlotWidth = VirtualPlotHeight;
						VirtualPlotHeight = isave;
					}
					while (VirtualPlotWidth / VirtualPagesPerRow > MAXVPTILEWIDTH)
		        		VirtualPagesPerRow++;
					while (VirtualPlotHeight / NumVirtualRows > MAXVPTILEHEIGHT)
		        		NumVirtualRows++;
					VirtualPageWidth = VirtualPlotWidth / VirtualPagesPerRow;
					VirtualPageHeight = VirtualPlotHeight / NumVirtualRows;   
		//			VirtualPrintDPI = 300; 
					NumVirtualPages = VirtualPagesPerRow * NumVirtualRows; 
					DeleteDC (p->hDC);
					p->hDC = CreateCompatibleDC (hDC);
					curProgID = 10005;
					hBitmap = CreateCompatibleBitmap (hDC,VirtualPageWidth,VirtualPageHeight);
					curProgID = -1;
					hbmpVirtPrinterOld = SelectObject(p->hDC, hBitmap);
					*IsVirtualPrinter = TRUE;
					IgnoreLock = TRUE;
					GSSiGlobFree(&p->hDevMode);
					GSSiGlobFree(&p->hDevNames);
					IgnoreLock = FALSE;

				} 
		}
		else if (rtn)
		{   
			int	Planes = GetDeviceCaps(p->hDC, PLANES);
			int	Colors = GetDeviceCaps(p->hDC,NUMCOLORS);

			*VirtPrinterImageFile = 0;
			 if (p->hDevMode)
			 { 	
		    	IgnoreLock = TRUE;
				pDevMode = (LPDEVMODE)GlobalLock (p->hDevMode);
				GlobalUnlock (p->hDevMode); 
		    	IgnoreLock = FALSE;
		    	xPage = GetDeviceCaps(p->hDC, HORZRES);
			 }
			 if (PrinterDC && !wantGDIPlus && (App == 1 || strstr (PrinterName,"PDF") || GetGlobalBVal2 ("[%PRINTTOBITMAP]",FALSE)))
			 {
				int	PageWidth, PageHeight;
				HDC	hDCMain = GetDC (hWndMain);
				BITMAP	bm;

		    	PageWidth = GetDeviceCaps(p->hDC, HORZRES);
				PageHeight = GetDeviceCaps(p->hDC, VERTRES);
/*				{
					char mes[128];
					sprintf(mes, "%i %i", PageWidth, PageHeight);
					MessageBox(0, mes, 0, MB_OK);
				}*/
				curProgID = 10006;
				hBitmap = CreateCompatibleBitmap(hDCMain, PageWidth, PageHeight);
				curProgID = -1;
				if (hBitmap)
				{
					*PrinterDC = p->hDC;
					p->hDC = CreateCompatibleDC(hDCMain);
					GetObject(hBitmap, sizeof(bm), (LPSTR)&bm);
					hbmpVirtPrinterOld = SelectObject(p->hDC, hBitmap); 
				}
				ReleaseDC(hWndMain, hDCMain);
			 }
			//if (*CustomHeight)
			{    
				 int	i;
				 //for (i=0;i<256;i++)
/*				 if (p->hDevMode)
				 { 	
				 	BOOL	SaveAPOK = AutoPrintOK;
				 	
			    	IgnoreLock = TRUE;
					pDevMode = (LPDEVMODE)GlobalLock (p->hDevMode);
					pDevMode->dmPaperLength = 0;//atol (CustomHeight);
					pDevMode->dmPaperWidth = 0;//atol (CustomWidth); 
					i = pDevMode->dmPaperSize;
					pDevMode->dmPaperSize = i;
					GlobalUnlock (p->hDevMode); 
			    	IgnoreLock = FALSE;  
			    	AutoPrintOK = TRUE;
			    	PrintDlg (p); 
			    	AutoPrintOK = SaveAPOK;
			    	xPage = GetDeviceCaps(p->hDC, HORZRES);
			    	xPage = GetDeviceCaps(p->hDC, VERTRES);
				 }*/
			}
		    if (CreatePrintPreview)
		    {
				RECT	Rect;
				HDC		mfDC;

				VirtualPrintDPI = GSSiGetDeviceCaps(p->hDC, LOGPIXELSX);; 
		        if (*CustomWidth)
		        {
		       		VirtualPlotWidth = atof (CustomWidth) * VirtualPrintDPI;
		       		VirtualPlotHeight = atof (CustomHeight) * VirtualPrintDPI;
		        }
		        else
		        { 
				    VirtualPlotWidth = GSSiGetDeviceCaps(p->hDC, HORZRES);
			        VirtualPlotHeight = GSSiGetDeviceCaps(p->hDC, VERTRES);
			    }  
		        NumVirtualRows = 1; 
		        VirtualPagesPerRow = 1;
		        while (VirtualPlotWidth / VirtualPagesPerRow > MAXVPTILEWIDTH)
		        	VirtualPagesPerRow++;
		        while (VirtualPlotHeight / NumVirtualRows > MAXVPTILEHEIGHT)
		        	NumVirtualRows++;

	VirtualPagesPerRow = NumVirtualRows = 1;
				VirtualPageWidth = VirtualPlotWidth / VirtualPagesPerRow;
				VirtualPageHeight = VirtualPlotHeight / NumVirtualRows;   
				NumVirtualPages = VirtualPagesPerRow * NumVirtualRows; 
				Rect.left = Rect.top = 0;
				Rect.right = VirtualPlotWidth;
				Rect.bottom = VirtualPlotHeight;
				//p->hDC = CreateCompatibleDC (hDC);
				//hBitmap = CreateCompatibleBitmap (hDC,VirtualPageWidth,VirtualPageHeight);
				//hbmpVirtPrinterOld = SelectObject(p->hDC, hBitmap); 
				*IsVirtualPrinter = TRUE;
			}
	    }   
		ReleaseDC (hWndMain,hDC);    
		InPrintDriver = FALSE;
		return rtn;
   	}
} 

BOOL CreatePrintBitmap(HWND hWnd)
{
	return FALSE;
		int	PageWidth=5100, PageHeight=33000;
		HDC	hDCMain = GetDC(0);
		BITMAP	bm;
		BOOL rtn = FALSE;

		{
			char mes[128];
			sprintf(mes, "%i %i %i %i", PageWidth, PageHeight, MaxMemAlloc, TotMemAlloc);
			MessageBox(0, mes, 0, MB_OK);
		}
		curProgID = 10007;
		hBitmap = CreateCompatibleBitmap(hDCMain, PageWidth, PageHeight);
		curProgID = -1;
		if (hBitmap)
		{
			rtn = TRUE;
		}
		ReleaseDC(0, hDCMain);
		return rtn;
}
int Plot3PointCurve (HDC hDC,LPDPOINT pPC,LPDPOINT pPOC,LPDPOINT pPT)
{
	MNMXCORD MinMaxD;  
	DPOINT	 RP;
	double	 CLEN, RAD;
	int	rtn=0;
			
	if (RCURVE(&pPC->x,&pPC->y,&pPOC->x,&pPOC->y,&pPT->x,&pPT->y,&RP.x,&RP.y,&CLEN))
	{
		POINT	Points[3];

		Points[0] = DPointToPoint (*pPC);
		Points[1] = DPointToPoint (*pPOC);
		Points[2] = DPointToPoint (*pPT);
		rtn = Polyline (hDC,Points,3);
	}
	else
	{
		RAD = ldistp (RP,*pPC);
		MinMaxD.xmn = RP.x - RAD;
		MinMaxD.xmx = RP.x + RAD;
		MinMaxD.ymn = RP.y + RAD;
		MinMaxD.ymx = RP.y - RAD;
		if (CLEN > 0)
			SetArcDirection(hDC,AD_COUNTERCLOCKWISE);
		else
			SetArcDirection(hDC,AD_CLOCKWISE);

		rtn = Arc(hDC,IDNINT(MinMaxD.xmn),IDNINT(MinMaxD.ymn),IDNINT(MinMaxD.xmx),IDNINT(MinMaxD.ymx),
				  IDNINT(pPC->x),IDNINT(pPC->y),IDNINT(pPT->x),IDNINT(pPT->y));
	}
	return rtn;
}
 

DWORD DrawLineWithFlatEndF(HDC hDC, DWORD npt, HPFPOINT FPoints, DWORD Width, COLORREF Color)
{
	long rtn;
	HANDLE hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, npt * sizeof(POINT)+4);
	LPPOINT Points = GlobalLock(hPoints);

	for (int i = 0; i < npt; i++)
		Points[i] = FPointToPoint(FPoints[i]);
	rtn = DrawLineWithFlatEnd(hDC, npt, Points, Width, Color);
	GSSiGlobUlFree(&hPoints);
	return rtn;
}
DWORD DrawLineWithFlatEnd(HDC hDC, DWORD npt, HPPOINT Points, DWORD Width, COLORREF Color)
{
	HPEN	hPen,OldPen;
	LOGBRUSH	lb;
	long	rtn; 
	
	lb.lbStyle = BS_SOLID;
	lb.lbColor = Color;
	lb.lbHatch = 0;
	//Width = 1;
	hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,Width,&lb,0,0);
	OldPen = SelectObject (hDC,hPen);
//	if (npt < 3)
		rtn = Polyline (hDC,Points,npt);
/*	else
	{
		DPOINT PC,POC,PT;

		PC = PointToDPoint (Points[0]);
		PT = PointToDPoint (Points[npt-1]);
		POC = PointToDPoint (Points[max(2,npt/2)]);
		rtn = Plot3PointCurve (hDC,&PC,&POC,&PT);
	}*/
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);
   	return rtn;
}  


long GSSiGetDeviceCaps(HDC hDC, int opt)
{ 
	return GetDeviceCaps(hDC,opt);
}  

/*DWORD GM32DrawDibOpen (void)
{
	MYPROC Proc; 
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32DrawDibOpen" );
   	if(Proc == NULL )
   		return FALSE;
   	
   	return (lpfnCallProcEx32W(0,0,(DWORD)Proc));
} 
DWORD GM32DrawDibClose (DWORD hdd)
{
	MYPROC Proc; 
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32DrawDibClose" );
   	if(Proc == NULL )
   		return FALSE;
   	
   	return (lpfnCallProcEx32W(1,BitMask("0"),(DWORD)Proc,hdd));
} */
//   GM32DrawDibBegin
//   GM32DrawDibEnd
//   GM32DrawDibDraw

HDC LargeMemDC (HDC hDC16,DWORD * MemMapWidth,DWORD * MemMapHeight,DWORD *pOldBitmap) 
{
	HDC	rtn;
	 
   	rtn = GM32LargeMemDC (hDC16, MemMapWidth,MemMapHeight,pOldBitmap); 
   	return rtn;
}

BOOL DeleteLargeDC (HDC hDC16,DWORD hOldBitmap)  
{
	BOOL	rtn;
   	
   	rtn = GM32DeleteLargeMemDC (hDC16, hOldBitmap);
   	return rtn;
}  

BOOL SaveDCBitMap (HDC hDC16,LPSTR OutFile,long Format,DWORD Flag,COLORREF BackgroundColor,
						   	 HBITMAP hOverViewBitmap,
						   	 long VirtualPageRow,long VirtualPageCol,
							 long OverViewTileWidth, long OverViewTileHeight,long OverViewBitmapHeight)  
{
	BOOL	rtn;
	 
   	
   	rtn = GM32SaveDCBitMap (hDC16, OutFile,Format,Flag,BackgroundColor,
													    hOverViewBitmap,
													    VirtualPageCol,  VirtualPageRow,
													    OverViewTileWidth,  OverViewTileHeight,
													    OverViewBitmapHeight);
   	return rtn;
}  

HDIB32 BitmapToDIB32 (HBITMAP hBitmap)  
{
	MYPROC	Proc;
	DWORD	rtn;
	 
   	rtn = GM32BitmapToDIB ((DWORD)hBitmap); 
   	return (HDIB32)rtn;
} 

BOOL GetDibPalette (HDIB32 hDib,LPLONG pNumColors,LPRGBQUAD pPalette)
{
   	BOOL	rtn = GM32GetDIBPalette (hDib,pNumColors,pPalette); 
   	return (BOOL)rtn;
} 

BOOL SetDibPalette (HDIB32 hDib,DWORD NumColors,LPRGBQUAD pPalette)
{
	DWORD	rtn;
	 
   	
   	rtn = GM32SetDIBPalette (hDib,NumColors,pPalette); 
   	return (BOOL)rtn;
}

HDIB32 QuantizeDib (HDIB32 hDib,DWORD Opt)
{
	DWORD	rtn;
	 
   	
   	rtn = GM32QuantizeDIB (hDib,Opt); 
   	return (HDIB32)rtn;
} 

HDIB32 QuantizeDibEx (HDIB32 hDib,DWORD Opt,DWORD PalSize,DWORD ResPalSize,LPRGBQUAD Pallet)
{
	DWORD	rtn;
	 
   	rtn = GM32QuantizeDIBEx (hDib,Opt,PalSize,ResPalSize,Pallet); 
   	return (HDIB32)rtn;
} 

BOOL SaveBitmap (HBITMAP hBitmap,LPSTR OutFile,long Format,DWORD Flag)  
{
	BOOL	rtn;
	char	file[MAX_PATH];

	strcpy (file,OutFile);
	ExpandText (file);
	makedirectories (file,FALSE,FALSE);
   	
   	rtn = GM32SaveBitmap (hBitmap, OutFile,Format,Flag); 
   	return rtn;
}

BOOL SaveDIB32 (HDIB32 hBitmap,LPSTR OutFile,long Format,DWORD Flag)  
{
	BOOL	rtn;
	 
   	makedirectories (OutFile,FALSE,FALSE);
   	rtn = GM32SaveDIB (hBitmap, OutFile,Format,Flag); 
   	return rtn;
} 

HDIB32 AllocateDIB (DWORD Width,DWORD Height,DWORD BitsPerPixel)
{
	HDIB32	rtn;
	 
   	
   	rtn = GM32AllocateDIB (Width,Height,BitsPerPixel); 
   	return rtn;
}

BOOL PasteDIB (HDIB32 ToDIB,HDIB32 FromDIB,DWORD Left,DWORD Top,DWORD Alpha)
{
	    	
   	BOOL rtn = GM32PasteDIB (ToDIB,FromDIB,Left,Top,Alpha); 
   	return rtn;
}

 

/*long StretchDIBits32 (HDC hDC,long destX,long destY,long destW,long destH,long xoff,long yoff,long bmwidth,long bmheight,
					  LPBYTE pImage, LPBITMAPINFOHEADER pDibInfo,long ColorType,long RastOpts,LPDOUBLE pFactor)
{
	MYPROC	Proc;
	long	rtn;
	 
	if (!ghLib)
		return 0;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32StretchDIBits" );
   	if(Proc == NULL )
		return 0;
   	
   	rtn = lpfnCallProcEx32W(14,BitMask("00000000011001"),(DWORD)Proc,
   													 (DWORD)hDC, (DWORD)destX, (DWORD)destY, (DWORD)destW, (DWORD)destH, 
   													 (DWORD)xoff, (DWORD)yoff,(DWORD) bmwidth,(DWORD) bmheight,
   													 (DWORD)pDibInfo, (DWORD)pImage,(DWORD)ColorType,(DWORD) RastOpts,(DWORD)pFactor); 
   	return rtn;
}*/

/*BOOL GMDestroyDIB32 (HDIB32 hDib)
{ 
	MYPROC	Proc;
	BOOL	rtn; 
	
	if (!ghLib ||!hDib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GMFIBMPUnload" );
   	if(Proc == NULL )
		return FALSE;
   	
   	rtn = lpfnCallProcEx32W(1,BitMask("0"),(DWORD)Proc, (DWORD)hDib);
	return rtn;
}*/

long StretchDIBitsFromHandle (HDC hDC,long destX,long destY,long destW,long destH,long xoff,long yoff,long sourcew,long sourceh,
					 		  HDIB32 hDib,DWORD ColorType,DWORD RasterOpt, double Factor)
{
	long	rtn;
	LPDOUBLE	pFactor=&Factor;   
	BOOL	debug=FALSE;
	 
   	rtn = GM32StretchDIBitsFromHandle(
   													 hDC, destX, destY, destW, destH, 
   													 xoff, yoff, 
   													 sourcew,sourceh,
   													 hDib,ColorType,RasterOpt,pFactor); 
   	return rtn;
}


BOOL GMNADCON (LPDPOINT Point,DWORD Dir)
{
	BOOL	st;   
	DPOINT	OutPoint;
	
   	
   	st = GM32NADCON (Point,&OutPoint,Dir);  
   	if (st)
   		*Point = OutPoint;
	return st;
}

BOOL CompressFrame (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
	short	ln;
	   	
   	ln = GM32CompressFrame (lpbiIn, lpbiOut);
	return ln;
}

/*DWORD OpenGarminUSB (LPSTR ProductID)
{
	MYPROC Proc;
	DWORD	rtn; 
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32OpenGarminUSB" );
   	if(Proc == NULL )
   		return FALSE;
   	
   	rtn = lpfnCallProcEx32W(1,BitMask("1"),(DWORD)Proc, (DWORD)ProductID);
   	return rtn;
} 

DWORD CloseGarminUSB (void)
{
	MYPROC Proc; 
	char	NullArg[2];
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GM32CloseGarminUSB" );
   	if(Proc == NULL )
   		return FALSE;
   	
   	return (lpfnCallProcEx32W(1,BitMask("1"),(DWORD)Proc,(DWORD)NullArg));
} */

DWORD GetPacketUSB (LPBYTE Packet)
{
   	
   	return (GM32GetPacketUSB(Packet));
} 

DWORD SendPacketUSB (LPBYTE Packet)
{
   	
   	return (GM32SendPacketUSB (Packet));
}  

DWORD MrSidVersion (LPSTR Version)
{
   	
   	return (GMMrSidVersion (Version));
} 

HDIB32 MrSidOpen (LPSTR InFile)
{
	char	File[256];
	
	_fstrcpy (File,InFile);
	ExpandText (File);
   	
   	return (GMMrSidOpen (File));
} 

DWORD MrSidClose (HDIB32 ImageHandle)
{
   	return (GMMrSidClose (ImageHandle));
}  

DWORD MrSidGetInfo (HDIB32 ImageHandle,
					LPDWORD pWidth,
					LPDWORD pHeight,
					LPDWORD	pColorSpace,
					LPDWORD	pNumBands,
					LPDWORD	pDataType,
					LPDOUBLE pMinMag,
					LPDOUBLE pMaxMag,
					LPDWORD	pIsLocked,
					LPDOUBLE pULX,
					LPDOUBLE pULY,
					LPDOUBLE pXres,
					LPDOUBLE pYres,
					LPDOUBLE pXRot,
					LPDOUBLE pYRot,
					LPDWORD pNumMetaRecords)
{
	MYPROC Proc; 
	DWORD	ColorSpace, NumBands, DataType, IsLocked, st;
   	st = GMMrSidGetImageInfo (
											ImageHandle,
						   					(DWORD) pWidth,
						   					(DWORD) pHeight,
						   					(DWORD) &ColorSpace,
						   					(DWORD) &NumBands,
						   					(DWORD) &DataType,
						   					(DWORD) pMinMag,
						   					(DWORD) pMaxMag,
						   					(DWORD) &IsLocked,
						   					(DWORD) pULX,
						   					(DWORD) pULY,
						   					(DWORD) pXres,
						  					(DWORD) pYres,
						   					(DWORD) pXRot,
						   					(DWORD) pYRot,
						   					(DWORD) pNumMetaRecords);
	*pColorSpace = ColorSpace;
	*pNumBands = NumBands;
	*pDataType = DataType;
	*pIsLocked = IsLocked;
	return st;
} 

DWORD MrSidGetMetadataRecord(HDIB32 ImageHandle,
                               DWORD recordNum,
                               LPSTR ptag,
                               LPSHORT pdatatype,
                               LPSHORT pnumDims,
                               LPDWORD *ppdims,
                               LPVOID *ppdata)
{
	DWORD	st;  
	
   	st = GMMrSidGetMetadataRecord(ImageHandle,recordNum,
											(DWORD) ptag,
											(DWORD) pdatatype,
											(DWORD) pnumDims,  
											(DWORD) ppdims,
											(DWORD) ppdata);
    return st;
}

HDIB32 MrSidGetImage (HDIB32 ImageHandle,
					LPDOUBLE pULX,
					LPDOUBLE pULY,
					LPDWORD pWidth,
					LPDWORD pHeight,
					LPDOUBLE pMag,
					DWORD	ConvertToGray,
					DWORD		Intensity)
{
	DWORD	ColorSpace, NumBands, DataType, IsLocked, DisplayErrorMessage; 
	HDIB32 hDib;
	
   	DisplayErrorMessage = GetGlobalBVal2 ("[%DISPLAYSIDERRORS]",FALSE);
   	hDib  = GMMrSidGetImage (ImageHandle,
											pULX,
											pULY,
											 pWidth,
											 pHeight,
											 pMag,
											 ConvertToGray,Intensity,
											DisplayErrorMessage);
	return hDib;
}

DWORD ConvertGeodeticToMGRS (LPDOUBLE pLat,LPDOUBLE pLon,DWORD Precision,LPSTR MGRS)
{

	DWORD	st;
	double	lat=*pLat*RADDEG;
	double	lon=*pLon*RADDEG;
	
   	st = GMConvert_Geodetic_To_MGRS (&lat,
											 &lon,
											 Precision,
											  MGRS);
	return st;
}

DWORD ConvertMGRSToGeodetic (LPDOUBLE pLat,LPDOUBLE pLon,LPSTR MGRS)
{

	DWORD	st; 
	double	lat,lon;
	
   	st = Convert_MGRS_To_Geodetic ( MGRS,
											 &lat,
											 &lon);
	if (!st)
	{
		*pLat = lat * DEGRAD;
		*pLon = lon * DEGRAD;
	}
	return st;
}



BOOL LoadBlatLib (void)
{
	char	BlatLib[128]="[%DL]blat.dll"; 
//	return FALSE;
	if (ghBlatLib)
		return TRUE;
	ExpandText (BlatLib);
	if (!(ghBlatLib = LoadLibrary( BlatLib))) 
		return FALSE; 
	return TRUE;
}

//DWORD SendEMail (LPSTR Server,LPSTR From,LPSTR To,LPSTR Subject,LPSTR Body)
DWORD SendEMail (LPSTR cmd2)
{
	FARPROC	blat_send;  
	DWORD	rc=0;
	char	cmd[1024];
	
	if (!LoadBlatLib())
		return FALSE;

	blat_send = GetProcAddress( ghBlatLib, (LPCSTR)"Send" );
   	if(blat_send == NULL )
   		return FALSE;
   	
//	sprintf (cmd,"-s \"%i\" -server \"%s\" -f \"%i\" -t \"%i\"",Subject,Server,From,To);meberle@ci.maple-grove.mn.us
//strcpy (cmd,"-server \"mg-mail.ci.maple-grove.mn.us\" "
strcpy (cmd,"-server \"smtp.frontiernet.net\" "\
"-s \"CrimeMaster server re-started\" " \
"-f jeffgis@aol.com " \
"-t jeffgis@aol.com " \
"-log \"C:\\smtp.log\"");
	rc = blat_send(cmd);

//   	Proc((DWORD)BlatCommandLine);
   	return !rc;
}  

/*	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GMSendEMail" );
   	if(Proc == NULL )
   		return FALSE;
   	
   	return (lpfnCallProcEx32W(1,BitMask("1"),(DWORD)Proc,(DWORD)BlatCommandLine));
} */ 

BOOL URLToFile (LPSTR URL,LPSTR File)
{
   	BOOL	rc = GMURLDownloadToFile (URL,File);
   	return rc;
} 

LPSTR requestFromURL(LPSTR url)
{
	char tempFile[MAX_PATH];
	LPSTR	pFile = 0;
	BOOL	doRemove = FALSE;
	BOOL	readFromExistingFile = TRUE;

	GetGlobalCVal("[%URLFILE]", tempFile, 0);
	if (!*tempFile)
	{
		doRemove = TRUE;
		readFromExistingFile = FALSE;
		GSSiGetTempFileName(0, "gmt", 0, tempFile);
	}
	if (readFromExistingFile || URLToFile(url, tempFile))
	{
		int	lFile = GSSiLength(tempFile);

		if (lFile > 0)
		{
			OFSTRUCTGM OFStruct = { 0 };
			HANDLE	Fid = OpenFileGM(tempFile, &OFStruct, OF_READ);

			if (Fid != INVALID_HANDLE_VALUE)
			{
				pFile = malloc(lFile + 1);
				BigRead64(Fid, pFile, lFile);
				pFile[lFile] = 0;
				GSSiClose64(&Fid);
			}
		}
	}
	if (doRemove)
		remove(tempFile);
	return pFile;
}

/*BOOL GetLastFileWriteTime (LPSTR File,LPDWORD pLowTime,LPDWORD pHighTime,LPDWORD pTimeDiff)
{
	MYPROC	Proc;  
	DWORD	rc;  
	
	if (!ghLib)
		return FALSE;
	Proc = (MYPROC) lpfnGetProcAddress32W( ghLib, "GMGetLastFileWriteTime" );
   	if(Proc == NULL )
   		return FALSE;
   	rc = lpfnCallProcEx32W(4,BitMask("1111"),(DWORD)Proc,(DWORD)File,(DWORD)pLowTime,(DWORD)pHighTime,(DWORD)pTimeDiff);
   	return rc;
}*/ 


