#include "graphint.h"   
#include "extrndb.h"   
#include "pnet.h" 
#include "umio.h"
#include <mmsystem.h>
#include "gmextern.h"
#include "RampCompliance.h"
#include "CurbRamps.h"
#include "CRAPI.h"


static char SubDef[1024] = { 0 };
static char	MonthAbv[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static short	nSetVals=0;
static UINT	lSetVals=0;
static char	CurrentDialogType[16];
static HIGHLIGHTDATA	HighlightData;
static THEMEHIGHLIGHTKEY	ThemeHighlightKey;
static THEMEHIGHLIGHTDATA	ThemeHighlightData;
static char	CurrentDialogFile[256]="";
#define MAXSCREENS	9
static HBITMAP	hScreenBM[MAXSCREENS]={0};
static HWND	hWndFound;
static HWND wantWnd;
static LPSTR	pFindWindowText=0;
extern BOOL InAtPrint;
typedef struct {
	DWORD process, thread;
}WINPROCESSANDTHREAD;
typedef WINPROCESSANDTHREAD *LPWINPROCESSANDTHREAD;

BOOL CreateGMStartupFile(LPSTR OutFile, LPSTR ConfigPath, BOOL LinkZoom, BOOL RetainZoom)
{
	long len;
	GetPersistentTempFileName("gmlnk",OutFile);
	HFILE Fid = GSSiOpenFile(OutFile, 0, OF_CREATE);
	if (Fid == HFILE_ERROR)
		return FALSE;
	if (LinkZoom)
		sprintf(strchr(ConfigPath, 0), "(%ld)", (ULONG)hWndMain);
	len = strlen(ConfigPath);
	BigWrite(Fid, (HPSTR)&len, 4, -1);
	BigWrite(Fid, (HPSTR)ConfigPath, len, -1);
	if (RetainZoom && hSavedZooms)
	{
		len = GlobalSize(hSavedZooms);
		LPSHORT pNumSavedViews = (LPSHORT)GlobalLock(hSavedZooms);

		BigWrite(Fid, (HPSTR)&len, 4, -1);
		BigWrite(Fid, (HPSTR)pNumSavedViews, len, -1);
		GlobalUnlock(hSavedZooms);
	}
	GSSiClose2(&Fid);
	return TRUE;
}
int GetCurrentMonitor(void)
{
	int rtn = 0;
	RECT rect, outRect;
	double area, maxarea = 0;
	
	if (numMonitors > 1)
	for (int i = 0;i<numMonitors;i++)
	{
		GetWindowRect(hWndMain, &rect);
		InflateRect(&rect, -32, -32);
		if (IntersectRect(&outRect, &MonitorRectangle[i], &rect))
		{
			area = RECTWIDTH(&rect) * RECTHEIGHT(&rect);
			if (area > maxarea)
			{
				area = maxarea;
				rtn = i;
			}
		}
	}
	return rtn;
}
void MoveCursorToMonitor(int imon)
{
	POINT pt;
	
	GetCursorPos(&pt);
	if (!PtInRect(&MonitorRectangle[imon], pt))
	{
		int x = (MonitorRectangle[imon].left + MonitorRectangle[imon].right) / 2;
		int y = (MonitorRectangle[imon].top + MonitorRectangle[imon].bottom) / 2;
		SetCursorPos(x, y);
	}
	return;
}
void MoveToMonitor(int imon,int fromMon,HWND hWnd)
{
	if (!hWnd)
		hWnd = hWndMain;
	if (fromMon != imon)
	{
		if (imon < numMonitors)
		{
			MoveWindow(hWnd, MonitorRectangle[imon].left, MonitorRectangle[imon].top, RECTWIDTH(&MonitorRectangle[imon]), RECTHEIGHT(&MonitorRectangle[imon]), TRUE);
			MoveToolbarsToMonitor(fromMon, imon);
			MoveCursorToMonitor(imon);
		}
	}
}
BOOL CALLBACK WEEnumWndProc(HWND hCtrl, LONG lParam)
{
	if (hCtrl == wantWnd)
	{
		hWndFound = hCtrl;
		return FALSE;
	}
	return TRUE;
}
int WhichMonitorIsRectMostOn(RECT rect, LPINT pPctOn)
{
	int rtn = -1;
	int maxSizeOnMonitor = 0;
	RECT outRect;
	int rectSize = RECTWIDTH(&rect) * RECTHEIGHT(&rect);

	*pPctOn = 0;
	for (int i=0; i< numMonitors; i++)
	{
		if (IntersectRect(&outRect, &MonitorRectangle[i], &rect))
		{
			int size = RECTWIDTH(&outRect) * RECTHEIGHT(&outRect);
			if (size > maxSizeOnMonitor)
			{
				maxSizeOnMonitor = size;
				*pPctOn = IDNINT ((100.0 * size) / rectSize);
				rtn = i;
			}
		}
	}
	return rtn;
}
RECT MoveRectToAMonitor(RECT rect)
{
	RECT rtn = rect;
	int pctOn;
	int imon = WhichMonitorIsRectMostOn(rect, &pctOn);

	if (imon < 0 || pctOn < 100)
	{ 
		imon = max(0, imon);
		IntersectRect(&rtn, &MonitorRectangle[imon], &rect);
	}
	return rtn;
}
HWND WindowExists(HWND hWnd)
{
	wantWnd = hWnd;
	hWndFound = 0;
	EnumWindows(WEEnumWndProc, 0);
	return hWndFound;
}

BOOL CALLBACK FWBNEnumWndProc(HWND hCtrl, LONG lParam)
{
	char        str[130];
	long	lUserData;
	HWND	hPar;

	GetWindowText(hCtrl, str, 128);
	if (!stricmp(str, (LPSTR)lParam))
	{
		hWndFound = hCtrl;
		return FALSE;
	}
	return TRUE;
}

HWND FindWindowByName(LPSTR WindowName)
{
	char	str[256], mess[128];
	DWORD	WVer;
	int		WinVer, DosVer;
	UINT	ierr;
	BOOL	rtn = TRUE;
	//DLGPROC lpfnEnumWndProc;
	DWORD thread, process;
	 
	hWndFound = FindWindow (WindowName,0);
	if (!hWndFound)
		hWndFound = FindWindow(0,WindowName);
	//EnumWindows(FWBNEnumWndProc, (LPARAM)WindowName);
	//ierr = GetWindowModuleFileName(hWndFound, str, 256);

	return hWndFound;
}
BOOL CALLBACK ShowEnumWndProc(HWND hCtrl, LPWINPROCESSANDTHREAD pWpt)
{
	DWORD process;
	DWORD thread = GetWindowThreadProcessId(hCtrl, &process);
	if (pWpt->thread == thread && pWpt->process == process)
	{
		ShowWindow(hCtrl, SW_SHOW);
		//SetWindowPos(hCtrl, HWND_TOP,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	}
	return TRUE;
}
BOOL CALLBACK HideEnumWndProc(HWND hCtrl, LPWINPROCESSANDTHREAD pWpt)
{
	DWORD process;
	DWORD thread = GetWindowThreadProcessId(hCtrl, &process);
	if (pWpt->thread == thread && pWpt->process == process)
	{
		ShowWindow(hCtrl, SW_HIDE);
		//SetWindowPos(hCtrl, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
	}
	return TRUE;
}

void ShowHideWindows(LPSTR WindowName, UINT fun)
{
	WINPROCESSANDTHREAD wpt;

	wpt.thread = GetWindowThreadProcessId(hWndFound, &wpt.process);

	if (fun == SW_SHOW)
		EnumWindows((WNDENUMPROC)ShowEnumWndProc, (LPARAM)&wpt);
	else
		EnumWindows((WNDENUMPROC)HideEnumWndProc, (LPARAM)&wpt);
	return;
}
BOOL CALLBACK ShowChildEnumWndProc(HWND hCtrl, LONG lParam)
{
	ShowWindow(hCtrl, lParam);
	return TRUE;
}

void ShowHideChildren(HWND hWndPar, UINT fun)
{
	ShowWindow(hWndPar, fun);
	EnumChildWindows(hWndPar, ShowChildEnumWndProc, fun);

	return;
}

BOOL CALLBACK FWBPEnumWndProc(HWND hCtrl, LONG lParam)
{
	char    txt[1024];
	long	lUserData; 
	HWND	hPar;
	DWORD	ProcessID=1;
	    
	GetWindowThreadProcessId (hCtrl,&ProcessID);
	if (ProcessID == (DWORD)lParam)
	{
		if (pFindWindowText && *pFindWindowText)
		{
			GetWindowText (hCtrl,txt,256);
			if (stricmp (txt,pFindWindowText))
				return TRUE;
		}
		hWndFound = hCtrl;
		return FALSE;
	}
	return TRUE;
}

HWND FindWindowByProcessID (DWORD ProcessID,LPSTR Text)
{   
	char	str[256], mess[128];
	DWORD	WVer;
	int		WinVer, DosVer;
	UINT	ierr;
	BOOL	rtn = TRUE;
   // DLGPROC lpfnEnumWndProc;
    
    hWndFound = 0;
	pFindWindowText = Text;
	EnumWindows (FWBPEnumWndProc,ProcessID);   
	return hWndFound;
} 
BOOL CALLBACK TOALLEnumWndProc(HWND hCtrl, LONG lParam)
{
	char    txt[1024];

	if (pFindWindowText && *pFindWindowText)
	{
		LPSTR wc = strchr (pFindWindowText,'*');
		if (wc)
			*wc = 0;
		int	n = strlen(pFindWindowText);
		GetWindowText(hCtrl, txt, 1022);
		if (n && !strnicmp(txt, pFindWindowText,n))
			SendConnectedProcessMessage(hCtrl,lParam, 0, 0);
	}
	return TRUE;
}

void SendMessageToAllProcesses(LPSTR Text,UINT msg)
{
	pFindWindowText = Text;
	EnumWindows(TOALLEnumWndProc, msg);
}
int	GetFunctionValue2(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{GSSiEnterProg (1348);
#endif
{   HANDLE	hMem=0,hMem2=0, hDLT,hSurf;
	LPSTR	Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,Arg7, ParLoc, lpstr, lpstrb;  
	BOOL	FromLimits, Immediate; 
	LPSTR	pEnd, pCR, pFile, Arg[20] = { 0 }, pVal, pPar;
	short	nArgs;
	HFILE	Fid1, Fid2, Fid3;   
	int		l, CvtDir, year,i;    
	DPOINT	Point, Point2;
	POINT	Point16;
	RECT	Rect; 
	HWND	hWnd;
	MNMXCORD	Bounds,Bounds2;           
	long	hNum, Refno,ii, nlong;
	LPSTR	lpColon,CmdMess;
	time_t	systime; 
	BOOL	TORF, rtn=FALSE, Err, PickVis;  
	LPVIEWPORT	SaveVP=CurView;   
	short	SaveCfg = CurrentConfig;
	short SymNum, Mode, irc;			
	short	n, n1,n2,pos, nrem, nRc;  
	double	AZ, Dist,Elevation, RVal;
	HFILE	Fid;
	LPOFSTRUCTGM	pOFStruct;
	LPDPOINT	pPoint;
	HANDLE	hDibInfo;
	long	ImageOffset; 
	double	Offset;
	HANDLE	hSQL;
	HANDLE	hTran;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle; 
	HANDLE	hPoints;
	short	nPoints;
	short	ndec; 
	short	incDbOff;
	HDC		hDC;
	COLORREF	Color;
	
	if (LinkToVar)
	{
		ExpandText (Args);
{
#if ENABLETRACE
GSSiExitProg (1348);
#endif
		return 0;
}
	}
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc ( 785,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s(%s)",LastFunctionName,Args);
		GSSiTraceLev (lpstr,1,2);
		GSSiGlobUlFree (&hMem);
	}
	switch (FunID)
	{    
		case 501: /* $THEME(LOAD,theme,vpname(opt)) Loads and activates theme
						or
							(DEACT,vpname (opt)) deactivates theme 
						or  (SET,vp name) sets to theme in specified legend vp
						or	(REFCLASS,refno,vp name(opt)) returns class in current theme (or opt specified theme) for specified refno    
						or	(SETVAL,vp name,varname,value)
						or	(GETVAL,vp name,varname,value)
						or	(QUAN,VPName,filename)
						or  (SELECTCLASS,VPName) to display selection dialog
						or  (SELECTCLASS,VPName,class,TorF) if class is 0 all classes set
						or	(GETCLASSVALS,vp name,
						or	(GETCLASS,vp name,value,activeclassesonly)
						or	(GEOCENTER,DISPLAY,vp name)
						or	(GEOCENTER,GET,vp name,class)
						or	(CLASSCOUNT,SET,vp name,class,count)
						or	(SHOWCLASSMEMBERS,vp name,classno,POINT or FLASH,fromPt,ALL or STEP,macro)
						or	(HIGHLIGHT,TorFhighlight/unhighlight,vp name,class(optional 0 for all, -1 for selected classes),TF-computeAreaAndLength

						*/
		{	 
			int		nRc; 
			short	iclass;
			
			SaveVP = CurView;      
			nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (!strcmp (Arg[1],"GETTYPE")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err && CurView->pTheme)
				{
					GetThemeIDText (CurView->pTheme->ID,OutLoc);
					SetCurView ( SaveVP);
					goto Rtnl;
				}
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp(Arg[1], "HIGHLIGHT"))
			{
				int nHlt = 0;
				SetCurView(SetVPFromName(Arg[3], &Err));
				if (!Err && CurView->pTheme)
				{
					BOOL UnHighlight = !atob(Arg[2]);
					int  class = atoi(Arg[3]);
					BOOL ComputeAreaAndLength = atob(Arg[5]);
					nHlt = HighlightFromTheme(CurView, CurView->pTheme, 0,0,0, UnHighlight, ComputeAreaAndLength);
				}
				SetCurView(SaveVP);
				itoa(nHlt, OutLoc, 10);
				goto Rtnl;
			}
			else if (!_fstrcmp(Arg[1], "GEOCENTER"))
			{
				SetCurView(SetVPFromName(Arg[3], &Err));
				if (!Err && CurView->pTheme)
				{
					if (!stricmp(Arg[2], "DISPLAY"))
						rtn = DisplayThemeGeoCenters(CurView->pTheme);
					else
					{
						Point = ComputeThemeClassGeoCenter(CurTheme, atoi(Arg[4]));
						dpointtoa(OutLoc, &Point);
						SetCurView(SaveVP);
						goto Rtnl;
					}
				}
				else
					rtn = FALSE;
				SetCurView(SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp(Arg[1], "CLASSCOUNT"))
			{
				SetCurView(SetVPFromName(Arg[3], &Err));
				if (!Err && CurView->pTheme)
				{
					if (!stricmp(Arg[2], "SET"))
					{
						int iclass = atoi(Arg[4]);
						int count = atoi(Arg[5]);
						CurView->pTheme->ClassCount[iclass] = count;
						goto RtnTrue;
					}
					else
					{
						int iclass = atoi(Arg[4]);
						itoa(CurView->pTheme->ClassCount[iclass], OutLoc, 10);
						goto Rtnl;
					}
				}
				else
					rtn = FALSE;
				SetCurView(SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp(Arg[1], "ACT"))
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err && CurView->pTheme)
				{
					CurView->pTheme->IsActive = TRUE; 
					rtn = TRUE;
				}
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"VALUELIST")) 
			{   
				GetSymListFromTheme (Arg[2],OutLoc,4000);
				goto Rtnl;
			}
			else if (!_fstrcmp (Arg[1],"VALUE")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err && CurView->pTheme)
				{
					strcpy (OutLoc,CurView->pTheme->CurValue); 
				}
				else
					*OutLoc = 0;   
				SetCurView ( SaveVP);
				goto Rtnl;
			}
			else if (!_fstrcmp (Arg[1],"SELECTCLASS")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (nArgs == 2)
				{
					rtn = SelectThemeClasses(-1,0);
				}
				else
				{
					BOOL Select = atob(Arg[4]);
					int iclass = atoi(Arg[3]);
					rtn = SelectThemeClasses(iclass,Select);
				}
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"QUAN")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				rtn = SetThemeQuan (CurView->pTheme,Arg[3]);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"SET")) //$THEME(SET,ViewportName(opt):config(1 or 2))
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err && CurView->pTheme)
				{
					CurTheme = CurView->pTheme; 
					rtn = TRUE;
				}
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"EDIT")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err && CurView->pTheme)
				{   
					UINT	StartCmd=0;
					
					if (*Arg[3])
						StartCmd = IDC_SHOW_FIELDS;
					CurTheme = CurView->pTheme; 
					rtn = EditTheme (CurView->hWnd,StartCmd);
				}
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"DEACT")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err && CurView->pTheme)
				{
					CurView->pTheme->IsActive = FALSE; 
					rtn = TRUE;
				}
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"REFCLASS")) 
			{   
				long	refno;
				LPTHEME	SaveTheme=CurTheme;
				
				refno = atol (Arg[2]); 
				if (*Arg[3])
				{
					SetCurView ( SetVPFromName (Arg[3],&Err));
					if (!Err && CurView->pTheme)
						CurTheme = CurView->pTheme; 
				}
				iclass = GetThemeRefClass (refno);
				itoa (iclass,OutLoc,10);
				CurTheme = SaveTheme;
				SetCurView ( SaveVP);
				goto Rtnl;  
			}
			else if (!_fstrcmp (Arg[1],"SHOWCLASSMEMBERS")) 
			{   
				long	rtn=0;
				LPTHEME	SaveTheme=CurTheme;
		
				if (*Arg[2])
				{
					SetCurView ( SetVPFromName (Arg[3],&Err));
					if (!Err && CurView->pTheme)
						CurTheme = CurView->pTheme; 
				}
				iclass = atol (Arg[3]);
				if (!stricmp (Arg[4],"POINT"))
				{
					POINT fromPt = atopt16 (Arg[5],&Err);
					rtn = ShowClassMembers (CurTheme,iclass,fromPt,0,Arg[6]);
				}
				itoa (rtn,OutLoc,10);
				CurTheme = SaveTheme;
				SetCurView ( SaveVP);
				goto Rtnl;  
			}

			else if (!_fstrcmp (Arg[1],"CLOSE"))
			{  
				rtn = TRUE;
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err)
					CloseTheme();
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"SETVAL"))
			{  
				rtn = FALSE;
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err)
				{
					if (CurView->pTheme)
					{
						if (!_fstricmp(Arg[3], "TITLEPCT"))
						{
							CurView->pTheme->TitleHeight = atoi(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "SYMBOLPCT"))
						{
							CurView->pTheme->ColorsWidth = atoi(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "SHOWTEXT"))
						{
							CurView->pTheme->ShowValue = atob(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "SHOWCLASSID"))
						{
							CurView->pTheme->showClassID= atob(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "DELAYTEXT"))
						{
							CurView->pTheme->DelayTextDisplay = atob(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "CONTENTS"))
						{
							if (stricmp(Arg[2], "Street Names"))
								ii = 1;
							rtn = SetThemeContents(CurView->pTheme, Arg[4]);
							if (CurView->pTheme->hVisList)
							{
								GSSiGlobFree(&CurView->pTheme->hVisList);
								CurView->pTheme->hVisList = ReadVisList(&CurView->pTheme->Contents[1]);
							}
						}
						else if (!_fstricmp(Arg[3], "SETCOLOR"))
						{
							CurView->pTheme->NotSetColor = !atob(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "DATABASE"))
						{
							strncpy0 (CurView->pTheme->DataFile,Arg[4],MAX_PATH-1);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "SQL"))
						{
							strncpy0(CurView->pTheme->SQL, Arg[4], 255);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "TITLE"))
						{
							strncpy0(CurView->pTheme->Title, Arg[4], sizeof(CurView->pTheme->Title)-1);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "MISSOPT"))
						{
							CurView->pTheme->MissOpt = atoi (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"NUMCLASS"))
						{
							CurView->pTheme->NumDesiredClass = CurView->pTheme->NumClass = atoi (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"REFVAL"))
						{
							strcpy (CurView->pTheme->RefValChar,Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"CLASSMIN"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								CurView->pTheme->ClassMin[iclass-1] = atof (Arg[5]);
								rtn = TRUE;
							}
						}
						else if (!_fstricmp (Arg[3],"CLASSMAX"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								CurView->pTheme->ClassMax[iclass-1] = atof (Arg[5]);
								rtn = TRUE;
							}
						}
						else if (!_fstricmp (Arg[3],"CLASSCOLOR"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								CurView->pTheme->ClassColor[iclass-1] = atol (Arg[5]);
								rtn = TRUE;
							}
						}
						else if (!_fstricmp (Arg[3],"TITLECOLOR"))
						{
							CurView->pTheme->TitleBoxBG = atol (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"BACKGROUNDCOLOR"))
						{
							CurView->pTheme->BGColor = atol (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"CLASSDESCRIPTOR"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								strncpy0 (CurView->pTheme->ClassBM[iclass-1],Arg[5],sizeof(CurView->pTheme->ClassBM[iclass-1]));
								rtn = TRUE;
							}
						}
						else if (!_fstricmp (Arg[3],"SORTOPT"))
						{
							CurView->pTheme->SortOption = atoi (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"INVERT"))
						{
							CurView->pTheme->InvertLegend = atob (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"FLIP"))
						{
							CurView->pTheme->FlipLegend = atob (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "FACTOR"))
						{
							CurView->pTheme->FactorLegend = atob(Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "BEGINDATAPASSMACRO"))
						{
							strncpy0(CurView->pTheme->BeginDataPassMacro, Arg[4], sizeof(CurView->pTheme->BeginDataPassMacro) - 1);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "BEGINDISPLAYMACRO"))
						{
							strncpy0(CurView->pTheme->BeginDisplayMacro, Arg[4], sizeof(CurView->pTheme->BeginDisplayMacro) - 1);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "ENDDISPLAYMACRO"))
						{
							strncpy0(CurView->pTheme->EndDisplayMacro, Arg[4], sizeof(CurView->pTheme->EndDisplayMacro) - 1);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "GRAPHICSDISPLAYMACRO"))
						{
							strncpy0(CurView->pTheme->GraphicsAttributesMacro, Arg[4], sizeof(CurView->pTheme->GraphicsAttributesMacro) - 1);
							rtn = TRUE;
						}
						else if (!_fstricmp(Arg[3], "DEPTHOPTIONS"))
						{   
							rtn=FALSE;
							if (CurView->pTheme)
								rtn = DialogBox(hInst, (LPSTR)"DEPTHOPTIONS", CurView->hWnd,(DLGPROC) DEPTHOPTIONSMsgProc);
						}

					}
				}
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"GETVAL"))
			{  
				rtn = FALSE;
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err)
				{
					if (CurView->pTheme)
					{
						if (!_fstricmp(Arg[3], "TITLEPCT"))
						{
							itoa(CurView->pTheme->TitleHeight, OutLoc, 10);
							goto Rtnl;
						}
						else if (!_fstricmp(Arg[3], "SYMBOLPCT"))
						{
							itoa(CurView->pTheme->ColorsWidth, OutLoc, 10);
							goto Rtnl;
						}
						else if (!_fstricmp(Arg[3], "INTEST"))
						{
							itoa(CurView->pTheme->InTestChar, OutLoc, 10);
							goto Rtnl;
						}
						else if (!_fstricmp(Arg[3], "TITLE"))
						{
							strcpy(OutLoc, CurView->pTheme->Title);
							goto Rtnl;
						}
						else if (!_fstricmp(Arg[3], "MISSOPT"))
						{
							itoa (CurView->pTheme->MissOpt,OutLoc,10);
							goto Rtnl;
						}
						else if (!_fstricmp (Arg[3],"SETCOLOR"))
						{
							btoa (!CurView->pTheme->NotSetColor,OutLoc);
							goto Rtnl;
						}
						else if (!_fstricmp (Arg[3],"NUMCLASS"))
						{
							itoa (CurView->pTheme->NumClass,OutLoc,10);
							goto Rtnl;
						}
						else if (!_fstricmp (Arg[3],"REFVAL"))
						{
							strcpy (OutLoc,CurView->pTheme->RefValChar);
							goto Rtnl;
						}
						else if (!_fstricmp (Arg[3],"CLASSMIN"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								ftoa (OutLoc,CurView->pTheme->ClassMin[iclass-1]);
								goto Rtnl;
							}
						}
						else if (!_fstricmp (Arg[3],"CLASSMAX"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								ftoa (OutLoc,CurView->pTheme->ClassMax[iclass-1]);
								goto Rtnl;
							}
						}
						else if (!_fstricmp (Arg[3],"CLASSCOLOR"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								itoa (CurView->pTheme->ClassColor[iclass-1],OutLoc,10);
								goto Rtnl;
							}
						}
						else if (!_fstricmp (Arg[3],"CLASSDESCRIPTOR"))
						{
							iclass = atoi (Arg[4]);
							if (iclass > 0 && iclass < MAX_THEME_CLASSES)
							{
								strncpy0 (OutLoc,CurView->pTheme->ClassBM[iclass-1],sizeof(CurView->pTheme->ClassBM[iclass-1]));
								goto Rtnl;
							}
						}
						else if (!_fstricmp (Arg[3],"SORTOPT"))
						{
							itoa (CurView->pTheme->SortOption,OutLoc,10);
							goto Rtnl;
						}
						else if (!_fstricmp (Arg[3],"INVERT"))
						{
							btoa (CurView->pTheme->InvertLegend,OutLoc);
							goto Rtnl;
						}

						else if (!_fstricmp (Arg[3],"FLIP"))
						{
							btoa (CurView->pTheme->FlipLegend,OutLoc);
							goto Rtnl;
						}

						else if (!_fstricmp (Arg[3],"FACTOR"))
						{
							btoa (CurView->pTheme->FactorLegend,OutLoc);
							goto Rtnl;
						}

					}
				}
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"GETCLASS"))
			{
				i = -1;
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err)
				{
					if (CurView->pTheme)
						i = GetThemeClass (CurView->pTheme,Arg[3],atob(Arg[4])); 
				}
				SetCurView ( SaveVP);
				itoa (i,OutLoc,10);
				goto Rtnl;
			}
			else if (!_fstrcmp (Arg[1],"GETCLASSVALS"))
			{  
				HFILE Fid;
				int	Class;
				HANDLE	hKey=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
				LPSTR	pKey=GlobalLock (hKey);
				LPSTR	str=pKey+1024;

				rtn = FALSE;
				if (*Arg[4] && (Fid = GSSiOpenFile (Arg[4],0,OF_CREATE)))
				{
					fputstring ("%CLASSVALUE\t%CLASSNUM",Fid);
					SetCurView ( SetVPFromName (Arg[2],&Err));
					if (!Err)
					{
						if (CurView->pTheme)
						{
							if (CurView->pTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME && *CurView->pTheme->ScatterFile)
							{
								HANDLE hBT = BT_OPEN (CurView->pTheme->ScatterFile,0, BT_READ, 0); 
								if (hBT)
								{
									int	pos=BT_FIRST,cond=BT_ANY;
									int	lKey = GetBTKeyLen(hBT);

									while (!BT_FIND (hBT,pKey,pos,cond,(LPSTR)&Class))
									{
										pos = BT_NEXT;

										pKey[lKey] = 0;
										if (Class && (!stricmp (Arg[3],"ALL") || (!stricmp (Arg[3],"SELECTED") && !CurView->pTheme->ClassStatus[Class-1])))
										{
											sprintf (str,"%s\t%i",pKey,Class);
											fputstring (str,Fid);
										} 
									}
									rtn = TRUE;
								}
								BT_CLOSE (hBT);
							}
							else if (CurView->pTheme->ID == GF_SINGLE_VALUE_THEME)
							{
								for (Class=1;Class<=CurView->pTheme->NumClass;Class++)
								{
									if ((!stricmp (Arg[3],"ALL") || (!stricmp (Arg[3],"SELECTED") && !CurView->pTheme->ClassStatus[Class-1])))
									{
										sprintf (str,"%f\t%i",CurView->pTheme->ClassMin[Class-1],Class);
										fputstring (str,Fid);
										sprintf (str,"%f\t%i",CurView->pTheme->ClassMax[Class-1],Class);
										fputstring (str,Fid);
									} 
								}
								rtn = TRUE;
							}
						}
					}
					GSSiClose2 (&Fid);
				}
				GSSiGlobUlFree (&hKey);
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"RESET"))
			{  
				rtn = FALSE;
				SetCurView ( SetVPFromName (Arg[2],&Err));
				if (!Err)
				{
					if (CurView->pTheme)
						CurView->pTheme->Pass = 0; 
					rtn = TRUE;
				}
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"LOAD"))  
			{
				if (*Arg[3])
				{
					SetCurView ( SetVPFromName (Arg[3],&Err));
					if (Err)
						goto RtnFalse;
				}
				CloseTheme();
				Truncate (Arg[2]);
				if (!*Arg[2])
				{
					SetCurView ( SaveVP);
					goto RtnTrue;
				} 
				Fid = GSSiOpenFile (Arg[2],0,OF_READ);
				if (Fid == HFILE_ERROR)
				{
					if (!_fstrchr(Arg[2],'\\'))
					{                
						HANDLE	hTemp=GSSiGlobAlloc ( 836,GMEM_MOVEABLE,256);
						LPSTR	Name=GlobalLock (hTemp);
						
						_fstrcpy (Name,"themes\\");
						_fstrcat (Name,Arg[2]); 
						if (!_fstrchr (Name,'.'))
							_fstrcat (Name,".thm");
						Fid = GSSiOpenFile (Name,0,OF_READ);  
						if (Fid == HFILE_ERROR) 
						{
							GSSiGlobUlFree (&hTemp);
							SetCurView ( SaveVP);
							goto RtnFalse;
						} 
						SetCurVal (Name,IDS_FILETHM);  
						GSSiGlobUlFree (&hTemp);
					} 
					else  
					{
						SetCurView ( SaveVP);
						goto RtnFalse;
					}
				}
				else
					SetCurVal (Arg[2],IDS_FILETHM);  
				ReadObject (&Fid,TRUE,&CurView->pTheme,0); 
	            CurView->pTheme->DisplayViewport = CurView->ID; 
	            CurView->pTheme->IsActive = TRUE;
				GSSiClose2 (&Fid);
				SetCurView ( SaveVP);
				goto RtnTrue;
			}
			else
			{
				SetCurView ( SaveVP);
				goto RtnFalse;
			}
			
		}
		
		case 502: /* $WHILE(statement,string) */
		{	 
			int		nRc; 
			HFILE	Fid;
			LPSTR	clause, WhileString;
			HANDLE	hMem1,hMem2;
			long	lmem1,lmem2;
			char	clause1;
			
			hMem = GSSiGlobAlloc ( 837,GMEM_MOVEABLE,USHRT_MAX);
			Arg2 = GlobalLock(hMem);
			if (!(ParLoc = MatchLev (Args,',')))
		   	    goto RtnFalse;
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			lmem2 = _fstrlen (Arg2);
			*ParLoc = 0;
			lmem1 = _fstrlen (Args);
NextWhile:  
			if (HaltReport)
				goto RtnFalse;
			hMem1 = GSSiGlobAlloc ( 838,GMEM_MOVEABLE,lmem1+4);
			clause = GlobalLock(hMem1); 
			_fstrcpy (clause,Args);  
			ExpandTextDB(clause,pBrkPt, bpOffset, bpLen);
			clause1 = clause[0];
			GlobalUnlock (hMem1);
			GSSiGlobUlFree (&hMem1);
			if (clause1 =='0')
				goto RtnTrue; 
			hMem2 = GSSiGlobAlloc ( 839,GMEM_MOVEABLE,4096);
			WhileString = GlobalLock(hMem2); 
			_fstrcpy (WhileString,Arg2);
			incDbOff = lmem2 + 1;
			ExpandTextDB(WhileString, pBrkPt, bpOffset+incDbOff, bpLen);
			GlobalUnlock (hMem2);
			GSSiGlobUlFree (&hMem2);			
			goto NextWhile;
		}
		
		case 503: /* $FETCH(fileid or handle) fetch next rec */
		{				
			int			irc=1;    
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
	        
	        if (IsInteger (Arg[1]))
	        {   
	        	HANDLE hList = (HANDLE)atol (Arg[1]);
	        	short	lVarName; 
	        	long	lList;
	        	LPSTR	pList, pVal, pNextVal;
	        	if (!hList)
	        		goto RtnFalse;
	        	pList = GlobalLock (hList);
	        	lVarName = _fstrlen (pList);
	        	pVal = pList + lVarName + 1;
	        	if (*pVal)
	        	{   
	        		short	lVal = _fstrlen (pVal);
	        		
	        		lList = GlobalSize (hList);
	        		lList -= lVarName + lVal + 2;
	        		SetGlobalValue (pList,pVal);
	        		pNextVal = pVal + lVal + 1; 
	        		hmemmove (pVal,pNextVal,lList);
	        	}
	        	else
	        		irc = 0;
	        	GlobalUnlock (hList);
	        	if (irc)
	        		goto RtnTrue;
	        	goto RtnFalse; 
	        }
	        if (!_fstricmp (Arg[1],"POLY"))
	        {   
				LPDPOINT	DPoint;

	        	if (!hPolyCoord || CurPolyCoord >= NumPolyCoord)
					goto RtnFalse;
				DPoint = GlobalLock (hPolyCoord);
				SetGlobalValueDPoint ("%POLYPOINT",DPoint[CurPolyCoord++]);
				GlobalUnlock (hPolyCoord);
        		goto RtnTrue;
	        }
	        if (!_fstricmp (Arg[1],"HLTLIST"))
	        {   
	        	if (*Arg[2])
					TORF = atob (Arg[2]);
				else
					TORF = TRUE;
	        	if (GetNextHLTListItem (FALSE,TORF)) //2010_12_12 flipped args for waypoint text extract
	        		goto RtnTrue;
	        	else
	        		goto RtnFalse; 
	        }
		    if (!_fstrnicmp (Arg[1],"THEME:",6))
		    {   
		    	short iview;
		    	
			    for (iview=0;iview<*pNumViewports;iview++)
			    {   
		        	if (!_fstricmp (&Arg[1][6],pViewports[iview]->Name)) 
		        	{
		        		if (pViewports[iview]->pTheme)
		        		{   
		        			LPTHEME SaveTheme = CurTheme;
		        			
		        			CurTheme = pViewports[iview]->pTheme;
		        			if (CurTheme->hHighlightFile)
		        			{    
								if (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_NEXT,BT_ANY,(LPSTR)&ThemeHighlightData))
								{
									SetGlobalValueLong ("%PICKED_REFNO",ThemeHighlightKey.Refno); 
								    SetGlobalValue ("%PICKED_PREFIX",ThemeHighlightData.Prefix);  
									SetGlobalValue ("%PICKED_UDI",ThemeHighlightData.UDI); 
							    	CurTheme = SaveTheme;
					    			goto RtnTrue; 
					    		}
					    	} 
					    	CurTheme = SaveTheme;
		        		}
		        	}
		        }
		        goto RtnFalse;
		    }
    
            if (!FilePathHandle)
            	goto RtnFalse;
			FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);  //sizeof(FILEPATH)
			lpFileHandle = &FilePathPtr->FileHandle;
            
			for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
			{
				if (*lpFileHandle)
				{
					SQLPtr = (LPOPENSQLDATA)GlobalLock (*lpFileHandle);
					if (!_fstricmp (SQLPtr->IDName,Arg[1]))
						goto GotFile;
					GlobalUnlock (*lpFileHandle);
				} 
			}
			GlobalUnlock (FilePathHandle);
            goto RtnFalse;
            
