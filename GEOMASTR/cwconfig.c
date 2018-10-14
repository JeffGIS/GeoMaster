#include "graphint.h"  
#include "umio.h"
#include "extrndb.h"  
#include "dibapi.h"

#include <commctrl.h>
#include <mmsystem.h>
//#include "slsapi.h"

#include <sys\types.h>
#include <sys\stat.h>         

#include "gmextern.h"
#include "TestSuite.h"

#include <process.h>

BOOL InDebug=FALSE;

void _testMemIO(const char *lpszPathName);
BOOL RecoverBadFile (void);

BOOL CreatePrintBitmap(HWND hWnd);
int SetLastMessage(long mes, WPARAM wParam);

int PASCAL WinMainGeoMaster(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);
int PASCAL WinMainGMEdit(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);
int PASCAL WinMainGMDoc(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);
LONG FAR PASCAL WndProcGMEdit(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam);
LONG FAR PASCAL WndProcGMDoc(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam);
LONG FAR PASCAL WndProcGeoMaster(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam);
int ConvertPRJtoProj4(char *in, char * out);
LRESULT CALLBACK GetMsgProc(
  int code,       // hook code
  WPARAM wParam,  // removal flag
  LPARAM lParam   // address of structure with message
);
int FileDlgWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static char selectedStartCmd[1024]; 
static MSG	pmsg[100]={0};
static	int	activeCount=0;
static WINDOWPOS lastWP={0};
static HWND		hWndLinkedTo=0;
static HBITMAP	hBMMove=0;
static RECT		MoveStartRect, MoveEndRect;
static DPOINT	GPSLoc;
static WORD		GPSXid;
static BOOL		HaveWMCreate=FALSE;
static char		BackgroundMacro[MAX_PATH];
static HANDLE	hTrace;
static MNMXCORD	Bounds;
static HANDLE	hActAdd=0;
static HANDLE	hStreetAdd=0;
static HANDLE	hIntersect=0;
static RECT	DeskRect, DeskRectC;
static int	ShowMax=10;
static BOOL	NoMenu=FALSE; 
static BOOL	NoAccel=FALSE;
static LPSTR	Command;
static BOOL	InTime=FALSE;
static BOOL	IsAccel=FALSE;
static BOOL	InPaint=FALSE;
static BOOL	WantDDE=FALSE;
//static DWORD	expnot=851349;
static HACCEL	hAccelTableHLT;
static DLGPROC	lpProcInstance;
static HHOOK	Func;
static HHOOK	FAR*lpFilterFunc=&Func;
static HHOOK	MHookFunc;
static LPSTR	lpFileDesc;
static short	i2;
static short	Pcode;
static short	npicked;
static short	ButtonFuncOpt;
static long	iii=0;
static DLGPROC	lpfnPRINTINGMsgProc;
static BOOL	YearToDate=FALSE;
static long	NumPaint=1;    
static BOOL	SaveHavePaint=FALSE;   
static char AppNames[][12]={"GeoMaster","LakeMaster","SportMap","CrimeMaster"};   
static char	VisDialog[2][10]={"VISIBLE","VISIBLE1"};
static char	title1[]="Load GeoMaster Configuration File";
static char	title2[]="Load GeoMaster Format Configuration File";
static char	title3[]="Load GeoMaster Menu Configuration File";
static char title4[]="Find GeoMaster Graphics Data";
static RECT customRect;
static BOOL setWindowToTopOfZ = FALSE;
static int  mapServerWidth, mapServerHeight;

BOOL	DoReset=FALSE;
static		char		CfgNameIn[MAX_PATH]=""; 
//			char MODULEIDSTRING[6] = "2301", MODULENAME[32]="GeoMaster"; DWORD	App=0; short TrialDays=15; char DefaultArgs[]="BASIC1.GMC";char ApName[]="GeoMaster";
			char MODULEIDSTRING[6] = "2401", MODULENAME[32] = "GeoMaster"; DWORD	App = 0; short TrialDays = 15; char DefaultArgs[] = "BASIC1.GMC"; char ApName[] = "GeoMaster"; BOOL	CheckForLicense = FALSE;

//char MODULEIDSTRING[6] = "2401", MODULENAME[32] = "NewVision GIS"; DWORD	App = 0; short TrialDays = 15; char DefaultArgs[] = "BASIC1.GMC"; char ApName[] = "PCViewer"; BOOL	CheckForLicense = FALSE;
//char MODULEIDSTRING[6] = "2401", MODULENAME[32] = "NewVision GIS"; DWORD	App = 0; short TrialDays = 15; char DefaultArgs[] = "BASIC1.GMC"; char ApName[] = "NewVisionGIS"; BOOL	CheckForLicense = FALSE;
//			char MODULEIDSTRING[6] = "2501", MODULENAME[32]="CrimeMaster"; DWORD	App=3; short TrialDays=15; char DefaultArgs[]="BASIC1.GMC"; char ApName[]="CrimeMaster";BOOL	CheckForLicense=FALSE;
//			char MODULEIDSTRING[6] = "2302", MODULENAME[32]="LakeMaster MN/WI"; DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2601", MODULENAME[32]="SportMap Minnesota";DWORD App=2; short TrialDays=30;char DefaultArgs[]="SPORTMAP.GMC /NOMENU";char ApName[]="SportMap";
//			char MODULEIDSTRING[6] = "2304", MODULENAME[32]="LakeMaster LMich";DWORD	App=1; short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2305", MODULENAME[32]="LakeMaster LErie";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2306", MODULENAME[32]="LakeMaster Mich ";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2307", MODULENAME[32]="LakeMaster ND/SD";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2308", MODULENAME[32]="LakeMaster Lake Ontario";DWORD	App=1;short TrialDays=30; char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2309", MODULENAME[32]="LakeMaster Illinois";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2310", MODULENAME[32]="LakeMaster Iowa";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2311", MODULENAME[32]="LakeMaster Ohio";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2312", MODULENAME[32]="LakeMaster New York";DWORD	App=1;short TrialDays=30; char DefaultArgs[]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";
//			char MODULEIDSTRING[6] = "2313", MODULENAME[32]="LakeMaster Wisc ";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU";char ApName[]="LakeMaster";BOOL	CheckForLicense=TRUE;
//			char MODULEIDSTRING[6] = "2319", MODULENAME[32]="LakeMaster LKOTW";DWORD	App=1;short TrialDays=30;char DefaultArgs[]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";BOOL	CheckForLicense=TRUE;  //lake of the woods
//			char MODULEIDSTRING[6] = "2315", MODULENAME[32]="Contour Pro Minnesota";DWORD	App=1;short TrialDays=30;char DefaultArgs[128]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";BOOL	CheckForLicense=TRUE;   
//			char MODULEIDSTRING[6] = "", MODULENAME[32]="";DWORD	App=1;short TrialDays=30;char DefaultArgs[128]="LKMASTER.GMC /NOMENU"; char ApName[]="LakeMaster";BOOL	CheckForLicense=TRUE;   
//			char MODULEIDSTRING[6] = "2503", MODULENAME[32]="MOXIE Minnesota";DWORD	App=3;short TrialDays=30;char DefaultArgs[]="KENNEDY.GMC /NOMENU";char ApName[]="MOXIE";

HHOOK	PrevFIHook;
static	int nhookcalls=0;
static	UINT	inClientMain=0;

void my_invalid_parameter_handler(
	const wchar_t * expression,
	const wchar_t * function,
	const wchar_t * file,
	unsigned int line,
	uintptr_t pReserved
	)
{
	MessageBox(0, "Invalid parameter", 0, MB_ICONEXCLAMATION);
	_exit(23);
}

DWORD CALLBACK ForegroundIdleProc(
  int code,      // hook code
  DWORD wParam,  // not used
  LONG lParam    // not used
  )
{
	LRESULT st = CallNextHookEx(PrevFIHook,      // handle to current hook
								code,      // hook code passed to hook procedure
								wParam,  // value passed to hook procedure
								lParam   // value passed to hook procedure
								);

	nhookcalls++;
	return st;
}

void SetInClientMain (void)
{
	inClientMain=0;
	return;
}


BOOL DoSaveConfig (HWND hWnd,BOOL AutoSave)
{
	 BOOL rtn=FALSE;
	 RECT WindowRect;
	 BOOL saveHaltPaint = HaltPaint;

	 HaltMapDisplay(FALSE,TRUE);
	 HaltPaint = TRUE;
	 GMDestroyDIB32 (hWindowDib32);
	 hWindowDib32 = 0;
	 GetClientRect (hWndMain,&WindowRect);   
	 ClientRectToScreenRect (hWnd,&WindowRect);
	 hWindowDib32 = CopyScreenToDIB32 (&WindowRect); 
	 SaveFullWindowBitmap(hWndMain);
	 setDoPaint( FALSE);
	 if (AutoSave || GetSaveName2 (hWnd,CfgName,IDS_FILTERGMC,".GMC",IDS_FILEGMC))
	 {    
 		if (GetGlobalBVal2 ("[%CFGSAVEPROMPT]",TRUE))
 		{
		  DLGPROC lpfnCONFIGPARMSMsgProc;

		  lpfnCONFIGPARMSMsgProc = MakeProcInstance((DLGPROC)CONFIGPARMSMsgProc, hInst);
		  rtn = DialogBox(hInst, (LPSTR)"CONFIGPARMS", hWnd, lpfnCONFIGPARMSMsgProc);
		  FreeProcInstance(lpfnCONFIGPARMSMsgProc);
		 }
		 else
		 {
     		SaveZoom = TRUE;
     		SaveGlobals = TRUE;
	 		SaveCfgImage = GetGlobalBVal2 ("[%CFGSAVEIMAGE]",FALSE);
     		rtn=1; 
		 }
		SetConfig(1);
		if (rtn)
     		SaveConfig (CfgName,TRUE);
	 } 
	 GMDestroyDIB32 (hWindowDib32);
	 hWindowDib32 = 0;
	 HaltPaint = saveHaltPaint;
	 return rtn;
}


/************************************************************************/
/*                                                                      */
/* nCwRegisterClasses Function                                          */
/*                                                                      */
/* The following function registers all the classes of all the windows  */
/* associated with this application. The function retrns an error code */
/* if unsuccessful, otherwise it retrns 0.                             */
/*                                                                      */
/************************************************************************/

int nCwRegisterClasses(LPSTR Menu)
#if ENABLETRACE
{GSSiEnterProg (451);
#endif
{    
	HBITMAP hBmp;
 WNDCLASS   wndclass;    /* struct to define a window class             */
 _fmemset(&wndclass, 0x00, sizeof(WNDCLASS));

  /* load WNDCLASS with window's characteristics                         */
 wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS | CS_OWNDC;
 wndclass.lpfnWndProc = (WNDPROC)WndProc;
 /* Extra storage for Class and Window objects                          */
 wndclass.cbClsExtra = 0;
 wndclass.cbWndExtra = 0;
 wndclass.hInstance = hInst;
 wndclass.hIcon = LoadIcon(hInst, "GWIZ");
 wndclass.hCursor = NULL;//LoadCursor(0, IDC_ARROW);
 /* Create brush for erasing background                                 */
 /*hBmp = LoadBitmap (hInst,"BACKGROUND_1");
 hBackBrush1 = CreatePatternBrush (hBmp); 
 DeleteObject (hBmp);*/
 wndclass.hbrBackground = NULL;//hBackBrush1;
 wndclass.lpszMenuName = Menu;   /* Menu Name is App Name */
 wndclass.lpszClassName = szAppName; /* Class Name is App Name */
 if(!RegisterClass(&wndclass))
{
#if ENABLETRACE
GSSiExitProg (451);
#endif
   return -1;
}
  /* 
   wndclass.style = NULL;
   wndclass.lpfnWndProc = DDEWndProc;
   wndclass.cbClsExtra = 0;
   wndclass.cbWndExtra = 0;
   wndclass.hInstance = hInst;
   wndclass.hIcon = NULL;
   wndclass.hCursor = NULL;
   wndclass.hbrBackground = NULL;
   wndclass.lpszMenuName = NULL;
   wndclass.lpszClassName = "ClientDDEWndClass";    

 if(!RegisterClass(&wndclass)) retrn -1;  */
  // following registration added by lda
       //later removed when we started using ODBC
   //  wndclass.style = NULL;                    /* Class style(s).                    */
   //  wndclass.lpfnWndProc = DdeCallback;       /* Function to retrieve messages for  */
                                               /* windows of this class.             */
   //  wndclass.cbClsExtra = 0;                  /* No per-class extra data.           */
   //  wndclass.cbWndExtra = 0;                  /* No per-window extra data.          */
   //  wndclass.hInstance = hInst;               /* Application that owns the class.   */
   //  wndclass.hIcon =NULL;
   //  wndclass.hCursor = NULL;
   //  wndclass.hbrBackground = NULL;
   //  wndclass.lpszMenuName =  NULL;           /* Name of menu resource in .RC file. */
   //  wndclass.lpszClassName = "ServerWClass"; /* Name used in call to CreateWindow. */
// if(!RegisterClass(& wndclass))   retrn -1; 
  // wndclass.style = NULL;
  // wndclass.lpfnWndProc = DDEWndProc;
  // wndclass.cbClsExtra = 0;
  // wndclass.cbWndExtra = 0;
  // wndclass.hInstance = hInst;
  // wndclass.hIcon = NULL;
  //wndclass.hCursor = NULL;
  // wndclass.hbrBackground = NULL;
  // wndclass.lpszMenuName = NULL;
  // wndclass.lpszClassName = "ClientDDEWndClass";
 // if(!RegisterClass(&wndclass))   retrn -1;
 // end of lda addition   

{
#if ENABLETRACE
GSSiExitProg (451);
#endif
 return(0);
}
#if ENABLETRACE
}
#endif
} /* End of nCwRegisterClasses                                          */



