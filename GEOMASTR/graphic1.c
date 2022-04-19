#include "graphint.h"          
#include "umio.h"
#include "extrndb.h" 
#include "std.h"          
#include "gps.h"
#include "CRAPI.h"

#include "gmextern.h"

#include <sys\types.h>
#include <sys\stat.h>     

static BOOL doPaint = TRUE;

#define DEMODLL_API __declspec(dllimport)

BOOL DEMODLL_API ToggleMenuUnderline(int i);
    

static POINT    Points[4];

static long	StartHLTRef;
static BOOL	IncludePrompt=TRUE;
static BOOL	Wincap=TRUE;
static BOOL	CreateRefIndex=FALSE;
static BOOL	CreateTAGIndex=FALSE;
static RECT	ClipRectx;   

static	HHOOK	g_hook=NULL;
void AAShutDown(void)
;
void BlowOut (LPSTR Message, LPSTR Title)
#if ENABLETRACE
{GSSiEnterProg (1);
#endif
{   
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;  
	static		BOOL	InBlowOut=FALSE;

	if (isGMEdit)
		return;
	if (Message)
	{
		char messandtitle[1024];

		if (strlen (Message) + strlen (Title) < 1020)
		{
			sprintf (messandtitle,"%s\r\n%s",Title,Message);
			GSSiMsgBox (0,messandtitle,Title,MB_ICONEXCLAMATION|MB_TASKMODAL,0);
		}
		else
			GSSiMsgBox (0,Message,Title,MB_ICONEXCLAMATION|MB_TASKMODAL,0);
	}
	AbendWriter (Message,Title,0,0);
	if (!InBlowOut && GetGlobalBVal2("[%IGNORESYSMSG]",FALSE))
	{   
		char	AbortText[256];
		
		InBlowOut = TRUE; 
		GSSiRemove ("[%DL]sysmsg.txt");   
		if (GetGlobalCVal ("[%ABORTTEXT]",AbortText,0))
			AppendFile ("[%DL]abortlog.txt",AbortText);
	}
	if (!InQuitGraphics)
		QuitGraphics ();   
	
	exit (1);		                       
#if ENABLETRACE
}
#endif
}

void LogCode (LPSTR str)
{
	OFSTRUCTGM	OFStruct = { 0 };
	HANDLE Fid=OpenFileGM ("c:\\temp\\codes.txt",&OFStruct,OF_READWRITE);

	if (Fid == INVALID_HANDLE_VALUE)
		Fid=OpenFileGM ("c:\\temp\\codes.txt",&OFStruct,OF_CREATE);
	llFileSeek (Fid,0,2);
	BigWrite64 (Fid,str,strlen(str)+1,-1);
	BigWrite64(Fid,"\r\n",2,1);
	GSSiClose64 (&Fid);
	return;
}

LRESULT CALLBACK CBTProc(int nCode,
    WPARAM wParam,
    LPARAM lParam
	){
	int	ii;
	char	str[256];

	if (nCode < 0)
		return CallNextHookEx( 0,nCode,wParam,lParam);
	switch (nCode)
	{
	case HCBT_ACTIVATE:		// Specifies the handle to the window about to be activated. Specifies a long pointer to a CBTACTIVATESTRUCT structure containing the handle to the active window and specifies whether the activation is changing because of a mouse click.  
		{
			LPCBTACTIVATESTRUCT s = (LPCBTACTIVATESTRUCT)lParam;
			sprintf (str,"Activate:%i",wParam);
			LogCode (str);
		}
		return 0;
	case HCBT_CLICKSKIPPED:	// Identifies the mouse message removed from the system message queue. Specifies a long pointer to a MOUSEHOOKSTRUCT structure containing the hit-test code and the handle to the window for which the mouse message is intended. 
							// The HCBT_CLICKSKIPPED value is sent to a CBTProc hook procedure only if a WH_MOUSE hook is installed. For a list of hit-test codes, seeWM_NCHITTEST. 
		break;
	case HCBT_KEYSKIPPED:	// Identifies the virtual-key code. Specifies the repeat count, scan code, key-transition code, previous key state, and context code. The HCBT_KEYSKIPPED value is sent to a CBTProc hook procedure only if a WH_KEYBOARD hook is installed. For more information, seeWM_KEYUP orWM_KEYDOWN.  
		break;
	case HCBT_QS:			// Is undefined and must be zero. Is undefined and must be zero.  
		break;
	case HCBT_CREATEWND:	// Specifies the handle to the new window. Specifies a long pointer to a CBT_CREATEWND structure containing initialization parameters for the window. The parameters include the coordinates and dimensions of the window. By changing these parameters, a CBTProc hook procedure can set the initial size and position of the window.  
		{
			LPCBT_CREATEWND s = (LPCBT_CREATEWND)lParam;
			sprintf (str,"Window Created:%i %s",wParam,s->lpcs->lpszName);
			LogCode (str);
		}
		return 0;
	case HCBT_DESTROYWND:	// Specifies the handle to the window about to be destroyed. Is undefined and must be set to zero.  
			sprintf (str,"Window Destroyed:%i",wParam);
			LogCode (str);
		return 0;
	case HCBT_MINMAX:		// Specifies the handle to the window being minimized or maximized. Specifies, in the low-order word, a show-window value (SW_) specifying the operation. For a list of show-window values, see theShowWindow. The high-order word is undefined.  
			sprintf (str,"MinMax:%i",wParam);
			LogCode (str);
		return 0;
	case HCBT_MOVESIZE:		// Specifies the handle to the window to be moved or sized. Specifies a long pointer to aRECT structure containing the coordinates of the window. By changing the values in the structure, a CBTProc hook procedure can set the final coordinates of the window.  
		{
			RECT * s = (RECT *) lParam;
			sprintf (str,"MoveSize:%i",wParam);
			LogCode (str);
		}
		return 0;
	
	case HCBT_SETFOCUS:		// Specifies the handle to the window gaining the keyboard focus. Specifies the handle to the window losing the keyboard focus.  
			sprintf (str,"Set Focus:%i",wParam);
			LogCode (str);
		return 0;
	case HCBT_SYSCOMMAND:	// Specifies a system-command value (SC_) specifying the system command. For more information about system-command values, seeWM_SYSCOMMAND.  Contains the same data as the lParam value of aWM_SYSCOMMAND message: If a system menu command is chosen with the mouse, the low-order word contains the x-coordinate of the cursor, in screen coordinates, and the high-order word contains the y-coordinate; otherwise, the parameter is not used.  
			sprintf (str,"SysCommand:%i",wParam);
			LogCode (str);
		return 0;
	}
	return 0;
}

void PATBMPDef (BOOL clear)
{
	UINT	PatBMP[5] = { IDB_94PCT, IDB_50PCT, IDB_25PCT, IDB_06PCT, IDB_00PCT };

	int i;
	for (i = 0; i < 5; i++)
	{
		if (clear)
			GSSiDeleteObject(&hPatBMP[i]);
		else
			hPatBMP[i] = LoadBitmap(ghInst, MAKEINTRESOURCE(PatBMP[i]));
	}
}