GotFile:
			if (!stricmp (Arg[2],"FIELD"))
			{
				int	FldNum = atoi (Arg[3]);

				LPFIELDINFO	lpFieldInfo;
				LPOPENFILEDATA	FilePtr;
	
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				lpFieldInfo = &FilePtr->FldInfo;
				if (FldNum < 1 || FldNum >  FilePtr->NumFields)
					irc = 0;
				else
				{
					char	str[128];
					LPSTR	pBeg, pEnd;

					irc = 1;
					strcpy (str,lpFieldInfo[FldNum-1].name);
					SetGlobalValue ("%FIELDNAME",str);
					*str = 0;
					CreateGMTextHeader (&lpFieldInfo[FldNum-1], str);
					pBeg = strrchr (str,'(');
					if (pBeg)
					{
						pBeg++;
						pEnd = strrchr (pBeg,')');
						*pEnd = 0;
					}
					else
						pBeg = str;
					SetGlobalValue ("%FIELDTYPE",pBeg);
				}
				GlobalUnlock (SQLPtr->OFHandle);
			}
			else
				irc = FetchDBRec (*lpFileHandle);
			GlobalUnlock (*lpFileHandle);
			GlobalUnlock (FilePathHandle);
			
			if (irc && ContinueProcessing)
				goto RtnTrue;
			else
				goto RtnFalse;
		}

		case 504: /* $TABLE(database,SQL,SelectMacro,title,SaveConfigVar,VPName-Top,ListOfVarToDisplay(opt)can contain alt headings in paren) creates grid control from database using SQL */ 
			{	
			int		l, len; 
			LPSTR	lpOut; 
			long	nrecs;
			LPSTR	pFile, pList;
			
			nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if ((pEnd = MatchLev (Arg[3],';')))
				*pEnd++ = 0;
			else
				pEnd = strchr (Arg[3],0);
			_fstrcpy (UGridPickMacro,Arg[3]); 
			_fstrcpy (UGridPickMacroDblClk,pEnd); 
			_fstrcpy (UGridTitle,Arg[4]); 
			if (*Arg[7])
			{
				UgridVarList = GSSiGlobAlloc (1595,GMEM_MOVEABLE,4096);
				pList = GlobalLock (UgridVarList);
				strcpy (pList,Arg[7]);
				GlobalUnlock (UgridVarList);
			}
	    	if (!hScreenFile)
	    	{
				LPSTR	pDot;

		    	hScreenFile = GSSiGlobAlloc ( 840,GMEM_MOVEABLE,256);
	    		pFile = GlobalLock (hScreenFile);
	        	GSSiGetTempFileName (0,"gm",0,pFile); 
				pDot = strrchr (pFile,'.');
				strcpy (pDot,".txt");
		    }
		    else
	    		pFile = GlobalLock (hScreenFile);
			if (OutputToFile (pFile,TRUE,Arg[1], Arg[2], 0,0,0,0,FALSE,TRUE,FALSE,FALSE,FALSE,0,0,0,TRUE)) 
			{
	    		GlobalUnlock (hScreenFile);  
        		ShowGrid(hWndMain,Arg[5],Arg[6]);
				GSSiGlobFree (&UgridVarList);
				goto RtnTrue;
			}
			else 
			{
	    		GlobalUnlock (hScreenFile);  
				GSSiGlobFree (&UgridVarList);
				goto RtnFalse;
			}
		} 
		
		case 507: // $CLOSE(fileid,delete(opt)def False)
		{				
			LPHANDLE	lpFileHandle2; 
			HANDLE	hSQL, hDeleteList=0;  
			BOOL	CloseAll=FALSE;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!nArgs || !*Arg[1])
			{
				EscapeFunction (FALSE);
				goto RtnTrue;
			}
	        if (!_fstricmp (Arg[1],"HLTLIST"))
	        {
				HltFetchPos = BT_FIRST;
	        	goto RtnTrue;  
	        }
	        if (!_fstricmp (Arg[1],"POLY"))
	        {
				GSSiGlobFree (&hPolyCoord);
	        	goto RtnTrue;  
	        }
		    if (!_fstrnicmp (Arg[1],"THEME:",6))
		    {   
		    	short iview;
		    	
			    for (iview=0;iview<*pNumViewports;iview++)
			    {   
		        	if (!_fstricmp (&Arg[1][6],pViewports[iview]->Name)) 
		        	{
		        		if (pViewports[iview]->pTheme)
		        		{   
		        			LPTHEME SaveTheme = CurTheme;
		        			
		        			CurTheme = pViewports[iview]->pTheme;
		        			if (CloseThemeHighlightFile ())
		        			{    
		        				CurTheme = SaveTheme;
					    		goto RtnTrue; 
					    	} 
					    	CurTheme = SaveTheme;
		        		}
		        	}
		        }
		        goto RtnFalse;
		    }
    
			hSQL = (HANDLE)atol (Arg[1]);
            if (!FilePathHandle)
            	goto RtnFalse;
	        
	        if (!_fstricmp (Arg[1],"ALL"))
	        	CloseAll = TRUE;
CloseNext:
			FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);  
			lpFileHandle = &FilePathPtr->FileHandle;
            
			for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
			{   
				if (hSQL && hSQL == *lpFileHandle)
					goto GotCloseFilehSQL;
				if (*lpFileHandle)
				{
					SQLPtr = (LPOPENSQLDATA)GlobalLock (*lpFileHandle);
					if (CloseAll || !_fstricmp (SQLPtr->IDName,Arg[1]))
						goto GotCloseFile;
					GlobalUnlock (*lpFileHandle);
				} 
			}
			GlobalUnlock (FilePathHandle);
            goto RtnFalse;
            
GotCloseFile:
			GlobalUnlock (*lpFileHandle);
			hSQL = *lpFileHandle;   