BOOL ProcessCommandLine (LPSTR lpszCmdLine)
#if ENABLETRACE
{GSSiEnterProg (436);
#endif
{
 /***********************************************************************/
 /* HANDLE hInstance;       handle for this instance                    */
 /* HANDLE hPrevInstance;   handle for possible previous instances      */
 /* LPSTR  lpszCmdLine;     long pointer to exec command line           */
 /* int    nCmdShow;        Show code for main window display           */
 /***********************************************************************/

 MSG        msg;           /* MSG structure to store your messages        */
 int        nRc;           /* retrn value from Register Classes          */ 
 LPSTR	lpSpace, lpStart, lpEnd, lpStr, lpNext, pSC, pFS; 
 char	CmdLineA[1024], str[256];  
 LPSTR	CmdLine=CmdLineA;
 time_t	Time; 
 BOOL	Loaded;  
 LPSTR	lpOpt, pEnd; 
 int ierr;
 long	nf; 
 POINT	CPoint;   
 HWND	hFocus, hParent;            
 OFSTRUCTGM	OFStruct;
 long	ii;   
 char drive[34],dir[MAX_PATH]; 
 int	winx=CW_USEDEFAULT, winy=CW_USEDEFAULT, winh=CW_USEDEFAULT, winw=CW_USEDEFAULT;     
 static	BOOL	First=TRUE;
 BOOL	ForceConfig = FALSE;

 memset (CmdLineA,0,sizeof(CmdLineA));

/* ltoa (GetComputerID(),str,10);
 GSSiMsgBox (GetFocus(),str,"Processor ID",MB_OK);
 
 sprintf (str,"%f",GetDriveSize ('c'));
 GSSiMsgBox (GetFocus(),str,"Drive Size",MB_OK); */
 _fstrcpy(CmdLine,lpszCmdLine);  
 if (First)
 {  
 //MessageBox (0,lpszCmdLine,0,MB_OK);
//	sprintf (str,"$TEXTTOCLIPBOARD(%s)",CmdLine);
//	ProcessText (str);
	InfoBoxInit(&TAGBox);
	_getcwd (CurDir,MAX_PATH);  
//	SetHandleCount(MAXFILEHANDLES+16);    
//	GetPrivateProfileString ("Install","InDir","c:\\geomastr",str,256,"geomastr.ini"); 
//	_fstrcat (str,"\\");  
//	SetGlobalValue("%INDIR",str);
	_fstrcpy (szAppName,AppNames[App]);
	SetGlobalValue ("%APPID",szAppName);
	CreateInternalGlobals ();
	 if ((lpStart = _fstrstr(CmdLine," /CACHE ")))
	 {
 		lpStart += 8;
 		if ((lpNext = MatchLev (lpStart,' ')))
 		{
			char	TestForCDrive[MAX_PATH];

 			*lpNext++ = 0;  
			strcpy (TestForCDrive,lpStart);
			ExpandText (TestForCDrive);
			if (strnicmp (TestForCDrive,"c:\\",3))
			{
 				SetReplacePath (lpStart,3);
 				if ((lpStart = _fstrchr (lpNext,' ')))
 					*lpStart = 0;
 				SetReplacePath (lpNext--,4);
 				*lpNext = ' ';
 				if (lpStart)
 					*lpStart = ' ';
			}
			else
				*--lpNext = ' ';
 		}
	 }
 }
 else
 {
	 	if (strstr (CmdLine,"/F "))
		ForceConfig = TRUE;
	switch (App)
	{
		case 0:
			strcpy (str,"[%USERDIR]geomastr.ini");
			ExpandText (str);
			GetPrivateProfileString ("Install","DefaultArgs",lpszCmdLine,CmdLine,255,str);
			if (!*CmdLine)
				strcpy (CmdLine,DefaultArgs);
			break;
		default:
			_fullpath (str,"gmapp.ini",255);
			if (!CmdLine[0])
				GetPrivateProfileString ("Install","DefaultArgs",DefaultArgs,CmdLine,255,str);
		break;
	}
//	GetPrivateProfileString ("Install","AppName","GeoMaster",szAppName,sizeof(szAppName),str);   
		 
	 if ((lpSpace = _fstrstr (CmdLine,".gmc")))
 	 	lpSpace += 4;
 	 else
	 	lpSpace = _fstrstr (CmdLine," ");
	 if (lpSpace) 
	 {
	 	if (*lpSpace == '(')
	 	{
	 		if ((pEnd = MatchLev (lpSpace+1,')')))
	 		{   
	 			*lpSpace++ = 0;
	 			*pEnd = 0;  
	 			//MessageBox (0,lpSpace,0,MB_OK);
	 			ProcessText (lpSpace);
	 			lpSpace = pEnd + 1;
	 		}
	 	}
	 	*lpSpace = 0;
	 }
	 if (!ForceConfig && !App && !strstr (CmdLine,".tmp") && !strstr (CmdLine,".TMP"))
	 {
		strcpy (str,"[%USERDIR]geomastr.ini");
		ExpandText (str);
		GetPrivateProfileString ("User","DefaultConfig",CmdLine,CfgName,255,str);
		//if (!ExistFile (CfgName))
		//	strcpy (CfgName,CmdLine);
	 }
	 else if (!strstr (CmdLine,".tmp") && !strstr (CmdLine,".TMP"))
		strcpy (CfgName,CmdLine); 
	 else
	 {
		GetTempPath (MAX_PATH,CfgName); 
		sprintf (strchr(CfgName,0),"gmtemp\\%s",CmdLine);  
	 }
	 if (lpSpace) 
	 {
	 	CmdLine = lpSpace;
	 	*lpSpace = ' ';
	 }
	 _fstrupr (CfgName);
	 if (_fstrstr(CfgName,".PLT"))
	 {  
	 	char	fullp[MAX_PATH], Drive[4];
	 	int		drive;
	 	
	 	_fullpath (fullp,CfgName,sizeof(fullp));
		SetGlobalValue("%PLOT",fullp);
		_fstrcpy (CfgName,"BASIC1.GMC");
		GetPrivateProfileString ("Install","InstallDir","C:\\GEOMASTR",CurDir,128,"geomastr.ini");
	 	_splitpath (CurDir,Drive,0,0,0);
	 	drive = Drive[0] - 'A' + 1;
	 	_chdrive (drive);
	 	_chdir (CurDir);
	 } 
	 else if (_fstrstr(CfgName,".TMP"))
	 {
	 	char	TempName[MAX_PATH];
	 	long	len;
	 	LPSHORT pNumSavedViews;
	 	HFILE	Fid;
		LPSTR	pPar;
		BOOL	SaveAllowCache = AllowCache;

//		MessageBox (0,CfgName,0,MB_OK);
		AllowCache = FALSE;
	 	_fstrcpy (TempName,CfgName);
	 	Fid = GSSiOpenFile (TempName,0,OF_READ);
		AllowCache = SaveAllowCache;
		if (Fid != HFILE_ERROR)
		{
	 		BigRead (Fid,CfgName,MAX_PATH);
			if ((pPar = strrchr (CfgName,'(')))
			{
				*pPar++ = 0;
				hWndLinkedTo = (HWND)atol (pPar);
			}
	 		if (BigRead (Fid,(HPSTR)&len,4) == 4)
	 		{
				hSavedZooms = GSSiGlobAlloc (1527,GMEM_MOVEABLE,len);
				pNumSavedViews = (LPSHORT)GlobalLock (hSavedZooms);
				BigRead (Fid,(HPSTR)pNumSavedViews,len);
				GlobalUnlock (hSavedZooms);     
				ForceBounds = TRUE;
	 		}
	 		GSSiClose (Fid);
	 		GSSiRemove (TempName);
		}
	 }
	 else 
	 {
	 	if (!_fstrchr (CfgName,'.'))
	 		_fstrcat (CfgName,".GMC");
	 	_fstrcpy (CfgNameIn,CfgName);
	 }
}
//OpenPicStream (0);

 if (_fstrstr (CmdLine,"/ID"))
 {  
 	GSSiMsgBox (GetFocus(),MODULENAME,"",MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (436);
#endif
 	return FALSE;
}
 }
  if (First)
 { 
	 OriginalDrive = SaveDrive = _getdrive();
	 _getcwd (OriginalDir,MAX_PATH);
	 _getcwd (CurDir,MAX_PATH);  
	 CreateCursors();		 	
	 SetStartupGlobalValues (); 
	 _fstrcpy (str,"[%DATA_LOC]");
	 ExpandText (str);
	 if (!*str)
	 {
		_fstrcpy (str,"[%START_DIR]");
		ExpandText (str);
		if (*str)
		{   
			LPSTR lpEnd;
				 		
			lpEnd = _fstrchr (str,0);
			lpEnd--;
			if (*lpEnd != '\\')
				_fstrcat (str,"\\");
		}
		SetGlobalValue("%DATA_LOC",str);
	 }
	 else
	 {
		LPSTR lpEnd;
				 		
		lpEnd = _fstrchr (str,0);
		lpEnd--;
		if (*lpEnd != '\\') 
		{
			_fstrcat (str,"\\");
			SetGlobalValue("%DATA_LOC",str);
		}
	 } 
	 SetGlobalValue ("%DL",str);  
	 SetGlobalValue("%INDIR",str);
	 HaveDL = TRUE;
	 lpEnd = _fstrchr (str,0);
	 if (lpEnd != str)
	 {
	 	lpEnd--;
	 	if (*lpEnd == '\\')
	 		*lpEnd = 0;
	 }
	 if (*str && _fstricmp (str,CurDir))
	 	LoadGlobalInit ("[%DL]global.ini",FALSE);
	 PeopleNet = GetGlobalBVal ("[PEOPLENET]");
	 _fstrcpy (str,"[%FILEVPEDIT]=@[%DL]"); 
	 ExpandText (str);
	 if (GetGlobalCVal ("[%USERDIR]",str,0))
	 	LoadGlobalInit ("[%USERDIR]global.ini",FALSE); 
	 //dumpvars("Before.txt");  
	 _fstrcpy (UI.HomeDir,CurDir);
	 UI.svrName[0]='\0';
	 UI.Userid[0]='0';
 }
 ExpandText (CmdLine);
 if ((pSC = strrchr (CmdLine,';')))
	 *pSC = 0;
 _fstrcat (CmdLine," ");
 if (First)
 {
 }
 if ((lpStart = _fstrstr(CmdLine," /TOP ")))
 {
	 setWindowToTopOfZ = TRUE;
 }
 if ((lpStart = _fstrstr(CmdLine," /ZM ")))
 {

 	lpStart += 5;
 	if ((lpNext = _fstrchr (lpStart,'/')))
		*lpNext = 0;
	StartupZoom = atobounds (lpStart,&ierr);
	if (lpNext)
		*lpNext = '/';
 }
 if ((lpStart = _fstrstr(CmdLine," /SUBDIR ")))
 {
 	lpStart += 9;
 	if ((lpNext = _fstrchr (lpStart,' ')))
 	{
 		*lpNext++ = 0;  
 		SetReplacePath (lpStart,1);
 		if ((lpStart = _fstrchr (lpNext,' ')))
 			*lpStart = 0;
 		SetReplacePath (lpNext--,2);
 		*lpNext = ' ';
 		if (lpStart)
 			*lpStart = ' ';
 	}
 }
	 
 if ((lpStart = _fstrstr(CmdLine," /BACK ")))
 {
 	lpStart += 7;
 	if ((lpNext = _fstrchr (lpStart,' ')))
 	{
 		*lpNext++ = 0;  
 		sprintf (BackgroundMacro,"$MACRO(%s)",lpStart);
 		*--lpNext = ' ';
 		if (lpStart)
 			*lpStart = ' ';  
 		BackgroundTask = TRUE;
 	}
 }
	 
 
 if (_fstrstr(CmdLine," /TRACE "))
 	SetTrace(1); 
 else if (_fstrstr(CmdLine," /TRAC "))
 	SetTrace(2);
 
/* lpOpt = _fstrstr(CmdLine," /H ");
 if (lpOpt)
 {
 	UseHardDrive=TRUE;
 	lpOpt +=4;
	_fstrcpy(HardDrive,lpOpt);
 	lpSpace = _fstrchr (HardDrive,' '); 
 	if (lpSpace)
 		*lpSpace = 0;
 	_fstrcpy (HardDrive2,HardDrive); 
 } */ 
// if (_fstrstr(CmdLine," /GIF ")) GIDIndexFile=TRUE;
 if ((lpStart = _fstrstr (CmdLine,"/QUERY ")))
 {
 	lpStart+=7;
 	_fstrcpy (ThemeDB,lpStart); 
 	DisplayFinOpt=3;
 }	                     
 if ((lpStart = _fstrstr (CmdLine,"/DLL ")))
 {
 	lpStart+=5;
 	if ((lpSpace = _fstrchr (lpStart,' ')))
 		*lpSpace = 0;
// 	_fstrcpy (DLLDir,lpStart); 
 	if (lpSpace)
 		*lpSpace = ' ';
 }	                     
 if (_fstrstr(CmdLine," /DDE ")) WantDDE = TRUE;
 if (_fstrstr(CmdLine," /MIN "))
 	ShowMax=0;
 if (_fstrstr(CmdLine," /MINT "))
 	ShowMax=1;
 if (_fstrstr(CmdLine," /MINB "))
 	ShowMax=2;
 if (_fstrstr(CmdLine," /MINL "))
 	ShowMax=3;
 if (_fstrstr(CmdLine," /MINR "))
 	ShowMax=4;
 if (_fstrstr(CmdLine," /MINTL "))
 	ShowMax=5;
 if (_fstrstr(CmdLine," /MINTR "))
 	ShowMax=6;
 if (_fstrstr(CmdLine," /MINBL "))
 	ShowMax=7;
 if (_fstrstr(CmdLine," /MINBR "))
 	ShowMax=8;
 if ((lpOpt = strstr(CmdLine," /RECT ")))
 {
	 lpOpt += 7;
 	 ShowMax=9;
	 customRect = atorect (lpOpt,&ierr);
	 if (ierr || IsRectEmpty (&customRect))
		 ShowMax = 0;
 }
 if (_fstrstr(CmdLine," /NOMENU "))
 {   
 	 NoMenu=TRUE;
 	 SysMenu=TRUE;//prevents user menu from loading
 }  
 if (_fstrstr(CmdLine," /NOACCEL "))
 	 NoAccel=TRUE;
 
 if (_fstrstr(CmdLine," /NODISPLAY ")) NoDisplay=TRUE;
 if (_fstrstr(CmdLine," /SYSMENU ")) SysMenu=TRUE;
 if (_fstrstr(CmdLine," /DM ")) 	DisplayMarkers=TRUE;
 if (_fstrstr(CmdLine," /DF ")) 	DisplayFiles=TRUE;
// if (_fstrstr(CmdLine," /VIF "))	RawIndexFile=TRUE;
 if (_fstrstr(CmdLine," /SCAN "))DoDescScan=TRUE;

 if (_fstrstr(CmdLine," /DB ")) UMIODebug=TRUE;
 if (_fstrstr(CmdLine," /LOG ")) LogOn=TRUE;
 if (_fstrstr(CmdLine," /DBE ")) DebugExistFile=TRUE;
 if (_fstrstr(CmdLine," /NPF ")) PatternBrush=FALSE; 
 if (_fstrstr(CmdLine, " /UPDATESERVER ")) UpdateServer = TRUE; //MGV police new update server
 if ((lpStart = _fstrstr(CmdLine, " /MAPSERVER ")))//background map server
 {
	 lpStart += 12;
	 if ((lpNext = _fstrchr(lpStart, ' ')))
	 {
		 *lpNext = 0;
		 MapServerCalledFromWnd = (HWND)atoi(lpStart);
		 *lpNext++ = ' ';
		 lpStart = lpNext;
		 if (*lpStart == '\'')
		 {
			 lpStart++;
			 lpNext = strchr(lpStart, '\'');
			 *lpNext = 0;
			 strcpy(MapserverFile, lpStart);
			 *lpNext++ = '\'';
		 }
		 else
		 {
			 lpNext = _fstrchr(lpStart, ' ');
			 *lpNext++ = 0;
			 strcpy(MapserverFile, lpStart);
			 *--lpNext = ' ';
		 }
		 MapServer = TRUE;
		 BufferedScreen = TRUE;
		 AllowCache = FALSE;
		 NoAccel = TRUE;
		 NoMenu = TRUE;
		 wantGDIPlus = FALSE;

	 }
 }
 if (_fstrstr(CmdLine, " /RESET "))
 	DoReset = TRUE;  
// 	MessageBox (0,CmdLine,0,MB_OK);
 if ((lpStart = _fstrstr(CmdLine, " /SERVER ")))
 {
 	MemMap=TRUE;  
	NoMenu = FALSE;// TRUE;
 	InServerMode = TRUE;
   	if (BufferedScreen)
	{
		BufferedScreen = FALSE;
		ScreenBufferDC ((HWND)1,0);
	}
 	lpStart += 9;  
// 	MessageBox (0,"memmapset",0,MB_OK);
 	if (*lpStart && *lpStart != '/')
 	{
		char	TCPAddress[20];
		USHORT	port;
// 	MessageBox (0,"2",lpStart,MB_OK);
		if ((pFS = strchr (lpStart,'/')))
			*pFS = 0;
 		lpEnd = _fstrchr (lpStart,' ');
 		if (lpEnd && *(lpEnd+1) != '/')
 		{
 			port = atoi (lpEnd); 
			if (port)
			{
 				*lpEnd = 0;  
 				_fstrcpy (TCPAddress,lpStart);
 				*lpEnd = ' ';  
			}
			else
				goto PortOnly;
 		}
		else
		{
PortOnly:
 			port = atoi (lpStart); 
			GetMyIPNetAddress (TCPAddress);
		}
		if (pFS)
			*pFS = '/';
		OpenTCPIPServer (TCPAddress,port);
 	}
 }
 if ((lpStart = _fstrstr(CmdLine," /REPLAY ")))
 {
 	lpStart += 9;  
// 	MessageBox (0,"memmapset",0,MB_OK);
 	if (*lpStart && *lpStart != '/')
 	{
 		lpEnd = _fstrchr (lpStart,' ');
 		if (lpEnd)
 		{
 			*lpEnd = 0;  
			if (*LastChr (lpStart) == ';')
				*LastChr (lpStart) = 0;
 			OpenTCPReplay (lpStart);
 			*lpEnd = ' ';  
  		}
 	}
 }
 if (_fstrstr(CmdLine," /SHARE "))
 	ShareEnabled = TRUE;
 if (_fstrstr(CmdLine," /KFO "))
 	KeepFilesOpen = TRUE;
 if (First)
 {
	 GSSiTrace("*** Start Program ***",0); 
	 GSSiTrace (GMVersion,0);
	 GSSiTrace(CmdLine,0);
	 Time = time(0); 
	 GSSiTrace(ctime(&Time),0); 
 }
 _fstrcpy (str,"[%USER_ID]"); 
 ExpandText (str);
 if (_fstricmp (str,"[%USER_ID]"))
	_fstrcpy(UI.Userid,str);
 _fstrcpy (str,"[%ULTIMAP_SERVER]");
 ExpandText (str);
 if (_fstricmp (str,"[%ULTIMAP_SERVER]"))
	_fstrcpy(UI.svrName,str);

 lpStart = _fstrstr (CmdLine,"/TB ");
 if (!lpStart) lpStart = _fstrstr (CmdLine,"/IB ");
 if (lpStart)
 {
 	lpStart+=4;
 	lpSpace = _fstrstr(lpStart," ");
 	if (lpSpace) *lpSpace = 0;
	_fstrcpy(TagFile,lpStart);
	if (lpSpace)
	 	*lpSpace = ' ';
 }	                     
 lpStart = _fstrstr (CmdLine,"/OAX ");
 if (lpStart)
 {
 	lpStart+=4;
 	OrthoAdjustX = atof (lpStart);
 }	                     
 lpStart = _fstrstr (CmdLine,"/OAY ");
 if (lpStart)
 {
 	lpStart+=4;
 	OrthoAdjustY = atof (lpStart);
 }	                     

/*  lpStart = _fstrstr (CmdLine,"/LIB");
 if (lpStart)
 {
   Loaded = LoadLibraries(hInstance);           
  if(!Loaded) FreeLibraries();
 } */
  
 lpSpace = _fstrstr (CmdLine,"/C ");
 if (lpSpace)
 {	*lpSpace = '\0';   
 	CreateConfig = TRUE;
 }
 
 if (_fstrstr (CmdLine, "/POLICE "))
	 _fstrcpy(szAppName, "GeoMaster/Police"); 
/* else if (_fstrstr (CmdLine, "/HIGHWAYS ")) 
 {
 	RawIndexFile=TRUE; 
 	Highways = TRUE;
 	VehSizeOpt = 2;
	_fstrcpy(szAppName, "GeoMaster/Highways"); 
 } */
 else
	GetGlobalCVal ("[%APPID]",szAppName,"GeoMaster");
 if (First)
	  	LoadGlobalInit ("global.ini",TRUE);

 First = FALSE;

 
{
#if ENABLETRACE
GSSiExitProg (436);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetNodeParms (LPSTR NodeName,LPSTR Parms)
{
	char	str[260];
	char	File[MAX_PATH]="[%DL]nodeparm.txt";
	HFILE	Fid;
	BOOL	AllowCacheSave = AllowCache;
	
	strcpy (str,"$BATTERY(EXISTS)");
	ExpandText (str);
	if (*str == '1')
	{
		strcpy (File,"[%DL]nodeparm_laptop.txt");
		if (!ExistFile (File))
			strcpy (File,"[%DL]nodeparm.txt");
	}
		
	*Parms = 0;
	ExpandText (File);
	//GetShortPathName (File,128);
	AllowCache = FALSE;
	Fid = GSSiOpenFile (File,0,OF_READ);
	AllowCache = AllowCacheSave;
	if (Fid == HFILE_ERROR)
		return FALSE;
	while (fgetstring (str,256,Fid))
	{
		LPSTR pTAB=strchr (str,'\t');

		if (pTAB)
		{
			*pTAB = ' ';
			strcpy (Parms,pTAB);  
			*pTAB = 0;
			if (!_fstricmp (NodeName,str))
			{
				GSSiClose (Fid);
				return TRUE;
			}
		}
	}
	GSSiClose (Fid);

	return FALSE;
}

BOOL ProcessNodeParms (void)
{   
	char	str[512];
	
	char	CDriveSerno[20], SysNodeName[32], SysUserName[32],Winver[64];
	GSSiGetNodeInfo (CDriveSerno,SysNodeName,SysUserName,Winver);
     
    if (!GetGlobalCVal ("[%NODENAME]",NodeName,SysNodeName))
		SetGlobalValue("%NODENAME",NodeName);
    if (!GetGlobalCVal ("[%USERNAME]",UserName,SysUserName))
		SetGlobalValue("%USERNAME",UserName);  
	if (!*UserName)
		_fstrcpy (UserName,NodeName);
	GetNodeParms (NodeName,str);
	ProcessCommandLine (str);
	return TRUE;
}  

void ExpandDL (void)
{
	char	str[256];
	
	GetGlobalCVal ("[%DL]",str,0);
//	GetLongPathName (str,256);
	SetGlobalValue ("%DL",str);
	SetGlobalValue ("%DATA_LOC",str);
	strcpy(str, "[%USERDIR]");
	ExpandText(str);
	if (*str && FileType(str) == 2)
	{
		strcpy(str, "[%USERDIR]geomastr.ini");
		ExpandText(str);
	}
	else
	{
		GetGlobalCVal("[%DL]", str, 0);
		strcat(str, "geomastr.ini");
	}
	strcpy (GMIni,str);
	return;
}

BOOL ProcessUserParms (void)
{
	return TRUE;
}

BOOL FAR PASCAL SelectGMCmdMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	LPSTR lpStart, lpTab;
	OFSTRUCTGM	OFStruct;
	static char cmdFile[MAX_PATH];
	HFILE	fid;
	char	txt[1024], str[1024];
	int		tabStops[2] = { 500, 2000 };
	char	path[MAX_PATH];
	int		choice;

	switch (Message)
	{
	case WM_INITDIALOG:
		fid = OpenFileGM("lastcmdline.txt", &OFStruct, OF_READ);
		if (fid != HFILE_ERROR)
		{
			fgetstring2(txt, 1020, fid);
			SetDlgItemText(hWndDlg, IDC_COMMAND, txt);
			fgetstring2(txt, 1020, fid);
			SetDlgItemText(hWndDlg, IDC_DIRECTORY, txt);
			_lclose(fid);
		}

	case GSSI_REINITDIALOG:

		SendDlgItemMessage(hWndDlg, IDC_LIST1, LB_SETTABSTOPS,1, (LPARAM)tabStops);
		*selectedStartCmd = 0;
		cwCenter(hWndDlg, 0);
		SendDlgItemMessage(hWndDlg, IDC_LIST1, LB_RESETCONTENT, 0, 0);
		fid = OpenFileGM("../cmdlines.txt", &OFStruct, OF_READ);
		if (fid != HFILE_ERROR)
		{
			strcpy(cmdFile, OFStruct.szPathName);
			while (fgetstring2(txt, 1020, fid))
			{
				lpStart = _fstrstr(txt, "/WD ");
				if (lpStart)
				{
					char	path[MAX_PATH];
					LPSTR  lpEnd = 0;

					*lpStart = 0;
					lpStart += 4;
					if (*lpStart == '"')
						lpEnd = strchr(lpStart + 1, '"');
					else
					{
						int i = strcspn(lpStart, " ;");
						if (lpStart[i])
							lpEnd = &lpStart[i];
					}
					if (lpEnd)
						*lpEnd++ = 0;
					else
						lpEnd = strchr(txt, 0);
					sprintf(str, "%s%s\t%s", txt, lpEnd, lpStart);
					SendDlgItemMessage(hWndDlg, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)str);
				}
			}
			_lclose(fid);
		}
		break; /* End of WM_INITDIALOG                                 */

	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

	case GSSI_GMEDITCOMPLETE:
		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hWndDlg, FALSE);
			break;

		case IDC_DIRECTORY:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				if (GetDlgItemText(hWndDlg, IDC_DIRECTORY, path, MAX_PATH - 1))
				{
					char curPath[MAX_PATH];
					GetCurrentDirectory(MAX_PATH - 1, curPath);
					if (_chdir(path) < 0)
						EnableWindow(GetDlgItem(hWndDlg, IDOK), FALSE);
					else
						EnableWindow(GetDlgItem(hWndDlg, IDOK), TRUE);
					_chdir(curPath);
				}
			}
			break;

		case IDC_LIST1:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
				choice = SendDlgItemMessage(hWndDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);
				if (choice != LB_ERR)
				{
					SendDlgItemMessage(hWndDlg, IDC_LIST1, LB_GETTEXT, choice, (DWORD)str);
					lpTab = strchr(str, '\t');
					*lpTab++ = 0;
					SetDlgItemText(hWndDlg, IDC_COMMAND, str);
					SetDlgItemText(hWndDlg, IDC_DIRECTORY, lpTab);
					GetCurrentDirectory(MAX_PATH - 1, path);
					if (_chdir(lpTab) < 0)
						EnableWindow(GetDlgItem(hWndDlg, IDOK), FALSE);
					else
						EnableWindow(GetDlgItem(hWndDlg, IDOK), TRUE);
					_chdir(path);
				}
				break;
			case LBN_DBLCLK:
				PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				break;
			}
			break;

		case IDC_EDIT:
			GMEdit(hWndDlg, cmdFile);
			break;

		case IDOK:
		{
			char	drive[32];
			int		idrive;
			GetDlgItemText(hWndDlg, IDC_COMMAND, selectedStartCmd, 1020);
			GetDlgItemText(hWndDlg, IDC_DIRECTORY, path, MAX_PATH);
			fid = OpenFileGM("lastcmdline.txt", &OFStruct, OF_CREATE);
			if (fid != HFILE_ERROR)
			{
				fputstring2(selectedStartCmd, fid);
				fputstring2(path, fid);
				_lclose(fid);
			}
			if (*path)
			{
				ii = _chdir(path);
				_splitpath(path, drive, 0, 0, 0);
				if (*drive)
				{
					_fstrupr(drive);
					idrive = *drive - 'A' + 1;
					_chdrive(idrive);
				}
			}

			EndDialog(hWndDlg, TRUE);
			break;
		}
	}
		break;    /* End of WM_COMMAND                                 */

	default:
		return FALSE;
	}
	return TRUE;
}



BOOL GetCmdFileEntry(LPSTR cmdItem)
{
	BOOL rtn = FALSE;
	rtn = DialogBox(hInst, (LPCTSTR)"SELECTGMSTARTCMD", 0, (DLGPROC)SelectGMCmdMsgProc);
	strcpy(cmdItem, selectedStartCmd);
	return rtn;
}

void writeTestStruct(void)
{
	int l;
	FILE *fid = fopen("c:\\temp\\teststruct.bin", "wt");
	struct {
		int i4val;
		short i2val;
		float f4val;
		double f8val;
		char	textval[32];
	}test;
	test.i4val = 12345678;
	test.i2val = 456;
	test.f4val = 1002.345;
	test.f8val = 123456789.123;
	strcpy(test.textval, "How is this working");

	l = sizeof (test);
	fwrite(&test, l, 1, fid);
	fclose(fid);
	return;

}
void testConvertBitmapToPoly(LPSTR file);
/*void loadColors(void)
{
#include "c:\Temp\colorchart.h"
	OFSTRUCT OFStruct;
	HFILE fidIn = GSSiOpenFile("C:\\Users\\jeffrey\\Dropbox\\ColorChart.txt", &OFStruct, OF_READ);
	HFILE fidOut = GSSiOpenFile("c:\\temp\\colors.h",&OFStruct,OF_CREATE);
	char line[260];
	char outLine[260];
	int nColors = 0;
	int r[256], g[256], b[256];
	char name[256][32];
	int maxName = 0;
	int sort[NUMCOLORNAMES];
	while (fgetstring(line, 256, fidIn))
	{
		LPSTR pName = strstr(line, "name:\"");
		LPSTR pRGB = strstr(line, "rgb:\"#");
		LPSTR pEnd;
		if (pName && pRGB)
		{
			char Xcolor[12];
			COLORREF icolor;

			pName += 6;
			pEnd = strchr(pName, '\"');
			*pEnd = 0;
			pRGB += 6;
			pEnd = strchr(pRGB, '\"');
			*pEnd = 0;
			sprintf(Xcolor, "0X%s", pRGB);
			icolor = strtol(Xcolor, 0, 16);
			r[nColors] = GetRValue(icolor);
			g[nColors] = GetGValue(icolor);
			b[nColors] = GetBValue(icolor);
			pEnd = strstr(pName, "/grey");
			if (pEnd)
				*pEnd = 0;
			strcpy(name[nColors], pName);
			maxName = max (maxName,strlen(name[nColors]));
			nColors++;
		}
	}
	GSSiClose(fidIn);
	for (int i = 0; i < NUMCOLORNAMES; i++)
	{
		for (int j = 0; j < nColors; j++)
		{
			if (!strcmp(name[j], colorName[i]))
			{
				sort[i] = j;
				goto s10;
			}
		}

		ii = 1;
		s10:
		continue;
	}
	sprintf(outLine, "#define NUMCOLORNAMES %i", nColors);
	fputstring(outLine, fidOut);
	sprintf(outLine, "\tchar colorName[NUMCOLORNAMES][%i] ={\"%s\"", maxName + 1, name[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,\"%s\"", name[sort[i]]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	sprintf(outLine, "\tunsigned char R[NUMCOLORNAMES] ={%i", r[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,%i", r[sort[i]]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	sprintf(outLine, "\tunsigned char G[NUMCOLORNAMES] ={%i", g[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,%i", g[sort[i]]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	sprintf(outLine, "\tunsigned char B[NUMCOLORNAMES] ={%i", b[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,%i", b[sort[i]]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	GSSiClose(fidOut);
}*/
/*void loadColorChart(void)
{
#include "c:\Temp\colorchart.h"
	OFSTRUCT OFStruct;
	LPSTR pName;
	HFILE fidIn = GSSiOpenFile("C:\\Users\\jeffrey\\Dropbox\\ColorChart.txt", &OFStruct, OF_READ);
	HFILE fidOut = GSSiOpenFile("c:\\temp\\colorchart.h", &OFStruct, OF_CREATE);
	char line[12000];
	char outLine[260];
	int nColors = 0;
	char name[256][32];
	int r[256], g[256], b[256];
	int maxName = 0;
	BigRead(fidIn, line, sizeof(line));
	line[sizeof(line)-1] = 0;
	pName = strstr(line, "background-color:");
	while (pName)
	{
		LPSTR pEnd = strchr(pName, '\"');
		*pEnd++ = 0;
		pName += 17;
		strcpy(name[nColors], pName);
		LPSTR pRGB = strstr(pEnd, "#");
		if (pName && pRGB)
		{
			char Xcolor[12];
			COLORREF icolor;

			pRGB++;
			pEnd = strchr(pRGB, '\"');
			*pEnd = 0;
			sprintf(Xcolor, "0X%s", pRGB);
			icolor = strtol(Xcolor, 0, 16);
			b[nColors] = GetRValue(icolor);
			g[nColors] = GetGValue(icolor);
			r[nColors] = GetBValue(icolor);
			LPSTR pGray = strstr(pName, "/grey");
			if (pGray)
				*pGray = 0;
			maxName = max (maxName,strlen(name[nColors]));
			nColors++;
		}
		pName = strstr(pEnd+1, "background-color:");
	}
	GSSiClose(fidIn);
	sprintf(outLine, "#define NUMCOLORNAMES %i", nColors);
	fputstring(outLine, fidOut);
	sprintf(outLine, "\tchar colorName[NUMCOLORNAMES][21] ={\"%s\"", name[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,\"%s\"", name[i]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	sprintf(outLine, "\tunsigned char R[NUMCOLORNAMES] ={%i", r[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,%i", r[i]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	sprintf(outLine, "\tunsigned char G[NUMCOLORNAMES] ={%i", g[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,%i", g[i]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	sprintf(outLine, "\tunsigned char B[NUMCOLORNAMES] ={%i", b[0]);
	fputstring(outLine, fidOut);
	for (int i = 1; i < nColors; i++)
	{
		sprintf(outLine, "\t,%i", b[i]);
		fputstring(outLine, fidOut);
	}
	sprintf(outLine, "\t};");
	fputstring(outLine, fidOut);

	GSSiClose(fidOut);
}
*/

WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	char cmdLine[1024];
	//char monName[128];
	int  monStatus, mouseType;
	LPSTR keyloc;
	BOOL haveKey = FALSE;
	CreatePrintBitmap(0);
//	MessageBox(0, lpszCmdLine, 0, MB_OK);
	//loadColors();
	//loadColorChart();
	//testConvertBitmapToPoly("C:\\Temp\\AreaTests\\test_100102158.bmp");
	//testConvertBitmapToPoly("C:\\Temp\\AreaTests\\test_100031654.bmp");
	//testConvertBitmapToPoly("C:\\Temp\\AreaTests\\test_100095547.bmp");
//#define FV	$(TargetName) 
	//UDPmain(22336);
	//GetMassShapeFiles();
	//writeTestStruct();
	//char tt[] = {FV};
	/*char prj[1024];
	char outprj[1024];
	HFILE fid = GSSiOpenFile("C:\\GMMobile\\MetroUTM\\MGFire\\Water System\\wMain.prj", 0, OF_READ);
	BigRead(fid, prj, 1023);
	GSSiClose(fid);
	ConvertPRJtoProj4(prj,outprj);*/
	_set_invalid_parameter_handler(
		my_invalid_parameter_handler
		);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	numMonitors = GetNumMonitors();
	typeChassis = ChassisType();
	//mouseType = MouseType();
	if (typeChassis == 3)
	{
		RECT rect;

		isTouchScreen = TRUE;
		GetClientRect(GetDesktopWindow(), &rect);
		if (RECTWIDTH(&rect) > 1280 || RECTHEIGHT(&rect)>1280)
			isTouchScreen = FALSE;
	}
	//monStatus = MonitorType(1,monName);
	//MessageBox (0,lpszCmdLine,"First",MB_OK);
	if (strstr(lpszCmdLine, "/CMDFILE "))
	{
		if (!GetCmdFileEntry(cmdLine))
			return 0;
	}
	else
		strncpy (cmdLine,lpszCmdLine,1024);
	if ((keyloc = strstr(cmdLine, "KEYLOC=")))
	{
		*keyloc = 0;
		keyloc += 7;
		if (stricmp(keyloc, "03231949"))
			return 0;
		haveKey = TRUE;
	}
	if (*LastChr (cmdLine) != ';')
		strcat (cmdLine," ");

	CreateVarSpace(VARSPACE_GLOBAL);

	if (strstr(cmdLine, "/GMEdit"))
	{
		isGMEdit = TRUE;
		CreateBigMem();
		AllowCache = FALSE;
		ProcessCommandLine("");
		return WinMainGMEdit(hInstance, hPrevInstance, cmdLine, nCmdShow);
	}
	else if (strstr(cmdLine, "/GMDoc"))
	{
		isGMEdit = TRUE;
		CreateBigMem();
		AllowCache = FALSE;
		ProcessCommandLine("");
		return WinMainGMDoc(hInstance, hPrevInstance, cmdLine, SW_MAXIMIZE);
	}
	else
	{
		//if (!haveKey)
		//	return 0;
		return WinMainGeoMaster(hInstance, hPrevInstance, cmdLine, nCmdShow);
	}
}