BOOL SetContinueProcessing(BOOL set)
{
	BOOL rtn = ContinueProcessing;
	if (set == -1)
		ContinueProcessing = FALSE;
	else
		ContinueProcessing = set;

	if (!set)
		ii = 1;
	return rtn;
}
BOOL InitGraphics (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (2);
#endif
{ 
	char	str[256], name[MAX_PATH];
	short	l, OffsetChoice,i;

	PATBMPDef(FALSE);
	SetSavedGraphicsFid (0);
	//_controlfp (,); controls floating point exceptions
    for (i=0;i<MAXMULTILEVELZOOM;i++)
    	hbmpMultiLevel[i] = 0;   
    _fstrcpy (CurHelpTopic,"topic5");
	DragAcceptFiles (hWnd,TRUE);   
	SysStartTime = GetTickCount();  
	CreateFidSmall (); 
//	CreateFidDBF ();
	InitProj4CoordConv (FALSE);
	GoogleTilesInit ();
	hCmdMess=GSSiGlobAlloc (  39,GHND,MAX_CMDMESSAGE);  
    GetCurVal (name,sizeof(name),IDS_FILEVPOFF); 
	LocationChoice = FillList (0,0,name,str,0);
	LocationOffset = atof(str);
	l = _fstrlen(str);
	if (str[l-1]=='f' || str[l-1]== 'F') LocationOffset *= FTM;
	
	if (!GetGlobalCVal ("[%OFFSETLINEDIST]",str,0))
	{
	    GetCurVal (name,sizeof(name),IDS_FILEOFFLINE); 
		OffsetChoice = FillList (0,0,name,str,0); 
		SetGlobalValue ("%OFFSETLINEDIST",str); 
	}
	if (GetGlobalCVal("[%IR50PATH]", str, 0))
	{
		HINSTANCE hlib = LoadLibrary(str);
		ii = 1;
	}

	hStartupMenu = GSSiGlobAlloc (  40,GHND,256);
	hStartupCommand = GSSiGlobAlloc (  41,GHND,256);
	hCommand = GSSiGlobAlloc (  42,GHND,1024);   
	ShowValue (0,TRUE); 
	if (App == 1)
		ConvertWaypointGMD (hWnd);    
	ProcessText ("[%PROFILEFILE]=$GETTEMPFILE(gmp,.gmd);$APPEND([%PROFILEFILE],Holder)");
    ProcessGlobal ("[%SYSTEMSTARTUPCOMMAND]");
	//g_hook = SetWindowsHookEx(WH_CBT, CBTProc, hInst, GetCurrentThreadId());

{
#if ENABLETRACE
GSSiExitProg (2);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

long ClearGMTempFiles (void)
{
	char	str[256], ListFile[256], test[256]; 
	long	n=0;
	HFILE	Fid; 
	long	TimeDiff;
	time_t	CurTime,lastFileWriteTime;
	time_t	SecondsInWeek = 60 * 60 * 24 * 7, FileTimerUnitsInSecond=10000000, AboutOneWeek = SecondsInWeek * FileTimerUnitsInSecond;
	
	GSSiGetTempFileName (0,"gt",0,ListFile); 
	Fid = GSSiOpenFile (ListFile,0,OF_CREATE);   
	GetTempDir (str);
	*strrchr(str,'\\') = 0; 
	//MessageBox (0,str,"In Cleartempfiles",MB_OK);

	//SearchFilesInDir (str, "", Fid,&n,"*",1,TRUE);   
	SearchDirectoriesInDir (str,Fid,&n,"*",-1);
	GSSiClose2 (&Fid);
	CurTime = GetLastFileWriteTime (ListFile,&TimeDiff);
	Fid = GSSiOpenFile (ListFile,0,OF_READ);  
/*	{
		char	mess[128];
		sprintf (mess,"Found %i files",n);
		MessageBox (0,mess,"In removetmp",MB_OK);
	}*/
	n=0; 
	while (fgetstring (str,250,Fid))
	{   
		_fstrcpy (test,str);    
		_fstrlwr (test);
		//if (_fstrstr (test,"\\~gm") ||_fstrstr (test,"\\gm"))
		{
			if ((lastFileWriteTime = GetLastFileWriteTime (str,&TimeDiff)))
			{
				if (CurTime - lastFileWriteTime > AboutOneWeek) 
				{
					n++;
					//GSSiRemove (str); 
					DeleteDirAndContents (str);
				}
			}
		}	
	}
	GSSiClose2 (&Fid);
	GSSiRemove (ListFile);
	return n;
}
    
void FreeSavedConfigs (void)
{   
	short	i;
	
	for (i=0;i<MAXCFGTAGS;i++)
		GSSiGlobFree (&hSavedConfig[i]);
	return;
}

void LogUsageInfo (int From,LPSTR mess)
{
	char str[512],DateTime[64];

	if (LogUsage && !MapServer)
	{  
		BOOL	SaveSE=ShareEnabled;
		long	Minutes; 
		time_t	systime; 
		RECT	WindRect,ClientRect;  
		BOOL	SaveAllowCache = AllowCache;
		BOOL	is64Bit = Is64BitMachine();
		char	winVersion[256];

		GetWindowsVersion(winVersion);
		if (is64Bit)
			strcat(winVersion, "-64");
		else
			strcat(winVersion, "-32");

		AllowCache = FALSE;
		systime=time(&systime);
		
		_fstrcpy (DateTime,ctime(&systime));
		*_fstrchr(DateTime,'\n') = 0;   
		switch (From)
		{
		case 0:
		Minutes = (GetTickCount()-SysStartTime)/1000;
		GetWindowRect (hWndMain,&WindRect);
		GetClientRect (hWndMain,&ClientRect);
		ShareEnabled = TRUE;
		if (!ExistFile ("[%%DL]usage.txt"))
			AppendFile ("[%%DL]usage.txt","DATE\tUSER\tNODE\tWINDOWSVER\tMINUTES\tSCREENS\tVERSION\tNTEMP\tWL\tWT\tWR\tWB\tCL\tCT\tCR\tCB\tCLB\tCTB\tCRB\tCBB");

		sprintf (str,"%s\t%s\t%s\t%s\t%ld\t%ld\t%s\t%ld\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i",DateTime,UserName,NodeName,winVersion,Minutes,NumScreensDisplayed,GMVersion,nTempFilesCleared,nCheckPointUpdateBlocks,
																			  		 WindRect.left,WindRect.top,WindRect.right,WindRect.bottom,
																					 ClientRect.left,ClientRect.top,ClientRect.right,ClientRect.bottom,
																					 ClientRectStart.left,ClientRectStart.top,ClientRectStart.right,ClientRectStart.bottom);
		break;
		case 1:
			sprintf (str,"%s\t%s\t%s\tLoad config:%s",DateTime,UserName,NodeName,mess);
			break;
		}
		AppendFile ("[%%DL]usage.txt",str);
		ShareEnabled = SaveSE;
		AllowCache = SaveAllowCache;
	} 
	return;
}

void SaveWindowPosition(void)
{
	RECT rect;
	char txt[128];

	GetWindowRect(hWndMain, &rect);
	recttoa(txt, rect);
	WritePrivateProfileString("User", "LastWindowPos", txt, GMIni);
}
void QuitGraphics()
#if ENABLETRACE
{GSSiEnterProg (3);
#endif
{   
	time_t	Time; 
	LPSTR	pFile,pMessage; 
	char	str[256],ExitMessage[256];
	short	i; 
	
	GSSiSetCursor((HCURSOR)-1);
	CRAPI_Destroy();
	GM32NADCONFREE();
	SaveWindowPosition();
	FreehPDChunkReport();

	if (LogMSGFile != HFILE_ERROR)
	{
		GSSiClose2 (&LogMSGFile);
	}

	MergeImageIntoViewport(0, 0, 0, 0);
	RegisterPopupMessageClass(TRUE);
	RegisterSmallMessageClass(TRUE);
	ProcessText ("$LINKLINES(CLEAR)");
	PATBMPDef(TRUE);

	if (g_hook)
		UnhookWindowsHookEx (g_hook);
	HaltMapDisplay (FALSE,FALSE);
	InQuitGraphics = TRUE;
	BT_CLOSEANDDELETE(&hGMDKeyList);

	ClearFullWindowBitmap(0);
	DestroyAllToolbars ();
	//StopBackgroundCache ();
	//ShowCounts (1);
	
	if (GPSInputWnd) 
	{
		PostMessage( GPSInputWnd, WM_CLOSE, 0, 0L ) ; 
		GPSInputWnd = 0;
	}

	if (hExitMessage)
	{
		pMessage = GlobalLock (hExitMessage);
		_fstrcpy (ExitMessage,pMessage);
		GSSiGlobUlFree (&hExitMessage);
	}
	else
		*ExitMessage = 0;
	LogUsageInfo (0,"");
	AAShutDown();
	InitProj4CoordConv (TRUE);
	for (i=0;i<MAXMULTILEVELZOOM;i++)
		GSSiDeleteObject (&hbmpMultiLevel[i]); 
	DestroySavedGraphicsFile (0);
	UseTrustedCacheFile(0);
 	CloseTRANS2 (&hTranFileToBase); 
	CloseTRANS2 (&hTranBaseToFile);  
	CloseTRANS2 (&hTranFileToVP);  
	GSSiGlobFree (&hSaveCfgImagesFileName);
	WriteToHBird (0,0);
	GetPolygonPickAccelerator (2);
	ProcessText ("$SCREEN()");
	LoadCustomStreenNameConversions (2);
	GetAccountNum (0);
	DestroyFence (0);
	VehicleLinkedToFence (0,0,0);
	GSSiGlobFree (&hGFVehList);
	DestroyVoterLists ();
	PointListCommands (-1,0,0);
	ZoomConnectedProcesses (TRUE);
	SetShowValDB (0);   
	CloseDisplayedHighlightedRefs();
	ProcessText ("$DELETEFILE([%PROFILEFILE])");
	JoinLinesBetweenPoints (0,0,0,0);
	DisplayContourLabels (TRUE);
    DisplayLayeredSymbols (0,TRUE);
    DisplayHollowLines (TRUE); 
	GSSiGlobFree (&hShowValMaskAccelerator); 
	GSSiGlobFree (&hMemMapColorMap);		
    hDibFree (&hWindowDib32);    
   	GSSiGlobFree (&hVirtualPrintFile);
    GetLowranceWPList (TRUE);
	GSSiDeleteObject (&hBlackPen);
	GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	ProcessGlobal ("[%ENDCMD]");
	GSSiGlobFree (&hCfgImages); 
	CloseDGNCellLibrary ();  
	ClearSpecial (); 
	ResetStreetLabels ();
	GSSiGlobFree (&hSkipRefs);
	GSSiGlobFree (&hGRText); 
	GSSiGlobFree (&hGRTextTPL); 
	if (SetConfig(0))
		UnallocateConfig ();
	if (SetConfig(1))
		UnallocateConfig ();  
    GSSiGlobFree (&hConfigDescription);   
    *CfgName = 0;
	GetNumZIPsInMunic (0,0);
	GetMunicFromName (0);	
	SetShowValPoly (0,TRUE);
	if (hWndAddEdit)
		SendMessage (hWndAddEdit,WM_COMMAND,IDCANCEL,0);
	if (hWndAddMatch)
		SendMessage (hWndAddMatch,WM_COMMAND,IDC_EXIT,0);
	if (hWndStrmPoint)
		SendMessage (hWndStrmPoint,WM_CLOSE,0,0);
	if (hWndStrmPipe)
		SendMessage (hWndStrmPipe,WM_CLOSE,0,0);
	if (hWndUserForm)
		SendMessage (hWndUserForm,WM_CLOSE,0,0);  
	if (hDynamicDialog)
		SendMessage (hDynamicDialog,WM_CLOSE,0,0);
	if (hWndVehStatus)
		SendMessage (hWndVehStatus,WM_CLOSE,0,0);
	if (OkToContinueTime)
		KillTimer (hWndMain,8);
	if (HaveReplayTimer)
		KillTimer (hWndMain,HaveReplayTimer);
	KillTimer(hWndMain, 1);

    CloseSymDict();  
	AddBMPToCache (0,0);
	AddBMPToCache32 (0,0);
	CloseFidSmall (); 
//	CloseFidDBF (TRUE);
	if (WSAIsBlocking())
		WSACancelBlockingCall ();
	GSSiGlobFree (&hCmdMess);
	ClearFullWindowBitmap (0);
	if (FidBlockedRefs != HFILE_ERROR)
		GSSiClose2 (&FidBlockedRefs); 
	if (hBlockedRefFile)
	{    
		pFile = GlobalLock (hBlockedRefFile);
		GSSiRemove (pFile);
    	GSSiGlobUlFree (&hBlockedRefFile);
    }   
	if (hTempImageFile)
	{    
		pFile = GlobalLock (hTempImageFile);
		GSSiRemove (pFile);
    	GSSiGlobUlFree (&hTempImageFile);
    }
	ClearSizingFonts ();
	DTMClose (0); 
    DestroyDummyVehicles ();
	CloseWinSock(TRUE);
	LogServerActivity  ("Server closed");
	OpenTCPReplay (0);
	CloseCDLookUpTable ();    
	ClosePrevLayers ();
	DestroySELECTVALUES();
	DestroyRandomBrushes (); 
	DestroySavedPolys ();  
	ClearCurrentCD ();  
	FreeSavedConfigs ();
	CacheAlreadyChecked (0,0,0);
	GSSiGlobUlFree (&hPNGrid);
	GSSiGlobFree (&hHollowLinesFile);  
   	GSSiGlobFree (&hLastDeleteList);
	GSSiGlobFree (&hEmbeddedGFCommand);
	GSSiGlobFree (&hPMRecList);
	GSSiGlobFree (&hFileSymList);
	GSSiGlobFree (&hSymConversionTable);   
	GSSiGlobFree (&hSymbolAttributes);
	GSSiGlobFree (&hSymNames);
    GSSiGlobFree (&hUnSplinedPoly);
	GSSiGlobFree (&hCurvePoints); 
    GSSiGlobFree (&hTAGDef);  
	GSSiGlobFree (&hLastCmd);
	GSSiGlobFree (&hTextString);  
   	GSSiGlobFree (&hAddGraphicsFun);
   	GSSiGlobFree (&hAddGraphicsFun2); 
   	GSSiGlobFree (&hEndDisplayCommand); 
	GSSiGlobFree (&hPolyCoord);
   	if (hProjectionFile)
   	{
   		LPSTR pName = GlobalLock (hProjectionFile);
   		
  		GSSiRemove (pName);	
    	GSSiGlobUlFree (&hProjectionFile);
    }
	CloseTRANS2 (&hTranProjection);
	CloseTRANS2 (&hTranProjectionReverse);

	if (hWndAddWaypoint)
	    DestroyWindow(hWndAddWaypoint);   
	if (hWndTraverseEntry)
		SendMessage(hWndTraverseEntry, WM_CLOSE, 0, 0L);
	DestroyOpenFileSymNames ();
	ConvertSymName (0,0,FALSE,0);
	GSSiGlobFree (&hCommand);
    GSSiGlobFree (&hStartupMenu);  
   	GSSiGlobFree (&hStartupCommand);
	EnlargeScreen (0,0);

	DispText (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
	UndoEnabled = FALSE;
    ClearHighlightList (FALSE); 
    DestroyCursors ();
    GSSiGlobFree (&hDesiredDocs);
    GSSiGlobFree (&hSetVals);
//    GSSiGlobFree (&hNullFrames);   
    DestroySavedZooms ();
    CloseODBCPasswordFile ();
	ClearPolyOff (FALSE);
	GetCurrentGraphicsDBName (0,1);
	GetCurrentGraphicsDBName (0,2);
	GetCurrentPNDBName (0);
	GSSiGlobFree (&hGCmdString);  
	GSSiGlobFree (&hPolyBuffer);   
	GSSiGlobFree (&hElevBuffer);
	GSSiGlobFree (&hPolyPartLen);
	GSSiGlobFree (&hAreaOffFile);  
	GSSiGlobFree (&hScreenFile);
//	CloseAddressFiles (TRUE); 
	if (hWndDigControl)
		DestroyWindow (hWndDigControl);
	GSSiGlobFree (&hDigControlPoints); 
   	if (hdcMemMap)
	{
		HBITMAP hbm = SelectObject(hdcMemMap, hbmpOld);
		DeleteObject (hbm); 
		DeleteDC (hdcMemMap);
	}	
	GSSiDeleteObject(&hBitmapScreenBuffer);
//	TerminateConversations(0);
    ConvertCoordClose ();  
//    FreeLibraries();   
    if (ncalls1 != ncalls2)
    	ncalls1=0;
	CloseOrthos(TRUE);
	ClearSavedScreens((HWND)-1,0,0);
	STNDSN_CLEAR(); 
	CloseStreetNameTable ();
//	CloseGEOSPANVideo ();
	CloseDigConnection(); 
	GPSClose (&GPSStream);
	GSSiGlobFree (&hSavedPickList);
	GSSiGlobFree (&hUserCmd); 
	GSSiGlobFree (&hToolCmd);
	GSSiGlobFree (&HLTOutFields); 
	GetSymAttrFile (0,0,0,0,0,0);
	ODBCTerminate (TRUE);
	//dumpvars("After.txt");
	CDClose(); 
	CloseBufferedMacros ();
	Time = time(0);
	GSSiTrace(ctime(&Time),0);
	sprintf (str,"Max HFile = %i",(short)MaxHFile);
	GSSiTrace (str,0);
	GSSiTrace("*** End Program ***",0); 
	SetTrace (FALSE);
	if (hUserMenu) 
		DestroyUserPopups (&hCurPopups);
	CloseAllRequestedFiles (FALSE);  
	CheckOpenFiles ();
	SaveDataToUndoFile (0,0,0,0,0);    
	CheckPointEnd ();
	AllowJournal = FALSE;
	ConvertToNewLocation (0,0);
	DestroyVarSpace((HANDLE)-1);
	ScreenBufferDC((HWND)1, 0);
	GSSiGlobFree (&hSaveWindowText);
	GSSiGlobFree (&hPNAddData);
	RegisterFloatMenuClass(TRUE);
	RegisterPromptClass(TRUE);
	RegisterBgUpdateClass(TRUE);
	RegisterImageZoomClass(TRUE);
	if (!FGDBCheck())
		ii = 1;
	if (fidLogFileUse != INVALID_HANDLE_VALUE)
		CloseHandle(fidLogFileUse);
	FreeTrustedFiles();
	GetTempDir(0);
	GSSiFreeImage_Unload(0);
#if CHECKMEM
	TrackObject (0,-100); 
#endif
	if (*ExitMessage)
		GSSiMsgBox (0,ExitMessage,"",MB_OK,0); 
{
#if ENABLETRACE
GSSiExitProg (3);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

void SetCurView (LPVIEWPORT pVP)
#if ENABLETRACE
{GSSiEnterProg (4);
#endif
{   
	short	ii;
	
	if (pVP != CurView)
		ii = 1;
	CurView = pVP; 
	if (!CurView)
	{
		if (*pCommandViewport)
			CurView = pViewports[*pCommandViewport-1];
		else
			ii=1;
	} 
/*	if (CurView && !CurView->ID)
		ii=1;
	if (CurView && CurView->ID == 4)
		ii=1;*/
	if (CurView && !inUnallocateConfig)
	{
		LPVIEWPORT BoundsVP = CurView;
		if (CurView->Type == SUBVIEWPORT && CurView->Parent)
			BoundsVP = pViewports[CurView->Parent - 1];
		while (BoundsVP->DisplayInParent && BoundsVP->Parent && BoundsVP->ID != BoundsVP->Parent)
			BoundsVP = pViewports[BoundsVP->Parent - 1];

		if (CurView->ID != BoundsVP->ID)
		{
			CurView->HaveBounds = BoundsVP->HaveBounds;
			CurView->Rotation = BoundsVP->Rotation;
			CurView->MidPointW = BoundsVP->MidPointW;
			CurView->ScreenRect = BoundsVP->ScreenRect;
			CurView->DrawRect = BoundsVP->DrawRect;
			CurView->WBounds = BoundsVP->WBounds;
			CurView->Scale = BoundsVP->Scale;
			CurView->Scale = BoundsVP->Scale;
			CurView->CurrentGoogleZoom = BoundsVP->CurrentGoogleZoom;
			CurView->CurrentGoogleType = BoundsVP->CurrentGoogleType;
			CurView->CurrentGoogleScale = BoundsVP->CurrentGoogleScale;
			CurView->BorderPct = BoundsVP->BorderPct;
			CurView->BorderPct = BoundsVP->BorderPct;
			CurView->DesiredHeight = BoundsVP->DesiredHeight;
			CurView->TagPointID = BoundsVP->TagPointID;
			CurView->TagPointType = BoundsVP->TagPointType;
			CurView->TagPoint = BoundsVP->TagPoint;
			CurView->TagPointActual = BoundsVP->TagPointActual;
			CurView->WidthType = BoundsVP->WidthType;
			CurView->DisplayInInches = BoundsVP->DisplayInInches;
			CurView->DisplayedFullScreen = BoundsVP->DisplayedFullScreen;
			CurView->AutoSize = BoundsVP->AutoSize;
			CurView->DisplayRect = BoundsVP->DisplayRect;
			CurView->ProfileRect = BoundsVP->ProfileRect;
			CurView->DisplayRect = BoundsVP->DisplayRect;
			CurView->Width = BoundsVP->Width;
			CurView->Height = BoundsVP->Height;
			CurView->Margin = BoundsVP->Margin;
			CurView->Rect = BoundsVP->Rect;
			CurView->xForm = BoundsVP->xForm;
			CurView->ZMScale = BoundsVP->ZMScale;
		}
	}

{
#if ENABLETRACE
GSSiExitProg (4);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void CloseAllIndexes ()
#if ENABLETRACE
{GSSiEnterProg (5);
#endif
{   short iview, ifile;
    LPVIEWPORT  SaveView;
    
    if (FidConfig == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (5);
#endif
    	return;
}
    CloseOrthoAVI ();
    SaveView = CurView;
    for (iview=0;iview<*pNumViewports;iview++)
    {   
        SetCurView (pViewports[iview]);
        if (!CurView) break;      
        if (CurView->SubFile)
        {
            _fstrcpy (CurView->lpFiles[CurView->RestoreFile],CurView->OrigFile); 
			CurView->SubFile = 0;
			SetRestoreFile(CurView, 0);
        } 
        GSSiGlobFree (&CurView->hBinFileList);
        for (ifile=0;ifile<CurView->NumFiles;ifile++)
        {   
        
            if (CurView->FileType[ifile]>3 && CurView->hlpIndex[ifile])
            {   
                CloseMapIndex (CurView->lpFiles[ifile],
                               CurView->hlpIndex[ifile],FALSE,TRUE);
                CurView->hlpIndex[ifile] = 0;
            }
        }
    }
    SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (5);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
void SethCursor (HCURSOR hc)
{
	hCursor = hc;
	return;
}

short DisplayPlotInit (HWND hWnd, BOOL Imediate)
#if ENABLETRACE
{GSSiEnterProg (6);
#endif
{   
	char	Name[256];   
	short	ii;
	
    CloseMap(FALSE);       
    _fstrcpy (Name,PltName);
    ExpandText (Name);
    if (!Name[0] && PltType != 6)
{
#if ENABLETRACE
GSSiExitProg (6);
#endif
    	return(0);
}
    if (!Printing && !Imediate)
    {                             
        OldCursor = GSSiSetCursor (hDrawingCursor);
		if (InfoBoxEditTimer)
			KillTimer (hWndMain,InfoBoxEditTimer);
        idTimer =  SetTimer(hWndMain, 1, 1, (TIMERPROC) 0); 
        if (!idTimer)
        	ii=1;
    }

{
#if ENABLETRACE
GSSiExitProg (6);
#endif
    return (1);
}
#if ENABLETRACE
}
#endif
}  

void SetPltNameGlobals (void)
#if ENABLETRACE
{GSSiEnterProg (7);
#endif
{
	HANDLE	hName=GSSiGlobAlloc (  43,GMEM_MOVEABLE,1024);
	LPSTR	FullName=GlobalLock (hName);
	LPSTR	name=FullName+256;
	LPSTR	pltname=name+256; 
    	
	_fstrcpy (pltname,PltName);
	ExpandText (pltname);
	_fullpath (FullName,pltname,256);
	_splitpath (FullName,0,0,name,0);
    SetGlobalValue ("%PLTPATH",FullName);  
    SetGlobalValue ("%PLTNAME",name);
    GSSiGlobUlFree (&hName);
{
#if ENABLETRACE
GSSiExitProg (7);
#endif
    return ;
}
#if ENABLETRACE
}
#endif
}     


void ClearMap (void)
#if ENABLETRACE
{GSSiEnterProg (9);
#endif
{   
    if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (9);
#endif
    	return;
}
    CurView->WindowIsZoomed = FALSE;   
    if (CurView->WindowZoomedToOrtho == 1)
    	CurView->WindowZoomedToOrtho = 0;
    if (idTimer) CloseMap (FALSE);
    PltName[0]='\0';
    CurView->HaveBounds = 0;
    CloseDynWindows ();
	RedisplayWindow();   
#if ENABLETRACE
}
#endif
}

BOOL WindowZoomed (void)
#if ENABLETRACE
{GSSiEnterProg (10);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (10);
#endif
    return (CurView->WindowIsZoomed);
}
#if ENABLETRACE
}
#endif
} 

BOOL GetNextOrthoTileFromIndex(void)
{
	BOOL rtn = FALSE;
	
	if (CurView->FileType[CurView->CurFile] == 5 && CurView->hlpIndex[CurView->CurFile])
	{
		LPFILEINDEX lpIndex = (LPFILEINDEX)GlobalLock(CurView->hlpIndex[CurView->CurFile]);
Next:
		if (GetNextIndexEntry(lpIndex))
		{
			MNMXCORD	TestBounds = lpIndex->CurrentEntry->Bounds;
			char		CurEntryName[MAX_PATH];

			rtn = TRUE;
			strcpy(CurEntryName, lpIndex->CurrentEntry->Name);
			if (!AdjustFileBounds(CurView->ID, CurView->CurFile, &TestBounds, CurEntryName))
				rtn = FALSE;
			else if (!RectInWBounds(&TestBounds, 0))
				goto Next;
			else
			{
				LPSTR	lpAT;

				if ((lpAT = _fstrrchr(lpIndex->CurrentEntry->Name, '@')))
				{
					lpAT++;
					CurOrthoFrame = atol(lpAT);
					if (CurOrthoFrame < 0)
						goto Next;
				}
				else
					CurOrthoFrame = lpIndex->FileInIndex - 1;
				CloseOrthos(FALSE);
				if (hOrthos)
				{
					LPORTHO CurOrtho = (LPORTHO)GlobalLock(hOrthos) + OrthoID;
					float   RSQMIN;
					double  BASEX[4], BASEY[4], BMX[4], BMY[4];

					lpIndex->CurrentEntry->Bounds = TestBounds;
					_fstrcpy(CurOrtho->Name, PltName);
					if (*CurEntryName == '.')
						_fstrcpy(CurOrtho->OrigName, CurrentOrthoOrigName);
					else
						_fstrcpy(CurOrtho->OrigName, CurEntryName);
					CurOrtho->Frame = CurOrthoFrame;
					CurOrtho->Bounds = lpIndex->CurrentEntry->Bounds;
					CurOrtho->Width = lpIndex->CurrentEntry->BMWidth;
					CurOrtho->Height = lpIndex->CurrentEntry->BMHeight;
					CurOrtho->BitCount = lpIndex->CurrentEntry->BMBitCount;
					ComputeIndexOrthoRes(lpIndex);
					CurOrtho->Res = lpIndex->OrthoRes;

					//Offset = CurOrthoFrame;
					CurOrtho->LastUsed = OrthoUse++;
					CloseTRANS2(&hTranBMToBase);
					CloseTRANS2(&hTranBaseToBM);
					if (CurOrtho->BitCount != 8)
						ii = 1;
					BMX[0] = -1;
					BMX[1] = -1;
					BMX[2] = lpIndex->CurrentEntry->BMWidth;
					BMX[3] = BMX[2];
					BMY[0] = -1;
					BMY[1] = lpIndex->CurrentEntry->BMHeight;
					BMY[2] = BMY[1];
					BMY[3] = -1;
					BASEX[0] = CurOrtho->Bounds.xmn - CurOrtho->Res;
					BASEX[1] = BASEX[0];
					BASEX[2] = CurOrtho->Bounds.xmx + CurOrtho->Res;
					BASEX[3] = BASEX[2];
					BASEY[0] = CurOrtho->Bounds.ymn - CurOrtho->Res;
					BASEY[1] = CurOrtho->Bounds.ymx + CurOrtho->Res;
					BASEY[2] = BASEY[1];
					BASEY[3] = BASEY[0];
					CurView->FileMNMX = CurOrtho->Bounds;
					hTranBMToBase = STRAN2(1614, BMX, BMY, BASEX, BASEY, 4, &RSQMIN, 1, 0);
					hTranBaseToBM = STRAN2(1615, BASEX, BASEY, BMX, BMY, 4, &RSQMIN, 1, 0);
					GlobalUnlock(hOrthos);
					GlobalUnlock(CurView->hlpIndex[CurView->CurFile]);
				}
				else
					rtn = FALSE;
			}
		}
		else if ((lpIndex = GetNextIndexHeader(&CurView->hlpIndex[CurView->CurFile], TRUE)))
			goto Next;
		else 
			rtn = FALSE;
	}

	return rtn;
}

BOOL DisplaySeg (HDC *hDC,BOOL Immediate)
#if ENABLETRACE
{GSSiEnterProg (12);
#endif
{   HANDLE      hpltBuf=0;
    LPSTR       LPpltBuf;
//    static char       LPpltBuf[UINT_MAX];
    LPSHORT       ipnt;
    short         iview,ii;
	short		nContinues;
	BOOL		rtn;
    long        SegStart;  
//    MNMXCORD	SaveWBounds = CurView->WBounds;
//    MNMXCORL	SaveBounds = CurView->Bounds;
    clock_t     starttime;    
    HDIB32		hDib32;
    static		long	DebugSeg= 4488536;
	static		long	originalOffset = -1;
    
    if (!InDisplayProcessing && !Pick)
    	ii=1;
    nBytes = 0;
    if (!PltName[0] && PltType != 6)
{
#if ENABLETRACE
GSSiExitProg (12);
#endif
    	return FALSE; 
}
     
//    ExpandBounds (&CurView->WBounds,CurView->MaxSymbolWidth/BaseDistToWinDist);  
//    ExpandMinMaxL (&CurView->Bounds, CurView->MaxSymbolWidth);
    if (*hDC)
    {
        SetDisplayMode (*hDC,GF_TEXTMODE);
        SelectClipRgn (*hDC,0);
    }
    if (!OpenMap (CurView->hWnd,CurView->hDC))
    	goto Next;
	if (Display && !Pick && !FileMode && CurView->hDC)
    {   
        if (!CurView->hRgn)
        	CurView->hRgn = CreateVPRgn (FALSE,FALSE);
        if (SelectClipRgn (CurView->hDC,CurView->hRgn) == NULLREGION && !PrintingToMF)
		{
		    GSSiDeleteObject(&CurView->hRgn);
			goto NextSeg;
		}
        SetDisplayMode (CurView->hDC,GF_MAPMODE);
    }
    if (ShowFileBounds)
    {
        DisplayFileBounds (CurView->FileMNMX); 
        if (GetCopySize)
        {   
        	HFILE	Fid;
			OFSTRUCTGM	OFStruct;
        	long	Offset, lRec;
        	
		    if (PltType == 5)
		    {   
		    	LPORTHO CurOrtho;
		    	
			    if (!hOrthos)
			    	goto Next;
			    CurOrtho = (LPORTHO)GlobalLock (hOrthos) + OrthoID;
			    Fid = GSSiOpenFile (CurOrtho->Name,&OFStruct,OF_READ);
			    GSSillseek (Fid,CurOrtho->Frame,0);
			    BigRead (Fid,(HPSTR)&lRec,4); 
			    TotCopySize += lRec;
			    GSSiClose2 (&Fid);  
			    GlobalUnlock (hOrthos);
			}
			else 
			{
			    Fid = GSSiOpenFile (PltName,&OFStruct,OF_READ);
			    lRec = GSSillseek (Fid,0,2);
			    TotCopySize += lRec;
			    GSSiClose2 (&Fid);
			}
       }
        goto Next;
    }
    
    if (DoMapCopy)
    	goto Next;
                     
    if (PltType == 6)//highlight list
    {
		CurView->WantDisplayHighlight = TRUE;
   		goto Next;
    }
    	
    if (PltType == 7)
    {   
    	MapType = MT_MACRO;
    	if (ProcessDisplayMacro (FidMap,2))
    		goto RtnTrue;
    	else
    		goto Next;
    }
    	
    if (PltType == 3 || PltType == 5)
    {   
        SetDisplayMode (*hDC,GF_TEXTMODE); 
        if (PltType == 5)
        { 
			if (Pick)
			{
				if (CurVis->WantType[5])
				{
					int	OrthoSym = GetDictSymbolNumber ("ORTHOPHOTO");

					if (OrthoSym)
					{   
						char	OrthoUDI[64];
						DPOINT	OrthoPoints[4];
						MNMXCORD	Bounds;
						
						if (hOrthos)
						{
							LPORTHO CurOrtho = (LPORTHO)GlobalLock (hOrthos) + OrthoID; 
    		
							Bounds = CurOrtho->Bounds;
							GlobalUnlock (hOrthos);
						}
						else
							Bounds = CurView->FileMNMX;
						BoundsToPoints (&Bounds,OrthoPoints,0);
						CurrentRefno = LONG_MIN + (CurView->CurFile -1) * USHRT_MAX + CurView->SubFile*1024 + FileInIndex;
						CurrentDesc = OrthoSym;
						_fstrcpy (CurrentPrefix,"ORTHO");  
						GetGlobalCVal ("[%ORTHOUDI]",OrthoUDI,"Unknown");
						strncpy0 (CurrentUDI,OrthoUDI,MAX_UDI_LEN);  
						SetGlobalValue2 (hPrefix,CurrentPrefix,0); 
						SetUDIValue (CurrentPrefix,CurrentUDI);
						rtn = PickPolygonD (OrthoPoints,4,0,0,9999999,0);
						if (rtn)
							ii=1;
						goto NextSeg;
					} 
				}
			}
			else
			{
				if (CurView->PassID != 1)
	        		goto Next;
				//GMEnableMenuItem(hWndMain, IDM_Z_ORTHO, MF_BYCOMMAND | MF_ENABLED);
				CurView->HaveOrthos = TRUE; 
				if (MapFileType(PltName, 0, 0) == MT_SID)
					DisplaySIDInVP32 (CurView,PltName);
        		else if (_fstrstr(PltName,".BMP") || _fstrstr(PltName,".JPG") || _fstrstr(PltName,".PNG") || _fstrstr(PltName,".GIF")|| _fstrstr(PltName,".TIF")|| _fstrstr(PltName,".PCX") || !_fstrnicmp(PltName, "http:", 5) || !_fstrnicmp(PltName, "https:", 6))
					goto DisplayImage;
				else
				{
					int n = 1;
					do
					{
						DisplayOrthoPhoto();
						if (!(n++ % 16))
							SetContinueProcessing(CheckForContinue(FALSE, 0));

					} while (ContinueProcessing && GetNextOrthoTileFromIndex());
					ii = 1;
					//ShowBufferedScreen(TRUE, TRUE, -99, 0);
				}
			}
        }
        else
        {
DisplayImage:
        	ExpandText (PltName); 
        	if (strnicmp(PltName, "http:", 5) && strnicmp(PltName, "https:", 6))
        		_fstrupr (PltName);
        	if (_fstrstr(PltName,".TXT"))
        		DisplayTextFileInRect (*hDC,&CurView->DrawRect,PltName);
			else if (_fstrstr(PltName, ".BMP") || _fstrstr(PltName, ".JPG") || strstr(PltName, ".JP2") || 
				     _fstrstr(PltName, ".PNG") ||
					 _fstrstr(PltName, ".GIF") ||
					 _fstrstr(PltName, ".TIF") ||
					 _fstrstr(PltName, ".PCX") ||
					!_fstrnicmp(PltName, "http:", 5) ||
					!_fstrnicmp(PltName, "https:", 6))
{
		        if ((CurView->PassID != 1 && CurView->PassID != 5) && CurView->Type != 5)
		        	goto Next; 
    			if (!CurView->HaveBounds)
			    {   
			        BOOL    SaveDisableHalt = DisableHalt;
			        
			        DisableHalt = TRUE;
			        CurView->NewBounds.xmn = CurView->DrawRect.left;
			        CurView->NewBounds.ymn = CurView->DrawRect.top;
			        CurView->NewBounds.xmx = CurView->DrawRect.right;
			        CurView->NewBounds.ymx = CurView->DrawRect.bottom;
			        SetBounds(CurView->hWnd,CurView->hDC);
			        DisableHalt = SaveDisableHalt;
			    }  
				GSSiDeleteObject(&CurView->hRgn);
				CurView->hRgn = CreateVPRgn(FALSE,FALSE);
				SelectClipRgn (CurView->hDC,CurView->hRgn);
				GSSiDeleteObject(&CurView->hRgn);
				ResetProfileVP(CurView);
			    if (CurView->StretchImage[CurView->CurFile] == 1)
					DisplayBMFileInVP32(CurView->hDC, PltName, 0, TRUE, CurView->FileTransparency[CurView->CurFile]);//CurView->Rotation);		        	
				/*{
				    hDib32 = LoadDIB32(PltName,TRUE); 
				    if (_fstrstr (PltName,"LOGO"))
				    	ii=1;
					SetDisplayMode (*hDC, GF_SCREENMODE); 
					DisplayBMInRect32 (CurView->hDC,hDib32,CurView->ScreenRect,TRUE);
					DestroyDIB32(hDib32,FALSE);
				}*/
				else
					DisplayBMFileInVP32(CurView->hDC, PltName, 0, FALSE, CurView->FileTransparency[CurView->CurFile]);//CurView->Rotation);		        	
//            	DisplayBMFileInRect (CurView->hDC,PltName,CurView->DrawRect,TRUE); 
            }
        	else if (strstr(PltName,".SID"))
        	{
		        if (CurView->PassID != 1 && CurView->Type != 5)
		        	goto Next; 
				//GMEnableMenuItem(hWndMain, IDM_Z_ORTHO, MF_BYCOMMAND | MF_ENABLED);
				CurView->HaveOrthos = TRUE; 
    			if (!CurView->HaveBounds)
			    {   
			        BOOL    SaveDisableHalt = DisableHalt;
			        
			        DisableHalt = TRUE;
			        CurView->NewBounds.xmn = CurView->DrawRect.left;
			        CurView->NewBounds.ymn = CurView->DrawRect.top;
			        CurView->NewBounds.xmx = CurView->DrawRect.right;
			        CurView->NewBounds.ymx = CurView->DrawRect.bottom;
			        SetBounds(CurView->hWnd,CurView->hDC);
			        DisableHalt = SaveDisableHalt;
			    }
				DisplaySIDInVP32 (CurView, PltName);		        	
//            	DisplayBMFileInRect (CurView->hDC,PltName,CurView->DrawRect,TRUE); 
            }
	        	else if (_fstrstr(PltName,".TIF"))
        	{
		        if (CurView->PassID != 1)
		        	goto Next; 
    			if (!CurView->HaveBounds)
			    {   
			        BOOL    SaveDisableHalt = DisableHalt;
			        
			        DisableHalt = TRUE;
			        CurView->NewBounds.xmn = CurView->DrawRect.left;
			        CurView->NewBounds.ymn = CurView->DrawRect.top;
			        CurView->NewBounds.xmx = CurView->DrawRect.right;
			        CurView->NewBounds.ymx = CurView->DrawRect.bottom;
			        SetBounds(CurView->hWnd,CurView->hDC);
			        DisableHalt = SaveDisableHalt;
			    }
		        	
            	DisplayTIFFileInRect (CurView->hDC,PltName,CurView->DrawRect,TRUE); 
            }
        	else if (_fstrstr(PltName,".PCX"))
        	{
		        if (CurView->PassID != 1)
		        	goto Next; 
    			if (!CurView->HaveBounds)
			    {   
			        BOOL    SaveDisableHalt = DisableHalt;
			        
			        DisableHalt = TRUE;
			        CurView->NewBounds.xmn = CurView->DrawRect.left;
			        CurView->NewBounds.ymn = CurView->DrawRect.top;
			        CurView->NewBounds.xmx = CurView->DrawRect.right;
			        CurView->NewBounds.ymx = CurView->DrawRect.bottom;
			        SetBounds(CurView->hWnd,CurView->hDC);
			        DisableHalt = SaveDisableHalt;
			    }
		        	
            	DisplayPCXFileInRect (CurView->hDC,PltName,CurView->DrawRect,TRUE); 
            }
        	else if (_fstrstr(PltName,".SBM")) 
        	{
        		if (CurView->ID != 12)
        			ii=1;
            	DisplayBMFileInRect (CurView->hDC,PltName,CurView->DrawRect,FALSE); 
            }
        }
        goto Next;
    } 
    if (FidMap == HFILE_ERROR)
    	goto RtnFalse; 

NextSeg:
    if (!FindNextSegment())
    {
Next:
    	CloseMap(FALSE);
        if (Printing || !ContinueProcessing)
        	goto RtnFalse;
        while (DisplayViewID<NumViewportsToDisplay)
        {   
            if (DisplayViewport (CurView->hWnd,CurView->hDC,Immediate))
            {
				if (!OpenMap (CurView->hWnd, *hDC))
					goto Next;
            	goto RtnTrue;
            }
        }
        if (Display && !Pick)ShowDynWindows ();
        for (iview=0;iview<*pNumViewports;iview++)
        {   
            SetCurView ( pViewports[iview]);
            CurView->Display = FALSE;
        }

        goto RtnFalse;
    } 
    if (MapType == MT_SID)
    {
		if (!Pick && CurView->PassID == 1)
		{
			//GMEnableMenuItem(hWndMain, IDM_Z_ORTHO, MF_BYCOMMAND | MF_ENABLED);
			CurView->HaveOrthos = TRUE; 
 	        SetDisplayMode (*hDC,GF_TEXTMODE);
			DisplaySIDInVP32 (CurView,0);
		} 
		goto Next;
//		CloseMap (FALSE);
//		goto RtnTrue;
	}	    
    if (MapType == MT_SHP)
    {
    	MNMXCORD	SHPBounds;
    	short	n=0, maxn=100;
    	do 
    	{
			if (SHPRecOffset == 65918)
				ii=1;
			if (!ReadSHPRecordHeader (FidMap,SHPRecOffset,&SHPBounds))
				goto RtnTrue;
			if (ForceRefIndex || ForceTAGIndex)
	    		StatusWindowUpdate2 (0,NumSHPRecs,CurrentSHPRec); 
			if (RectInWBounds (&SHPBounds,1))
			{
				if (FastMapCopy)
				{
					CopyRec = 2;
					UpdateItem = 0;
					hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
					lUpdateBuf = 0;
				}
				ProcessSHPRecord (*hDC,FidMap,CurrentSHPRec); 
				if (FastMapCopy)
				{
					LPSTR	pRec = GlobalLock (hUpdateBuf);

					CopyRec = FALSE;
					BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
					BigWrite (FastMapCopyFid,pRec,lUpdateBuf,-1);
					GSSiGlobUlFree (&hUpdateBuf);
					FastMapCopynRecs++;
					if (!(FastMapCopynRecs % 100))
					{
						char	mess[64];

						sprintf (mess,"%i records copied",FastMapCopynRecs);
						StatusExtraInfoUpdate(mess);
					}
				}
			}
			else
				ii=1;
			n++;
		} while (n < maxn && FindNextSegment());
        if (n < maxn)
        	goto Next;
		goto RtnTrue;
    }  
    if (MapType == MT_PERSONAL_GEO_DB)
    {
    	MNMXCORD	SHPBounds;
    	short	n=0, maxn=100;
    	do 
    	{
			if (!ReadPGDBRecordHeader (&SHPBounds))
				goto RtnTrue;
			if (ForceRefIndex || ForceTAGIndex)
	    		StatusWindowUpdate2 (0,NumSHPRecs,CurrentSHPRec); 
			if (RectInWBounds (&SHPBounds,1))
			{
				if (FastMapCopy)
				{
					CopyRec = 2;
					UpdateItem = 0;
					hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
					lUpdateBuf = 0;
				}
				ProcessPGDBRecord (*hDC,CurrentSHPRec,0);  
				if (FastMapCopy)
				{
					LPSTR	pRec = GlobalLock (hUpdateBuf);

					CopyRec = FALSE;
					BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
					BigWrite (FastMapCopyFid,pRec,lUpdateBuf,-1);
					GSSiGlobUlFree (&hUpdateBuf);
					FastMapCopynRecs++;
					if (!(FastMapCopynRecs % 100))
					{
						char	mess[64];

						sprintf (mess,"%i records copied",FastMapCopynRecs);
						StatusExtraInfoUpdate(mess);
					}
				}
			}
			n++;
		} while (n < maxn && FindNextSegment());
        if (n < maxn)
        	goto Next;
		goto RtnTrue;
    }
    if (MapType == MT_FILE_GEO_DB)
    {
    	MNMXCORD	SHPBounds;
    	short	n=0, maxn=100;
		BOOL quitProcessing;

    	do 
    	{
			if (!ReadFGDBRecordHeader (&SHPBounds))
				goto RtnTrue;
			if (ForceRefIndex || ForceTAGIndex)
	    		StatusWindowUpdate2 (0,NumSHPRecs,CurrentSHPRec); 
			if (RectInWBounds (&SHPBounds,1))
			{
				if (FastMapCopy)
				{
					CopyRec = 2;
					UpdateItem = 0;
					hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
					lUpdateBuf = 0;
				}
				ProcessFGDBRecord (*hDC,CurrentSHPRec);  
				if (FastMapCopy)
				{
					LPSTR	pRec = GlobalLock (hUpdateBuf);

					CopyRec = FALSE;
					BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
					BigWrite (FastMapCopyFid,pRec,lUpdateBuf,-1);
					GSSiGlobUlFree (&hUpdateBuf);
					FastMapCopynRecs++;
					if (!(FastMapCopynRecs % 100))
					{
						char	mess[64];

						sprintf (mess,"%i records copied",FastMapCopynRecs);
						StatusExtraInfoUpdate(mess);
					}
				}
			}
			if (!(n % 16) && !CheckForContinue(FALSE, &quitProcessing))
			{
		    	CloseMap(FALSE);
				if (quitProcessing)
					goto RtnFalse;
				else
					goto RtnTrue;
			}
			n++;
		} while (n < maxn && FindNextSegment());
        if (n < maxn)
        	goto Next;
		goto RtnTrue;
    }
    else if (MapType == MT_DTM)
    {   
    	DisplayDTMSegment ();
		goto RtnTrue;
    }
    else if (MapType == MT_ORA)
    {
    	MNMXCORD	ORABounds;
    	short	n=0, maxn=10;
    	
    	do 
    	{
    		if (CurrentORARec > 326)
    			ii=1;
			if (!ReadORARecordHeader (FidMap,ORARecOffset,&ORABounds))
				goto RtnTrue; 
			ExpandORAPointBounds (&ORABounds);  
			if (ForceRefIndex || ForceTAGIndex)
	    		StatusWindowUpdate2 (0,NumORARecs,CurrentORARec); 
			if (RectInWBounds (&ORABounds,1))
			{
				if (FastMapCopy)
				{
					CopyRec = 2;
					UpdateItem = 0;
					hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
					lUpdateBuf = 0;
				}
				ProcessORARecord (*hDC,FidMap,CurrentORARec); 
				if (FastMapCopy)
				{
					LPSTR	pRec = GlobalLock (hUpdateBuf);

					CopyRec = FALSE;
					BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
					BigWrite (FastMapCopyFid,pRec,lUpdateBuf,-1);
					GSSiGlobUlFree (&hUpdateBuf);
					FastMapCopynRecs++;
					if (!(FastMapCopynRecs % 100))
					{
						char	mess[64];

						sprintf (mess,"%i records copied",FastMapCopynRecs);
						StatusExtraInfoUpdate(mess);
					}
				}
			}
			n++;
		} while (n < maxn && FindNextSegment());
        if (n < maxn)
        	goto Next;
 		goto RtnTrue;
    }
	else if (MapType == MT_GMD)
	{
		MNMXCORD	GMDBounds;
		short	n = 0, maxn = 100;

		do
		{
			ExpandGMDPointBounds(&GMDBounds);
			if (ForceRefIndex || ForceTAGIndex)
				StatusWindowUpdate2(0, NumGMDRecs, CurrentGMDRec);
			if (WantGMDNegGrid || RectInWBounds(&GMDBounds, 1))
				ProcessGMDRecord(*hDC, (HANDLE)FidMap, CurrentGMDRec);
			n++;
		} while (n < maxn && FindNextSegment());
		if (n < maxn)
			goto Next;
		goto RtnTrue;
	}
	else if (MapType == MT_SQLITE)
	{
		MNMXCORD	GMDBounds;
		short	n = 0, maxn = 100;

		do
		{
			ProcessSQLITERecord(*hDC,-1);
			n++;
		} while (n < maxn && ContinueProcessing && FindNextSegment());
		if (n < maxn)
			goto Next;
		goto RtnTrue;
	}
	else if (MapType == MT_DGN7)
	{
		ProcessDGNRecord(*hDC, -1);
		goto RtnTrue;
	}
	else if (MapType == MT_DGN8)
	{
		ProcessDGN8Record(*hDC, -1);
		goto RtnTrue;
	}
	else if (MapType == MT_GPX)
    {
		ProcessGPXRecord (*hDC,-1); 
		goto RtnTrue;
    }
//    starttime=clock(); 
    GSSillseek (FidMap,CurrentSeg,0);  
    if (CurrentSeg == DebugSeg)
    	ii=1; 
    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
/*    if (ContinuationOffset >= 0)
        DescScan (-2,nBytes);                   
    else
        DescScan (-1,nBytes);*/
    ContinuationOffset = -1;
    if (nBytes<=0)
        goto RtnTrue;  
    SegStart = GSSillseek (FidMap,0,1);
    hpltBuf = GSSiGlobAlloc (  45,GMEM_MOVEABLE,(DWORD)nBytes+32);
    LPpltBuf = GlobalLock (hpltBuf); 
    nRead = BigRead (FidMap,LPpltBuf,nBytes);  
//    MapIOTime+=clock()-starttime;
    if (nRead != nBytes) 
    {   
       	ProcessInvalidRecord (0,0,2);
        goto RtnFalse;
    }
    nPltBytesRead += nRead;  
	if (ForceRefIndex || ForceTAGIndex)
    	StatusWindowUpdate2 (0,CurPltFileLen,nPltBytesRead); 
    _fmemset ((LPSTR)(LPpltBuf+nBytes),0,32);
    nBlocksRead++; 
    lBlocksRead+=nRead;
    ipnt = (LPSHORT) LPpltBuf;
/*    if (ConvertMap)
    {
		ConvertPoly (ipnt,hTranFileToBaseOld,hTranBaseToFile);
        GSSillseek (Fid,SegStart,0);
        _lwrite (Fid,LPpltBuf,nBytes);
    }
    else*/
		if (FastMapCopy)
		{
			CopyRec = 2;
			UpdateItem = 0;
			hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
			lUpdateBuf = 0;
			HaveFirstHeader = FALSE;
		}
		ItemAddedToHLT = FALSE;
		HLTGraphicsPos = 0;
		nContinues = 1;
		while (FidMap != HFILE_ERROR && ContinueProcessing && CurView && ProcessGraphicsRec(*hDC, ipnt, LPpltBuf, nRead))
        {
			BOOL quitProcessing;
		    GSSiGlobUlFree (&hpltBuf);
            
			if (nContinues == 1)
				originalOffset = CurrentSeg;
			CurrentSeg = ContinuationOffset;
			if (CurrentSeg == DebugSeg)
				ii = 1;
		    GSSillseek (FidMap,ContinuationOffset,0);
		    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
		    hpltBuf = GSSiGlobAlloc (  48,GMEM_MOVEABLE,(DWORD)nBytes+2);
		    LPpltBuf = GlobalLock (hpltBuf);
		    LPpltBuf[nRead] = 0;
		    nRead = BigRead (FidMap,LPpltBuf,nBytes);
		    if (nRead != nBytes) 
		    {
				ProcessSingleItem=HaveFirstHeader=FALSE;
		    	InvalidItem (0,TRUE);
		    	goto RtnTrue;
		    }
		    ipnt = (LPSHORT)LPpltBuf;
			if (!(nContinues++ % 16))
			{
				SetContinueProcessing(CheckForContinue(TRUE, 0));
			}
        }
		if (FastMapCopy)
		{
			LPSTR	pRec = GlobalLock (hUpdateBuf);

			if (lUpdateBuf)
			{
				if (!*HLTGraphicsFile)
				{
					BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
					BigWrite (FastMapCopyFid,pRec,lUpdateBuf,-1);
				}
			}
			CopyRec = FALSE;
			GSSiGlobUlFree (&hUpdateBuf);
			FastMapCopynRecs++;
			if (!(FastMapCopynRecs % 100))
			{
				char	mess[64];

				sprintf (mess,"%i records copied",FastMapCopynRecs);
				StatusExtraInfoUpdate(mess);
			}
		}

RtnTrue:
//    CurView->WBounds = SaveWBounds;   
//    CurView->Bounds = SaveBounds;
    GSSiGlobUlFree (&hpltBuf);
    if (ContinueProcessing && *hDC)
    {
        SetDisplayMode (*hDC,GF_TEXTMODE);
        if (CurView && *hDC == CurView->hTransparentDC)
			SelectVPClipRgn (0);
		else
			SelectVPClipRgn (0); 
    } 
    if (nBytes<=0)
{
#if ENABLETRACE
GSSiExitProg (12);
#endif
		return TRUE;
}
	else   
{
#if ENABLETRACE
GSSiExitProg (12);
#endif
    	return (nBytes);
}
RtnFalse:
//    CurView->WBounds = SaveWBounds;   
//    CurView->Bounds = SaveBounds;
    GSSiGlobUlFree (&hpltBuf);
    if (CurView && *hDC)
    {
        SetDisplayMode (*hDC,GF_TEXTMODE);
        SelectClipRgn (*hDC,0);
    }
{
#if ENABLETRACE
GSSiExitProg (12);
#endif
    return (FALSE);
}
#if ENABLETRACE
}
#endif
}

int abcd (int i)
{
	static	char s[16+6+32]="jes32349mes80251";
	LPSTR	loc;

	if (!i)
		return 0;
	loc = s + 16;

	for (i=0;i<6;i++)
		MODULEIDSTRING[i] = *(LPSTR)(loc + 5-i) - 'z';
	loc = s + 22;
	for (i=0;i<32;i++)
		MODULENAME[i] = *(LPSTR)(loc + 31-i) - 'z';
	return 1;
}

int SetDisplayMode (HDC hDC, short Mode)
#if ENABLETRACE
{GSSiEnterProg (13);
#endif
{  
	short	ii;  
	short	CurMode;
	int		curMapMode = 0;
	int		CurGraphicsMode = 0;
	XFORM xForm; 
   	int	WindowFactor;

   if (!hDC || FileMode)
{
#if ENABLETRACE
GSSiExitProg (13);
#endif
   	return 0; 
}
   CurMode = GetMapMode (hDC);
   CurGraphicsMode = GetGraphicsMode(hDC);
   switch (Mode)
   {   
		case GF_MAPMODE:
		if (CurView && CurView->Rotation)
		{
			SetGraphicsMode(hDC, GM_ADVANCED);
			SetWindowOrgEx  ( hDC, CurView->wOrigX, CurView->wOrigY,0 );
			SetViewportOrgEx( hDC, CurView->DrawRect.left, CurView->DrawRect.top,0 );
			if (CurView->wExtX &&  CurView->wExtY)
        		SetWindowExtEx  ( hDC, CurView->wExtX, CurView->wExtY,0 ); 
			if (CurView->vExtX && CurView->vExtY) 
			{
        		if (!SetViewportExtEx( hDC, CurView->vExtX, CurView->vExtY,0 ))
        			ii=1;
			}
			SetWorldTransform(hDC, &CurView->xForm); 
    		if (CurrentConfig)
    			WindowFactor = ScreenWindowFactor;
    		else
    			WindowFactor = 1;
    		if (!SetMapMode    ( hDC, MM_ISOTROPIC ))
    			ii=1;
		}
		else
		{
    		if (!SetMapMode ( hDC, MM_ISOTROPIC ))
    			ii=1;
			SetWindowOrgEx  ( hDC, CurView->wOrigX, CurView->wOrigY,0 );
			SetViewportOrgEx( hDC, CurView->DrawRect.left, CurView->DrawRect.top,0 );
			if (CurView->wExtX &&  CurView->wExtY)
        		SetWindowExtEx  ( hDC, CurView->wExtX, CurView->wExtY,0 ); 
			if (CurView->vExtX && CurView->vExtY) 
			{
        		if (!SetViewportExtEx( hDC, CurView->vExtX, CurView->vExtY,0 ))
        			ii=1;
			}
		}
		break;
 
    case GF_TEXTMODE:
    	
		if (CurView && CurView->Rotation)
		{
/*			xForm.eM11 = (FLOAT) 0.8660; 
			xForm.eM12 = (FLOAT) 0.5000; 
			xForm.eM21 = (FLOAT) -0.5000; 
			xForm.eM22 = (FLOAT) 0.8660; 
			xForm.eDx  = (FLOAT) 0.0; 
			xForm.eDy  = (FLOAT) 0.0; */
			SetGraphicsMode(hDC, GM_ADVANCED);
			SetWorldTransform(hDC, &CurView->xForm); 
    		if (CurrentConfig)
    			WindowFactor = ScreenWindowFactor;
    		else
    			WindowFactor = 1;
    		if (!SetMapMode    ( hDC, MM_ISOTROPIC ))
    			ii=1;
			SetWindowOrgEx  ( hDC, 0, 0,0 );
			SetViewportOrgEx( hDC, 0, 0,0 );    
       		SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
			SetViewportExtEx( hDC, 1024/WindowFactor, 1024/WindowFactor,0 );  
		}
		else
		{
			if (GetGraphicsMode(hDC) == GM_ADVANCED)
			{
				xForm.eM11 = (FLOAT) 1.0; 
				xForm.eM12 = (FLOAT) 0.0; 
				xForm.eM21 = (FLOAT) 0.0; 
				xForm.eM22 = (FLOAT) 1.0; 
				xForm.eDx  = (FLOAT) 0.0; 
				xForm.eDy  = (FLOAT) 0.0; 
				SetWorldTransform(hDC, &xForm); 
			}
			SetGraphicsMode(hDC, GM_COMPATIBLE);
    		if (CurrentConfig)
    			WindowFactor = ScreenWindowFactor;
    		else
    			WindowFactor = 1;
    		if (!SetMapMode    ( hDC, MM_ISOTROPIC ))
    			ii=1;
			SetWindowOrgEx  ( hDC, 0, 0,0 );
			SetViewportOrgEx( hDC, 0, 0,0 );    
       		SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
			SetViewportExtEx( hDC, 1024/WindowFactor, 1024/WindowFactor,0 );  
		}
		break;

		case GF_SCREENMODE:
			if (GetGraphicsMode(hDC) == GM_ADVANCED)
			{
				xForm.eM11 = (FLOAT) 1.0; 
				xForm.eM12 = (FLOAT) 0.0; 
				xForm.eM21 = (FLOAT) 0.0; 
				xForm.eM22 = (FLOAT) 1.0; 
				xForm.eDx  = (FLOAT) 0.0; 
				xForm.eDy  = (FLOAT) 0.0; 
				SetWorldTransform(hDC, &xForm); 
			}
			SetGraphicsMode(hDC, GM_COMPATIBLE);
    		if (CurrentConfig)
    			WindowFactor = ScreenWindowFactor;
    		else
    			WindowFactor = 1;
    		if (!SetMapMode    ( hDC, MM_ISOTROPIC ))
    			ii=1;
			SetWindowOrgEx  ( hDC, 0, 0,0 );
			SetViewportOrgEx( hDC, 0, 0,0 );    
       		SetWindowExtEx  ( hDC, 1024, 1024,0 ); 
			SetViewportExtEx( hDC, 1024/WindowFactor, 1024/WindowFactor,0 );
			break;
	}
	if (CurView && CompareDC && hDC != CompareDC)
		SetDisplayMode (CompareDC, Mode);
{
#if ENABLETRACE
GSSiExitProg (13);
#endif
    return curMapMode;
}
#if ENABLETRACE
}
#endif
}  

void ProjectMinMax (LPMINMAX pMinMax)
{   
	POINT	Point;
	mnmxCor	NewMinMax;
	
	switch (CurView->FileProjectionType)
	{   
		
	case 0:
	case 3:
		break;
	default:
		MinMaxInit (&NewMinMax);
		
		Point.x = pMinMax->xmn;
		Point.y = pMinMax->ymn;
		ProjectFilePt (&Point);
		AddPointToMinMax (Point,&NewMinMax);
		Point.x = pMinMax->xmn;
		Point.y = pMinMax->ymx;
		ProjectFilePt (&Point);
		AddPointToMinMax (Point,&NewMinMax);
		Point.x = pMinMax->xmx;
		Point.y = pMinMax->ymx;
		ProjectFilePt (&Point);
		AddPointToMinMax (Point,&NewMinMax);
		Point.x = pMinMax->xmx;
		Point.y = pMinMax->ymn;
		ProjectFilePt (&Point);
		AddPointToMinMax (Point,&NewMinMax);
		*pMinMax = NewMinMax;
		break;
	}
	return;
}

BOOL BlockInWindow (LPMINMAX pMinMaxIN, short From)
#if ENABLETRACE
{GSSiEnterProg (14);
#endif
{    
	LPMNMXCORD	lpRect; 
	POINT  Point;            
	mnmxCor	MinMax;
	LPMINMAX pMinMax=&MinMax;
	HANDLE	hMaskArea;
    
    if (CurView->DisplayInParent && CurView->Parent) 
    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea; 
    else
    	hMaskArea = CurView->hMaskArea;
     
     if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
     	return TRUE;
}
     if (!CurView->FileFactor)
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
     	return FALSE; 
}
     MinMax = *pMinMaxIN;
     if (CurView->FileProjectionType==1)
     	ProjectMinMax (pMinMax);
     if (From && pMinMax->xmn > pMinMax->xmx)
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
     	return TRUE;
}
     if (CurView->FileFactor == 1)
     {
	     if (pMinMax->xmn > CurView->Bounds.xmx ||
	         pMinMax->ymn > CurView->Bounds.ymx ||
	         pMinMax->xmx < CurView->Bounds.xmn ||
	         pMinMax->ymx < CurView->Bounds.ymn)
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
	         	return FALSE;
}
     }
     else
     {
	     if (pMinMax->xmn/CurView->FileFactor > CurView->Bounds.xmx ||
	         pMinMax->ymn/CurView->FileFactor > CurView->Bounds.ymx ||
	         pMinMax->xmx/CurView->FileFactor < CurView->Bounds.xmn ||
	         pMinMax->ymx/CurView->FileFactor < CurView->Bounds.ymn)
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
	         	return FALSE; 
}
	 }
     if (MaskOffsetLine && hMaskArea)
     { 
		DPOINT MinPoint, MaxPoint;
		MNMXCORD mnmx;
		
        Point.x = pMinMax->xmn;
        Point.y = pMinMax->ymn;
		MinPoint = FilePtToBasePt (Point);
        mnmx.xmn = MinPoint.x;
        mnmx.ymn = MinPoint.y;
        Point.x = pMinMax->xmx;
        Point.y = pMinMax->ymx;
		MaxPoint = FilePtToBasePt (Point);
        mnmx.xmx = MaxPoint.x;
        mnmx.ymx = MaxPoint.y; 
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
        return (RectInWBounds (&mnmx,From));
}
     }
{
#if ENABLETRACE
GSSiExitProg (14);
#endif
     return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL BlockCompletelyInWindow (mnmxCor MinMax)
#if ENABLETRACE
{GSSiEnterProg (15);
#endif
{    
	 LPMNMXCORD	lpRect; 
     POINT  Point;
     
     if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (15);
#endif
     	return TRUE;
}
     if (!CurView->FileFactor)
{
#if ENABLETRACE
GSSiExitProg (15);
#endif
     	return FALSE;
}
     if (CurView->FileProjectionType)
     	ProjectMinMax (&MinMax);
     if (MinMax.xmn/CurView->FileFactor >= CurView->Bounds.xmn &&
         MinMax.ymn/CurView->FileFactor >= CurView->Bounds.ymn &&
         MinMax.xmx/CurView->FileFactor <= CurView->Bounds.xmx &&
         MinMax.ymx/CurView->FileFactor <= CurView->Bounds.ymx)
     {
{
#if ENABLETRACE
GSSiExitProg (15);
#endif
	     return TRUE;
}
	 }
{
#if ENABLETRACE
GSSiExitProg (15);
#endif
     return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL RectInWBounds (LPMNMXCORD pMinMax,short From)
#if ENABLETRACE
{GSSiEnterProg (16);
#endif
{    
	BOOL	rtn=TRUE;
	LPMNMXCORD	lpRect;  
	HPDPOINT	lpDpoints;  
	DPOINT	AreaPoints[5];
	HANDLE	hMaskArea;     
	long	NumMaskPoints;
	LPHANDLE phMaskAccelerator;
    
    if (CurView->DisplayInParent && CurView->Parent)   
    {
    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea;
    	phMaskAccelerator = &pViewports[CurView->Parent-1]->hMaskAccelerator[0];  
    	NumMaskPoints = pViewports[CurView->Parent-1]->NumMaskPoints;
    }
    else 
    {
    	hMaskArea = CurView->hMaskArea;
    	phMaskAccelerator = &CurView->hMaskAccelerator[0];
    	NumMaskPoints = CurView->NumMaskPoints;
    } 
	 
	if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (16);
#endif
	 	return TRUE;
}
     if (pMinMax->xmn > CurView->WBounds.xmx ||
         pMinMax->ymn > CurView->WBounds.ymx ||
         pMinMax->xmx < CurView->WBounds.xmn ||
         pMinMax->ymx < CurView->WBounds.ymn)
{
#if ENABLETRACE
GSSiExitProg (16);
#endif
         return (FALSE);
}
     if (MaskOffsetLine && hMaskArea && !BleedThrough)
     { 
	    lpRect = (LPMNMXCORD) GlobalLock(hMaskArea); 
	    if (pMinMax->xmn > lpRect->xmx ||
	        pMinMax->ymn > lpRect->ymx ||
	        pMinMax->xmx < lpRect->xmn ||
	        pMinMax->ymx < lpRect->ymn)
	        rtn = FALSE;
	    else// if (!From)//doesnt speed processing - check latter
	    {   
	    	double	AreaAZ[4];
	    	
		    lpRect++;
		    lpDpoints = (HPDPOINT) lpRect;  
            BoundsToPoints (pMinMax,AreaPoints,AreaAZ);   
            rtn = PolyInArea (GF_AREA,4,AreaPoints,AreaAZ,NumMaskPoints,lpDpoints,1,0,0,phMaskAccelerator);
        }
	    GlobalUnlock (hMaskArea);
     }
{
#if ENABLETRACE
GSSiExitProg (16);
#endif
     return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL PtInWBounds (LPDPOINT Point)
#if ENABLETRACE
{GSSiEnterProg (17);
#endif
{    if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (17);
#endif
		 return TRUE;
}
	
     if (Point->x > CurView->WBounds.xmx ||
         Point->y > CurView->WBounds.ymx ||
         Point->x < CurView->WBounds.xmn ||
         Point->y < CurView->WBounds.ymn)
{
#if ENABLETRACE
GSSiExitProg (17);
#endif
         	return (FALSE);
}
{
#if ENABLETRACE
GSSiExitProg (17);
#endif
     return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL RedisplayViewport (BOOL Imediate, BOOL OnePass)
#if ENABLETRACE
{GSSiEnterProg (18);
#endif
{   short iview;
    LPVIEWPORT  SaveView, SaveInVP=CurView; 
    short DisplayVP; 
    BOOL	rtn,Final=1;
    HDC	hDC,hDCMain = GetDC (hWndMain);  
	BOOL	SaveDisplay[MAX_VIEWPORTS];
    
	if (!Imediate)
		InImediate = FALSE;
	if (MemMap)
    	hDC = hdcMemMap; 
	else if (!BufferedScreen)
		hDC = CurView->hDC;
	else if (MapServer)
		hDC = GetDC(NULL);
	else
    	hDC = hDCMain;	
	BlockSocketProcessing (1);
	if (CurView->hWnd)
	{
	hDC = ScreenBufferDC (CurView->hWnd,hDC);  
	ReleaseDC (hWndMain,hDCMain);
	for (iview=0;iview<*pNumViewports;iview++)  
		pViewports[iview]->hDC = hDC;    
	}
    
//BlockSocketProcessing (5);
	if (!CurView->ID && Imediate)
	{   
		Display = FALSE;
		NumViewportsToDisplay = -1;    
		CurView->CurFile = -1;
	    if (OnePass)       
	        CurView->PassID = 5; 
//BlockSocketProcessing (6);
	    if (!DisplayViewport (CurView->hWnd,CurView->hDC,Imediate))   
	    	rtn = FALSE; 
	    else
	    {
	    	ImediateProcessing (Imediate,FALSE); 
	    	rtn = TRUE; 
		}
	    if (SaveInVP)
   	    	SetCurView (SaveInVP);
   	    if (!rtn)
		 	EndDisplayProcessing (TRUE);
 
		BlockSocketProcessing (FALSE);
{
#if ENABLETRACE
GSSiExitProg (18);
#endif
    	return rtn;  
}   
	}
	ClearFullWindowBitmap (0);
    DisplayCycle++;  
    TotCopySize = 0;
    PrevLayerVP=0;
    if (!CurView)  
    {

		BlockSocketProcessing (FALSE);
{
#if ENABLETRACE
GSSiExitProg (18);
#endif
    	return FALSE;
}   
	}
//BlockSocketProcessing (8);
	if (!GetGlobalBVal ("[%IDM]"))
		Imediate=TRUE;
    if (RedisplayOnly)
    {
		PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
{
#if ENABLETRACE
GSSiExitProg (18);
#endif
    	return TRUE;
}
    }
//    if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum); 
    
//BlockSocketProcessing (9);
    ResetToViewport = CurView;  
    if (!InShowZoomArea)
		InLinkedList (-1);
    if (Imediate < 2)
		HaltMapDisplay(FALSE,FALSE);
	useGDIPlus = wantGDIPlus;
	SetContinueProcessing(TRUE);
    //GMEnableMenuItem(hWndMain, IDM_Z_ORTHO, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
    NumViewportsToDisplay = *pNumViewports;
    DisplayViewID=0; 
   // FormatViewport = 0;  
    for (iview=0;iview<*pNumViewports;iview++) 
    {
		SaveDisplay[iview] = pViewportsD[iview]->Display;
        pViewportsD[iview]->Display = FALSE;
        if (!Imediate && !Printing && pViewportsD[iview]->Active && !CurView->Type)  
        {
	        //FormatViewport = pViewports[iview]->ID;
	        pViewportsD[iview]->Display = TRUE;
	        SetCurView ( pViewportsD[iview]);
	        ResetViewport (TRUE,FALSE);
	    }
	}
	SetCurView ( ResetToViewport);
    
NextLink:
    SaveView=CurView;  
    DisplayVP = CurView->ID;
    for (iview=0;iview<*pNumViewports;iview++)
    {   
        SetCurView ( pViewports[iview]); 
        if (CurViewActive ())
        {
             if (CurView->pTheme)
             {
             	if (CurView->pTheme->TargetViewport == DisplayVP)
                	CurView->Display = TRUE; 
             }
             if (CurView->DisplayInParent && CurView->Parent == DisplayVP)
             {
                CurView->Display = TRUE;
			    ResetViewport (TRUE,FALSE);  
    	        SaveInVP = 0;
			 }
        }
    }
    SetCurView (SaveView); 
    CurView->Display = TRUE;
    ResetViewport (TRUE,FALSE); 
    if (!CurView->LinkedTo || InLinkedList(CurView->LinkedTo))
        SetCurView ( ResetToViewport);
    else
    {   
    	if (CurView->hFileTransIn != (HANDLE)1 &&
    		pViewports[CurView->LinkedTo-1]->hFileTransIn != (HANDLE)1)
    	{
 	       SetCurView ( pViewports[CurView->LinkedTo-1]);  
 	       SaveInVP = 0;
	       goto NextLink;
        }
    }
/*  ResetOvrlapViewport (CurView->Rect,CurView->ID);*/
    
    if (OnePass)
        CurView->PassID = 5; 
    if (!DisplayViewport (CurView->hWnd,CurView->hDC,Imediate))   
    {
	    if (SaveInVP)
   	    	SetCurView (SaveInVP); 
		if (Imediate < 2)
			EndDisplayProcessing (TRUE);
		BlockSocketProcessing (FALSE);
{
#if ENABLETRACE
GSSiExitProg (18);
#endif
    	return FALSE;  
}   
	}
	BlockSocketProcessing (2);
    ImediateProcessing (Imediate,TRUE);   
	if (Imediate)
	{
		for (iview=0;iview<*pNumViewports;iview++)
			pViewportsD[iview]->Display = SaveDisplay[iview];
		BlockSocketProcessing (FALSE);
	}
	else
	    InDisplayProcessing = TRUE;   
    if (SaveInVP)
    	SetCurView (SaveInVP); 
{
#if ENABLETRACE
GSSiExitProg (18);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void ImediateProcessing (BOOL Imediate,BOOL Final)    
#if ENABLETRACE
{GSSiEnterProg (19);
#endif
{
    BOOL    Continue,DoPeek=!MapServer;
    
    if (Imediate)       
    {
        MSG         msg, SZmsg;  
        BOOL		SaveInDisplayProcessing = InDisplayProcessing;
		LPVIEWPORT	SaveVP = CurView;

		if (MemMap)
			DoPeek = FALSE;
		useGDIPlus = wantGDIPlus;
		InDisplayProcessing=TRUE;
        Continue=TRUE; 
        InImediate = TRUE;
		memset (&SZmsg,0,sizeof(SZmsg));
        while (Continue && ContinueProcessing)
        {   
            if (OpenMap (CurView->hWnd,CurView->hDC))
            {   
            	HaveSeg=TRUE;
                if (DisplaySeg (&CurView->hDC,Imediate))
                {   
                    while (DoPeek && Continue && GSSiPeekMessage(&msg, hWndMain, 0, WM_COMMAND, PM_REMOVE))
                    {   
#if ENABLETRACE
						SetLastMessage(-1 * (long)msg.message, msg.wParam);
#endif

                    	switch (msg.message)
                    	{   
							case WM_KEYDOWN:
								if (msg.wParam == 27) 
									Continue = FALSE;
								break;
							case WM_DESTROY:
                    		case WM_CLOSE:
                    			Continue = FALSE;
                    			PostMessage (msg.hwnd,msg.message,msg.wParam,msg.lParam);
                    			break;
                    		case WM_COMMAND:
								if (PrintMsgWnd && IsDialogMessage(PrintMsgWnd, &msg)) 
                    		    	break;
                    		    if (msg.wParam == IDM_EXIT)
                    		    {   
                    				Continue = FALSE;
                    				PostMessage (msg.hwnd,msg.message,msg.wParam,msg.lParam);
                    			} 
                   				break;
							case WM_LBUTTONUP:
							{
    							POINT	ButtonPoint = POINTStoPOINT(MAKEPOINTS(msg.lParam));

								if (PtInRect (&EscRect,ButtonPoint))
								{
									EscapeFunction (TRUE);
									break;
								}
							}
							case WM_RBUTTONUP:
							case WM_LBUTTONDOWN:
							case WM_RBUTTONDOWN:
							case WM_LBUTTONDBLCLK:
								if (InSmoothZoom)
									SZmsg = msg;
								break;
                    		case WM_PAINT:
                    		case WM_COMMNOTIFY:
					    		TranslateMessage(&msg);
      							DispatchMessage(&msg);
      						default:
      							break;
      					}
                    }                                      
                }
                else
                {   
//                  EndDisplayProcessing (FALSE);   
                    Continue=FALSE;
                }
            }
            else
                Continue=FALSE;   
        }
		CurView = SaveVP;
        if (Imediate < 2)
			EndDisplayProcessing (Final); //moved from above 5/13/99 to fix peoplenet null map
		else
			BlockSocketProcessing (FALSE);
		InDisplayProcessing=SaveInDisplayProcessing;
        InImediate = FALSE;
		if (SZmsg.message)
			PostMessage (SZmsg.hwnd,SZmsg.message,SZmsg.wParam,SZmsg.lParam);
/*       	while (DisplayNetMarkerThemeLegend(FALSE)); 
       	if (NetMarkTimer)
       	{
			KillTimer(hWndMain,NetMarkTimer);
	       	NetMarkTimer = 0;
	    }
*/        
    }
{
#if ENABLETRACE
GSSiExitProg (19);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ResetOvrlapViewport (RECT rect, short ID)
#if ENABLETRACE
{GSSiEnterProg (20);
#endif
{   LPVIEWPORT  SaveView; 
    short         iview;
    RECT        IntRect;
    
    SaveView = CurView;
    for (iview=0;iview<*pNumViewports; iview++)
    {
        SetCurView (pViewports[iview]);
        if (CurView->ID != ID && IntersectRect (&IntRect,&rect,&CurView->Rect))
        {
            ResetViewport(TRUE,FALSE);
        }
    }
    SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (20);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
 
void RedisplayViewports (BOOL Imediate)
#if ENABLETRACE
{GSSiEnterProg (21);
#endif
{   short iview;

	if (!Imediate)
		InImediate = FALSE;
	ClearFullWindowBitmap(0);
    NumViewportsToDisplay = *pNumViewports;
    DisplayViewID=0;
    for (iview=0;iview<NumViewportsToDisplay;iview++)
    {   
        SetCurView ( pViewports[iview]); 
        CurView->Display = TRUE;
        ResetViewport (TRUE,FALSE);
    } 
    if (DisplayViewport (CurView->hWnd,CurView->hDC,Imediate))
    {
		if (Imediate)
    		ImediateProcessing (Imediate,TRUE);
		else
			InDisplayProcessing = TRUE;   
    }
	if (!idTimer && !CurrentConfig)
	{
		SetConfigDisplayRect ();
	 	SetConfig(1);
	   	InvalidateRect (hWndMain,&ConfigDisplayRect,TRUE);
	}
{
#if ENABLETRACE
GSSiExitProg (21);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

short ZoomWindow (HWND hWnd, float ZoomFactor,BOOL Imediate)
#if ENABLETRACE
{GSSiEnterProg (22);
#endif
{    double bwidth, bheight, MidX, MidY;
     
	 setDoPaint( TRUE);
     if (ZoomFactor)
	 {
		 CurView->Scale /= ZoomFactor;
		 ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,Imediate);
	 }
 /*    {
	     MidX = (CurView->WBounds.xmn + CurView->WBounds.xmx) / 2;
	     MidY = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
	     bwidth = (CurView->WBounds.xmx - CurView->WBounds.xmn)/ZoomFactor;
	     bheight = (CurView->WBounds.ymx - CurView->WBounds.ymn)/ZoomFactor;
	     CurView->NewBounds.xmn = MidX - bwidth/2;
	     CurView->NewBounds.ymn = MidY - bheight/2;
	     CurView->NewBounds.xmx = MidX + bwidth/2;
	     CurView->NewBounds.ymx = MidY + bheight/2; 
	 }*/
	 else if (CurView->PriorBounds.xmx > CurView->PriorBounds.xmn)
	 {
	 	CurView->NewBounds = CurView->PriorBounds; 
		ZoomToRect(CurView->NewBounds,Imediate);
	 }
{
#if ENABLETRACE
GSSiExitProg (22);
#endif
     return (0);
}
#if ENABLETRACE
}
#endif
}

void ZoomToBM ()
#if ENABLETRACE
{GSSiEnterProg (23);
#endif
{    
    LPVISLIST   SavelpVis;
    VISLIST SaveVis; 
    short     i, SaveMaxOrthoRes=MaxOrthoRes;
	double dval;
    
	SavelpVis = CurVis;
	SaveVis = *CurVis;
	if (dval = GetGlobalDVal2("[%CURORTHORES]", 0))
	{
		CurView->OrthoRes = dval;
		goto GotFile;
	}
    MaxOrthoRes=0;
    CurView->WindowZoomedToOrtho = TRUE;
    for (i=0;i<CurView->NumFiles;i++)
    {
		if (CurView->FileType[i] != 5 && MapFileType(CurView->lpFiles[i], 0, 0) != MT_SID)
            CurVis->FileIsVisible[i]=FALSE;
    }
    CurView->PassID = 1;
    CurView->CurFile=-1;  
//    IncrementFile ();
    while (GetNextViewportFile (FALSE))
    {
		if (PltType == 5 || (CurView->CurFile >= 0 && MapFileType(CurView->lpFiles[CurView->CurFile], 0, 0) == MT_SID))
            goto GotFile;
    }   
    CurVis = SavelpVis;
    *CurVis = SaveVis;    
    MaxOrthoRes = SaveMaxOrthoRes;
{
#if ENABLETRACE
GSSiExitProg (23);
#endif
    return; 
}
    
GotFile:
    MaxOrthoRes = SaveMaxOrthoRes;
    CurVis = SavelpVis;
    *CurVis = SaveVis;
    CurView->WindowIsZoomed = TRUE;
    SetNewBoundsToOrtho();       
    DisplayCycle++;
	//SetScaleAndMidpointFromBounds (CurView);
//	SetBounds (CurView->hWnd,NULL);
	CurView->WindowIsZoomed = TRUE; 
    SetBounds (CurView->hWnd,CurView->hDC);
    DisplayCycle--;
    CurView->CurZoomAreaRef = 0;
    RedisplayViewport(FALSE,FALSE);
{
#if ENABLETRACE
GSSiExitProg (23);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void SetNewBoundsToOrtho()
#if ENABLETRACE
{GSSiEnterProg (24);
#endif
{    double bwidth, bheight, MidX, MidY, Scale;
     double LogPixsX; 
     short	ipt;
     
     if (!CurView->OrthoRes)
{
#if ENABLETRACE
GSSiExitProg (24);
#endif
     	return;  
}
     MidX = (CurView->WBounds.xmn + CurView->WBounds.xmx) / 2;
     MidY = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
     if (CurView->OrthoRes < 0)
     {  
     	if (CurView->DesiredWidth)
     		LogPixsX = ((double)CurView->Rect.right - (double)CurView->Rect.left + 1.0)/ CurView->DesiredWidth;
     	else
        	LogPixsX = (double)GetDeviceCaps(CurView->hDC, LOGPIXELSX); 
        if (! PRJ_TYPE[1])
        {   
        	DPOINT	p1,p2;
        	
        	p1.x = MidX - 0.5;
			p2.x = MidX + 0.5;
			p1.y = p2.y = MidY;
			CurView->MetersPerDegreeX = ArcDistance (p1,p2);
        	Scale = -(CurView->OrthoRes/LogPixsX) / CurView->MetersPerDegreeX;  
        	CurView->BaseUnitsPerPixel =  Scale;
        }
        else
        	Scale = -(CurView->OrthoRes/LogPixsX);  
     }
     else
        Scale = CurView->OrthoRes;  
     if (CurView->WindowZoomedToOrtho == 2)
     { 
     	double CurScale = (CurView->NewBounds.xmx - CurView->NewBounds.xmn)/
     					  ((long)CurView->DrawRect.right - (long)CurView->DrawRect.left +1); 
     	Scale = max (Scale,CurScale);
     }
	 CurView->Scale = Scale;
 //checkvp(1);	
	 bwidth = ((long)CurView->DrawRect.right - (long)CurView->DrawRect.left +1) *  CurView->Scale;
     bheight = ((long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top +1) * CurView->Scale;
     CurView->NewBounds.xmn = MidX - bwidth/2;
     CurView->NewBounds.ymn = MidY - bheight/2;
     CurView->NewBounds.xmx = MidX + bwidth/2;
     CurView->NewBounds.ymx = MidY + bheight/2;
{
#if ENABLETRACE
GSSiExitProg (24);
#endif
     return;
}
#if ENABLETRACE
}
#endif
}

void SetLinkedVPBounds (LPMNMXCORD TransBounds,LPMNMXCORD NewBounds,double Scale,RECT Rect,BOOL LinkToScale)
{   
	DPOINT	MidPoint = MinMaxMidPointD (TransBounds);
	 
	if (LinkToScale)
	{ 
		NewBounds->xmn = MidPoint.x - Scale * (double)(Rect.right - Rect.left)/2;
		NewBounds->xmx = MidPoint.x + Scale * (double)(Rect.right - Rect.left)/2;
		NewBounds->ymn = MidPoint.y - Scale * (double)(Rect.bottom - Rect.top)/2;
		NewBounds->ymx = MidPoint.y + Scale * (double)(Rect.bottom - Rect.top)/2;
	}
	else
        *NewBounds = *TransBounds;
    return; 
}

void SetBounds (HWND hWnd,HDC hDC)
//if hDC == 1 does not set prior bounds
#if ENABLETRACE
{GSSiEnterProg (25);
#endif
{    
    MNMXCORD    SaveBounds, TransBounds;
    LPVIEWPORT  SaveView=CurView;
    LPVISLIST	SaveVis=0;
    UINT		lSaveVis = 0;
	RECT		Rect1;
    
	if (!IsBadWritePtr(CurVis, 4))
		SaveVis = CurVis;
    if (CurView->Type == VPTYPE_PROFILE || 
       (!CurView->NumFiles && !(CurView->pTheme && CurView->pTheme->ID == GF_COMPARE_VIEWPORTS_THEME)))
{
#if ENABLETRACE
GSSiExitProg (25);
#endif
    	return; 
}
	if (CurView->BoundsDisplayCycle == DisplayCycle)
	{
		SetBounds2(hWnd, CurView->hDC);
#if ENABLETRACE
		GSSiExitProg(25);
#endif
		return;
	}
	if (SaveVis)
    	lSaveVis = GlobalSize (SaveVis->hVisList);
	CurView->BoundsDisplayCycle = DisplayCycle;
    SetNewBounds = TRUE;
    CancelWindowZoom();
    if (hDC>(HDC)1) 
    	CurView->PriorBounds = CurView->WBounds; 
    else if (hDC)
    	hDC = CurView->hDC;
    if (CurView->OrthoRes < 0 && CurView->WindowZoomedToOrtho)
    {
        CurView->WBounds = CurView->NewBounds;
        SetNewBoundsToOrtho();
    }
    if (!InShowZoomArea)
		InLinkedList(-1);
    InLinkedList(CurView->ID);
    Rect1 = CurView->ScreenRect;

	LPINT pInt = GlobalLock(hNulls);
	GlobalUnlock(hNulls);

    SetBounds2 (hWnd,hDC); //pViewports[7]
    SaveBounds = CurView->NewBounds;
    if (TranFileBounds (1,&SaveBounds) != 2) 
    {
		DPOINT	FromPt[2], ToPt[2], FileBasePt[2], BaseCenterpoint;
		double	Scale;
	
//    	double Scale1=(SaveBounds.xmx - SaveBounds.xmn)/((long)CurView->DrawRect.right - (long)CurView->DrawRect.left);
    	FromPt[0].x = SaveBounds.xmn;
		FromPt[0].y = SaveBounds.ymn;
    	FromPt[1].x = SaveBounds.xmx;
		FromPt[1].y = SaveBounds.ymx;
		BaseCenterpoint = MinMaxMidPointD (&SaveBounds);
		//TranFilePoint (1,&BaseCenterpoint);
		ToPt[0] = FromPt[0];
		ToPt[1] = FromPt[1];
		TranFilePoint (2,&ToPt[0]);
		TranFilePoint (2,&ToPt[1]);
		ToPt[0] = BasePtToWinPtD (&ToPt[0]);
		ToPt[1] = BasePtToWinPtD (&ToPt[1]);
		Scale = ldistp (FromPt[0],FromPt[1]) / ldistp (ToPt[0],ToPt[1]);
//		Scale = LDIST (SaveBounds.xmn,SaveBounds.ymn,SaveBounds.xmx,SaveBounds.ymx) /
//    					LDIST (CurView->DrawRect.left,CurView->DrawRect.bottom,CurView->DrawRect.right,CurView->DrawRect.top);
    	
	    while (!Printing && CurView->LinkedTo && !InLinkedList(CurView->LinkedTo))
	    {   
			DPOINT	Center=BaseCenterpoint;
			double	Scale2 = Scale;
			double	ScaleFactor=1;

	        SetCurView ( pViewports[CurView->LinkedTo-1]); 
			if (CurView->BoundsDisplayCycle != DisplayCycle)
			{
	    		TransBounds = SaveBounds;  
	    		switch (TranFileBounds (2,&TransBounds))
	    		{   
	    			case 0:
	    				break;  
	    			case 1:
						TranFilePoint (2,&Center);
    					FromPt[0].x = SaveBounds.xmn;
						FromPt[0].y = SaveBounds.ymn;
    					FromPt[1].x = SaveBounds.xmx;
						FromPt[1].y = SaveBounds.ymx;
						FileBasePt[0] = FromPt[0];
						FileBasePt[1] = FromPt[1];
						TranFilePoint (2,&FileBasePt[0]);
						TranFilePoint (2,&FileBasePt[1]);
						if (CurView->LinkToScale)
							ScaleFactor = ldistp (FileBasePt[0],FileBasePt[1]) / ldistp (FromPt[0],FromPt[1]);
 						ToPt[0] = BasePtToWinPtD (&FileBasePt[0]);
						ToPt[1] = BasePtToWinPtD (&FileBasePt[1]);
						Scale2 = ldistp (FromPt[0],FromPt[1]) / ldistp (ToPt[0],ToPt[1]);
						CurView->WBounds=CurView->NewBounds=FactorBounds (&CurView->WBounds,Scale/Scale2);
						CreateBaseToVPTran (CurView->DrawRect);
 						ToPt[0] = BasePtToWinPtD (&FileBasePt[0]);
						ToPt[1] = BasePtToWinPtD (&FileBasePt[1]);
						Scale2 = ldistp (FileBasePt[0],FileBasePt[1]) / ldistp (ToPt[0],ToPt[1]);

	//					Scale = LDIST (TransBounds.xmn,TransBounds.ymn,TransBounds.xmx,TransBounds.ymx) /
	//					    					LDIST (CurView->DrawRect.left,CurView->DrawRect.bottom,CurView->DrawRect.right,CurView->DrawRect.top);
	    				break;
	    			case 2:
	    				goto Next; 
	    		}
	    		if (CurView->WindowZoomedToOrtho == 1)
	        		CurView->WindowZoomedToOrtho = 0;  
				CurView->WindowIsZoomed =  TRUE;
				CurView->MidPointW = Center;
				if (CurView->LinkToScale)
					CurView->Scale = Scale*ScaleFactor;
				else
				{
					double Factor1 = (double)(CurView->ScreenRect.right - CurView->ScreenRect.left)/(double)(Rect1.right - Rect1.left);
					double Factor2 = (double)(CurView->ScreenRect.bottom - CurView->ScreenRect.top)/(double)(Rect1.bottom - Rect1.top);

					CurView->Scale = Scale2 / min (Factor1,Factor2);
				}
	//		    SetBounds2 (hWnd,hDC);
	/*	        SetLinkedVPBounds (&TransBounds,&CurView->NewBounds,Scale,CurView->DrawRect,CurView->LinkToScale);
				CurView->WBounds = CurView->NewBounds;
				if (CurView->LinkToScale)
				{
					CurView->MidPointW = MinMaxMidPointD (&CurView->NewBounds);
					CurView->Scale = Scale;
				}
				else
					SetScaleAndMidpointFromBounds (CurView);
				CurView->BoundsDisplayCycle = DisplayCycle;*/
				SetBounds2 (hWnd,hDC); 
				if (hDC>(HDC)1) 
		    		CurView->PriorBounds = CurView->WBounds; 
			}
	Next:;
	    }
	}   
    SetCurView ( SaveView);

    if (lSaveVis && !IsBadWritePtr (SaveVis,lSaveVis))  
    	CurVis = SaveVis;
	if (CurView->MaxOffsetDist)
	{
		int	    inc;
		double  Offset=CurView->MaxOffsetDist;
		double	pct = Offset / (CurView->WBounds.xmx - CurView->WBounds.xmn);

		CurView->WBounds.xmn -= Offset;
		CurView->WBounds.xmx += Offset;
		inc = IDNINT (RECTWIDTH (&CurView->DrawRect) * pct);
		inc = IDNINT (Offset/CurView->BaseUnitsPerPixel);
		CurView->DrawRect.left -= inc;
		CurView->DrawRect.right += inc;
		pct = Offset / (CurView->WBounds.ymx - CurView->WBounds.ymn);
		inc = IDNINT (RECTHEIGHT (&CurView->DrawRect) * pct);
		inc = IDNINT (Offset/CurView->BaseUnitsPerPixel);
		CurView->WBounds.ymn -= Offset;
		CurView->WBounds.ymx += Offset;
		CurView->NewBounds = CurView->WBounds;
		CurView->DrawRect.bottom += inc;
		CurView->DrawRect.top -= inc;
	}
 
{
#if ENABLETRACE
GSSiExitProg (25);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

BOOL InLinkedList (short LinkedTo)
#if ENABLETRACE
{GSSiEnterProg (26);
#endif
{   
    short i;
    static  short nInList=0, InList[MAX_VIEWPORTS];

	if (LinkedTo < 0)   
    {
        nInList = 0;
{
#if ENABLETRACE
GSSiExitProg (26);
#endif
        return (TRUE); 
}
    }

	for (i=0;i<nInList;i++)
    {
        if (InList[i] == LinkedTo)
{
#if ENABLETRACE
GSSiExitProg (26);
#endif
        	return TRUE;
}
    }
    InList[nInList++]=LinkedTo; 
{
#if ENABLETRACE
GSSiExitProg (26);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}

double GetVPScale (LPVIEWPORT CurView)
{
	double Scale;
	DPOINT	RectPoints[4], BoundsPoints[4];
	
	RectToDPoints (&CurView->ScreenRect,RectPoints);
	BoundsToPoints (&CurView->WBounds,BoundsPoints,0);
	Scale = ldistp (BoundsPoints[0],BoundsPoints[2])/ldistp (RectPoints[0],RectPoints[2]);
	return Scale;
}

MNMXCORD RotateBounds (LPMNMXCORD pBounds,double Rotation)
{
	DPOINT		BoundsPoints[4], MidPoint, Point;
	MNMXCORD	RBounds;
	UINT		i;

	DBoundsInit (&RBounds);
	BoundsToPoints (pBounds,BoundsPoints,0);
	MidPoint = MinMaxMidPointD (pBounds);
	for (i=0;i<4;i++)
	{
		Point  = dnewpt (MidPoint,getazd (&MidPoint,&BoundsPoints[i])+CurView->Rotation,ldistp (MidPoint,BoundsPoints[i]));
		AddDPointToMinMax (&Point,&RBounds);
	}
	return RBounds;
}

BOOL VPBoundsIsLocked(void)
{
	if (!stricmp (CurView->Name,"Index Map"))
		return TRUE;
	return FALSE;
}

BOOL SetVPOutputProjection (int ProjectionID,LPMNMXCORD pProjectedBounds)
{
	float	RSQMIN;
	double	XFROM[4], YFROM[4], XTO[4], YTO[4]; 

	CloseTRANS2 (&CurView->hTranProjectionToScreen);
	CloseTRANS2 (&CurView->hTranScreenToProjection);
	if (ProjectionID && pProjectedBounds)
	{
		XFROM[0] = XFROM[1] = pProjectedBounds->xmn;
		YFROM[0] = YFROM[3] = pProjectedBounds->ymn;
		XFROM[2] = XFROM[3] = pProjectedBounds->xmx;
		YFROM[1] = YFROM[2] = pProjectedBounds->ymx;
		XTO[0] = XTO[1] = CurView->ScreenRect.left;
		YTO[0] = YTO[3] = CurView->ScreenRect.bottom;
		XTO[2] = XTO[3] = CurView->ScreenRect.right;
		YTO[1] = YTO[2] = CurView->ScreenRect.top;
		CurView->hTranProjectionToScreen = STRAN2 (1762,XFROM,YFROM,XTO,YTO,4,&RSQMIN,1,0);  
		CurView->hTranScreenToProjection = STRAN2 (1762,XTO,YTO,XFROM,YFROM,4,&RSQMIN,1,0); 
		CurView->FileProjectionType = 3;
	}
	else
		CurView->FileProjectionType = 0;
	return TRUE;
}

void AdjustBoundsAndDrawRectToRotation (void)
{
	float	RSQMIN;
	double	XFROM[2], YFROM[2], XTO[2], YTO[2]; 
	DPOINT	Point1, Point2;
	POINT	Point;
	HANDLE	hTran;
	double	Scale;

	if (!CurView)
		return;
	CloseTRANS2(&CurView->hTranScreenToVP);
	CloseTRANS2 (&CurView->hTranVPToScreen);
	CurView->DrawRect = CurView->ScreenRect;

	Point  = RectMid (&CurView->ScreenRect);
	Point1 = PointToDPoint (Point);
	Point2 = dnewpt (Point1,-CurView->Rotation,100);
	XFROM[0] = Point1.x;
	YFROM[0] = Point1.y;
	XFROM[1] = Point1.x + 100;
	YFROM[1] = YFROM[0];
	XTO[0]   = XFROM[0];
	YTO[0]   = YFROM[0];
	XTO[1]   = Point2.x;
	YTO[1]	 = Point2.y;
	CurView->hTranScreenToVP = STRAN2 (1608,XFROM,YFROM,XTO,YTO,2,&RSQMIN,1,0);  
	CurView->xForm =  SetXFORMFromTRANS (CurView->hTranScreenToVP);
	CloseTRANS2 (&CurView->hTranScreenToVP);
	Point  = RectMid (&CurView->ScreenRect);
	Point1 = PointToDPoint (Point);
	Point2 = dnewpt (Point1,CurView->Rotation,100);
	XFROM[0] = Point1.x;
	YFROM[0] = Point1.y;
	XFROM[1] = Point1.x + 100;
	YFROM[1] = YFROM[0];
	XTO[0]   = XFROM[0];
	YTO[0]   = YFROM[0];
	XTO[1]   = Point2.x;
	YTO[1]	 = Point2.y;
	CurView->hTranScreenToVP = STRAN2 (1609,XFROM,YFROM,XTO,YTO,2,&RSQMIN,1,0);  
	CurView->hTranVPToScreen = STRAN2 (1610,XTO,YTO,XFROM,YFROM,2,&RSQMIN,1,0); 
	TRANRect (&CurView->DrawRect,CurView->hTranScreenToVP);
	CurView->WBounds = CurView->BoundsBeforeRotation;
	Point1 = MinMaxMidPointD (&CurView->WBounds);
	Point2 = dnewpt (Point1,CurView->Rotation,100);
	XFROM[0] = Point1.x;
	YFROM[0] = Point1.y;
	XFROM[1] = Point1.x + 100;
	YFROM[1] = YFROM[0];
	XTO[0]   = XFROM[0];
	YTO[0]   = YFROM[0];
	XTO[1]   = Point2.x;
	YTO[1]	 = Point2.y;
	hTran = STRAN2 (1611,XFROM,YFROM,XTO,YTO,2,&RSQMIN,1,0);  
	TranBounds (hTran,&CurView->WBounds);
	CurView->NewBounds = CurView->WBounds;
	CloseTRANS2 (&hTran);
	Scale = GetVPScale (CurView);
	return;
}

void SetBounds2 (HWND hWnd,HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (27);
#endif
{
	DPOINT	MidPointVP = RectMidD (&CurView->ScreenRect), Point;
	DPOINT	VPPoints[4];
	UINT	i;
	double	Factor;
	double	MinScale = 0.00001;
//checkvp(1);	
//	if (!CurView->Type && CurView->HaveBounds)
//		return;
	if (!CurView->Scale)
		CurView->Scale = MinScale;
	if (CurView->LastWidth && !CurView->WindowZoomedToOrtho)
	{
/*		Factor = ((((double)(CurView->ScreenRect.right - CurView->ScreenRect.left)) / CurView->LastWidth) +
				 (((double)(CurView->ScreenRect.bottom - CurView->ScreenRect.top)) / CurView->LastHeight));
		CurView->Scale /= (Factor/2);*/
		Factor = min ((((double)(CurView->ScreenRect.right - CurView->ScreenRect.left)) / CurView->LastWidth),
				 (((double)(CurView->ScreenRect.bottom - CurView->ScreenRect.top)) / CurView->LastHeight));
		CurView->Scale /= Factor;
	}
	if (CurView->Display)
	{
		CurView->LastWidth = CurView->ScreenRect.right - CurView->ScreenRect.left;
		CurView->LastHeight = CurView->ScreenRect.bottom - CurView->ScreenRect.top;
	}
//	if (!VPBoundsIsLocked())
 //checkvp(1);	
	{
		double w;
		int	ii=0;
		MNMXCORD	SaveBounds = CurView->NewBounds;
		static	double	lastscale;

		DBoundsInit (&CurView->NewBounds);
		RectToDPoints (&CurView->ScreenRect,VPPoints);
		for (i=0;i<4;i++)
		{
			Point  = dnewpt (CurView->MidPointW,getazd (&MidPointVP,&VPPoints[i]),ldistp (MidPointVP,VPPoints[i])*CurView->Scale);
		//	ProjectBasePt (&Point);
			AddDPointToMinMax (&Point,&CurView->NewBounds);
		}
		w = BoundsWidth (&CurView->NewBounds)/(CurView->LLNormFactor * 2);
		CurView->NewBounds.xmn = CurView->MidPointW.x - w;
		CurView->NewBounds.xmx = CurView->MidPointW.x + w;
		lastscale = CurView->Scale;
		if (ii) CurView->NewBounds = SaveBounds;
	}
 //checkvp(1);	
    CurView->BoundsBeforeRotation = CurView->WBounds = CurView->NewBounds;
/*	{
		VIEWPORT	SaveVP=*CurView;
		SetScaleAndMidpointFromBounds (CurView);
		*CurView = SaveVP;
	}*/
    if (CurView->OrthoRes > 0 && CurView->WindowZoomedToOrtho == 2)
    {
        CurView->WBounds = CurView->NewBounds;
        SetNewBoundsToOrtho();
    }
 //checkvp(1);	
	AdjustBoundsAndDrawRectToRotation ();
     //ClipRect = CurView->DrawRect;
	LPINT pInt = GlobalLock(hNulls);
	GlobalUnlock(hNulls);

     SetBoundsRect2 (CurView->DrawRect,hDC);  
     if (Display)
     {
	     RedrawActiveFunctions(GF_CLEAR);
	     ClearSavedScreens(CurView->hWnd,CurView,&CurView->Rect);
		 if (CurDataRectVP == CurView)
			 CurDataRectID = -1;
	     if (hDC && !ReorgFile)//3/27/06 && !DisableHalt)
	     {
	     	BoundsDisplayShow (CurView->lpBoundsDisplay,&CurView->BoundsDisplayed); //old way
	     	BoundsDisplayTheme (FALSE);
	     }
	 }
     ProcessZoomMacroFile (hDC);   
{
#if ENABLETRACE
GSSiExitProg (27);
#endif
     return;
}
#if ENABLETRACE
}
#endif
}

void CreateBaseToVPTran (RECT Rectx)
#if ENABLETRACE
{GSSiEnterProg (29);
#endif
{
     float  RSQMIN;
     double XWIN[4], YWIN[4], XBASE[4], YBASE[4]; 

	 CloseTRANS2(&CurView->hTranVPToBase);
	 CloseTRANS2(&CurView->hTranBaseToVP);
	 CloseTRANS2(&CurView->hTranScreenToBase);
	 CloseTRANS2(&CurView->hTranBaseToScreen);
     if (CurView->HaveBounds)
     {
		 if (CurView->Rotation != 0)
		 {
			 DPOINT CenterPointVP = RectMidD(&CurView->ScreenRect), CenterPointW = CurView->MidPointW;
			 DPOINT pt;
			 double az;
			 DPOINT ScreenPoints[4];
			 DPOINT WPoints[4];
			 int flip[4] = { 1,0,3,2 };

			 RectToDPoints(&CurView->ScreenRect, ScreenPoints);
			 for (int i = 0; i < 4; i++)
			 {
				 double dist = ldistp(CenterPointVP, ScreenPoints[i]);
				 dist *= CurView->Scale;
				 az = getazd(&CenterPointVP, &ScreenPoints[flip[i]]);
				 az = LTWOPI(az - CurView->Rotation);
				 WPoints[i] = dnewpt(CenterPointW, az, dist);
				 XWIN[i] = ScreenPoints[i].x;
				 YWIN[i] = ScreenPoints[i].y;
				 XBASE[i] = WPoints[i].x;
				 YBASE[i] = WPoints[i].y;
			 }
			 CurView->hTranScreenToBase = STRAN2(1855, XWIN, YWIN, XBASE, YBASE, 4, &RSQMIN, 1, 0);
			 CurView->hTranBaseToScreen = STRAN2(1856, XBASE, YBASE, XWIN, YWIN, 4, &RSQMIN, 1, 0);
		 }
		 XWIN[0] = CurView->DrawRect.left;
		 XWIN[1] = CurView->DrawRect.left;
		 XWIN[2] = CurView->DrawRect.right;
		 XWIN[3] = CurView->DrawRect.right;
		 YWIN[0] = CurView->DrawRect.bottom;
		 YWIN[1] = CurView->DrawRect.top;
		 YWIN[2] = CurView->DrawRect.top;
		 YWIN[3] = CurView->DrawRect.bottom;
		 XBASE[0]=CurView->WBounds.xmn;
	     XBASE[1]=CurView->WBounds.xmn;
	     XBASE[2]=CurView->WBounds.xmx;
	     XBASE[3]=CurView->WBounds.xmx;
	     YBASE[0]=CurView->WBounds.ymn;
	     YBASE[1]=CurView->WBounds.ymx;
	     YBASE[2]=CurView->WBounds.ymx;
	     YBASE[3]=CurView->WBounds.ymn;
	 }
	 else
     {
		 XWIN[0] = CurView->DrawRect.left;
		 XWIN[1] = CurView->DrawRect.left;
		 XWIN[2] = CurView->DrawRect.right;
		 XWIN[3] = CurView->DrawRect.right;
		 YWIN[0] = CurView->DrawRect.bottom;
		 YWIN[1] = CurView->DrawRect.top;
		 YWIN[2] = CurView->DrawRect.top;
		 YWIN[3] = CurView->DrawRect.bottom;
		 XBASE[0]=XWIN[0];
	     XBASE[1]=XWIN[1];
	     XBASE[2]=XWIN[2];
	     XBASE[3]=XWIN[3];
	     YBASE[0]=YWIN[0];
	     YBASE[1]=YWIN[1];
	     YBASE[2]=YWIN[2];
	     YBASE[3]=YWIN[3];
	 }  
/*	 if (CurView->hTranFormat)
	 {  
	 	UINT	i;
	 	
	 	for (i=0;i<4;i++)
		    TRANS2 (XWIN[i],YWIN[i],&XWIN[i],&YWIN[i],CurView->hTranBaseToWin); 
	 		
	 }*/
	 LPINT pInt = GlobalLock(hNulls);
	 GlobalUnlock(hNulls);

	 CurView->hTranVPToBase = STRAN2 (1612,XWIN,YWIN,XBASE,YBASE,4,&RSQMIN,1,0);
	 pInt = GlobalLock(hNulls);
	 GlobalUnlock(hNulls);
	 CurView->hTranBaseToVP = STRAN2 (1613,XBASE,YBASE,XWIN,YWIN,4,&RSQMIN,1,0);
	 pInt = GlobalLock(hNulls);
	 GlobalUnlock(hNulls);
	 if (!CurView->hTranScreenToBase)
	 {
		 CurView->hTranScreenToBase = STRAN2(1612, XWIN, YWIN, XBASE, YBASE, 4, &RSQMIN, 1, 0);
		 CurView->hTranBaseToScreen = STRAN2(1613, XBASE, YBASE, XWIN, YWIN, 4, &RSQMIN, 1, 0);
	 }
{
#if ENABLETRACE
GSSiExitProg (29);
#endif
     return;
}
#if ENABLETRACE
}
#endif
}

DPOINT BasePtToGoogleTilePt (LPDPOINT ppt,int level)
{
	DPOINT gpt;

	ConvertCoord (ppt,1,2);
	LatLongToPixelXYd (ppt->y,ppt->x, level, &gpt.x,&gpt.y);
	return gpt;
}

DPOINT WinPtSToBasePt (POINTS Point)
{
    return ScreenPtToBasePt (POINTStoPOINT(Point));
}

DPOINT ScreenPtToBasePt (POINT Point)
{
    DPOINT WinPointD, WorldPoint;

	WinPointD = EnlargedPoint(Point);
	return ScreenPtDToBasePt(WinPointD);
}

DPOINT ScreenPtDToBasePt (DPOINT WinPointD)
{
    DPOINT WorldPoint;
     
	if (!CurView->pTheme)
		CreateBaseToVPTran(CurView->DrawRect);
	else if (CurView->pTheme->ID != GF_PROFILE_THEME)
		CreateBaseToVPTran(CurView->DrawRect);
	else
	{
		CloseTRANS2(&CurView->hTranVPToBase);
		CloseTRANS2(&CurView->hTranBaseToVP);
		CurView->hTranVPToBase = STRANRectToBounds(&CurView->ProfileRect, &CurView->ProfileBounds);
		CurView->hTranBaseToVP = STRANBoundsToRect(&CurView->ProfileBounds, &CurView->ProfileRect);
	}
	if (CurView->hTranScreenToBase)
	{
		WorldPoint = TranPoint(&WinPointD, CurView->hTranScreenToBase);
	}
	else
	{
		WinPointD = TranPoint(&WinPointD, CurView->hTranScreenToVP);
		WorldPoint = TranPoint(&WinPointD, CurView->hTranVPToBase);
	}
	return WorldPoint;
}

DPOINT WinPtToBasePt (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (30);
#endif
{    DPOINT WinPointD, WorldPoint, VPPoint;
     
     if(!CurView->hTranBaseToVP)
     {
     	if (!CurView->pTheme) 
			CreateBaseToVPTran (CurView->DrawRect);
		else if (CurView->pTheme->ID != GF_PROFILE_THEME)
			CreateBaseToVPTran(CurView->DrawRect);
		else
		{
			CloseTRANS2(&CurView->hTranVPToBase);
			CloseTRANS2(&CurView->hTranBaseToVP);
			CurView->hTranVPToBase = STRANRectToBounds(&CurView->ProfileRect, &CurView->ProfileBounds);
			CurView->hTranBaseToVP = STRANBoundsToRect(&CurView->ProfileBounds, &CurView->ProfileRect);
		}
	 }
     WinPointD = EnlargedPoint (Point);
	 TRANS2(WinPointD.x, WinPointD.y, &VPPoint.x, &VPPoint.y, CurView->hTranScreenToVP);
	 TRANS2(VPPoint.x, VPPoint.y, &WorldPoint.x, &WorldPoint.y, CurView->hTranVPToBase);
	 /*{	//temp code
POINT	TestPoint=BasePtToWinPt (&WorldPoint); 
short	ii;
if (TestPoint.x != Point.x || TestPoint.y != Point.y)
	ii=1;
}*/     
{
#if ENABLETRACE
GSSiExitProg (30);
#endif
     return (WorldPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT WinPtToBasePtD (LPDPOINT WinPointD)
#if ENABLETRACE
{GSSiEnterProg (31);
#endif
{    DPOINT WorldPoint;
     
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);
     TRANS2 (WinPointD->x,WinPointD->y,&WorldPoint.x,&WorldPoint.y,CurView->hTranVPToBase);
{
#if ENABLETRACE
GSSiExitProg (31);
#endif
     return (WorldPoint);
}
#if ENABLETRACE
}
#endif
}

LPDPOINT ProjectBasePt (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (785);
#endif
{   
/*    if (CurView->hTranFormat)  
    {
   		TRANS2 (pPoint->x,pPoint->y,&pPoint->x,&pPoint->y,CurView->hTranFormat); 
    } */
	switch (CurView->FileProjectionType)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
		case 1: /* lat long normalization */
			pPoint->x *= CurView->LLNormFactor;
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
		case 2: //transformation
			*pPoint = TranPoint (pPoint,hTranProjection);  
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
		case 3: /* alternate projection */
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
	}
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
	return pPoint;
}
#if ENABLETRACE
}
#endif
}

LPDPOINT UnProjectBasePt (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (785);
#endif
{   
/*    if (CurView->hTranFormat)  
    {
   		TRANS2 (pPoint->x,pPoint->y,&pPoint->x,&pPoint->y,CurView->hTranFormat); 
    } */
	switch (CurView->FileProjectionType)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
		case 1: /* lat long normalization */
			pPoint->x /= CurView->LLNormFactor;
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
		case 2: //transformation
			*pPoint = TranPoint (pPoint,hTranProjectionReverse);  
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
		case 3: /* alternate projection */
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
			return pPoint;
}
	}
{
#if ENABLETRACE
GSSiExitProg (785);
#endif
	return pPoint;
}
#if ENABLETRACE
}
#endif
}

POINT BasePtFLTToScreenPt (LPFLTPOINT pWPointF)
{
	DPOINT	VPPoint, ScreenPointD, WPoint={pWPointF->x,pWPointF->y};

	VPPoint = BasePtToWinPtD (&WPoint);
	ScreenPointD = TranPoint (&VPPoint,CurView->hTranVPToScreen);
	return DPointToPoint (ScreenPointD);
}

POINT BasePtToScreenPt (LPDPOINT pWPoint)
{
	DPOINT	VPPoint, ScreenPointD;

	if (CurView->hTranBaseToScreen)
	{
		ScreenPointD = TranPoint(pWPoint, CurView->hTranBaseToScreen);
	}
	else
	{
		VPPoint = BasePtToWinPtD(pWPoint);
		ScreenPointD = TranPoint(&VPPoint, CurView->hTranVPToScreen);
	}
	return DPointToPoint (ScreenPointD);
}

DPOINT BasePtToScreenPtD (LPDPOINT pWPoint)
{
	DPOINT	VPPoint, ScreenPointD;

	if (CurView->hTranBaseToScreen)
	{
		ScreenPointD = TranPoint(pWPoint, CurView->hTranBaseToScreen);
	}
	else
	{
		VPPoint = BasePtToWinPtD(pWPoint);
		ScreenPointD = TranPoint(&VPPoint, CurView->hTranVPToScreen);
	}
	return ScreenPointD;
}

POINT BasePtToWinPt (LPDPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (32);
#endif
{   DPOINT WinPointD, Intmod;
 	DPOINT WinPointDt;
    POINT  WinPoint;
     
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);
     
/*     if (CurView->hTranFormat)  
     {
     		TRANS2 (WPoint->x,WPoint->y,&WinPointD.x,&WinPointD.y,CurView->hTranFormat); 
     		WPoint = &WinPointD;
     } */
     switch (CurView->FileProjectionType)
     {  
     	DPOINT PPoint;
     	
     	case 2:
			PPoint = *WPoint;
		    ProjectBasePt (&PPoint);
		    TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP); 
     		break;
     	case 1:
			PPoint = *WPoint;
		    TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP); 
			break;
     	case 0:
			TRANS2(WPoint->x, WPoint->y, &WinPointD.x, &WinPointD.y, CurView->hTranBaseToVP);
			break;
     	case 3: //google maps projection
			{
			PPoint = *WPoint;
			ConvertCoord(&PPoint, 1, GOOGLEMAPSPROJECTION);
     		TRANS2 (WPoint->x,WPoint->y,&WinPointDt.x,&WinPointDt.y,CurView->hTranBaseToVP);
     		TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranProjectionToScreen);
			}
     		break;
     }  
     WinPointD.x += (WinPointD.x < 0) ? -0.5 : 0.5;
     WinPointD.y += (WinPointD.y < 0) ? -0.5 : 0.5;
     WinPoint.x = min((double)INT_MAX,max((double)INT_MIN,WinPointD.x));
     WinPoint.y = min((double)INT_MAX,max((double)INT_MIN,WinPointD.y));
{
#if ENABLETRACE
GSSiExitProg (32);
#endif
     return (WinPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT BasePtToWinPtD (LPDPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (33);
#endif
{    DPOINT WinPointD = { 0 }, Intmod;
     POINT  WinPoint;
     
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);
     
/*     if (CurView->hTranFormat)  
     {
     		TRANS2 (WPoint->x,WPoint->y,&WinPointD.x,&WinPointD.y,CurView->hTranFormat); 
     		WPoint = &WinPointD;
     }*/
     switch (CurView->FileProjectionType)
     {  
     	DPOINT PPoint;
     	
     	case 2:
			PPoint = *WPoint;
		    ProjectBasePt (&PPoint);
		    TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP); 
     		break;
     	case 1:
			PPoint = *WPoint;
		    TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP); 
			break;
     	case 0:
     		TRANS2 (WPoint->x,WPoint->y,&WinPointD.x,&WinPointD.y,CurView->hTranBaseToVP);
     		break;
		case 3:
			PPoint = *WPoint;
			ConvertCoord(&PPoint, 1, GOOGLEMAPSPROJECTION);
     		TRANS2 (PPoint.x,PPoint.y,&WinPointD.x,&WinPointD.y,CurView->hTranProjectionToScreen);
			break;
     }
{
#if ENABLETRACE
GSSiExitProg (33);
#endif
     return (WinPointD);
}
#if ENABLETRACE
}
#endif
}

POINT BasePtToFilePt (DPOINT WPoint) //base point in - projected file point out
#if ENABLETRACE
{GSSiEnterProg (34);
#endif
{    DPOINT WinPointD;
     POINT  WinPoint;

     TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFile);  
//     ProjectFilePtD (&WinPointD);
     WinPoint.x = IDNINT (min(INT_MAX,max(INT_MIN,WinPointD.x)));
     WinPoint.y = IDNINT (min(INT_MAX,max(INT_MIN,WinPointD.y)));
{
#if ENABLETRACE
GSSiExitProg (34);
#endif
     return (WinPoint);
}
#if ENABLETRACE
}
#endif
}

POINT FilePtToWinPt (POINT Point) 
#if ENABLETRACE
{GSSiEnterProg (34);
#endif
{    DPOINT WinPointD;
     POINT  WinPoint;

     TRANS2 ((double) Point.x,(double) Point.y,&WinPointD.x,&WinPointD.y,hTranFileToVP);  
     WinPoint.x = IDNINT (min(32767.0e0,max(-32767.0e0,WinPointD.x)));
     WinPoint.y = IDNINT (min(32767.0e0,max(-32767.0e0,WinPointD.y)));
{
#if ENABLETRACE
GSSiExitProg (34);
#endif
     return (WinPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT BasePtToFilePtNPD (DPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (35);
#endif
{    DPOINT WinPointD;

     TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFile);
{
#if ENABLETRACE
GSSiExitProg (35);
#endif
     return (WinPointD);
}
#if ENABLETRACE
}
#endif
}

POINT BasePtToFilePtNP (DPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (36);
#endif
{    DPOINT WinPointD;
     POINT  WinPoint;

     TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFile);  
     WinPoint.x = IDNINT (min(32767.0e0,max(-32767.0e0,WinPointD.x)));
     WinPoint.y = IDNINT (min(32767.0e0,max(-32767.0e0,WinPointD.y)));
{
#if ENABLETRACE
GSSiExitProg (36);
#endif
     return (WinPoint);
}
#if ENABLETRACE
}
#endif
}

POINT BasePtToFilePtReorg (DPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (37);
#endif
{    DPOINT WinPointD;
     POINT  WinPoint;
     
     if (hTranBaseToFileReorg)
     	TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFileReorg); 
     else
		TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFile);  
     WinPoint.x = IDNINT (min(32767.0e0,max(-32767.0e0,WinPointD.x)));
     WinPoint.y = IDNINT (min(32767.0e0,max(-32767.0e0,WinPointD.y)));
{
#if ENABLETRACE
GSSiExitProg (37);
#endif
     return (WinPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT BasePtToFilePtReorgD (DPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (38);
#endif
{    DPOINT WinPointD;

     if (hTranBaseToFileReorg)
     	TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFileReorg);  
     else
		TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFile);  
{
#if ENABLETRACE
GSSiExitProg (38);
#endif
     return (WinPointD);
}
#if ENABLETRACE
}
#endif
}

DPOINT BasePtToFilePtD (DPOINT WPoint)
#if ENABLETRACE
{GSSiEnterProg (39);
#endif
{    DPOINT WinPointD;

     TRANS2 (WPoint.x,WPoint.y,&WinPointD.x,&WinPointD.y,hTranBaseToFile);
{
#if ENABLETRACE
GSSiExitProg (39);
#endif
     return (WinPointD);
}
#if ENABLETRACE
}
#endif
}



DPOINT FilePtToBasePt (POINT Point)//unprojected file point in - base point out
#if ENABLETRACE
{GSSiEnterProg (40);
#endif
{   DPOINT  NewPoint;
    
    ProjectFilePt (&Point);
    TRANS2 ((double) Point.x,(double) Point.y,
            &NewPoint.x,&NewPoint.y,hTranFileToBase);
{
#if ENABLETRACE
GSSiExitProg (40);
#endif
    return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT CopyFilePtToBasePt (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (41);
#endif
{   DPOINT  NewPoint;
    
    ProjectFilePt (&Point);
    TRANS2 ((double) Point.x,(double) Point.y,
            &NewPoint.x,&NewPoint.y,hTranCopyFile);
{
#if ENABLETRACE
GSSiExitProg (41);
#endif
    return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT FilePtToBasePtD (DPOINT Point)//projected file point in - base point out
#if ENABLETRACE
{GSSiEnterProg (42);
#endif
{   DPOINT  NewPoint;
    
    TRANS2 (Point.x, Point.y,
            &NewPoint.x,&NewPoint.y,hTranFileToBase);
{
#if ENABLETRACE
GSSiExitProg (42);
#endif
    return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT FilePtToBasePtNP (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (43);
#endif
{   DPOINT  NewPoint;
    
    TRANS2 ((double) Point.x,(double) Point.y,
            &NewPoint.x,&NewPoint.y,hTranFileToBase);
{
#if ENABLETRACE
GSSiExitProg (43);
#endif
    return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

DPOINT FilePtToBasePtNPD (DPOINT Point)
#if ENABLETRACE
{GSSiEnterProg (44);
#endif
{   DPOINT  NewPoint;
    
    TRANS2 (Point.x, Point.y,
            &NewPoint.x,&NewPoint.y,hTranFileToBase);
{
#if ENABLETRACE
GSSiExitProg (44);
#endif
    return (NewPoint);
}
#if ENABLETRACE
}
#endif
} 

void AdjustBoundsToMainClipRect (LPMNMXCORL pBounds)
{   
	RECT WinRect, ClipRect;
	POINT	FilePt, WinPt;
	
	if (!InPlotView)
		return;
	RectInit (&WinRect); 
	FilePt.x = min (INT_MAX,max(INT_MIN,IDNINT(pBounds->xmn*CurView->FileFactor)));
	FilePt.y = min (INT_MAX,max(INT_MIN,IDNINT(pBounds->ymn*CurView->FileFactor)));
	WinPt = FileCoordToWinCoord(FilePt);
	AddPointToRect (WinPt,&WinRect);
	FilePt.x = min (INT_MAX,max(INT_MIN,IDNINT(pBounds->xmx*CurView->FileFactor)));
	FilePt.y = min (INT_MAX,max(INT_MIN,IDNINT(pBounds->ymx*CurView->FileFactor)));
	WinPt = FileCoordToWinCoord(FilePt);
	AddPointToRect (WinPt,&WinRect);
	
	IntersectRect (&ClipRect,&MainClipRect,&WinRect);
	WinPt.x = ClipRect.left;
	WinPt.y = ClipRect.bottom;
	FilePt = WinCoordToFileCoord (WinPt);
	pBounds->xmn = FilePt.x;
	pBounds->ymn = FilePt.y;
	WinPt.x = ClipRect.right;
	WinPt.y = ClipRect.top;
	FilePt = WinCoordToFileCoord (WinPt);
	pBounds->xmx = FilePt.x;
	pBounds->ymx = FilePt.y;
	return;
}  

BOOL BoundsToMinMax (LPMINMAX pMinMax,LPMNMXCORD pBounds)
{   
	POINT	Point;
	POINTS	PointS;
	DPOINT	DPoint;
	
	MinMaxInit (pMinMax);
	DPoint.x = pBounds->xmn;
	DPoint.y = pBounds->ymn;
	Point = BasePtToFilePt(DPoint);
	AddPointToMinMax (Point,pMinMax);
	DPoint.y = pBounds->ymx;
	Point = BasePtToFilePt(DPoint);
	AddPointToMinMax (Point,pMinMax);
	DPoint.x = pBounds->xmx;
	Point = BasePtToFilePt(DPoint);
	AddPointToMinMax (Point,pMinMax);
	DPoint.y = pBounds->ymn;
	Point = BasePtToFilePt(DPoint);
	AddPointToMinMax (Point,pMinMax);
	return TRUE;
}

BOOL SetFileBounds ()
#if ENABLETRACE
{GSSiEnterProg (45);
#endif
{    double     xscale, yscale, scale, dist1, dist2;
     long   rect_width, rect_height, iscale;
     long   bounds_width, bounds_height;
     long   ifactor;
     DPOINT DPoint, BasePt1, BasePt2, FilePT1, FilePT2;
     POINT  Point1, Point2, WinPT1, WinPT2, Point;
     MNMXCORD   WinBounds;
     long   MAX_BOUNDS=64000;
     RECT   WinRect ;
	 LPVIEWPORT	pSaveVP=CurView;
	 BOOL	rtn = FALSE;
     
//messes up MGV police	 if (CurView->DisplayInParent && CurView->Parent)
//		SetViewport (CurView->Parent);

     if (CurView->hWnd && !IsIconic (CurView->hWnd) && !FileMode)
        GetClientRect (CurView->hWnd,&WinRect);
/*	 else if (InVirtualPrint)
	 {  
		WinRect.left = WinRect.top = 0;
		WinRect.bottom = VirtualPageHeight; 
		WinRect.right = VirtualPageWidth; 
	 }*/
     else
        WinRect = MainRect;
     
     CurView->FileFactor = 1;
     Point1.x=0;
     Point1.y=0;     
     BasePt1 = FilePtToBasePt(Point1);
     Point2.x=10000;
     Point2.y=10000;
     BasePt2 = FilePtToBasePt(Point2); 
     FileDistToBaseDist = ldistp (BasePt1,BasePt2)/idist(Point1,Point2); 
     if (FileDistToBaseDist == 0)
        FileDistToBaseDist=1;
     if (!Display || Pick || FullCurves)  
     	CurveExpansionFactor = 1;
     else
     	CurveExpansionFactor = FileDistToWinDist;
     WinPT1.x = 100;
     WinPT1.y = 100; 
     FilePT1 = WinCoordToFileCoordD(WinPT1);
     WinPT2.x = 200;
     WinPT2.y = 100; 
     FilePT2 = WinCoordToFileCoordD(WinPT2);             
     dist1 = idist(WinPT1,WinPT2);
     dist2 = ldistp(FilePT1,FilePT2);
     if (dist2 == 0)
        dist2 = 1;
     FileDistToWinDist = dist1/dist2; 
     BaseDistToWinDist = FileDistToWinDist / FileDistToBaseDist;   
     if (dist1 == 0)
        dist1 = 1;
     WindowToFileFactor = dist2/dist1; 
     
     PixelsPerHInch=(float)(10*dist1/dist2); 
     if (FileMode)
     	WidthFactor = 1;
     else
     	WidthFactor = dist2/dist1;
//     if (Pick) rturn(TRUE);
	 if (isnan (WidthFactor) || WidthFactor < 0 || WidthFactor > 1000)
		 WidthFactor = 1;

     Point.x = WinRect.left;
     Point.y = WinRect.bottom;
     DPoint = WinPtToBasePt (Point);
     WinBounds.xmn = DPoint.x;
     WinBounds.ymn = DPoint.y;
     Point.x = WinRect.right;
     Point.y = WinRect.top;
     DPoint = WinPtToBasePt (Point);
     WinBounds.xmx = DPoint.x;
     WinBounds.ymx = DPoint.y;
     TRANS2 (WinBounds.xmn,WinBounds.ymn,
             &CurView->FileBounds.xmn,&CurView->FileBounds.ymn,hTranBaseToFile);
     TRANS2 (WinBounds.xmx,WinBounds.ymx,
             &CurView->FileBounds.xmx,&CurView->FileBounds.ymx,hTranBaseToFile);
             
Loop:
	 if (!CurView->FileFactor)
		goto Exit;
	 CurView->Bounds.xmn = IDNINT (CurView->FileBounds.xmn/CurView->FileFactor);
     CurView->Bounds.xmx = IDNINT (CurView->FileBounds.xmx/CurView->FileFactor);
     CurView->Bounds.ymn = IDNINT (CurView->FileBounds.ymn/CurView->FileFactor);
     CurView->Bounds.ymx = IDNINT (CurView->FileBounds.ymx/CurView->FileFactor); 
     rect_width = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left;
     rect_height = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top;
     bounds_width = CurView->Bounds.xmx - CurView->Bounds.xmn;
     bounds_height = CurView->Bounds.ymx - CurView->Bounds.ymn;
     if (!Pick && 
     	 (bounds_width > MAX_BOUNDS || bounds_height > MAX_BOUNDS ||
         CurView->Bounds.xmn < SHRT_MIN || CurView->Bounds.xmx > SHRT_MAX ||
         CurView->Bounds.ymn < SHRT_MIN || CurView->Bounds.ymx > SHRT_MAX ))
     {
        CurView->FileFactor = CurView->FileFactor * 2;
        goto Loop;
     }

     TRANS2 (CurView->WBounds.xmn,CurView->WBounds.ymn,
             &CurView->FileBounds.xmn,&CurView->FileBounds.ymn,hTranBaseToFile);
     TRANS2 (CurView->WBounds.xmx,CurView->WBounds.ymx,
             &CurView->FileBounds.xmx,&CurView->FileBounds.ymx,hTranBaseToFile);
     CurView->Bounds.xmn = IDNINT (CurView->FileBounds.xmn/CurView->FileFactor)-1;
     CurView->Bounds.xmx = IDNINT (CurView->FileBounds.xmx/CurView->FileFactor)+1;
     CurView->Bounds.ymn = IDNINT (CurView->FileBounds.ymn/CurView->FileFactor)-1;
     CurView->Bounds.ymx = IDNINT (CurView->FileBounds.ymx/CurView->FileFactor)+1;
     rect_width = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left;
     rect_height = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top;
     bounds_width = CurView->Bounds.xmx - CurView->Bounds.xmn;
     bounds_height = CurView->Bounds.ymx - CurView->Bounds.ymn;  
     if (rect_width == 0)
        rect_width = 1;
     if (rect_height == 0)
        rect_height = 1;
     xscale = (double) bounds_width / rect_width;
     yscale = (double )bounds_height / rect_height;
     if (xscale > yscale)
        scale = xscale;
     else
        scale = yscale;
     
     if (scale == 0)
        scale = 1;
     ifactor = (long)min (SHRT_MAX,SHRT_MAX / scale);
     iscale = (long)(scale * ifactor);
     if (iscale < 1) iscale = 1;
     if (ifactor == 0)
        ifactor = 1;
     scale = (float) iscale / ifactor;
     
     if (!FileMode)
     	WidthFactor /= CurView->FileFactor;
	 if (WidthFactor < 0 || WidthFactor > 1000)
		 WidthFactor = 1;
     CurView->wOrigX = (short)CurView->Bounds.xmn;
     CurView->wOrigY = (short)CurView->Bounds.ymx;
     CurView->wExtX = (short)iscale;
     CurView->wExtY = (short)-iscale;
     CurView->vExtX = (short)ifactor;
     CurView->vExtY = (short)ifactor;  
     AdjustBoundsToMainClipRect (&CurView->Bounds);
	 rtn = TRUE;
Exit:
	 CurView = pSaveVP;
{
#if ENABLETRACE
GSSiExitProg (45);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

void GetItemLoc (short item,LPLONG Segment,LPWORD Offset)
#if ENABLETRACE
{GSSiEnterProg (46);
#endif
{
    *Segment = PickList[item].Segment;
    *Offset = PickList[item].Offset;
{
#if ENABLETRACE
GSSiExitProg (46);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL OpenPlotFile (void)
#if ENABLETRACE
{GSSiEnterProg (47);
#endif
{   OFSTRUCTGM    OFStruct;

    if (FidMap != HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (47);
#endif
    	return (TRUE);
}
    if (!PltName[0])
{
#if ENABLETRACE
GSSiExitProg (47);
#endif
    	return(FALSE);
}
    FidMap = GSSiOpenFile (PltName,(LPOFSTRUCTGM)&OFStruct,OF_READ); 
    if (FidMap == HFILE_ERROR)
    {
{
#if ENABLETRACE
GSSiExitProg (47);
#endif
        return FALSE;
}
    }
{
#if ENABLETRACE
GSSiExitProg (47);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ClosePlotFile (void)
#if ENABLETRACE
{GSSiEnterProg (48);
#endif
{   if (FidMap == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (48);
#endif
		return (FALSE);
}
    GSSiClose2 (&FidMap);
    FidMap = HFILE_ERROR;
{
#if ENABLETRACE
GSSiExitProg (48);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

void ShowPickedItem (HWND hWnd, int InItem)
#if ENABLETRACE
{GSSiEnterProg (49);
#endif
{   LPSHORT       ipnt=0, EndItem;
    HANDLE      hpltBuf=0;
    LPSTR       LPpltBuf;
    LPITEM      ItemHeader;
    POINT       CenterPoint;
    HANDLE      hVisList=0;
    LPVISLIST   SaveVis=0;
    LPVIEWPORT  SaveView;
    short		iLen, Item, SavePassID=CurView->PassID;     
    MNMXCORD	Rect;
    char		str[64];
	BOOL		saveUseGDIPlus = useGDIPlus;

	useGDIPlus = wantGDIPlus;

    Item = max (InItem,0);                     
    SaveView = CurView;
	SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID);

    if (!GetPickName (Item))
        goto Exit;

    if (!PickName[0])
		goto Exit;
    _fstrcpy (PltName,PickName);
	ClearFullWindowBitmap (0);
    CloseMap (FALSE);
	if (InItem >= 0)
		SelectVisList (FALSE);
    if (!OpenMap (CurView->hWnd,CurView->hDC))
    	goto Exit;

	GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);
    SetDisplayMode (CurView->hDC, GF_MAPMODE);
    if (InItem >= 0)
    {
	    hVisList=GSSiGlobAlloc (  46,GHND,sizeof(VISLIST));
	    SaveVis = CurVis;
	    CurVis = (LPVISLIST)GlobalLock (hVisList); 
	    CurVis->hVisList=hVisList;
	    InitVis (); 
	    if (PickList[Item].Type == 4)
	    {
	    	_fmemset (&CurVis->WantType,0,20);
	    	CurVis->WantType[2] = 1;
	    }
	    else
		    CurVis->WantType[8]=1; 
		if (SaveVis) 
		{
			CurVis->WantType[6] = SaveVis->WantType[6]; 
			CurVis->WantType[7] = SaveVis->WantType[7]; 
		}
	}
    if (!MapType) 
    {
	    GSSillseek (FidMap,PickList[Item].Segment,0);
	    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
	    hpltBuf = GSSiGlobAlloc (  47,GMEM_MOVEABLE,(DWORD)nBytes+2);
	    LPpltBuf = GlobalLock (hpltBuf);
	    LPpltBuf[nRead] = 0;
	    nRead = BigRead (FidMap,LPpltBuf,nBytes);
	    if (nRead != nBytes || PickList[Item].Offset > nRead) 
	    {
	    	InvalidItem (0,TRUE);
	    	goto Exit;
	    }
	    ipnt = (LPSHORT)(LPpltBuf + PickList[Item].Offset);
	    ItemHeader = (LPITEM) ipnt;
	    if (InvalidItem (ItemHeader,TRUE))
	        goto Exit; 
	    iLen = abs(ItemHeader->Len);
	    if (iLen)
	    {
		    EndItem = ipnt + iLen;
		    EndItem+=6;
		    *EndItem = 0;   
		} 
		FileBoundsToWBounds (&ItemHeader->MinMax,&Rect); 
	}
	else if (MapType == MT_SHP)
	{
	    if (SavePassID)
			CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		ReadSHPRecordHeader (FidMap,SHPRecOffset,&Rect);
	}	
	else if (MapType == MT_PERSONAL_GEO_DB)
	{
	    if (SavePassID)
			CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
	    ltoa (PickList[Item].Segment,str,10);
    	SetGlobalValue("%OBJECTID",str);
		if (!FetchDBRec (PGDBHandle))
    	{ 
    		CloseMap(FALSE);
	    	goto Exit;
    	}
		ReadPGDBRecordHeader (&Rect);
	}	
	else if (MapType == MT_FILE_GEO_DB)
	{
	    if (SavePassID)
			CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
	    ltoa (PickList[Item].Segment,str,10);
    	SetGlobalValue("%OBJECTID",str);
		if (!FetchDBRec (FGDBHandle))
    	{ 
    		CloseMap(FALSE);
	    	goto Exit;
    	}
		ReadFGDBRecordHeader (&Rect);
	}	
	else if (MapType == MT_ORA)
	{
	    if (SavePassID)
			CurView->PassID=4;
    	CurrentORARec = PickList[Item].Segment;
    	ORARecOffset = GetORARecordOffset (CurrentORARec,FALSE);
		ReadORARecordHeader (FidMap,ORARecOffset,&Rect);
	}	
	else if (MapType == MT_DGN7)
	{
		if (SavePassID)
			CurView->PassID = 4;
		CurrentDGNRec = PickList[Item].Segment;
		GetDGNRecordBounds(CurrentDGNRec, &Rect);
	}
	else if (MapType == MT_DGN8)
	{
		if (SavePassID)
			CurView->PassID = 4;
		CurrentDGNRec = PickList[Item].Segment;
		GetDGN8RecordBounds(CurrentDGNRec, &Rect);
	}
	else if (MapType == MT_GPX)
	{
		if (SavePassID)
			CurView->PassID = 4;
		CurrentGPXRec = PickList[Item].Segment;
		GetGPXRecordBounds(CurrentDGNRec, &Rect);
	}
	else if (MapType == MT_SQLITE)
	{
		if (SavePassID)
			CurView->PassID = 4;
		CurrentSQLITERec = PickList[Item].Segment;
		GetSQLITERecordBounds(CurrentSQLITERec, &Rect);
	}
	else if (MapType == MT_INDEX)
		Rect = CurView->FileMNMX;
	if (!RectInWBounds (&Rect,1))
    {   if (AutoPan)
            CenterWindow (MinMaxMidPointD (&Rect),FALSE);
    }
    else
    {   
		BOOL	OpenBP;

    	if (InItem >= 0)
    	{
        	Highlight = TRUE;
			if (SavePassID)
				CurView->PassID = 4; 
		}
	    GSSiDeleteObject(&CurView->hRgn);
        CurView->hRgn = CreateVPRgn(FALSE,FALSE);
        SelectClipRgn (CurView->hDC,CurView->hRgn);
        GSSiDeleteObject(&CurView->hRgn);
        SetDisplayMode (CurView->hDC, GF_MAPMODE);
        OpenBP = OpenBasePens();
        PickedPointLoc = PickList[Item].BeginPoint; 
        ProcessSingleItem = TRUE;
		HaveFirstHeader=FALSE; 
		if (MapType == MT_SHP)
		{   
			if (SavePassID)
				CurView->PassID = 4; 
			ProcessSHPRecord (CurView->hDC,FidMap,CurrentSHPRec); 
	    }
		else if (MapType == MT_PERSONAL_GEO_DB)
		{   
			if (SavePassID)
				CurView->PassID = 4; 
			ProcessPGDBRecord (CurView->hDC,CurrentSHPRec,0); 
	    }
		else if (MapType == MT_FILE_GEO_DB)
		{   
			if (SavePassID)
				CurView->PassID = 4; 
			ProcessFGDBRecord (CurView->hDC,CurrentSHPRec); 
	    }
		else if (MapType == MT_ORA)
		{   
			if (SavePassID)
				CurView->PassID = 4; 
			ProcessORARecord (CurView->hDC,FidMap,CurrentORARec); 
	    }
		else if (MapType == MT_GMD)
		{
			if (SavePassID)
				CurView->PassID = 4;
			ProcessGMDRecord(CurView->hDC, (HANDLE)FidMap, CurrentGMDRec);
		}
		else if (MapType == MT_SQLITE)
		{
			if (SavePassID)
				CurView->PassID = 4;
			if (GetSQLITERecord(CurrentSQLITERec))
				ProcessSQLITERecord(CurView->hDC,CurrentSQLITERec);
		}
		else if (MapType == MT_DGN7)
		{
			if (SavePassID)
				CurView->PassID = 4;
			ProcessDGNRecord(CurView->hDC, CurrentDGNRec);
		}
		else if (MapType == MT_DGN8)
		{
			if (SavePassID)
				CurView->PassID = 4;
			ProcessDGN8Record(CurView->hDC, CurrentDGNRec);
		}
		else if (MapType == MT_GPX)
		{   
		    if (SavePassID)
				CurView->PassID = 4; 
			ProcessGPXRecord (CurView->hDC,CurrentGPXRec); 
	    }
		else if (MapType == MT_INDEX)
		{
		}
        else while(ProcessGraphicsRec (CurView->hDC,ipnt,LPpltBuf,nRead))
        {
		    GSSiGlobUlFree (&hpltBuf);
		    
		    GSSillseek (FidMap,ContinuationOffset,0);
		    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
		    hpltBuf = GSSiGlobAlloc (  48,GMEM_MOVEABLE,(DWORD)nBytes+2);
		    LPpltBuf = GlobalLock (hpltBuf);
		    LPpltBuf[nRead] = 0;
		    nRead = BigRead (FidMap,LPpltBuf,nBytes);
		    if (nRead != nBytes) 
		    {
				ProcessSingleItem=HaveFirstHeader=FALSE;
		    	InvalidItem (0,TRUE);
		    	goto Exit;
		    }
		    ipnt = (LPSHORT)LPpltBuf;
        }
		ProcessSingleItem=HaveFirstHeader=FALSE;
        CloseBasePens(OpenBP);
        Highlight = FALSE;
        CloseMap(FALSE);  
        CurView->PassID = SavePassID;
    }
    GSSiGlobUlFree (&hpltBuf);
    hpltBuf=0; 
	DisplaySavedGraphicsFile(CurView->hDC, 7);
Exit:
	useGDIPlus = saveUseGDIPlus;
	SaveFullWindowBitmap (hWndMain); 
    CurView->PassID = SavePassID;
    if (SaveVis)
    {
	    GSSiGlobUlFree (&hVisList);
	    CurVis = SaveVis;
	} 
    SetCurView ( SaveView);
	{
		HDC	hDC = GetDC (CurView->hWnd);
		ReleaseDC (CurView->hWnd,hDC);
	}

{
#if ENABLETRACE
GSSiExitProg (49);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
short CenterWindow (DPOINT CenterPoint, BOOL Imediate)
#if ENABLETRACE
{GSSiEnterProg (50);
#endif
{    double BWidth, BHeight;
     
	 HaltMapDisplay(FALSE,FALSE); 
	 CurView->LastWidth = 0;
	 ZoomToPointAndScale (CenterPoint,CurView->Scale,Imediate);
{
#if ENABLETRACE
GSSiExitProg (50);
#endif
     return (0);
}
#if ENABLETRACE
}
#endif
}

BOOL ItemInRegion (HRGN hRgn,HPPOINTS lpPoints, long npnts)
#if ENABLETRACE
{GSSiEnterProg (52);
#endif
{   RECT    Rect, NewRect;
    HPPOINTS lpPoint;
    DWORD     i;
    
	RectInit (&Rect);

    for (i=0,lpPoint=lpPoints;i<npnts;i++,lpPoint++)
    {
        Rect.left = min (Rect.left,lpPoint->x);
        Rect.right = max (Rect.right,lpPoint->x);
        Rect.top = min (Rect.top,lpPoint->y);
        Rect.bottom = max (Rect.bottom,lpPoint->y);
    }

{
#if ENABLETRACE
GSSiExitProg (52);
#endif
    return (IntersectRect (&NewRect,&Rect,&CurView->Rect));
}
#if ENABLETRACE
}
#endif
}

void DisplayCloseIcon (void)
{ 
	char	str[MAX_PATH];
	int		x,y;
	
	if (CurView && CurView->hDC && !InShowZoomArea && !Printing && !InSmoothZoom && !MemMap)
	{
	    SaveDC (CurView->hDC);
		SetDisplayMode (CurView->hDC, GF_SCREENMODE); 
	    /*SetMapMode    (CurView->hDC, MM_TEXT );
	    SetWindowOrgEx  ( CurView->hDC, 0, 0,0 );
	    SetViewportOrgEx( CurView->hDC, 0, 0,0 );*/    
	    SelectVPClipRgn (0);
		SetBkMode (CurView->hDC,TRANSPARENT);
		if (CurView->CloseIcon && !DisplayIsSavedImage)
		{  
			RECT	Rect=CurView->Rect; 
			HPEN	hSavePen, hBlackPenDW = CreatePen (PS_SOLID,2,0);
					
					     	
			Rect.top += 2;
			Rect.right -= 2;
			Rect.bottom = Rect.top + 18;  
			Rect.left = Rect.right - 18;
			FillRect (CurView->hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
			//	FrameRect (CurView->hDC,&Rect,GetStockObject(BLACK_BRUSH));   
			hSavePen = SelectObject (CurView->hDC,hBlackPenDW);
			MoveToEx (CurView->hDC, Rect.left+5, Rect.bottom-5,0);
			LineTo (CurView->hDC, Rect.right-5,Rect.top+5);
			MoveToEx (CurView->hDC, Rect.left+5, Rect.top+5,0);
			LineTo (CurView->hDC, Rect.right-5,Rect.bottom-5);
			SelectObject (CurView->hDC,hSavePen);
			GSSiDeleteObject(&hBlackPenDW);
			CurView->CloseIconRect = Rect;        
			
			Draw3DBorder(CurView->hDC, &Rect,-UP_3D, FALSE); 
		} 
		if (CurView->MinMaxIcon && !DisplayIsSavedImage)
		{  
			RECT	Rect=CurView->Rect; 
			HPEN	hSavePen, hBlackPenDW = CreatePen (PS_SOLID,2,0);
					
					     	
			Rect.top += 2;
			Rect.right -= 2;
			Rect.bottom = Rect.top + 18;  
			Rect.left = Rect.right - 18;
			FillRect (CurView->hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
				FrameRect (CurView->hDC,&Rect,GetStockObject(BLACK_BRUSH));   
			hSavePen = SelectObject (CurView->hDC,hBlackPenDW);
			MoveToEx (CurView->hDC, Rect.left+5, Rect.bottom-5,0);
			//LineTo (CurView->hDC, Rect.right-5,Rect.top+5);
			MoveToEx (CurView->hDC, Rect.left+5, Rect.top+5,0);
			//LineTo (CurView->hDC, Rect.right-5,Rect.bottom-5);
			SelectObject (CurView->hDC,hSavePen);
			GSSiDeleteObject(&hBlackPenDW);
			CurView->MinMaxIconRect = Rect;        
			
			Draw3DBorder(CurView->hDC, &Rect,-UP_3D, FALSE); 
			InflateRect (&Rect,-3,-3);
			Draw3DBorder(CurView->hDC, &Rect,-UP_3D, FALSE); 
			InflateRect (&Rect,-2,-2);
			if (CurView->DisplayedFullScreen)
				FillRectColor (CurView->hDC,&Rect,RGB(128,128,128));
		} 
		if (CurView->AutoVisIcon && !DisplayIsSavedImage)
		{  
			RECT	Rect=CurView->Rect; 
			HPEN	hSavePen, hBlackPenDW = CreatePen (PS_SOLID,2,0);
			HFONT	hFont;
			int		OldColor,n=1,i=0;		
			LPSTR	pBS; 
			char	AVName[64];
						     	
			_fstrcpy (str,CurView->VisName);
			ExpandText (str);
			if ((pBS = _fstrrchr (str,'\\')))
				pBS++;
			else
				pBS = str;
			_fstrcpy(AVName,pBS);   
			if ((pBS = _fstrrchr (AVName,'.')))
				*pBS = 0;
			Rect.top += 2;
			Rect.left += 2;
			Rect.bottom = Rect.top + 20;  
			Rect.right = Rect.left + 20;
			FillRect (CurView->hDC,&Rect,GetStockObject(WHITE_BRUSH));   
			FrameRect (CurView->hDC,&Rect,GetStockObject(BLACK_BRUSH));  
			InflateRect (&Rect,-2,-2);
//			sprintf (str," Auto Visibility: %s",AVName); 
			_fstrcpy (str," Auto Visibility"); 
	        hFont = SelectObject(CurView->hDC, GetStockObject (SYSTEM_FONT));
	        OldColor = SetTextColor (CurView->hDC,RGB(255,255,255));
	        x = Rect.right + 5;
	        y = Rect.top; 
			while (i <= n)
			{   
				TextOut (CurView->hDC, x+i, y+i, str,_fstrlen(str));  
				TextOut (CurView->hDC, x-i, y+i, str,_fstrlen(str)); 
				TextOut (CurView->hDC, x-i, y-i, str,_fstrlen(str));
				TextOut (CurView->hDC, x+i, y-i, str,_fstrlen(str));  
				i++;
			} 
            SetTextColor (CurView->hDC,0); 
			TextOut(CurView->hDC, x, y, str,_fstrlen(str));  
			SelectObject (CurView->hDC,hFont);    
			FillRect (CurView->hDC,&Rect,GetStockObject(WHITE_BRUSH));
			FrameRect (CurView->hDC,&Rect,GetStockObject(BLACK_BRUSH));  
			if (!CurView->DisableZoomMacro) 
			{
				hSavePen = SelectObject (CurView->hDC,hBlackPenDW);
				MoveToEx (CurView->hDC, Rect.left+2, Rect.bottom-7,0);
				LineTo (CurView->hDC, Rect.left+6,Rect.bottom-3);
				LineTo (CurView->hDC, Rect.right-5,Rect.top+3);
				SelectObject (CurView->hDC,hSavePen);
			}
			Draw3DBorder(CurView->hDC, &Rect,-DOWN_3D, TRUE);
			GSSiDeleteObject(&hBlackPenDW);
			CurView->AutoVisIconRect = Rect;        
			
			SetTextColor (CurView->hDC,OldColor);
		} 
	    RestoreDC (CurView->hDC,-1);
	}
	return;
}
	

void SetNewBoundsToBounds(void)
#if ENABLETRACE
{GSSiEnterProg (53);
#endif
{
    CurView->NewBounds=CurView->WBounds;
#if ENABLETRACE
}
#endif
}

BOOL setDoPaint( BOOL DoPaint)
{
	BOOL rtn = doPaint;
//	static BOOL check = FALSE;
//	static int n = 0;
//	char txt[256];
	doPaint = DoPaint;
/*	if (check && !doPaint)
		ii = 1;
	sprintf(txt, "%s %i %i %i",file,line, rtn,doPaint);
	SetWindowText(hWndMain, txt);
	if (DoPaint != rtn)
		Sleep(5000);*/
	return rtn;
}
BOOL DoPaint(void)
{
	if (!doPaint)
		ii = 1;
	if (InDisplayProcessing)
		ii = 1;
	return doPaint;
}

void PaintMap (HWND hWnd, HDC hDC,BOOL ImediateIn,LPRECT pUpdateRect,int From)
#if ENABLETRACE
{GSSiEnterProg (54);
#endif
{ 
    static  BOOL    First=TRUE;  
    BOOL	Imediate=ImediateIn;
	BOOL	SaveInImediate = InImediate;
    RECT	IntRect, rect;   
	int		i;
    

    NumScreensDisplayed++;
// 3/27/06    if (DisableHalt)    
if (InDisplayProcessing || skipPaint-- > 0)
{
#if ENABLETRACE
GSSiExitProg (54);
#endif
    	return;
}
    if (!DoPaint())
{
#if ENABLETRACE
GSSiExitProg (54);
#endif
    	return;
}   
	GetClientRect (hWndMain,&ClientRect);
	if (IsRectEmpty (&ClientRectStart))
		ClientRectStart = ClientRect;

    if (!PeopleNet && !MemMap) 
    	HaltMapDisplay(FALSE,FALSE); 
    if (!OpenConfig(hWnd,hDC))
{
#if ENABLETRACE
GSSiExitProg (54);
#endif
    	return;  
}  
	useGDIPlus = wantGDIPlus;
	if (MemMap && From != 1)
		SetConfig (1);
	else
		ClearMeterPrompts (hDC);
Top: 
	hDC = ScreenBufferDC (hWnd,hDC);
	if (!CurrentConfig)
		Imediate = TRUE;
	else
		Imediate = ImediateIn;
    InDisplayProcessing = TRUE;
    DisplayCycle++;
    TotCopySize = 0;
	PrevLayerVP=0;
	SaveInImediate = InImediate;
	if (SaveInImediate)
		ii=1;
    InImediate = Imediate;
	if (!NumViewportsArray[0])
	{
		rect=ClientRect;

		if (pUpdateRect)
			rect = *pUpdateRect;
		FillRectPoly (hDC,&rect,WindowColor);
	}
    if (FirstDisplayOfConfig)
    {   
        First = FALSE;
        DisplayFinOpt = 2;
		DisplayAllToolbars(4);
        FirstDisplayOfConfig = FALSE;
    }
    HavePaint = TRUE;  
    if (ConfigLevel || IgnoreWPC)
    	SetMainRect (hWnd,hDC,pUpdateRect,1);
    else
    	SetMainRect (hWnd,hDC,0,1);
	SetDisplayMode (hDC, GF_SCREENMODE); 
/*    SetMapMode    ( hDC, MM_TEXT );
    SetWindowOrgEx  ( hDC, 0,   0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 ); */
	SelectClipRgn (hDC,0);
	if (!pUpdateRect) 
	{
    	if ((!MemMap || From) && NumViewportsArray[0]) 
    	{
    		SetConfig (0);   
    		Imediate = TRUE;
    	   	SetMainRect (hWnd,hDC,0,1);
		}
    	FillRectPoly (hDC,&MainRect,WindowColor);
    }
    else if (!ConfigLevel)
    {   
    	RECT	NewRect;
    	
    	UnionRect(&NewRect, &ConfigDisplayRect, pUpdateRect);
    	if (!EqualRect (&NewRect,&ConfigDisplayRect))
    	{
	    	if (NumViewportsArray[0])
	    	{
	    		SetConfig (0); 
	    		Imediate = TRUE;
	    	   	SetMainRect (hWnd,hDC,0,1);
			}
	    }
        SelectClipRgn (hDC,0);
	    if (WindowColor)
   			FillRectPoly (hDC,pUpdateRect,WindowColor);
	    else
   			FillRectPoly (hDC,pUpdateRect,WindowColor+1); //+1 keeps convert color from making yellow from black
    }
	if (!CurrentConfig)
	{
		SetConfigDisplayRect ();
		rect = ConfigDisplayRect;
	}
	else
		rect = MainRect;
    if (SetupViewports (hWnd,hDC,0,rect,0))
    {
	    NumViewportsToDisplay = *pNumViewports;
	    for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
	    {   
	        SetCurView (pViewportsD[DisplayViewID]);      
	        {
		    	RECT	TestRect = CurView->Rect; 
		    	
		    	TestRect.left /= ScreenWindowFactor;
		    	TestRect.top /= ScreenWindowFactor;
		    	TestRect.right = TestRect.left + (CurView->Rect.right - CurView->Rect.left)/ScreenWindowFactor;
		    	TestRect.bottom = TestRect.top + (CurView->Rect.bottom - CurView->Rect.top)/ScreenWindowFactor;
		        if (!pUpdateRect || IntersectRect (&IntRect,pUpdateRect,&TestRect))
		        {
			        CurView->Display = TRUE; 
			        ResetViewport (TRUE,TRUE); 
			    }
			    else
			        CurView->Display = FALSE; 
			}
			    	
	    }
	    DisplayViewID = 0; 
	    /*if (hCfgImage)
	    {
	    	LPBITMAPINFOHEADER pDibInfo = (LPBITMAPINFOHEADER)GlobalLock (hCfgImage); 
	    	RECT	WindowRect;
	    	
	    	GetClientRect (hWndMain,&WindowRect);  
	    	if (pDibInfo->biWidth != WindowRect.right - WindowRect.left ||
	    		pDibInfo->biHeight != WindowRect.bottom - WindowRect.top)
	    		GSSiGlobUlFree (&hCfgImage);
	    	else
		    	GlobalUnlock (hCfgImage); 
	    }*/

	    if (hCfgImage || hCfgImages)
	    {   
	    	RECT	Rect;
	    	
	    	if (CurrentConfig)
	    	{
			    short	iv;
	    		LPVIEWPORT	pSaveVP;
			    
				SetDisplayMode (hDC, GF_SCREENMODE); 
		        SelectClipRgn (hDC,0);
	   			GetClientRect (hWnd,&Rect);
				if (hCfgImage)
					DisplayBMInRect32(hDC, hCfgImage, MainRect, FALSE);
				else if (NumSavedImages)
				{
					short image, nearimage=0;
					int	  Size, SizeDiff=INT_MAX, Diff;
					int	  SizeMain=RECTWIDTH(&MainRect) * RECTHEIGHT(&MainRect);
					LPSTR	pCfgImage;
					HDIB32	hDib;
					double adjustwidthmin = 1, adjustheightmin = 1;
					for (image = 0;image < NumSavedImages;image++)
					{
						double adjustwidth, adjustheight;
						RECT testrect = AdjustRectToRect(&SavedImageData[image].ClientRect, &MainRect, &adjustwidth, &adjustheight);
						Size = RECTWIDTH(&testrect) * RECTHEIGHT(&testrect);

						Diff = abs (Size - SizeMain);
						if (Diff < SizeDiff)
						{
							SizeDiff = Diff;
							nearimage = image;
							adjustwidthmin = adjustwidth;
							adjustheightmin = adjustheight;
						}
					}
					pCfgImage = GlobalLock (hCfgImages); 
					CfgImageLen = SavedImageData[nearimage].ImageLen;
					pCfgImage += SavedImageData[nearimage].Offset;
					hDib = LoadDIBFromMem (pCfgImage,CfgImageLen, FIF_TIFF,0);
					GSSiGlobUlFree (&hCfgImages);
					hCfgImage = hDib;
					DisplayBMInRect32 (hDC,hCfgImage,MainRect,FALSE);
					//Sleep (2000);
					SetViewport(*pCommandViewport);
					DPOINT mp = MinMaxMidPointD(&SavedImageData[nearimage].Bounds);
					double w = BoundsWidth(&SavedImageData[nearimage].Bounds);
					double h = BoundsHeight(&SavedImageData[nearimage].Bounds);
					CurView->NewBounds.xmn = mp.x - w / 2;
					CurView->NewBounds.xmx = mp.x + w / 2;
					CurView->NewBounds.ymn = mp.y - h / 2;
					CurView->NewBounds.ymx = mp.y + h / 2;

					SetScaleAndMidpointFromBounds (CurView);
			        SetBounds(CurView->hWnd,CurView->hDC);
					GMDestroyDIB32(hCfgImage);
					hCfgImage = 0;
				}

				else
					DisplayBMInRect32 (hDC,hCfgImage,MainRect,FALSE);
	   			if (CurrentConfig)
				{
					if (NumSavedImages > 1)
		    			GSSiGlobFree (&hCfgImages);
					else
					{
						GMDestroyDIB32 (hCfgImage);
						hCfgImage = 0;
					}
				}
		    	pSaveVP =CurView;  
		    	for (iv = 0;iv<*pNumViewports;iv++)
		    	{   
		    		//CurView = pViewports[iv];
					SetViewport(pViewports[iv]->ID);
					if (ConfigVersion > 7)
						SetBounds (hWnd,0);
					else
		    			CreateBaseToVPTran (CurView->DrawRect); 
		    	}
		    	CurView = pSaveVP;
		    	SaveFullWindowBitmap (hWnd);
		    	DisplayIsSavedImage = TRUE;
	   		}  
	    	InDisplayProcessing=FALSE;
			EndDisplayProcessing (TRUE);
   			DisplayIsSavedImage = FALSE;
	    }
	    else
		{
			while (DisplayViewID<NumViewportsToDisplay)
	        	if (DisplayViewport (hWnd,hDC,Imediate))
	        		goto Next;
		}
	}
    InDisplayProcessing = FALSE;  
	if (DisplayPartialBuffer)
		ShowBufferedScreen (TRUE,TRUE,0,0);
	goto Exit;
Next:
	if (DisplayPartialBuffer)
		ShowBufferedScreen (TRUE,TRUE,0,0);
    ImediateProcessing (Imediate,2);
Exit:    
	if (!idTimer)
	{
		if (!CurrentConfig)
		{
			if (DisplayPartialBuffer)
				ShowBufferedScreen (TRUE,TRUE,0,0);
		 	SetConfig(1);  
			SetConfigDisplayRect ();
		 	pUpdateRect =  &ConfigDisplayRect;
	//	   	InvalidateRect (hWndMain,&ConfigDisplayRect,TRUE);
			goto Top;
		}
		else 
		{
		    InDisplayProcessing = FALSE;  
			if (!MemMap)
				SaveFullWindowBitmap (hWndMain); 
		}
	}
	if (hWndBGUpdateMsg)
		PostMessage (hWndBGUpdateMsg,WM_COMMAND,IDOK,0);
	DisplayVPDialogs (TRUE);
	DrawMenuBar (hWnd);
	InImediate = SaveInImediate;
	if (InImediate)
		ii=1;
{
#if ENABLETRACE
GSSiExitProg (54);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

void AddThemeToVP (LPVIEWPORT CurView,LPTHEME pTheme)
{
	int	ii;

	if (CurView->NumThemes >= MAX_VIEWPORT_THEMES)
		MessageBox (0,"Too many themes in viewport",0,MB_ICONEXCLAMATION);
	else
	{
		//keep hotspot themes at end of list
		if (CurView->NumThemes && CurView->pThemes[CurView->NumThemes - 1]->ID == GF_HOTSPOT_THEME)
		{
			CurView->pThemes[CurView->NumThemes] = CurView->pThemes[CurView->NumThemes - 1];
			CurView->pThemes[CurView->NumThemes - 1] = pTheme;
			CurView->NumThemes++;
		}
		else
			CurView->pThemes[CurView->NumThemes++] = pTheme;
		if (pTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME && pTheme->DisplayViewport)
			SetThemeOrder (pViewports[pTheme->DisplayViewport-1]);
		if (pTheme->ID == GF_COMPARE_VIEWPORTS_THEME)
			ii=1;
	}
	return;
}

void RemoveThemeFromVP (LPVIEWPORT CurView,LPTHEME pTheme)
{
	UINT	i,n;

	for (i=0,n=0;i<CurView->NumThemes;i++)
	{
	    if (CurView->pThemes[i]!=pTheme)
	    	CurView->pThemes[n++]=CurView->pThemes[i];
	}	
	CurView->NumThemes=n;
	return;
}

void SetThemeOrder (LPVIEWPORT CurView)
#if ENABLETRACE
{GSSiEnterProg (55);
#endif
{   
	UINT	i; 
	
	CurView->NumThemes=0;
	for (i=0;i<*pNumViewports;i++)                  
	{
		LPTHEME pTheme = pViewports[i]->pTheme;
		if (pTheme && pViewports[i]->ID != CurView->ID)
		{
			if (pTheme->TargetViewport == CurView->ID)
				AddThemeToVP (CurView,pTheme);
			if (pTheme->ID == GF_COMPARE_VIEWPORTS_THEME && pTheme->DataType == CurView->ID) 
				AddThemeToVP (CurView,pTheme);
		}
	}
{
#if ENABLETRACE
GSSiExitProg (55);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}    

COLORREF  Convert0TransToOpaque(COLORREF color)
{
	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);
	int i = GetIValue(color);
	if (!i)
		i = 256;
	color = RGBI(r, g, b, i);
	return color;
}

void FillVPBackground (void)
{   
	 COLORREF Color = Convert0TransToOpaque (CurView->BackGroundColor);
	 
	 if (ComputePCTTheme)
	 {
	 	if (CurView->ID == ComputePCTTheme->TargetViewport)
	 		Color = ComputePCTBKGColor;
	 }
	 Color = ConvertColor (Color,-1);
	 if (CurView->MarginPan && !CurView->Transparent && !Printing)
	    FillRectPoly (CurView->hDC,&CurView->ScreenRect,Color);
	 else if (ClearViewport)
	    FillRectPoly (CurView->hDC,&CurView->Rect,Color); 
	 ClearViewport = TRUE;
	 if (CurView->numBackgroundAreaPoints && CurView->hBackgroundArea)
	 {
			HBRUSH	hBrush, hOldBrush;
			HPEN	hOldPen;
			HPDPOINT	pPoints=GlobalLock (CurView->hBackgroundArea);
			HANDLE	hPolyPartLen=0;
			
			hOldPen = SelectObject(CurView->hDC, GetStockObject (NULL_PEN)); 
			hBrush = CreateSolidBrush(CurView->BackgroundAreaColor);
			hOldBrush = SelectObject (CurView->hDC,hBrush); 
			GWPolygonD (CurView->hDC,pPoints,CurView->numBackgroundAreaPoints,CurView->numBackgroundAreaParts,hPolyPartLen,0,FALSE,TRUE,0);  
			SelectObject (CurView->hDC,hOldBrush);
			SelectObject (CurView->hDC,hOldPen);
			DeleteObject (hBrush);
			GlobalUnlock (CurView->hBackgroundArea);
	 }
	 //if (!stricmp (CurView->Name,"Format"))
	 //	 			ShowBufferedScreen (TRUE,FALSE);//dbug

	 return;
}

void OpenVPDialog ()
{
	if (!CurView->hWndDlg)
		ProcessText (CurView->DlgInitCmd);
	return;
}

void CloseVPDialog ()
{
	if (CurView->hWndDlg)
	{
		DestroyWindow (CurView->hWndDlg);
		CurView->hWndDlg = 0;
	}
	return;
}

BOOL SetHaveOrthos(void)
{
	BOOL rtn = FALSE;

	for (int ifile = 0; ifile < CurView->NumFiles; ifile++)
	{
		if (CurView->FileType[ifile] == 5)
		{
			if (CurVis && CurVis->FileIsVisible[ifile] && CurVis->WantType[5])
			{
				char fileName[1024];
				strcpy(fileName, CurView->lpFiles[ifile]);
				ExpandText(fileName);
				if (FileType(fileName))
					return TRUE;
			}
		}
	}
	return FALSE;
}

void ResetViewport (BOOL WantDisplayPass,BOOL FromPaintMap)
#if ENABLETRACE
{GSSiEnterProg (56);
#endif
{    
     HPEN   NewPen;
     RECT   ShadowRect, SaveRect;
     LPVIEWPORT SaveVP;
     short    itheme, DisplayVP, TargetVP, i; 
	 LPCOORDINATEDISPLAY	CD;       
	 long	Color; 
	 BOOL	RegionIsNull = FALSE;
	 BOOL	forceDataPass;

	 if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (56);
#endif
	 	return; 
}    
	 if (!CurView->Display)
	 {
		 if (CurView->hWndDlg)
			 ShowWindow (CurView->hWndDlg,SW_HIDE);
{
#if ENABLETRACE
GSSiExitProg (56);
#endif
	 	return; 
} 
	 }   
	 *MaskAreaFile = 0;
     GSSiDeleteObject(&CurView->hRgn);
	 CurView->HaveOrthos = SetHaveOrthos ();
	 SetThemeOrder (CurView);
 	 CloseEditRect(CurView); 
 	 RemoveInfoBoxRectForVP(CurView->ID);
	 DisplayProfileLink (0);
	 EnlargeScreen (0,0);   
	 if (CurView->hProfileRoute)
	 	DisplayProfileLoc (0,0,0,FALSE);
     if (CurView->pTheme)
     	CurView->pTheme->VPDisplayed = FALSE;
     if (!CurViewActive()/* || CurView->Width < 0*/)
	 {
		 CloseVPDialog ();
{
#if ENABLETRACE
GSSiExitProg (56);
#endif
     	return;
}
	 }
	 //7/7/2005 DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
     if (CurView->pTheme) 
     {
     	CurView->pTheme->VPDisplayed = TRUE;
     	if (CurView->pTheme->ID == PF_COORD_DISPLAY)
     	{
     		CD=(LPCOORDINATEDISPLAY)CurView->pTheme;
     		if (*CD->AltCVTFile)
     		{
				char file[MAX_PATH];
				sprintf(file, "[%%DL]PROJECTIONS\\%s", CD->AltCVTFile);
                SetGlobalValue("%ALT_PROJECTION",file);
			    ConvertCoordClose ();
				ConvertCoordInit(); 
			} 
     	}
     	else if (CurView->pTheme->ID != GF_CACHE_DISPLAY_THEME)
     	{   
     		LPTHEME SaveTheme=CurTheme;
     		
     		CurTheme = CurView->pTheme;
     		ThemeDisplayLegend2(0,CurView->ID); 
     		CurTheme = SaveTheme;   
     	}
     }
/*     Color = GetGlobalLVal2("[%BACKGROUND_COLOR]",-1);
     if (Color >= 0)
        CurView->BackGroundColor = Color;  3/26/02  */
     CurView->CurFile=-1; 
     CurView->Display=TRUE;  
     GSSiGlobFree (&CurView->hTAGList);
     CurView->hTAGList = GSSiGlobAlloc (  49,GHND,1024);
     _fmemset (CurView->CurVisType,0,sizeof(CurView->CurVisType)); 
     CurView->MaxSymbolWidth = 0;
     /*IncrementFile ();*/
     SetDisplayMode (CurView->hDC, GF_SCREENMODE);
     CurView->WantPass[0]=FALSE;
     CurView->FirstFile = TRUE;        
     SetROP2(CurView->hDC,R2_COPYPEN);
	 forceDataPass = FALSE;
	 for (itheme = 0; itheme < CurView->NumThemes; itheme++)         //pViewports[27]
	 {
		 CurTheme = CurView->pThemes[itheme];
		 if (CurTheme->ID == GF_HOTSPOT_THEME &&
			 CurTheme->IsActive &&
			 CurTheme->VPDisplayed)
			 forceDataPass = TRUE;
	 }
	 for (itheme = 0; itheme<CurView->NumThemes; itheme++)         //pViewports[27]
	 {
		 CurTheme = CurView->pThemes[itheme];
		 if (ThemeNeedsDataPass(FALSE,forceDataPass))
		 {
			ProcessDataPassBeginMacro();
        	CurView->WantPass[0]=TRUE;
			if (CurTheme->SymNum > 0)  
			{
				GSSiGlobFree (&CurTheme->hVisList);
				CurTheme->hVisList = SetThemeVisList (CurTheme->SymNum); 
				if (CurTheme->DisplayViewport ==  19)
				{
					int rtn = GetVisibility (376);
					ii = rtn;
				}
			} 
			else if (CurTheme->SymNum == -2) 
			{
				GSSiGlobFree (&CurTheme->hVisList);
				CurTheme->hVisList = ReadVisList (&CurTheme->Contents[1]);  
			}
		}
     }
     if (CurView->WantPass[0])
     {
		 BOOL doOpen = FALSE;
         CurView->PassID=0; 
         DisplayDataPassMessage (); 
		 //open theme db so testchar can function in data pass
		 for (itheme = 0; itheme < CurView->NumThemes; itheme++)         //pViewports[27]
		 {
			CurTheme = CurView->pThemes[itheme];
			if (CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY ||
				CurTheme->ID == GF_NORTH_ARROW_THEME || CurTheme->ID == GF_CITY_THEME ||
				!CurTheme->IsActive || !CurTheme->VPDisplayed)
			{
				doOpen = FALSE;
			}
			else if (CurTheme->SkipInvalid || CurTheme->MissOpt == 1)
				doOpen = TRUE;
			else for (i = 0; i < CurTheme->NumClass; i++)
				if (CurTheme->ClassStatus[i])
					doOpen = TRUE;
			if (doOpen)
				OpenThemeDataFile(CurTheme->DataFile);
		}
     }
     else
     {
       	ThemeEndDataPass(FALSE);
        CurView->PassID=1;
     }
     CurView->WantPass[1]=WantDisplayPass;
/*       CurView->HaveBounds=FALSE;*/
     
     if (!WantDisplayPass)
{
#if ENABLETRACE
GSSiExitProg (56);
#endif
     	return;
}
     
     if (!FromPaintMap)
     {                               
	     RedrawActiveFunctions(GF_CLEAR);
		 if (CurView->ID == *pCommandViewport) 
		 {
			LPVIEWPORT	SaveView=CurView; 
			short	i;
		       		
			for (i=0;i<*pNumViewports;i++)
			{
				SetCurView ( pViewportsD[i]); 
				if (CurView != SaveView)
			       	RedrawActiveFunctions(GF_CLEAR_CMD);  
			}
			SetCurView ( SaveView);
		 } 
	 }
	 if (CurView->DisplayInParent && CurView->Parent)
	 {
	 	CurView->Rect = pViewports[CurView->Parent-1]->Rect;
	 	CurView->DrawRect = pViewports[CurView->Parent-1]->DrawRect;  
	 	CurView->WBounds = pViewports[CurView->Parent-1]->WBounds;  
	 	CurView->NewBounds = pViewports[CurView->Parent-1]->NewBounds;  
	 	CurView->HaveBounds = pViewports[CurView->Parent-1]->HaveBounds;  
	 	CurView->WindowIsZoomed = pViewports[CurView->Parent-1]->WindowIsZoomed;  
	 	CurView->WindowZoomedToOrtho = pViewports[CurView->Parent-1]->WindowZoomedToOrtho;  
	 	CurView->OrthoRes = pViewports[CurView->Parent-1]->OrthoRes;  
	 	CurView->BaseUnitsPerPixel = pViewports[CurView->Parent-1]->BaseUnitsPerPixel;  
	 	CurView->MetersPerPixel = pViewports[CurView->Parent-1]->MetersPerPixel;  
	 	CurView->MetersPerDegreeX = pViewports[CurView->Parent-1]->MetersPerDegreeX;  
	 	CurView->Scale = pViewports[CurView->Parent-1]->Scale;  
	 	CurView->MidPointW = pViewports[CurView->Parent-1]->MidPointW;  
	 }
     if (CurView->hReport)
     {
        RECT    ReportRect; 
        double  Factor1, Factor2;
        char    str[32];
		LPREPORT	pReport;
        
        ltoa (CurView->ReportRefno,str,10);
        SetGlobalValue ("%INT_REFNO",str);
        SetGlobalValue (CurView->Prefix,CurView->UDI);
        SetGlobalValue ("%PREFIX",CurView->Prefix); 
        SetUDIValue (CurView->Prefix,CurView->UDI); 
        CurrentUDILen = _fstrlen(CurView->UDI);
//        ReportRect = SizeReport (CurView->hDC,CurView->hReport,CurView->DrawRect,CurView->FitToWindow);
		if (!DisplayReport(CurView->hDC, CurView->hReport, CurView->DrawRect, &CurView->DrawRect, 1.0, CurView->ReportRefno, &ReportRect, CurView->FitToWindow))
		{
			UnloadReport(&CurView->hReport);
		}
        if (!CurView->FitToWindow)
        {
            CurView->ReportFactor = DeviceToScreenFactor();
           // CurView->DrawRect = ReportRect; 
		   // CurView->DrawRect.bottom *= DeviceToScreenFactor();
			//CurView->DrawRect.top *= DeviceToScreenFactor();
			//CurView->DrawRect.left *= DeviceToScreenFactor();
			//CurView->DrawRect.right *= DeviceToScreenFactor();
        }
        else
        { 
            Factor1 = (double)(CurView->DrawRect.right - CurView->DrawRect.left+1)/
                      ((double)(ReportRect.right - ReportRect.left+1)*0.7);
            Factor2 = (double)(CurView->DrawRect.bottom - CurView->DrawRect.top+1)/
                      (double)(ReportRect.bottom - ReportRect.top+1);
            CurView->ReportFactor = min (Factor1,Factor2);
        }
		if (CurView->hReport)
		{
			pReport = GlobalLock (CurView->hReport);
			pReport->Just *= CurView->ReportFactor;
			GlobalUnlock (CurView->hReport);
		}
     }
	 if (CurView->DisplayInParent && CurView->Parent)   
{
#if ENABLETRACE
GSSiExitProg (56);
#endif
	 	return;
}
     SaveRect = CurView->DrawRect;
     CurView->DrawRect = CurView->Rect;
	 if (!FileMode && CurView->hDC)
     {
	     GSSiDeleteObject(&CurView->hRgn);
//		 SetDisplayMode (CurView->hDC, GF_TEXTMODE);
         CurView->hRgn = CreateVPRgn(FALSE,TRUE);
         if (SelectClipRgn (CurView->hDC,CurView->hRgn) == NULLREGION && !PrintingToMF)
         	RegionIsNull = TRUE;
//		 SetDisplayMode (CurView->hDC, GF_SCREENMODE);
         GSSiDeleteObject(&CurView->hRgn);
     }
     CurView->DrawRect = SaveRect;
     if (!RegionIsNull && CurView->MarginPan && !CurView->Transparent && !Printing)
     {
         FillRectPoly (CurView->hDC,&CurView->Rect,RGB(98,98,98));
     }
     if (!RegionIsNull && CurView->BackGroundColor!=ULONG_MAX && !CurView->Transparent) 
     	FillVPBackground (); 
	 DestroyVehicles (CurView->ID);
     DisplayCurrentHotspots ();
     if (!RegionIsNull && CurView->BorderPct >= 0)
     {
         NewPen=CreatePen(PS_SOLID,(short) IDNINT (max (DeviceToScreenFactor()/2,(double)((MaxDimension * 2 * CurView->BorderPct)/100))),CurView->BorderColor);
/*       OldPen = SelectObject (CurView->hDC,NewPen);
         OldBrush = SelectObject (CurView->hDC,GetStockObject(0_BRUSH)); */
         DrawRectPoly (CurView->hDC,&CurView->Rect,NewPen); 
/*       Rectangle (CurView->hDC,CurView->Rect.left,CurView->Rect.top,
                    CurView->Rect.right,CurView->Rect.bottom); 
         SelectObject (CurView->hDC,OldPen);
         SelectObject (CurView->hDC,OldBrush); */
         DeleteObject (NewPen);
     }
     if (CurView->pTheme && CurView->pTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
     	ThemeDisplayLegend(1,CurView->ID);
/*	 if (CurView->hWndDlg)
	 {
		 ShowWindow (CurView->hWndDlg,SW_SHOW);
		 SendMessage(CurView->hWndDlg, GSSI_REPOSITION,0, CurView); 
	 }*/
     DisplayCloseIcon ();
     if (CurView->BoundsDisplayID &&
     	 CurView->BoundsDisplayID <= *pNumViewports)
     {   
         DisplayVP = CurView->BoundsDisplayID;
         TargetVP  = CurView->ID;
         CurView->BoundsDisplayed = FALSE;
         BoundsDisplayTheme (TRUE);
         SaveVP = CurView;
         SetCurView (pViewports[DisplayVP-1]);
         CurView->BoundsDisplayVP = TargetVP;
         SetCurView ( SaveVP);
     } 
     ProcessVPPixelThemes (); 
	 ResetProfileVP(CurView);
	 OpenVPDialog ();
	 if (CurView->hWndDlg)
	 {
		 SendMessage(CurView->hWndDlg, GSSI_REPOSITION,0, (LPARAM)CurView); 
		 InvalidateRect(CurView->hWndDlg,0,TRUE);
	 }
{
#if ENABLETRACE
GSSiExitProg (56);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayViewport (HWND hWnd, HDC hDC, BOOL Immediate)
#if ENABLETRACE
{GSSiEnterProg (57);
#endif
{ 
BOOL    WT; 
static	short   FirstDisplayPass=1;

if (!Immediate)
	InImediate = FALSE;

	if (NumViewportsToDisplay < 0)
		goto NextView;
Start:
    while (DisplayViewID < NumViewportsToDisplay)
    {   SetCurView ( pViewportsD[DisplayViewID]);
        if (CurViewActive() && CurView->Display &&
            !( (CurView->Type == 7 || (CurView->pTheme && CurView->pTheme->ID == GF_BOUNDS_DISPLAY_THEME))
                && !Immediate && !(CurView->ID == *pCommandViewport)))
            goto NextView;
       	DisplayViewID++; 
/*        if (Printing)
{
#if ENABLETRACE
GSSiExitProg (57);
#endif
        	return FALSE;
}*/
    }  
    Counter = 100;
	PltName[0]='\0';
{
#if ENABLETRACE
GSSiExitProg (57);
#endif
    return (FALSE);
} 
    
NextView:
    if (CurView->hReport)
    {    
        char    str[32];
        
        ltoa (CurView->ReportRefno,str,10);
        SetGlobalValue ("%INT_REFNO",str);
        SetGlobalValue (CurView->Prefix,CurView->UDI);
        SetGlobalValue ("%PREFIX",CurView->Prefix); 
        SetUDIValue (CurView->Prefix,CurView->UDI); 
        CurrentUDILen = _fstrlen(CurView->UDI);
        DisplayReport (hDC,CurView->hReport,CurView->DrawRect,&CurView->DrawRect,
                       CurView->ReportFactor,CurView->ReportRefno,0,CurView->FitToWindow);
        DisplayViewID++; 
        goto Start;
    }
    if (CurView->Type == 7 && !Printing && !(CurView->ID == *pCommandViewport))
    {
        if (CurView->Bitmap && EqualRect(&CurView->Rect,&CurView->BitmapRect))
        {
            if(Display && !Pick)
            	DisplayTAGs(hDC);
            DisplayViewID++; 
            goto Start;
        }
        else
        	DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
        Counter = MaxTimePerSeg;
    }
/*    else
        Counter = 100;*/ // 8/28/04 to allow interupt while displaying large orthos
    SelectVisList (FALSE); 
    FirstDisplayPass=1;
    if (CurVis) 
    {
        if (!CurVis->WantType[5] || ComputePCTTheme)
        {
            if (CurView->PassID == 1) 
            {
				SetTransparency (0);
				CurView->PassID++;
                CurView->CurFile=-1;
			}
            FirstDisplayPass=2;   
        }
        if (!CurVis->WantType[0])
        {
            if (CurView->PassID == 2)
            {
				SetTransparency (0);
				CurView->PassID++;
                CurView->CurFile=-1;
			}
            if (FirstDisplayPass==2)
            	FirstDisplayPass=3;   
        }
    }
     
NextFile: 
    if (CurView->PassID >= 5 || CurView->PassID <= 3)
    {
        if (CurView->PassID < 5 && CurView->FirstFile && CurView->PassID==FirstDisplayPass)
        {
            if (CurView->HaveBounds)
            {
            	HANDLE hSaveDisplay=GSSiGlobAlloc (  50,GMEM_MOVEABLE,2*MAX_VIEWPORTS);
            	short	SaveDisplayViewID = DisplayViewID;
            	short	SaveNumViewportsToDisplay = NumViewportsToDisplay; 
            	short	SaveFDP = FirstDisplayPass; 
            	short	i,nvp=min (*pNumViewports,NumViewportsToDisplay);
            	LPSHORT	pSaveDisplay=(LPSHORT)GlobalLock (hSaveDisplay);
            	
            	for (i=0;i<nvp;i++)
            	{
            		GSSiDeleteObject (&pViewportsD[i]->hRgn);
            		*pSaveDisplay++ = pViewportsD[i]->Display; 
            	}
            	GlobalUnlock (hSaveDisplay);
            	SetBounds(CurView->hWnd,(HDC)1);  
            	pSaveDisplay = (LPSHORT)GlobalLock (hSaveDisplay);   
            	NumViewportsToDisplay = SaveNumViewportsToDisplay;
            	DisplayViewID = SaveDisplayViewID;      
            	FirstDisplayPass = SaveFDP;
            	for (i=0;i<nvp;i++)
            		pViewportsD[i]->Display = *pSaveDisplay++;
            	GSSiGlobUlFree (&hSaveDisplay); 
            	SelectVisList (FALSE);
            }
		}
    }
    if (CurView->PassID >= 5 || CurView->PassID <= 3)
    {   
    	int	RegionType;
    	
        if (Display)
        {
			char	HaveMaskAreaFile = 0;

	        if (!CurView->hRgn)
			{
				HaveMaskAreaFile = *MaskAreaFile;
		        CurView->hRgn = CreateVPRgn (FALSE,FALSE);
			}
	        RegionType = SelectVPClipRgn (CurView->hRgn);
			GSSiDeleteObject(&CurView->hRgn);
	        if (!Immediate && (!PrintingToMF && RegionType == NULLREGION) && !HaveMaskAreaFile)
			{
	         	goto NoFile; 
			}
	    }
	}
    if (CurView->FirstFile && CurView->PassID==FirstDisplayPass)
    {   
    	USHORT	i;
    	
		if (!Printing)
			CurView->WantDisplayHighlight = FALSE;
		DisplayTAGs2 (CurView->hDC,1,0);    
	    _fmemset (CurView->MaxFileDisplayedPointWidth,0,sizeof(CurView->MaxFileDisplayedPointWidth)); 
/*		{
			char	str[256];

			sprintf (str,"%ld %ld",CurView->ID,CurView->MaxFileDisplayedPointWidth[0]);
			MessageBox (0,str,0,MB_OK);
		}*/
	    MinPickItemWidth = 0;
		if (CurView->ID == 1)
			totContourPoints=trimmedContourPoints=0;
        ThemeBeginDisplayPass(FALSE,CurView->ID); 
        if (CurView->PassID == 99)  
        {   
 			SetTransparency (0);
       		DisplayDistanceLine (TRUE);
	        DisplayPolyOff ();
	        DisplayMaskArea(); 
	        DisplayEditLimits(); 
			DisplayTAGs (CurView->hDC);
        	goto NextVP;
        } 
    }
    if (CurView->PassID >= 5 || CurView->PassID <= 3)
    {
        if (!GetNextViewportFile (FALSE))
        	goto NoFile;
        CurView->FirstFile = FALSE;
        if (DisplayPlotInit(hWnd,Immediate))
{
#if ENABLETRACE
GSSiExitProg (57);
#endif
        	return (TRUE);
}
		else 
			goto NextFile;
    }
NoFile:
    PltName[0]='\0';
	DisplaySavedGraphicsFile (CurView->hDC,CurView->PassID+1);
    if (CurView->PassID>=3)
    {   
		SetTransparency (0);
        CurView->PassID=4; 
/*        if (ComputePCTTheme && Display && !Pick)
        {
	        DisplayPolyOff ();
	        DisplayMaskArea(); 
        }*/
        if (ThemeEndDisplayPass(FALSE,FALSE,FALSE))
        {
			ResetViewport (TRUE,FALSE); 
			goto NextView;
        }
        DisplayVPThemeLegends ();
        DisplayVPThemeDelayedAndInfobox ();
        if (Display && !Pick)
        {  
        	short	SaveVPID=CurView->ID;
        	
        	DisplayDistanceLine (TRUE);
	        DisplayPolyOff ();
	        DisplayMaskArea();
        	if (CurView->DisplayInParent && CurView->Parent > 0)
        		SetViewport (CurView->Parent); 
	        DisplayPointInAreaThemes (SaveVPID); 
	        SetViewport (SaveVPID);
	        DisplayEditLimits(); 
//	        HalfToneViewport ();
	    }
        ThemeDisplayLegend(3,CurView->ID); 
/*        if (CurView->Type == 7 && !(CurView->ID == *pCommandViewport))
        {    
			DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
            if (!Printing)
            {
                CurView->Bitmap = SaveScreen2 (CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
                CurView->BitmapRect = CurView->Rect;
            }
        }*/
		DisplaySavedGraphicsFile (CurView->hDC,7);
		DisplayCurStreets (FALSE,0);
		if (CurView->WantDisplayHighlight)
		{
			int StartHLTRef = LONG_MIN;
    		DisplayHLTData (&StartHLTRef);
		}
		DisplayCloseIcon ();
		//if (Display && !Pick)
		//	DisplayTAGs(hDC);
		if (!Printing)
        {  
			CurView->CurDataRectID = -1;
	       	RedrawActiveFunctions(GF_REDRAW);  
	       	if (CurView->ID == *pCommandViewport) 
	       	{
	       		LPVIEWPORT	SaveView=CurView; 
	       		short	i;
	       		
	       		for (i=0;i<*pNumViewports;i++)
	       		{
					SetCurView ( pViewportsD[i]); 
					if (CurView != SaveView)
				       	RedrawActiveFunctions(GF_REDRAW_CMD);  
	       		}
	       		SetCurView ( SaveView);
	       	} 
	    }
NextVP: 
		if (!CurView->ID)
{
#if ENABLETRACE
GSSiExitProg (57);
#endif
        	return TRUE;
}
        DisplayViewID++;   
/*        if (Printing)
{
#if ENABLETRACE
GSSiExitProg (57);
#endif
        	return FALSE;
} */
        while (DisplayViewID < NumViewportsToDisplay)
        {   SetCurView ( pViewportsD[DisplayViewID]);
            if (CurViewActive() && CurView->Display &&
                !((CurView->Type == 7 || (CurView->pTheme && CurView->pTheme->ID == GF_BOUNDS_DISPLAY_THEME))
                 && !Immediate && !(CurView->ID == *pCommandViewport) ))
                break;
            DisplayViewID++; 
        } 
//        if (Immediate)
        	goto Start;
    } 
    else
    {   
  		SetTransparency (0);
		ShowBufferedScreen(TRUE, FALSE,0, 0);
		CurView->PassID++;
        CurView->CurFile=-1; 
        CurView->FirstFile = TRUE;
//        IncrementFile (); 
        if (CurView->PassID == 1)
        	ThemeEndDataPass(FALSE);
        if (!CurVis || ComputePCTTheme)
            WT = FALSE;
        else
            WT = CurVis->WantType[5];//Orthos
        if (!WT && CurView->PassID == 1)
        {
 			SetTransparency (0);
            CurView->PassID++; 
			ShowBufferedScreen(TRUE, FALSE, CurView->ID, 0);
			CurView->CurFile = -1;
		}
        if (!CurVis)
            WT = FALSE;
        else
            WT = CurVis->WantType[0];  
        if (!WT && CurView->PassID == 2) 
        {
			SetTransparency (0);
            CurView->PassID++;   
			ShowBufferedScreen(TRUE, FALSE, CurView->ID, 0);
			CurView->CurFile = -1;
		}
		if (CurView->PassID > 2 && ComputePCTTheme)
			goto NoFile;
        goto NextFile;
    }
     
#if ENABLETRACE
}
#endif
}

BOOL LoadMapDir (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (58);
#endif
{
    short nRc;
       
	setDoPaint(  FALSE);
     {
#if WIN32
      nRc = DialogBox(hInst, (LPSTR)"LOAD_MD", hWnd,(DLGPROC) LOADMDMsgProc);
#else
      DLGPROC lpfnLOADMDMsgProc;
      lpfnLOADMDMsgProc = MakeProcInstance((DLGPROC)LOADMDMsgProc, hInst);
      nRc = DialogBox(hInst, (LPSTR)"LOAD_MD", hWnd, lpfnLOADMDMsgProc);
      FreeProcInstance(lpfnLOADMDMsgProc);
#endif
     }

{
#if ENABLETRACE
GSSiExitProg (58);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 
double CvtDist (double Dist,short FromUnits,short ToUnits) 
#if ENABLETRACE
{GSSiEnterProg (59);
#endif
{
	double NewDist=Dist;
	
	if (FromUnits == 1 && ToUnits == 2)
		NewDist *= FTM;
	else if (FromUnits == 2 && ToUnits == 1)
		NewDist *= MFT;
	
{
#if ENABLETRACE
GSSiExitProg (59);
#endif
	return NewDist;
}
#if ENABLETRACE
}
#endif
}
BOOL GetMapIndexBounds (LPSTR Name,LPMNMXCORD FileBounds)
#if ENABLETRACE
{GSSiEnterProg (60);
#endif
{
	OFSTRUCTGM    OFStruct;
    short     Version;
    HFILE   FidIndex=0;
    long    Signature, EndOffset;
    
Start:
	FileBounds->xmn = 0;
	FileBounds->xmx = -1;
	FileBounds->ymn = 0;
	FileBounds->ymx = -1;
    FidIndex = GSSiOpenFile (Name,(LPOFSTRUCTGM) &OFStruct,OF_READ);
    if (FidIndex == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (60);
#endif
        return(TRUE);                                                
}
    EndOffset = GSSillseek(FidIndex,(LONG)-(6),2);
    BigRead (FidIndex,(HPSTR)&Signature,4);
    BigRead (FidIndex,(HPSTR)&Version,2);
    if (Signature != 80251)
    {    
        GSSiClose2 (&FidIndex); 
        GSSiMessageBox (0,"This is not a valid index file",OFStruct.szPathName, MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (60);
#endif
        return(FALSE);
}
    }
    if (Version > 3)
    {
    BadMap:  
        GSSiClose2 (&FidIndex);
        GSSiMessageBox (0,"This index file version is not recognized",Name, MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (60);
#endif
        return(FALSE);
}
    }
    if (Version < 1) goto BadMap; 
    if (Version == 1)
    {
        GSSiClose2 (&FidIndex);
        ConvertIndexV1ToV2(Name); 
        goto Start;
    }
        
    GSSillseek(FidIndex,0,0);
    BigRead (FidIndex,(HPSTR)FileBounds,sizeof(MNMXCORD));
    GSSiClose2 (&FidIndex); 
{
#if ENABLETRACE
GSSiExitProg (60);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetPickName (int Item)
#if ENABLETRACE
{GSSiEnterProg (65);
#endif
{    
    HFILE       FidSub; 
    LPSTR lpSlash, lpParen;
    LPFILEINDEX lpIndex;  
    LPVIEWPORT  SaveView;  
	OFSTRUCTGM    OFStruct;
    HANDLE	hBinFileList;
    BOOL    SaveIgnoreBounds;
    short     ifile, SaveCurFile=CurView->CurFile;
	char    SubFileName[MAX_PATH], OrigFile[MAX_PATH] = { 0 }, CurEntryName[MAX_PATH];
    short       NumFiles;
    LPSHORT      lpNumFiles;       
    LPFILELISTENTRY lpEntry;
	LPSTR	pTAB;
	short	SavePass;
	
    if (*HLTGraphicsFile && !PickList[Item].IsDispersed)
	{
		strcpy (PickName,HLTGraphicsFile);
{
#if ENABLETRACE
GSSiExitProg (65);
#endif
    	return TRUE;
}
	}
	IndexEntryStartRef=LONG_MAX;
    if (Item < 0 || Item > MAXPICKITEMS-1)
{
#if ENABLETRACE
GSSiExitProg (65);
#endif
    	return FALSE;
}
    if (PickList[Item].FileNum == -1) //used in mergetag process in shapefile load
    {
    	_fstrcpy (PickName,PltName);
{
#if ENABLETRACE
GSSiExitProg (65);
#endif
    	return TRUE;
}
    } 
    *PickDirectory = 0;
    SaveView = CurView;     
    SaveIgnoreBounds = IgnoreBounds;
    IgnoreBounds=TRUE;    
	SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID);
   	SaveCurFile = CurView->CurFile;
   	SavePass = CurView->PassID;
	CurView->PassID=4;
    if (PickList[Item].FileNum <0 || PickList[Item].FileNum >= CurView->NumFiles)
		goto RtnFalse;
    CurView->CurFile = PickList[Item].FileNum; 
    FileNum = CurView->CurFile; 
    if (CurView->SubFile)
    {
        _fstrcpy (CurView->lpFiles[CurView->RestoreFile],CurView->OrigFile); 
		CurView->SubFile = 0;
		SetRestoreFile(CurView, 0);
    } 
    SubFile = PickList[Item].SubFile;
    PltType = CurView->FileType[CurView->CurFile]; 
    if (CurView->FileType[CurView->CurFile]<4)
    {
        _fstrcpy (PickName,CurView->lpFiles[CurView->CurFile]);
    }
    else if (CurView->FileType[CurView->CurFile]==9) 
    	goto RtnFalse;
    else if (CurView->FileType[CurView->CurFile]==8) 
    {   
    	LPVIEWPORT	SaveVP=CurView;
    	BOOL	Err;
    	
    	CurView = SetVPFromName (CurView->lpFiles[CurView->CurFile],&Err);
		_fstrcpy (PickName,CurView->lpFiles[0]); 
    	PltType = CurView->FileType[0]; 
		SubVPMidPointWin = RectMid (&CurView->DrawRect); 
		CurView = SaveVP; 
		SubVPMidPointWorld = WinPtToBasePt (SubVPMidPointWin);
    	if (Err)
			goto RtnFalse;
    }
    else
    { 
        if (CurView->hlpIndex[CurView->CurFile]) 
        {
            CloseMapIndex (CurView->lpFiles[CurView->CurFile],
                           CurView->hlpIndex[CurView->CurFile],FALSE,TRUE); 
            CurView->hlpIndex[CurView->CurFile] = 0;
        }               
        if (PickList[Item].SubFile)
        {   
            _fstrcpy (OrigFile,CurView->lpFiles[CurView->CurFile]);
        	if (UseBinFileList) 
        	{
            	char        File[128], drive[8], dir[128];
            
	            _fstrcpy (File,OrigFile);
	            ExpandText (File);
	            _splitpath (File,drive,dir,0,0);
	            sprintf (File,"%s%sfilelist.bin",drive,dir);
            	hBinFileList = LoadBinaryFileList (OrigFile,File);
		        lpEntry = (LPFILELISTENTRY)GlobalLock (hBinFileList);   
		        lpNumFiles = (LPSHORT) lpEntry;
		        NumFiles = *lpNumFiles;   
		        if ((PickList[Item].SubFile-1) > NumFiles || !NumFiles)
		        {   
		            GSSiGlobUlFree (&hBinFileList);
		            goto RtnFalse;
		        }
		        lpEntry += (PickList[Item].SubFile);
		        _fstrcpy(CurView->lpFiles[CurView->CurFile],lpEntry->Name); 
				IndexEntryStartRef = lpEntry->StartRefno;
		        GSSiGlobUlFree (&hBinFileList); 
		    }
        	else
        	{
				LPSTR pPar, pDot;

	            FidSub = GSSiOpenFile(CurView->lpFiles[CurView->CurFile],&OFStruct,OF_READ);
	            if (FidSub == HFILE_ERROR)
	                goto RtnFalse;
	            ifile = 1;
	            while (ifile <= PickList[Item].SubFile)
	            {
	                if (!fgetstring(SubFileName,254,FidSub))
	                {
	                    GSSiClose2 (&FidSub);
	                    goto RtnFalse;
	                }
	                ifile++;
	            }
	            GSSiClose2 (&FidSub);
				if ((pTAB = strrchr (SubFileName,'\t')))
				{
					*pTAB++ = 0;
					IndexEntryStartRef = atol (pTAB);
				}
		    	Truncate (SubFileName);
				Strip (SubFileName,'\t'); //remove any extra tabs
				pPar = strrchr (SubFileName,'(');
				pDot = strchr (SubFileName,'.');
				if (pDot && (pDot < pPar && (strnicmp (pDot,".MDB",4) && strnicmp (pDot,".GDB",4))))
					*pPar = 0;

	            _fstrcpy (CurView->lpFiles[CurView->CurFile],SubFileName);
	        }
        }
        
		switch (MapFileType(CurView->lpFiles[CurView->CurFile], 0, 0))
    	{
    		case MT_INDEX:
		        if (!(CurView->hlpIndex[CurView->CurFile]=
		            OpenMapIndex (CurView->lpFiles[CurView->CurFile],0)))
		        	goto RtnFalse;
		        else
		        	_fstrcpy (PickDirectory,CurView->lpFiles[CurView->CurFile]);
		        break;
	        
	        case MT_PLT:
	        case MT_ORA:
			case MT_GMD:
			case MT_DGN7:
			case MT_DGN8:
			case MT_PERSONAL_GEO_DB:
	        case MT_FILE_GEO_DB:
	        case MT_SHP:
			case MT_GPX:
		        _fstrcpy (PickName,CurView->lpFiles[CurView->CurFile]);
		        PltType = 2;  
		        goto Exit;
		    
		    default:
		    	goto RtnFalse;
		}
        lpIndex = (LPFILEINDEX)GlobalLock(CurView->hlpIndex[CurView->CurFile]);
            
Next:
		FileInIndex = PickList[Item].FileInIndex;
        GetNextIndexEntry(lpIndex);
        if (lpIndex->FileInIndex < PickList[Item].FileInIndex)
        {
	        if (lpIndex->FileInIndex >=(long)lpIndex->NumFiles)
	        {   
	            if (!(lpIndex=GetNextIndexHeader(&CurView->hlpIndex[CurView->CurFile],TRUE)))
	                goto RtnFalse;
	        }
            goto Next;  
        }
        
        _fstrcpy (PickName,lpIndex->CurrentEntry->Name);
	    _fstrcpy (CurEntryName,lpIndex->CurrentEntry->Name);

/*	    if (*PickName == '\\')
	    	lpSlash = 0;
	    else
	    	lpSlash = _fstrrchr(PickName,'\\');
	    if (!lpSlash) */
	    {
	        _fstrcpy (PickName,CurView->lpFiles[CurView->CurFile]);
	        ExpandText (PickName);
			if (CurView->FileType[CurView->CurFile] != 5)
			{
				lpSlash = _fstrrchr(PickName,'\\');
				if (lpSlash)
				{
					++lpSlash;
					*lpSlash = '\0';
				} 
				if (*CurEntryName == '\\')  
	        		_fstrcat (PickName,&CurEntryName[1]);
				else
					_fstrcat (PickName,CurEntryName);
			}		 
	    } 
		//ConvertNameToCDName (PickName);

        lpSlash = _fstrrchr(PickName,'\\');
        if (!lpSlash)
        {
            _fstrcpy (PickName,CurView->lpFiles[CurView->CurFile]);
            lpSlash = _fstrrchr(PickName,'\\');
            if (lpSlash)
            {
                ++lpSlash;
                *lpSlash = '\0';
            }
            _fstrcat (PickName,lpIndex->CurrentEntry->Name); 
        }
		{
			LPSTR pDot = strrchr (PickName,'.');

			if (!pDot)
				pDot = PickName;
			lpParen=_fstrrchr (pDot,'(');
			if (lpParen)
				*lpParen++ = 0;
		}
        GlobalUnlock(CurView->hlpIndex[CurView->CurFile]);
        CloseMapIndex (CurView->lpFiles[CurView->CurFile],
                       CurView->hlpIndex[CurView->CurFile],FALSE,FALSE);
        CurView->hlpIndex[CurView->CurFile] = 0;

    } 
Exit:
    if (OrigFile[0])
        _fstrcpy (CurView->lpFiles[CurView->CurFile],OrigFile);
    CurView->CurFile = SaveCurFile;
    CurView->PassID = SavePass;
    SetCurView ( SaveView); 
    IgnoreBounds = SaveIgnoreBounds; 
	MapType = MapFileType(PickName, 0, 0);
{
#if ENABLETRACE
GSSiExitProg (65);
#endif
    return (TRUE);     
}
RtnFalse:    
    IgnoreBounds = SaveIgnoreBounds;
    if (OrigFile[0])
        _fstrcpy (CurView->lpFiles[CurView->CurFile],OrigFile);
    CurView->CurFile = SaveCurFile;  
    CurView->PassID = SavePass;
    SetCurView ( SaveView);
    PickName[0]=0;
	MapType = 0;
{
#if ENABLETRACE
GSSiExitProg (65);
#endif
    return (FALSE);
}
#if ENABLETRACE
}
#endif
}

void GetPCTPLOTType (void)
{   
	short	itype;
	USHORT	i;
	
	for (i=0;i<CurView->NumFiles;i++)
	{
		if (!_fstricmp (CurView->lpFiles[i],"[%PLOT]"))
		{
			switch (GetFileTypeFromName(CurView->lpFiles[i],FALSE))
			{
				default:
					break;
				case 1:
					CurView->FileType[i] = 2;
					break;
				case 2:
					CurView->FileType[i] = 4;
					break;
				case 3:
					CurView->FileType[i] = 5;   
					break;
				case 4:
					CurView->FileType[i] = 3; 
					break;
				case 8:
					CurView->FileType[i] = 9;
					break; 
			}
		}
	}
	return;
}

BOOL GetNextViewportFile (BOOL VisScan)
#if ENABLETRACE
{GSSiEnterProg (66);
#endif
{   
    char    Mess[256];
    short    ii;     
    long	Offset=-1;
    MNMXCORD    Bounds, TestBounds;
    LPFILEINDEX lpIndex=0;      
    FILEINDEX	SaveFileIndex;
    LPVIEWPORT  SaveView;
    LPSTR   lpSlash;
    float   RSQMIN;
    double  BASEX[4], BASEY[4], BMX[4], BMY[4]; 
    static  BOOL    UseAVI=FALSE; 
    char    drive[8], dir[MAX_PATH], CurEntryName[MAX_PATH];
    LPSTR	lpParen,lpColon,pName,pPar;
    BOOL	ForceInc = FALSE;

	IndexEntryStartRecord = 0;
	if (CurView->ID == 6)
		ii=1;
    
    if (!ContinueProcessing)
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
        return (FALSE);
}
    if (CurView->Type == SUBVIEWPORT)
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
        return (FALSE);
}
	if (DisplayCycle > CurView->DisplayCycle)
	{
		CurView->DisplayCycle = DisplayCycle;
		ProcessText (CurView->BeginDisplayCmd);
		DestroySavedGraphicsFile (0);
		SetSavedGraphicsDC (CurView->hDC);
	}
    if (CurView->CurFile < 0)
	{
		OpenDisplayedHighlightedRefs();
	}
    if (CurView->ID == *pCommandViewport && DisplayOnlyHLT)
    {   
    	if (CurView->CurFile < 0)
    	{
	    	*PltName = 0;
	    	PltType = 6;
	    	CurView->CurFile = 1;
		    StartHLTRef = LONG_MIN;		
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
	    	return TRUE;
}
	    }
	    else
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
	    	return FALSE;
}
    }
Start:
    if (CurView && CurView->CurFile >= CurView->NumFiles || (!VisScan && !Pick && CurView->Type == VPTYPE_PROFILE))
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
        return (FALSE);
}
    if (ForceInc ||
    	CurView->CurFile == -1 ||
    	CurView->FileType[CurView->CurFile] < VPFILETYPE_PLOTDIR ||
    	CurView->FileType[CurView->CurFile] == VPFILETYPE_HLTLIST ||
    	CurView->FileType[CurView->CurFile] == VPFILETYPE_SUBVP ||
    	(CurView->SubFile && !CurView->hlpIndex[CurView->CurFile]) ||  
    	((CurView->FileType[CurView->CurFile] == VPFILETYPE_PLOTDIR || CurView->FileType[CurView->CurFile] == VPFILETYPE_DTM) && !CurView->hlpIndex[CurView->CurFile]) ||
    	(CurView->FileType[CurView->CurFile] == VPFILETYPE_ORTHODIR && !CurView->hlpIndex[CurView->CurFile]))
    {   
    	ForceInc = FALSE;
    	GetPCTPLOTType ();
    	if (!IncrementFile ())
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
	        return (FALSE);
}
    } 
    if (CurView->CurFile >= CurView->NumFiles)
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
        return (FALSE);
}
    FileNum = CurView->CurFile;
    SubFile = max (0,CurView->SubFile-1);  
    PltType = CurView->FileType[CurView->CurFile];
    if (PltType == VPFILETYPE_SUBVP)
    {   
    	LPVIEWPORT	SaveVP=CurView;
    	BOOL	Err; 
    	BOOL	Active;
    	
    	CurView = SetVPFromName (CurView->lpFiles[CurView->CurFile],&Err);
		_fstrcpy (PltName,CurView->lpFiles[0]);  
		PltType = CurView->FileType[0];    
		Active = CurViewActive ();
		CurView = SaveVP; 
		if (Err || !Active)
		{
	        ForceInc = TRUE;
        	goto Start;
        }
		WantGMDNegGrid = TRUE;
    }
    else
    {    
     	if (CurView->CurFile >= 0)
         	_fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]); 
         else
         	ii=1; 
	}         
    if (PltType== VPFILETYPE_DTM)
    	if (!Pick)
    		SetDTMRenderAs (CurView->CurFile);
    if (PltType== VPFILETYPE_HLTLIST)
    {
//        IncrementFile (); 
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
	   	return (TRUE);
}
    } 
    if (PltType == VPFILETYPE_MACRO)
	{
		if (ExistFile (PltName))
    		goto DisplayFile;  
	}
    else if (PltType< VPFILETYPE_PLOTDIR || (PltType == VPFILETYPE_DTM && CurView->DTMRenderAs[CurView->CurFile] != DTM_RENDER_CONTOURS))
    {   
    	 ConvertLayer = -1;
//    	 if (!VisScan && !PeopleNet) removed 5/7/01 - ConvertCoordClose now called whenever [%ALT_PROJECTION] is set
//	     	ConvertCoordClose ();
         FileInIndex = 0; 
         if (!VisScan && !PeopleNet && !FastPick)
         {
			 ProcessGlobal ("[%LAYERINIT]");
	         _fstrcpy (Mess,PltName);
			 ExpandText(Mess);
			 strupr(Mess);
			 if ((pPar = strstr (Mess,".GMD(")))
				 *(pPar+4) = 0;
			 if ((pPar = strstr (Mess,".MDB(")))
				 *(pPar+4) = 0;
			 if ((pPar = strstr (Mess,".GDB(")))
				 *(pPar+4) = 0;
			 if (*Mess)
			 {
				 char	temp[MAX_PATH];

				 strcpy (temp,Mess);
				 *MaskAreaFile = 0;
				 _splitpath (Mess,drive,dir,0,0);
				 sprintf (Mess,"%s%sglobal.ini",drive,dir);
				 LoadGlobalInit (Mess,FALSE); 
			 }
	     }
//         IncrementFile (); 
         if (ForceRefIndex || ForceTAGIndex || UseRefOrTAGIndex)
            OpenRefIndex(TRUE);
         goto DisplayFile;
    } 
    if (VisScan && CurView->FileType[CurView->CurFile]== VPFILETYPE_ORTHODIR)
    {
//        IncrementFile ();  
		ForceInc = TRUE;
        goto Start;
    }
        
    if (!CurView->hlpIndex[CurView->CurFile]) 
    {   
		switch (MapFileType(CurView->lpFiles[CurView->CurFile], 0, 0))
    	{
    		case MT_INDEX:
		        if ((!VisScan && !MapIndexVisible (CurView->lpFiles[CurView->CurFile])) ||
		        	!(CurView->hlpIndex[CurView->CurFile]=OpenMapIndex (CurView->lpFiles[CurView->CurFile],0)))
		        {
		           // IncrementFile (); 
		            ForceInc = TRUE;
		            goto Start;
		        }
		        _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]); 
		        break;
	        
	        case MT_DTM:
		        UseAVI=FALSE;
				if (CurView->DTMRenderAs[CurView->CurFile] == DTM_RENDER_CONTOURS)
				{   
					LPSTR pDot = _fstrrchr (CurView->lpFiles[CurView->CurFile],'.');
					
					if (pDot && !_fstricmp (pDot,".TIN"))
					{
			        	if (!(CurView->hlpIndex[CurView->CurFile]=OpenMapIndex (CurView->lpFiles[CurView->CurFile],0)))
				        {
				            ForceInc = TRUE;
				            goto Start;
				        }
				        goto S10; 
				    }
		        	_fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]); 
   			        goto DisplayFile;
				}
				else
					goto Next;
	        	break;

	        case MT_PLT:
	        case MT_ORA:
			case MT_GMD:
	        case MT_SID:
			case MT_DGN7:
			case MT_DGN8:
			case MT_SHP:
			case MT_IMAGE:
			case MT_GPX:
			case MT_PERSONAL_GEO_DB:
			case MT_FILE_GEO_DB:
		        _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);  
		       // IncrementFile (); 
		        goto DisplayFile;
		    
		    default:
		    	goto Next;
		}
		ExpandText (PltName);
        UseAVI=FALSE;
        PltType = CurView->FileType[CurView->CurFile];
        if (PltType == VPFILETYPE_ORTHODIR)
        {
	        lpSlash = _fstrrchr(PltName,'\\');
	        if (lpSlash)
	        {
	            ++lpSlash;
	            *lpSlash = '\0';
	        } 
            _fstrcat (PltName,"orthos[%INDEXRES]"); 
            _fstrcat (PltName,ImageExtension); 
            
            if (ShowFileBounds || FileOnSystem(PltName)) 
                UseAVI=TRUE;
            else
            {   
		        lpIndex = (LPFILEINDEX)GlobalLock(CurView->hlpIndex[CurView->CurFile]); 
		        SaveFileIndex = *lpIndex;
			    if (GetNextIndexEntry(lpIndex))
			    {   
			    	if (_fstrstr (lpIndex->CurrentEntry->Name,".pcx") ||
			    		_fstrstr (lpIndex->CurrentEntry->Name,".tif") ||
			    		_fstrstr (lpIndex->CurrentEntry->Name,".sid") ||
			    		_fstrstr (lpIndex->CurrentEntry->Name,".bmp"))
			    	{    
			    		*lpIndex = SaveFileIndex;
	            		GlobalUnlock (CurView->hlpIndex[CurView->CurFile]);  
	            		goto S10; 
	            	}
			    }
            	GSSiGlobUlFree (&CurView->hlpIndex[CurView->CurFile]);
	           // IncrementFile ();   
	            ForceInc = TRUE;
    	        goto Start;
        	}
       
        }
S10:    if (CurView->hlpIndex[CurView->CurFile])
		{
			lpIndex = (LPFILEINDEX)GlobalLock(CurView->hlpIndex[CurView->CurFile]);
        	if (PltType != VPFILETYPE_ORTHODIR && (ForceRefIndex || ForceTAGIndex || UseRefOrTAGIndex))
            	OpenRefIndex(TRUE);
        }
        else
        	lpIndex = 0;
    }
    else
        lpIndex = (LPFILEINDEX)GlobalLock(CurView->hlpIndex[CurView->CurFile]);
            
Next: 
	if (!lpIndex)
    {
        //IncrementFile ();
        ForceInc = TRUE;
        goto Start;
    }
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
            
        if (UpdateOrthoIndex && CurView->FileType[CurView->CurFile]==5)
        {   
            HFILE   FidIndex;
			OFSTRUCTGM    OFStruct;
            
            FidIndex = GSSiOpenFile(lpIndex->FileName,&OFStruct,OF_READWRITE);
            GSSillseek (FidIndex,lpIndex->FirstIndexFileOffset,0);
            BigWrite (FidIndex,&lpIndex->FirstIndex,(size_t)lpIndex->Length,-1);
            GSSiClose2 (&FidIndex);
        } 
        if (!(lpIndex=GetNextIndexHeader(&CurView->hlpIndex[CurView->CurFile],TRUE)))
        {   
            //IncrementFile (); 
            ForceInc = TRUE;
            goto Start;
        }
    }
    if (lpIndex->FirstFoundFile)
    {   
        lpIndex->FirstFoundFile=FALSE;
/*        if (CurView->FileType[CurView->CurFile]==5)
        { 
            if (CurView->WindowZoomedToOrtho && CurView->OrthoRes >=0)
            {
                CurView->OrthoRes = lpIndex->OrthoRes;
                SetNewBoundsToOrtho();
                SetBounds(CurView->hWnd,1);
            }
            else if (CurView->WindowIsZoomed)
                SetNewBoundsToBounds();
            else if (!CurView->HaveBounds)
            {
                SaveView = CurView;
                CurView = pViewports[0];
                Bounds = CurView->WBounds;
                CurView = SaveView;
                CurView->NewBounds = Bounds;
                SetBounds(CurView->hWnd,1); 
            }
        }*/
            
    }
    if (!GetNextIndexEntry(lpIndex))
    	goto Next;   
//	IndexEntryStartRecord = lpIndex->CurrentEntry->StartRecordNumber;
    pName = lpIndex->CurrentEntry->Name;
    if (*pName == '\\')
    	pName++;
    _fstrcpy (CurEntryName,pName);
	{
		LPSTR pDot = strrchr (CurEntryName,'.');

		if (!pDot)
			pDot = CurEntryName;
		lpParen=_fstrrchr (pDot,'(');
		if (lpParen)
			*lpParen++ = 0;
	}
	lpColon=_fstrchr (CurEntryName,':');
	if (lpColon)
		*lpColon++ = 0;
    
    if (lpIndex->UsesTimes && lpParen)
    {
    	long	MinTime, MaxTime;
    	LPSTR	lpSpace=_fstrchr(lpParen,' ');
    	
    	*lpSpace++ = 0;
    	MinTime = atol (lpParen);
    	MaxTime = atol (lpSpace);
		if (MinTime > TimeRangeEnd || MaxTime < TimeRangeBeg)
    		goto Next;               
    }
    TestBounds = lpIndex->CurrentEntry->Bounds;
    if (!AdjustFileBounds (CurView->ID,CurView->CurFile,&TestBounds,CurEntryName))
    	goto NotIn;
    if (!RectInWBounds (&TestBounds,0))
    {
NotIn:   
        if (CurView->FileType[CurView->CurFile]!=5) goto Next; 
        if (!UpdateOrthoIndex) goto Next; 
    }
    if (CurView->OrthoDisplayName[0]&&CurView->FileType[CurView->CurFile]== VPFILETYPE_ORTHODIR)
    {   
        char    TestName[MAX_PATH];
        
        if (*CurEntryName == '.')
        	_fstrcpy (TestName,CurrentOrthoOrigName);
        else
       		_fstrcpy(TestName,CurEntryName); 
        if (!_fstrchr (CurView->OrthoDisplayName,':'))
        {
            LPSTR   lpColon;
            
            lpColon = _fstrchr (TestName,':');
            if (lpColon)
                *lpColon = 0; 
            else
            {
                lpColon = _fstrrchr (TestName,'.');
                if (lpColon)
                    *lpColon = 0; 
            }
        }
        if (_fstricmp(CurView->OrthoDisplayName,TestName))
            goto Next;
    }
    if (CurView->FileType[CurView->CurFile]== VPFILETYPE_ORTHODIR)
    {
        lpIndex->CurrentEntry->Bounds=TestBounds;
        ComputeIndexOrthoRes (lpIndex);
        if (CurView->OrthoRes >=0 && CurView->WindowZoomedToOrtho)
    		CurView->OrthoRes = lpIndex->OrthoRes;
    }
/*    _fstrcpy (PltName,CurEntryName);
    if (*PltName == '\\')
    	lpSlash = 0;
    else
    	lpSlash = _fstrrchr(PltName,'\\'); 
    if (!lpSlash || UseAVI)*/
    {
        _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]);
        ExpandText (PltName);
        lpSlash = _fstrrchr(PltName,'\\');
        if (lpSlash)
        {
            ++lpSlash;
            *lpSlash = '\0';
        } 
        if (UseAVI)
        {
            _fstrcat (PltName,"orthos[%INDEXRES]");  
            _fstrcat (PltName,ImageExtension); 
        }
        else if (*CurEntryName == '\\')  
        	_fstrcat (PltName,&CurEntryName[1]);
        else
            _fstrcat (PltName,CurEntryName);
         
    } 
	//ConvertNameToCDName (PltName);
    if (!UseAVI && !Pick)
    {
        if (!ExistFile (PltName)) 
        {
        	GlobalUnlock (CurView->hlpIndex[CurView->CurFile]);
            goto Start;
        } 
    }

    PltType = CurView->FileType[CurView->CurFile];
	if (PltType == VPFILETYPE_PLOTDIR || PltType == VPFILETYPE_DTM || (PltType == VPFILETYPE_ORTHODIR && MapFileType(PltName, 0, 0) == MT_SID))
    	goto Exit;  
    if (UseAVI) 
    {   
    	LPSTR	lpAT;
    	
    	if ((lpAT = _fstrrchr (lpIndex->CurrentEntry->Name,'@')))
    	{
    		lpAT++;
    		CurOrthoFrame = atol (lpAT); 
    		if (CurOrthoFrame < 0)
    			goto Next;
    	}
    	else
        	CurOrthoFrame = lpIndex->FileInIndex-1;
    }
    else
        CurOrthoFrame = -1;
    ExpandText (PltName);
    if (!OrthoInBuffer(PltName, CurOrthoFrame))
    {  
    	LPORTHO CurOrtho = (LPORTHO)GlobalLock (hOrthos) + OrthoID; 
    	
        _fstrcpy (CurOrtho->Name,PltName);
        if (*CurEntryName == '.')
        	_fstrcpy (CurOrtho->OrigName,CurrentOrthoOrigName);
        else
        	_fstrcpy (CurOrtho->OrigName,CurEntryName); 
        CurOrtho->Frame = CurOrthoFrame;
        CurOrtho->Bounds = lpIndex->CurrentEntry->Bounds;
        CurOrtho->Width=lpIndex->CurrentEntry->BMWidth;
        CurOrtho->Height=lpIndex->CurrentEntry->BMHeight; 
        CurOrtho->BitCount=lpIndex->CurrentEntry->BMBitCount;
        ComputeIndexOrthoRes (lpIndex);
        CurOrtho->Res = lpIndex->OrthoRes; 
        GlobalUnlock (hOrthos);
    } 
    {
    	LPORTHO CurOrtho = (LPORTHO)GlobalLock (hOrthos) + OrthoID; 
     
	    Offset = CurOrthoFrame;
	    CurOrtho->LastUsed = OrthoUse++;
	    CloseTRANS2 (&hTranBMToBase);
	    CloseTRANS2 (&hTranBaseToBM); 
	    if (CurOrtho->BitCount != 8)
	        ii=1;
	    BMX[0]=-1;
	    BMX[1]=-1;
	    BMX[2]=lpIndex->CurrentEntry->BMWidth;
	    BMX[3]=BMX[2];
	    BMY[0]=-1;
	    BMY[1]=lpIndex->CurrentEntry->BMHeight;
	    BMY[2]=BMY[1];
	    BMY[3]=-1;
	    BASEX[0]=CurOrtho->Bounds.xmn-CurOrtho->Res;
	    BASEX[1]=BASEX[0];
	    BASEX[2]=CurOrtho->Bounds.xmx+CurOrtho->Res;
	    BASEX[3]=BASEX[2];
	    BASEY[0]=CurOrtho->Bounds.ymn-CurOrtho->Res;
	    BASEY[1]=CurOrtho->Bounds.ymx+CurOrtho->Res;
	    BASEY[2]=BASEY[1];
	    BASEY[3]=BASEY[0];
		CurView->FileMNMX=CurOrtho->Bounds;
	    hTranBMToBase = STRAN2 (1614,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,0);
	    hTranBaseToBM = STRAN2 (1615,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,0);
        GlobalUnlock (hOrthos);
	}
Exit:
    FileInIndex = lpIndex->FileInIndex; 
    GlobalUnlock(CurView->hlpIndex[CurView->CurFile]); 
DisplayFile: 
	if (!VisScan && !FileIsVisible (PltName))
		goto Start;

    if (ForceRefIndex || ForceTAGIndex)
   		StatusWindowUpdate (0,PltName, NumCopyFiles,++CopyFileNum);
    if (GetGlobalBVal ("[%DISPLAYFILENAME]")) 
    	SetWindowText (hWndMain,PltName); 
    if (DoMapCopy)
    {
    	if (CurView->SubFile)
	    	CopyMapFile(CurView->OrigFile,PltName,Offset,CurFileIndexEntry);
    	else
	    	CopyMapFile(CurView->lpFiles[FileNum],PltName,Offset,CurFileIndexEntry);
	}
/*    if (RestartOption)
    {
        HFILE   FidRestart;
        OFSTRUCTGM    OFStruct;
        
        if (RestartName[0])
        {
            if (_fstricmp (RestartName,PltName)==0)
                RestartName[0]=0; 
            if (FileNum)
                goto Start;
        }
        FidRestart = GSSiOpenFile ("restart.txt",&OFStruct,OF_CREATE);
        fputstring (PltName,FidRestart);
        GSSiClose2 (&FidRestart);
    } */
{
#if ENABLETRACE
GSSiExitProg (66);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}                        

double ComputeRes (long h, long w, MNMXCORD bounds)
#if ENABLETRACE
{GSSiEnterProg (67);
#endif
{   double dBM, dBase, res;

    dBM = LDIST (1,1,w,h);
    dBase = LDIST (bounds.xmn,bounds.ymn,bounds.xmx,bounds.ymx);
    res = dBase / dBM;
{
#if ENABLETRACE
GSSiExitProg (67);
#endif
    return (res);
}
    
#if ENABLETRACE
}
#endif
} 

BOOL WantThisPass (void)  
{   
	char	str[32];
	LPSTR	pVal, pEnd;
	
	if (CurView->PassID && CurView->PassID < 4) 
	{
	    if (GetGlobalCVal ("[%WANTPASS]",str,0))
	    {
	    	LPSTR	pVal=str;
		            	
	    	while (*pVal)
	    	{
	        	LPSTR	pEnd = _fstrchr (pVal,',');
			            	
	    		if (CurView->PassID == atoi (pVal))
	    			return TRUE;
	    		if (pEnd)
	    			pVal = pEnd + 1;
	    		else
	    			pVal = _fstrchr (pVal,0);
	    	}
			return FALSE;
	    } 
	}
	return TRUE;
}

BOOL IncrementFile ()
#if ENABLETRACE
{GSSiEnterProg (68);
#endif
{
    short       NumFiles,ii;
    LPSHORT     lpNumFiles;       
    MNMXCORD    TestBounds;
    HANDLE		hStr=GSSiGlobAlloc (1548,GMEM_MOVEABLE,1024);
    LPSTR		str=GlobalLock (hStr);
    LPSTR		File = str + 256;
    LPSTR		dir = File + 256;  
    LPSTR		TAGVisList = dir + 256;
	LPSTR		pGMD;
	char		SaveWhere[256];
    
    *MaskAreaFile = 0;
    if (!CurView) 
    { 
    	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (68);
#endif
    	return FALSE;                     
}
	}
Start:  
	IndexEntryStartRef=LONG_MAX;
    if (CurView->SubFile && CurView->CurFile >= 0)
    {   
        LPFILELISTENTRY lpEntry;
        
        if (CurView->SubFile == 1 ||
        	(CurView->CurFile >= 0 && CurView->SubFile > 1 && !CurView->hBinFileList))
        {   
            char        drive[8];
            
            UseDGNColors = TRUE;
			ProcessGlobal ("[%LAYERINIT]");
		    SetGlobalValue ("%LAYER_PROJECTION","");
            _fstrcpy (File,CurView->OrigFile);
            ExpandText (File);
            _splitpath (File,drive,dir,0,0);
            sprintf (File,"%s%sglobal.ini",drive,dir); 
            SetGlobalValue ("%WANTPASS","");
            LoadGlobalInit (File,FALSE); 
			if (!WantThisPass())
				goto NextFile;
			GetViewportScale (CurView->hDC); 
            sprintf (File,"%s%sfilelist.bin",drive,dir);
            if (!PeopleNet)
            {
	            _fstrcpy (str,"[%LAYER_PROJECTION]");  
	            ExpandText (str);
	            if (*str) 
	            {
		    		ConvertCoordClose ();
				    ConvertLayer = CurView->CurFile;
					LoadProjection(0, str);

				}
	            else
	            	ConvertLayer = -1;
	        }
	        GSSiGlobFree (&CurView->hBinFileList);
            CurView->hBinFileList = LoadBinaryFileList (CurView->OrigFile,File);
            if (!PeopleNet)
            {
	            _fstrcpy (str,"[%LAYER_PROJECTION]");  
	            ExpandText (str);
	            if (*str) 
	            {
		    		ConvertCoordClose ();
				    ConvertLayer = CurView->CurFile;
					LoadProjection(0, str);
				}
	            else
	            	ConvertLayer = -1;
	        }
        }
        if (!CurView->hBinFileList)
           goto NextFile;
        lpEntry = (LPFILELISTENTRY)GlobalLock (CurView->hBinFileList); 
        lpNumFiles = (LPSHORT)lpEntry;
        NumFiles = *lpNumFiles;   
        if (CurView->SubFile > NumFiles || !NumFiles)
        {   
            GSSiGlobUlFree (&CurView->hBinFileList);
            goto NextFile;
        }
        lpEntry += (CurView->SubFile);
        IndexEntryStartRef = lpEntry->StartRefno;
        CurView->SubFile++; 
	    TestBounds = lpEntry->MinMax;
	    if (!AdjustFileBounds (CurView->ID,CurView->CurFile,&TestBounds,0)) 
	    {
	        GlobalUnlock (CurView->hBinFileList);
	    	goto Start;
        }
        if (!RectInWBounds(&TestBounds,0))
	    {
	        GlobalUnlock (CurView->hBinFileList);
	    	goto Start;
        }
        _fstrcpy(CurView->lpFiles[CurView->RestoreFile],lpEntry->Name); 
        GlobalUnlock (CurView->hBinFileList); 
       	GSSiGlobUlFree (&hStr);

{
#if ENABLETRACE
GSSiExitProg (68);
#endif
        return TRUE;
}
    }
NextFile: 
    if (CurView->SubFile)
        _fstrcpy (CurView->lpFiles[CurView->RestoreFile],CurView->OrigFile);
	CurView->SubFile = 0;
	SetRestoreFile(CurView, 0);
    DisplayHollowLines (FALSE);  
	DisplayLayeredSymbols (CurView->hDC,FALSE);
    DisplayHollowLines (FALSE);  
    CurView->CurFile++;  
    LoadIndexParm (0); 
    CloseOrthoAVI ();
    if (!CurView->CurFile)
    	CurView->FirstFile = TRUE;
    if (CurView->CurFile && CurView->hlpIndex[CurView->CurFile-1]) 
    {
        CloseMapIndex (CurView->lpFiles[CurView->CurFile-1],
                       CurView->hlpIndex[CurView->CurFile-1],FALSE,TRUE);
        CurView->hlpIndex[CurView->CurFile-1] = 0;   
    }
    if (CurView->CurFile >= CurView->NumFiles)
    { 
    	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (68);
#endif
    	return FALSE; 
}
    }
    _fstrupr (CurView->lpFiles[CurView->CurFile]); 
    if (!CurVis || !CurVis->FileIsVisible[CurView->CurFile])
    	goto NextFile; 
    if (!AutoTypeConvert ())
    	goto NextFile;
    if (CurVis->FileIsVisible[CurView->CurFile]==2 && CurView->PassID == 2)
    	goto NextFile;
	if ((CurView->FileType[CurView->CurFile] != VPFILETYPE_IMAGE && CurView->FileType[CurView->CurFile] != VPFILETYPE_ORTHODIR && MapFileType(CurView->lpFiles[CurView->CurFile], 0, 0) != MT_SID) && CurView->PassID == 1)
        goto NextFile;
    if (CurView->FileType[CurView->CurFile]== VPFILETYPE_ORTHODIR && (!CurVis->WantType[5] || ForceTAGIndex || ForceRefIndex || ReorgFile))
        goto NextFile;
    if (CurView->FileType[CurView->CurFile]== VPFILETYPE_ORTHODIR && CurView->PassID >1 && CurView->PassID != 5)// && !PickOrtho)
        goto NextFile;
	if (CurView->FileType[CurView->CurFile] == VPFILETYPE_HLTLIST && Pick)
		ii = 1;
    if (CurView->FileType[CurView->CurFile]== VPFILETYPE_SUBVP)
    {   
    	LPVIEWPORT	SaveVP=CurView;
    	BOOL	Err;
    	
    	CurView = SetVPFromName (CurView->lpFiles[CurView->CurFile],&Err);
		_fstrcpy (str,CurView->lpFiles[0]); 
		SubVPMidPointWin = RectMid (&CurView->DrawRect); 
		if (!Err && !Pick)
		{
			if (CurView->Active)
				CurView->Display = TRUE;
			ResetViewport (TRUE,FALSE); 
		}
		CurView = SaveVP; 
		SubVPMidPointWorld = WinPtToBasePt (SubVPMidPointWin);
    	if (Err)
    		goto NextFile;
    }
    else
		_fstrcpy (str,CurView->lpFiles[CurView->CurFile]);
	*SaveWhere = 0;
	if ((pGMD = _fstrstr (str,".GMD("))) 
	{
		pGMD += 4;
		strcpy (SaveWhere,pGMD);
		*pGMD = 0;
	}
	ExpandText(str);
	strupr (str);
	strcat (str,SaveWhere);
	if (!PeopleNet && (!FastPick || CurView->CurFile != FastPickFileNum || FileInIndex != FastPickFII) &&
		 CurView->FileType[CurView->CurFile]!=6)
	{   
		LPSTR	pMDB=0, pGDB=0, pSLT; 
		BOOL	st = FALSE;
		
		pGMD = 0;
		if ((pGDB = _fstrstr (str,".GDB("))) 
		{
			pGDB += 4;
			*pGDB = 0;
			if (FileType (str) == 2)
				st = TRUE;
		}
		else
		{
			if ((pMDB = _fstrstr (str,".MDB("))) 
			{
				pMDB += 4;
				*pMDB = 0;
			}
			else if ((pGMD = _fstrstr(str, ".GMD(")))
			{
				pGMD += 4;
				*pGMD = 0;
			}
			else if ((pSLT = _fstrstr(str, ".SLT(")))
			{
				pSLT += 4;
				*pSLT = 0;
			}
			st = ExistFile(str);
		}
		if (pMDB)
			*pMDB = '(';	  
		else if (pGDB)
			*pGDB = '(';	  
		else if (pGMD)
			*pGMD = '(';
		else if (pSLT)
			*pSLT = '(';
		if (!st)
        	goto NextFile;
    } 
    	
	if (_fstrstr(str,"FILELIST.TXT"))
    {
        CurView->SubFile=1;
		SetRestoreFile(CurView, CurView->CurFile);
        _fstrcpy (CurView->OrigFile,CurView->lpFiles[CurView->CurFile]);
        goto Start;
    } 
    if (ForceTAGIndex || ForceRefIndex)
    {   
    	LPSTR	lpBS;
    	
    	if (CurView->FileType[CurView->CurFile]==6)
    		goto NextFile;
    	_fstrcpy (str,CurView->lpFiles[CurView->CurFile]);
    	ExpandText(str);
    	_fullpath (TAGVisList,str,255); 
    	lpBS = _fstrrchr (TAGVisList,'\\');
    	lpBS++;
    	*lpBS = 0; 
    	if (ForceTAGIndex)
    		_fstrcat (TAGVisList,"tagindex.vis");
    	else
    		_fstrcat (TAGVisList,"refindex.vis");
 	    if (ExistFile(TAGVisList))
		 	LoadVisList (TAGVisList);
		else
			InitVis ();
    }
    StartHLTRef = LONG_MIN;	
   	GSSiGlobUlFree (&hStr);
	//11/23/2011 force [%SHOWONLYFIRSTREF] on if update layer visible
	if (CurView->CurFile == CurView->UpdateFile-1)
	{
		IgnorePrevLayers = FALSE;
		UseRefOrTAGIndex = TRUE;
	}
{
#if ENABLETRACE
GSSiExitProg (68);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL AutoTypeConvert (void)
#if ENABLETRACE
{GSSiEnterProg (69);
#endif
{
    if (AutoType && (CurView->FileType[CurView->CurFile]==2 || CurView->FileType[CurView->CurFile]==4))
    {
    	LPSTR	pPlt=_fstrstr (CurView->lpFiles[CurView->CurFile],".PLT");
    	short	itype;
    	 
    	if (pPlt)
    	{
    		pPlt += 4;
    		*pPlt = 0;
    		itype = FileType (CurView->lpFiles[CurView->CurFile]);
    		switch (itype)
    		{ 
    			case 0:
{
#if ENABLETRACE
GSSiExitProg (69);
#endif
    				return FALSE;
}
    			case 1: 
    				CurView->FileType[CurView->CurFile] = 2;
    				break;
    			case 2: 
    				CurView->FileType[CurView->CurFile] = 4;
    				_fstrcat (CurView->lpFiles[CurView->CurFile],"\\INDEX");
    				break;
    		}
    	}
    }
{
#if ENABLETRACE
GSSiExitProg (69);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

HANDLE LoadBinaryFileList (LPSTR TextFile,LPSTR BinFile)
#if ENABLETRACE
{GSSiEnterProg (70);
#endif
{  
    HFILE   FidTxt, FidBin;
    struct _stati64    stattxt, statbin; 
    double  dtime;
    short     ii, NumFiles; 
    LPSHORT   lpNumFiles;
    OFSTRUCTGM    OFStruct;
    long     len;
    LPFILELISTENTRY lpEntry; 
    MNMXCORD	TotMinMax;
	MNMXCORD	NullMinMax;
    HANDLE  hBinFileList;   
    char    str[280]; 
    short	First2Bytes, Marker=-32349;
	LPSTR	pTAB;
	long	StartRef;
	BOOL	SaveDisplay = Display;
    
    DBoundsInit (&TotMinMax);
    DBoundsInit (&NullMinMax);
    if (PeopleNet)
    {   
    	statbin.st_size = USHRT_MAX;
		FidBin = GSSiOpenFile (BinFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
		if (FidBin != HFILE_ERROR)
			goto GetBin;
	}
    FidTxt = GSSiOpenFile (TextFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
    if (FidTxt==HFILE_ERROR) 
{
#if ENABLETRACE
GSSiExitProg (70);
#endif
        return 0;  
}
    if (!UseBinFileList)
    	goto MakeBin;
    ii=GSSifstat (FidTxt,&stattxt);
    FidBin = GSSiOpenFile (BinFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
    if (FidBin==HFILE_ERROR) 
        goto MakeBin;
    ii=GSSifstat (FidBin,&statbin);
    dtime = difftime (stattxt.st_mtime,statbin.st_mtime); 
    if (dtime<0 && statbin.st_size) 
    {
        GSSiClose2 (&FidTxt);
        goto GetBin;
    }
    GSSiClose2 (&FidBin); 
    
MakeBin:
	Display = FALSE;
    if (UseBinFileList)
    	FidBin = GSSiOpenFile (BinFile,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
    NumFiles = 0;    
    hBinFileList = GSSiGlobAlloc (  53,GMEM_MOVEABLE,16*(long)USHRT_MAX);
    lpNumFiles = (LPSHORT)GlobalLock (hBinFileList);
    lpEntry = (LPFILELISTENTRY)lpNumFiles; 
    lpEntry++;
    len = sizeof(FILELISTENTRY);
	InLoadBinaryFileList = TRUE;
    while (fgetstring(str,278,FidTxt))
    { 
		LPSTR	pPar, pDot;

		if ((pTAB = strrchr (str,'\t')))
		{
			*pTAB++ = 0;
			StartRef = atol (pTAB);
		}
		else
			StartRef = LONG_MAX;
    	Truncate (str);
		Strip (str,'\t'); //remove any extra tabs
		pPar = strrchr (str,'(');
		pDot = strchr (str,'.');
		if (pDot < pPar && strnicmp (pDot,".MDB",3) && strnicmp (pDot,".GDB",3))
			*pPar = 0;
        _fstrcpy (lpEntry->Name,str);
		lpEntry->StartRefno = StartRef;
		switch (MapFileType(lpEntry->Name, 0, 0))
        {
        	case MT_INDEX:
        		if (!GetMapIndexBounds (lpEntry->Name,&lpEntry->MinMax))
			    	lpEntry->MinMax = NullMinMax;
			    break;
			case MT_DGN7:
			case MT_DGN8:
			case MT_ORA:
			case MT_GMD:
			case MT_SHP:  
			case MT_PLT:
			case MT_SID:
			case MT_GPX:
			case MT_IMAGE:
			case MT_PERSONAL_GEO_DB:
			case MT_FILE_GEO_DB:
			{
				int SavePltType = PltType;
				int saveSubFile = CurView->SubFile;

				PltType = 2;
				_fstrcpy (PltName,lpEntry->Name);

			    if (OpenMap ((HWND)1,0))
		        {   
				    CloseMap (FALSE);
		        	lpEntry->MinMax = CurView->FileMNMX;
			    }
			    else
			    	lpEntry->MinMax = NullMinMax;
				PltType = SavePltType;
				CurView->SubFile = saveSubFile;
			}
			    break;
			default:
			    	lpEntry->MinMax = NullMinMax;
		} 
		AddMinMaxD (&TotMinMax,&lpEntry->MinMax);
        NumFiles++; 
        lpEntry++; 
        len += sizeof(FILELISTENTRY); 
		if (NumFiles >= 256)    
			ii=1;
//			break; 
    } 
	InLoadBinaryFileList = FALSE;
	Display = SaveDisplay;
    GSSiClose2 (&FidTxt); 
    GlobalUnlock (hBinFileList);  
    lpNumFiles = (LPSHORT)GlobalLock (hBinFileList);
    lpEntry = (LPFILELISTENTRY)lpNumFiles; 
    *lpNumFiles = NumFiles; 
    lpEntry->MinMax = TotMinMax;
    if (!UseBinFileList) 
    {
    	GlobalUnlock (hBinFileList);  
{
#if ENABLETRACE
GSSiExitProg (70);
#endif
	    return hBinFileList;
}
	}
    BigWrite (FidBin,(HPSTR)&Marker,2,-1); 
    BigWrite (FidBin,(HPSTR)lpNumFiles,len,-1); 
    GSSiClose2 (&FidBin);
    GSSiGlobUlFree (&hBinFileList);
    FidBin = GSSiOpenFile (BinFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
    ii=GSSifstat (FidBin,&statbin);
    
GetBin:
    BigRead (FidBin,(HPSTR)&First2Bytes,2);
	if (First2Bytes != Marker)
	{
	    GSSiClose2 (&FidBin); 
	    FidTxt = GSSiOpenFile (TextFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
		goto MakeBin;
	}
    hBinFileList = GSSiGlobAlloc (  54,GMEM_MOVEABLE,(int)statbin.st_size);
    lpNumFiles = (LPSHORT)GlobalLock (hBinFileList);
    BigRead (FidBin,(HPSTR)lpNumFiles,(int)statbin.st_size-2);
    GlobalUnlock (hBinFileList);
    GSSiClose2 (&FidBin);   
{
#if ENABLETRACE
GSSiExitProg (70);
#endif
    return hBinFileList;
}
#if ENABLETRACE
}
#endif
}
 
void ClearAllBounds (void)
#if ENABLETRACE
{GSSiEnterProg (71);
#endif
{   short iview;
    LPVIEWPORT	SaveView;
    
    SaveView = CurView;
    for (iview = 0;iview<*pNumViewports; iview++)
    {   SetCurView ( pViewports[iview]); 
        ClearBounds ();
    }  
    SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (71);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ClearBounds(void)
#if ENABLETRACE
{GSSiEnterProg (72);
#endif
{
     CurView->HaveBounds = FALSE;
     CurView->WindowIsZoomed = FALSE;
   	if (CurView->WindowZoomedToOrtho == 1)
       	CurView->WindowZoomedToOrtho = 0;
{
#if ENABLETRACE
GSSiExitProg (72);
#endif
     return;
}

#if ENABLETRACE
}
#endif
}

void SetClassColor (int i,int type, COLORREF color)
#if ENABLETRACE
{GSSiEnterProg (82);
#endif
{   
    short Style = PS_SOLID;
    
    if (i>=MAX_NEW_OBJECTS)
{
#if ENABLETRACE
GSSiExitProg (82);
#endif
    	return;
}
    if (CurView->NewObject[i].Handle) 
        GSSiDeleteObject(&CurView->NewObject[i].Handle); 
    else 
    {
        if (type != 1)
			CurView->NewObject[i].Width=(float)0.0;  
        Style = PS_SOLID;
    }
    
    CurView->NewObject[i].Type  = (unsigned char)type;
    CurView->NewObject[i].R     = GetRValue(color);
    CurView->NewObject[i].G     = GetGValue(color);
    CurView->NewObject[i].B     = GetBValue(color);
    if (type == 3)
    	CurView->NewObject[i].Width = GetWValue(color);  
    CurView->NewObject[i].Style = (unsigned char)Style;  
    CurView->NewObject[i].ProPen = 0;
    CreateNewObject(i);
{
#if ENABLETRACE
GSSiExitProg (82);
#endif
    return;
}

#if ENABLETRACE
}
#endif
}

void SetClassLineType (int i, int Type, float Width, int Style,int ProPen, COLORREF color)
#if ENABLETRACE
{GSSiEnterProg (83);
#endif
{
    if  (i>=MAX_NEW_OBJECTS)
{
#if ENABLETRACE
GSSiExitProg (83);
#endif
    	return;
}
    if (CurView->NewObject[i].Handle) 
        DeleteObject(CurView->NewObject[i].Handle); 
    CurView->NewObject[i].R=GetRValue(color);
    CurView->NewObject[i].G=GetGValue(color);
    CurView->NewObject[i].B=GetBValue(color);
    CurView->NewObject[i].Type=(unsigned char)Type;
    CurView->NewObject[i].Width=Width;  
    CurView->NewObject[i].Style=(unsigned char)Style;  
    CurView->NewObject[i].ProPen=(unsigned char)ProPen;  
    CreateNewObject(i);
{
#if ENABLETRACE
GSSiExitProg (83);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

short HaveMapTimer (void)
#if ENABLETRACE
{GSSiEnterProg (84);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (84);
#endif
    return (idTimer);
}
#if ENABLETRACE
}
#endif
}

void NewMap(void)
#if ENABLETRACE
{GSSiEnterProg (85);
#endif
{   
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (85);
#endif
		return;
}
    CurView->WindowIsZoomed = FALSE; 
    if (CurView->WindowZoomedToOrtho == 1)
    	CurView->WindowZoomedToOrtho = 0;
	ClearHighlightList (FALSE); 
#if ENABLETRACE
}
#endif
}

void HaltMapDisplay(BOOL ClearCFGStack,BOOL saveScreen)
#if ENABLETRACE
{GSSiEnterProg (86);
#endif
{   short iview; 
    LPVIEWPORT	SaveView=CurView;
    LPTHEME		SaveTheme=CurTheme; 
    short	ii;  
                        
//    DisplayFinOpt = 0;  
	if (MemMap)
		ii = 1;
	KillTimer(hWndMain, 1);
	TrapKillTimer = FALSE;
    if (DisableHalt)
{
#if ENABLETRACE
GSSiExitProg (86);
#endif
    	return; 
}
/*    {//tempdebu
    	char str[128];
	    static	long lastcycle=-1;
    	
    	haltseq++;  
    	if (haltseq == debughalt)
    		ii=1;
    	sprintf (str,"Halt %ld",haltseq);
    	SetWindowText (hWndMain,str);    
    	if (DisplayCycle==lastcycle)
    		ii=1;
    	lastcycle = DisplayCycle;
    } */
//    CloseSymDict(); 
	if (InDisplayProcessing == 1)
		ii = 1;
	useGDIPlus = FALSE;
	NotifyFunction((LPVIEWPORT)-1, GF_HALTDISPLAY);

    DisplayHollowLines (TRUE);  
    DisplayLayeredSymbols (0,TRUE);
	LoadIndexParm (0);
	CloseDisplayedRefs();
	ClosePrevLayers ();
// keeps BMP Output from working	CloseOrthos ();
    if (hSymdb)
    	ii=1;
	ComputePCTTheme=0;
    for (iview = 0;iview<*pNumViewports;iview++)
    {   
    	if (pViewports[iview])
    	{
	        SetCurView ( pViewports[iview]); 
			if (CurView->pTheme)
	        {
				LPTHEME	SaveTheme = CurTheme;
				CurTheme = CurView->pTheme;
				if (CurView->PassID == 0)
	                ThemeEndDataPass(FALSE);
	            else if (CurView->PassID <5)
	                ThemeEndDisplayPass(TRUE,FALSE,TRUE);
				CurTheme = SaveTheme;
			}
			if (CurView->pTheme && CurView->pTheme->ID == GF_STREET_TEXT_THEME)
			{
				LPTHEME	SaveTheme = CurTheme;

				CurTheme = CurView->pTheme;
				DisplayStreetLabels (TRUE);
				CurTheme = SaveTheme;
			}

	    }   
    }  
    SetCurView ( SaveView);
    CloseAllIndexes ();
    CloseMap (FALSE); 
    CloseRefIndex(TRUE);  
//    CloseSymDict();
    AddTextRect(0);
    CloseTRANS2 (&hTranBMToBase);
    CloseTRANS2 (&hTranBaseToBM);
//    ConvertCoordClose ();
    if (ClearCFGStack) 
    {
    	while (ConfigLevel)
		{   
			LPSTR	pFile;
			HDC		hDC=GetDC (hWndMain);
			
	        UnallocateConfig (); 
	        pFile = GlobalLock (hSavedConfig[ConfigLevel-1]);
	        _fstrcpy (CfgName,pFile);
	        GlobalUnlock (hSavedConfig[ConfigLevel-1]); 
	        OpenConfig(hWndMain,hDC);   
	        GSSiRemove (CfgName);
	        ConfigLevel--;
	        if (!ConfigLevel)
	        	_fstrcpy (CfgName,Lev0CfgName);
	        ReleaseDC (hWndMain,hDC);
		}	
		NextCFGTAG[0] = 0;
		BlockSocketProcessing (FALSE);
    } 
    InDisplayProcessing = FALSE;  
    TrapKillTimer=TRUE;   
    CurTheme = SaveTheme;
	if (saveScreen)
		SaveFullWindowBitmap (0); 
	firstDisplayComplete = TRUE;
{
#if ENABLETRACE
GSSiExitProg (86);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void GetItemMinMax (LPMINMAX pMinMax,LPMNMXCORD Rect) 
#if ENABLETRACE
{GSSiEnterProg (87);
#endif
{
    POINT   Point;
    DPOINT  DPoint;
    
    if (pMinMax)
    {
        Point.x = pMinMax->xmn;
        Point.y = pMinMax->ymn;
        DPoint = FilePtToBasePt (Point);
        Rect->xmn = DPoint.x-FileDistToBaseDist;
        Rect->ymn = DPoint.y-FileDistToBaseDist;
        Point.x = pMinMax->xmx;
        Point.y = pMinMax->ymx;
        DPoint = FilePtToBasePt (Point);
        Rect->xmx = DPoint.x+FileDistToBaseDist;
        Rect->ymx = DPoint.y+FileDistToBaseDist;
    }
{
#if ENABLETRACE
GSSiExitProg (87);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void GetFileMinMax (LPMINMAX pMinMax,LPMNMXCORD Rect) 
#if ENABLETRACE
{GSSiEnterProg (87);
#endif
{
    POINT   Point;
    DPOINT  DPoint;
    
	if (!Rect)
		return;
	MinMaxInit (pMinMax);
    DPoint.x = Rect->xmn;
    DPoint.y = Rect->ymn;
    Point = BasePtToFilePt (DPoint); 
    AddPointToMinMax (Point,pMinMax);
    DPoint.y = Rect->ymx;
    Point = BasePtToFilePt (DPoint); 
    AddPointToMinMax (Point,pMinMax);
    DPoint.x = Rect->xmx;
    Point = BasePtToFilePt (DPoint); 
    AddPointToMinMax (Point,pMinMax);
    DPoint.y = Rect->ymn;
    Point = BasePtToFilePt (DPoint); 
    AddPointToMinMax (Point,pMinMax);
{
#if ENABLETRACE
GSSiExitProg (87);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
BOOL BuildRefIndexes (BOOL UseCurrentViewport)
#if ENABLETRACE
{GSSiEnterProg (434);
#endif
{   
 	 LPSTR	pDupFiles;
 	 HANDLE	handle;   
 	 HCURSOR	hcurSave;
 	 BOOL	SaveURT = UseRefOrTAGIndex;
	 BOOL	saveRedisplayOnly = RedisplayOnly;

	 UseRefOrTAGIndex=FALSE;		     	 
	 TimeRangeBeg = 0;
	 TimeRangeEnd = LONG_MAX;
	 if (!UseCurrentViewport)
	 	SetViewport(*pCommandViewport);

	 DisableHalt = TRUE; 
	 DisableMarginPan = TRUE; 
	 setDoPaint( FALSE); 
	 SelectVisList (FALSE);
	 if (!SaveBuildTAGVis) 
	 {   
		HANDLE	hVis;
			    		
        hVis=GSSiGlobAlloc (   1,GHND,sizeof(VISLIST));
        SaveBuildTAGVis =(LPVISLIST) GlobalLock (hVis);  
        *SaveBuildTAGVis = *CurVis;
        SaveBuildTAGVis->hVisList = hVis;
        SaveBuildTAGVis->LastVisList = 0;
        SaveBuildTAGVis->NextVisList = 0;
     }
	 InitVis (); 
	 CurVis->WantType[5]=0; 
     ClearAllBounds();
	 IgnoreBounds = TRUE; 
	 NumCopyFiles = 0;
	 CopyFileNum = 0;
	 DoMapCopy = 4; //gets number of files   
     CurView->CurZoomAreaRef = 0;
	 IgnoreSelectVP = TRUE; 
	 CreateStatusWind (hWndMain,1,"Getting List of Files");
     hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	 RedisplayOnly = FALSE;
	 RedisplayViewport(TRUE, TRUE);
	 RedisplayOnly = saveRedisplayOnly;
	 GSSiSetCursor(hcurSave);
	 IgnoreSelectVP = FALSE;
	 DestroyStatusWindow(0);  
	 DoMapCopy = 0; 
	 if (!ContinueProcessing)
	 {  
	 	SetContinueProcessing ( TRUE);
	 	goto Exit;
	 }  
//	 NumCopyFiles *= 2;
	 CreateStatusWind (hWndMain,2,"Building TAG/Ref Index");
	 ForceTAGIndex =  TRUE;
	 ForceRefIndex =  TRUE;
	 InRebuildRefIndex = TRUE; 
	 CloseRefIndex (TRUE);
     ClearAllBounds();
	 IgnoreBounds = TRUE; 
	 StoreTAGBounds = GetGlobalBVal2 ("[%STORETAGBOUNDS]",TRUE);
//	 SetWindowText (hWndMain,"Creating TAG and Reference index");
	 if (!UseCurrentViewport)
	 	SetViewport(*pCommandViewport);
     CurView->CurZoomAreaRef = 0;
	 hDupFiles = GSSiGlobAlloc (   2,GHND,USHRT_MAX);
	 IgnoreSelectVP = TRUE;
	 Display = FALSE;
	 RedisplayOnly = FALSE;
	 RedisplayViewport(TRUE, TRUE);
	 RedisplayOnly = saveRedisplayOnly;
	 Display = TRUE;
	 IgnoreSelectVP = FALSE;
	 if (!UseCurrentViewport)
	 	SetViewport(*pCommandViewport);
	 SelectVisList (FALSE);
	 InitVis (); 
	 CurVis->WantType[5]=0; 
	 ForceTAGIndex =  FALSE; 
	 StoreTAGBounds = FALSE;

/*	 SetWindowText (hWndMain,"Creation of TAG index complete");
	 ForceRefIndex =  TRUE;
     ClearAllBounds();
	 IgnoreBounds = TRUE;
	 SetWindowText (hWndMain,"Creating Reference index");
	 SetViewport(*pCommandViewport); 
	 hDupFiles = GSSiGlobAlloc (   2,GHND,UINT_MAX);
     CurView->CurZoomAreaRef = 0;
	 IgnoreSelectVP = TRUE;
	 RedisplayViewport(TRUE,TRUE);  
	 IgnoreSelectVP = FALSE; */
	 
	 if (!UseCurrentViewport)
		 SetViewport(*pCommandViewport);
	 pDupFiles = GlobalLock (hDupFiles);
   	 while (*pDupFiles)
   	 {
		hDupRef = BT_OPEN (pDupFiles, 0, BT_READ, 0);    
		if (hDupRef)
			FixDupRef (); 
   		pDupFiles = _fstrchr (pDupFiles,0);
   		pDupFiles++;
   	 } 
     GSSiGlobUlFree (&hDupFiles);
Exit:
	 if (SaveBuildTAGVis)
	 {
		handle = SaveBuildTAGVis->hVisList; 
		if (CurVis)
		{
			SaveBuildTAGVis->hVisList = CurVis->hVisList;
			*CurVis = *SaveBuildTAGVis; 
		}
		else
			SelectVisList (FALSE);
		GSSiGlobUlFree (&handle);
		SaveBuildTAGVis = 0;
	 } 
	 ForceRefIndex =  FALSE; 
//	 SetWindowText (hWndMain,"Creation of TAG/Reference index complete");
	 IgnoreBounds = FALSE;
	 DestroyStatusWindow(0);  
	 DisableHalt = FALSE;  
	 DisableMarginPan = FALSE;
	 setDoPaint( TRUE); 
	 InRebuildRefIndex = FALSE; 
	 UseRefOrTAGIndex = SaveURT;
{
#if ENABLETRACE
GSSiExitProg (434);
#endif
     return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayVPDialogs (BOOL Reposition)
{
	LPVIEWPORT SaveVP = CurView;
	int	SaveCurCfg = CurrentConfig;
	UINT	iview;
	BOOL	rtn=FALSE;
	
	if (!IgnoreWPC && !IsIconic (hWndMain))
	{
	 	SetConfig(0);
		for (iview=0;iview<*pNumViewports;iview++) 
		{
			SetCurView(pViewports[iview]);
			if (pViewports[iview]->hWndDlg)
			{
				if (Reposition)
					SendMessage(pViewports[iview]->hWndDlg, GSSI_REPOSITION,0, (LPARAM)pViewports[iview]); 
				InvalidateRect(pViewports[iview]->hWndDlg,0,TRUE);
			}
		}  
	 	SetConfig(1);
		for (iview=0;iview<*pNumViewports;iview++) 
		{
			SetCurView(pViewports[iview]);
			if (pViewports[iview]->hWndDlg)
			{
				if (Reposition)
					SendMessage(pViewports[iview]->hWndDlg, GSSI_REPOSITION,0, (LPARAM)pViewports[iview]); 
				InvalidateRect(pViewports[iview]->hWndDlg,0,TRUE);
			}
		}  
		SetConfig(SaveCurCfg);
		CurView = SaveVP;
		rtn = TRUE;
	}
	return rtn;
}
BOOL VPIsCovered(LPVIEWPORT pVP)
#if ENABLETRACE
{
	GSSiEnterProg(424);
#endif
	{
		RECT	rMyRect, rOtherRect, rDestRect;
		HWND	hPrevWnd, hNextWnd;
		POINT	Point1, Point2;
		HWND	Owner;
		HWND	hWnd = hWndMain;

		rMyRect = pVP->ScreenRect;
		Point1.x = rMyRect.left;
		Point1.y = rMyRect.top;
		Point2.x = rMyRect.right;
		Point2.y = rMyRect.bottom;
		ClientToScreen(hWnd, &Point1);
		ClientToScreen(hWnd, &Point2);
		rMyRect.left = Point1.x;
		rMyRect.top = Point1.y;
		rMyRect.right = Point2.x;
		rMyRect.bottom = Point2.y;
		/*  Start from the current window and use the GetWindow()
		*  function to move through the previous window handles.
		*/
		for (hPrevWnd = hWnd;
			(hNextWnd = GetWindow(hPrevWnd, GW_HWNDPREV)) != NULL;
			hPrevWnd = hNextWnd)
		{
			/*  Get the window rectangle dimensions of the window that
			*  is higher Z-Order than the application's window.
			*/
			GetWindowRect(hNextWnd, &rOtherRect);
			FixRect(&rOtherRect);
			/*  Check to see if this window is visible and if intersects
			*  with the rectangle of the application's window. If it does,
			*  call MessageBeep(). This intersection is an area of this
			*  application's window that is not visible.
			*/
			Owner = 0;// GetWindow(hNextWnd, GW_OWNER);
			if (!IsRectEmpty(&rOtherRect) && IsWindowVisible(hNextWnd) &&
				IntersectRect(&rDestRect, &rMyRect, &rOtherRect) &&
				((Owner != hWnd) || (!WindowBelongsToViewport(hWnd))))
			{
#if ENABLETRACE
				GSSiExitProg(424);
#endif
				return TRUE;
			}
		}
{
#if ENABLETRACE
	GSSiExitProg(424);
#endif
	return FALSE;
}
#if ENABLETRACE
	}
#endif
}