GotCloseFilehSQL:
			GlobalUnlock (FilePathHandle);
			if (!CloseAll && atob (Arg[2]))
				hDeleteList = GetFilesToClose (hSQL);
			CloseDataFile (FALSE, &hSQL);
			DeleteFilesInList (hDeleteList);
			GSSiGlobFree (&hDeleteList);
			if (CloseAll && FilePathHandle)
				goto CloseNext;
			goto RtnTrue;
		}

		case 508: // $SLEEP(Seconds)
		{				
			double	Seconds;
			time_t	MicroSecs;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Seconds = atof (Arg[1]);
			MicroSecs = Seconds*1000; 
            Wait (MicroSecs);
	
			goto RtnTrue;
		}

		case 509: // $RESET(Viewport Name,ALL)  resets the function stack - terminates active functions
		{				
    		
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs)
				SetCurView ( SetVPFromName (Arg[1],&Err));  
    		ResetFunStack (atob(Arg[2]));
			if (nArgs && CurView->StartupFunction)
	    	{                               
		       	AddLBUTTON = TRUE;
				AddGraphicsFunction (0, CurView->StartupFunction,0);        
			}
            SetCurView ( SaveVP);
			goto RtnTrue;
    	} 
    	
		case 510: // $MACRO(macro file pathname,arg1,arg2...arg10)
				  // $MACRO(pathname(type),arg1,arg2) runs multiple macros from directory pathname (not sure about this one)
				  // $MACRO() clears macro buffer
		{				
			HANDLE	hMacroArgs=0;
			LPSTR	LastArg, pMacroArg,pName,pSTR, pPAR, pEnd,pLoc;   
			short	NumArgs=0;
			
			if (!*Args)
			{
				CloseBufferedMacros();
				goto RtnTrue;
			}
			hMem = GSSiGlobAlloc ( 843,GMEM_MOVEABLE,4*2048);
			Arg1 = GlobalLock(hMem); 
			pName = Arg1 + 2048;
			pSTR = pName + 2048;   
			pOFStruct = (LPOFSTRUCTGM)(pSTR + 2048);
			if ((ParLoc = MatchLev (Args,',')))
			{
				*ParLoc++ = 0; 
				incDbOff = strlen(Args) + 1;
				hMacroArgs=GSSiGlobAlloc ( 844,GHND,4096L*MAX_MACRO_ARGS);
				pMacroArg = GlobalLock (hMacroArgs);
				while (ParLoc)
				{   
					NumArgs++;
					LastArg = ParLoc;
					if ((ParLoc = MatchLev (ParLoc,',')))
						*ParLoc++ = 0;
					_fstrcpy (pMacroArg,LastArg); 
					if (_fstricmp (Args,"INLINE"))
						ExpandTextDB(pMacroArg, pBrkPt, bpOffset+incDbOff, bpLen);
					incDbOff += strlen(LastArg) + 1;
					pMacroArg += 4096;
				}
				GlobalUnlock (hMacroArgs);
			}
			_fstrcpy (Arg1,Args);
			ExpandTextDB(Arg1, pBrkPt, bpOffset, bpLen);
	        
			if (!(pLoc = MatchLev (Arg1,'.')))
				pLoc = Arg1;
	        if (!(pPAR = _fstrchr (pLoc,'(')))
				ProcessMacroFile (Arg1,OutLoc,&hMacroArgs,NumArgs);
			else
			{   
				long	TotFiles=0;
				
				*pPAR++ = 0; 
				if ((pEnd = _fstrchr (pPAR,')')))
					*pEnd = 0;
				GSSiGetTempFileName(0,"gm",0,pName);
				Fid =	GSSiOpenFile (pName,pOFStruct,OF_CREATE);
				SearchFilesInDir (Arg1,0, Fid,&TotFiles,pPAR,1,TRUE,FALSE); 
				GSSillseek (Fid,0,0);    
				while (fgetstring (pSTR,256,Fid))
				{  
					LPSTR pTAB = strchr (pSTR,'\t');

					if (pTAB)
						*pTAB = 0;
					ProcessMacroFile (pSTR,OutLoc,&hMacroArgs,NumArgs);
				}
				GSSiClose2 (&Fid);			
				GSSiRemove (pName);
			}
			GSSiGlobFree (&hMacroArgs);
			goto Rtnl;
		}   
		 
		case 511: // $IMAGE(image file pathname)  
				  // $IMAGE(WINDOW,image file name)
				  // $IMAGE(SPLIT,imagefile,outdir,outtype,width,height,format(opt))
				  // $IMAGE(WIDTH,imageFile);
				  // $IMAGE(HEIGHT,imageFile);
				  // $IMAGE(CONVERT,infile,outfile,flag)
		{				
			
			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!nArgs)
			{
				if (hWndFullBM)
					PostMessage(hWndFullBM, WM_CLOSE, 0, 0L);

				goto RtnTrue;
			}


			if (!_fstricmp(Arg[1], "LISTTIFFTAGS"))
			{
				HDIB32	hDib32;
				BOOL	st;
				BITMAPINFOHEADER DibInfo = { 0 };
				double	ScaleX, ScaleY;
				DPOINT	BitmapPoint, WorldPoint;

				hDib32 = BMPHandleFromEXT(Arg[2]);
				if (!hDib32)
					goto RtnFalse;

				GetBitmapInfoFromHandle(&DibInfo, hDib32);
				st = GetGeoTiffData(hDib32, &ScaleX, &ScaleY, &BitmapPoint, &WorldPoint);
				GMDestroyDIB32(hDib32);
				goto RtnTrue;
			}
			if (!_fstricmp(Arg[1], "CONVERT"))
			{
				BOOL SaveBMPCache = AllowBMPCaching;
				BOOL SaveAllowCache = AllowCache;
				int flag = atoi(Arg[4]);
				AllowBMPCaching = FALSE;
				AllowCache = FALSE;
				HDIB32 hDIB = LoadDIB32(Arg[2], FALSE, 0);
				rtn = SaveDIB32(hDIB, Arg[3], -1, flag);
				DestroyDIB32(hDIB, FALSE);
				AllowBMPCaching = SaveBMPCache;
				AllowCache = SaveAllowCache;
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "SETICONCOLORS"))
			{
				COLORREF colors[2] = { RGB(255, 0, 0), RGB(0, 0, 0) };
				rtn = SetIconColors(Arg[2], colors, 2);
				goto Rtnrtn;
			}
			
			if (!_fstricmp(Arg[1], "SCREEN"))
			{
				_fstrcpy(FullBM, Arg[2]);
				rtn = ShowFullBM(TRUE, 0, 0);
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "TEMP"))
			{
				rtn = (BOOL)InitTempImageInstance(hWndMain, Arg[2], 0);
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "VIEW"))
			{
				rtn = (BOOL)ViewImage(hWndMain, Arg[2]);
				goto Rtnrtn;
			}

			if (!_fstricmp(Arg[1], "DUMP"))
			{
				int width = 0;
				int height = 0;
				HDIB32	hDib32 = BMPHandleFromEXT(Arg[2]);
				if (hDib32)
				{
					HFILE fidOut = GSSiOpenFile(Arg[3], 0, OF_CREATE);
					if (fidOut != HFILE_ERROR)
					{
						BITMAPINFOHEADER DibInfo;

						GetBitmapInfoFromHandle(&DibInfo, hDib32);

						width = FreeImage_GetWidth(hDib32);
						height = FreeImage_GetHeight(hDib32);
						LPSTR outLine = malloc(width * 14);
						float fval;
						int	  ival;
						for (int row = 0; row < height; row++)
						{
							RGBQUAD* p32Bit = (RGBQUAD*)FreeImage_GetScanLine(hDib32, row);
							*outLine = 0;
							for (int col = 0; col < width; col++)
							{
								fval = *(float*)p32Bit;
								ival = *(int*)p32Bit;
								RGBQUAD quad = *p32Bit++;
								sprintf(strchr(outLine,0), "%.2f ", fval);
							}
							fputstring(outLine, fidOut);
						}
						GSSiClose(fidOut);
						free(outLine);
					}
					DestroyDIB32(hDib32, FALSE);
				}
				goto RtnTrue;
			}
			if (!_fstricmp(Arg[1], "WIDTH"))
			{
				int width = 0;
				HDIB32	hDib32 = LoadDIB32(Arg[2], FALSE, 0);
				if (hDib32)
					width = FreeImage_GetWidth(hDib32);
				ltoa(width, OutLoc, 10);
				DestroyDIB32(hDib32, FALSE);
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1], "HEIGHT"))
			{
				int height = 0;
				HDIB32	hDib32 = LoadDIB32(Arg[2], FALSE, 0);
				if (hDib32)
					height = FreeImage_GetHeight(hDib32);
				ltoa(height, OutLoc, 10);
				DestroyDIB32(hDib32, FALSE);
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1], "BOUNDS"))
			{
				MNMXCORD BitmapBounds = { 0 };
				MNMXCORD WBounds = { 0 };
				*OutLoc = 0;
				if (GetImageBounds(Arg[2], 0, &BitmapBounds, &WBounds))
				{
					boundstoa(OutLoc, &BitmapBounds);
				}
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1], "WINDOW"))//$IMAGE(WINDOW,file,waitforkey,rect(opt))
			{
			    HDIB32	hDib32 = LoadDIB32(Arg[2],FALSE, 0);
				RECT	WindowRect;

 				rtn = FALSE;
				if (hDib32)
				{
					HDC	hDC = GetDC (hWndMain);
					HBITMAP	hBM=0;

					rtn = TRUE;
					SetDisplayMode (hDC, GF_SCREENMODE); 
					WindowRect = atorect(Arg[4], &Err);
					if (Err)
						GetWindowRect(hWndMain,&WindowRect);
					if (atob (Arg[3]))
					{
						hBM = SaveScreen (hDC, WindowRect);
					}
					DisplayBMInRect32 (hDC,hDib32,WindowRect,FALSE);
					DestroyDIB32(hDib32,FALSE);
					if (atob (Arg[3]))
					{
						WaitForKeystroke (TRUE);
						RestoreScreenRect (hDC, hBM, WindowRect, WindowRect);
					}
					GSSiDeleteObject (&hBM);
					ReleaseDC (hWndMain,hDC);
				}
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "SPLIT"))
			{
				rtn = SplitImage(Arg[2], Arg[3], Arg[4], atoi(Arg[5]), atoi(Arg[6]), Arg[7]);
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "SETTRANS"))
			{
				COLORREF fromColor = atol(Arg[3]);
				rtn = ConvertBitmapColorToTransparent(Arg[2], fromColor);
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "CONVERTCOLOR"))
			{
				MNMXCORD	rect = atobounds(Arg[3], &Err);

				if (!Err)
					rtn = ConvertBitmapColorsInRect(Arg[2], &rect, atoi(Arg[4]), atoi(Arg[5]), atob(Arg[6]));
				else
					rtn = -1;
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1], "CONVERTCOLORINRANGE"))//$IMAGE(CONVERTCOLORINRANGE,infile,outfile,fromcolor,tocolor,range,bounds(opt))
			{
				MNMXCORD	rect = atobounds(Arg[7], &Err);
				LPMNMXCORD pBounds=0;

				if (!Err)
					pBounds = &rect;
				rtn = ConvertBitmapColorsInRange(Arg[2], Arg[3], atoi(Arg[4]), atoi(Arg[5]), atof(Arg[6]),pBounds);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1], "VIEW"))
			{
				rtn = ViewImage (CurView->hWnd,Arg[2]);
				goto Rtnrtn;
			}
			if (!_fstricmp(Arg[1], "MULTIPLE"))
			{
				CallImageDisplayMultipleMsgProc(Arg[2],Arg[3]);
				rtn = TRUE;
				goto Rtnrtn;
			}

			if ((pPar = strrchr (Arg[1],'(')))
				*pPar = 0;
			if (!ExistFile (Arg[1]))
			{
				GSSiMessageBox (0,Arg[1],"Unable to Access Image File",MB_ICONEXCLAMATION,0);
				goto RtnFalse;
			} 
			else
			{   
				BOOL SaveBMPCache = AllowBMPCaching;
				BOOL SaveAllowCache = AllowCache;
				int rotate = atoi (Arg[2]);
				int left = atoi (Arg[3]);


				if (pPar)
					*pPar = '(';
				AllowBMPCaching = FALSE;
				AllowCache = FALSE;
				if (InInfoBox || Printing)
				{   
					sprintf (OutLoc,"$IMAGE(%s)",Arg[1]); 
					AllowBMPCaching = SaveBMPCache;
					goto Rtnl;
				}
				_fstrcpy (FullBM,Arg[1]);
				SetImageFileRotation (rotate);
				ShowFullBM(TRUE,rotate,left); 
				SetImageFileRotation (0);
				AllowBMPCaching = SaveBMPCache;
				AllowCache = SaveAllowCache;
		
				goto RtnTrue; 
			}
		}   

		case 512: // $AFTER(string,str,optional l(last) or f(default)) - retrn portion of string after str
		{
			LPSTR	lpChr, LastLoc; 
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			_fstrlwr (Arg[3]);
			if (*Arg[2] == '\'' && Arg[2][2] == '\'' && Arg[2][1] != 0) 
			{
				*Arg[2] = Arg[2][1];
				Arg[2][1] = 0;
			}
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			l = _fstrlen (Arg[2]);
			if (!l)
				goto RtnFalse;
			if (*Arg[3] == 'l')
			{   
				lpChr = 0;
				do
				{
					LastLoc = lpChr;
					lpChr = _fstrstr (Arg[1],Arg[2]);  
					if (lpChr)
						Arg[1] = lpChr + 1;
				}
				while (lpChr); 
				lpChr = LastLoc;
			}
			else
				lpChr = _fstrstr (Arg[1],Arg[2]);
			if (lpChr && *lpChr)
			{  
				lpChr += l;
				_fstrcpy (OutLoc,lpChr);
			} 
			else
				*OutLoc = 0;  
			goto Rtnl;
		}
		
		case 513: //$PRINT(showprintdialog,pagenum,totpage)
		{	
			long	CurPage,TotPage; 
			HWND	hwnd;    
			
			SaveVP=CurView;  
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			CurPage = IDNINT(atof(Arg[2]));
			TotPage = IDNINT (atof(Arg[3]));
			if (!atob(Arg[1]))
				hwnd = 0;
			else
				hwnd = hWndMain;
            rtn = PrintMap (hwnd,CurPage,TotPage);
            SetCurView ( SaveVP);
            if (rtn)
   				goto RtnTrue;
			else
				goto RtnFalse;
		} 
		
		case 514: // $ALERT(message,wave file) 
		{	double dist;
			COLORREF	Color[2]={RGB(255,0,0),RGB(255,255,0)};
			short	cycle=0;
			HANDLE	hSaveScreen=0; 
			long	ScreenID; 
			LPHANDLE	phSaveScreen=&hSaveScreen;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			GetClientRect(CurView->hWnd, &Rect);
			Point16 = RectMid (&Rect);
			Point = WinPtToBasePt (Point16); 
			SetWindowText (hWndMain,Arg[1]);
			sndPlaySound(Arg[2],SND_ASYNC|SND_LOOP|SND_NODEFAULT); 
			do 
			{   
				if (hSaveScreen)
					phSaveScreen = NULL;
				DisplayMarker (Point,500,Arg[1],1.0,0,Color[cycle%2],FALSE,FALSE,phSaveScreen,&ScreenID,0,0,0); 
				Sleep (1000); 
				cycle++;
			}
			while (!WaitForKeystroke (FALSE));
			RestoreScreen2 (CurView->hDC, hSaveScreen,ScreenID,FALSE);
		    DestroySavedScreen (&hSaveScreen,ScreenID);
			sndPlaySound(0,SND_ASYNC);			
			goto RtnTrue;
		}
		
		case 515: // $GFKEY(key) executes gf key command in command vp
		{				
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			PostMessage(hWndMain, WM_COMMAND, IDM_GFKEY, *Arg[1]);
			goto RtnTrue;
		}   

		case 516: // $ONERR(string)
		{				
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			SetErrorProcessing(Arg[1]);
			goto RtnTrue;
		}   
		case 517: // $ABORT()  resets the function stack - exits current command
		{				
    		
    		ResetFunStack (FALSE);
			goto RtnFalse;
    	} 
    	
		case 518: // $GFCMD(VPName,file,loc,TFruncommand(opt-def=T)) or
				  // $GFCMD(VPName,gfcmd)
				  // $GFCMD(gfcmd) runs in current vp
		{	 
			short	CmdArg = 2;
			short	Inc=0;  
			
			nArgs = GetFunArgs (Args,Arg,-5,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs > 1)
			{ 
				ExpandText (Arg[1]);
				if (CurrentConfig)
					Inc = 1000;
				if (!(n = GetVPIDFromName (Arg[1])))
					goto RtnFalse;  
			}
			switch (nArgs) 
			{  
				 
				case 1:
					CmdArg = 1; 
					n = CurView->ID;
				case 2:
				{   
					LPSTR	pGCmd; 
					long	lcmd = _fstrlen (Arg[CmdArg]);
					
					if (n < 1000)
						n += Inc;			
				   	GSSiGlobFree (&hAddGraphicsFun2);
			   		AddGraphicsFunVP = n;
					hAddGraphicsFun2 = GSSiGlobAlloc ( 851,GHND,lcmd+1);
					pGCmd = GlobalLock (hAddGraphicsFun2); 
					_fstrcpy (pGCmd,Arg[CmdArg]);
					GlobalUnlock (hAddGraphicsFun2);  
					PostMessage(hWndMain, GSSI_ADDGF, 0, n);
					goto RtnTrue; 
				}
				default:
					ExpandText (Arg[2]);
					ExpandText (Arg[3]);
					ExpandText (Arg[4]);
					ExpandText (Arg[4]);
					if (nArgs > 3)
						rtn = atob (Arg[4]);
					else
						rtn = TRUE;	
					SetViewport (n); 
					SetGlobalValue ("%PICKED_VIEWPORT",CurView->Name); 
					nlong = atol (Arg[3]); 
					n = atoi (Arg[5]);
					rtn = RunGFCommandFromFileAtLoc (Arg[2],nlong,rtn,n);
					SetCurView ( SaveVP);  
					if (rtn)
						goto RtnTrue;
					goto RtnFalse;
			}
		}
		
		case 519: /* $UNHLT(ALL) clears highlight list*/
				  // $UNHLT(ITEM,Refno or PREFIX:UDI); 
		{	   
			HANDLE	hHlt = GSSiGlobAlloc ( 852,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA));
			LPHIGHLIGHTDATA	pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hHlt);
			long	StartRef;

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstrcmp(Arg[1], "ALL"))
			{   
				StartRef=LONG_MIN;
				
				while (!BT_FIND (hHighlight,(LPSTR)&StartRef,BT_FIRST,BT_GT,(LPSTR)pHighlightData))
				{
					PickList[0]=pHighlightData->PD;
				    RemoveFromHighlightList (StartRef,2);
			    	ShowPickedItem (CurView->hWnd,0); 
				    RemoveFromHighlightList (StartRef,0);
			    	ShowPickedItem (CurView->hWnd,0); 
				}
				ClearHighlightList(FALSE);
				GSSiGlobUlFree (&hHlt); 
				goto RtnTrue;
			} 
			if (!_fstricmp(Arg[1],"ITEM"))
			{
				ExpandText (Arg[2]); 
				if ((lpColon = _fstrchr (Arg[2],':'))) 
				{
					StartRef=LONG_MIN;
		
					*lpColon++=0;
				
					while (!BT_FIND (hHighlight,(LPSTR)&StartRef,BT_FIRST,BT_GT,(LPSTR)pHighlightData))
					{   
						if (!_fstricmp (pHighlightData->PD.Prefix,Arg[2]) && !_fstricmp (pHighlightData->PD.UDI,lpColon))
						{
			HLTRemove:
							PickList[0]=pHighlightData->PD;
						    RemoveFromHighlightList (StartRef,2);
					    	ShowPickedItem (CurView->hWnd,0); 
						    RemoveFromHighlightList (StartRef,0);
					    	ShowPickedItem (CurView->hWnd,0);
					    	GSSiGlobUlFree (&hHlt);
					    	goto RtnTrue;
					    } 
					}
			    	GSSiGlobUlFree (&hHlt);
					goto RtnFalse;
				}
				else 
				{
					Refno = atol (Arg[2]); 
					if (!BT_FIND(hHighlight, (LPSTR)&Refno, BT_FIRST, BT_EQ, (LPSTR)pHighlightData))
					{
						RemoveFromHighlightList(Refno, 1);
						GSSiGlobUlFree(&hHlt);
						goto RtnTrue;
					}
				}
			}
	    	GSSiGlobUlFree (&hHlt);
			goto RtnFalse; 
		} 
		
		case 520: //$BTREE(REORG,name)
		
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			if (!_fstricmp (Arg[1],"REORG"))
			{
				CreateStatusWind (CurView->hWnd,1,Arg[2]);
				rtn = ReorgBTree (Arg[2], GetDlgItem(PrintMsgWnd,PRINT_VIEW));
				DestroyStatusWindow(0);
				if (rtn) 
					goto RtnTrue;
			}
			goto RtnFalse;
		
		case 521: //$SUBDL(pathname) substitues current [%DL] value with @[%DL]
		
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			_fstrcpy (OutLoc,Arg[1]);
			SubstituteDL (OutLoc,TRUE);
			goto Rtnl;
		
		case 522: //$STRIP(string,char(opt-default is space,leading only)  
		{
			char	StripChr=' ';
			int	ln=2048;

			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse; 
			_fstrcpy (OutLoc,Arg[1]);
			if (atob(Arg[3]))
			{
				char c = *Arg[2];
				LPSTR pResult = Arg[1];
				while (*pResult && *pResult == c)
					pResult++;
				strcpy(OutLoc, pResult);
			}
			else
			{

				if (nArgs == 2)
					STRIPR(OutLoc, &ln, Arg[2], strlen(Arg[2]));
				else
					Strip(OutLoc, StripChr);
			}
			goto Rtnl;
		}
		case 523: //$ISHLT(refno) return highlight status
		
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			Refno = atol (Arg[1]);
			if (BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData))
				goto RtnFalse;
			goto RtnTrue;
		
		case 524: //$BASIC(file,sql,updatefieldlist,title,autoupdate,sqlfieldlist,displayrect,withphotos,prompt)
		{
			BOOL haveRectGlobal = FALSE;
			nArgs = GetFunArgs (Args,Arg,10,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			lpDB = Arg[1];
			lpSQL = Arg[2];
			lpUpdateFieldList = Arg[3];
			lpBasicTitle = Arg[4];
			lpBasicPrompt = Arg[9];
			lpAutoUpdateFieldList = 0;
			lpSQLFieldList = 0;
			if (*Arg[5])
				lpAutoUpdateFieldList = Arg[5];
			if (*Arg[6])
				lpSQLFieldList = Arg[6];
			displayRect = atorect (Arg[7],&Err);
			if (Err && *Arg[7])
			{
				displayRect = GetGlobalRectVal(Arg[7], 0);
				haveRectGlobal = TRUE;
			}
			showOnlyData = atob(Arg[8]);
			if (!atob (Arg[8]))
			{
				showOnlyData = TRUE;
				BasicDisplayItem = -1;
				nRc = DialogBox(hInst, (LPSTR)"IDENTIFY", hWndMain, (DLGPROC)IDENTIFYMsgProc);

				skipPaint = 1;
				if (haveRectGlobal)
					SetGlobalValueRect(Arg[7], displayRect);
			}
			else
			{

				BasicDisplayItem = -1;
				rampPhotoFile = Arg[9];
				nRc = DialogBox(hInst, (LPSTR)"IDENTIFY_WITH_PHOTO", hWndMain, (DLGPROC)IDENTIFY_WITH_PHOTOMsgProc);

				skipPaint = 1;
				if (haveRectGlobal)
				{
					char txt[128];
					recttoa(txt, displayRect);
					SetGlobalValueRect(Arg[7], displayRect);
					WritePrivateProfileString("User", "IDWithPhotoWindowPos", txt, GMIni);
				}
			}
			lpUpdateFieldList = 0;
			lpAutoUpdateFieldList = 0;
			lpSQLFieldList = 0;
			lpBasicTitle = 0;
			itoa(nRc, OutLoc, 10);
		    goto Rtnl; 
		}
		case 525: //$VOTER(ADD)
		{
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;  
			if (!_fstricmp (Arg[1],"ADD"))
			{
				if (AddNewVoters (hWndMain))
			    	goto RtnTrue;
			} 
			else if (!_fstricmp (Arg[1],"EDIT"))
			{   
				nlong = atol (Arg[2]);
				i = atoi (Arg[3]);
				
				if (EditVoter (hWndMain,nlong,i))
					goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"DEFINE"))
			{   
				
				if (DefineVoter (hWndMain))
					goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"FIND"))
			{
				if (!_fstricmp (Arg[2],"NAME"))
				{
		            DLGPROC lpfnVOTER_NAME_LOCMsgProc;
					int		nRc;
					
					if (nArgs > 2)
						InitVoterNameSearch (Arg[3],Arg[4],Arg[5],Arg[6]);
					else
						InitVoterNameSearch (0,0,0,0);
		            lpfnVOTER_NAME_LOCMsgProc = MakeProcInstance((DLGPROC)VOTER_NAME_LOCMsgProc, hInst);
		            nRc = DialogBox(hInst, (LPSTR)"VOTER_NAME_LOC", CurView->hWnd, lpfnVOTER_NAME_LOCMsgProc);
		            FreeProcInstance(lpfnVOTER_NAME_LOCMsgProc); 
					itoa (nRc,OutLoc,10);
					goto Rtnl;  
				}
			}
		    goto RtnFalse;
		}
		
		case 526: //$MUNIC(NAME,num)
				  //$MUNIC(ABV,num)
				  //$MUNIC(NUMBER,name)
		
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse; 
			if (!_fstricmp (Arg[1],"NUMBER"))
			{
		    	nlong = GetMunicFromName (Arg[2]);
		    	ltoa (nlong,OutLoc,10);
		    	goto Rtnl;
		    }
			if (!_fstricmp (Arg[1],"NAME"))
			{
		    	nlong = atol (Arg[2]); 
		    	if (!GetMunicName (nlong,OutLoc,0))
		    		goto RtnFalse;
		    	goto Rtnl;
		    }
			if (!_fstricmp (Arg[1],"ABV"))
			{
		    	nlong = atol (Arg[2]); 
		    	if (!GetMunicName (nlong,0,OutLoc))
		    		goto RtnFalse;
		    	goto Rtnl;
		    }  
			if (!_fstricmp (Arg[1],"TABLE"))
			{
		    	if (!_fstricmp (Arg[2],"LOAD"))
		    	{
			    	if (ReloadMunicNameTable (Arg[3]))
			    		goto RtnTrue;  
			    }
		    }  

			goto RtnFalse;

		case 527: //$LOGIN(Title,uservarname,pwvarname)

		{
			DLGPROC lpfnDBLOGINMsgProc;
			int	nRc;
			char	var[64],val[128];

			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			strcpy (LoginTitle,Arg[1]);
			sprintf (var,"[%s]",Arg[2]);
			GetGlobalCVal (var,val,0);
			strcpy (LoginUserID,val);
			sprintf (var,"[%s]",Arg[3]);
			GetGlobalCVal (var,val,0);
			strcpy (LoginPassword,val);
			lpfnDBLOGINMsgProc = MakeProcInstance((DLGPROC)LOGINMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"DBLOGIN", hWndMain, lpfnDBLOGINMsgProc);
			FreeProcInstance(lpfnDBLOGINMsgProc);
			if (nRc)
			{
				SetGlobalValue (Arg[2],LoginUserID);
				SetGlobalValue (Arg[3],LoginPassword);
	     		goto RtnTrue; 
			}
			goto RtnFalse;
		} 

		case 528: //$TIMER(START,id,seconds,macro)
				  //$TIMER(STOP,id)

		{
			int	id;
			
			nArgs = GetFunArgs (Args,Arg,-4,&hMem, pBrkPt, bpOffset, bpLen); 
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			ExpandText (Arg[3]);
			id = atoi (Arg[2]);
			if (!stricmp (Arg[1],"START"))
			{
				int ms = atof (Arg[3]) * 1000;

				SetUserTimer (id,ms,Arg[4]);
			}
			else if (!stricmp (Arg[1],"RESET"))
			{
				int ms = atof (Arg[3]) * 1000;
				ResetUserTimer (id,ms);
			}
			else if (!stricmp (Arg[1],"STOP"))
			{
				KillUserTimer (id);
			}
			else
				goto RtnFalse;
			goto RtnTrue;
		}

		case 529: //$FENCE(TEST,handle,point,LastDist(Opt))
				  //$FENCE(LOAD,type(POLYGON,CIRCLE or ROUTE),TAG,Offset,Point)

		{
			int	id;
			
			nArgs = GetFunArgs (Args,Arg,9,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!stricmp (Arg[1],"CREATE"))
			{
				rtn = DialogBox (hInst, (LPSTR)"GEOFENCE", hWndMain, (DLGPROC)GEOFENCEMsgProc);
				goto Rtnrtn; 
			}
			else if (!stricmp (Arg[1],"DECOMPRESS"))
			{
				HFILE	Fid2,Fid=GSSiOpenFile (Arg[2],0,OF_READ);

				if (Fid != HFILE_ERROR)
				{
					LPSTR pDot = strrchr (Arg[2],'.');

					if (pDot && !stricmp (pDot,".gfc"))
					{
						int	   lFileDCmp, lFileCmp = GSSifilelength (Fid);
						HANDLE hFileCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lFileCmp);
						LPBYTE pFileCmp = GlobalLock (hFileCmp);
						HANDLE hFileDCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,1024L*1024L*4);
						LPBYTE pFileDCmp = GlobalLock (hFileDCmp);

						BigRead (Fid,pFileCmp,lFileCmp);

						lFileDCmp = DecompressBinaryRecordUnsafe (pFileDCmp,pFileCmp,lFileCmp);
						strcpy (pDot,".gfb");
						Fid2 = GSSiOpenFile (Arg[2],0,OF_CREATE);
						BigWrite (Fid2,pFileDCmp,lFileDCmp,-1);
						GSSiClose2 (&Fid2);
						GSSiGlobUlFree (&hFileCmp);
						GSSiGlobUlFree (&hFileDCmp);
						rtn = TRUE;
					}
					GSSiClose2 (&Fid);
				}
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"TEST"))
			{
				HANDLE	handle=(HANDLE)atoi (Arg[2]);

				Point = atopt (Arg[3],&Err);
				if (Err)
					rtn = 0;
				else
					rtn = PointInFence (handle,&Point,atoi(Arg[4]));
				itoa (rtn,OutLoc,10);
				goto Rtnl; 
			}
			else if (!stricmp (Arg[1],"POINT"))
			{
				HANDLE	handle=(HANDLE)atoi (Arg[2]);
				double	dist = atof (Arg[3]);

				Point = PointOnFence (handle,&dist);
				dpointtoa (OutLoc,&Point);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"LOAD"))
			{
				HANDLE	handle=0;

				Point = atopt (Arg[6],&Err);
				handle = LoadFence (Arg[2],Arg[3],&Point,atof(Arg[4]),*Arg[5]);
				itoa ((int)handle,OutLoc,10);
				goto Rtnl; 
			}
			else if (!stricmp (Arg[1],"GETLIST"))
			{
				rtn = GetFenceList (Arg[2],Arg[3]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"GETDEFS"))
			{
				rtn = GetFenceDefs (Arg[2],Arg[3]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"GETLINKS"))
			{
				rtn = GetFenceLinks (Arg[2],Arg[3]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"GETSTOPS"))
			{
				rtn = GetFenceStops (Arg[2],Arg[3]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"ADDLIST"))
			{
				rtn = AddFenceToList (Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],Arg[7],*Arg[8],atob(Arg[9]));
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"REMOVE"))
			{
				rtn = RemoveFenceFromList (Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"DELETE"))
			{
				rtn = DeleteFence (Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"ACTIVATE"))
			{
				rtn = ActivateFence (Arg[2],Arg[3],TRUE);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"DEACTIVATE"))
			{
				rtn = ActivateFence (Arg[2],Arg[3],FALSE);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"CHECK"))
			{
				Point.x = atof (Arg[5]);
				Point.y = atof (Arg[6]);
				rtn = CheckFence (Arg[2],Arg[3],Arg[4],Point.x,Point.y);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"IMPORT"))//$FENCE(IMPORT,IPAddress,Port,Account
			{
				rtn = ImportFences (Arg[2],Arg[3],Arg[4]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"SORT"))//$FENCE(SORT,filelist path
			{
				rtn = OrderFenceFilelist (Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"EXPORT"))//$FENCE(EXPORT,IPAddress,Port,Account
			{
				rtn = ExportFences (Arg[2],Arg[3],Arg[4]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"UPLOAD"))//$FENCE(UPLOAD,Account,FenceName,Type,Offset,Units,Critical,First
			{
				rtn = UploadFence (Arg[2],Arg[3],Arg[4]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"UPLINKS"))//$FENCE(UPLINK,Account)
			{
				rtn = UploadFenceLinks (Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"UPSTOPS"))//$FENCE(UPLINK,Account)
			{
				rtn = UploadRouteStops (Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"TESTGRID"))//$FENCE(TESTGRID,lat,lon)
			{
				double lat = atof (Arg[2]);
				double lon = atof (Arg[3]);

				rtn = AmInGeofence (lat,lon);
				itoa (rtn,OutLoc,10);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"GETCOORD"))//$FENCE(GETCOORD,Account,Name)
			{
				rtn = GetFenceCoord (Arg[2],Arg[3]);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"BACKUP"))//$FENCE(BACKUP,Account,FilePathName)
			{
				rtn = BackupFences (Arg[2],Arg[3]);
				CmdMess = GlobalLock (hCmdMess);
				if (rtn)
					sprintf (strchr(CmdMess,0),">FenceBackup:OK;\r\n");
				else
					sprintf (strchr(CmdMess,0),">FenceBackup:ERROR;\r\n");
				GlobalUnlock (hCmdMess);
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"RESTORE"))//$FENCE(RESTORE,Account,FilePathName)
			{
				rtn = RestoreFences (Arg[2],Arg[3]);
				CmdMess = GlobalLock (hCmdMess);
				if (rtn)
					sprintf (strchr(CmdMess,0),">FenceRestore:OK;\r\n");
				else
					sprintf (strchr(CmdMess,0),">FenceRestore:ERROR;\r\n");
				GlobalUnlock (hCmdMess);
				goto Rtnrtn;
			}

			else
				goto RtnFalse;
			goto RtnTrue;
		}

		case 530: //$CLEAR(IMAGEBUFFER)
		{
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			AddBMPToCache32 (0,0, 0);
			goto RtnTrue;
		}

		case 531: //$REFNO(FINDDUPS,Outfile)
				  //SREFNO(SKIP,refno,vpid(opt));
		{
		    BTVARDESC   BTVar[2]; 
			char	SymName[66];
			HANDLE	hDB=0;
			long	Refno;
			short	Dummy;
			short	st;
			long	NumDup,NumDup2;
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 

			if (!stricmp (Arg[1],"SKIP"))
			{
				LPSKIPREF	pSkipRef;

				if (nArgs < 2)
				{
					GSSiGlobFree (&hSkipRefs);
					nSkipRefs = 0;
					goto RtnTrue;
				}
				if (nSkipRefs)
					hSkipRefs = GSSiGlobalReAlloc (1684,hSkipRefs,(nSkipRefs+1)*sizeof(SKIPREF),GMEM_MOVEABLE);
				else
					hSkipRefs = GSSiGlobAlloc (1684,GMEM_MOVEABLE,sizeof(SKIPREF));
				pSkipRef = GlobalLock (hSkipRefs);
				pSkipRef[nSkipRefs].VPID = atoi (Arg[3]);
				pSkipRef[nSkipRefs++].Refno = atoi (Arg[2]);
				GlobalUnlock (hSkipRefs);
			}
			else if (!stricmp (Arg[1],"ALLOCATE"))
			{
				long	StartRefno = atoi (Arg[2]);

				Refno = GetNextRefno (&StartRefno,TRUE,TRUE);
				itoa (Refno,OutLoc,10);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"FINDDUPS"))
			{
				if (nArgs < 2)
					goto RtnFalse;
				BTVar[0].BT_VARTYP=BT_INTEGER;
				BTVar[0].BT_VARLEN=4;
				BTVar[0].BT_VAROFF=0;
				BTVar[1].BT_VARTYP=BT_INTEGER;
				BTVar[1].BT_VARLEN=4;
				BTVar[1].BT_VAROFF=4;
				GSSiGetTempFileName (0,"gm",0,Arg[6]); 
				BT_CREATE (Arg[6], sizeof(DUPREFDATA), FALSE, 2, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
				hDupRef = BT_OPEN (Arg[6], 0, BT_WRITE, 0);
				GSSiGetTempFileName (0,"gm",0,Arg[6]); 
				BT_CREATE (Arg[6], 2, FALSE, 1, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
				hDupRef2 = BT_OPEN (Arg[6], 0, BT_WRITE, 0);

				SetViewport (*pCommandViewport);
				FastMapCopynRecs = 0;
	//			GetWindowText (CurView->hWnd,Arg[5],255);
				RedisplayViewport (TRUE,TRUE);
	//			SetWindowText (CurView->hWnd,Arg[5]);
				SetViewport (*pCommandViewport);
				SetSysMess ("Creating output file");
				SetPrompt2 (0,PRMT_SYSMESS);
				NumDup = BT_NUM_IN_INDEX (hDupRef);
				NumDup2 = BT_NUM_IN_INDEX (hDupRef2);
	//			sprintf (Arg[6],"%i - %i duplicate references found",NumDup,NumDup2);
	//			MessageBox (0,Arg[6],"",MB_OK);
				Fid = GSSiOpenFile (Arg[2],0,OF_CREATE);
				st = BT_FIND (hDupRef2,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&Dummy);
	//			sprintf (Arg[6],"st = %i,Refno = %i,hDupRef = %i,hDupRef2 = %i",st,Refno,(int)hDupRef,(int)hDupRef2);
	//			MessageBox (0,Arg[6],"",MB_OK);
				
				while (!st)
				{
					DUPREFKEY	DupRefKey;
					DUPREFDATA	DupRefDat;
					short		st2;

					DupRefKey.Refno = Refno;
					DupRefKey.dupnum = 1; 
					st2 = BT_FIND (hDupRef,(LPSTR)&DupRefKey,BT_FIRST,BT_EQ,(LPSTR)&DupRefDat);
					while (!st2 && DupRefKey.Refno == Refno)
					{
						PickList[0].FileNum = DupRefDat.FileNum;
						PickList[0].SubFile = DupRefDat.SubFile;
						PickList[0].FileInIndex = DupRefDat.FileInIndex;
						GetDictSymName (DupRefDat.Desc,SymName);
						GetPickName (0);
						sprintf (Arg[4],"%ld\t%s\t%s",Refno,SymName,PickName);
						fputstring (Arg[4],Fid);
						st2 = BT_FIND (hDupRef,(LPSTR)&DupRefKey,BT_NEXT,BT_ANY,(LPSTR)&DupRefDat);
					}
					st = BT_FIND (hDupRef2,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&Dummy);
				}
				GSSiClose2 (&Fid);
				BT_CLOSEANDDELETE (&hDupRef);
				BT_CLOSEANDDELETE (&hDupRef2);
				goto RtnTrue;
			}
			goto RtnFalse;
		}

		case 532: //$POINT(DISPLAY,id or coord,symbol,size,color,text,viewport)
		{
			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (PointCommands (nArgs,Arg,OutLoc))
				goto Rtnl;
			goto RtnFalse;
		}

		case 533: //$CRASH(int type)
		{
			double x=10,y=0;

			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			switch (atoi (Arg[2]))
			{
			case 1:
				x = x/y;
				break;
			case 2:
				strcpy (pViewports[100]->Name,"CrashThisSucker");
				break;
			}

			goto RtnTrue;
		}

		case 534: //$DEBUG(string)

			ExpandText (Args);
			MessageBox (0,Args,0,MB_OK);
			goto RtnTrue;
		
		case 535: //$CACHE(SERVER
			nArgs = GetFunArgs (Args,Arg,-6,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (CacheCommands (nArgs,Arg,OutLoc))
				goto Rtnl;
			goto RtnFalse;

		case 536: //ABEND(SET,USER,DLDIR)
				  //ABEND(FORCE)
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp (Arg[1],"SET"))
				AddVectoredHandlers (Arg[2],Arg[3]);
			else if (!stricmp (Arg[1],"SAVEDIR"))
				strcpy (SaveLocalConfigNetDir,Arg[2]);
			else if (!stricmp (Arg[1],"FORCE"))
				IllegalInst();
			goto RtnTrue;
		
		case 537: //$UNLIT(value)
		{
			int ln;
			HANDLE hStr = GSSiGlobAlloc (1776,GMEM_MOVEABLE,4096);
			LPSTR  pStr = GlobalLock (hStr);

			ExpandText (Args);
			ln = strlen (Args);
			strcpy (pStr,Args);
			if (!strnicmp (Args,"$LIT(",5) && *LastChr (Args) == ')')
				strncpy0 (OutLoc,&Args[5],ln-6);
			else
				strcpy (OutLoc,Args);
			GSSiGlobUlFree (&hStr);
			ExpandText (OutLoc);
			goto Rtnl;
		}

		case 538: //$PLIST(CREATE,name,pointlist)
				  //$PLIST(DESTROY,name)
				  //$PLIST(ADD,name,pointlist)
				  //$PLIST(THIN,name,dist(if 0 removes dup points))
				  //$PLIST(DISPLAY,name,FILL,color)
				  //$PLIST(DISPLAY,name,DRAW,color,width)
				  //$PLIST(LENGTH,name)
				  //$PLIST(AREA,name)
				  //$PLIST(AZM,name,pct,before;after;at(default)) at averages before and after if at node point
				  //$PLIST(INTERSECT,name,name2,COUNT;id;Farthest;nearest,farornearpoint)
		{
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (PointListCommands(nArgs, Arg, OutLoc))
				goto Rtnl;
			goto RtnFalse;
		}
		case 539: //$ISINT(val,testforlongint(opt))
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (atob(Arg[2]))
			{
				if (IsLongInteger(Arg[1]))
					goto RtnTrue;
			}
			else
			{
				if (IsInteger(Arg[1]))
					goto RtnTrue;
			}
			goto RtnFalse;
		}

		case 540: //$ISFLT(val)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (IsReal(Arg[1]))
				goto RtnTrue;
			goto RtnFalse;
		}
		case 541: //$ORTHO(CONVERT,JPEG,year,nparts)
			      //$ORTHO(SETVERSION,indexfile,versionnum)
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "CONVERT"))
			{
				if (nArgs < 4)
					goto RtnFalse;
				int nparts = atoi(Arg[4]);
				if (ConvertToJP2(Arg[3], nparts))
					goto RtnTrue;
			}
			else if (!stricmp(Arg[1], "SETVERSION"))
			{
				HFILE FidIndex = GSSiOpenFile(Arg[2], 0, OF_READWRITE);
				if (FidIndex != HFILE_ERROR)
				{
					long EndOffset = GSSillseek(FidIndex, (LONG)-(6), 2);
					int Signature;
					short Version;
					BOOL rtn = FALSE;
					BigRead(FidIndex, (HPSTR)&Signature, 4);
					BigRead(FidIndex, (HPSTR)&Version, 2);
					if (Signature == 80251)
					{
						int off = GSSillseek(FidIndex, EndOffset, 0);
						BigWrite(FidIndex, &Signature, 4,-1);
						Version = atoi(Arg[3]);
						BigWrite(FidIndex, &Version, 2,-1);
						rtn = TRUE;
					}
					GSSiClose (FidIndex);
					if (rtn)
						goto RtnTrue;
				}
			}
			goto RtnFalse;
		}
		case 542: //$VALUE(fieldname,file,sql)
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs == 3)
			{
				HANDLE hSQL = 0;
				char dataFile[MAX_PATH + 16];
				char varName[256];
				char value[1024];
				sprintf(dataFile, "%%VFILE=%s", Arg[2]);
				if (OpenDataFile(dataFile, Arg[3], BT_READ, &hSQL))
				{
					sprintf(varName, "%%VFILE.%s", Arg[1]);
					if (GetValFromOpenFiles(varName, value, 1024))
					{
						strcpy(OutLoc, value);
					}
					CloseDataFile(FALSE, &hSQL);
				}
			}
			goto Rtnl;
		}
		case 601: /* $TAGLOC(Prefix,minchar,SaveGlobalName(optional-not in brackets),Title(opt),Viewport(opt),Layer(opt),locatetagonly(opt,T locates,)) Tag locator */
		{	 
            DLGPROC lpfnTAGLOCMsgProc;
			int		nRc;
			
			AddToView = FALSE;
			nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (*Arg[3])
				TagLochSaveGlobal = AllocateVar(Arg[3]);
			else
				TagLochSaveGlobal = 0;
        	_fstrcpy(TagLocPrefix,Arg[1]);
        	TagLocMinChar = atol(Arg[2]);
            TagLocTitle = Arg[4]; 
            _fstrcpy (TagLocViewport,Arg[5]);
            _fstrcpy (TagLocLayer,Arg[6]);
              lpfnTAGLOCMsgProc = MakeProcInstance((DLGPROC)TAGLOCMsgProc, hInst); 
              nRc = DialogBox(hInst, (LPSTR)"TAGLOC", hWndMain, lpfnTAGLOCMsgProc);
              FreeProcInstance(lpfnTAGLOCMsgProc); 
             TagLocTitle = 0; 
             _fstrupr (Arg[7]);
             if (!nRc)
             	goto RtnFalse;
             if (*Arg[7] == 'T' || *Arg[7] == 'Y' || *Arg[7] == '1')
             {
             	if (nRc)
             		goto RtnTrue;
             	goto RtnFalse;
             }
             if (*Arg[7] == 'Z') 
             {
             	 if (ValidBounds(&TAGLocBounds))
             	 {
             	 	ZoomToRect(TAGLocBounds,FALSE);
             	 	goto RtnTrue;
             	 }
             	 goto RtnFalse;
             }
             if (*Arg[7] == 'B') 
             {
             	 if (ValidBounds(&TAGLocBounds))
             	 {
 					boundstoa (OutLoc,&TAGLocBounds); 
					goto Rtnl;
             	 }
             	 goto RtnFalse;
             }
             	 	
              if (nRc==1)
            {
              	PostMessage(hWndMain, WM_COMMAND, 58000, 0L);        
              	goto RtnTrue;
            }
            else if (nRc > 1 && ValidBounds(&TAGLocBounds))
            {
				ZoomToRect(TAGLocBounds,FALSE);
              	goto RtnTrue;
            }
            else
				goto RtnFalse;
		}
		
		case 602: /* $LOADPM(pickmacro file) Load pickmacro file */
		{				
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (!ExistFile(Arg[1]))
            	goto RtnFalse;
            _fstrcpy (CurView->PickMacroFile,Arg[1]);
			goto RtnTrue;
		}
		
		case 603: /* $DIALOG(ddofile,key,newvals) invokes dialog using key to find rec and newvals to seed new rec */ 