int PASCAL WinMainGeoMaster(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
#if ENABLETRACE
{GSSiEnterProg (437);
#endif
{                   
 /***********************************************************************/
 /* HANDLE hInstance;       handle for this instance                    */
 /* HANDLE hPrevInstance;   handle for possible previous instances      */
 /* LPSTR  lpszCmdLine;     long pointer to exec command line           */
 /* int    nCmdShow;        Show code for main window display           */
 /***********************************************************************/
 MSG        msg;           /* MSG structure to store your messages        */
 int        nRc=0;           /* retrn value from Register Classes          */ 
 time_t	Time; 
 BOOL	Loaded,SwitchWindow=FALSE;  
 LPSTR	lpOpt; 
 long	nf; 
 POINT	CPoint;   
 HWND	hFocus, hParent;            
 int	ii,winx=CW_USEDEFAULT, winy=CW_USEDEFAULT, winh=CW_USEDEFAULT, winw=CW_USEDEFAULT;   
 char	Menu[]="geomaster";
 LPSTR	pMenu=Menu;
 long	nMess;
 HANDLE	hMess=0;
 LARGE_INTEGER Frequency;
 BOOL	HaveHFPC = QueryPerformanceFrequency(&Frequency);
 INITCOMMONCONTROLSEX InitCtrls;

 ii = 1;
 ii = ii | 64;
 ii = ii ^ 64;
 //ReadLidarHeader ("G:\\srs.las");
 //ReadLidarHeader ("G:\\3542-28-16_a_a.las");
 //Create256RotationHeaders ();
 //RGBTRIPLE c=COLORREFtoRGBTRIPLE (16679288);
 memset (&InitCtrls,0,sizeof(INITCOMMONCONTROLSEX));
 InitCtrls.dwICC = ICC_WIN95_CLASSES;
 InitCtrls.dwSize = sizeof (INITCOMMONCONTROLSEX);
 InitCommonControlsEx(&InitCtrls);
// LoadPuertoRicoCities (1);
// AddPuertoRicoToCities4 (1);

 //testfgdb(1);
// LPSTR	ptest=calloc (30000000,1);
//TestFillTBRows (1);
// free (ptest);
//checkcpl(1);
// GetDriveSerialNumber ("E:\\");
// GetSDChipSerialNumber("E:\\");

 /*{
	 HFILE Fid=GSSiOpenFile ("C:\\Users\\Jeff\\Documents\\Copy of Latitudes1.txt",0,OF_READ);
	 HFILE Fid2=GSSiOpenFile ("h:\\gmavl\\attribut\\epbusstops.txt",0,OF_CREATE);
	 char	line[1024],line2[1024];
	 LPSTR	pLoc;
	 int	nOut,nTab;

	 while (fgetstring (line,1020,Fid))
	 {
		 pLoc = line;
		 nOut = 0;
		 nTab = 0;
		 while (*pLoc)
		 {
			 if (*pLoc == '\t')
		 {
				 if (nOut && !nTab++)
					 line2[nOut++] = *pLoc;
			 }
			 else if (*pLoc != ' ')
			 {
				 line2[nOut++] = *pLoc;
				 nTab = 0;
			 }
			 else if (!nTab)
				 line2[nOut++] = *pLoc;
			 pLoc++;
		 }
		 line2[nOut++] = *pLoc;
		 fputstring (line2,Fid2);
	 }
	 GSSiClose (Fid);
	 GSSiClose (Fid2);
}
*/
 
#if !CHECKMEM
	AddVectoredHandlers (0,0);
#endif
//long size=1024L*1024L*256;
 //HANDLE hMem=GSSiGlobAlloc (0,GHND,size);
 //GSSiGlobFree (&hMem);
 //RecoverBadFile ();
 //int	t=IDNINT(atof("1E+08"));

  //MessageBox (0,lpszCmdLine,"In GeoMaster",MB_OK);

#if CHECKMEM
	InDebug=TRUE;
#endif
if (App == 1)
{
	CheckForLicense = TRUE;
	abcd (App);
}

 switch (App)
 {
 case 1:
	 if (!InDebug && (!*MODULENAME || strncmp (MODULEIDSTRING,"23",2)))
	 {
		 MessageBox (0,"Invalid module - not stamped",0,MB_ICONEXCLAMATION);
		 exit (1);
	 }
 default:
	 break;
 }

SetOldStructSizes ();

//MessageBox (0,"Past AddVectoredHandlers","",MB_OK);
 _tzset ();
 time (&SystemStartTime);
//EnableTrace=2;
 hInst = hInstance; 
// MessageBox (0,lpszCmdLine,0,MB_OK);  
// if (OFS_MAXPATHNAME != 256)
//	 MessageBox (0,"OFS_MAXPATHNAME is not 256",0,MB_ICONEXCLAMATION);
ii=_WIN32_WINNT;
{
	THEME theme;

	ii=sizeof (theme.filler);
	ii=1;
}
/*{
	typedef	struct	{int	nrow,ncol;
			int	left,bottom;
			int	cellw,cellh;
			int	ifac;
			double	fac;
			int	GridOffset;
			}STATEGRIDHEADER;
typedef STATEGRIDHEADER	*LPSTATEGRIDHEADER;
STATEGRIDHEADER	Head1;
typedef	struct	{int	nrow,ncol;
			int	left,bottom;
			int	cellw,cellh;
			int	ifac;
			int	GridOffset;
			double	fac;
			}STATEGRIDHEADER2;
typedef STATEGRIDHEADER2	*LPSTATEGRIDHEADER2;
STATEGRIDHEADER2 Head2;
HFILE	Fid = GSSiOpenFile ("c:\\pngrid\\400\\pngrid.bin",0,OF_READWRITE);
	BigRead (Fid,&Head1,sizeof(Head1));
	GSSillseek (Fid,0,0);
	Head2.nrow = Head1.nrow;
	Head2.ncol = Head1.ncol;
	Head2.left = Head1.left;
	Head2.bottom = Head1.bottom;
	Head2.cellw = Head1.cellw;
	Head2.cellh = Head1.cellh;
	Head2.ifac = Head1.ifac;
	Head2.GridOffset = Head1.GridOffset;
	Head2.fac = Head1.fac;
	BigWrite (Fid,&Head2,sizeof(Head2),-1);
	GSSiClose (Fid);
}*/

// testMemIO("c:\\test.tif");
//if (!LoadGM32Lib (FALSE)) 
//	return FALSE;
 CreateBigMem ();  
 GoogleTilesInit ();		
 _getcwd (StartupDir,64);   
 _fstrcpy (AppName,ApName);
 //MessageBox (0,lpszCmdLine,"Command Line",MB_OK);
 ProcessNodeParms ();
if (!ProcessCommandLine (lpszCmdLine))  
 {
{
#if ENABLETRACE
GSSiExitProg (437);
#endif
	FreeBigMem (); 
	return FALSE;
}
 } 
ExpandDL ();
/*{
	 BITMAPINFOHEADER    bmiHeader1, bmiHeader2;
	 BITMAPFILEHEADER	fH1,fH2;
	 BYTE im1[256*256];
	 BYTE im2[256*256];
	 int	i,n=0;
	 HANDLE	hMemCmp1;
	 LPBYTE pMemCmp1;
	 HANDLE	hMemCmp2;
	 LPBYTE pMemCmp2;
	 HANDLE	hMemF1;
	 LPBYTE f1;
	 HANDLE	hMemF2;
	 LPBYTE f2;
	 int	lf1,lf2;
	 int	lMemCmp1, lMemCmp2;
	 LPLONG	pPal1,pPal2;

	 HFILE Fid1 = GSSiOpenFile ("c:\\temp\\test_good.bmp",0,OF_READ);
	 HFILE Fid2 = GSSiOpenFile ("c:\\temp\\test_bad2.bmp",0,OF_READ);

	 lf1=GSSifilelength (Fid1);
	 lf2=GSSifilelength (Fid2);
	 hMemF1 = GSSiGlobAlloc (0,GMEM_MOVEABLE,lf1);
	 hMemF2 = GSSiGlobAlloc (0,GMEM_MOVEABLE,lf2);
	 f1 = GlobalLock (hMemF1);
	 f2 = GlobalLock (hMemF2);
	 BigRead (Fid1,f1,lf1);
	 BigRead (Fid2,f2,lf2);
	 GSSillseek (Fid1,0,0);
	 GSSillseek (Fid2,0,0);
	 BigRead (Fid1,&fH1,sizeof(BITMAPFILEHEADER));
	 BigRead (Fid2,&fH2,sizeof(BITMAPFILEHEADER));
	 BigRead (Fid1,&bmiHeader1,sizeof(BITMAPINFOHEADER));
	 BigRead (Fid2,&bmiHeader2,sizeof(BITMAPINFOHEADER));
	 GSSillseek (Fid1,fH1.bfOffBits,0);
	 GSSillseek (Fid2,fH2.bfOffBits,0);
	 BigRead (Fid1,im1,sizeof(im1));
	 BigRead (Fid2,im2,sizeof(im2));
	 for (i=0;i<sizeof(im2);i++)
	 {
		 if (im1[i] != im2[i])
		 {
			 n++;
		 }
	 }
	 pPal1 = &f1[sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)];
	 pPal2 = &f2[sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)];
	 for (i=bmiHeader2.biClrImportant;i<256;i++)
		 *pPal2++ = 0;
	 for (i=0;i<lf1-sizeof(im1);i++)
	 {
		 if (f1[i] != f2[i])
		 {
			 n++;
		 }
	 }
	 hMemCmp1 = GSSiGlobAlloc (0,GMEM_MOVEABLE,lf1+1024);
	 pMemCmp1 = GlobalLock (hMemCmp1);
	 lMemCmp1 = CompressByteArray (pPal1,pMemCmp1,1024);
	 lMemCmp1 = CompressByteArray (f1,pMemCmp1,lf1);
	 hMemCmp2 = GSSiGlobAlloc (0,GMEM_MOVEABLE,lf2+1024);
	 pMemCmp2 = GlobalLock (hMemCmp2);
	 lMemCmp2 = CompressByteArray (pPal2,pMemCmp2,1024);
	 lMemCmp2 = CompressByteArray (f2,pMemCmp2,lf2);
	 GSSiGlobUlFree (&hMemCmp1);
	 GSSiGlobUlFree (&hMemCmp2);
 	 GSSiGlobUlFree (&hMemF1);
 	 GSSiGlobUlFree (&hMemF2);

	 GSSiClose (Fid1);
	 GSSiClose (Fid2);

}*/

if (!MapServer)
	nTempFilesCleared = ClearGMTempFiles ();
//MessageBox (0,"Past ClearGMTempFiles","",MB_OK);
ProcessUserParms ();
{
	HANDLE	hStr=GSSiGlobAlloc (   4,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr); 

	GetGlobalCVal ("[%DL]",str,0);
	AddVectoredHandlers (UserName,str);
	GSSiGlobUlFree (&hStr);
}
PeopleNet = GetGlobalBVal ("[PEOPLENET]");

//if (!RunFromCache)
//	AllowCache = TRUE;
 if(!hPrevInstance)
   {
    /* register window classes if first instance of application         */   
    if (NoMenu)
    	pMenu = 0;
    if ((nRc = nCwRegisterClasses(pMenu)) == -1)
      {
			HANDLE	hStr=GSSiGlobAlloc (   3,GMEM_MOVEABLE,256);
			LPSTR	str=GlobalLock (hStr); 
			/* registering one of the windows failed                         */
			LoadString(hInst, IDS_ERR_REGISTER_CLASS, str, 255);
			GSSiMsgBox(0, str, 0, MB_ICONEXCLAMATION,0);
			GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (437);
#endif
			FreeBigMem (); 
			return nRc;
}
      }
   }
	 GetWindowRect(GetDesktopWindow(), &DeskRect); 
	 //GetClientRect(0, &DeskRectC); 
	 SystemParametersInfo(SPI_GETWORKAREA,0,&DeskRectC,0);


	 switch (ShowMax)
	 {
		case 0:
 			winx=0;
 			winy=0;
 			winw=600;
 			winh=300;
		break;
		case 1://MINT
			winx = DeskRectC.left;
			winy = DeskRectC.top;
			winw = RECTWIDTH (&DeskRectC);
			winh = RECTHEIGHT (&DeskRectC)/2;
			break;
		case 2://MINB
			winx = DeskRectC.left;
			winy = RECTHEIGHT (&DeskRectC)/2;
			winw = RECTWIDTH (&DeskRectC);
			winh = RECTHEIGHT (&DeskRectC)/2;
			break;
		case 3://MINL
			winx = DeskRectC.left;
			winy = DeskRectC.top;
			winw = RECTWIDTH (&DeskRectC)/2;
			winh = RECTHEIGHT (&DeskRectC);
			break;
		case 4://MINR
			winx = RECTWIDTH (&DeskRectC)/2;
			winy = DeskRectC.top;
			winw = RECTWIDTH (&DeskRectC)/2;
			winh = RECTHEIGHT (&DeskRectC);
			break;
		case 5://MINTL
			winx = DeskRectC.left;
			winy = DeskRectC.top;
			winw = RECTWIDTH (&DeskRectC)/2;
			winh = RECTHEIGHT (&DeskRectC)/2;
			break;
		case 6://MINTR
			winx = RECTWIDTH (&DeskRectC)/2;
			winy = DeskRectC.top;
			winw = RECTWIDTH (&DeskRectC)/2;
			winh = RECTHEIGHT (&DeskRectC)/2;
			break;
		case 7://MINBL
			winx = DeskRectC.left;
			winy = RECTHEIGHT (&DeskRectC)/2;
			winw = RECTWIDTH (&DeskRectC)/2;
			winh = RECTHEIGHT (&DeskRectC)/2;
			break;
		case 8://MINBR
			winx = RECTWIDTH (&DeskRectC)/2;
			winy = RECTHEIGHT (&DeskRectC)/2;
			winw = RECTWIDTH (&DeskRectC)/2;
			winh = RECTHEIGHT (&DeskRectC)/2;
			break;
		case 9://RECT
 			winx = customRect.left;
			winy = customRect.top;
			winw = RECTWIDTH (&customRect);
			winh = RECTHEIGHT (&customRect);
			break;
//if (GetGlobalBVal2 ("[%DualScreen]",FALSE))
		default:
		 winw = CW_USEDEFAULT;
		 winh = 0;
		 winx = CW_USEDEFAULT;
		 winy = 0;
//			winx = DeskRectC.left;
//			winy = DeskRectC.top;
//			winw = RECTWIDTH (&DeskRectC);
//			winh = RECTHEIGHT (&DeskRectC);
}
 /* create application's Main window                                    */
	 {
		 char mapServerAppName[] = { "GeoMaster MapServer" };
		 HWND hPar = 0;
		 LPSTR pAppName = szAppName;
		 DWORD style = WS_CAPTION |        /* Title and Min/Max           */
			 WS_SYSMENU |        /* Add system menu box         */
			 WS_MINIMIZEBOX |        /* Add minimize box            */
			 WS_MAXIMIZEBOX |        /* Add maximize box            */
			 WS_THICKFRAME |        /* thick sizeable frame        */
			 //WS_MAXIMIZE |        /* create maximized window     */
			 //WS_CLIPCHILDREN |         /* don't draw in child windows areas */
			 //WS_CLIPSIBLINGS |
			 WS_OVERLAPPED;

		 if (MapServer)
		 {
			 style = 0;
			 mapServerWidth = winw;
			 mapServerHeight = winh;
			 winw = 100;
			 winh = 100;
			 pAppName = mapServerAppName;
			 if (dbug)
			 {
				 char mess[128];
				 sprintf(mess, "winxywh %i %i %i %i", winx, winy, winw, winh);
				 MessageBox(0, mess, "", MB_OK);
			 }
		 }

		 hWndMain = CreateWindowEx(WS_EX_APPWINDOW,
			 szAppName,               /* Window class name           */
			 pAppName,             /* Window's title              */
			 style,
			 winx, winy, winw, winh,
			 /*CW_USEDEFAULT, 0,  */      /* Use default X, Y            */
			 /*CW_USEDEFAULT, 0,*/        /* Use default X, Y            */
			 hPar,                    /* Parent window's handle      */
			 0,                    /* Default to Class Menu       */
			 hInst,                   /* Instance of window          */
			 0);                   /* Create struct for WM_CREATE */
	 }
 if (hWndLinkedTo)
	 PostMessage (hWndLinkedTo,GF_CONNECT_PROCESS,(WPARAM)hWndMain,0);
 //TraceWnd = hWndMain;
 if(hWndMain == 0)
   {
		HANDLE	hStr=GSSiGlobAlloc (   4,GMEM_MOVEABLE,256);
		LPSTR	str=GlobalLock (hStr); 
		LoadString(hInst, IDS_ERR_CREATE_WINDOW, str, 255);
		GSSiMsgBox(0, str, 0, MB_ICONEXCLAMATION,0);
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (437);
#endif
		FreeBigMem (); 
		return IDS_ERR_CREATE_WINDOW;
}
   }
 else
 {
	 CreatePrintBitmap(hWndMain);

	 OpenTCPIPServer2(hWndMain);
	 if (_fstrstr(lpszCmdLine, " /TARGET "))
	 {
		 HFILE fid = GSSiOpenFile("c:\\temp\\targetwindow.bin", 0, OF_CREATE);
		 BigWrite(fid, &hWndMain, sizeof(HWND), -1);
		 GSSiClose(fid);
	 }

 }
   PromptFocus = hWndMain;
    {
    	static	FirstAct=TRUE;
	    	
    	if (FirstAct)
    	{
	    	FirstAct = FALSE;
			if (!MapServer && !ValidateLicense(hWndMain))
			{
				DestroyWindow(hWndMain); 
{
#if ENABLETRACE
GSSiExitProg (437);
#endif
				FreeBigMem (); 
	        	return 0;
}
	        }
	    } 
	}
 
 // added by LDA 
 GotItUp = FALSE;
 Ready = FALSE; 
  //  aFormats[0].atom = CF_TEXT; // exception - predefined.
  //  for (i = 1; i < CFORMATS; i++) 
  // {
  //      aFormats[i].atom = RegisterClipboardFormat(aFormats[i].sz);
   // } // end of lda addition
if (setWindowToTopOfZ)
	SetWindowPos (hWndMain,HWND_TOP,0,0,0,0,SWP_NOSIZE);
if (MapServer)
	ShowWindow(hWndMain, SW_HIDE);
else if (BackgroundTask && !UpdateServer)
	ShowWindow(hWndMain, SW_SHOWMINIMIZED);
else if (ShowMax == 10)
{
	HDC hDC;

	showWindowCmd = SW_SHOWMAXIMIZED;
	ShowWindow(hWndMain, showWindowCmd);
	ShowWindow(hWndMain, SW_HIDE);
	hDC = GetDC(hWndMain);
	/*{
		HPEN	hCPen, hPen = 0;

		LOGPEN	lPen;
		int	width;

		hCPen = SelectObject(hDC, GetStockObject(BLACK_PEN));
		GetObject(hCPen, sizeof(LOGPEN), &lPen);
		width = 1;
	}*/

	OpenConfig(hWndMain, hDC);
	ReleaseDC(hWndMain, hDC);
}
else
{
	HDC hDC;

	showWindowCmd = SW_SHOW;
	ShowWindow(hWndMain, SW_HIDE);
	hDC = GetDC(hWndMain);
	OpenConfig(hWndMain, hDC);
	ReleaseDC(hWndMain, hDC);
}
   {
	   RECT	rect;

	   GetWindowRect (hWndMain,&rect);
	   rect.left = 0;
   }
   if (!NoAccel)
   {
	   if (!_fstricmp (szAppName,"SportMap"))  
	   		hAccelTable = LoadAccelerators(hInst,szAppName);
	   else //if (!Highways)
	   		hAccelTable = hAccelMain = LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ACCELERATOR1)); 
//	   else
//	   		hAccelTable = LoadAccelerators(hInst,"HIGHWAY_ACCEL"); 
   }	

   hAccelAlt = LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ALTACCEL));
   hAccelTableHLT = LoadAccelerators(hInst,"HLT_ACCEL");
//   Func = SetWindowsHookEx(WH_MSGFILTER, FilterFunc, hInst,0 ); Used for Dynamic dialog stuff
//   MHookFunc = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hInst,0 );
//   if (WantDDE)
//   	InitDdeStuff(hInst,FALSE);//Highways);                                       
    // end of lda addition 
   ii = GetCurrentThreadId();
//PrevFIHook = SetWindowsHookEx(WH_FOREGROUNDIDLE, ForegroundIdleProc, hInst,GetCurrentThreadId() );
 ii=GetLastError();  
if (BackgroundTask)
	PostMessage (hWndMain,GF_PROCESS_BACKGROUND_CMD,0,0);
else if (MapServer)
{
	if (HaveWMCreate)
	{
		if (OpenConfig(hWndMain, 0))
		{
			PostMessage(MapServerCalledFromWnd, GF_MAPSERVER_READY, (WPARAM)hWndMain, MapserverVPID);
			MoveWindow(hWndMain, 0, 0, mapServerWidth, mapServerHeight, TRUE);
			//PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 99L);
			  //MessageBox(0, "Mapserver Open", "", MB_OK);
			SetTimer(hWndMain, SUICIDE_TIMER, 2000, 0);
		}
		else
		{
			PostMessage(MapServerCalledFromWnd, GF_MAPSERVER_FAILED, (WPARAM)hWndMain, MapserverVPID);
			PostMessage(hWndMain, WM_COMMAND, IDM_EXIT, 0L);
		}

	}
}
nMess = -1;
 while(hWndMain && GetMessage(&msg, 0, 0, 0))        /* Until WM_QUIT message    */
   {    
#ifndef	NDEBUG
	 nMess++;
	 if (nMess > 99)
		 nMess = 0;
	 pmsg[nMess] = msg;   // pmsg[nMess-1] pmsg[nMess-2] pmsg[nMess-3] pmsg[nMess-4] pmsg[nMess-5] 
	 sizeof (msg);

	 switch (msg.message)
	 {
		 case SPI_SETMOUSESPEED:
			 ii = 1;
		 break; case WM_PRINT:
			 ii = 1;
		 break; case WM_PRINTCLIENT:
			 ii = 1;
		 break; case WM_CLOSE:
			 ii = 1;
		 break; case WM_PAINT:
			 ii = 1;
		 break; case WM_LBUTTONDBLCLK:
			 ii = 1;
		 break; case WM_SETFOCUS:
			 ii = 1;
		 break; case WM_MOVE:
			 ii = 1;
		 break; case WM_WINDOWPOSCHANGED:
			 ii = 1;
		 break; case WM_WINDOWPOSCHANGING:
			 ii = 1;
		 break; case WM_F1DOWN:
			 ii = 1;
		 break; case WM_SHOWWINDOW:
			 ii = 1;
		 break; case WM_IME_SETCONTEXT:
			 ii = 1;
		 break; case WM_KEYDOWN:
			 ii = 1;
		 break; case WM_SYSKEYDOWN:
			 ii = 1;
		 break; case WM_LBUTTONDOWN :
			 ii = 1;
		 break; case WM_LBUTTONUP :
			 ii = 1;
		 break; case WM_CHAR:
			 ii = 1;
		 break; case WM_NOTIFY:
			 ii = 1;
		 break; case WM_TIMER:
			 ii = 1;
			 break;
	 }
	 if (msg.message == WM_CHAR && msg.wParam == 9) //TAB
			 ii = 1;
#endif
		if (msg.message == WM_CHAR && msg.wParam == 26) //CNTL/Z 
		{
			if (UndoEnabled)
			{
				EscapeFunction (TRUE);
				if (UndoChanges (hWndMain))  
				{
					RedisplayWindow();            
				} 
			}
			ii=1; 
		}
		if (hWndAddEdit)
		{	   
			if (IsDialogMessage(hWndAddEdit, &msg))
			{
        		continue; 
        	} 
        }
		if (hWndVehHist)
		{	   
			if (IsDialogMessage(hWndVehHist, &msg))
			{
        		continue; 
        	} 
	   		if (msg.message == WM_KEYDOWN && (msg.wParam == 40 || msg.wParam == 38))
				continue;
        }
        if (hWndSlider && msg.hwnd == hWndSlider)
		{
			if (msg.message == WM_MOUSEMOVE)
					PostMessage(hWndHighlight, WM_COMMAND, IDC_SLIDERMOUSE, msg.lParam);  
			if (msg.message == WM_PAINT)
					PostMessage(hWndHighlight, WM_COMMAND, IDC_SLIDERPAINT, msg.wParam);  
		}
		if (hWndStrmPipe)
		{   
		
			if (IsDialogMessage(hWndStrmPipe, &msg))
			{
				if (msg.message == WM_CHAR && msg.wParam == 5)
					PostMessage(msg.hwnd, WM_COMMAND, IDC_DYNEDIT, 0L);  
        		continue; 
        	}
        }
		else if (hWndStrmPoint)
		{   
		
			if (IsDialogMessage(hWndStrmPoint, &msg))
			{
				if (msg.message == WM_CHAR && msg.wParam == 5)
					PostMessage(msg.hwnd, WM_COMMAND, IDC_DYNEDIT, 0L);  
        		continue; 
        	}
        }
		if (hWndUserForm)
		{   
		
			if (IsDialogMessage(hWndUserForm, &msg))
			{
				if (msg.message == WM_CHAR && msg.wParam == 5)
					PostMessage(msg.hwnd, WM_COMMAND, IDC_DYNEDIT, 0L);  
        		continue; 
        	}
        }
		if (hDynamicDialog)
		{   
		
			if (IsDialogMessage(hDynamicDialog, &msg))
			{
				if (msg.message == WM_CHAR && msg.wParam == 5)
					PostMessage(msg.hwnd, WM_COMMAND, IDM_CONTROL_M, 0L);  
        		continue; 
        	}
        }
       
/*	if (!WSAIsBlocking ())*/  
	if (msg.message == WM_KEYDOWN && SwitchWindow) 
	{   
		HWND	InFocus = GetParFocus();  
		RECT	Rect;
		
		if (!CurrentConfig)
			SetConfig (1);
		GetCursorPos (&CPoint); 
		GetWindowRect (InFocus,&Rect);
		if (!PtInRect (&Rect,CPoint))
		{
			ScreenToClient (CurView->hWnd,&CPoint);   
			HaltMapDisplay(FALSE,TRUE);
			SelectViewport (CPoint,TRUE,FALSE,FALSE);   
			if (InFocus != GetFocus())
				PostMessage(GetFocus(),msg.message, msg.wParam,msg.lParam);
		} 
			
	}  

//     	IsAccel = TranslateAccelerator(hWndMain,hAccelTable,&msg); 
//	 SetWindowText (hWndMain,"NewMSG");
	 hFocus =GetParFocus(); 
	 InAccel = TRUE;  
	 if ((!hFocus || hFocus == hWndMain || hFocus == hWndAltAccel) && (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN))
     	IsAccel = TranslateAccelerator(hWndMain,hAccelMain,&msg); 
     else if (HLTDlgWnd && hFocus == HLTDlgWnd)
     	IsAccel = TranslateAccelerator(HLTDlgWnd,hAccelTableHLT,&msg); 
     else
      	InAccel = IsAccel = FALSE; 
	 if (!IsAccel)
	 {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg); 
//	    Sleep (50);
//   		 SetWindowText (hWndMain,"NotACC");
     } 
     else
//   		 SetWindowText (hWndMain,"WasACC");
		ii=1; 
   }

 /* Do clean up before exiting from the application                     */
 //UnhookWindowsHookEx(Func);
 //UnhookWindowsHookEx(MHookFunc);
   FreeBigMem();
#if CHECKMEM
   GSSiGLOBALLOCCLOSE();
#endif
   UnhookWindowsHookEx(PrevFIHook);
   CwUnRegisterClasses(); 
{
#if ENABLETRACE
GSSiExitProg (437);
#endif
//	 exit:
 CoUninitialize();
#ifndef NDEBUG
	_CrtDumpMemoryLeaks();
#endif
return msg.wParam;
}
#if ENABLETRACE
}
#endif
} /*  End of WinMain                                                    */
/************************************************************************/
/*                                                                      */
/* Main Window Procedure                                                */
/*                                                                      */
/* This procedure provides service routines for the Windows events      */
/* (messages) that Windows sends to the window, as well as the user     */
/* initiated events (messages) that are generated when the user selects */
/* the action bar and pulldown menu controls or the corresponding       */
/* keyboard accelerators.                                               */
/*                                                                      */
/************************************************************************/

LONG FAR PASCAL WndProc(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
	if (isGMEdit)
		return WndProcGMEdit(hWnd,Message,wParam,lParam);
	else if (!inDisplayAllToobars)
		return WndProcGeoMaster(hWnd,Message,wParam,lParam);
	return 0;
}