/*		{	double	rval;
			int		l, len; 
			LPSTR	lpOut; 
			char	fillchar;
			
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc ( 855,GMEM_MOVEABLE,2048*3);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			Arg3 = Arg2 + 2048; 
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			if (!(ParLoc = MatchLev (Arg2,','))) goto Rtn0;
			_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			ExpandText (Arg1);
			ExpandText (Arg2);
			ExpandText (Arg3);
			
			if (EditDynamicDialog (CurView->hWnd,Arg1,Arg2,Arg3))
				goto RtnTrue;
			else
            	goto RtnFalse;
			
		} */
		{
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			SetGlobalValue("%ARG1",Arg[2]);
			SetGlobalValue("%ARG2",Arg[3]); 
			if (*Arg[4])
				CenterVP = GetVPIDFromName (Arg[4]);
			else
				CenterVP = 0; 
            _fstrcpy (CurrentDialogType,Arg[1]);
		    if (!stricmp (Arg[1],"LOAD"))
			{
				if (!stricmp (Arg[2],"PANZOOMROTATE"))
		 			CreatePanZoomRotTool (hWndMain,atopt16 (Arg[3],0),atob (Arg[4]));
			}
		    else if (!stricmp (Arg[1],"REMOVE"))
			{
				if (!stricmp (Arg[2],"PANZOOMROTATE"))
		 			CreatePanZoomRotTool (0,atopt16 (Arg[3],0),FALSE);
			}
		    else if (!_fstricmp (Arg[1],"STORMPIPE"))
            {
                  DLGPROC	lpfnSTRMPIPEMsgProc; 
                  
		          if (hWndStrmPoint)
		          	SendMessage (hWndStrmPoint,WM_CLOSE,0,0);
                  if (!hWndStrmPipe)
                  {
	                  lpfnSTRMPIPEMsgProc = MakeProcInstance((DLGPROC)STRMPIPEMsgProc, hInst);
	                  CreateDialog(hInst, (LPSTR)"STRMPIPE", hWndMain, lpfnSTRMPIPEMsgProc); 
	              }
	              
	              if (!_fstricmp (Arg[3],"HLT")) 
	              {
                  	PostMessage(hWndStrmPipe, WM_COMMAND, IDC_SETMULTIPLE, 0L); 
                  	EnableWindow (hWndMain,FALSE);
                  }
	              else 
                  	PostMessage(hWndStrmPipe, WM_COMMAND, IDC_GETRECORD, 0L);
	              
            } 
		    else if (!_fstricmp (Arg[1],"SANPIPE"))
            {
                  DLGPROC	lpfnSTRMPIPEMsgProc; 
                  
		          if (hWndStrmPoint)
		          	SendMessage (hWndStrmPoint,WM_CLOSE,0,0);
                  if (!hWndStrmPipe)
                  {
	                  lpfnSTRMPIPEMsgProc = MakeProcInstance((DLGPROC)STRMPIPEMsgProc, hInst);
	                  CreateDialog(hInst, (LPSTR)"STRMPIPE", hWndMain, lpfnSTRMPIPEMsgProc); 
	              }
	              
	              if (!_fstricmp (Arg[3],"HLT"))
	              {
                  	PostMessage(hWndStrmPipe, WM_COMMAND, IDC_SETMULTIPLE, 0L);
                  	EnableWindow (hWndMain,FALSE);
                  }
	              else 
                  	PostMessage(hWndStrmPipe, WM_COMMAND, IDC_GETRECORD, 0L);
	              
            } 
		    else if (!_fstricmp (Arg[1],"STORMPOINT"))
            {
                  DLGPROC	lpfnSTRMPOINTMsgProc; 
                  
		          if (hWndStrmPipe)
		          	SendMessage (hWndStrmPipe,WM_CLOSE,0,0);
                  if (!hWndStrmPoint)
                  {
	                  lpfnSTRMPOINTMsgProc = MakeProcInstance((DLGPROC)STRMPOINTMsgProc, hInst);
	                  CreateDialog(hInst, (LPSTR)"STRMPOINT", hWndMain, lpfnSTRMPOINTMsgProc); 
	              }
	               
	              if (!_fstricmp (Arg[3],"HLT")) 
	              {
                  	PostMessage(hWndStrmPoint, WM_COMMAND, IDC_SETMULTIPLE, 0L);
                  	EnableWindow (hWndMain,FALSE);
                  }

	              else 
                  	PostMessage(hWndStrmPoint, WM_COMMAND, IDC_GETRECORD, 0L);
	              
            } 
/*		    else if (!_fstricmp (Arg[1],"USER"))
            {
                  DLGPROC	lpfnUSERFORMMsgProc; 
                  
                  if (!hWndUserForm)
                  {
	                  lpfnUSERFORMMsgProc = MakeProcInstance((DLGPROC)USERFORMMsgProc, hInst);
	                  CreateDialog(hInst, (LPSTR)"USERFORM", hWndMain, lpfnUSERFORMMsgProc); 
	              }
	               
	              if (!_fstricmp (Arg[3],"HLT")) 
	              {
                  	PostMessage(hWndUserForm, WM_COMMAND, IDC_SETMULTIPLE, 0L);
                  	EnableWindow (hWndMain,FALSE);
                  }

	              else 
                  	PostMessage(hWndUserForm, WM_COMMAND, IDC_GETRECORD, 0L);
	              
            }  */
		    else if (!_fstricmp (Arg[1],"SANPOINT"))
            {
                  DLGPROC	lpfnSTRMPOINTMsgProc; 
                  
		          if (hWndStrmPipe)
		          	SendMessage (hWndStrmPipe,WM_CLOSE,0,0);
                  if (!hWndStrmPoint)
                  {
	                  lpfnSTRMPOINTMsgProc = MakeProcInstance((DLGPROC)STRMPOINTMsgProc, hInst);
	                  CreateDialog(hInst, (LPSTR)"STRMPOINT", hWndMain, lpfnSTRMPOINTMsgProc); 
	              }
	               
	              if (!_fstricmp (Arg[3],"HLT")) 
	              {
                  	PostMessage(hWndStrmPoint, WM_COMMAND, IDC_SETMULTIPLE, 0L); 
                  	EnableWindow (hWndMain,FALSE);
                  }
	              else 
                  	PostMessage(hWndStrmPoint, WM_COMMAND, IDC_GETRECORD, 0L);
	              
            } 
		    else if (!_fstricmp (Arg[1],"SETVAL"))
            { 
				  if (hWndStrmPipe)
                  	PostMessage(hWndStrmPipe, WM_COMMAND, IDC_SETVAL, 0L);
				  else if (hWndStrmPoint)
                  	PostMessage(hWndStrmPoint, WM_COMMAND, IDC_SETVAL, 0L);
            } 
		    else if (!_fstricmp (Arg[1],"LOAD"))
            {   
            	if (hDynamicDialog)
            	{
       				SendMessage (hDynamicDialog,WM_CLOSE,0,0);
            		/*if (_fstricmp (Arg[2],CurrentDialogFile))
           				SendMessage (hDynamicDialog,WM_CLOSE,0,0);
           			else
           			{
                  		PostMessage(hDynamicDialog, WM_COMMAND, IDC_SETVAL, 0L);
                  		goto RtnTrue;
                  	} */
                }
           				
                _fstrcpy (CurrentDialogFile,Arg[2]);
				rtn = EditDynamicDialog (CurView->hWnd,Arg[2],Arg[3],0); 
				goto Rtnrtn;
            }
            else 
            	goto RtnFalse;
            goto RtnTrue;
		} 
		case 604: /* $REPORT(Reportfile,Viewport,TF(opt loadonly),TF(opt fit-to-vp),TF(use current print setup)) */
		{	 
			if (InInfoBox && !InAtPrint)
			{
				HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
				LPSTR	pMem = GlobalLock (hMem);

				sprintf (pMem,"$REPORT(%s)",Args);
				strcpy (OutLoc,pMem);
				GSSiGlobUlFree (&hMem);
				goto Rtnl;
			}
			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			_fstrupr (Arg[2]);
			if (ScanForReports)
				HaveReports = TRUE; 
			else if (!stricmp (Arg[2],"POPUP"))
			{
				HWND	hWnd = GetFocus();
				RECT	TBRect;
				POINT	ScreenPoint;
				HANDLE	hTxt=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
				LPSTR	ptxt=GlobalLock (hTxt);
				MSG		msg;
				HANDLE	hSaveScreen;
				HDC		hDC;
				
				if (hWndScroll2)
					hWnd = hWndScroll2;
				GetCursorPos (&ScreenPoint); 
				ScreenToClient (hWnd,&ScreenPoint);   
				sprintf (ptxt,"$REPORT(%s)",Arg[1]);
				YellowTextBox (hWnd,ptxt,ScreenPoint,&TBRect, (LPRECT)2,TRUE,0);
				hDC = GetDC (hWnd);
				InflateRect(&TBRect, 1, 1);
				if (hWnd == hWndMain)
				{
 					if (hLastBox)
					{   
						RestoreScreen2 (hDC, hLastBox,0,FALSE);
						DestroySavedScreen (&hLastBox,0);
					}
					hSaveScreen = hLastBox = SaveScreen2 (hWnd,hDC,TBRect,0,0);
				}
				else
					hSaveScreen = SaveScreen2 (hWnd,hDC,TBRect,0,0);
				YellowTextBox (hWnd,ptxt,ScreenPoint,0,0,TRUE,0);
				GSSiGlobUlFree (&hTxt);
				while (GSSiGetMessage(&msg, hWnd,0,0)>0)
				{   
           			if (msg.message == WM_MOUSEMOVE)
					{
    					POINT Point = POINTStoPOINT(MAKEPOINTS(msg.lParam));

						if (!PtInRect (&TBRect,Point))
							break;
					}
          			TranslateMessage(&msg);
					DispatchMessage(&msg);
 				}
				if (hWnd == hWndMain)
				{
					RestoreScreen2 (hDC, hLastBox,0,FALSE);
					DestroySavedScreen (&hLastBox,0);
				}
				else
				{
					RestoreScreen2 (hDC, hSaveScreen,0,FALSE);
					DestroySavedScreen (&hSaveScreen,0);
				}
				ReleaseDC (hWnd,hDC);
			}
			else if (!Printing && !InAtPrint && !_fstricmp (Arg[2],"PRINT"))
			{
				SetCurVal (Arg[1],IDS_FILERPT);
				if (PrintScrollReport (hWndMain,atob(Arg[5])))
					goto RtnTrue;
			}
			else if (InAtPrint || !_fstricmp (Arg[2],"PRINT"))
			{
				if (AddReportToPrintList (Arg[1],CurrentRefno,CurrentPrefix,CurrentUDI,Arg[3]))
					goto RtnTrue;
			}
			else if (Report (Arg[1],Arg[2], 0, 0, 0,atob(Arg[3]),atob(Arg[4])))
	           	goto RtnTrue;
            goto RtnFalse;
		}
		
		case 605: // $APPEND(file,line)
		{	
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				i = AppendFile (Arg[1],Arg[2]);
			else
				i = AppendFile2 (Arg[1],Arg[2]);
			itoa (i,OutLoc,10);
			goto Rtnl;
		}
		
		case 606: // $VERIFY(message)
		{				
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (GSSiMessageBox (0,Arg[1], " ", MB_YESNO, 0) == IDYES)
				goto RtnTrue;  
			goto RtnFalse;
		}

		case 607: /* $EXPORT(Type,ControlFile) */
		{	 
			short	type;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			_fstrcpy (AutoExportName,Arg[2]);
			if (!_fstricmp (Arg[1],"SHP"))
				type = SHP;
			else if (!_fstricmp (Arg[1],"TXT"))
				type = TXT;
			else if (!_fstricmp (Arg[1],"ORTHO"))
				type = ORTHBMP;
			else if (!_fstricmp (Arg[1],"ORACLE"))
				type = ORACLE;
			else if (!_fstricmp (Arg[1],"ORACLEDTM"))
				type = ORACLEDTM;
			else if (!_fstricmp (Arg[1],"DTM"))
				type = DTMTOTEXT;
			else if (!_fstricmp (Arg[1],"DGN"))
				type = DGN;
			else
				goto RtnFalse;
			if (ExportData (hWndMain,type))
	           	goto RtnTrue;
            else
				goto RtnFalse;
		}
		
		case 608: /* $IMPORT(Type,ControlFile) */
		{	 
            short	nRc;
            
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			_fstrcpy (AutoExportName,Arg[2]);
			if (*Arg[3])
				sprintf (_fstrchr (AutoExportName,0),",%s",Arg[3]);
			if (!_fstricmp (Arg[1],"DGN"))  
            {
                  DLGPROC	lpfnLOADDGNDUMPMsgProc; 

                  lpfnLOADDGNDUMPMsgProc = MakeProcInstance((DLGPROC)LOADDGNDUMPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADDGNDUMP", hWndMain, lpfnLOADDGNDUMPMsgProc);
                  FreeProcInstance(lpfnLOADDGNDUMPMsgProc);
                  *AutoExportName=0;
                  if (nRc)
                  	goto RtnTrue;
                  else
                  	goto RtnFalse;
            }
            else if (!_fstricmp (Arg[1],"SHP")) 
            {
                  DLGPROC	lpfnLOADSHPMsgProc; 
                  
                  lpfnLOADSHPMsgProc = MakeProcInstance((DLGPROC)LOADSHPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADSHP", hWndMain, lpfnLOADSHPMsgProc);
                  FreeProcInstance(lpfnLOADSHPMsgProc);  
                  *AutoExportName=0;
                  if (nRc)
                  	goto RtnTrue;
                  else
                  	goto RtnFalse;
            }
            else if (!_fstricmp (Arg[1],"DXF")) 
            {
                  DLGPROC	lpfnLOADDXFMsgProc; 

                  lpfnLOADDXFMsgProc = MakeProcInstance((DLGPROC)LOADDXFMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"LOADDXF", hWndMain, lpfnLOADDXFMsgProc);
                  FreeProcInstance(lpfnLOADDXFMsgProc);
                  *AutoExportName=0;
                  if (nRc)
                  	goto RtnTrue;
                  else
                  	goto RtnFalse;
            }
            else if (!_fstricmp (Arg[1],"POINT")) 
            {
                  DLGPROC	lpfnPOINTMAPMsgProc; 
                  
                  lpfnPOINTMAPMsgProc = MakeProcInstance((DLGPROC)POINTMAPMsgProc, hInst);
                  nRc = DialogBox(hInst, (LPSTR)"POINTMAP", hWndMain, lpfnPOINTMAPMsgProc);
                  FreeProcInstance(lpfnPOINTMAPMsgProc);
                  *AutoExportName=0;
                  if (nRc)
                  	goto RtnTrue;
                  else
                  	goto RtnFalse;
            }
            else if (!_fstricmp (Arg[1],"IMAGE")) 
            {
			      DLGPROC lpfnLOADMDMsgProc;
                  
			      lpfnLOADMDMsgProc = MakeProcInstance((DLGPROC)LOADMDMsgProc, hInst);
			      nRc = DialogBox(hInst, (LPSTR)"LOAD_MD", hWndMain, lpfnLOADMDMsgProc);
			      FreeProcInstance(lpfnLOADMDMsgProc);
                  *AutoExportName=0;
                  if (nRc)
                  	goto RtnTrue;
                  else
                  	goto RtnFalse;
            }
            else if (!_fstricmp (Arg[1],"ADDRESS")) 
            {
				BOOL sm = SetModeless(TRUE);
				 nRc =  DialogBox(hInst, (LPSTR)"ADDLOC_FROMADD", hWndMain, (DLGPROC)ADDLOC_FROMADDMsgProc);
				 SetModeless(sm);
				  //                  nRc = DialogBox(hInst, (LPSTR)"ADDLOC_FROMADD", hWnd, lpfnADDLOC_FROMADDMsgProc);
				  //nRc = DialogBox(hInst, (LPSTR)"ADDLOC_FROMADD", hWndMain, (DLGPROC)ADDLOC_FROMADDMsgProc);
                  //*AutoExportName=0;

                  if (nRc)
                  	goto RtnTrue;
                  else
                  	goto RtnFalse;
                  //PostMessage(hWndMain, WM_COMMAND, IDM_ADDLOC_FROMADD, 0L);
                  //goto RtnTrue;
            }
			else
            	goto RtnFalse;
		} 

		case 609: /* $BROWSE(pathname,startloc,backlines,forlines,searchstring)*/
			{	 
			LPSTR	lpEnd; 
			DPOINT	p1,p2;
			short	backlines, forwardlines;
			long	Loc;
			
			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			Loc = atof (Arg[2]);
			backlines = atof (Arg[3]);
			forwardlines = atof (Arg[4]);
			if (BrowseTextFile (Arg[1],Loc,backlines,forwardlines,Arg[5]))
				goto RtnTrue;
			else
				goto RtnFalse;
		} 
		
		case 610: // $INSERT(hSQL)
		{	
			HANDLE	hUpdateString, hFieldString, hValueString;
			LPSTR	pUpdateString, pFieldString, pValueString;
			HANDLE	hSQL; 
			LPSTR	pSetVals;  
			short	n;
			LPOPENFILEDATA  FilePtr;
			LPOPENSQLDATA   SQLPtr; 
			BOOL	rtn, NeedQuote;
						
			if (!hSetVals)
				goto RtnFalse;
			hMem = GSSiGlobAlloc ( 858,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			hSQL = (HANDLE)atol(Arg[1]);
			if (!hSQL)
				goto RtnFalse;
			hUpdateString = GSSiGlobAlloc ( 859,GMEM_MOVEABLE,USHRT_MAX);
			pUpdateString = GlobalLock (hUpdateString);
			hFieldString = GSSiGlobAlloc ( 860,GMEM_MOVEABLE,USHRT_MAX);
			pFieldString = GlobalLock (hFieldString);
			hValueString = GSSiGlobAlloc ( 861,GMEM_MOVEABLE,USHRT_MAX);
			pValueString = GlobalLock (hValueString);  
			_fstrcpy (pFieldString,"(");
			_fstrcpy (pValueString,"VALUES (");  
			n = nSetVals;  
			pSetVals = GlobalLock (hSetVals);
			while (n--)
			{   
				NeedQuote = NeedSQLValueQuote (hSQL,pSetVals);
				_fstrcat (pFieldString,pSetVals); 
				if (n)
					_fstrcat (pFieldString,",");
				pSetVals = _fstrchr (pSetVals,0);
				pSetVals++; 
				if (NeedQuote)
					_fstrcat (pValueString,"'");
				_fstrcat (pValueString,pSetVals); 
				if (NeedQuote)
					_fstrcat (pValueString,"'");
				if (n)
					_fstrcat (pValueString,",");
				pSetVals = _fstrchr (pSetVals,0);
				pSetVals++;  
			} 
			GlobalUnlock (hSetVals);
			sprintf (pUpdateString,"%s) %s)",pFieldString,pValueString);
           	GSSiGlobUlFree (&hFieldString);
           	GSSiGlobUlFree (&hValueString);
	        SQLPtr = (LPOPENSQLDATA) GlobalLock (hSQL);
	        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);  
           	rtn=InsertExternalFieldData (FilePtr,pUpdateString);
           	GSSiGlobUlFree (&hUpdateString);
            if (rtn)
            	goto RtnTrue;
            else
            	goto RtnFalse;
		}
		
		case 611: // $BEFORE(string,str,optional l or f(default))) - return portion of string before char
		{
			LPSTR	lpChr, LastLoc; 
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			_fstrlwr (Arg[3]);
			if (*Arg[2] == '\'' && Arg[2][2] == '\'' && Arg[2][1] != 0) 
			{
				*Arg[2] = Arg[2][1];
				Arg[2][1] = 0;
			}
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			l = _fstrlen (Arg[2]);
			if (!l)
				goto RtnFalse;
			if (*Arg[3] == 'l')
			{
				lpChr = Arg[1];
				LastLoc = 0;
				do
				{
					if (*lpChr)
					{
						lpChr = _fstrstr(lpChr + 1, Arg[2]);
						if (lpChr)
							LastLoc = lpChr;
					}
					else
						lpChr = 0;
				} while (lpChr);
				lpChr = LastLoc;
			}
			else if (l == 1)
				lpChr = strchr(Arg[1], *Arg[2]);
			else
				lpChr = strstr ((const char* )Arg[1],(const char* )Arg[2]);
			if (lpChr)
				*lpChr = 0;
			_fstrcpy (OutLoc,Arg[1]);
			goto Rtnl;
		}
		
		case 612:	/* $OFFSET(HLT,dist) offsets hlt item */   
		{	
			LPHIGHLIGHTDATA	pHighlightData;
						
			nArgs = GetFunArgs(Args, Arg,3, &hMem, pBrkPt, bpOffset, bpLen);
			pHighlightData = (LPHIGHLIGHTDATA)(Arg[3]);
			
			pos = BT_FIRST;
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)pHighlightData))
			{   
				pos = BT_NEXT;
				PickList[0]=pHighlightData->PD;
				Dist = atof(Arg[2]);
				OffsetPickedArea (0,Dist);  
			}
			goto RtnTrue;
		}
		
		
		case 613: // $RETURN(value) returns from current macro
		{				
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			DebugReturn(Arg[1]);
			if (pMacroReturnValue)
				_fstrncpy(pMacroReturnValue, Arg[1], MAXARGLENGTH);
			SetContinueProcessing (-1);
            goto RtnTrue;
		}
		
		case 614: //$SYMNUM(symname)
		{
			short	n;
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			n = GetDictSymbolNumber(Arg[1]);
			itoa (n,OutLoc,10);
			goto Rtnl;  
		}

		case 615:	// $BOUNDS(MAP,pathname) returns bounds of map 
					// $BOUNDS(REF,refno,(opt)picklayerid) returns bounds of refno  
					// $BOUNDS(POINT,point,dist) returns bounds from point and dist  
					// $BOUNDS(INIT) returns init bounds
					// $BOUNDS(ADD,B1,B2) add bounds if both valid (xmn >= xmx) 
					// $BOUNDS(FACTOR,bounds,factor)
					// $BOUNDS(MULTIPLY,bounds,factor)
					// $BOUNDS(INC,B1,D) add D to bounds 
					// $BOUNDS(VISLIM) returns bounds of visible limits
					// $BOUNDS(VALID,BOUNDS) return 0 if not valid 1 if valid  
					// $BOUNDS(HLTLIM) highlight bounds
					// $BOUNDS(TRAN,BOUNDS,tranfile,direction (F(default)orR)
				    // $BOUNDS(CONVERT,BOUNDS,from,to)
					// $BOUNDS(TRANFILE,tranfile,direction (F(default)orR)  
					// $BOUNDS(WIDTH,BOUNDS) returns width
					// $BOUNDS(HEIGHT,BOUNDS) returns height  
					// $BOUNDS(RADIUS,BOUNDS) radius of circle that encompasses bounds  
					// $BOUNDS(MID,BOUNDS) returns midpoint 
					// $BOUNDS(MIN,BOUNDS) returns min point  
					// $BOUNDS(MAX,BOUNDS) returns max point 
					// $BOUNDS(LEFTorRIGHTorTOPorBOTTOM,BOUNDS) 
					// $BOUNDS(ULorLLorURorLR) 
					// $BOUNDS(CONTAINS,BOUNDS,POINTorBOUNDS)
					// $BOUNDS(LAYER,layer name,vpname)
					// $BOUNDS(DISPLAY,BOUNDS,BORDERCOLOR,FILLCOLOR);
					// $BOUNDS(3DTO2D,BOUNDS3D);
					// $BOUNDS(2DTO3D,BOUNDS,YMN,YMX);
		{				
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (!_fstricmp (Arg[1],"INIT"))
			{
				DBoundsInit (&Bounds);
				boundstoa (OutLoc,&Bounds); 
				goto Rtnl;
			} 
			else if (!_fstricmp(Arg[1], "3DTO2D"))
			{
				MNMXCORD3D	Bounds3D;

				Bounds3D = atobounds3D(Arg[2], &Err);
				Bounds.xmn = Bounds3D.xmn;
				Bounds.xmx = Bounds3D.xmx;
				Bounds.ymn = Bounds3D.ymn;
				Bounds.ymx = Bounds3D.ymx;

				boundstoa(OutLoc, &Bounds);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "2DTO3D"))
			{
				MNMXCORD3D	Bounds3D;
				Bounds = atobounds(Arg[2], &Err);
				Bounds3D.xmn = Bounds.xmn;
				Bounds3D.xmx = Bounds.xmx;
				Bounds3D.ymn = Bounds.ymn;
				Bounds3D.ymx = Bounds.ymx;
				Bounds3D.zmn = atof(Arg[3]);
				Bounds3D.zmx = atof(Arg[4]);
				bounds3Dtoa(OutLoc, &Bounds3D);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "INBOUNDS"))
			{
				MNMXCORD	Bounds2;

				Bounds = atobounds(Arg[2], &Err);
				Bounds2 = atobounds(Arg[3], &Err);
				rtn = BoundsInBounds(&Bounds, &Bounds2, atoi(Arg[4]));
				goto Rtnrtn;
			}
			else if (!_fstricmp(Arg[1], "DISPLAY"))
			{
				HANDLE hPoints = GSSiGlobAlloc(1225, GMEM_MOVEABLE, 4 * sizeof(DPOINT));
				LPDPOINT pPoint = (HPDPOINT)GlobalLock(hPoints);
				Bounds = atobounds(Arg[2], &Err);
				COLORREF borderColor = atoi(Arg[3]);
				COLORREF fillColor = atoi(Arg[4]);
				BOOL doFill = TRUE;
				BOOL doBorder = TRUE;
				if (fillColor == -1)
					doFill = FALSE;
				DisplayBounds(Bounds,doFill,doBorder,fillColor,borderColor);
				/*BoundsToPoints(&Bounds, pPoint, 0);
				SelectObject(CurView->hDC, GetStockObject(GRAY_BRUSH));
				GMPolygon(CurView->hDC, pPoint, 4);
				GSSiGlobUlFree(&hPoints);*/
				goto RtnTrue;
			}
			else if (!_fstricmp(Arg[1], "VIEWPORT"))
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));  
				boundstoa (OutLoc,&CurView->WBounds);
				CurView = SaveVP;
				goto Rtnl;
			} 
			else if (!_fstricmp (Arg[1],"LAYER"))
			{   
				SetCurView ( SetVPFromName (Arg[3],&Err));  
				for (i=0;i<CurView->NumFiles;i++)
				{
					if (!_fstricmp (Arg[2],CurView->FileID[i]))
					{ 
						if (!GetMapBounds (CurView->lpFiles[i],&Bounds))
							goto RtnFalse;
						boundstoa (OutLoc,&Bounds);
						goto Rtnl;
					}
				}
				goto RtnFalse;  
			} 
			else if (!_fstricmp (Arg[1],"POINTS"))
			{   
				DBoundsInit (&Bounds);
			    nPoints = GetPointsFromList (Arg[2],&hPoints);  
			    pPoint = (HPDPOINT)GlobalLock (hPoints);
				for (n=0;n<nPoints;n++)
					AddDPointToMinMax (&pPoint[n],&Bounds); 
				GSSiGlobUlFree (&hPoints);
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"CONVERT"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;
				ConvertRectCoord (&Bounds2, &Bounds,atoi(Arg[3]),atoi(Arg[4]));
				boundstoa (OutLoc,&Bounds2); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"TRAN"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;
				if (!(hTran=LoadTranFileWithDandT(Arg[3])))
					goto RtnFalse;
				TranBounds (hTran,&Bounds);
				CloseTRANS2 (&hTran);
				boundstoa (OutLoc,&Bounds); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"TRANFILE"))
			{   
				Bounds = GetTranFileBounds (Arg[2],Arg[3]);
				boundstoa (OutLoc,&Bounds); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"VALID"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;
				goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"WIDTH"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;  
				RVal = Bounds.xmx - Bounds.xmn;
				ftoa (OutLoc,RVal);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "HEIGHT"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				RVal = Bounds.ymx - Bounds.ymn;
				ftoa(OutLoc, RVal);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "RADIUS"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				RVal = MinMaxRadius (&Bounds);
				ftoa(OutLoc, RVal);
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"MID"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;  
				Point = MinMaxMidPointD (&Bounds);
				dpointtoa (OutLoc,&Point); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"MIN"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;  
				Point.x = Bounds.xmn;
				Point.y = Bounds.ymn;
				dpointtoa (OutLoc,&Point); 
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MAX"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				Point.x = Bounds.xmx;
				Point.y = Bounds.ymx;
				dpointtoa(OutLoc, &Point);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MINX"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				ftoa(OutLoc, Bounds.xmn);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MAXX"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				ftoa(OutLoc, Bounds.xmx);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MINY"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				ftoa(OutLoc, Bounds.ymn);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MAXY"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				ftoa(OutLoc, Bounds.ymx);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MINWH"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err || !ValidBounds(&Bounds))
					goto RtnFalse;
				Point = MinMaxMidPointD(&Bounds);
				double w = Bounds.xmx - Bounds.xmn;
				double h = Bounds.ymx - Bounds.ymn;
				double minw = atof(Arg[3]);
				double minh = atof(Arg[4]);
				w = max(minw, w)/2;
				h = max(minh, h)/2;
				Bounds.xmn = Point.x - w;
				Bounds.xmx = Point.x + w;
				Bounds.ymn = Point.y - h;
				Bounds.ymx = Point.y + h;
				boundstoa(OutLoc, &Bounds);
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"HLTLIM"))
			{   
				boundstoa (OutLoc,&HLTBounds);
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"VISLIM"))
			{   
				if (!GetVisBounds (&Bounds,CurView->hDC))
					goto RtnFalse;
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"ADD"))
			{   
				BOOL	Err1,Err2;
				
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err1);
				Bounds2 = atobounds (Arg[3],&Err2);
				if (Err1 && Err2)
					goto Rtnl;
				if (!Err1 && !Err2)
				{
					if (ValidBounds (&Bounds) || ValidBounds (&Bounds2))
						AddMinMaxD (&Bounds, &Bounds2);
					else
						goto Rtnl;
				}
				else if (Err2)
				{
					Point = atopt (Arg[3],&Err);
					if (!Err)
						AddDPointToMinMax (&Point,&Bounds);
				}
				boundstoa (OutLoc,&Bounds); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"INC"))
			{   
				 
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				RVal = atof (Arg[3]);
				ExpandBounds (&Bounds,RVal);
				boundstoa (OutLoc,&Bounds); 
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "FACTOR"))
			{

				*OutLoc = 0;
				Bounds = atobounds(Arg[2], &Err);
				if (Err)
					goto Rtnl;
				RVal = atof(Arg[3]);
				Bounds = FactorBounds(&Bounds, RVal);
				boundstoa(OutLoc, &Bounds);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "MULTIPLY"))
			{

				*OutLoc = 0;
				Bounds = atobounds(Arg[2], &Err);
				if (Err)
					goto Rtnl;
				RVal = atof(Arg[3]);
				Bounds = MultiplyBounds(&Bounds, RVal);
				boundstoa(OutLoc, &Bounds);
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"UL"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				Point.x = Bounds.xmn;
				Point.y = Bounds.ymx;
				dpointtoa (OutLoc,&Point); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"UR"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				Point.x = Bounds.xmx;
				Point.y = Bounds.ymx;
				dpointtoa (OutLoc,&Point); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"LL"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				Point.x = Bounds.xmn;
				Point.y = Bounds.ymn;
				dpointtoa (OutLoc,&Point); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"LR"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				Point.x = Bounds.xmx;
				Point.y = Bounds.ymn;
				dpointtoa (OutLoc,&Point); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"LEFT"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				ftoa (OutLoc,Bounds.xmn); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"BOTTOM"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				ftoa (OutLoc,Bounds.ymn); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"RIGHT"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				ftoa (OutLoc,Bounds.xmx); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"TOP"))
			{   
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				ftoa (OutLoc,Bounds.ymx); 
				goto Rtnl;
			}
			else if (!_fstricmp (Arg[1],"MAP"))
			{   
				SetViewport (*pCommandViewport); 
				SelectVisList (FALSE); 
				if (!GetMapBounds (Arg[2],&Bounds))
					goto RtnFalse;
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;  
			} 
			else if (!_fstricmp (Arg[1],"POINT"))
			{   
				Point = atopt (Arg[2],&Err);
				if (Err)
					goto RtnFalse; 
				Dist = atof (Arg[3]);
				Bounds.xmn = Point.x - Dist;
				Bounds.xmx = Point.x + Dist;
				Bounds.ymn = Point.y - Dist;
				Bounds.ymx = Point.y + Dist;
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;  
			} 
			else if (!_fstricmp(Arg[1], "TOPOINTS"))
			{
				Bounds = atobounds(Arg[2], &Err);
				sprintf(OutLoc, "%f %f %f %f %f %f %f %f", Bounds.xmn, Bounds.ymn, Bounds.xmn, Bounds.ymx, Bounds.xmx, Bounds.ymx, Bounds.xmx, Bounds.ymn);
				goto Rtnl;
			}
			else if (!_fstricmp(Arg[1], "CONTAINS"))
			{
				Bounds = atobounds(Arg[2], &Err);
				if (Err)
					goto RtnFalse;
				Point = atopt(Arg[3], &Err);
				if (Err)
					goto RtnFalse;
				if (PointInBounds(Point, &Bounds))
					goto RtnTrue;
			}
			else if (!_fstricmp(Arg[1], "REF"))
			{   
				short	PickFile, PickLayerID;
				
				PickLayerID = atoi (Arg[3]);
				if (!PickLayerID)
					PickFile = -1; 
				else
					PickFile = GetPickFile (PickLayerID);
				SetGlobalValue("%PICKLAYERID","0");
				Refno = atol (Arg[2]);
				if (!PickByRefno (Refno,0,0,PickFile))
					goto RtnFalse;
				Bounds = PickList[0].Rect;
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;  
			} 
			
			else if (!_fstricmp (Arg[1],"TAG"))
			{   
				short	PickFile, PickLayerID;  
				LPSTR	UDI;
				
				PickLayerID = atoi (Arg[3]);
				if (!PickLayerID)
					PickFile = -1; 
				else
					PickFile = GetPickFile (PickLayerID);
				SetGlobalValue("%PICKLAYERID","0");
				UDI = _fstrchr (Arg[2],':');
				if (!UDI)
					goto RtnFalse;
				*UDI++=0;
				if (!PickByRefno (0,Arg[2],UDI,PickFile))
					goto RtnFalse;
				Bounds = PickList[0].Rect;
				SetPickGlobals (0);
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;  
			}
			else if (!_fstricmp(Arg[1], "TOOLBAR"))//$BOUNDS(TOOLBAR,All or id,vp)
			{
				SetCurView(SetVPFromName(Arg[3], &Err));
				if (GetToolbarBounds(CurView->hWnd, &Bounds))
				{
					boundstoa(OutLoc, &Bounds);
					goto Rtnl;
				}
			}
			
			goto RtnFalse;
		} 
		
		case 616: // $DECODE(table,numval) - returns decoded value
		{
			LPSTR	lpChr; 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = atoi(Arg[2]);
			GetListValue (Arg[1], n, OutLoc);
			goto Rtnl;
		}
		
		case 617: //$TEXTAZ(AZ)
		{
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			AZ = atof(Arg[1]);
			if (AZ > HALFPI && AZ < HALFPI*3)
				AZ = LTWOPI (AZ+PY);
			sprintf (OutLoc,"%f",AZ); 
			goto Rtnl;  
		}

		case 618: // $BACKUP(backuplist.txt,backupdirectory)
		{
			LPSTR	lpChr; 
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (BackupFiles(Arg[1], Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 619: // $EXPAND(value,arg1IsFile(TorF),maxexpandedfilelength(def is USHRT_MAX))  
		{	
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (atob(Arg[2]))
			{
				int memSize = GSSiLength(Arg[1]);

				if (memSize > 0)
				{
					int maxMemSize = atoi(Arg[3]);

					if (maxMemSize <= 0)
						maxMemSize = MAXARGLENGTH;
					else
						maxMemSize += 2;
					memSize = max(memSize, maxMemSize);
					{
						HFILE Fid = GSSiOpenFile(Arg[1], 0, OF_READ);
						HANDLE hFile = GSSiGlobAlloc(0, GMEM_MOVEABLE, memSize);
						LPSTR pFile = GlobalLock(hFile);
						int len = memSize;
						LPSTR pLoc = pFile;

						while (fgetstring(pLoc, len, Fid))
						{
							int l = strlen(pLoc);
							len -= l;
							pLoc += l;
						}
						
						GSSiClose2 (&Fid);
						ExpandText(pFile);
						strcpy(OutLoc, pFile);
						GSSiGlobUlFree(&hFile);
					}
				}
			}
			else
			{
				ExpandText(Arg[1]);
				_fstrcpy(OutLoc, Arg[1]);
			}
   			goto Rtnl;
		}
		
		case 620: // $PARENT(SYMNAME)  
		{				
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (!(SymNum = GetDictSymbolNumber (Arg[1])))
				goto Rtnl;
			if ((SymNum = GetDictSymParent (SymNum)))
				GetDictSymName (SymNum,OutLoc);
			goto Rtnl;   
		}
		
		case 621: // $SIGNOF(numval)  
		{				
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			RVal = atof(Arg[1]);
			if (RVal < 0)
				_fstrcpy (OutLoc,"-"); 
			else
				_fstrcpy (OutLoc,"+"); 
			goto Rtnl;   
		}  
		
		case 622: // $MEMMAP(Pathname,dimensions)
		{
			LPSTR	lpChr; 
			
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen);  
			SetViewport (*pCommandViewport); 
	    	if (hdcMemMap)
	    	{
				HBITMAP hbm = SelectObject(hdcMemMap, hbmpOld);
				DeleteObject (hbm); 
				DeleteDC (hdcMemMap);
				CurView->hDC = OldDC;
				MemMap = FALSE;     
	    	}
	    	if (!nArgs)
	    		goto RtnTrue; 
	    	if (!_fstricmp (Arg[1],"%SCREEN"))
	    	{
	    	    GetClientRect(hWndMain,&Rect);
	        	MemMapWidth = Rect.right-Rect.left+1;
	            MemMapHeight = Rect.bottom-Rect.top+1;
            }
            else
            {
				if (sscanf (Arg[2],"%i %i",&MemMapWidth,&MemMapHeight) != 2) 
					goto RtnFalse; 
			}
			MemMap = TRUE;     
			_fstrcpy (MemMapName,Arg[1]);
			hdcMemMap = CreateCompatibleDC(CurView->hDC);    
			curProgID = 10004;
			hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight);
			curProgID = -1;
			OldDC = CurView->hDC;
			CurView->hDC = hdcMemMap;
			hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
		    SetMainRect (CurView->hWnd,CurView->hDC,0,8);
		    SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0); 
		    goto RtnTrue;
		}
		
		case 623: // $BMPDIM(pathname of bmp)returns width height  
		{				
		    short	RowsPerStrip,PhotoInterp,SamplesPerPixel,PlanarConfig,BitsPerSample[8];
		    short	TIFFCompression;
		    HANDLE	hTIFFOffsets=0, hTIFFLengths=0, hDibInfo=0; 
		    short	NumStrips, SkipBits;
		    
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			Fid = GSSiOpenFile (Arg[1],0,OF_READ);
			if (Fid == HFILE_ERROR)
				goto RtnFalse;
			if (_fstrstr (Arg[1],".tif") || _fstrstr (Arg[1],".TIF")) 
			{
            	rtn = SetupTIFHeader (Fid, &hDibInfo,&NumStrips,&hTIFFOffsets,&hTIFFLengths,
            						 &RowsPerStrip,&PhotoInterp,
									 &SamplesPerPixel,&PlanarConfig,&TIFFCompression,
									 BitsPerSample,&SkipBits); 
				GSSiGlobFree (&hTIFFOffsets);
				GSSiGlobFree (&hTIFFLengths);
			}
			else
			{
				rtn = ReadBitMapHeader (Fid,&hDibInfo, &ImageOffset);
			}
			GSSiClose2 (&Fid);
			if (rtn)
			{    
				LPBITMAPINFO    pDibInfo=(LPBITMAPINFO)GlobalLock (hDibInfo);
									    
				sprintf (OutLoc,"%ld %ld",pDibInfo->bmiHeader.biWidth,pDibInfo->bmiHeader.biHeight);
				GSSiGlobUlFree (&hDibInfo);
				goto Rtnl;
			}
			GSSiGlobUlFree (&hDibInfo);
			goto RtnFalse;
		}  
		
		case 624:	//$MAPSET	(NEW,Name,IDGlobalName)
					//			(DELETE,Name)
					//			(SHOW,Name)
					//			(PRINT,Name)
					//			(ADDMAP,MSID,Mapname) 
					//			(LOADMAP,MSID,MapID) 
		{	
			long	NewNum;
			LPSTR	MSDir, str, str2;
			
			hMem = GSSiGlobAlloc ( 874,GMEM_MOVEABLE,6*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048;
			Arg3 = Arg2 + 2048;
			MSDir = Arg3 + 2048; 
			str	= MSDir + 2048;
			str2 = str + 2048;
			
			GetGlobalCVal ("[%MAPSETDIR]",MSDir,"[%DL]mapsets");  
			ExpandText (MSDir);
			if ((ParLoc = MatchLev (Args,',')))
			{
				_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
				*ParLoc = 0; 
			}
			else
				*Arg2 = 0;
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if ((ParLoc = MatchLev (Arg2,',')))
			{
				_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
				*ParLoc = 0;  
			}
			else
				*Arg3 = 0;
			ExpandText (Arg1);
			ExpandText (Arg2);
			ExpandText (Arg3); 
			if (!_fstricmp (Arg1,"CREATE"))
            {
				DLGPROC lpfnCREATE_MAPSETMsgProc;
				
				lpfnCREATE_MAPSETMsgProc = MakeProcInstance((DLGPROC)CREATE_MAPSETMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"CREATE_MAPSET", hWndMain, lpfnCREATE_MAPSETMsgProc);
				FreeProcInstance(lpfnCREATE_MAPSETMsgProc);

            }
			else if (!_fstricmp (Arg1,"NEW"))
			{   
				DWORD	ErrDW;
				
				sprintf (str,"%s\\dummy.plt",MSDir);
				NewNum = GetNewRefno(str,0,0,0,0);				
				sprintf (str,"%s\\ms%6.6ld",MSDir,NewNum); 
				if (!GSSiMakeDir (str,&ErrDW))
				{   
					GSSiMessageBox (0,MSDir,"Unable to create new map set",MB_ICONEXCLAMATION,0);
					goto RtnFalse;
				} 
				sprintf (str,"%s\\mapsets.txt",MSDir); 
				sprintf (str2,"%s|[%s]=ms%6.6ld",Arg2,Arg3,NewNum);
				AppendFile (str,str2);
				sprintf (str,"%s\\ms%6.6ld\\nextref.txt",MSDir,NewNum); 
				AppendFile (str,"0");  
				sprintf (str,"ms%6.6ld",NewNum);
				SetGlobalValue (Arg3,str);
			}
			else if (!_fstricmp (Arg1,"DELETE"))
			{
			}
			else if (!_fstricmp (Arg1,"SHOW"))
			{
			}
			else if (!_fstricmp (Arg1,"PRINT"))
			{
			} 
			else if (!_fstricmp (Arg1,"ADDMAP"))
			{
				sprintf (str,"%s\\%s\\dummy.plt",MSDir,Arg2);
				NewNum = GetNewRefno(str,0,0,0,0);				
				sprintf (str2,"%s|[MAPID]=map%5.5ld",Arg3,NewNum);
				sprintf (str,"%s\\%s\\maps.txt",MSDir,Arg2);
				AppendFile (str,str2); 
				{
					BOOL SaveSaveZoom = SaveZoom, SaveSaveGlobals = SaveGlobals, rtn;
				
					SaveZoom = TRUE;
					SaveGlobals = TRUE;
					sprintf (str2,"%s\\%s\\map%5.5ld.gmc",MSDir,Arg2,NewNum);
					SetConfig(1);
					rtn = SaveConfig (str2,TRUE);
					SaveZoom = SaveSaveZoom;
					SaveGlobals = SaveSaveGlobals;
					if (rtn)
						goto RtnTrue;  
		        }
			} 
			else if (!_fstricmp (Arg1,"LOADMAP"))
			{
				sprintf (str,"%s\\%s\\%s.gmc",MSDir,Arg2,Arg3);
				if (!ExistFile (str))
					goto RtnFalse;
	            ForceBounds = FALSE;
				SaveZooms (0);
		        UnallocateConfig ();
	    		_fstrcpy (CfgName,str);
				CheckForContinue(TRUE, 0);
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
			} 
			goto RtnTrue;
		}
		
		case 625: // $SUBSET(CD Name)returns null if CD not installed or %INSTALLED% if installed
		{				
			hMem = GSSiGlobAlloc ( 875,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (CDIsInstalled (Arg1))   
				_fstrcpy (OutLoc,"%INSTALLED%");
			else
				*OutLoc = 0;
			goto Rtnl;
		}  
		
		case 626: // $LENGTH (coord pair)
		{				
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			Bounds = atobounds(Arg[1], &Err);
			if (Err)   
				*OutLoc = 0; 
			else
			{
				Dist = LDIST (Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymx);
				sprintf (OutLoc,"%.14lg",Dist);
			}
			goto Rtnl;
		}  
		
		case 627: //$RUNSQL ()
        {   
        	LPSTR pStatusText, pLoopText=0; 
        	HANDLE	hDB=0;
        	long	nRecs=0, TotRecs=0; 
        	HCURSOR	hcurSave=0;
        	
			nArgs = GetFunArgs (Args,Arg,-4,&hMem, pBrkPt, bpOffset, bpLen);  
			if (nArgs < 2)
				goto RtnFalse;
			ExpandText (Arg[1]);
            if ((pStatusText = _fstrchr (Arg[1],'!')))
            	*pStatusText++ = 0; 
            if (*Arg[2] == '(' && *LastChr(Arg[2]) == ')')
				*LastChr (Arg[2]++) = 0;
			ExpandText (Arg[2]);
            if (!OpenDataFile (Arg[1],Arg[2],BT_READ,&hDB))
            	goto RtnFalse;
			hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
			if (pStatusText)
			{   
				if ((pLoopText = _fstrrchr (pStatusText,'!')))
					*pLoopText++ = 0;
		        while (FetchDBRec (hDB))
		        	TotRecs++;
				CloseDataFile (FALSE,&hDB);
				OpenDataFile (Arg[1],Arg[2],BT_READ,&hDB);
				GSSiSetCursor (hcurSave);
				CreateStatusWind (CurView->hWnd,1,pStatusText);
		    } 
/*	        SQLPtr = (LPOPENSQLDATA) GlobalLock (hDB);
	        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		    i = FilePtr->FileHandle;    
            GlobalUnlock (SQLPtr->OFHandle);
            GlobalUnlock (hDB);
			SetContinueProcessing ( ExternalSQLDirect (i,Arg2); */
            if (nArgs > 2)
            {
		        while (ContinueProcessing && FetchDBRec (hDB))
		        {   
		        	_fstrcpy (Arg[4],Arg[3]);
		        	ExpandText (Arg[4]); 
		        	nRecs++;
		            if (pStatusText)
						StatusWindowUpdate (0,pLoopText,TotRecs,nRecs);
	            }
	        }
	        else 
	        {
	        	FetchDBRec (hDB);
	        	if (ContinueProcessing)
	        		nRecs = 1;
	        }
            SetContinueProcessing(TRUE);
			CloseDataFile (FALSE,&hDB);
            if (pStatusText)
				DestroyStatusWindow(0); 
			else 
				GSSiSetCursor (hcurSave);
			ltoa (nRecs,OutLoc,10);  
            goto Rtnl;
		}
		
		case 628: // $SETELV(point,elv,surfacehandle)
		{	 
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			GetDistAndUnits (Arg[2],&Elevation,&n,TRUE);
			hSurf = (HANDLE)atol (Arg[3]); 
			if (!hSurf)
			{
				HANDLE	hSQL = GetDBByIDName (Arg[3]);   
				
				if (hSQL)
					hSurf = GetDBHandleFromSQL (hSQL);    
			}
			Point = atopt (Arg[1],&Err);  
			if (SetNGIELV (Point,Elevation,hSurf,n))
				goto RtnTrue;
			goto RtnFalse; 
		}
		
		case 629:	//$MARKER(Point,Size,Color,Text,TextSize)
		{
        	BOOL SaveDM = DisplayMarkers;

			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
        	Point = atopt (Arg[1],&Err); 
        	if (Err)
        		goto RtnFalse; 
        	n = atoi (Arg[2]);  
			Color = atoi (Arg[3]);
        	RVal = atof (Arg[5]);
        	DisplayMarkers = TRUE;  
			DisplayMarker (Point,n,Arg[4],RVal,0,Color,atob(Arg[7]),FALSE,0,0,0,0,0);
			DisplayMarkers = SaveDM; 
			goto RtnTrue;
		}
  
		case 630:	//$RENAME(toname,fromname)
		{
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
        	if (GSSiRename (Arg[2],Arg[1]))
				goto RtnTrue;
			goto RtnFalse;
		}
  
		case 631:	//$STATUS(Create,Title,AllowBlock,AllowLine2)
					//$STATUS(Update,tot,n,info)
					//$STATUS(Destroy)
		{
			int	nlines=1;
			static HWND MessageWindow = 0;

			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			switch (*Arg[1])
			{
				case 'C':
					if (atob(Arg[4]))
						nlines = 2;
					if (atob(Arg[3]))  
					{
						long	SaveCurrentMacro=CurrentMacro;
						
						CurrentMacro=0;
						CreateStatusWind (CurView->hWnd,-nlines,Arg[2]);  
						CurrentMacro = SaveCurrentMacro;
					}
					else
						CreateStatusWind (CurView->hWnd,nlines,Arg[2]);
					break;
				case 'D':
					DestroyStatusWindow(0); 
					break;
				case 'U':
					hNum = atol (Arg[2]);
					nlong = atol (Arg[3]);
					if (!StatusWindowUpdate (0,Arg[4],hNum,nlong))
					{
						DestroyStatusWindow(0); 
						goto RtnFalse;
					}
					break; 
				case 'S':
					hNum = atol (Arg[2]);
					nlong = atol (Arg[3]);
					StatusWindowUpdate2 (Arg[4],hNum,nlong);
					break; 
				case 'M':
					if (MessageWindow)
						DestroyWindow(MessageWindow);
					MessageWindow = 0;
					if (*Arg[2])
						MessageWindow = CreateGoogleMessage(Arg[2]);
					break;
				default:
					goto RtnFalse;
			}
			if (ContinueProcessing)
				goto RtnTrue;
			SetContinueProcessing ( TRUE);
			goto RtnFalse;
		}
  
		case 632:	//$ZOOMVP(vpname,immediate(TorF),type,...
		{   
			short	VPID;
			BOOL	noDisplay = FALSE;

			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				goto RtnFalse;
			if ((VPID = GetVPIDFromName (Arg[1])))
			{
				if (!stricmp(Arg[2], "-1"))
				{
					noDisplay = TRUE;
					Immediate = TRUE;
				}
				else
					Immediate = atob(Arg[2]); 
				if (!_fstricmp(Arg[3],"BOUNDS"))
				{
					Bounds = atobounds (Arg[4],&Err);
					if (!Err)
					{   
						SetViewport (VPID); 
						if (CurViewActive())
						{
							int saveNumFiles = CurView->NumFiles;

						    CurView->CurZoomAreaRef = 0;
							if (noDisplay)
								CurView->NumFiles = 0;
							ZoomToRect(Bounds,Immediate);
							CurView->NumFiles = saveNumFiles;
							SetCurView ( SaveVP);
							goto RtnTrue;
						} 
					}
				} 
				SetCurView ( SaveVP);
			}
			goto RtnFalse;
		}
		
		case 633:	//$IDPOLY()...
		{
			if (IDPolygons ())
				goto RtnTrue;
			goto RtnFalse;
		}   

		case 634:	//$TOGGLE(value)
		{
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
        	if (atob (Arg[1]))
				goto RtnFalse;
			goto RtnTrue;
		}
  
		case 635:	//$FIELDS(datafile)
		{
        	HANDLE	hDB=0;  
			int iType = -2;
        	
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (!(hDB = GetOpenDatabaseFromID(Arg[1])))
			{
				if (!OpenDataFile(Arg[1], "", BT_READ, &hDB))
					goto RtnFalse;
			}
			else
				iType = 2;
			if (*Arg[2])
			{
				rtn = GetFieldList (CurView->hWnd,hDB,0,Arg[3],Arg[2]);
				goto Rtnrtn;
			}
			else
			{
				if (DisplayFieldList (CurView->hWnd,hDB,0,iType,Arg[3]))
					goto RtnTrue;
				else
				{
					CloseDataFile (FALSE,&hDB);
					goto RtnFalse;
				}
			}

		}
  
		case 636: //$CURSOR(WAIT,1 or -1)
		{
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;  
			if (!_fstricmp (Arg[1],"WAIT"))
			{
				WaitCursor (atoi(Arg[2]));
			    	goto RtnTrue;
			}
			goto RtnFalse;
		} 

		case 637: //$CANZIP(FROM,3dig can FSA)
				  //$CANZIP(TO,numeric zipcode)
		{   
			char	canzip[4];
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;  
			if (!_fstricmp (Arg[1],"FROM")) 
			{
				nlong = CanZipCharToNum (Arg[2]);
				ltoa (nlong,OutLoc,10);
				goto Rtnl;
			}
			if (!_fstricmp (Arg[1],"TO"))
			{  
				nlong = atol (Arg[2]);
				CanZipNumToChar (nlong,OutLoc);
				goto Rtnl;
			}
			goto RtnFalse;
		} 

		case 638: //$WINDOW(win,SETTEXT,text)

		{
			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!*Arg[1] || IsInteger (Arg[1]))
			{
				hWnd = (HWND)atol (Arg[1]);
				if (!hWnd)
					hWnd = hWndMain;
			}
			else
			{
				hWnd = FindWindowByName (Arg[1]);
				if (!hWnd)
					goto RtnFalse;
			}
			if (nArgs == 1)
			{
				ltoa((int)hWnd, OutLoc, 10);
				goto Rtnl;
			}
			if (!stricmp(Arg[2], "COMMANDSTRING"))
			{
				char fileName[MAX_PATH + 32];
				GetTempPath(MAX_PATH, fileName);

				sprintf(strchr(fileName, 0), "GF_PROCESS_CMD_FROMOTHERWINDOW.bin");
				HANDLE hFile = OpenFileGM(fileName, 0, OF_CREATE);
				int ln = strlen(Arg[3]) + 1;
				BigWrite64(hFile, &ln,sizeof(int), 0);
				BigWrite64(hFile, Arg[3], ln, 0);
				FlushFileBuffers(hFile);
				GSSiClose64(&hFile);
				SendMessage(hWnd, GF_PROCESS_CMD_FROMOTHERWINDOW, 0,0);
				goto RtnTrue;
			}
			if (!stricmp(Arg[2], "SETTEXT"))
			{
				if (hWnd)
					SetWindowText(hWnd, Arg[3]);
			}
			else if (!stricmp(Arg[2], "COMMAND"))
				SendMessage(hWnd, WM_COMMAND, atol(Arg[3]), 0L);
			else if (!stricmp(Arg[2], "PARENT"))
			{
				HWND parent = GetParent(hWnd);
				ltoa((int)parent, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[2], "TOPPARENT"))
			{
				HWND parent = GetTopParent(hWnd);
				ltoa((int)parent, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[2], "GETTEXT"))
			{
				GetWindowText(hWnd, OutLoc, 256);					 
				goto Rtnl;
			}
			else if (!stricmp(Arg[2], "MESSAGE"))
			{
				HANDLE hMessage = GSSiGlobAlloc(0, GMEM_MOVEABLE, strlen(Arg[3]) + 4);
				LPSTR pMessage = GlobalLock(hMessage);
				strcpy(pMessage, Arg[3]);
				GlobalUnlock(hMessage);
				PostMessage(hWnd, GSSI_WINDOW_MESSAGE, (WPARAM)hMessage, 0);
			}
			else if (!stricmp(Arg[2], "POSITION"))
			{
				int x = atoi(Arg[3]);
				int y = atoi(Arg[4]);
				int w = atoi(Arg[5]);
				int h = atoi(Arg[6]);
				if (w && h)
					SetWindowPos(hWnd, HWND_TOP, x, y, w, h, 0);
			}
			else if (!stricmp(Arg[2], "MOVE"))
			{
				Rect = atorect(Arg[3], &Err);
				if (!Err)
					MoveWindow(hWnd, Rect.left, Rect.top, RECTWIDTH(&Rect), RECTHEIGHT(&Rect), TRUE);
			}
			else if (!stricmp(Arg[2], "HIDE"))
				ShowWindow(hWnd, SW_HIDE);
			else if (!stricmp(Arg[2], "SHOW"))
				ShowWindow(hWnd, SW_SHOW);
			else if (!stricmp(Arg[2], "SHOWTOP"))
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			else if (!stricmp(Arg[2], "HIDEALL"))
				ShowHideWindows(Arg[1], SW_HIDE);
			else if (!stricmp(Arg[2], "SHOWALL"))
				ShowHideWindows(Arg[1], SW_SHOW);
			else if (!stricmp(Arg[2], "SHOWCHILDREN"))
				ShowHideChildren(hWnd, SW_SHOW);
			else if (!stricmp(Arg[2], "HIDECHILDREN"))
				ShowHideChildren(hWnd, SW_HIDE);
			else if (!stricmp(Arg[2], "HIDEPAR"))
			{
				HWND parent = GetParent(hWnd);
				ShowWindow(parent, SW_HIDE);
			}
			else if (!stricmp(Arg[2], "HIDETOPPAR"))
			{
				HWND parent = GetTopParent(hWnd);
				ShowWindow(parent, SW_HIDE);
			}
			else if (!stricmp(Arg[2], "SHOWPAR"))
				SetWindowPos(GetTopParent(hWnd), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

			//ShowWindow(GetTopParent(hWnd), SW_SHOW);
			else if (!stricmp(Arg[2], "MINIMIZE"))
				ShowWindow(hWnd, SW_FORCEMINIMIZE);
			else if (!stricmp(Arg[2], "MAXIMIZE"))
				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			else if (!stricmp(Arg[2], "RESTORE"))
				ShowWindow(hWnd, SW_RESTORE);
			else if (!stricmp(Arg[2], "ACTIVE"))
				SetActiveWindow(hWnd);
			else if (!stricmp(Arg[2], "FOCUS"))
				SetFocus(hWnd);
			else if (!stricmp(Arg[2], "CLEAR"))
				ClearFullWindowBitmap(hWnd);
			goto RtnTrue;
		} 

		case 639: //$SCREEN(SAVE,n) or (RESTORE,n)
		{
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			rtn = FALSE;
			if (!nArgs)
			{
				for (i=0;i<MAXSCREENS;i++)
					GSSiDeleteObject (&hScreenBM[i]);
				rtn = TRUE;
			}
			else if (!stricmp (Arg[1],"TEST"))
			{
				HDIB32 hDibbmp,hDibpng,hDibDeflate;
				LPSTR  pBitsbmp, pBitspng, pBitsDeflate;
				int	iSize;

				hDibbmp = BMPHandleFromEXT ("c:\\temp\\test_new.bmp"); 
				hDibpng = BMPHandleFromEXT ("c:\\temp\\test_new.png"); 
				hDibDeflate = BMPHandleFromEXT ("c:\\temp\\test_new_tif.bmp"); 
				pBitsbmp = FreeImage_GetBits (hDibbmp);
				pBitspng = FreeImage_GetBits (hDibpng);
				pBitsDeflate = FreeImage_GetBits (hDibDeflate);

				iSize = FreeImage_GetDIBSize (hDibbmp);
				iSize = FreeImage_GetDIBSize (hDibpng);
				iSize = FreeImage_GetDIBSize (hDibDeflate);
				ii = memcmp (pBitsbmp,pBitspng,iSize);
				ii = memcmp (pBitsbmp,pBitsDeflate,iSize);
				GMDestroyDIB32 (hDibbmp); 
				GMDestroyDIB32 (hDibpng); 
				GMDestroyDIB32 (hDibDeflate); 
			}
			else if (!stricmp (Arg[1],"SAVE"))
			{
				if (strchr (Arg[2],'\\'))
				{
					GdiFlush();
					GetWindowRect(hWndMain,&Rect); 
					HDC hDC = GetDC(hWndMain);
					HBITMAP hScreen = SaveScreen(hDC, Rect);
					ReleaseDC(hWndMain, hDC);
					HDIB32 hDIB32 = BitmapToDIB32(hScreen);
					DeleteObject(hScreen);
					LPBITMAPINFOHEADER pDibInfo = (LPBITMAPINFOHEADER)GetDibHeader(hDIB32);
					HDIB32 hDIB24 = GSSiFreeImage_ConvertTo24Bits(hDIB32);
					SaveDIB32 (hDIB24,Arg[2], 0, 0);
					GMDestroyDIB32 (hDIB32);
					GMDestroyDIB32 (hDIB24);
				}
				else
				{
					n = atoi (Arg[2]);
					if (n > 0 && n < MAXSCREENS+1)
					{
						n--;
						GSSiDeleteObject (&hScreenBM[n]);
						GetWindowRect(hWndMain,&Rect);
						hDC = GetDC (CurView->hWnd);
						hScreenBM[n] = SaveScreen (hDC,Rect); 
						ReleaseDC (CurView->hWnd,hDC);
						rtn = TRUE;
					}
				}
			}
			else if (!stricmp (Arg[1],"RESTORE"))
			{
				GetWindowRect(hWndMain,&Rect);
				hDC = GetDC (CurView->hWnd);
				SetDisplayMode (hDC, GF_SCREENMODE); 
				SelectClipRgn (hDC,0);
				FillRect (hDC,&Rect,GetStockObject (LTGRAY_BRUSH));
				n = atoi (Arg[2]);
				if (n > 0 && n < MAXSCREENS+1 && hScreenBM[--n])
				{
					RestoreScreen (hDC, hScreenBM[n],Rect);
					rtn = TRUE;
				}
				ReleaseDC (CurView->hWnd,hDC);
			}
			else if (!stricmp (Arg[1],"PUSH"))
			{
				n = atoi (Arg[2]);
				if (n > 0 && n < MAXSCREENS && hScreenBM[--n])
				{
					GSSiDeleteObject (&hScreenBM[MAXSCREENS-1]);
					for (i=MAXSCREENS-1;i>n;i--)
						hScreenBM[i] = hScreenBM[i-1];
					hScreenBM[i] = 0;
					rtn = TRUE;
				}
			}
			goto Rtnrtn;
		}

		case 640: //THREAD(START,threadtype,
		{
			static	arg[128];

			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!nArgs)
				goto RtnFalse;
			if (!stricmp (Arg[1],"START"))
			{
				if (!stricmp (Arg[2],"CACHE"))//filelistpath,cachepath,datalocpath
				{
					static	HANDLE hArgs;
					LPSTR	arg1,arg2,arg3;

					hArgs = GSSiGlobAlloc (9999,GMEM_MOVEABLE,4096*3);
					arg1=GlobalLock (hArgs);
					arg2=arg1+4096;
					arg3=arg2+4096;

					strcpy (arg1,Arg[3]);
					strcpy (arg2,Arg[4]);
					strcpy (arg3,Arg[5]);
					GlobalUnlock (hArgs);
					hCacheThread = (HANDLE)_beginthread( BackgroundCache, 0, &hArgs);
					ii=SetThreadPriority (hCacheThread,THREAD_PRIORITY_LOWEST);
					if (!ii)
						ii=0;
					itoa ((int)hCacheThread,OutLoc,10);

				}
			}
			if (!stricmp (Arg[1],"STOP"))
			{
				if (!stricmp (Arg[2],"CACHE"))
				{
					//hCacheThread = _endthread(hCacheThread);
				}
			}
			goto Rtnl;
		}
		case 642: //$BUTTON(LU or LD or RU or RD,client pos (blank for cursor pos)
			{
				nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
				
				Err = FALSE;
				if (nArgs == 2)
					Point16 = atopt16 (Arg[2], &Err);

    			if (Err)
					goto RtnFalse;
				PostMessage (CurView->hWnd,WM_LBUTTONDOWN,0,MAKELONG(Point16.x,Point16.y)); 
				PostMessage (CurView->hWnd,WM_LBUTTONUP,0,MAKELONG(Point16.x,Point16.y)); 
				goto RtnTrue;
			}
		case 643: //$BITMAP(DISPLAY(onlyinreport),pathname,width,height)
			{
				float	width, height;
				BOOL	wantPickBox;
				HDIB32	hDIB32;
				RECT	rect;
				
				rtn = FALSE;
				nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen); 
				if (!stricmp (Arg[1],"SPLIT"))
				{
					if (SplitBitmap (Arg[2],atoi(Arg[3]),atoi(Arg[4]),Arg[5],Arg[6],atoi(Arg[7])))
						goto RtnTrue;
					goto RtnFalse;
				}
				if (!stricmp (Arg[1],"FIX"))
				{
					if (FixBitMapHeader (Arg[2]))
						goto RtnTrue;
					goto RtnFalse;
				}
				if (!stricmp (Arg[1],"FLIP"))
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						if (FreeImage_FlipVertical (hDIB))
						{
							GMFIBMPHandleToEXT (Arg[2],hDIB,atoi(Arg[3]));
							goto RtnTrue;
						}
						GSSiFreeImage_Unload(hDIB);
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "HASBACKGROUNDCOLOR"))
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);

					if (hDIB)
					{
						rtn = FreeImage_HasBackgroundColor(hDIB);
						GSSiFreeImage_Unload(hDIB);
						goto Rtnrtn;
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "ISTRANSPARENT"))
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);

					if (hDIB)
					{
						rtn = FreeImage_IsTransparent(hDIB);
						GSSiFreeImage_Unload(hDIB);
						goto Rtnrtn;
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "RESCALE"))//file,width,height,outfile,opt
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);

					if (hDIB)
					{
						int width = atoi(Arg[3]);
						int height = atoi(Arg[4]);
						HDIB32 hDibOut = FreeImage_Rescale(hDIB, width, height, FILTER_CATMULLROM);

						if (hDibOut)
						{
							GMFIBMPHandleToEXT(Arg[5], hDibOut, atoi(Arg[6]));
							GSSiFreeImage_Unload(hDIB);
							GSSiFreeImage_Unload(hDibOut);
							goto RtnTrue;
						}
						GSSiFreeImage_Unload(hDIB);
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "SINGLECHANNEL"))//file,channel,outfile,opt
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);

					if (hDIB)
					{
						int ichan = atoi(Arg[3]);
						HDIB32 hDibOut = FreeImage_GetChannel(hDIB, ichan);

						if (hDibOut)
						{
							GMFIBMPHandleToEXT(Arg[4], hDibOut, atoi(Arg[5]));
							GSSiFreeImage_Unload(hDIB);
							GSSiFreeImage_Unload(hDibOut);
							goto RtnTrue;
						}
						GSSiFreeImage_Unload(hDIB);
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "WIDTH"))//infile
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						int width = FreeImage_GetWidth (hDIB);
						GSSiFreeImage_Unload(hDIB);
						itoa (width,OutLoc,10);
						goto Rtnl;
					}
				}
				else if (!stricmp(Arg[1], "HEIGHT"))//infile
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);

					if (hDIB)
					{
						int height = FreeImage_GetHeight(hDIB);
						GSSiFreeImage_Unload(hDIB);
						itoa(height, OutLoc, 10);
						goto Rtnl;
					}
				}
				else if (!stricmp(Arg[1], "PCTCOLOR"))//$BITMAP(PCTCOLOR,bitmappath,color,maskpath,maskcolor)
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);
					COLORREF bmColor = atoi(Arg[3]);
					strcpy(OutLoc, "-5");
					if (hDIB)
					{
						HDIB32 hDIBMask = BMPHandleFromEXT(Arg[4]);
						if (hDIBMask)
						{
							COLORREF maskColor = atoi(Arg[5]);
							double pct = GetPCTColorInBitmapWithMask(hDIB, hDIBMask, bmColor, maskColor);
							GSSiFreeImage_Unload(hDIBMask);
							ftoa(OutLoc, pct);
						}
						GSSiFreeImage_Unload(hDIB);
						goto Rtnl;
					}

					goto Rtnl;
				}

				else if (!stricmp(Arg[1], "OVERLAP"))//$BITMAP(OVERLAP,outpath,inpaths(sep by ;),color)
				{
					int rtn = CreateOverlapMap(Arg[2], Arg[3], atoi(Arg[4]));
					itoa(rtn, OutLoc, 10);
					goto Rtnl;
				}
				else if (!stricmp(Arg[1], "THUMBNAIL"))//infile,maxwidth,outfile,opt
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						int width = atoi (Arg[3]);
						HDIB32 hDibOut = FreeImage_MakeThumbnail (hDIB,width,TRUE);

						if (hDibOut)
						{
							GMFIBMPHandleToEXT (Arg[4],hDibOut,atoi(Arg[5]));
							GSSiFreeImage_Unload(hDIB);
							GSSiFreeImage_Unload(hDibOut);
							goto RtnTrue;
						}
						GSSiFreeImage_Unload(hDIB);
					}
					goto RtnFalse;
				}
				else if (!stricmp (Arg[1],"PASTE"))//file1,file2,left,top,outfile,alpha(opt))
				{
					HDIB32 hDIB1 = BMPHandleFromEXT (Arg[2]); 
					HDIB32 hDIB2 = BMPHandleFromEXT (Arg[3]); 

					if (hDIB1 && hDIB2)
					{
						int left = atoi (Arg[4]);
						int top = atoi (Arg[5]);
						int alpha = 256;

						if (*Arg[7])
							alpha = atoi (Arg[7]);
						if (FreeImage_Paste (hDIB1,hDIB2,left,top,alpha))
						{
							rtn = GMFIBMPHandleToEXT (Arg[6],hDIB1,atoi(Arg[8]));
							GSSiFreeImage_Unload(hDIB1);
							GSSiFreeImage_Unload(hDIB2);
							goto Rtnrtn;
						}
						GSSiFreeImage_Unload(hDIB1);
						GSSiFreeImage_Unload(hDIB2);
					}
					goto RtnFalse;
				}

				if (!CurReport)
					goto RtnFalse;
				if (CurReport->hScrollLine)
					goto RtnFalse;
				if (!CurReport->hDC)  
					goto RtnFalse;
				width = atof (Arg[3]);
				height = atof (Arg[4]);
				wantPickBox = atob(Arg[5]);
				hDIB32 = LoadDIB32(Arg[2],FALSE, 0);
				if (hDIB32)
				{
				    BITMAPINFOHEADER DibInfo;  
					int deviceWidth=0;

				    GetBitmapInfoFromHandle (&DibInfo,hDIB32);
					if (CurReport->hDC)
						deviceWidth = GetDeviceCaps (CurReport->hDC,HORZRES);
					if (width > 0.0 && width < 1.0)
						width *= RECTWIDTH (&CurReport->Rect);
					if (height > 0.0 && height < 1.0)
						height *= deviceWidth;
					if (!width && !height)
					{
						width = DibInfo.biWidth;
						height = DibInfo.biHeight;
					}
					else if (!width && height)
						width = (height * DibInfo.biWidth) / DibInfo.biHeight;
					else if (width && !height)
						 height= (width * DibInfo.biHeight) / DibInfo.biWidth;
					rect.left = CurReport->x;
					rect.left = CurReport->x;
					rect.top = CurReport->y;
					rect.right = rect.left + width;
					rect.bottom = rect.top + height;
					if (!CurReport->WantSize)
					{
						rtn = DisplayBMInRect32(CurReport->hDC, hDIB32, rect, TRUE);
						if (wantPickBox)
						{
							char macro[MAX_PATH + 64];
							int id = 0;
							if (CurView)
								id = CurView->ID;
							sprintf(macro, "$WEB(%s)", Arg[2]);
							PickBoxAdd(id, rect, macro);
						}
					}
					else
						rtn = TRUE;
					DestroyDIB32 (hDIB32,FALSE);
				}

				CurReport->x += width;
				CurReport->curLineHeight = max (height,CurReport->curLineHeight);

				goto Rtnrtn;
			}

		case 644: //$FORALL(RECORDS,file,sql,initalize,return,executable statements)
				  //$FORALL(FIELDS,file,type(def all),initalize,return,executable statements)
				  //$FORALL(DISTINCT,file,sql,initalize,return,value,executable statements)
		{
			nArgs = GetFunArgs (Args,Arg,-9,&hMem, pBrkPt, bpOffset, bpLen); 
			RunForAll(nArgs, Arg, OutLoc, pBrkPt, bpOffset, bpLen);
			goto Rtnl;
		}
		case 645: //$GOOGLE(GROUNDRES,base coord,zoomLev)
				  //$GOOGLE(SCALE,base coord,zoomlev)
		{
			
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);

			if (!stricmp(Arg[1], "GROUNDRES"))
			{
				DPOINT pt = atopt(Arg[2], &Err);
				ConvertCoord(&pt, 1, 2);
				RVal = GroundResolution(pt.y, atoi(Arg[3]));
				ftoa(OutLoc, RVal);
				goto Rtnl;
			}
			if (!stricmp(Arg[1], "PIXELXY"))
			{
				DPOINT pt = atopt(Arg[2], &Err);
				POINT  pixpt;
				ConvertCoord(&pt, 1, 2);
				int ilev = atoi(Arg[3]);
				LatLongToPixelXY(pt.y, pt.x, ilev, &pixpt.x, &pixpt.y);
				pttoa(OutLoc, pixpt);
				goto Rtnl;
			}
			if (!stricmp(Arg[1], "LATLON"))
			{
				DPOINT pt = atopt(Arg[2], &Err);
				DPOINT llpt;
				int ilev = atoi(Arg[3]);
				PixelXYToLatLongd(pt.x, pt.y, ilev, &llpt.y, &llpt.x);
				dpointtoa(OutLoc, &llpt);
				goto Rtnl;
			}
			if (!stricmp(Arg[1], "SCALE"))
			{
				DPOINT pt = atopt(Arg[2], &Err);
				int ilev = atoi(Arg[3]);
				DPOINT pt1, pt2;
				DPOINT ptp1, ptp2;
				double d;

				pt1 = pt2 = pt;
				ConvertCoord(&pt1, 1, 2);
				LatLongToPixelXYd(pt1.y, pt1.x, ilev, &ptp1.x, &ptp1.y);
				ptp1.x += 1000;
				PixelXYToLatLongd(ptp1.x, ptp1.y, ilev, &pt2.y, &pt2.x);
				//pt2.y = -pt2.y;
				ConvertCoord(&pt2, 2, 1);
				d = ldistp(pt, pt2);
				RVal = d / 1000;
				ftoa(OutLoc, RVal);
				goto Rtnl;
			}
			if (!stricmp(Arg[1], "ROWCOL"))
			{
				DPOINT pt = atopt(Arg[2], &Err);
				int ilev = atoi(Arg[3]);
				int row, col, googleRow;
				MNMXCORD bounds;
				double scale = GetGoogleTileBoundsFromPointAndZoom(ilev, pt, &bounds, &row, &col, &googleRow);

				sprintf(OutLoc, "%i|%i|%i", row, col, googleRow);
				goto Rtnl;
			}

			goto RtnFalse;
		}
		case 646: //$GMEDIT(file,TorF(create if new),TorF(save position))
		{
			LPSTR pcmd;
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (!ExistFile(Arg[1]))
			{
				if (atob(Arg[2]))
				{
					HFILE fid = GSSiOpenFile(Arg[1], 0, OF_CREATE);
					GSSiClose2 (&fid);
				}
				else
					goto RtnFalse;
			}
			pcmd = malloc(MAX_PATH * 2);
			sprintf(pcmd, "$SESSION(CREATE,GMEdit /GMEdit '%s' %i)", Arg[1],atob(Arg[3]));
			ExpandText(pcmd);
			free(pcmd);
			goto RtnTrue;
		}
		case 647: //$FIXMAP(gmdfile,pltfile)
		{
			LPSTR pcmd;
			int marker;

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 4)
				goto RtnFalse;
			RVal = atof(Arg[3]);
			marker = atoi(Arg[4]);
			rtn = FixMapCmd(Arg[1],Arg[2],RVal,marker);
			itoa(rtn, OutLoc, 10);
			goto Rtnl;
		}
		case 648: //$SQLITE(gmdfile,pltfile)
		{
			nArgs = GetFunArgs(Args, Arg, 16, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			rtn = SQLiteCmd(nArgs, Arg);
			itoa(rtn, OutLoc, 10);
			goto Rtnl;
		}

		case 649: //$INLIST(file,sql,var,addquotes)
		{
			HANDLE hDB=0;
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs < 3)
				goto RtnFalse;
			{
				HANDLE hFile = GSSiGlobAlloc(0, GMEM_MOVEABLE, 1024);
				LPSTR  pFile = GlobalLock(hFile);
				LPSTR  pVar = pFile + 300;
				LPSTR  pVal = pVar + 128;

				sprintf(pFile, "%INLIST=%s", Arg[1]);
				sprintf(pVar, "[%INLIST.%s]", Arg[3]);
				if (OpenDataFile(pFile, Arg[2], BT_READ, &hDB))
				{
					char delim[2] = { 0 };
					char quote[2] = { 0 };
					if (atob(Arg[4]))
						quote[0] = '\'';
					while (FetchDBRec(hDB))
					{
						strcpy(pVal, pVar);
						ExpandText(pVal);
						sprintf(strchr(OutLoc, 0), "%s%s%s%s", delim,quote, pVal,quote);
						delim[0] = ',';
					}
					CloseDataFile(FALSE, &hDB);
				}
				GSSiGlobUlFree(&hFile);
			}
			goto Rtnl;
		}
		case 650: //$LOWORD(val)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			rtn = atoi(Arg[1]);
			n = LOWORD(rtn);
			itoa(n, OutLoc, 10);
			goto Rtnl;
		}
		case 651: //$HIWORD(val)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			rtn = atoi(Arg[1]);
			n = HIWORD(rtn);
			itoa(n, OutLoc, 10);
			goto Rtnl;
		}
		case 652: //$NVCRIS(EXPORT,FromDB,BYINTorBYRAMP,LISTFILE(nullforALL),OUTFile,codesystem(0,1),headertype(0,1),wantPhotos(opt))
			//$NVCRIS(EXPORT, FromDB, BYRAMP, intID,rampNum,retired, OUTFile,codesystem(0,1),headertype(-1,0,1),wantPhotos(opt))
			//$NVCRIS(EXPORT, FromDB, ALL,OUTFile,codesystem(0,1),headertype(-1,0,1),completioncode(0all,1complete,2paid),wantPhotos(opt)
			//$NVCRIS(EXPORT,FromDB, INT,OUTFile,opt(0=all,1=withramps,2=paidonly),header(0=none and only intid,1=header and streetnames and coord wo type,2=same with type);
			//$NVCRIS(EXPORT,FromDB, PRIORITY,OUTFile,header(0=none,1=standard,2=with types));
			//$NVCRIS(LOADLIST,ListFile,ToDB)
			//$NVCRIS(COMPCODE,int,ramp,retired,db,codesystem(0,1))
			//$NVCRIS(OBSTRUCTIONCODE,obstruction)
			//$NVCRIS(TEXTURECODE,texture)
			//$NVCRIS(CODEFORVALUE,varname,varvalue)
			//$NVCRIS(FORMATSTREETS,codedstreets)
			//$NVCRIS(RAMPHEADER,headertype(0,1),OutFile(opt),wantPhotos(opt))
			//$NVCRIS(CREATEDATABASE,path,deleteexisting)
			//$NVCRIS(OPEN,path,createifnotexists,varname)
			//$NVCRIS(CLOSE,handle)
			//$NVCRIS(RAMPOFFSETCOORD,intPoint21,rampPoint21)
			//$NVCRIS(FIXRAMPNUM,rampnum)
			//$NVCRIS(PRINTRAMP,FromDB,intnum,rampnum)
			//$NVCRIS(PRINTRAMPLIST,FromDB,listpath)
			//$NVCRIS(STREETNAMES,FromDB,intnum)
			//$NVCRIS(CCODETOFILE, [%ARG(1)], [~TEMPFILE]);
			//$NVCRIS(CREATERAMPINDEX,dbpath);
			//$NVCRIS(DATATYPE,Curbramp or Sidewalk);

		{
			rtn = FALSE;
			nArgs = GetFunArgs(Args, Arg, 10, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "EXPORT"))
			{
				if (!stricmp(Arg[3], "BYINT"))
					rtn = OutputRampsForIntersectionsInListToFile(Arg[4], Arg[5], Arg[2], atoi(Arg[6]), atoi(Arg[7]), atob(Arg[8]));
				else if (!stricmp(Arg[3], "BYRAMP"))
					rtn = OutputRampToFile(atoi(Arg[4]), atoi(Arg[5]), atoi(Arg[6]), Arg[7], Arg[2], atoi(Arg[8]), atoi(Arg[9]), atob(Arg[10]));

					//rtn = OutputRampForIntersectionAndRampnumToFile(atoi(Arg[4]), atoi(Arg[5]), Arg[6], Arg[2], atoi(Arg[7]), atoi(Arg[8]));
				else if (!stricmp(Arg[3], "INT"))
					rtn = OutputIntsWithRampsToFile(Arg[4], Arg[2], atoi(Arg[5]), atoi(Arg[6]));
				else if (!stricmp(Arg[3], "ALL"))
					rtn = OutputAllRampsToFile(Arg[4], Arg[2], atoi(Arg[5]), atoi(Arg[6]), atob(Arg[8]));
					//rtn = OutputRampsToFile(Arg[4], Arg[2], atoi(Arg[5]), atoi(Arg[6]), atoi(Arg[7]));
				else if (!stricmp(Arg[3], "PRIORITY"))
					rtn = OutputPriorityLocToFile(Arg[4], Arg[2], atoi(Arg[5]));
			
			}
			else if (!stricmp(Arg[1], "DATATYPE"))//$NVCRIS(DATATYPE,Curbramp or Sidewalk)
			{
				rtn = SetNVCrisDataType(Arg[2]);
			}
			else if (!stricmp(Arg[1], "CCODETOFILE"))//$NVCRIS(CCODETOFILE,ccode,file)
			{
				rtn = CCodeToFile(Arg[2], Arg[3]);
			}
			else if (!stricmp(Arg[1], "UPDATEPICTID"))//$NVCRIS(UPDATEPICTID,sqlfile,oldsequence,newsequence)
			{
				rtn = UpdatePictureID(Arg[2], atoi(Arg[3]), atoi(Arg[4]));
			}
			else if (!stricmp(Arg[1], "LOADLIST"))//LOADLIST,fromListFile,toDB,showProgress,dbType,convertInsert,errFile(opt),addFileID,checkPointOpt(0=none,1=yes,2=only if no errors)
			{
				rtn = LoadFilesInListInChronologicalSequence(Arg[2], Arg[3], atob(Arg[4]), atoi(Arg[5]), atob(Arg[6]), Arg[7], atob(Arg[8]), atoi(Arg[9]));
			}
			else if (!stricmp(Arg[1], "COMPCODE"))
			{
				rtn = ComplianceCodeForRamp(atoi(Arg[2]), atoi(Arg[3]), atob(Arg[4]), Arg[5], atoi(Arg[6]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "CCCODES"))
			{
				int which = 0;
				if (!stricmp(Arg[2], "COMPUTE"))
					which = 1;
				ComputeCCCodes(atol(Arg[3]), atol(Arg[4]), atol(Arg[5]), which,OutLoc); // retrieves both summary and detail sep by |, if which 0 retrieves current , 1 computes new
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "FROMCODE"))
			{
				rtn = GetFromCodeText(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "FORMATSTREETS"))
			{
				rtn = FormatStreets(Arg[2], OutLoc);
				goto Rtnl;
			}

			else if (!stricmp(Arg[1], "OBSTRUCTIONCODE"))
			{
				rtn = NVCObstructionToCode(Arg[2]);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "OBSTRUCTIONFROMCODE"))
			{
				ObstructionFromCode(atoi(Arg[2]),OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "TEXTURECODE"))
			{
				rtn = NVCTextureToCode(Arg[2]);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "TEXTUREFROMCODE"))
			{
				TextureFromCode(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "CONDITIONFROMCODE"))
			{
				ConditionFromCode(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "MATERIALFROMCODE"))
			{
				MaterialFromCode(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "BOULEVARDMATERIALFROMCODE"))
			{
				BoulevardMaterialFromCode(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "TEXTURELIST"))
			{
				GetTextureList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "MATERIALLIST"))
			{
				GetMaterialCodeList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "BOULEVARDMATERIALLIST"))
			{
				GetBoulevardMaterialCodeList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "CONDITIONLIST"))
			{
				GetConditionCodeList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "SIDEWALKPOINTTYPELIST"))
			{
				GetPointTypeList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "OBSTRUCTIONLIST"))
			{
				GetObstructionList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "RAMPTYPELIST"))
			{
				GetRampTypesList(OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "RAMPTYPEFROMCODE"))
			{
				RampTypeFromCode(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "POINTTYPEFROMCODE"))
			{
				PointTypeFromCode(atoi(Arg[2]), OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "CODEFORVALUE"))
			{
				GetRampCodeForValue(Arg[2], Arg[3], Arg[4], OutLoc);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "FIXVARNAME"))
			{
				ConvertRampDisplayFieldToDBField(Arg[2], 128);
				strcpy(OutLoc, Arg[2]);
				goto Rtnl;
			}
			
			else if (!stricmp(Arg[1], "CREATEDATABASE"))
			{
				rtn = NVCreateDB(Arg[2], atob(Arg[3]));
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "COPYDATABASE"))//$NVCRIS(COPYDATABASE,fromdb,todb
			{
				rtn = NVCreateDB(Arg[3],TRUE);
				if (rtn)
				{
					rtn = NVCopyDB(Arg[2], Arg[3]);
				}
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "INIT"))
			{
				CRAPI_Init();
				rtn = CRAPI->sharedInstance.GSSiPadNumber < 0 ? FALSE : TRUE;
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "OPEN"))
			{
				rtn = NVOpenDB(Arg[2], atob(Arg[3]), Arg[4]);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "CLOSE"))
			{
				rtn = NVCloseDB(atol(Arg[2]));
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "CREATERAMPINDEX"))
			{
				rtn = NVCreateRampIndex(Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "CREATECCODES"))
			{
				rtn = NVCreateCCodes(Arg[2]);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "EXECUTE"))
			{
				rtn = executeAndSendCmd(DATABASEID_CURBRAMPS, Arg[3], FALSE);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "EXECUTEANDSEND"))
			{
				rtn = executeAndSendCmd(DATABASEID_CURBRAMPS, Arg[3], TRUE);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "FIXRAMPNUM"))
			{
				rtn = fixRampNum(atoi(Arg[2]));
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "GETCORNER"))
			{
				rtn = getMiddleRampIDFromRampID(atoi(Arg[2]));
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "PRINTRAMP"))
			{
				int intnum = atoi(Arg[3]);
				int rampnum = atoi(Arg[4]);
				rtn = PrintCurbRamp(Arg[2], intnum, rampnum);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "STREETNAMES"))
			{
				int intnum = atoi(Arg[3]);
				GetIntersectionStreetNames(Arg[2], intnum, OutLoc,0);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "RAMPOFFSETCOORD"))
			{
				BOOL err;
				POINT int21 = atopt16(Arg[2], &err);
				POINT ramp21 = atopt16(Arg[3],&err);
				rtn = getRampOffsetCoord(ramp21,int21);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "RAMPHEADER"))//
			{
				const char * pHeader = rampToTextHeader(atoi(Arg[2]),atob(Arg[4]));
				if (!*Arg[3])
				{
					strcpy(OutLoc, pHeader);
				}
				else
				{
					Fid1 = GSSiOpenFile(Arg[3], 0, OF_CREATE);
					fputstring((LPSTR)pHeader, Fid1);
					GSSiClose2 (&Fid1);
					strcpy(OutLoc, "1");
				}
				goto Rtnl;
			}

			if (rtn)
				goto RtnTrue;
			else
				goto RtnFalse;
		}
		case 653: //$LASZIP(GETBOUNDS,file,boundsvar)
		{
			rtn = FALSE;

			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "GETBOUNDS"))
			{
				double zmn, zmx;
				rtn = getLAZMinMax(Arg[2], &Bounds.xmn, &Bounds.xmx, &Bounds.ymn, &Bounds.ymx, &zmn, &zmx);
				if (*Arg[3])
					SetGlobalValueBounds(Arg[3], &Bounds);
				goto Rtnrtn;
			}
			if (!stricmp(Arg[1], "GETBOUNDS3D"))
			{
				MNMXCORD3D Bounds3D;
				rtn = getLAZMinMax(Arg[2], &Bounds3D.xmn, &Bounds3D.xmx, &Bounds3D.ymn, &Bounds3D.ymx, &Bounds3D.zmn, &Bounds3D.zmx);
				if (*Arg[3])
					SetGlobalValueBounds3D(Arg[3], &Bounds3D);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "LAZTOTEXT"))//$LAZTOTEXT(infile,outfile,pointtype)
			{
				rtn = writeLAZFileToText(Arg[2], atoi(Arg[4]), Arg[3]);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "CLASSIFY"))//$LAZTOTEXT(CLASSIFY,infile,outfile)
			{
				rtn = classifyLAZFile(Arg[2], Arg[3]);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "DTMFROMLAZ"))//$LAZTOTEXT(CLASSIFY,infile,outfile)
			{
				rtn = DTMFromLAZFile(Arg[2], Arg[3]);
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else
				rtn = mainlaszip(nArgs, &Arg[1]);
			goto RtnFalse;
		}
			break;
		case 654: //$UNIQUE(CREATE,len)
			//$UNIQUE(ADD,handle,val)
			//$UNIQUE(GET,handle,first,valvar,countvar)
			//$UNIQUE(DUMP,handle,path);
			//$UNIQUE(CLOSE,handle)
		{
			HANDLE hBT;
			int vlen;
			int count;
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "CREATE"))
			{
				vlen = atoi(Arg[2]);
				hBT = CreateUniqueList(vlen, 0);
				itoa((UINT)hBT, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "ADD"))
			{
				hBT = (HANDLE)atoi(Arg[2]);
				vlen = GetBTKeyLen(hBT);
				LPSTR value = malloc(vlen + 4);
				_fstrncpy(value, Arg[3], vlen);
				if (BT_FIND(hBT, value, BT_FIRST, BT_EQ, (LPSTR)&count))
					count = 0;
				count++;
				BT_PUT(hBT, value, (LPSTR)&count);
				free(value);
				goto RtnTrue;
			}
			else if (!stricmp(Arg[1], "DUMP"))
			{
				int count;
				rtn = FALSE;
				HFILE fid = GSSiOpenFile(Arg[3], 0, OF_CREATE);
				if (fid != HFILE_ERROR)
				{
					char Line[256];
					sprintf(Line, "VALUE\tCOUNT");
					fputstring(Line, fid);
					hBT = (HANDLE)atoi(Arg[2]);
					vlen = GetBTKeyLen(hBT);
					LPSTR value = malloc(vlen + 4);
					int pos = BT_FIRST;
					while (!BT_FIND(hBT, value, pos, BT_ANY, (LPSTR)&count))
					{
						pos = BT_NEXT;
						value[vlen] = 0;
						sprintf(Line, "%s\t%i",value,count);
						fputstring(Line, fid);
						rtn = TRUE;
					}
					free(value);
					GSSiClose(fid);
				}
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "GET"))
			{
				int count;
				hBT = (HANDLE)atoi(Arg[2]);
				vlen = GetBTKeyLen(hBT);
				LPSTR value = malloc(vlen + 4);
				int pos = BT_NEXT;
				if (atob(Arg[3]))
					pos = BT_FIRST;
				if (!BT_FIND(hBT, value, pos, BT_ANY, (LPSTR)&count))
				{
					value[vlen] = 0;
					SetGlobalValue(Arg[4], value);
					SetGlobalValueLong(Arg[5], count);
					rtn = TRUE;
				}
				else
					rtn = FALSE;
				free(value);
				goto Rtnrtn;
			}
			else if (!stricmp(Arg[1], "CLOSE"))
			{
				hBT = (HANDLE)atoi(Arg[2]);
				rtn = BT_CLOSEANDDELETE(&hBT);
				goto Rtnrtn;
			}
		}
			break;
		case 655: // $PROMPT(prompt text);
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

			DisplayPromptText(0, Arg[1]);
			goto RtnTrue;
		}
		case 656: // $VPNAME(FROMTHEMETITLE,title)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (!stricmp(Arg[1], "FROMTHEMETITLE"))
			{
				for (int iview = 0; iview < *pNumViewports; iview++)
				{
					if (pViewports[iview]->pTheme)
					{
						if (!stricmp(Arg[2], pViewports[iview]->pTheme->Title))
						{
							strcpy(OutLoc, pViewports[iview]->Name);
							break;
						}
					}
				}
			}
			goto Rtnl;
		}
		case 657: // $REMOVE(text,chartoremove) removes all special characters if 2nd arg 0
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

			RemoveCharacters (Arg[1],Arg[2]);
			strcpy(OutLoc, Arg[1]);
			goto Rtnl;
		}
		case 658: // $NONINT(value)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			LPSTR nonInt = FirstNonInt(Arg[1]);
			if (nonInt)
			{
				strcpy(OutLoc, nonInt);
			}
			goto Rtnl;
		}

		case 659: // $SETSUB(subname,sub)
		{
			nArgs = GetFunArgs(Args, Arg, -2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			strcpy(SubDef, Arg[2]);
			goto Rtnl;
		}
		case 660: // $RUNSUB(subname)
		{
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			LPSTR sub = malloc(4096);
			strcpy(sub, SubDef);
			ExpandText(sub);
			free(sub);
			goto Rtnl;
		}


		default:
			goto Rtn0;
	}
Rtnrtn:
	if (!rtn)
		goto RtnFalse;
RtnTrue: 
	l = 1;                
	_fstrcpy(OutLoc,"1");
	goto Exit;
RtnFalse: 
	l = 1;
	_fstrcpy(OutLoc,"0");
	goto Exit;
Rtn0:
	l = 0;
	goto Exit;
Rtnl:
	l = _fstrlen(OutLoc); 
Exit: 
	if (SaveCfg != CurrentConfig)
	{
		SetConfig(SaveCfg);
	}
	if (pNumViewports  && *pNumViewports)
	{
		SetCurView(SaveVP);
	}
	GSSiGlobUlFree (&hMem);
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc ( 914,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s",Args);
		GSSiTraceLev (lpstr,-1,2);
		GSSiGlobUlFree (&hMem);
	}
{
#if ENABLETRACE
GSSiExitProg (1348);
#endif
	return (l);
}
#if ENABLETRACE
}
#endif
}  

		
			