LONG FAR PASCAL WndProcGeoMaster(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (438);
#endif
{
 HMENU      hMenu=0;            /* handle for the menu                 */
 HBITMAP    hBitmap=0;          /* handle for bitmaps                  */
 HDC        hDC;                /* handle for the display device       */
 PAINTSTRUCT ps;                /* holds PAINT information             */
 int        nRc=0;              /* retrn code                         */
 POINT		MousePoint;
 WORD wSize;
 DWORD dwLen;
 HDC	hPr, hMemoryDC;   
 RECT	Rect;
 RECT	WindowRect;
 POINT		Factors;
 DWORD		lCursorLoc;
 POINT		CursorLoc; 
 HBRUSH		BkBrush;  
 BOOL		RedisplayMenu;
 int		SDC, i, SaveDrive;
short	vpid;  
char	txt[32];
POINT ptCurrent;
HMENU hmenu;
LPVIEWPORT	SaveView, SaveView2; 
short	iview,l,ii, WantVP;
BOOL	rc;
static DLGPROC lpfnDEBUGINFOMsgProc;
time_t	Time;  
HANDLE	hSTR;
LPSTR	str;   
BOOL	ClearFullDisplay=FALSE;  
static	DWORD	LastMouselParam=0;

//FileDlgWndProc(hWnd, Message, wParam,lParam);

#if ENABLETRACE
SetLastMessage(Message,wParam);
#endif
/*  if (BackgroundTask && HaveWMCreate)
  {
  	char	mess[32];
  	sprintf (mess,"MES %x - %x",Message,wParam); 
  	GSSiTrace (mess);
  }*/
  if (Message == GF_UPDATE_FILE_RECEIVED)
  {
	  ProcessUpdateFile ((HANDLE)lParam);
	  	goto Return0;
  }
  if (Message == GSSI_GPSwnd)
  	GPSInputWnd = (HWND)lParam;
  if (Message == GF_CONNECT_PROCESS)
	  AddConnectedProcess ((HWND)wParam,lParam);
#ifndef	NDEBUG
  
  if (Message == WM_COMMAND && LOWORD (wParam) == IDM_DISPLAY_VEHICLES)
	  ii=1;
  switch (Message)
  {
  case WM_PAINT:
	  ii = 1;
  break; case WM_ERASEBKGND:
	  ii = 1;
  break; case WM_TIMER:
	  ii=1;
  break; case WM_SIZE:
	  ii=1;
  break; case WM_LBUTTONDBLCLK:
  {
	  ii = IsPointOnTouchScreen(hWnd,POINTStoPOINT(MAKEPOINTS(lParam)));

  }
  break; case WM_F1DOWN:
  	ii=1;
  break; case WM_SHOWWINDOW:
	ii=1;
  break; case  WM_IME_SETCONTEXT:
	ii = 1;
  break; case WM_MOUSEMOVE:
	 ii=1;//ClearVehicleInfoRect ();
  break; case WM_CLOSE:
 	ii=1;
  break; case WM_LBUTTONUP:
 	ii=1;
  break; case WM_RBUTTONUP:
 	ii=1;
  break; case WM_LBUTTONDOWN:
 	ii=1;
  break; case GF_CLOSE:
 	ii=1;
  break; case WM_COMMAND:
 	ii=1;
  break; case GF_EXECUTE:
 	ii=1;
  break; case WM_NOTIFY:
  {
	LPNMHDR pNH = (LPNMHDR)lParam;

	switch (((LPNMHDR)lParam)->code)
    {
    /*case TCN_SELCHANGE:
        { 
            int iPage = TabCtrl_GetCurSel(g_hwndTab); 
			LRESULT result;

			 sprintf (g_achTemp,"Tab %i",iPage);
             result = SendMessage(g_hwndDisplay, WM_SETTEXT, 0,
                (LPARAM) g_achTemp); 
		    return TRUE;
        } */
    }

 	ii=1;
  }

 break; case GF_DISPLAY_ALL_ORTHOS:
 	ii=1;
	 }
#endif
 if (ProcessDataDisplayInput (hWnd,Message, wParam,lParam))
	goto Return0;
 if (ProcessCloseIcon (hWnd,Message, wParam,lParam))
	goto Return0;
if (Message == WM_SETCURSOR)
{
    	extern	BOOL ddbug;
    	 if (!idTimer)
    	 {
	    	 hCursor = VPCursor (hWnd);
			 if (!hCursor)
				goto ReturnDefault;
		 }
         //if (GetCursor () != hCursor)  
         	GSSiSetCursor (hCursor);
 		 goto Return0;

} 

if (InSmoothZoom && (Message == WM_LBUTTONDOWN || Message == WM_RBUTTONDOWN || Message == WM_LBUTTONUP || Message == WM_RBUTTONUP))
 {  
	 HaltMapDisplay (FALSE,TRUE);
 	 SetViewport (MultiZoomVP);
 	 if (ProcessGraphicsFunction (hWnd,Message, wParam,lParam))
		goto Return0;
	 else
 		goto ReturnDefault;

 }

if (Message == WM_CHAR && wParam == '\b' && !(lParam & KF_UP) && CursorIsLocked) 
{
	UnlockCursor ();
	goto Return0;
}

if (Message == WM_CHAR && wParam == 26) //CNTL/Z
	goto Return0;

if (Message == GF_MAPSERVER_REQUEST)
{
	HFILE Fid;
	OFSTRUCTGM OFStruct;
	 //MessageBox(hWnd, "Got request", "", MB_OK);

	MapserverRequestID = LOWORD(lParam);
	MapserverVPID = HIWORD(lParam);
	MapServerCalledFromWnd = (HWND)wParam;
	Fid = OpenFileGM(MapserverFile, &OFStruct, OF_READ);
	if (Fid != HFILE_ERROR)
	{
		LPSTR cmd = (LPSTR)malloc(4096);
		int ln = _llseek(Fid, 0, 2);

		_llseek(Fid, 0, 0);
		_lread(Fid, cmd,ln);
		_lclose(Fid);
		if (dbug)
			MessageBox(hWnd, cmd, "", MB_OK);
		ProcessText (cmd);
		free(cmd);
	}

	{
#if ENABLETRACE
		GSSiExitProg(438);
#endif
		return TRUE;
	}
}
if (Message == GF_PROCESSTCPCMD)
{
	if (lParam)
	{
		LPSTR	pCmd = GlobalLock ((HANDLE)lParam);

		ProcessTCPCmd ((SOCKET)wParam,pCmd);
		GSSiGlobUlFree (&(HANDLE)lParam);
	}
	else
		ProcessTCPData ((SOCKET)wParam);

{
#if ENABLETRACE
GSSiExitProg (438);
#endif
	return TRUE; 
}
}
if (Message == GF_TCPIPMESSAGE)
{   
	UINT	event = LOWORD (lParam);
	UINT	err = HIWORD (lParam);
	SOCKET	sock = wParam;
	BOOL	IDP = InDisplayProcessing;      
	
	if (HaveBlockingWindow)
		InDisplayProcessing=TRUE;	 
	if (err)
	{
		if (InServerMode && (err == WSAECONNRESET || err == WSAECONNABORTED))
		{
			if (!RestartSocket (sock))
				CloseTCPIPSocket (sock,TRUE);
		}
		else
			ProcessSocketError (sock,err,"Processing GF_TCPIPMESSAGE");
	}
	else
		switch (event)
		{
			case FD_READ:
				ProcessTCPData (sock);
				break;
			case FD_CLOSE:
			{
				if (RestartSocket (sock))
					break;
				CloseTCPIPSocket (sock,TRUE);
			}
				break;
			case FD_ACCEPT: 
				AcceptTCPConnection (hWnd,sock);
				break;
		}   
	InDisplayProcessing = IDP;
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
	return TRUE; 
}
}

if (Message == WM_COMMNOTIFY) 
{   
	BOOL	IDP=InDisplayProcessing;
	
	if (HaveBlockingWindow)
		InDisplayProcessing=TRUE;	
//	ProcessCOMMNotification( hWnd, (WORD) wParam, (LONG) lParam ); 
	InDisplayProcessing = IDP;
	goto ReturnDefault;
}

if (HaveBlockingWindow && Message == WM_PAINT)
{
	hDC = GetDC (hWndMain);
					
	GetWindowRect (hWndMain,&Rect);
    SelectClipRgn (hDC,0);
	RestoreScreen (hDC,hFullWindowBitMap,FullWindowBitMapRect);	     
	ReleaseDC (hWndMain,hDC);  
	goto ReturnDefault;
}

if ((HaveBlockingWindow && Message != GF_CLOSE) || Message == WM_CANCELMODE)
	goto ReturnDefault;
//if (Message == WM_PAINT && hRestoreMainWindow) 

 
if (Message == WM_LBUTTONDOWN)
	IgnoreLbutton = FALSE;
 if (Printing || Processing)
	goto ReturnDefault;
#ifndef	NDEBUG
 if (Message == WM_COMMAND)
 	ii=1; 
 if (Message == WM_SYSCOMMAND)
	ii=1;
 if (Message == WM_KEYDOWN)
 	ii=1;
 if (Message == WM_CHAR)
 	ii=1;
#endif
 if (Message == GF_CLOSE)
 {  
 	if (!CurrentConfig)
 		ii=1;
 	if (lParam)
 		SetViewport ((short)lParam); 
 	if (wParam == GF_DIGITIZE_POLYLINE)
 		ii=1;
 }
 if (Message == WM_TIMER && wParam == 3)
 {   WSACancelBlockingCall ();
 	 GSSiMsgBox( GetFocus(), "Time-out waiting for response","Fatal Error", MB_OK,0);
//	 CloseUM(TRUE);
 	 BlowOut(0,0);
 }

if (Message == WM_MOUSEMOVE)
{	if (IgnoreMouseMove)
	{
		IgnoreMouseMove=FALSE;              		
		goto ReturnDefault;
	}
	else if (lParam == LastMouselParam) 
		goto ReturnDefault;
	if (GetFocus() != hWndMain)
	{
		if (!WindowIsCovered(hWndMain, 2))
			SetFocus(hWndMain);
	}
	LastMouselParam = lParam; 
}
if (Message == WM_CLOSE)
	Processing = FALSE;
if (Processing || InFileWait)
	goto ReturnDefault;
if (WSAIsBlocking ())
	goto ReturnDefault;
if (Message == WM_LBUTTONUP )
{
    if (IgnoreLbutton)
		goto ReturnDefault;
    if (DebugWait)
	{
		DebugWait = FALSE;
		goto ReturnDefault;
	}
    IgnoreLbutton = TRUE;
}

if (Message == WM_LBUTTONDOWN )
{
	IgnoreLbutton = FALSE;
	IgnoreSelectVP = FALSE; 
	CurrentLBUTDOWNLoc=POINTStoPOINT(MAKEPOINTS(lParam));
	if (ProcessGraphicsFunction(hWnd, Message, wParam, lParam))
		goto Return0;
	HaltMapDisplay(TRUE,TRUE);
}
if (DebugWait)
	goto ReturnDefault;

if (ProcessInfoboxMacro (hWnd,Message, wParam,lParam))
	goto Return0;

if (!DisableMarginPan && 
	(Message == WM_LBUTTONDOWN || Message == WM_RBUTTONDOWN || Message == WM_LBUTTONUP))   
	if (!SelectViewport (POINTStoPOINT(MAKEPOINTS(lParam)),FALSE,wParam & MK_DIGITIZER_BUTTON,FALSE))
		goto S10; 
//if (HaveSeg) 
{
	if (ProcessPickBoxes(hWnd, Message, wParam, lParam))
		goto Return0;
	if (ProcessPassiveFunctions (hWnd,Message, wParam,lParam))
		goto Return0;
	if (Message == WM_CHAR)
	{
		short	vpid;
		POINT	CPoint; 
					  
		GetCursorPos (&CPoint); 
		ScreenToClient (hWnd,&CPoint);
	 	vpid = SelectViewport (CPoint,FALSE,FALSE,FALSE);
	 	if (!vpid)
	 		goto KeepGoing;
		if (wParam == 6)  //CNTL/F  
		{
		 	vpid = SelectViewport (CPoint,FALSE,FALSE,FALSE); 
		 	MakeVPFullScreen (vpid,2);
			goto Return0;
		}
	 	if (!pViewports[vpid-1]->hProfileRoute)
	 		goto KeepGoing;
		if (!pViewports[vpid-1]->pTheme ||
			 pViewports[vpid-1]->pTheme->ID != GF_PROFILE_THEME)
			goto KeepGoing;
		CurTheme = pViewports[vpid-1]->pTheme;
		switch (wParam)
		{   
			case 'v':
			case 'V':
            {
                 DLGPROC lpfnVISIBLEMsgProc;  
                 
                 Pickability = FALSE;  

                 lpfnVISIBLEMsgProc = MakeProcInstance((DLGPROC)VISIBLEMsgProc, hInst);
                 nRc = DialogBox(hInst, (LPSTR)VisDialog[VisListOpt], hWnd, lpfnVISIBLEMsgProc);
                 FreeProcInstance(lpfnVISIBLEMsgProc);
                 if (nRc)
                 	RedisplayViewport(FALSE,FALSE); 

            }
			 	break; 
			case 'c':
			case 'C':
		 		if (pViewports[vpid-1]->ProfileInCrossSection)
		 			goto KeepGoing;
				pViewports[vpid-1]->ProfileInCrossSection = TRUE;  
				DisplayProfileThemeLegend(4);
			 	break; 
			case 'p':
			case 'P':
		 		if (!pViewports[vpid-1]->ProfileInCrossSection)
			 		goto KeepGoing;
				pViewports[vpid-1]->ProfileInCrossSection = FALSE;  
				DisplayProfileThemeLegend(4);
			 	break; 
			case '+':
			case '-':
		 		if (!pViewports[vpid-1]->ProfileInCrossSection)
			 		goto KeepGoing;  
			 	if (wParam == '+')
			 		CreateNextCrossSection (1);
			 	else
			 		CreateNextCrossSection (-1);
				DisplayProfileThemeLegend(4);
				break;
			case 'S':
			case 's':  
			{
				LPVIEWPORT	SaveVP=CurView;
				
				CurView = pViewports[vpid-1];
				if (ProfileAndCrossSectionSettings (FALSE))
					DisplayProfileThemeLegend(4); 
				CurView = SaveVP;
			}
				break;
			case 'h':
			case 'H':
				MessageBox(0, "The following keys can be used to invoke options in the Profile Viewport:\n\n\tS\tSettings Screen\n\tC\tCross Section Mode\n\tP\tProfile Mode\n\t+\tNext Cross Section\n\t-\tPrevious Cross Section\n\tV\tVisibility of Data in Profile or Cross Section Mode\n\tH\tDisplay this Help Screen", "Profile Option Keys", MB_OK);
				break;
			case 'z':
			case 'Z':
			{
				LPVIEWPORT	SaveVP = CurView;

				CurView = pViewports[vpid - 1];
				ZoomToProfile(CurView);
				CurView = SaveVP;
			}

				break;
			default:  
				goto KeepGoing;
		}
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
			return 0; 
}
	} 
KeepGoing:
	if (ConfigLoaded && CurView && *pNumViewports)
	{
//		if (CurView->StartupFunction && !CurView->FunStackHandle && !CurView->CurrentFunction)
		if (CurView->StartupFunction && !CurView->CurrentFunction && !CurView->FunStackHandle)
	    {                               
	    	ResetFunStack (TRUE);
	       	AddLBUTTON = TRUE;
			AddGraphicsFunction (0, CurView->StartupFunction,0);        
		}
	}
	SetViewportForCommand (Message, wParam);   
	if (ProcessGraphicsFunction (hWnd,Message, wParam,lParam))
		goto Return0;
}
if (Message == GF_EXECUTE)
	goto Return0;
S10:
if (ProcessDocument (hWnd,Message, wParam,lParam))
	goto Return0;

 switch (Message)
   { 
        	
    case WM_MBUTTONDOWN:
		{
			if (WheelZoom (-1,0,1))
				goto ReturnDefault;
			CursorLoc = POINTStoPOINT(MAKEPOINTS(lParam));
	 		vpid = SelectViewport (CursorLoc,TRUE,FALSE,FALSE);
			if (VPIsMap(vpid))
			{
				WheelZoom (INT_MAX,0,1);
				if (!GetGlobalBVal2("[%DISABLEWHEEL]", FALSE))
				{
					itoa(GF_SLIDE_SCREEN, txt, 10);
					AddGraphicsCmd(hWndMain, txt, FALSE, 0);
					PostMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
				}
			}
			else
			{
				WheelZoom (0,0,1);
				goto ReturnDefault;
			}
		}
        break;
         
	case WM_MOUSEWHEEL:
	{
		int		fwKeys = LOWORD(wParam);    // key flags
		short	zDelta = (short) HIWORD(wParam);    // wheel rotation
		HWND	hVMWnd;

		CursorLoc = POINTStoPOINT(MAKEPOINTS(lParam));
		if ((hVMWnd = CursorInVisMenuWnd (CursorLoc)))
			PostMessage (hVMWnd,WM_MOUSEWHEEL,wParam,lParam);
		else
		{

	 		vpid = SelectViewport (CursorLoc,TRUE,FALSE,FALSE);
			if (VPIsMap(vpid))
			{ 
				DPOINT	Point = ScreenPtToBasePt (CursorLoc);

				WheelZoom (zDelta,0,1);
			}
			else
			{
				WheelZoom (0,0,1);
			}
		}
		goto ReturnDefault;
	}
    case WM_USER:
		PostMessage(hWndMain,MM_MCINOTIFY, 0, 0L); 
		break;  
    
    case WM_F1DOWN: 
    {
         //  wParam; contains the handle to the dialog control thats active
    	 HWND hw;  
    	 
		 hSTR=GSSiGlobAlloc (   5,GMEM_MOVEABLE,1024);
		 str=GlobalLock (hSTR); 
    	 
//         hw = GetDlgCtrlID(wParam); 
         setDoPaint( TRUE);    
         InHelp=TRUE;
         if (GFMenuWnd)
         {   
         	short	Choice;
         	LPSTR	lpTAB, lpBAR; 
         	
			 Choice=SendDlgItemMessage(GFMenuWnd,ACTIVE_FUN_LB,
								    LB_GETCURSEL,0,0); 
			 SendDlgItemMessage(GFMenuWnd,ACTIVE_FUN_LB,LB_GETTEXT,
							  Choice,(DWORD)str); 
			 lpTAB = _fstrchr (str,'\t');
			 if (lpTAB)
			 {
			 	lpBAR = _fstrchr (lpTAB,'|');
			 	if (lpBAR)
			 	{    
			 		lpTAB++;
			 		*lpBAR = 0;  
			 		DisplayHelp (0,lpTAB);
			 	}
			 } 
         } 
         
         else   
		 {   
			LoadString(ghInst, HelpID, CurHelpTopic,256);
			DisplayHTMLHelp(0,CurHelpTopic);   
		 } 
		 GSSiGlobUlFree (&hSTR);
         InHelp=FALSE;
    }
         break;  
    
	case GSSI_GMEDITCOMPLETE:
		GMEditReturn();
		break;

	case GSSI_GPSwnd:
    	 GPSInputWnd = (HWND)lParam;
    	 break;

    case GSSI_GPSX: 
    	 GPSLoc.x = ((double)lParam)/1000000L;
    	 GPSXid = wParam;
    	 break;

    case GSSI_GPSY:
    	 GPSLoc.y = ((double)lParam)/1000000L;
		 if (SetGPSOffset)
		 {
			 SetGPSOffset = FALSE;
			 GPSOffset.x = GPSLoc.x - GPSOffset.x;
			 GPSOffset.y = GPSLoc.y - GPSOffset.y;
		 }
 		 GPSLoc.x -= GPSOffset.x;
 		 GPSLoc.y -= GPSOffset.y;
		 if (GPSPassTo)
		 {
			 long ix,iy;

			 ix = GPSLoc.x * 10000000;
			 iy = GPSLoc.y * 10000000;
			 PostMessage (GPSPassTo,GSSI_GPS,ix,iy);
		 }
   		 if (wParam == GPSXid) 
    	 {
 			if (*LogGPSInputFile)
			{
				int	itime = time(0);
				char	str[80];

				sprintf (str,"%i\t%f\t%f",itime,GPSLoc.x,GPSLoc.y);
				AppendFile (LogGPSInputFile,str);
			}
   	 		ConvertCoord (&GPSLoc,2,1);
			SetVehicleLoc ("GPS",GPSLoc,0,0,0,TRUE,TRUE,0,0);
		 } 
    	 break;
    	 
    case GSSI_ADDGF:
		 SetViewport((short)lParam);
		 if (CurView->DisplayInParent && CurView->Parent)            	
			 SetViewport(CurView->Parent);
		 if (wParam)
		 	AddGraphicsFunction (CurView->hWnd,wParam,0);
		 else if (hAddGraphicsFun2) 
	     {
	    	LPSTR	pCmd=GlobalLock (hAddGraphicsFun2); 
		    short	lcmd = _fstrlen (pCmd);
		    HANDLE	hCmd2 = GSSiGlobAlloc (   6,GMEM_MOVEABLE,lcmd+1);
		    LPSTR	pCmd2 = GlobalLock (hCmd2);
		    
		    _fstrcpy (pCmd2,pCmd);            	
	    	GSSiGlobUlFree (&hAddGraphicsFun2);
	    	HaveCurrentLBUTTON=FALSE;  
	    	SetViewport (AddGraphicsFunVP);
	    	AddGraphicsCmd (hWndMain,pCmd2,TRUE,0); 
	    	GSSiGlobUlFree (&hCmd2);
	     } 
         break; 
         
    case GF_ADD_EMEBEDDED_CMD:
		 SetViewport((short)wParam);            	
		 if (hEmbeddedGFCommand)
	     {
	    	LPSTR	pCmd=GlobalLock (hEmbeddedGFCommand); 
		                	
	    	HaveCurrentLBUTTON=FALSE; 
	    	AddGraphicsCmd (hWndMain,pCmd,TRUE,0); 
	    	GSSiGlobUlFree (&hEmbeddedGFCommand);
	     } 
         break;
   
	case GF_PROCESS_CONNECTED_CMD:
		ProcessConnectedCommand(wParam);
		break;

	case PROCESS_COMMAND_MACRO:
		ProcessText(CommandMacro);
		break;

	case GF_END_PROCESS:
  	    PostMessage(hWndMain, WM_COMMAND, IDM_EXIT, 0L);//allows imediate processing to terminate 
		break;

    case WM_COMMAND:
         /* The Windows messages for action bar and pulldown menu items */
         /* are processed here.                                         */ 
         
     	 if (!CurrentConfig)
    	 	SetConfig (1);
        if (!DisableHalt)
         	SetContinueProcessing ( TRUE); 
         WantVP = LOWORD(lParam);
         if (WantVP > 0 && WantVP <= *pNumViewports) 
         	SetViewport(WantVP);
         else if (*pNumViewports && LOWORD(wParam) < 60000) 
         {  
         	POINT CPoint;
         	
         	GetCursorPos (&CPoint);
			ScreenToClient (hWnd,&CPoint);   
//changed !IsAccel to IsAccel 2/27/00   
			IsAccel = (HIWORD(wParam) == 1);
         	if (IsAccel)
         	{
         		if (!SelectViewport (CPoint,TRUE,FALSE,FALSE)) 
		    		SetViewport(*pCommandViewport);
		    }
		    else
				SetViewport(*pCommandViewport);
         }  
         if (LOWORD(wParam) != IDM_Z_WINDOW && LOWORD(wParam) != IDM_DISPLAY_VEHICLES)
		 	CancelWindowZoom();

//         HaltMapDisplay(FALSE);
//         setDoPaint( FALSE); 
         if (LOWORD(wParam) >= 64000) /* pickmacro*/
         {
         	int	item, irec;
         	
 //        	item = (wParam - 64000) / 256;
 //        	irec = (wParam - 64000) % 256; 
			item = LOWORD(wParam)-64000;
			irec = HIWORD(wParam);
         	ProcessPickedItems (hWnd,item, irec);

         }
         else if (LOWORD(wParam) == 63800) /* GRAPHICS_FUNCTION_THEME selection */
         	ExecuteGFFromTheme ();
         else if (LOWORD(wParam) > 63800) /* GRAPHICS_FUNCTION_THEME Ap List */
         {  
         	short WantAp = LOWORD(wParam) - 63800; 
         	
         	LoadGFFunctionList (WantAp); 
         }
         else if (LOWORD(wParam) > 60000) /* Viewports menu used for activate/deactivate */
         {  
         	short WantVP = LOWORD(wParam) - 60000;
         	
         	SaveView = CurView;  
         	if (SetViewport (WantVP))
         	{
	         	if (CurViewActive())
	         	{
	         	   CheckMenuItem(GetMenu(hWnd), LOWORD(wParam), MF_BYCOMMAND | MF_UNCHECKED);
	         	   CurView->Active = FALSE;
	         	   for (iview = 0;iview<*pNumViewports; iview++)
	         	   {          
	         	   	   SetCurView ( pViewports[iview]);         
	         	   	   if (CurViewActive() /*&& !FileMode*/)
	         	   	   {
						   GSSiDeleteObject(&CurView->hRgn);
			         	   CurView->hRgn = CreateVPRgn(FALSE,FALSE);
						   SelectClipRgn (CurView->hDC,CurView->hRgn);
						   GSSiDeleteObject(&CurView->hRgn); 
					   }
	               }
				   RedisplayWindow();   
	         	}
	         	else
	         	{
	         	   CheckMenuItem(GetMenu(hWnd), LOWORD(wParam), MF_BYCOMMAND | MF_CHECKED);
	         	   CurView->Active = TRUE; 
		           SaveView2 = CurView;
			       CurView->CurZoomAreaRef = 0;
	         	   RedisplayViewport (TRUE,FALSE);  
	         	   SetCurView ( SaveView2);
	         	   if (CurView->Type == 7)
	         	   {
	         	   		SetCurView ( pViewports[CurView->ZoomTarget-1]);
	      	   		    SetBounds (hWnd,0);
	         	   }
	         	}  
	        }
            setDoPaint( TRUE);
	       	SetCurView ( SaveView);

         } 
         else if (LOWORD(wParam) >= 59500) /* Toolbar commands */
         {  
         	int	CmdID;
         	
         	CmdID = LOWORD(wParam) - 59500;  
         	if (!CmdID) goto DisplayParcel; 
         	phWhichCmdList = &hToolCmd;
         	ExecuteUserCmd (CmdID);   
         	GSSiGlobFree (&hToolCmd);
         	setDoPaint( TRUE);
         }
         else if (LOWORD(wParam) >= 58000) /* User commands */
         {  
         	int	CmdID;
         	CmdID = LOWORD(wParam) - 58000;  
         	if (!CmdID)
         		goto DisplayParcel; 
         	phWhichCmdList = &hUserCmd;
         	ExecuteUserCmd (CmdID);
			ReloadMainMenu ();
         	setDoPaint( TRUE);
         }
 		 else switch(LOWORD (wParam))
          {
            case IDM_VIEW:
            {
				 hSTR=GSSiGlobAlloc (   7,GMEM_MOVEABLE,1024);
				 str=GlobalLock (hSTR); 
            	 
       			 HaltMapDisplay(FALSE,TRUE);
                 setDoPaint( FALSE);
				 ButtonFuncOpt=0;
				 strcpy (OFTitle,title4);
		         if (GetFileName3 (hWnd,str,IDS_FILTERPLT,IDS_FILEPLT))   
                 {
					NewMap();
					_fstrupr (str);  
                 	if (_fstrstr (str,".BMP"))
                 	{   
					    _fstrcpy (FullBM,str);
						ShowFullBM(FALSE,0,0);
                 	}
                 	else if (_fstrstr (str,".GMC"))
                 	{    
				        UnallocateConfig ();
                		_fstrcpy (CfgName,str);
						RedisplayWindow();   
                 	}
                 	else
                 	{   
			    		SetGlobalValue("%PLOT",str);
						RedisplayWindow();   
                 	}
                  }

				  setDoPaint( TRUE);
				  GSSiGlobUlFree (&hSTR);

            }
                 break;  
            
            case IDM_F1DOWN:
		         PostMessage(hWndMain, WM_F1DOWN,0,0);
            	 break;
            	 
            case IDM_TRANSFERLICENSE:    
            	 //SetTransferCode (expnot);
            	 break;
            
			case IDM_EDITLASTTXT:
				 EditLastTextFile ();
				 break;

			case IDM_EDITLASTMENU:
				 EditLastMenuFile ();
				 break;

            case IDM_NEWSYM:
                 {
                  DLGPROC lpfnNEWSYMBOLMsgProc;

                  lpfnNEWSYMBOLMsgProc = MakeProcInstance((DLGPROC)NEWSYMBOLMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"NEWSYMBOL", hWnd, lpfnNEWSYMBOLMsgProc);
                  FreeProcInstance(lpfnNEWSYMBOLMsgProc);

                 }
            	 break;
            	 
            case IDM_EDITSYM:
                 {
                  DLGPROC lpfnEDITSYMBOLMsgProc;

                  lpfnEDITSYMBOLMsgProc = MakeProcInstance((DLGPROC)EDITSYMBOLMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"EDITSYMBOL", hWnd, lpfnEDITSYMBOLMsgProc);
                  FreeProcInstance(lpfnEDITSYMBOLMsgProc);

                 }
            	 break;
            	 
            case IDM_ABOUT:
                 {
                  DLGPROC lpfnABOUTMsgProc;

                  lpfnABOUTMsgProc = MakeProcInstance((DLGPROC)ABOUTMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"ABOUT", hWnd, lpfnABOUTMsgProc);
                  FreeProcInstance(lpfnABOUTMsgProc);

                 }
            	 break;
            	 
            case IDM_REORG_BTREE:
                 {
                  DLGPROC lpfnBTREE_REORGMsgProc;

                  lpfnBTREE_REORGMsgProc = MakeProcInstance((DLGPROC)BTREE_REORGMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"BTREE_REORG", hWnd, lpfnBTREE_REORGMsgProc);
                  FreeProcInstance(lpfnBTREE_REORGMsgProc);

                 }
            	 break;
            	 
            case IDM_CITY_EXTRACT:
//            	 CreateCityExtract();
            	 break;  
            	 
            case IDM_SETDATERANGE:
                 {
                  DLGPROC lpfnDATELIMITSMsgProc;

                  lpfnDATELIMITSMsgProc = MakeProcInstance((DLGPROC)DATELIMITSMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"DATELIMITS", hWnd, lpfnDATELIMITSMsgProc);
                  FreeProcInstance(lpfnDATELIMITSMsgProc);

                 }
            	 break;

			case IDM_DISPLAY_DATADIR:
				 ProcessText ("$WEB([%DL])");
				 break;
            	 
            case IDM_SAVE_CONFIG:
				 DoSaveConfig (hWnd,FALSE);
            	 break;  
            	 
/*            case IDM_VAN_SMALL: 
            	 VehLen = 20;
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_SMALL, MF_BYCOMMAND | MF_CHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_MEDIUM, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_LARGE, MF_BYCOMMAND | MF_UNCHECKED);
                 break;
            	 
            case IDM_VAN_MEDIUM: 
            	 VehLen = 20 * 4;
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_SMALL, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_MEDIUM, MF_BYCOMMAND | MF_CHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_LARGE, MF_BYCOMMAND | MF_UNCHECKED);
                 break;
            	 
            case IDM_VAN_LARGE: 
            	 VehLen = 20 * 8;
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_SMALL, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_MEDIUM, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_VAN_LARGE, MF_BYCOMMAND | MF_CHECKED);
                 break; */
                
            case IDM_PLOTVIEW:  
            	if (!PrinterHeight)
            	{   
            		GSSiMsgBox (hWnd,"You must use the Print Setup command before using PlotView","",MB_ICONEXCLAMATION,0);
            		break;
            	}
             	if (InPlotView)
             	{
             		InPlotView = 0; 
             		ShowScrollBar(hWnd, SB_BOTH,FALSE);
					CheckMenuItem(GetMenu(hWnd), IDM_PLOTVIEW, MF_BYCOMMAND | MF_UNCHECKED);
             	}
             	else if (GetFormatDimensions (&FormatWidth,&FormatHeight))
             	{
             		InPlotView = 1;
             		ShowScrollBar(hWnd, SB_BOTH,TRUE);
					CheckMenuItem(GetMenu(hWnd), IDM_PLOTVIEW, MF_BYCOMMAND | MF_CHECKED);
             	}
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);  
                break;
                
            case IDM_RUNMACRO:
            	ProcessText("$MACRO($GETPATH(R,.txt,Select Macro File,LASTMACRO,[%DL]macros\\))");
             	break;

/*            {
            	DPOINT PT1,PT2,PT3,PT4;
            	double	lat=45,lon=-93;
            	
				PT1 = NewLatLong(lat,lon,5280,0);
				PT2 = NewLatLong(lat,lon,5280,PY);
				PT3 = NewLatLong(lat,lon,5280,PY/2);
				PT4 = NewLatLong(lat,lon,5280,3*PY/2);
            }*/	
//            	 LoadCitiesTable ();
                 
/*                 {   
                 	HFILE FidI;
                 	char	str[20];  
                 	OFSTRUCTGM	OFStruct;
                 	
                 	FidI = GSSiOpenFile ("creatint.txt",&OFStruct,OF_READ);
                 	if (FidI == HFILE_ERROR) break;
                 	while (fgetstring (str,16,FidI))
                 	{
	            	 	SetGlobalValue ("STATE",str);
	            	 	CreateIntersectionFile (TRUE); 
	            	} 
	            	GSSiClose (FidI);
	            	SetWindowText (hWnd,"Intersection Creation Complete"); 
            	 }   
            	 
            	 break; */          
            	 
            case IDM_GFKEY:
				SetViewport(*pCommandViewport);            	
	            IsGFunctionKey ((WORD)lParam,TRUE);
                break;
            	 
            case IDM_OPEN_DIGITIZER:
            	if (!OpenDigConnection(hWnd ))
            		break;
				GMEnableMenuItem(hWndMain, IDM_CLOSE_DIGITIZER, MF_BYCOMMAND | MF_ENABLED);
				GMEnableMenuItem(hWndMain, IDM_OPEN_DIGITIZER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            	break;
            	 
            case IDM_CLOSE_DIGITIZER:
            	CloseDigConnection ();
				GMEnableMenuItem(hWndMain, IDM_OPEN_DIGITIZER, MF_BYCOMMAND | MF_ENABLED);
				GMEnableMenuItem(hWndMain, IDM_CLOSE_DIGITIZER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            	break;   
            	
            case IDM_DIG_CONTROL:
            	OpenDigControlDialog (hWnd);
            	break;   
            	
            case IDM_NEWPOINTSET: 
            	SetNewElementValues (hWnd,1);
            	break;
            	 
            case IDM_NEWLINESET:   
            	SetNewElementValues (hWnd,2);
            	break;
            	 
            case IDM_NEWAREASET: 
            	SetNewElementValues (hWnd,3);
            	break;
            	 
            case IDM_SHOW_DELETES:
				 if (GWCheckMenuItem(hWnd, LOWORD(wParam)))
				 	ShowDeletedOpt=TRUE;
				 else
				 	ShowDeletedOpt=FALSE;
                 PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L);
            	 break;

            case IDM_SHOW_MILEPOINTS:
				 GWCheckMenuItem(hWnd, LOWORD(wParam));
                 PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L);
            	 break;

            case IDM_STOP_ATINT:
				 StopAtInt = GWCheckMenuItem(hWnd, LOWORD(wParam));
				 break;
				 
            case IDM_DISPLAY_HLT_PATTERN:
				 DisplayHLTPattern = GWCheckMenuItem(hWnd, LOWORD(wParam));
				 break;
				 
            case IDM_DISPLAY_NONHLT:
				 DisplayOnlyNonHLT = GWCheckMenuItem(hWnd, LOWORD(wParam));
				 CheckMenuItem(GetMenu(hWnd), IDM_DISPLAY_HLT, MF_BYCOMMAND | MF_UNCHECKED); 
				 DisplayOnlyHLT = FALSE;
				 break;
				 
            case IDM_DISPLAY_HLT:
				 DisplayOnlyHLT = GWCheckMenuItem(hWnd, LOWORD(wParam));
				 CheckMenuItem(GetMenu(hWnd), IDM_DISPLAY_NONHLT, MF_BYCOMMAND | MF_UNCHECKED);
				 DisplayOnlyNonHLT = FALSE;
				 break;
				 
            case IDM_COMMAND_INPUT:  
            	 Command = GlobalLock (hCommand); 
			   	 if (GetTextString (hWnd,Command,1024,"Enter the command",0,0,0,TRUE,TRUE))
			   	 	ExecuteCommandString (Command);
			   	 GlobalUnlock (hCommand);
            	 break;
            
            case IDM_EXPAND_INPUT:
            	 
            	 Command = GlobalLock (hCommand); 
			   	 if (GetTextString (hWnd,Command,1024,"Input",0,0,0,TRUE,TRUE)) 
			   	 {
		           	LPSTR mess; 
		           	HANDLE	handle; 
		           	LPSTR	pExpanded;
		           	
		           	handle = GSSiGlobAlloc (   8,GMEM_MOVEABLE,4096);
		           	pExpanded = GlobalLock (handle); 
		           	mess = pExpanded+3064;
		           	sprintf (mess,"Expansion of %s",Command); 
		           	_fstrcpy (pExpanded,Command);
			   	 	ExpandText (pExpanded);    
			   	 	GSSiMsgBox (GetFocus(),pExpanded,mess,0,0);
			   	 	GSSiGlobUlFree (&handle);
			   	 }
			   	 GlobalUnlock (hCommand);
            	 break; 
            
            case IDM_NET_MARKER_CHECK:
				 FindNetSegWOEndMarker (15); 
				 break;
				
			case IDM_NET_CREATE_INT_MARKERS:
				 CreateIntMarkers ();
				 break;
            	
            case IDM_CREATE_SEGMAX_INDEX:
       	 		 BuildSegMaxIndex (hWnd);
                 break;
                          
            case IDM_DUMP_INT_TO_TXT:
            	 DumpIntToTXT ();
            	 break;
            	 
            case IDM_DUMP_INTNAME_TO_TXT:
            	 DumpIntersectionStreets ("intnames.txt",TRUE);
            	 DumpIntersectionStreets ("intnames_all.txt",FALSE);
            	 break;
            	 
            case IDM_CREATE_ADDLOC:
            {
                  DLGPROC	lpfnADDLOC_CREATEMsgProc; 

                  lpfnADDLOC_CREATEMsgProc = MakeProcInstance((DLGPROC)ADDLOC_CREATEMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"ADDLOC_CREATE", hWnd, lpfnADDLOC_CREATEMsgProc);
                  FreeProcInstance(lpfnADDLOC_CREATEMsgProc);
            }
            	 break;       
            	 
            case IDM_RELOAD_STND_TABLES: 
            {
                  DLGPROC	lpfnABVEDITMsgProc; 

                  lpfnABVEDITMsgProc = MakeProcInstance((DLGPROC)ABVEDITMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"ABVEDIT", hWnd, lpfnABVEDITMsgProc);
                  FreeProcInstance(lpfnABVEDITMsgProc);
            }
            	 break; 
            
            case IDM_DUMPUSERADD:
            	 DumpUserAdd ();
            	 break;
            	 	       
            case IDM_UNLOAD_STREET_NAMES:
            	 UnloadStreets (hWnd);
            	 break;   
            	 
            case IDM_UNLOADMUNICS:
            	 DumpMunicNameTable ("c:\\munics.txt");
            	 break;
            
            case IDM_BUILDSTREETPOLYS:
				 SaveStreetPolys ();
            	 break;
            	 
            case IDM_RELOAD_STREET_NAMES:
            	 ReloadStreets (hWnd);
	           	 break;
            	 
            case IDM_FIND_NONNET_STREETS:
            	 FindNonNetworkedStreets ();
            	 break;
            	 
            case IDM_REMOVE_NONNET_STREETS:
            	 RemoveNonNetworkedStreets ();
            	 break;
            	 
            case IDM_ADDLOC_FROMINT: 
            {
                  DLGPROC	lpfnADDLOC_FROMINTMsgProc; 

                  lpfnADDLOC_FROMINTMsgProc = MakeProcInstance((DLGPROC)ADDLOC_FROMINTMsgProc, hInst);
                  CreateDialog(hInst, (LPSTR)"ADDLOC_FROMINT", hWnd, lpfnADDLOC_FROMINTMsgProc);
//                  nRc = DialogBox(hInst, (LPSTR)"ADDLOC_FROMINT", hWnd, lpfnADDLOC_FROMINTMsgProc);
//                  FreeProcInstance(lpfnADDLOC_FROMINTMsgProc);
            }
            	 break; 
            	       
            case IDM_STREET_SEGS_BETWEEN_INTS: 
            {
                  DLGPROC	lpfnSTREET_SEGS_BETWEEN_INTSMsgProc; 

                  lpfnSTREET_SEGS_BETWEEN_INTSMsgProc = MakeProcInstance((DLGPROC)STREET_SEGS_BETWEEN_INTSMsgProc, hInst);
                  CreateDialog(hInst, (LPSTR)"STREET_SEGS_BETWEEN_INTS", hWnd, lpfnSTREET_SEGS_BETWEEN_INTSMsgProc);
//                  nRc = DialogBox(hInst, (LPSTR)"STREET_SEGS_BETWEEN_INTS", hWnd, lpfnSTREET_SEGS_BETWEEN_INTSMsgProc);
//                  FreeProcInstance(lpfnSTREET_SEGS_BETWEEN_INTSMsgProc);
            }
            	 break; 
            	       
            case IDM_ADDLOC_FROMADD: 
            {
				if (*AutoExportName)
				{
					HWND hDlg = CreateDialog(hInst, (LPSTR)"ADDLOC_FROMADD", hWnd, (DLGPROC)ADDLOC_FROMADDMsgProc);
					PostMessage(hDlg, WM_COMMAND, IDC_ISMODELESS, 0);
				}
				else
				{
					nRc = DialogBox(hInst, (LPSTR)"ADDLOC_FROMADD", hWnd, (DLGPROC)ADDLOC_FROMADDMsgProc);
				}

            }
            	 break; 
            	       
            case IDM_TEST_STREET: 
            {
                  DLGPROC	lpfnTEST_STREETMsgProc; 

                  lpfnTEST_STREETMsgProc = MakeProcInstance((DLGPROC)TEST_STREETMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"TEST_STREET", hWnd, lpfnTEST_STREETMsgProc);
                  FreeProcInstance(lpfnTEST_STREETMsgProc);
            }
            	 break; 

            case IDM_INTACCIDPROF: 
            {
                  DLGPROC	lpfnINTACCIDPROFMsgProc; 

                  lpfnINTACCIDPROFMsgProc = MakeProcInstance((DLGPROC)INTACCIDPROFMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"INTACCIDPROF", hWnd, lpfnINTACCIDPROFMsgProc);
                  FreeProcInstance(lpfnINTACCIDPROFMsgProc);
            }
            	 break; 

            case IDM_ADDEDIT_HELPER:
                {
                  DLGPROC lpfnADDEDIT_HELPERMsgProc;
                  
				  setDoPaint( FALSE);
                  if (!hWndAddEditHelper)
                  { 
					  lpfnADDEDIT_HELPERMsgProc = MakeProcInstance((DLGPROC)ADDEDIT_HELPERMsgProc, hInst);
					  CreateDialog(hInst,"ADDEDIT_HELPER",hWnd, lpfnADDEDIT_HELPERMsgProc);
				  }
                }
                break;
            
            case IDM_SAVE_NETINT:
            	SaveIntersectFile ("netint.gmd");
				break;
            
			case IDM_PROCESSTEXT:
			{
				 LPSTR pCmd;

				 if (!lParam)
				 {
					if (GetDebug () && *LastPMFile)
					{
						char file[MAX_PATH];
						if (*LastPMFile == '\'')
							strcpy(file, LastPMFile);
						else
							sprintf(file, "'%s'", LastPMFile);
						GMEdit(hWndMain, file);
					}
				 	break;  
				 }
				 pCmd = GlobalLock ((HANDLE)lParam); 
				 if (!pCmd)
				 	break;
				 ProcessText (pCmd);
				 GSSiGlobUlFree ((LPHANDLE)&lParam);
			}
				 break;				 
				             	       
            case IDM_POINTMAP: 
            {
                  DLGPROC	lpfnPOINTMAPMsgProc; 

                  lpfnPOINTMAPMsgProc = MakeProcInstance((DLGPROC)POINTMAPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"POINTMAP", hWnd, lpfnPOINTMAPMsgProc);
                  FreeProcInstance(lpfnPOINTMAPMsgProc);
            }
            	 break; 
            
            case IDM_RELOAD_MENU:	       
            case IDM_LOAD_MENU: 
            {
            	 LPSTR	StartupMenu=GlobalLock (hStartupMenu);
            	  
			   	 if (LOWORD(wParam) == IDM_LOAD_MENU)
			   	 	GetTextString (hWnd,StartupMenu,128,"Enter the menu file",0,0,0,TRUE,TRUE);  
			   	 if (!*StartupMenu)
			   	 	LoadFullMenu (hWnd);
			   	 else if (!GMLoadMenu (hWnd,StartupMenu))
				    GSSiMsgBox( GetFocus(),"Failed to load menu file",0, MB_OK|MB_ICONEXCLAMATION,0);
				 GlobalUnlock (hStartupMenu);
            }
            	 break;
            	      
            case IDM_EDIT_MENU: 
            {
            	EditMenu (hWnd,hStartupMenu);  
            }
            	 break;
            	      
            case IDM_CREATE_FILELIST: 
            {
                DLGPROC	lpfnCREATEFILELISTMsgProc; 
            	
	           	 _getcwd (CurDir,256);
           	  	 SaveDrive = _getdrive(); 

                  lpfnCREATEFILELISTMsgProc = MakeProcInstance((DLGPROC)CREATEFILELISTMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"CREATEFILELIST", hWnd, lpfnCREATEFILELISTMsgProc);
                  FreeProcInstance(lpfnCREATEFILELISTMsgProc);
        		 _chdir (CurDir);
            	 _chdrive (SaveDrive);
            }
            	 break; 

            case IDM_LOADMIF: 
            {
                  DLGPROC	lpfnLOADMIFMsgProc; 

                  lpfnLOADMIFMsgProc = MakeProcInstance((DLGPROC)LOADMIFMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADMIF", hWnd, lpfnLOADMIFMsgProc);
                  FreeProcInstance(lpfnLOADMIFMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADSHP: 
            {
                  DLGPROC	lpfnLOADSHPMsgProc; 

                  lpfnLOADSHPMsgProc = MakeProcInstance((DLGPROC)LOADSHPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADSHP", hWnd, lpfnLOADSHPMsgProc);
                  FreeProcInstance(lpfnLOADSHPMsgProc);
            }
            	 break;       
            	 
/*            case IDM_LOADGEN: 
            {
                  DLGPROC	lpfnLOADGENMsgProc; 

                  lpfnLOADGENMsgProc = MakeProcInstance((DLGPROC)LOADGENMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADGEN", hWnd, lpfnLOADGENMsgProc);
                  FreeProcInstance(lpfnLOADGENMsgProc);
            }
            	 break;  */    
            	 
            case IDM_LOADUMAREAS: 
            {
                  DLGPROC	lpfnLOADUMAREASMsgProc; 

                  lpfnLOADUMAREASMsgProc = MakeProcInstance((DLGPROC)LOADUMAREASMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADUMAREAS", hWnd, lpfnLOADUMAREASMsgProc);
                  FreeProcInstance(lpfnLOADUMAREASMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADXFER: 
            {
                  DLGPROC	lpfnLOADXFERMsgProc; 

                  lpfnLOADXFERMsgProc = MakeProcInstance((DLGPROC)LOADXFERMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADXFER", hWnd, lpfnLOADXFERMsgProc);
                  FreeProcInstance(lpfnLOADXFERMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADDGNDUMP: 
            {
                  DLGPROC	lpfnLOADDGNDUMPMsgProc; 

                  lpfnLOADDGNDUMPMsgProc = MakeProcInstance((DLGPROC)LOADDGNDUMPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADDGNDUMP", hWnd, lpfnLOADDGNDUMPMsgProc);
                  FreeProcInstance(lpfnLOADDGNDUMPMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADFLOOD: 
            {
                  DLGPROC	lpfnLOADFLOODMsgProc; 

                  lpfnLOADFLOODMsgProc = MakeProcInstance((DLGPROC)LOADFLOODMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADFLOOD", hWnd, lpfnLOADFLOODMsgProc);
                  FreeProcInstance(lpfnLOADFLOODMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADSSURGO: 
            {
                  DLGPROC	lpfnLOADSSURGOMsgProc; 

                  lpfnLOADSSURGOMsgProc = MakeProcInstance((DLGPROC)LOADSSURGOMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADSSURGO", hWnd, lpfnLOADSSURGOMsgProc);
                  FreeProcInstance(lpfnLOADSSURGOMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADBNA: 
            {
                  DLGPROC	lpfnLOADBNAMsgProc; 

                  lpfnLOADBNAMsgProc = MakeProcInstance((DLGPROC)LOADBNAMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADBNA", hWnd, lpfnLOADBNAMsgProc);
                  FreeProcInstance(lpfnLOADBNAMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADDOQS: 
            {
                  DLGPROC	lpfnLOADDOQSMsgProc; 

                  lpfnLOADDOQSMsgProc = MakeProcInstance((DLGPROC)LOADDOQSMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADDOQS", hWnd, lpfnLOADDOQSMsgProc);
                  FreeProcInstance(lpfnLOADDOQSMsgProc);
            }
            	 break;       
            	 
            case IDM_CREATE_ORTHOCDS: 
            {
                  DLGPROC	lpfnCREATE_ORTHOCDSMsgProc; 

                  lpfnCREATE_ORTHOCDSMsgProc = MakeProcInstance((DLGPROC)CREATE_ORTHOCDSMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"CREATE_ORTHOCDS", hWnd, lpfnCREATE_ORTHOCDSMsgProc);
                  FreeProcInstance(lpfnCREATE_ORTHOCDSMsgProc);
            }
            	 break;       
            	 
            case IDM_CREATE_ORTHOCDS2: 
            {
                  DLGPROC	lpfnCREATE_ORTHOCDS2MsgProc; 

                  lpfnCREATE_ORTHOCDS2MsgProc = MakeProcInstance((DLGPROC)CREATE_ORTHOCDS2MsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"CREATE_ORTHOCDS2", hWnd, lpfnCREATE_ORTHOCDS2MsgProc);
                  FreeProcInstance(lpfnCREATE_ORTHOCDS2MsgProc);
            }
            	 break;       
            	 
            case IDM_LOADDXF: 
            {
                  DLGPROC	lpfnLOADDXFMsgProc; 

                  lpfnLOADDXFMsgProc = MakeProcInstance((DLGPROC)LOADDXFMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADDXF", hWnd, lpfnLOADDXFMsgProc);
                  FreeProcInstance(lpfnLOADDXFMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADTIGER: 
            {
                  DLGPROC	lpfnLOAD_TIGERMsgProc; 

                  lpfnLOAD_TIGERMsgProc = MakeProcInstance((DLGPROC)LOAD_TIGERMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOAD_TIGER", hWnd, lpfnLOAD_TIGERMsgProc);
                  FreeProcInstance(lpfnLOAD_TIGERMsgProc);
            }
            	 break;       
            	 
            case IDM_LOADTIGER_PN: 
            {
                  DLGPROC	lpfnLOAD_TIGER_PNMsgProc; 

                  lpfnLOAD_TIGER_PNMsgProc = MakeProcInstance((DLGPROC)LOAD_TIGER_PNMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOAD_TIGER_PN", hWnd, lpfnLOAD_TIGER_PNMsgProc);
                  FreeProcInstance(lpfnLOAD_TIGER_PNMsgProc);
            }
            	 break;       
            	 
			case IDM_TIGER_OUT:
				 TIGEROut();
				 break;       
				 
			case IDM_LOAD_DYNAMIC:
//				 OpenModelessDialog("dyndial.txt");
				 break;
				 
            case IDM_BMP_OUTPUT:
            	 ExportData (hWnd,ORTHBMP);
            	 break;
            	 
            case IDM_ORACLE_OUTPUT:
            	 ExportData (hWnd,ORACLE);
            	 break;
            	 
            case IDM_ORACLEDTM_OUTPUT:
            	 ExportData (hWnd,ORACLEDTM);
            	 break;
            	 
            case IDM_DTMTOTEXT_OUTPUT:
            	 ExportData (hWnd,DTMTOTEXT);
            	 break;
            	 
            case IDM_SHP_OUTPUT:
            	 ExportData (hWnd,SHP);
            	 break;
            	 
            case IDM_DXF_OUTPUT:
            	 ExportData (hWnd,DXF);
            	 break;
            	 
            case IDM_DGN_OUTPUT:
            	 ExportData (hWnd,DGN);
            	 break;
            	 
            case IDM_TXT_OUTPUT:
            	 ExportData (hWnd,TXT);    
            	//LoadSoilData(1);
            	 break;
            	 
            case IDM_MIF_OUTPUT: 
            	 ExportData (hWnd,MIF);
            	 break;       
            	 
            case IDM_ZOOM_SCALE:
       			 HaltMapDisplay(FALSE,FALSE);
            {
                  DLGPROC	lpfnZOOMSCALEMsgProc; 

                  lpfnZOOMSCALEMsgProc = MakeProcInstance((DLGPROC)ZOOMSCALEMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"ZOOMSCALE", hWnd, lpfnZOOMSCALEMsgProc);
                  FreeProcInstance(lpfnZOOMSCALEMsgProc);
                  if (nRc)
                      PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);

            }
            	 break;       
            	 
            case IDM_MAX_REFNO:
            {    
            	 long	MinRef;
				 long	MaxRef = GetMaxRefno (-1,&MinRef);
				 
				 hSTR=GSSiGlobAlloc (   9,GMEM_MOVEABLE,1024);
				 str=GlobalLock (hSTR); 
				 sprintf (str,"Min/Max refno in this configuration is %ld/%ld",MinRef,MaxRef);
				 GSSiMsgBox(GetFocus(),str," ", MB_OK,0);
            	 GSSiGlobUlFree (&hSTR);
            
            }
            	 break;
            case IDM_P_LEFT:
            case IDM_P_RIGHT:
            case IDM_P_UP:
            case IDM_P_DOWN:
            {
            	 int pandir;
            	 
            	 switch (LOWORD(wParam))
            	 {
		            case IDM_P_LEFT:
		            	pandir = 2;
		            	break;
		            case IDM_P_RIGHT:
		            	pandir = 0;
		            	break;
		            case IDM_P_UP:
		            	pandir = 1;
		            	break;
		            case IDM_P_DOWN:
		            	pandir = 3;
		            	break;
		         }
				 HaltMapDisplay(FALSE, FALSE);
    			 {
					POINT	CPoint; 
					GetCursorPos (&CPoint); 
					ScreenToClient (hWnd,&CPoint);
				 	vpid = SelectViewport (CPoint,TRUE,FALSE,FALSE);
                    if (CurView->Type == 7) 
                    {
			     		if (CurView->DisplayedFullScreen) 
			    		{
			    			ClearFullDisplay = TRUE;
			    			MakeVPFullScreen (CurView->ID,0);
			    		}
                      	SetZoomVP (CurView->BoundsDisplayVP); 
                    }
				 	else if (VPIsMap(vpid) && ((lParam < 1 || lParam >MAX_VIEWPORTS) || vpid == lParam))
				 		SetZoomVP (vpid); 
				 	else if (lParam > 0 && lParam <*pNumViewports)
				 		SetZoomVP (lParam); 
				 	else
				 		SetZoomVP (*pCommandViewport); 
				 }
                 PanWindow (hWnd, pandir);
		         if (ClearFullDisplay)
					PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
            }
                 break;

            case IDM_Z_IN:
			     ClearCurrentCD ();
            case IDM_Z_OUT:
		   		 if (ScaleIsSet (TRUE))
		   			break;
            case IDM_Z_CENTER:
            {
            	 float ZoomFactor = 2.0;
 				 DPOINT	Point;
           	 
     			 CurView->CurZoomAreaRef = 0;
            	 if (LOWORD(wParam) == IDM_Z_CENTER)
            	 	ZoomFactor = 1.0;
            	 else if (LOWORD(wParam) == IDM_Z_OUT)
            	 	ZoomFactor = 0.5;
       			 HaltMapDisplay(FALSE,FALSE);
				 if (!InAccel || SetZoomVP(lParam))
					 Point = CurView->MidPointW;
				 else
    			 {
					POINT	CPoint; 
					short	vpid; 
					
					GetCursorPos (&CPoint); 
					ScreenToClient (hWnd,&CPoint);
				 	vpid = SelectViewport (CPoint,TRUE,FALSE,FALSE);
					if (!vpid)
					{
						SetViewport(*pCommandViewport);
						Point = CurView->MidPointW;
					}
				 	else if (lParam < MAX_VIEWPORTS+1 && VPIsMap(vpid) && (lParam < 1 || vpid == lParam))
				 	{ 
                        POINT	CursorPoint=RectMid (&CurView->ScreenRect);

						Point = ScreenPtToBasePt (CPoint);
                        if (CurView->Type == 7)
                        {
				     		if (CurView->DisplayedFullScreen) 
				    		{
				    			ClearFullDisplay = TRUE;
				    			MakeVPFullScreen (CurView->ID,0);
				    		}
	                      	SetZoomVP (CurView->BoundsDisplayVP); 
	                    }
	                    else if (lParam <= MAX_VIEWPORTS)
	                    {
	                        ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
		                	SetCursorPos (CursorPoint.x,CursorPoint.y);
                        }
				 		/*{
					 		double	width = CurView->WBounds.xmx - CurView->WBounds.xmn;
					 		double	height = CurView->WBounds.ymx - CurView->WBounds.ymn;
	                        
					 		CurView->WBounds.xmn = Point.x - width/2;
					 		CurView->WBounds.ymn = Point.y - height/2;
					 		CurView->WBounds.xmx = Point.x + width/2;
					 		CurView->WBounds.ymx = Point.y + height/2; 
					 	}*/
				 	}
				 	else if (lParam > 0 && lParam <*pNumViewports)
					{
				 		SetZoomVP (lParam); 
						Point = MinMaxMidPointD (&CurView->WBounds);
					}
				 	else
					{
				 		SetZoomVP (*pCommandViewport);
						Point = MinMaxMidPointD (&CurView->WBounds);
					}
				 }
				 ZoomToPointAndScale (Point,CurView->Scale/ZoomFactor,FALSE);
				// ZoomWindow (hWnd, ZoomFactor);
		         if (ClearFullDisplay)
					PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
			}
                 break;
            case IDM_Z_PRIOR:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Out" here.                         */
       			 HaltMapDisplay(FALSE,FALSE);
    			 SetZoomVP (lParam);
                 ZoomWindow (hWnd,0);
                 break;

            case IDM_Z_ORTHO:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Out" here.                         */
		   		 if (ScaleIsSet (TRUE))
		   			break;
       			 HaltMapDisplay(FALSE,FALSE);
				 setDoPaint( TRUE);
				 SetViewport(-99);
				 SelectVisList (FALSE);
                 ZoomToBM ();
                 break;

            case IDM_Z_WINDOW:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Window" here.                      */
                  //CloseMap (); 
                  if (iii)	
                  	ii=0;
                  iii++;
//				  SetViewport(*pCommandViewport);
       			  HaltMapDisplay(FALSE,FALSE);
                  AddGraphicsFunction (hWnd,GF_WINDOW_ZOOM,0);
                  HaltPaint = TRUE;
                 break;

            case IDM_Z_REZOOM:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "ReZoom" here.                      */
				 if (!GetGlobalBVal2 ("[%REZOOMENABLE]",TRUE))
					 break;
	             	 UnallocateConfig();
			    	 PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
/*				 SetViewport(*pCommandViewport);  
				 CurView->CurZoomAreaRef = LONG_MAX;
				 ZoomToRect(RezoomRect,FALSE); 
//				 ProcessText ("$LOADCFG()");
				 CurView->WindowIsZoomed = FALSE;*/
                 break;
            
            case IDM_Z_VISLIMITS:
            {    
            	 
 		   		 if (ScaleIsSet (TRUE))
		   			break;
   				 SetZoomVP (lParam);
				 GetVisBounds (&Bounds,CurView->hDC); 
				 CurView->CurZoomAreaRef = LONG_MAX - 1;
				 ZoomToRect(Bounds,FALSE);
			}   
				 break;
				 
            case IDM_Z_EDITLIMITS: 
            {
		   		 if (ScaleIsSet (TRUE))
		   			break;
            	 
    			 if (!GetLayerBounds (&Bounds,CurView->hDC, CurView->UpdateFile-1))
    			 	break;
    			 SetZoomVP (lParam);
    			 CurView->CurZoomAreaRef = 0;
				 ZoomToRect(Bounds,FALSE);  
			}
    			 break;
            
            case IDM_Z_HLTLIMITS: 
            {
            	 
            	 if (!hHighlight)
            	 	break;
		   		 if (ScaleIsSet (TRUE))
		   			break;
    			 SetZoomVP (lParam);
//				 SetViewport(*pCommandViewport);  
				 CurView->CurZoomAreaRef = 0;
				 ZoomToRect(HLTBounds,FALSE);  
			}
    			 break;
            
            case IDM_Z_MASK:
            {   
            	LPMNMXCORD pBounds;
            	
		   		 if (ScaleIsSet (TRUE))
		   			break;
//				SetViewport(*pCommandViewport);
				if (CurView->hMaskArea) 
			    {
			    	pBounds = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
			    	Bounds = *pBounds;
			    	GlobalUnlock (CurView->hMaskArea);
					ZoomToRect(Bounds,FALSE);  
			    }
            } 
            	break;
            
            case IDM_Z_REDRAW:
            {
            	 BOOL	Imed=FALSE;
            	  
            	 if (InDisplayProcessing==1)
            	 	break;
            	 if (NeedFullRedisplay()) 
            	 {
            	 	RedisplayMenu = FALSE; 
            	 	goto DoRedisplay;    
            	 }
       			 HaltMapDisplay(FALSE,FALSE);
				 ClearFullWindowBitmap(0);
				 setDoPaint( TRUE);
                 IgnoreBounds=FALSE;  
                 if (lParam)
                 {
                 	if (!SetViewport((short)lParam))
                 		break;
                 	if (CurView->Type == INDEXVIEWPORT)
                 		Imed = TRUE;
                 }
	   			 RedisplayViewport(Imed,FALSE);
	   		}
                 break;
            
            case IDM_REDISPLAYVIEWPORTS:  
            	 setDoPaint( TRUE);
       			 HaltMapDisplay(FALSE,FALSE);
				 ClearFullWindowBitmap(0);
				 if (NumViewportsArray[0])
				 {
					 SetConfig(0);
					 RedisplayViewports(TRUE);
				 }
	           	 RedisplayViewports(FALSE);
				 break; 
				 
            case IDM_REDISPLAY: 
				if (!DoPaint())
					break;
				if (lParam == 99)
				{
					CurView->HaveBounds = TRUE;
					CurView->WindowIsZoomed = TRUE;
					ProcessText("$REDISPLAY(T)");
					break;
				}
            	 RedisplayMenu = TRUE;
      DoRedisplay:   
      			 DisplayCycle++;
       			 HaltMapDisplay(FALSE,FALSE);
  				 ClearFullWindowBitmap (0);
      			 if (InAccel)
       			 	ClearCurrentCD ();
		         setDoPaint( TRUE);
			     ContinueProcessing=TRUE;
			     if (lParam == -99)
    		        UnallocateConfig ();
     	         if (ConfigLevel)
     	         {  
     	         	UnallocateConfig ();
			     	InvalidateRect (hWndMain,&ConfigRect[ConfigLevel-1],TRUE); 
     	         }
     	         else
     	         {   
     	         	DetermineVPDisplaySequence ();
     	         	SetConfig (1);
					InDisplayProcessing = 2;
					for (iview = 0;iview < *pNumViewports; iview++)
						pViewportsD[iview]->Display = TRUE;
     	         	if (RedisplayMenu && NumViewportsArray[0])
     	         	{
     	         		SetConfig(0);
						SetConfigDisplayRect ();
			     		InvalidateRect (hWndMain,0,TRUE);
			     	}
			     	else if (IsRectEmpty (&ConfigDisplayRect))
			     		InvalidateRect (hWndMain,0,TRUE);
			     	else
					{
						SetConfigDisplayRect ();
			     		InvalidateRect (hWndMain,&ConfigDisplayRect,TRUE);
					}
			     }
			     break;    
			     
			case IDM_LOAD_DOC_FILE:
				 LoadDocumentFile (hWnd);
				 break;        
				 
			case IDM_TRACK_LINE_COLOR:
				 GetColor(hWndMain,&TrackColor);
				 break;  
			case IDM_IDCOMPUTER:
				 if (GetGlobalBVal2("[%ALLOWID]",FALSE))
				 	GetComputerID ();
				 break;
			case IDM_MEMLIST:
#if CHECKMEM  
				 TraceMem ();
#endif
				 break;
			case IDM_ALTACCEL:
				hAccelMain = hAccelAlt;
				CreateDialog(hInst, (LPCTSTR)"ALTACCEL", hWnd, (DLGPROC)AltAccelMsgProc);
				//hAccelMain = hAccelTable;
				break;
			case IDM_ALTACCELF1:
			case IDM_ALTACCELF2:
			case IDM_ALTACCELF3:
			case IDM_ALTACCELF4:
			case IDM_ALTACCELF5:
			case IDM_ALTACCELF6:
			case IDM_ALTACCELF7:
			case IDM_ALTACCELF8:
			case IDM_ALTACCELF9:
			case IDM_ALTACCELF10:
			case IDM_ALTACCELF11:
			case IDM_ALTACCELF12:
				if (hWndAltAccel)
				{
					PostMessage (hWndAltAccel,WM_COMMAND,wParam,0);
				}
					
				break;
			case IDM_CFG1:
			case IDM_CFG2:
			case IDM_CFG3:
			case IDM_CFG4:
			case IDM_CFG5:
			{
				HANDLE	hMem=GSSiGlobAlloc (  10,GMEM_MOVEABLE,256);
				LPSTR	pStr = GlobalLock (hMem);
			    int		icfg=1;
			    
			    switch (LOWORD(wParam))
			    {
			    	case IDM_CFG2:
			    	icfg = 2; 
			    	break; 
			    	case IDM_CFG3:
			    	icfg = 3; 
			    	break; 
			    	case IDM_CFG4:
			    	icfg = 4; 
			    	break; 
			    	case IDM_CFG5:
			    	icfg = 5; 
			    	break; 
			    }
			     HaltMapDisplay (FALSE,TRUE);	
			     sprintf (pStr,"[%%C]=$LOADCFG([%%CFG%i]);[%%C]=$REDISPLAY()",icfg);
				 ExpandText (pStr);    
				 GSSiGlobUlFree (&hMem);
			}
				 break;
			
			case IDM_GPS_ENABLE:   
				 ProcessText ("$GPSTRACKING(2)");
				 break;
			
			case IDM_USEFULLSCREEN: 
				 pViewportsD[0]->ShowFullScreen = TRUE;
				 UseFullScreen (hWnd,0);
				 setDoPaint(TRUE);
			 	 PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
				 break;
				 
			case IDM_ADDFORMAT:
			{
				 HANDLE hSN=GSSiGlobAlloc (  11,GHND,256);
				 LPSTR	Name = GlobalLock (hSN);
				 
       			 HaltMapDisplay(FALSE,TRUE);
				 setDoPaint( FALSE);
				 strcpy (OFTitle,title2);
				 if (GetFileName3(hWndMain,Name,IDS_FILTERGMC,IDS_FILEFMT))   
				 {   
					 if (LoadFormatCfg(Name))
					 {
						 setDoPaint(TRUE);
						 PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
					 }
				 } 
				 GSSiGlobUlFree (&hSN);
			}
				 break;
				 	 	 	 
			case IDM_ADDMENU:
			{
				 HANDLE	hMem=GSSiGlobAlloc (  12,GMEM_MOVEABLE,256);
				 LPSTR	pName=GlobalLock (hMem);
				 
       			 HaltMapDisplay(FALSE,TRUE);
				 setDoPaint( FALSE);
				 strcpy (OFTitle,title3);
				 if (GetFileName3(hWndMain,pName,IDS_FILTERGMC,IDS_FILEMEN))
				 {   
			 	 	IgnoreSavedMenu = TRUE;
					if (LoadMenuConfig (pName,0,HFILE_ERROR,0))
							 	PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAYVIEWPORTS, 0L);
				 }
				 GSSiGlobUlFree (&hMem);
			}
				 break;
				 	 	 	 
			case IDM_CONFIGURE:
			{
				 HANDLE hSN=GSSiGlobAlloc (  13,GMEM_MOVEABLE,256);
				 LPSTR	SaveName = GlobalLock (hSN);
				 
       			 HaltMapDisplay(FALSE,TRUE);
				 setDoPaint( FALSE);
				 _fstrcpy (SaveName,CfgName); 
				 strcpy (OFTitle,title1);
				 if (GetFileName3(hWndMain,CfgName,IDS_FILTERGMC,IDS_FILEGMC))   
				 {       
				 	 ForceBounds = FALSE;
				 	 SaveZooms (0);
	             	 UnallocateConfig();
					 setDoPaint(TRUE);
			    	 PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
				 } 
				 else
				 	_fstrcpy (CfgName,SaveName);
				 GSSiGlobUlFree (&hSN);
				 setDoPaint(TRUE);
				 break;
			}  
			
			case IDM_LOADCONFIG:
				{
					char	str[256];
					LPSTR	pBS;

					GetGlobalCVal ("[%CONFIGPATH]",str,NULL);
					if ((pBS = strrchr (str,'\\')))
						*pBS = 0;
					SetGlobalValue ("%CONFIGDIR",str);
					ProcessText ("$LOADCFG(,[%CONFIGPATH],[%CONFIGDIR],$DIRPATH(MY DOCUMENTS))");
				}
				break;
			     
			case IDM_AUTO_CLEAR_OFFSET:
				 AutoClearOffset = GWCheckMenuItem(hWnd, LOWORD(wParam));  
				 if (AutoClearOffset)
					 GMEnableMenuItem(hWndMain, IDM_MASK_OFFLINE, MF_BYCOMMAND | MF_ENABLED);
				 else
					 GMEnableMenuItem(hWndMain, IDM_MASK_OFFLINE, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
				 
				 break;  
				 
			case IDM_MASK_OFFLINE:
				 MaskOffsetLine = GWCheckMenuItem(hWnd, LOWORD(wParam)); 
				 break;
				 
			case IDM_CLEAR_OFF_LINES:    
//				 SetViewport(*pCommandViewport);
				 ClearPolyOff(FALSE);
				 break;   
				 
			case IDM_LOAD_INFO:
//                  LoadInfo();
				 break;
			     
			case IDM_ZOOM_LIST:
				DisplayZoomList(FALSE);
				break;

			case IDM_ZOOM_ZOOMLISTS:
				DisplayZoomList2(FALSE);
				break;

			case IDM_SAVE_ZOOM:
                 {
                  DLGPROC lpfnSAVEZOOMMsgProc;

                  lpfnSAVEZOOMMsgProc = MakeProcInstance((DLGPROC)SAVEZOOMMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"SAVEZOOM", hWnd, lpfnSAVEZOOMMsgProc);
                  FreeProcInstance(lpfnSAVEZOOMMsgProc);
                 }
            	 break;
			case IDM_ZOOM_SAVE2:
			{
				BOOL err;
				LPVIEWPORT pVP = SetVPFromName("Primary Viewport", &err);
				if (!err)
				{

					SaveZoomToCurrentList(&pVP->WBounds, 0);
				}
			}
				break;
            case IDM_L_PARCEL:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Parcel" here.                      */
                 /*AddGraphicsFunction (hWnd,GF_BLOWUP);*/
				 if (LocatePID (hWnd,hInst))
				 	goto DisplayParcel;

                 break; 
                 
            case IDM_L_COORDINATE: 
            {
            	DLGPROC lpfnLOC_COORDMsgProc;

                lpfnLOC_COORDMsgProc = MakeProcInstance((DLGPROC)LOC_COORDMsgProc, hInst);
                nRc = DialogBox(hInst, (LPSTR)"LOC_COORD", hWnd, lpfnLOC_COORDMsgProc);
                FreeProcInstance(lpfnLOC_COORDMsgProc); 
                if (nRc)
                {
			 		BOOL AP = SetAutoPan (FALSE);

			 		setDoPaint( TRUE);
					ZoomToPointAndDist (UserSpecifiedBasePoint, LocationOffset,FALSE);   
					ExecutePointLocationMacro (UserSpecifiedBasePoint,0);
                    PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L);  
                    SetAutoPan (AP);
			 	}
            }
            	 break;
            	 
            case IDM_L_LATLONG: 
            {
            	DLGPROC lpfnLOC_LATLONGMsgProc;

                lpfnLOC_LATLONGMsgProc = MakeProcInstance((DLGPROC)LOC_LATLONGMsgProc, hInst);
                nRc = DialogBox(hInst, (LPSTR)"LOC_LATLONG", hWnd, lpfnLOC_LATLONGMsgProc);
                FreeProcInstance(lpfnLOC_LATLONGMsgProc); 
                if (nRc)
                {
			 		BOOL AP = SetAutoPan (FALSE);

			 		setDoPaint( TRUE);
					ZoomToPointAndDist (UserSpecifiedBasePoint, LocationOffset,FALSE);   
					ExecutePointLocationMacro (UserSpecifiedBasePoint,0);
                    PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L); 
                    SetAutoPan (AP);
			 	}
            }
            	 break;
            	 
            case IDM_L_REFNO: 
            	
            {
            	DLGPROC lpfnLOC_REFNOMsgProc;

                lpfnLOC_REFNOMsgProc = MakeProcInstance((DLGPROC)LOC_REFNOMsgProc, hInst);
                nRc = DialogBox(hInst, (LPSTR)"LOC_REFNO", hWnd, lpfnLOC_REFNOMsgProc);
                FreeProcInstance(lpfnLOC_REFNOMsgProc); 
                if (nRc)
                {   
                	BOOL	AP;
                	
                	if (LocationOffset)
                	{
			 			AP = SetAutoPan (FALSE);
				 		setDoPaint( TRUE);
						ZoomToPickedItem (0,LocationOffset,OffsetFromLimits,FALSE,FALSE);
			 		}  
				 	else 
				 	{
			 			AP = SetAutoPan (TRUE);
	 					ShowPickedItem (hWndMain,0);
	 				} 
					SetAutoPan (AP);
	 			}
			 	
            }
            	break;
            	
/*            case IDM_L_ROUTEMP:
            {
            	DPOINT	Point;
				BOOL	GotPoint;
                
                GotPoint = NetworkLocation (hWnd,hInst,&Point);
				if (GotPoint && LocationOffset)
			 	{
			 		BOOL	AP = SetAutoPan (FALSE);
			 		
			 		setDoPaint( TRUE);
					ZoomToPointAndDist (Point, LocationOffset,FALSE);
					ExecutePointLocationMacro (Point);
                    PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L); 
                    SetAutoPan (AP);
			 	}
            }
                break; */

            case IDM_L_ADDRESS:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Address" here.                     */
				 if (AddressLocationPID (hWnd,hInst)) 
				 {  
				 	BOOL	AP;
DisplayParcel:	 	
//  				    SetViewport(*pCommandViewport);
			    	ClearCurrentCD ();
                    ProcessPickedItem (0,-1);
					if (!ExecuteItemLocationMacro (0))
					{
						if (LocationOffset)
					 	{
				 			AP = SetAutoPan (FALSE);
				 			
	    					if (AutoHighlight)
	    						AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
							ZoomToPickedItem (0,LocationOffset,OffsetFromLimits,FALSE,AddToView);
					 	}
					 	else 
					 	{
				 			AP = SetAutoPan (TRUE);
	    					if (AutoHighlight)
	    						AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
		 					ShowPickedItem (hWndMain,0);
		 				}
		 				SetAutoPan (AP);  
						hSTR=GSSiGlobAlloc (  15,GMEM_MOVEABLE,256);
						str=GlobalLock (hSTR);
						GetGlobalCVal ("[%OFFSETLINEDIST]",str,0);
		 				OffsetLineOffset = atobasedist (str,&rc); 
		 				GSSiGlobUlFree (&hSTR);
						if (fabs(OffsetLineOffset) > 0.000000001) 
						{   
							HaltMapDisplay(FALSE,FALSE);
				        	OffsetPickedArea (0,OffsetLineOffset);
					        DisplayPolyOff();  
					        DisplayMaskArea();
						    CurView->CurZoomAreaRef = 0;
				   			RedisplayViewport(FALSE,FALSE);
		 				} 
		 				else if (OffsetLineOffset > 0)
		 				{ 
							HaltMapDisplay(FALSE, FALSE);
							SelectAreaToOffsetFile(0, 0, 0);
						    CurView->CurZoomAreaRef = 0;
				   			RedisplayViewport(FALSE,FALSE);
		 				}
		 			}
	 				NumSavedPickList = NumPicked;
	 				if (GetGlobalBVal2 ("[%AUTOIDENTIFY]",TRUE) && NumSavedPickList>0 && !hSavedPickList)
	 				{   
	 					LPPICKDATA	pPickList;
	 					 
	 					hSavedPickList = GSSiGlobAlloc (  14,GMEM_MOVEABLE,NumSavedPickList*sizeof(PICKDATA));
	 					pPickList = (LPPICKDATA)GlobalLock (hSavedPickList);
	 					_fmemmove (pPickList,PickList,NumSavedPickList*sizeof(PICKDATA));
	 					GlobalUnlock (hSavedPickList);
		 				DisplayFinOpt = 1; 
		 			}
		 		 }

                 break; 
                 
           case  IDM_L_NET_ADDRESS:
           case  IDM_L_NET_ADDRESS2:
				 	
	                if (AddressLocation1 (hWnd,hInst,LOWORD(wParam)))
	                {   
	                	BOOL AP;
	       NetPointLoc:
	  				    SetViewport(*pCommandViewport);
				    	ClearCurrentCD ();
				 		AP = SetAutoPan (FALSE); 
				 		setDoPaint( TRUE); 
				 		ClearMaskArea ();
				 		if (LocationOffset)
							ZoomToPointAndDist (UserSpecifiedBasePoint, LocationOffset,FALSE);
						else
							CenterWindow (UserSpecifiedBasePoint,FALSE);
						ExecutePointLocationMacro (UserSpecifiedBasePoint,0);
	                    PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L);   
	                    SetAutoPan (AP);
				 	}
                 break;


            case IDM_L_STREET:
                {
                  DLGPROC lpfnLOC_STREETMsgProc;
                  
				  setDoPaint( TRUE);
                  if (!hWndLocStreet)
                  { 
					  lpfnLOC_STREETMsgProc = MakeProcInstance((DLGPROC)LOC_STREETMsgProc, hInst);
					  CreateDialog(hInst,"LOC_STREET",hWnd, lpfnLOC_STREETMsgProc);
				  }
                }
                break;

            case IDM_L_INTERSECTION:
                {
                  DLGPROC lpfnLOC_INTERSECTMsgProc;  

                  lpfnLOC_INTERSECTMsgProc = MakeProcInstance((DLGPROC)LOC_INTERSECTMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOC_INTERSECT", hWnd, lpfnLOC_INTERSECTMsgProc);
                  FreeProcInstance(lpfnLOC_INTERSECTMsgProc);  
                  if (nRc)
                  	goto NetPointLoc;
                 }
                 break;       
                 
            case IDM_LOCATION_OFFSET: 
                {
                  DLGPROC lpfnLOCATION_OFFSETMsgProc;

                  lpfnLOCATION_OFFSETMsgProc = MakeProcInstance((DLGPROC)LOCATION_OFFSETMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOCATION_OFFSET", hWnd, lpfnLOCATION_OFFSETMsgProc);
                  FreeProcInstance(lpfnLOCATION_OFFSETMsgProc);
                 }

                 break; 
                 
            case IDM_PICKABILITY:
            	 Pickability = TRUE;
            	 goto Visible;

            case IDM_VISIBILITY:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Visibility" here.                  */ 
                 Pickability = FALSE;
             Visible:
                 {
                  DLGPROC lpfnVISIBLEMsgProc;  
                  char	VisDialog[2][10]={"VISIBLE","VISIBLE1"};

				  //SetViewport(*pCommandViewport);
                  lpfnVISIBLEMsgProc = MakeProcInstance((DLGPROC)VISIBLEMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)VisDialog[VisListOpt], hWnd, lpfnVISIBLEMsgProc);
                  FreeProcInstance(lpfnVISIBLEMsgProc);
                  if (!Pickability && nRc)
                  	RedisplayViewport(FALSE,FALSE);

                 }
                 break;  
                 
            case IDM_VISIBILITY2:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Visibility" here.                  */ 
                 Pickability = FALSE;
             Visible2:
                 {
                  DLGPROC lpfnVISIBLE2MsgProc;

//				  SetViewport(*pCommandViewport);
                  lpfnVISIBLE2MsgProc = MakeProcInstance((DLGPROC)VISIBLE2MsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"VISIBLE2", hWnd, lpfnVISIBLE2MsgProc);
                  FreeProcInstance(lpfnVISIBLE2MsgProc);
                  if (!Pickability && nRc)
                  	RedisplayViewport(FALSE,FALSE);

                 }
                 break;  
                 
            case IDM_LOAD_VISIBILITY:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Visibility" here.                  */ 
                 Pickability = FALSE;
                 {
                  DLGPROC lpfnVISFILESMsgProc;
                  
//				  SetViewport(*pCommandViewport);
                  lpfnVISFILESMsgProc = MakeProcInstance((DLGPROC)VISFILESMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"VISFILES", hWnd, lpfnVISFILESMsgProc);
                  FreeProcInstance(lpfnVISFILESMsgProc);
                  if (!Pickability && nRc)
                  	RedisplayViewport(FALSE,FALSE);

                 }
                 break;  
                 
            case IDM_LOAD_PICKABILITY:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Visibility" here.                  */ 
                 Pickability = TRUE;
                 {
                  DLGPROC lpfnVISFILESMsgProc;

//				  SetViewport(*pCommandViewport);
                  lpfnVISFILESMsgProc = MakeProcInstance((DLGPROC)VISFILESMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"VISFILES", hWnd, lpfnVISFILESMsgProc);
                  FreeProcInstance(lpfnVISFILESMsgProc);

                 }
                 break;  
                 
            case IDM_VIEWPORTS:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Visibility" here.                  */ 
                 {
                  DLGPROC lpfnVIEWPORTSMsgProc;

                  lpfnVIEWPORTSMsgProc = MakeProcInstance((DLGPROC)VIEWPORTSMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"VIEWPORTS", hWnd, lpfnVIEWPORTSMsgProc);
                  FreeProcInstance(lpfnVIEWPORTSMsgProc);
				  if (nRc)
				  {
					  setDoPaint(TRUE);
					  PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
				  }
                 }
                 break;  
                 
            case IDM_MAPCOPY:
				 TimeRangeBeg=0;
				 TimeRangeEnd=LONG_MAX; 
				 GetGlobalCVal ("[%COPYMAPTO]",MapCopyPath,"");
               	 if ((DoMapCopy = GetTextString (hWnd,MapCopyPath,32,"Enter directory to copy to",0,0,0,TRUE,TRUE)))
			     {
					 VISLIST	SaveVis; 
					 int		SaveNumVis; 
					 OFSTRUCTGM	OFStruct;
					 HCURSOR	hcurSave;
               	 	 
               	 	 if (!GetTextString (hWnd,MapCopyProjection,128,"Select conversion projection or CANCEL for none","*.CVT",0,0,TRUE,TRUE))
               	 		MapCopyProjection[0]=0;
					 
/*					 CopyFID = GSSiOpenFile ("copylist.txt",&OFStruct,OF_READ);
					 DoMapCopy = 3;
					 if (CopyFID != HFILE_ERROR)
					 {
					 	short opt=GSSiMsgBox (GetFocus(),"Restart using existing list of files?",
					 								"Copy Map Files",MB_YESNOCANCEL);
					 	if (opt == IDCANCEL) 
					 	{
					 		DoMapCopy = 0;
					 		break;        
					 	}
					 	if (opt == IDYES)
					 		goto UseOldList;
					 } */
					 CopyFID = GSSiOpenFile ("copylist.txt",&OFStruct,OF_CREATE);
					 DoMapCopy = 1;
	                 setDoPaint( FALSE); 
	                 DisableHalt=TRUE;
					 SetViewport(*pCommandViewport);
					 SaveVis = *CurVis;
					 InitVis ();   
					 _fmemmove (CurVis->FileIsVisible,SaveVis.FileIsVisible,sizeof(CurVis->FileIsVisible));
					 SetViewport(*pCommandViewport);
				     CurView->CurZoomAreaRef = 0;
				     hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
					 RedisplayViewport(TRUE,TRUE); 
					 GSSiSetCursor(hcurSave);
					 SetViewport(*pCommandViewport);
					 *CurVis = SaveVis; 
					 CopyMapFile ("-DONE-","-DONE-",-1,CurFileIndexEntry); 
					 DoMapCopy = 2;
		UseOldList:
					 GSSiClose (CopyFID); 
					 CreateStatusWind (hWnd,1,"Copying Map Data");
					 CopyMapFile ("","",-1,CurFileIndexEntry); 
					 DoMapCopy = 0;
					 DestroyStatusWindow(0); 
					 GSSiRemove ("copylist.txt"); 
					 DisableHalt=FALSE;
					 setDoPaint( TRUE);
			     }
               	  
            	 break;

            case IDM_CLEAR:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Clear" here.                       */
//                                   LoadDLT("c:\\work.txt","attribut\\trees");

				 SetGlobalValue("%PLOT","");
                 ClearMap ();
                 break;
            
            case IDM_PRINTSETUP:  
            	 InPlotView = 0;
           		 ShowScrollBar(hWnd, SB_BOTH,FALSE);
				 CheckMenuItem(GetMenu(hWnd), IDM_PLOTVIEW, MF_BYCOMMAND | MF_UNCHECKED);
				 if (PrintMap (hWnd,0,-2))  
				 {
					pViewportsD[0]->ShowFullScreen = FALSE;
					GMEnableMenuItem(hWnd, IDM_PLOTVIEW, MF_BYCOMMAND | MF_ENABLED);
				 	PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
				 }
            	 break;
            	 
            case IDM_PRINT:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Print" here.                       */
                 PrintMap (hWnd,1,1);
                 break;

            case IDM_PRINT_MERGE:
                 {
                  DLGPROC lpfnPRINTMERGEMsgProc;

                  lpfnPRINTMERGEMsgProc = MakeProcInstance((DLGPROC)PRINTMERGEMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"PRINTMERGE", hWnd, lpfnPRINTMERGEMsgProc);
                  FreeProcInstance(lpfnPRINTMERGEMsgProc);
                 }
                  if (nRc==1)
                  	PrintMerge(hWnd);
                  else if (nRc==2)
                 {
                  
                  lpfnPRINTMERGETSTMsgProc = MakeProcInstance((DLGPROC)PRINTMERGETSTMsgProc, hInst);
                  CreateDialog(hInst, (LPSTR)"PRINTMERGETST", hWnd, lpfnPRINTMERGETSTMsgProc);
                 }
                 break;
            
            case IDM_SMALLBM:
				 CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_CHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_MEDIUMBM, MF_BYCOMMAND | MF_UNCHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_UNCHECKED); 
				 BitmapSizeOpt=1;
				 break;
            case IDM_MEDIUMBM:     
				 CheckMenuItem(GetMenu(hWnd), IDM_MEDIUMBM, MF_BYCOMMAND | MF_CHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_UNCHECKED);
				 BitmapSizeOpt = 2; 
				 break;
            case IDM_LARGEBM:     
				 CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_CHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_MEDIUMBM, MF_BYCOMMAND | MF_UNCHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_UNCHECKED);
				 BitmapSizeOpt = 3; 
				 break;
            case IDM_FORMATBMP:     
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATBMP, MF_BYCOMMAND | MF_CHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATTIF, MF_BYCOMMAND | MF_UNCHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATGEOTIF, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATJPEG, MF_BYCOMMAND | MF_UNCHECKED);
				 BitmapFormatOpt = 0; 
				 break;
            case IDM_FORMATTIF:     
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATBMP, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATTIF, MF_BYCOMMAND | MF_CHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATGEOTIF, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATJPEG, MF_BYCOMMAND | MF_UNCHECKED);
				 BitmapFormatOpt = 1; 
				 break;
            case IDM_FORMATGEOTIF:     
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATBMP, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATTIF, MF_BYCOMMAND | MF_UNCHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATGEOTIF, MF_BYCOMMAND | MF_CHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATJPEG, MF_BYCOMMAND | MF_UNCHECKED);
				 BitmapFormatOpt = 2; 
				 break;
            case IDM_FORMATJPEG:     
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATBMP, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATTIF, MF_BYCOMMAND | MF_UNCHECKED); 
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATGEOTIF, MF_BYCOMMAND | MF_UNCHECKED);
				 CheckMenuItem(GetMenu(hWnd), IDM_FORMATJPEG, MF_BYCOMMAND | MF_CHECKED);
				 BitmapFormatOpt = 3; 
				 break;
            case IDM_E_TOCLIPBOARD:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "To Clipboard" here.                */
                 ClipMap (hWnd,0,2,0);
		     	 PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
                 break;

            case IDM_E_TOFILE:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "To file" here.                     */   
				{	
			        DLGPROC lpfnEXPORTIMAGEMsgProc; 
			        int	nRc;
					
					hSTR=GSSiGlobAlloc (  15,GHND,256);
					str=GlobalLock (hSTR); 
		            lpfnEXPORTIMAGEMsgProc = MakeProcInstance((DLGPROC)EXPORTIMAGEMsgProc, hInst);
		            nRc = DialogBox(hInst, (LPSTR)"EXPORTIMAGE", hWnd, lpfnEXPORTIMAGEMsgProc);
		            FreeProcInstance(lpfnEXPORTIMAGEMsgProc);
	          		if (nRc)
	          		{
					    GetCurVal (str,256,IDS_FILEEXPORTIMAGE); 
            	 		ExpandText (str);
		                ClipMap (hWnd,str,BitmapSizeOpt,BitmapFormatOpt); 
				     	PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
					}
	            	GSSiGlobUlFree (&hSTR);
				}
                 break;

            case IDM_HLT_CLEAR:
            	 ClearHighlightList (FALSE);   
            	 break;   
            	 
            case IDM_HLT_SAVE: 
            {
            	char	Ext[8]; 
            	LPSTR	str, str2;
            	
				if (!hHighlight)
				{
					GSSiMsgBox( GetFocus(),"No Highlight List","Error", MB_OK,0);
					break;
				}  
           		 _fstrcpy (Ext,".HLT");
				hSTR=GSSiGlobAlloc (  16,GMEM_MOVEABLE,1024);
				str=GlobalLock (hSTR); 
				str2 = str + 512;
            	 if (GetSaveName2 (hWnd,str,0,Ext,IDS_FILEHLT))  
            	 {
            	 	_fstrcpy (str2,str);
            	 	*LastChr (str2) = '2';
	            	CopyHighlightList (str);
	             }
            	 GSSiGlobUlFree (&hSTR);
            }

            	 break;   
            	 
            case IDM_DISPLAY_HLT_LIST:
            	 ShowHLTList(hWnd,lParam);
            	 break;
            	 
			case IDM_HLT_IN_AREA:
				 HighlightInArea (hWnd,0,TRUE,TRUE,0);
			     CurView->CurZoomAreaRef = 0;
	   			 RedisplayViewport(FALSE,FALSE);
            	 break;   
            	 
			case IDM_UNHLT_IN_AREA:
				 HighlightInArea (hWnd,0,FALSE,FALSE,0);
			     CurView->CurZoomAreaRef = 0;
	   			 RedisplayViewport(FALSE,FALSE);
            	 break;   
            	 
			case IDM_HLT_IN_VP:
//				 SetViewport(*pCommandViewport);
				 HighlightInArea (hWnd,&CurView->WBounds,TRUE,TRUE,CurView->hMaskArea);
			     CurView->CurZoomAreaRef = 0;
	   			 RedisplayViewport(FALSE,FALSE);
            	 break;   
            	 
            case IDM_HLT_OUT_AREAS1: 
			     GetGlobalCVal ("[%HLTOUTPUTFILE]",HLTOutPath,0);
               	 if (GetTextString (hWnd,HLTOutPath,128,"Output File",0,0,0,TRUE,TRUE))
            	 	HLTOUTFormat=1;  
            	 
            	 break; 
            	 
            case IDM_HLT_OUTFORMAT:
                {
                  DLGPROC lpfnHLTOUT_FORMATMsgProc;

                  lpfnHLTOUT_FORMATMsgProc = MakeProcInstance((DLGPROC)HLTOUT_FORMATMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"HLTOUT_FORMAT", hWnd, lpfnHLTOUT_FORMATMsgProc);
                  FreeProcInstance(lpfnHLTOUT_FORMATMsgProc);
                  HLTOUTFormat=0;
                 }
            	 break;
            	 
            case IDM_HLT_OUT: 
            	 CreateHighlightOutput (hWnd,0,0,0);
            	 break;

            case IDM_TAG_CLEAR:
            	 ClearTAGs ();
            	 break; 
            	 
            case IDM_AREA_OUTLINE:
            	 OutlineZoomArea= GWCheckMenuItem(hWnd, LOWORD(wParam));
            	 break;
            	 
            case IDM_BUILDREFINDEXES: 
            	 BuildRefIndexes (FALSE);
            	 break;   
            	 
            case IDM_REORGFILE:
                 {
                  DLGPROC	lpfnREORGMAPMsgProc;
                  lpfnREORGMAPMsgProc = MakeProcInstance((DLGPROC)REORGMAPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"REORGMAP", hWnd, lpfnREORGMAPMsgProc);
                  FreeProcInstance(lpfnREORGMAPMsgProc);  
                  if (nRc==2)
		     	  	PostMessage(hWnd, WM_COMMAND, IDM_BUILDREFINDEXES, 0L);
                 }
            	 break;
            
            case ID_OPTIONS_DISPLAYFILEPARAM:
            	 DisplayFileParam =  GWCheckMenuItem(hWnd, LOWORD(wParam));
            	 break; 

/*			case IDM_FULL_SCREEN:
				FullScreenVideo();
             	break;*/
	             
            case IDM_DISPLAY_VEHICLES:
				 if (/*GetFocus() == hWndMain &&*/ !InDisplayProcessing && !IgnoreAVLTimer)// && !WindowIsCovered (hWnd,1))
            		UpdateAllVehicles(TRUE); 
            	 HavePendingDisplay = FALSE;
                 break;
                 
            case IDM_SCREENTOCLIPBOARD:
            	ExpandText ("$SCREENTOCLIPBOARD()");
            	break;
            case IDM_TEST_ATTRIBUTE:
                 setDoPaint( FALSE);
				 
                 {
				    DLGPROC lpfnDISPLAY_GWD_DATAMsgProc;
				    LPSTR	pName;
				     
				    hName=GSSiGlobAlloc (  17,GHND,256);
					pName=GlobalLock(hName);
					GlobalUnlock(hName);	
				    lpfnDISPLAY_GWD_DATAMsgProc = MakeProcInstance((DLGPROC)DISPLAY_GWD_DATAMsgProc, hInst);
				    CreateDialog(hInst, (LPSTR)"DISPLAY_GWD_DATA", hWndMain, lpfnDISPLAY_GWD_DATAMsgProc);
//				    FreeProcInstance(lpfnDISPLAY_GWD_DATAMsgProc); 
//				    GSSiGlobFree (&hName);
                 }

				  setDoPaint( TRUE);
            	 break; 
            
            case IDM_CHANGE_COMBO_FILE:
            {    
            	 LPSTR	pName;
            	 BOOL	rc;
            	 
            	 GSSiGlobFree (&hCFName);
            	 hCFName = GSSiGlobAlloc (  18,GHND,256);
            	 pName = GlobalLock (hCFName); 
				 rc = GetFileName3 (hWnd,pName,IDS_FILTERGCF,IDS_FILEGCF);
			 	 GlobalUnlock (hCFName);
				 if (!rc)
				 {       
				 	GSSiGlobFree (&hCFName);
				 	break;
				 }
            	 goto EditCombo;
            	      
            }
            	 break;

            case IDM_COMBO_FILE:
            	 GSSiGlobFree (&hCFName); 
        EditCombo:
            {
                  DLGPROC	lpfnCOMBO_FILEMsgProc; 

                  lpfnCOMBO_FILEMsgProc = MakeProcInstance((DLGPROC)COMBO_FILEMsgProc, hInst);
                  DialogBox(hInst, (LPSTR)"COMBO_FILE", hWnd, lpfnCOMBO_FILEMsgProc);
                  FreeProcInstance(lpfnCOMBO_FILEMsgProc);
            }
            	 break;
            	 
            case IDM_EXIT:
            	 goto Close;
            	 break;

            case IDM_H_BASICS:
                 /* Place User Code to respond to the                   */
                 /* Menu Item Named "Basics" here.                      */
                 //lda addition
         		 setDoPaint( TRUE);
				{   
					
					hSTR=GSSiGlobAlloc (  19,GMEM_MOVEABLE,256);
					str=GlobalLock (hSTR); 
					_fstrcpy (str,"$OPENAPP([%WD]help\\gmhelp.chm)");
					ExpandText (str);
					//WinHelp(GetFocus(),str,HELP_KEY,(DWORD)"Help Contents");  
            	 	GSSiGlobUlFree (&hSTR);
					break; 
				}
                 //end of lda addition
                  break;

            case IDM_CREATE_LAYERINDX:
            {
            	
	           	 _getcwd (CurDir,256);
           	  	 SaveDrive = _getdrive(); 
            	 LoadMapDir(hWnd);
        		 _chdir (CurDir);
            	 _chdrive (SaveDrive);
            }
            	 break;

            default:
				goto ReturnDefault;
           }
         if (!HaltPaint)
         	setDoPaint( TRUE);
         break;        /* End of WM_COMMAND                             */
    
    case WM_SETCURSOR:
    {
    	extern	BOOL ddbug;
    	 if (idTimer)
    	 {
    	 	break;
    	 }
    	 if (ddbug)
    	 	ii=1;
    	 hCursor = VPCursor (hWnd);
         if (!hCursor)
			goto ReturnDefault;

//         if (GetCursor () != hCursor)  
//         	GSSiSetCursor (hCursor);
    } 
    	 break;
    	 
    case WM_CREATE: 
    {
		short	w,h; 
		HDC		hDC;
		SIZE	txSize;

		 HANDLE hSTR=GSSiGlobAlloc (  20,GMEM_MOVEABLE,1024);
		 LPSTR	str=GlobalLock (hSTR); 
    
    	 GSSiTrace ("Begin WM_CREATE",0);

				
		hDC = GetDC (hWnd);
		GetTextExtentPoint32 (hDC,"ABCDEFG",7,&txSize);
		w = txSize.cx; 
		h = txSize.cy; 
		SmallFontLargeFontFactor = (double)64/(double)w;
		ReleaseDC (hWnd,hDC);
		
/*         SetScrollRange(hWnd, SB_HORZ, 0, 100, FALSE);                 */
         sHPos = 0;    /* scroll bar is initially set to 0              */

/*         SetScrollRange(hWnd, SB_VERT, 0, 100, FALSE);                */
         sVPos = 0;    /*    scroll bar is initially 0                  */

/*         if (!CBInit ()) PostQuitMessage(0);*/
		 hWndMain = hWnd;
		 CDInit (hWnd, hInst); /* Initialize Common Dialogs */     
		 InitGraphics (hWnd);
/*		 if (_fstrstr (szAppName,"Highways"))
		 	SetWindowText (hWnd,"Visual Surveyor");
		 else*/ if (!MapServer && GetGlobalCVal ("[%WT]",str,0))
		 	SetWindowText (hWnd,str);
		 ConvertCoordClose();
		 ConvertCoordInit();
		 ConvertCoordClose();   
		 if (!OkToContinue(TRUE))
		    exit(1);
		 ProcessGlobal ("[%STARTCMD]");
		 OkToContinueTime = GetGlobalLVal2 ("[%OkToContinueTime]",0);

		 GSSiTrace ("End of WM_CREATE",0);         
		 GSSiGlobUlFree (&hSTR); 
		 HaveWMCreate = TRUE;
	}
        break;       /*  End of WM_CREATE                              */
    
		    
    case WM_DROPFILES: 
    {
    	HANDLE	hFile = (HANDLE)wParam;   
    	HANDLE	hFileMem=GSSiGlobAlloc (1525,GMEM_MOVEABLE,256);
    	LPSTR	pFile = GlobalLock (hFileMem);
    	int	n=0, LocAdded;  
    	LPVIEWPORT	SaveVP;
    	
		DragQueryPoint (hFile,&CursorLoc); 
		SelectViewport (CursorLoc,TRUE,FALSE,FALSE);  
		SaveVP = CurView; 
		while (DragQueryFile (hFile,n++,pFile,256)) 
    	{
    	 	//GetLongPathName (pFile,256);
    	 	if (FileType (pFile) == 1)
    	 	{
	    	 	SubstituteDL (pFile,FALSE);
				if (MapFileType (pFile,0,0) == MT_IMAGE)
				{
					DPOINT Point;
					HDIB32 hDib = LoadDIB32 (pFile,FALSE); 

					if (hDib)
					{
						char	DateTaken[32];

						if (GetImageCoord (hDib, &Point,DateTaken))
						{
							SetGlobalValueDPoint ("%DROPPEDCOORD",Point);
							SetGlobalValue ("%DROPPEDFILE",pFile);
							CenterWindow (Point,TRUE);
							ProcessText ("$MACRO([%DL]macros\\dropimage.txt)");
							continue;
						}
					}

				}
	    	 	if (!(LocAdded = AddFileToViewport (pFile)))   
	    	 	{
	    	 		CurView = SaveVP;
	    	 		break;  
	    	 	}
	    	 	CurView = SaveVP;
				if (CurVis)
					CurVis->FileIsVisible[LocAdded-1] = TRUE;  
	    	 	if (GetMapBounds (pFile,&Bounds))
					ZoomToRect(Bounds,FALSE);
			}
    	 } 
    	 GSSiGlobUlFree (&hFileMem);  
    	 DragFinish (hFile);
	}
    	 break;
	
	case WM_ENTERSIZEMOVE:
	{
		hDC = GetDC (hWnd);
		GetClientRect (hWnd,&MoveStartRect);
		setDoPaint( FALSE);
		hBMMove = SaveScreen (hDC,MoveStartRect);
		ReleaseDC (hWnd,hDC);
		DisplayAllToolbars (0);
	}
		goto ReturnDefault;

	case WM_EXITSIZEMOVE:
		setDoPaint( TRUE);
		GSSiDeleteObject (&hBMMove);
		GetClientRect (hWnd,&MoveEndRect);
		AdjustToolbarPositions ();
		DisplayAllToolbars (3);
		if ((MoveStartRect.right - MoveStartRect.left) != (MoveEndRect.right - MoveEndRect.left) ||
			(MoveStartRect.bottom - MoveStartRect.top) != (MoveEndRect.bottom - MoveEndRect.top))
		{
			setDoPaint(TRUE);
			PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
		}
		goto ReturnDefault;

	case WM_MOVE:
		goto ReturnDefault;

//	case WM_QUERYOPEN:
//		goto ReturnDefault;
//		break;
	case WM_WINDOWPOSCHANGING:
		{
			LPWINDOWPOS pWpos = (LPWINDOWPOS)lParam;
			int w = pWpos->cx;
			int h = pWpos->cy;

			if (MapServer)
			{
				RECT wrect, crect;

				if (w && h)
				{
					GetWindowRect(hWnd, &wrect);
					GetClientRect(hWnd, &crect);
					pWpos->cx += RECTWIDTH(&wrect) - RECTWIDTH(&crect);
					pWpos->cy += RECTHEIGHT(&wrect) - RECTHEIGHT(&crect);
					crect.left = crect.top = 0;
					crect.right = w;
					crect.bottom = h;
					SetMainRect(CurView->hWnd, CurView->hDC, &crect, 0);
					SetupViewports(CurView->hWnd, CurView->hDC, 0, MainRect, 0);
				}
				return 0;
			}
			if (w >0 || h > 0)
				goto ReturnDefault;
		}
		/*{
			LPVIEWPORT SaveVP = CurView;

	 		SetConfig(1);
			for (iview=0;iview<*pNumViewports;iview++) 
			{
				CurView = pViewports[iview];
				if (pViewports[iview]->hWndDlg)
					SendMessage(pViewports[iview]->hWndDlg, GSSI_REPOSITION,0, (LPARAM)pViewports[iview]); 
			}  
			CurView = SaveVP;
		}*/
//		if (hWndBGUpdateMsg)
//			ShowWindow (hWndBGUpdateMsg,SW_HIDE);
		goto ReturnDefault;

	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_MINIMIZE:
	    	HaltMapDisplay(FALSE,TRUE); 
			//SaveFullWindowBitmap (hWndMain);
				// if (hCacheThread)
				//	 ii = SuspendThread(hCacheThread);
			break;
		case SC_RESTORE:
				//if (hCacheThread)
				//	ii = ResumeThread(hCacheThread);
			if (hFullWindowBitMap && !IgnoreActivate) 
			{
				RestoreFullWindowBitmap ();		
				UpdateVehicleStatusDlg ();
				DisplayAllToolbars (4);
			}
			else
				RepaintServerInfo();

			break;
		}
		goto ReturnDefault;

	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam; // points to size and position data

			if (lpwp->hwnd == hWndMain)
			{
				if (IsIconic (hWndMain))
				{
					if (lastWP.x != SHRT_MIN)
						lastWP.x = SHRT_MIN;
				}
				else if (memcmp (&lastWP.x,&lpwp->x,sizeof(int)*4))
				{
					if (lastWP.x == SHRT_MIN)
						RestoreFullWindowBitmap ();
					memcpy (&lastWP.x,&lpwp->x,sizeof(int)*4);
					DisplayVPDialogs (TRUE);
					if (hWndBGUpdateMsg)
						PostMessage (hWndBGUpdateMsg,WM_COMMAND,IDOK,0);
				}
			}
			goto ReturnDefault;
		}
		break;
	case GF_PROCESS_BACKGROUND_CMD:
       	 if (BackgroundTask && HaveWMCreate) 
         {  
			if (OpenConfig(hWnd,0))
            	ExpandText (BackgroundMacro);
			if (atoi (BackgroundMacro))
				PostMessage(hWnd, WM_CLOSE, 0, 0L);
         }
		break;
	case WM_ENABLE:
		goto ReturnDefault;
//	case WM_NCCALCSIZE:
//		ii=1;
//		break;
//	case WM_NCACTIVATE:
//		ii=1;
//		break;
	case WM_ACTIVATE:
		{
			switch (LOWORD(wParam))
			{
			case WA_INACTIVE:
				activeCount--;
				if (GetToolbarIDFromWnd ((HWND)lParam) < 0)
					DisplayAllToolbars (0);

				break;
			case WA_ACTIVE:
				ii=1;
			case WA_CLICKACTIVE:
				activeCount++;
				if (GetToolbarIDFromWnd ((HWND)lParam) < 0)
				{
					if (hFullWindowBitMap && !IgnoreActivate) 
						RestoreFullWindowBitmap ();					
					UpdateVehicleStatusDlg ();
					DisplayAllToolbars (4);
				}
				break;
			}
			goto ReturnDefault;
		}
	case WM_IME_NOTIFY:
		goto ReturnDefault;
		break;
	case WM_SHOWWINDOW:
		if (wParam)
		{
			if (hFullWindowBitMap && !IgnoreActivate)
				RestoreFullWindowBitmap();
			UpdateVehicleStatusDlg();
			DisplayAllToolbars(4);
		}
		else
			SaveFullWindowBitmap(hWndMain);

		goto ReturnDefault;
	case WM_IME_SETCONTEXT:
		if (HavePaint)
		{
			if (!wParam)
			{
				if (hFullWindowBitMap && !IgnoreActivate)
					RestoreFullWindowBitmap();
				UpdateVehicleStatusDlg();
				DisplayAllToolbars(4);
			}
			else if (!InDisplayProcessing)
				SaveFullWindowBitmap(hWndMain);
		}
		goto ReturnDefault;
	case WM_SIZE:     /*  code for sizing client area                   */
    	 ConfigDisplayRect.left = ConfigDisplayRect.right = 0;
         switch (wParam)
           {
            case SIZE_MINIMIZED:  
		         setDoPaint( FALSE);
		         SaveHavePaint = HavePaint;
		         HavePaint = FALSE;
		         HaveSeg = FALSE; 

            	if (BufferedScreen)
            		break;
		         if (CurView)
		         	CloseMap(FALSE);
                 break;
			case SIZE_MAXIMIZED:
				AdjustToolbarPositions();
				setDoPaint(TRUE);
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
				break;
            case SIZE_RESTORED:  
            {
				HaltMapDisplay(FALSE, TRUE);
				hDC = GetDC (hWnd);
				HavePaint = SaveHavePaint;		
				GetClientRect (hWndMain,&WindowRect); 
				NormalRect (&WindowRect);  
				//RestoreScreenRect (hDC,hBMMove,MoveStartRect,WindowRect);
				RestoreFullWindowBitmap();
				ReleaseDC (hWnd,hDC);
				AdjustToolbarPositions ();
				if (EqualRect (&FullWindowBitMapRect,&WindowRect))
            		break;
            }
            default: 
                break;

           }
//           if (hWndPrompt)
//		       PostMessage(hWndPrompt, GSSI_REINITDIALOG,0, 0L);  
		 
           ButtonFuncOpt=0;
		   goto ReturnDefault;

   	case WM_TIMER:
   		if (wParam > TCPTIMER && wParam <= TCPTIMER + MAXOPENSOCKETS) 
   			ProcessSocketTimer (wParam);
   		else
         switch (wParam)
           {  
            default:
            	ii=1;
            	break;
            case 1:
            {	
            	clock_t StartTime,EndTime; 
            	long	DiffTime;
				static	int lastVPID=1;
            	 
				 SetConfig (-1);
            	 if (!*pNumViewports || NoDisplay || InImediate || !DoPaint() || InTime) break; 
            	 InTime = TRUE;
               	 StartTime = GetTickCount();
                 if (Counter <MaxTimePerSeg)
	        	 	Counter = MaxTimePerSeg;  
	        	 hDC = GetDC   ( hWnd );
				 SaveDC (hDC);
	        	 ReopenMap (TRUE);
	        	 TrapKillTimer=FALSE; 

				 while (HaveMapTimer()&&Counter>0)
				 {	
					HaveSeg = TRUE;
//					if (OpenMap (hWnd, hDC))
					{   
						HDC	hDCBuf = HaveScreenBuffer (0);
						short	iview;
						
						if (hDCBuf && hDCBuf != CurView->hDC)
							for (iview=0;iview<*pNumViewports;iview++)  
								pViewports[iview]->hDC = hDCBuf; 
						iview = CurView->ID;
						if (CurView->ID == 27)
							ii=1;
						if (!DisplaySeg (&CurView->hDC,FALSE)) 
							EndDisplayProcessing (TRUE);
						else if (MemMap && _fstricmp (MemMapName,"%SCREEN"))
							HaltMapDisplay(FALSE, TRUE);
						else 
						{   
							EndTime = GetTickCount();
							DiffTime = EndTime - StartTime;
							Counter-=DiffTime;
							StartTime = EndTime; 
						}
					}
/*					else 
					{   
						CloseMap (FALSE);
						EndDisplayProcessing (TRUE);
						Counter = -1;               
					}*/
				 }
				 if (Counter > 0)
					 lastVPID = 1;
				 else if (*pNumViewports && CurView)
					 lastVPID = CurView->ID;
				 if (HaveScreenBuffer (0)) 
				 {
					char	txt[32];
					
					if (DisplayPartialBuffer)
						ShowBufferedScreen (TRUE,TRUE,0,0);
					if (!Blocks)
					{
				 		if (hSaveWindowText)
						{
							str = GlobalLock (hSaveWindowText);
							SetWindowText (hWnd,str);
							GSSiGlobUlFree (&hSaveWindowText);
						}
						else
						{
							hSaveWindowText = GSSiGlobAlloc (1526,GMEM_MOVEABLE,256);
							str = GlobalLock (hSaveWindowText);
							GetWindowText (hWnd,str,255); 
							GlobalUnlock (hSaveWindowText);
							Blocks++;
						}
					}
					else 
					{
						Blocks++;		
						sprintf (txt,"%ld blocks processed",Blocks);
						SetWindowText (hWndMain,txt);  
					}
				 }
				 TrapKillTimer=TRUE;
				 ReopenMap (FALSE);
				 RestoreDC (hDC,-1);
				 ReleaseDC     ( hWnd, hDC );   
				 if (InDisplayProcessing && !idTimer)
				 	ii=1;
				 InTime = FALSE; 
				 if (!idTimer && !CurrentConfig)
				 {
				 	SetConfig(1);
				   	InvalidateRect (hWndMain,&ConfigDisplayRect,TRUE);
				 }
			}
                 break;

            case 2:
//	 		  	 SendUMMessage ("%STALIVE");
                 break;

            case 3:/* Timeout waiting for response */
		      	 GSSiMsgBox( GetFocus(),"Timeout waiting for response",
	    			 	    "Fatal Error", MB_OK,0);
				 KillTimer(hWndMain, 3);
				 goto Close;
                 break; 
                 
            case 4:
 				 if (!DisplayNetMarkerThemeLegend(FALSE)) 
 				 {
 					KillTimer(hWndMain,NetMarkTimer);  
 					NetMarkTimer = 0; 
 				 }
            	 break; 
            	 
            case 5:
 				 if (!DisplayStreetAddresses(FALSE)) 
 				 {
 					KillTimer(hWndMain,StreetEditTimer);  
 					StreetEditTimer = 0; 
					if (!MemMap)
						SaveFullWindowBitmap (hWndMain);
 				 }
            	 break;
            
            case 6: //vehicle display timer 
//				 if (GetFocus() == hWndMain) 
				 if (!InDisplayProcessing && !IgnoreAVLTimer)
				 {
					time_t	curtime;     
					time (&curtime); 
					ComputeDummyVehicleLocations (AVLFactor*(curtime - AVLStartTime));
            		UpdateAllVehicles(TRUE); 
            	 }
//            	 ProcessCOMMNotification(hWndMain,0,0 );
            	 break; 
            	 
            case 7: //Infobox edit timer 
            	 KillTimer(hWndMain,7);
   	 			 InfoBoxEditTimer = 0;  
				 ProcessInfoBoxPickMacroFile (EditInfoBox);
            	 EditInfoBox = 0; 
            	 break;   
            case 8: //OkToContinue timer 
            {
            	 short st;
            	 
            	 KillTimer (hWnd,OKTOCONTINUETIMER);
            	 st = OkToContinue (TRUE);
            	 if (!st)
					BlowOut(0,0); 
	 			 SetTimer(hWnd, OKTOCONTINUETIMER, OkToContinueTime, (TIMERPROC) 0);
	 			 if (st == 3)
	 			 {
		 			 if (*UserName)  
		 			 {  
		 			 	BOOL	SaveSE=ShareEnabled;
		 			 	
		 			 	ShareEnabled = TRUE;
		 			 	AppendFile ("[%%DL]whosin.txt",UserName);
		 			 	ShareEnabled = SaveSE;
		 			 } 
		 		 }
		 	}
				 break;

            case 10: //GPS Timer
 				 CloseDigConnection ();
   				 OpenDigConnection(hWndMain);
		     	 PostMessage(hWndMain, WM_COMMAND, IDM_DISPLAY_VEHICLES, 0L);
          	 	 break;
			
			case SUICIDE_TIMER:
			{
								  static BOOL test = TRUE;
								  if (test)
								  {
									  test = FALSE;
									  ProcessText("$MAPSERVER(TEST)");
								  }
								  if (MapServerCalledFromWnd && !WindowExists(MapServerCalledFromWnd))
								  {
									  PostMessage(hWndMain, WM_COMMAND, IDM_EXIT, 0L);//allows imediate processing to terminate 
								  }
			}
				break;

			case GF_WHEELZOOM: //WheelZoom Timer
				 WheelZoom (0,0,1);
				 break;

			case TCPREPLAYTIMER:
				ProcessReplayTCPTimer ();
				break;
            }
		 break;  
		 
    case WM_LBUTTONDOWN:
		   if (wParam & MK_RBUTTON)
//       	   if (wParam == (MK_LBUTTON|MK_RBUTTON))
       	   		ButtonFuncOpt = 1; 		/* indicates R button down when L button pressed */
       	   else
       	   		ButtonFuncOpt = 2;
		   goto ReturnDefault;

/*    case WM_LBUTTONUP:
    	if (ButtonFuncOpt)
    	{	MousePoint.x = LOWORD(lParam);
	       	MousePoint.y = HIWORD(wParam);
	       	MouseInput (hWnd,Message,MousePoint,ButtonFuncOpt);
	    }
       	break;
*/  
	case WM_SYSKEYDOWN:
		switch (wParam)
		{
		 	default:
		    goto ReturnDefault;
		}
		break;
    case WM_KEYDOWN:
		if ((GetKeyState(VK_SHIFT) & 0x1000) &&
			(GetKeyState(VK_CONTROL) & 0x1000))
		{
			switch (wParam)
			{
				case 'D':
						SetDebug (TRUE);
				break;
				case 'E':
					if (GetDebug ())
						EditViewportAtCursor (hWnd);
				break;
				case 'T':
					if (GetDebug ())
						EditFundirAtCursor (hWnd);
				break;
			}
		}
		else switch (wParam)
		{
			case 27:  //ESC   
				EscapeFunction (TRUE);
	    	break;
			case VK_F9:
				if ((GetKeyState(VK_SHIFT) & 0x1000) &&
					(GetKeyState(VK_CONTROL) & 0x1000))
				
				 GMEdit (hWnd,"[%DL]global.ini"); 
				 break;
	    	
			case VK_F12:
				if ((GetKeyState(VK_SHIFT) & 0x1000))
					goto LoadGFMenu;  
				break;
		 	default:
				goto ReturnDefault;
		}
		break;
    case WM_RBUTTONDOWN:
    	GSSiTrace ("Got RBUT",0);
//       	   if (wParam == (MK_LBUTTON & MK_RBUTTON))
       	   if (wParam & MK_LBUTTON)
LoadGFMenu:
	   	   {
		    DLGPROC lpfnRBUTOPSMsgProc;
			HCURSOR	SaveCursor=GSSiSetCursor (LoadCursor(0, IDC_ARROW));     
            
            GSSiTrace ("Invoke GF menu",0);
		    lpfnRBUTOPSMsgProc = MakeProcInstance((DLGPROC)RBUTOPSMsgProc, hInst);
		    nRc = DialogBox(hInst, (LPSTR)"RBUTOPS", hWnd, lpfnRBUTOPSMsgProc);
		    FreeProcInstance(lpfnRBUTOPSMsgProc);
			GSSiSetCursor (SaveCursor);
		    if (nRc)
		    {
                if (hAddGraphicsFun)
                {
                	LPSTR	pCmd=GlobalLock (hAddGraphicsFun); 
                	
                	SetContinueProcessing ( TRUE);
					if (wParam & MK_LBUTTON)
//       	       	    if (wParam == (MK_LBUTTON|MK_RBUTTON))
                		HaveCurrentLBUTTON=TRUE;
                	AddGraphicsCmd (hWndMain,pCmd,FALSE,0); 
                	GSSiGlobUlFree (&hAddGraphicsFun);
                }
		    }
		    goto Return0;

		   }
	   goto ReturnDefault;

/*    case WM_MENUSELECT:
    {
		UINT	uItem = (UINT) LOWORD(wParam);   // menu item or submenu index 
		UINT	fuFlags = (UINT) HIWORD(wParam); // menu flags 
		HMENU	hmenu = (HMENU) lParam;          // handle to menu clicked 
        UINT	IDMess;
		MENUITEMINFO mii;
		UINT	uItem2=0;
        if (hmenu)
		{
			while (GetMenuItemInfo(hmenu,uItem2,TRUE,&mii))
				uItem2++;
		}
	    return DefWindowProc(hWnd, Message, wParam, lParam); 
	}
    	break;    //causing page fault in boundscheck on some menus*/
    	  
	case WM_NCHITTEST: 
	{
		long	lRetVal; 
		UINT	IDMess;
		
	    lRetVal = DefWindowProc(hWnd, Message, wParam, lParam); 
	    switch (lRetVal)
	    { 
	    	case HTCLIENT: 
				{
					HWND hfWnd = GetFocus ();

					inClientMain++;
					if (inClientMain == 1)
					{
						if (hfWnd != hWnd)
						{
							if (!inOpenFileDialog)
							{
								if (GetToolbarIDFromWnd(hfWnd) < 0)
								{
									if (!WindowIsCovered(hWnd, 2))
									{
										DisplayVPDialogs(FALSE);
										//DisplayAllToolbars (5);
										SetFocus(hWnd);
									}
									else
									{
										DisplayAllToolbars(1);
										SetFocus(hWnd);
									}
								}
								else
									SetFocus(hWnd);
							}
						}
					}
					GSSiSetCursor (hCursor);
				}
				ClearToolbarTrackEvents (0);
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
	    		return lRetVal;
}
 //       	IDMess = GetMenuPrompt (wParam); set prompt for menu items
 //     SetPrompt (IDMess,FALSE);
	    	default:   
				inClientMain = 0;
	    		if (LastVP)
				{
			    	NotifyFunction (LastVP,GF_EXIT_VIEWPORT);
					BackgroundUpdateMessage ("!REDISPLAY!");
				}
			    LastVP = 0;  
//	    		sprintf (str,"%ld",(long)lParam);
//	    		SetGlobalValue ("%M",str);
	    		
//	    		IDMess=PRMT_USERMESS;
				IDMess=0;
	    	break;
	    }
		SetPrompt (IDMess,FALSE); 
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
        return lRetVal;
}
	}
        break;

	case WM_MENUSELECT:
	{
		UINT uItem = (UINT) LOWORD(wParam);   // menu item or submenu index 
		UINT fuFlags = (UINT) HIWORD(wParam); // menu flags 
		HMENU hmenu = (HMENU) lParam;          // handle to menu clicked 

	}
	break;
 
        
    case WM_INITMENUPOPUP:
    {
    	short	ii;   
    	HMENU	hMenu=GetMenu(hWnd);
    	if (!hUserMenu && hMenu)
    	{
			HaltMapDisplay(TRUE, TRUE);
			if (hHighlight)
				EnableMenuItem(hMenu, IDM_Z_HLTLIMITS, MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(hMenu, IDM_Z_HLTLIMITS, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
	    	if (GetGlobalBVal ("[%DISPLAYHLTPATTERN]"))
	    		CheckMenuItem(hMenu, IDM_DISPLAY_HLT_PATTERN, MF_BYCOMMAND | MF_CHECKED); 
	    	else
	    		CheckMenuItem(hMenu, IDM_DISPLAY_HLT_PATTERN, MF_BYCOMMAND | MF_UNCHECKED); 
	    	if (AutoClearOffset)
	    		CheckMenuItem(hMenu, IDM_AUTO_CLEAR_OFFSET, MF_BYCOMMAND | MF_CHECKED); 
	    	else
	    		CheckMenuItem(hMenu, IDM_AUTO_CLEAR_OFFSET, MF_BYCOMMAND | MF_UNCHECKED); 
	    	if (MaskOffsetLine)
	    		CheckMenuItem(hMenu, IDM_MASK_OFFLINE, MF_BYCOMMAND | MF_CHECKED); 
	    	else
	    		CheckMenuItem(hMenu, IDM_MASK_OFFLINE, MF_BYCOMMAND | MF_UNCHECKED); 

			CheckMenuItem(GetMenu(hWnd), IDM_MEDIUMBM, MF_BYCOMMAND | MF_UNCHECKED); 
			CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_UNCHECKED);
	    	switch (BitmapSizeOpt)
	    	{
	    		case 1:
					CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_CHECKED);
					break; 
	    		case 2:
					CheckMenuItem(GetMenu(hWnd), IDM_MEDIUMBM, MF_BYCOMMAND | MF_CHECKED);
					break; 
	    		case 3:
					CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_CHECKED);
					break;
			} 
	    }
    	
    }
    goto ReturnDefault;
    
    case WM_SETFOCUS: 
    if (DoPaint ())
	{
		RECT	Rect;
		HDC hDC = HaveScreenBuffer (&Rect);

//		if (!WindowIsCovered(hWnd, 1))
//			setDoPaint(__LINE__, __FILE__, TRUE);
		if (hDC)
		{
			HDC	hDCMain = GetDC (hWnd);
			    
		    SaveDC (hDCMain);
			SetDisplayMode (hDC, GF_SCREENMODE); 
		   /* SetWindowOrgEx  ( hDC, 0, 0,0 );
		    SetViewportOrgEx( hDC, 0, 0,0 );    
		    SetMapMode    ( hDC, MM_TEXT );*/
		  	SelectClipRgn ( hDC,0);
		    i=BitBlt(hDCMain, Rect.left,Rect.top,
		                Rect.right-Rect.left+1,
		                Rect.bottom-Rect.top+1,
		           hDC, 0,0,  SRCCOPY);
		    RestoreDC (hDCMain,-1);  
		    ReleaseDC (hWnd,hDCMain);
			GdiFlush();
		}
		else if (hWnd == hWndMain)
			RestoreFullWindowBitmap ();
		UpdateVehicleStatusDlg ();		     
	}
    goto ReturnDefault;
    
    case WM_KILLFOCUS:
		//goto ReturnDefault;
		if (hWnd == hWndMain)
		{
			if (!WindowIsCovered(hWnd, 1) && !MemMap && !InDisplayProcessing)
				SaveFullWindowBitmap((HWND)-1);
		}
		goto ReturnDefault;
    
	case WM_PRINT:
		for (i = 0; i < *pNumViewports; i++)
			pViewports[i]->hDC = (HDC)wParam;
		ProcessText("$REDISPLAY(T)");

		break;
	case	WM_PRINTCLIENT:
		ii = 1;
		break;

    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */ 
    {
    	 RECT	UpdateRect;
		 HDC	hDCScreen;
		// if (!RunFromCache)
		//	 StartBackgroundCache ();

//         GSSiTrace ("Enter WM_PAINT");
         if (InPaint || Printing || idTimer || InDisplayProcessing==1)
			goto ReturnDefault;
		 InDisplayProcessing = 0;
			  
	     if (!GetUpdateRect (hWnd,&UpdateRect,TRUE))
	     {
//		 	if (DoPaint)
//				PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
			goto ReturnDefault;
	     }
         _fmemset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = hDCScreen = BeginPaint(hWnd, &ps);
		 if (hFullWindowBitMap && (int)hFullWindowBitMap != -1)
		 {    
			RECT	WindowRect;
					
			GetClientRect (hWndMain,&WindowRect);
			NormalRect (&WindowRect);
//			NormalRect (&FullWindowBitMapRect);   
			if (EqualRect (&FullWindowBitMapRect,&WindowRect))
			{
		        SelectClipRgn (hDC,0);
				if (!ShowBufferedScreen (TRUE,TRUE,0,&UpdateRect))
					RestoreScreen (hDC,hFullWindowBitMap,WindowRect);	
				DisplayVPDialogs (FALSE);
				if (hWndBGUpdateMsg)
					PostMessage (hWndBGUpdateMsg,WM_COMMAND,IDOK,0);
                EndPaint(hWnd, &ps);
				goto Return0;
		    }
		    else
				ClearFullWindowBitmap (0);
		 } 
         InPaint = TRUE;
         GSSiTrace ("Begin WM_PAINT",0);
         if (MemMap)
         {
			BITMAP	bm; 
			RECT	Rect;
			int		i,j;
            
            if (InServerMode)
            {
            	RECT	rect;
            	char	ImageFile[]="[%DL]geomaster.bmp";
            	
            	GetClientRect (hWnd,&Rect);
				FillRectPoly (hDC,&Rect,RGB(255,255,255));  
				if (ExistFile (ImageFile))
            		DisplayBMFileInRect (hDC,ImageFile,Rect,TRUE);
            }
            if (!hMemBitmap)
            {
				hdcMemMap = CreateCompatibleDC(hDC);    
				hMemBitmap = CreateCompatibleBitmap (hDC,(int)MemMapWidth,(int)MemMapHeight);
				hbmpOld = SelectObject(hdcMemMap, hMemBitmap);
			} 
			OldDC = hDC;
			hDC = hdcMemMap;
			SelectClipRgn (hDC,0);			
			SetDisplayMode (hDC,GF_TEXTMODE);
			
			Rect.left=0;
			Rect.top=0;
			Rect.bottom=MemMapHeight;
			Rect.right=MemMapWidth;  
			FillRectPoly (hDC,&Rect,RGB(255,255,255));  
			FirstMemMap = TRUE;
			
         } 
 /*        else
		 {
			 if (HaveSeg && DoPaint)
			 {
				 if (NumPaint++ % 2)
				 {
					 EndPaint(hWnd, &ps);
					 if (DoPaint)
					 {
						 //				     	PostMessage(hWnd, WM_COMMAND, IDM_REDISPLAY, 0L);
						 PaintMap(hWnd, hDC, FALSE, &UpdateRect, 0);
						 RedisplayLastPrompt();
					 }
					 InPaint = FALSE;
					 break;

				 }
			 }
            
         }*/
         /* Included in case the background is not a pure color         */
         SetBkMode(hDC, TRANSPARENT);
         
 		 SetDisplayMode (hDC,GF_TEXTMODE);
		 SelectClipRgn (hDC,0);

//         if (DoPaint) HaveSeg = FALSE; 
         if (!MemMap || FirstMemMap)
         {
	         PaintMap(hWnd,hDC,FALSE,&UpdateRect,0);
	         RedisplayLastPrompt ();
	     }
		 FirstMemMap=FALSE;
		 SelectClipRgn (hDC,0);
		 PaintServerInfo (hDCScreen,&ps.rcPaint);
         EndPaint(hWnd, &ps); 
         GSSiTrace ("End WM_PAINT",0);
         InPaint = FALSE; 
		 SetFocus (hWnd);
		 GdiFlush();
    }
         break;       /*  End of WM_PAINT                               */

    case WM_VSCROLL:
         switch(LOWORD(wParam))
           {
            case SB_LINEDOWN:
                 sVPos += 10;
                 break;

            case SB_LINEUP:
                 sVPos -= 10;
                 break;

            case SB_THUMBPOSITION:
                 sVPos = HIWORD(wParam);
                 break;

            case SB_PAGEUP:
                 sVPos -= PlotPageHeight;
                 break;

            case SB_PAGEDOWN:
                 sVPos += PlotPageHeight;
                 break;
            
            case SB_ENDSCROLL:
		         HaltMapDisplay (FALSE,TRUE);
		 		 PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L); 
		 		 goto Return0;

            default:
				goto ReturnDefault;
           }
         sVPos = max(0,min(sVPos,PlotScrollHeight));
         SetScrollPos(hWnd, SB_VERT, sVPos, TRUE);
         break;

    case WM_HSCROLL:
         switch(LOWORD(wParam))
           {
            case SB_LINEDOWN:
                 sHPos += 10;
                 break;

            case SB_LINEUP:
                 sHPos -= 10;
                 break;

            case SB_THUMBPOSITION:
                 sHPos = HIWORD(wParam);
                 break;

            case SB_PAGEUP:
                 sHPos -= PlotPageWidth;
                 break;

            case SB_PAGEDOWN:
                 sHPos += PlotPageWidth;
                 break;

            case SB_ENDSCROLL:
		         HaltMapDisplay (FALSE,TRUE);
		 		 PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L); 
		 		 goto Return0;
            default:
				 goto ReturnDefault;
		 }
         sHPos = max (0,min(sHPos,PlotScrollWidth));
         SetScrollPos(hWnd, SB_HORZ, sHPos, TRUE);
         break;

    case WM_CLOSE:  /* close the window                                 */
         /* Destroy child windows, modeless dialogs, then, this window  */
         /* Destroy child windows, modeless dialogs, then, this window  */
        // added by lda, but removed when we started using ODBC
            // DdeNameService(idInst, 0, 0, DNS_UNREGISTER); // unregister all services
            //  UnHszize();
            // DdeUninitialize(idInst);
        // end of lda addition  
		 PostMessage(hWndMain, WM_COMMAND, IDM_EXIT, 0L);//allows imediate processing to terminate 
		 break;
Close:   HaltMapDisplay(TRUE,FALSE);
		 if (hWndVehTime)
			 DestroyWindow (hWndVehTime);
		 DisplayToolbars = FALSE;
		 OpenTCPReplay (0);
		 if (*pNumViewports && GetGlobalBVal2 ("[%AUTOSAVECFG]",FALSE)) 
		 { 
		    BOOL rtn=TRUE;
			int	 autoSave = GetGlobalLVal ("[%AUTOSAVECFG]");

			if (autoSave == 3)
			{
				SetGlobalValueBool ("%CFGSAVEPROMPT",FALSE);
				autoSave--;
				rtn = FALSE;
			}
		 	if (autoSave == 2 && ConfigChangesMade)
			{
				char	pw[32],str[32]="";

				rtn = (GSSiMessageBox (0,"Do you wish to save configuration changes?","Save updates",MB_ICONQUESTION|MB_YESNO,0) == IDYES);
				if (rtn && GetGlobalCVal ("[%CFGSAVEPW]",pw,0))
				do
				{
	   	  			rtn = GetTextString (hWnd,str,32,"Enter password","",str,0,1,0);
				}while (rtn && strcmp (str,pw));
			}
			if (rtn)
			{
				SaveGlobals = TRUE;
		 		SaveZoom = TRUE;  
		 		SaveCfgImage = GetGlobalBVal2 ("[%CFGSAVEIMAGE]",FALSE);
				DoSaveConfig (hWnd,TRUE);
			}
		 }
         DestroyWindow(hWnd); 
         goto Return0;
         
 	case WM_DESTROY: 
//		 DeleteObject (hBackBrush1); 
//		 DescScan (-4,0); 
         if (hWnd == hWndMain) 
         {
			 CloseMap (FALSE);
			 CloseRefIndex (TRUE);
	         QuitGraphics();    
			 hWndMain = 0;
	         //DdeBye();
#if ENABLETRACE
GSSiEnterProg (0);
#endif
             PostQuitMessage(0);  /* Quit the application                 */
         }
        break;

    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should retrn the message to Windows   */
         /* for default message processing.                             */
ReturnDefault:
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
         return DefWindowProc(hWnd, Message, wParam, lParam);
}
Return0:
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
         return 0;
}
   }
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
 return 0L;   
}
#if ENABLETRACE
}
#endif
}


LRESULT CALLBACK GetMsgProc(
  int code,       // hook code
  WPARAM wParam,  // removal flag
  LPARAM lParam   // address of structure with message
  )
{
	if (code < 0)                         // MUST retrn DefHookProc()
	{
      return CallNextHookEx (MHookFunc,code, wParam, lParam);
	}
	if (hWndSlider)
	{
		  LPMSG lpmsg = (LPMSG) lParam; 
		  
	}
    return 0;//CallNextHookEx (MHookFunc,code, wParam, lParam);
}
 
int FAR PASCAL FilterFunc(int nCode,WPARAM wParam,LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (456);
#endif
{ 
  DWORD LastlParam;
  WORD  LastwParam, hw;
  LPMSG lpmsg = (LPMSG) lParam;  
  short	ii;

  if (nCode < 0)                         // MUST retrn DefHookProc()
{
#if ENABLETRACE
GSSiExitProg (456);
#endif
      return DefHookProc(nCode, wParam, lParam,(HHOOK *) &Func);
}
  if (lpmsg->message == WM_SHOWWINDOW)
  	ii=1;
  if (lpmsg->message == WM_F1DOWN)
  	ii=1;
  if((nCode == MSGF_DIALOGBOX || nCode == MSGF_MENU) &&
     lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_F1)
     { 
      //  lpmsg = (LPMSG) LastlParam;
      //  if(lpmsg->hwnd == 0)lpmsg = (LPMSG) lParam;
        PostMessage(hWndMain, WM_F1DOWN,lpmsg->wParam,lpmsg->lParam);
{
#if ENABLETRACE
GSSiExitProg (456);
#endif
        return 1L;
}
     }
     if(Ready && !GotItUp)
     {
       if(lpmsg->wParam == 'm' || lpmsg->wParam == 'M')
       { 
      //  lpmsg = (LPMSG) LastlParam;
      //  if(lpmsg->hwnd == 0)lpmsg = (LPMSG) lParam;
        if(GetAsyncKeyState(VK_CONTROL) & 0X8000)
        { 
          ControlM = TRUE;
          PostMessage(hDynamicDialog,WM_COMMAND,0,(long) IDM_CONTROL_M);
{
#if ENABLETRACE
GSSiExitProg (456);
#endif
          return 1L;
}
        }  
       }
       else if(lpmsg->message == WM_KEYDOWN && lpmsg->wParam == VK_TAB)
       {
          PostMessage(hDynamicDialog,WM_COMMAND,0,(long) VK_TAB);
{
#if ENABLETRACE
GSSiExitProg (456);
#endif
          return 1L;
}
       
       }
     }  
{
#if ENABLETRACE
GSSiExitProg (456);
#endif
     return 0;
}
#if ENABLETRACE
}
#endif
}      

  
 



/* MATHERR.C: To use _matherr, you must turn off the
 * Extended Dictionary flag within the Microsoft
 * Programmer's WorkBench environment, or use the /NOE
 * linker option outside the environment. For example:
 *     CL _matherr.c /link /NOE
 */
//#include <math.h>
//#include <string.h>
//#include <stdio.h>
/* Handle several math errors caused by passing a negative argument
    * to log or log10 (_DOMAIN errors). When this happens, _matherr returns
 * the natural or base-10 logarithm of the absolute value of the
 * argument and suppresses the usual error message.
 */
/*int _matherr( struct _exception *except )
{
   if( except->type == _DOMAIN )
   {
      if( strcmp( except->name, "log" ) == 0 )
      {
         except->retval = log( -(except->arg1) );
         printf( "Special: using absolute value: %s: _DOMAIN error\n", except->name );
         return 1;
      }
      else if( strcmp( except->name, "log10" ) == 0 )
      {
         except->retval = log10( -(except->arg1) );
         printf( "Special: using absolute value: %s: _DOMAIN error\n", except->name );
         return 1;
      }
   }
   else
   {
      printf( "Normal: " );
      return 0;  
   } 
   return 0;
} */

void usrerr(const char *fmt, ...)
{
	MessageBox (0,fmt,0,0x00000030L);
	return;
}


