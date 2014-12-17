#include "graphint.h"   
#include "extrndb.h"   
#include "pnet.h" 
#include "umio.h"
#include <mmsystem.h>
#include "gmextern.h"


static	char	MonthAbv[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static	BOOL	InAtPrint=FALSE;
static short	nSetVals=0;
static UINT	lSetVals=0;
static char	CurrentDialogType[16];
static char	DOW[7][10]={"SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY"};
static HIGHLIGHTDATA	HighlightData;
static THEMEHIGHLIGHTKEY	ThemeHighlightKey;
static THEMEHIGHLIGHTDATA	ThemeHighlightData;
static char	CurrentDialogFile[256]="";
#define MAXSCREENS	9
static HBITMAP	hScreenBM[MAXSCREENS]={0};
static HWND	hWndFound;
static HWND wantWnd;
static LPSTR	pFindWindowText=0;

typedef struct {
	DWORD process, thread;
}WINPROCESSANDTHREAD;
typedef WINPROCESSANDTHREAD *LPWINPROCESSANDTHREAD;

BOOL CALLBACK WEEnumWndProc(HWND hCtrl, LONG lParam)
{
	if (hCtrl == wantWnd)
	{
		hWndFound = hCtrl;
		return FALSE;
	}
	return TRUE;
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
	FARPROC lpfnEnumWndProc;
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
		EnumWindows(ShowEnumWndProc, (LPARAM)&wpt);
	else
		EnumWindows(HideEnumWndProc, (LPARAM)&wpt);
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
	char    txt[256];
	long	lUserData; 
	HWND	hPar;
	DWORD	ProcessID;
	    
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
    FARPROC lpfnEnumWndProc;
    
    hWndFound = 0;
	pFindWindowText = Text;
	EnumWindows (FWBPEnumWndProc,ProcessID);   
	return hWndFound;
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
						or	(GETCLASSVALS,vp name,
						or	(GETCLASS,vp name,value,activeclassesonly)
						or	(GEOCENTER,DISPLAY,vp name)
						or	(GEOCENTER,GET,vp name,class)
						or	(SHOWCLASSMEMBERS,vp name,classno,POINT or FLASH,fromPt,ALL or STEP,macro)
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
			else if (!_fstrcmp (Arg[1],"GEOCENTER")) 
			{   
				SetCurView ( SetVPFromName (Arg[3],&Err));
				if (!Err && CurView->pTheme)
				{
					if (!stricmp (Arg[2],"DISPLAY"))
						rtn = DisplayThemeGeoCenters (CurView->pTheme);
					else
					{
						Point = ComputeThemeClassGeoCenter (CurTheme,atoi (Arg[4]));
						dpointtoa (OutLoc,&Point);
						SetCurView ( SaveVP);
						goto Rtnl;
					}
				}
				else
					rtn = FALSE;   
				SetCurView ( SaveVP);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"ACT")) 
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
				if (SelectThemeClasses (-1))
					rtn = TRUE;
				else
					rtn = FALSE;
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"QUAN")) 
			{   
				SetCurView ( SetVPFromName (Arg[2],&Err));
				rtn = SetThemeQuan (CurView->pTheme,Arg[3]);
				goto Rtnrtn;
			}
			else if (!_fstrcmp (Arg[1],"SET")) 
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
						if (!_fstricmp (Arg[3],"CONTENTS"))
						{   
							rtn = SetThemeContents (CurView->pTheme,Arg[4]);   
							if (CurView->pTheme->hVisList)
							{
								GSSiGlobFree (&CurView->pTheme->hVisList);
								CurView->pTheme->hVisList = ReadVisList (&CurView->pTheme->Contents[1]);  
							}
						} 
						else if (!_fstricmp (Arg[3],"SETCOLOR"))
						{
							CurView->pTheme->NotSetColor = !atob (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"MISSOPT"))
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
						else if (!_fstricmp (Arg[3],"FACTOR"))
						{
							CurView->pTheme->FactorLegend = atob (Arg[4]);
							rtn = TRUE;
						}
						else if (!_fstricmp (Arg[3],"DEPTHOPTIONS"))
						{   
							rtn=FALSE;
							if (CurView->pTheme)
								rtn = DialogBox(hInst, (LPSTR)"DEPTHOPTIONS", CurView->hWnd, DEPTHOPTIONSMsgProc);
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
						if (!_fstricmp (Arg[3],"MISSOPT"))
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
					GSSiClose (Fid);
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
				GSSiClose (Fid);
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
				GSSiClose (Fid);			
				GSSiRemove (pName);
			}
			GSSiGlobFree (&hMacroArgs);
			goto Rtnl;
		}   
		 
		case 511: // $IMAGE(image file pathname)  
				  // $IMAGE(WINDOW,image file name)
				  // $IMAGE(SPLIT,imagefile,outdir,outtype,width,height)
		{				
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!nArgs)
			{
				if (hWndFullBM)
					PostMessage(hWndFullBM, WM_CLOSE, 0, 0L);

				goto RtnTrue;
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


			if (!_fstricmp (Arg[1],"WINDOW"))//$IMAGE(WINDOW,file,waitforkey,rect(opt))
			{
			    HDIB32	hDib32 = LoadDIB32(Arg[2],FALSE);
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
						GetWindowRect (hWndMain,&WindowRect);
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
			if (!_fstricmp (Arg[1],"SPLIT"))
			{
				rtn = SplitImage (Arg[2],Arg[3],Arg[4],atoi(Arg[5]),atoi(Arg[6]));
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"CONVERTCOLOR"))
			{
				MNMXCORD	rect=atobounds (Arg[3],&Err);
				
				if (!Err)
					rtn = ConvertBitmapColorsInRect (Arg[2],&rect,atoi(Arg[4]),atoi(Arg[5]),atob(Arg[6]));
				else
					rtn = -1;
				itoa (rtn,OutLoc,10);
				goto Rtnl;
			}
			if (!_fstricmp (Arg[1],"VIEW"))
			{
				rtn = ViewImage (CurView->hWnd,Arg[2]);
				goto Rtnrtn;
			}
			if ((pPar = strrchr (Arg[1],'(')))
				*pPar = 0;
			if (!ExistFile (Arg[1]))
			{
				GSSiMessageBox (Arg[1],"Unable to Access Image File",MB_ICONEXCLAMATION,0);
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
					sprintf (OutLoc,"$BITMAP(%s)",Arg[1]); 
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

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstrcmp(Arg[1], "ALL"))
			{   
				long	StartRef=LONG_MIN;
				
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
					long	StartRef=LONG_MIN;
		
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
					if (!BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)pHighlightData))
						goto HLTRemove;
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
		
		case 522: //$STRIP(string,char(opt-default is space)  
		{
			char	StripChr=' ';
			int	ln=2048;

			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse; 
			_fstrcpy (OutLoc,Arg[1]);
			if (nArgs == 2)
				STRIPR (OutLoc,&ln, Arg[2], strlen(Arg[2]));
			else
				Strip (OutLoc,StripChr);
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
		
		case 524: //$BASIC(file,sql,updatefieldlist,title,autoupdate,sqlfieldlist,displayrect)
		{
			nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			lpDB = Arg[1];
			lpSQL = Arg[2];
			lpUpdateFieldList = Arg[3];
			lpBasicTitle = Arg[4];
			if (*Arg[5])
				lpAutoUpdateFieldList = Arg[5];
			if (*Arg[6])
				lpSQLFieldList = Arg[6];
			displayRect = atorect (Arg[7],&Err);
		    {
		 	   FARPROC lpfnIDENTIFYMsgProc;
		         
		        BasicDisplayItem=-1; 
			    lpfnIDENTIFYMsgProc = MakeProcInstance((FARPROC)IDENTIFYMsgProc, hInst);
			    nRc = DialogBox(hInst, (LPSTR)"IDENTIFY", hWndMain, lpfnIDENTIFYMsgProc);
			    FreeProcInstance(lpfnIDENTIFYMsgProc);
		    }
			lpUpdateFieldList = 0;
			lpAutoUpdateFieldList = 0;
			lpSQLFieldList = 0;
			lpBasicTitle = 0;
		    goto RtnTrue; 
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
		            FARPROC lpfnVOTER_NAME_LOCMsgProc;
					int		nRc;
					
					if (nArgs > 2)
						InitVoterNameSearch (Arg[3],Arg[4],Arg[5],Arg[6]);
					else
						InitVoterNameSearch (0,0,0,0);
		            lpfnVOTER_NAME_LOCMsgProc = MakeProcInstance((FARPROC)VOTER_NAME_LOCMsgProc, hInst);
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
			FARPROC lpfnDBLOGINMsgProc;
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
			lpfnDBLOGINMsgProc = MakeProcInstance((FARPROC)LOGINMsgProc, hInst);
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
						GSSiClose (Fid2);
						GSSiGlobUlFree (&hFileCmp);
						GSSiGlobUlFree (&hFileDCmp);
						rtn = TRUE;
					}
					GSSiClose (Fid);
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
			AddBMPToCache32 (0,0);
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
				GSSiClose (Fid);
				BT_CLOSEANDDELETE (&hDupRef);
				BT_CLOSEANDDELETE (&hDupRef2);
				goto RtnTrue;
			}
			goto RtnFalse;
		}

		case 532: //$POINT(DISPLAY,id or coord,symbol,size,color,text)
		{
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen);
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

		case 538: //$POINTLIST(CREATE,name,pointlist)
				  //$POINTLIST(DESTROY,name)
				  //$POINTLIST(ADD,name,pointlist)
				  //$POINTLIST(THIN,name,dist(if 0 removes dup points))
				  //$POINTLIST(DISPLAY,name,FILL,color)
				  //$POINTLIST(DISPLAY,name,DRAW,color,width)
				  //$POINTLIST(LENGTH,name)
				  //$POINTLIST(AREA,name)
				  //$POINTLIST(AZM,name,pct,before;after;at(default)) at averages before and after if at node point
				  //$POINTLIST(INTERSECT,name,name2,COUNT;id;Farthest;nearest,farornearpoint)
		{
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (PListCommands (nArgs,Arg,OutLoc))
				goto Rtnl;
			goto RtnFalse;
		}

		case 601: /* $TAGLOC(Prefix,minchar,SaveGlobalName(optional-not in brackets),Title(opt),Viewport(opt),Layer(opt),locatetagonly(opt,T locates,)) Tag locator */
		{	 
            FARPROC lpfnTAGLOCMsgProc;
			int		nRc;
			
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
              lpfnTAGLOCMsgProc = MakeProcInstance((FARPROC)TAGLOCMsgProc, hInst); 
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
		 			CreatePanZoomRotTool (CurView->hWnd,atopt16 (Arg[3],0));
			}
		    else if (!stricmp (Arg[1],"REMOVE"))
			{
				if (!stricmp (Arg[2],"PANZOOMROTATE"))
		 			CreatePanZoomRotTool (0,atopt16 (Arg[3],0));
			}
		    else if (!_fstricmp (Arg[1],"STORMPIPE"))
            {
                  FARPROC	lpfnSTRMPIPEMsgProc; 
                  
		          if (hWndStrmPoint)
		          	SendMessage (hWndStrmPoint,WM_CLOSE,0,0);
                  if (!hWndStrmPipe)
                  {
	                  lpfnSTRMPIPEMsgProc = MakeProcInstance((FARPROC)STRMPIPEMsgProc, hInst);
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
                  FARPROC	lpfnSTRMPIPEMsgProc; 
                  
		          if (hWndStrmPoint)
		          	SendMessage (hWndStrmPoint,WM_CLOSE,0,0);
                  if (!hWndStrmPipe)
                  {
	                  lpfnSTRMPIPEMsgProc = MakeProcInstance((FARPROC)STRMPIPEMsgProc, hInst);
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
                  FARPROC	lpfnSTRMPOINTMsgProc; 
                  
		          if (hWndStrmPipe)
		          	SendMessage (hWndStrmPipe,WM_CLOSE,0,0);
                  if (!hWndStrmPoint)
                  {
	                  lpfnSTRMPOINTMsgProc = MakeProcInstance((FARPROC)STRMPOINTMsgProc, hInst);
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
                  FARPROC	lpfnUSERFORMMsgProc; 
                  
                  if (!hWndUserForm)
                  {
	                  lpfnUSERFORMMsgProc = MakeProcInstance((FARPROC)USERFORMMsgProc, hInst);
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
                  FARPROC	lpfnSTRMPOINTMsgProc; 
                  
		          if (hWndStrmPipe)
		          	SendMessage (hWndStrmPipe,WM_CLOSE,0,0);
                  if (!hWndStrmPoint)
                  {
	                  lpfnSTRMPOINTMsgProc = MakeProcInstance((FARPROC)STRMPOINTMsgProc, hInst);
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
				ScreenToClient (CurView->hWnd,&ScreenPoint);   
				sprintf (ptxt,"$REPORT(%s)",Arg[1]);
				YellowTextBox (hWnd,ptxt,ScreenPoint,&TBRect,0,TRUE,0);
				hDC = GetDC (hWnd);
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
				while (GSSiGetMessage(&msg, hWnd,0,0))
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
			if (GSSiMessageBox(Arg[1], " ", MB_YESNO, 0) == IDYES)
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
                  FARPROC	lpfnLOADDGNDUMPMsgProc; 

                  lpfnLOADDGNDUMPMsgProc = MakeProcInstance((FARPROC)LOADDGNDUMPMsgProc, hInst);
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
                  FARPROC	lpfnLOADSHPMsgProc; 
                  
                  lpfnLOADSHPMsgProc = MakeProcInstance((FARPROC)LOADSHPMsgProc, hInst);
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
                  FARPROC	lpfnLOADDXFMsgProc; 

                  lpfnLOADDXFMsgProc = MakeProcInstance((FARPROC)LOADDXFMsgProc, hInst);
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
                  FARPROC	lpfnPOINTMAPMsgProc; 
                  
                  lpfnPOINTMAPMsgProc = MakeProcInstance((FARPROC)POINTMAPMsgProc, hInst);
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
			      FARPROC lpfnLOADMDMsgProc;
                  
			      lpfnLOADMDMsgProc = MakeProcInstance((FARPROC)LOADMDMsgProc, hInst);
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
                  FARPROC	lpfnADDLOC_FROMADDMsgProc; 

				  
                  lpfnADDLOC_FROMADDMsgProc = MakeProcInstance((FARPROC)ADDLOC_FROMADDMsgProc, hInst);
			      nRc = DialogBox(hInst, (LPSTR)"ADDLOC_FROMADD", hWndMain, lpfnADDLOC_FROMADDMsgProc);
			      FreeProcInstance(lpfnADDLOC_FROMADDMsgProc);
                  *AutoExportName=0;
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
						lpChr = _fstrstr (lpChr+1,Arg[2]); 
						if (lpChr)
							LastLoc = lpChr;
					}
					else
						lpChr = 0;
				}
				while (lpChr); 
				lpChr = LastLoc;
			}
			else
				lpChr = _fstrstr (Arg[1],Arg[2]);
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
			ContinueProcessing = FALSE;
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
					// $BOUNDS(INC,B1,D) add D to bounds 
					// $BOUNDS(VISLIM) returns bounds of visible limits
					// $BOUNDS(VALID,BOUNDS) return 0 if not valid 1 if valid  
					// $BOUNDS(HLTLIM) highlight bounds
					// $BOUNDS(TRAN,BOUNDS,tranfile,direction (F(default)orR)
				    // $BOUNDS(CONVERT,BOUNDS,from,to)
					// $BOUNDS(TRANFILE,tranfile,direction (F(default)orR)  
					// $BOUNDS(WIDTH,BOUNDS) returns width
					// $BOUNDS(HEIGHT,BOUNDS) returns height  
					// $BOUNDS(MID,BOUNDS) returns midpoint 
					// $BOUNDS(MIN,BOUNDS) returns min point  
					// $BOUNDS(MAX,BOUNDS) returns max point 
				    // $BOUNDS(CONTAINS,BOUNDS,POINTorBOUNDS)
					// $BOUNDS(LAYER,layer name,vpname)
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
			else if (!_fstricmp (Arg[1],"INBOUNDS"))
			{   
				MNMXCORD	Bounds2;

				Bounds = atobounds (Arg[2],&Err);
				Bounds2 = atobounds (Arg[3],&Err);
				rtn = BoundsInBounds (&Bounds,&Bounds2,atoi(Arg[4]));
				goto Rtnrtn;
			} 
			else if (!_fstricmp (Arg[1],"VIEWPORT"))
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
			else if (!_fstricmp (Arg[1],"HEIGHT"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;  
				RVal = Bounds.ymx - Bounds.ymn;
				ftoa (OutLoc,RVal);
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
			else if (!_fstricmp (Arg[1],"MAX"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err || !ValidBounds (&Bounds))
					goto RtnFalse;  
				Point.x = Bounds.xmx;
				Point.y = Bounds.ymx;
				dpointtoa (OutLoc,&Point); 
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
			else if (!_fstricmp (Arg[1],"FACTOR"))
			{   
				 
				*OutLoc = 0;
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto Rtnl;
				RVal = atof (Arg[3]);
				Bounds = FactorBounds (&Bounds,RVal);
				boundstoa (OutLoc,&Bounds); 
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
			sprintf (OutLoc,"%Flf",AZ); 
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
		
		case 619: // $EXPAND(value)  
		{				
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			ExpandText(Arg[1]);
			_fstrcpy (OutLoc,Arg[1]);
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
	    	    GetClientRect (hWndMain,&Rect);
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
			hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight); 
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
			GSSiClose (Fid);
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
				FARPROC lpfnCREATE_MAPSETMsgProc;
				
				lpfnCREATE_MAPSETMsgProc = MakeProcInstance((FARPROC)CREATE_MAPSETMsgProc, hInst);
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
					GSSiMessageBox (MSDir,"Unable to create new map set",MB_ICONEXCLAMATION,0);
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
			ContinueProcessing = ExternalSQLDirect (i,Arg2); */
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
            ContinueProcessing = TRUE;
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

			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
        	Point = atopt (Arg[1],&Err); 
        	if (Err)
        		goto RtnFalse; 
        	n = atoi (Arg[2]);  
			Color = atoi (Arg[3]);
        	RVal = atof (Arg[5]);
        	DisplayMarkers = TRUE;  
			DisplayMarker (Point,n,Arg[4],RVal,0,Color,TRUE,FALSE,0,0,0,0,0);
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
				default:
					goto RtnFalse;
			}
			if (ContinueProcessing)
				goto RtnTrue;
			ContinueProcessing = TRUE;
			goto RtnFalse;
		}
  
		case 632:	//$ZOOMVP(vpname,immediate(TorF),type,...
		{   
			short	VPID;
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				goto RtnFalse;
			if ((VPID = GetVPIDFromName (Arg[1])))
			{
				Immediate = atob(Arg[2]); 
				if (!_fstricmp(Arg[3],"BOUNDS"))
				{
					Bounds = atobounds (Arg[4],&Err);
					if (!Err)
					{   
						SetViewport (VPID); 
						if (CurViewActive())
						{
						    CurView->CurZoomAreaRef = 0;
							ZoomToRect(Bounds,Immediate);
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
			if (!stricmp (Arg[2],"SETTEXT"))
			{
				if (hWnd)
					SetWindowText (hWnd,Arg[3]);
			}
			else if (!stricmp (Arg[2],"COMMAND"))
				SendMessage(hWnd, WM_COMMAND, atol(Arg[3]), 0L);
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
			else if (!stricmp(Arg[2], "HIDEALL"))
				ShowHideWindows(Arg[1], SW_HIDE);
			else if (!stricmp(Arg[2], "SHOWALL"))
				ShowHideWindows(Arg[1], SW_SHOW);
			else if (!stricmp(Arg[2], "SHOWCHILDREN"))
				ShowHideChildren(GetTopParent(hWnd), SW_SHOW);
			else if (!stricmp(Arg[2], "HIDECHILDREN"))
				ShowHideChildren(GetTopParent(hWnd), SW_HIDE);
			else if (!stricmp(Arg[2], "HIDEPAR"))
				ShowWindow(GetTopParent(hWnd), SW_HIDE);
			else if (!stricmp(Arg[2], "SHOWPAR"))
				SetWindowPos(GetTopParent(hWnd), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

				//ShowWindow(GetTopParent(hWnd), SW_SHOW);
			else if (!stricmp(Arg[2], "MINIMIZE"))
				ShowWindow(hWnd, SW_FORCEMINIMIZE);
			else if (!stricmp(Arg[2], "RESTORE"))
				ShowWindow(hWnd, SW_RESTORE);

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
					HDIB32 hDib32;

					GetWindowRect (hWndMain,&Rect); 
					hDib32 = CopyScreenToDIB32 (&Rect); 
					SaveDIB32 (hDib32,Arg[2],FIF_TIFF,TIFF_ADOBE_DEFLATE);
					GMDestroyDIB32 (hDib32); 
				}
				else
				{
					n = atoi (Arg[2]);
					if (n > 0 && n < MAXSCREENS+1)
					{
						n--;
						GSSiDeleteObject (&hScreenBM[n]);
						GetWindowRect (hWndMain,&Rect);
						hDC = GetDC (CurView->hWnd);
						hScreenBM[n] = SaveScreen (hDC,Rect); 
						ReleaseDC (CurView->hWnd,hDC);
						rtn = TRUE;
					}
				}
			}
			else if (!stricmp (Arg[1],"RESTORE"))
			{
				GetWindowRect (hWndMain,&Rect);
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
						FreeImage_Unload(hDIB);
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "HASBACKGROUNDCOLOR"))
				{
					HDIB32 hDIB = BMPHandleFromEXT(Arg[2]);

					if (hDIB)
					{
						rtn = FreeImage_HasBackgroundColor(hDIB);
						FreeImage_Unload(hDIB);
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
						FreeImage_Unload(hDIB);
						goto Rtnrtn;
					}
					goto RtnFalse;
				}
				else if (!stricmp(Arg[1], "RESCALE"))//file,width,height,outfile,opt
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						int width = atoi (Arg[3]);
						int height = atoi (Arg[4]);
						HDIB32 hDibOut = FreeImage_Rescale (hDIB,width,height,FILTER_CATMULLROM);

						if (hDibOut)
						{
							GMFIBMPHandleToEXT (Arg[5],hDibOut,atoi(Arg[6]));
							FreeImage_Unload(hDIB);
							FreeImage_Unload(hDibOut);
							goto RtnTrue;
						}
						FreeImage_Unload(hDIB);
					}
					goto RtnFalse;
				}
				else if (!stricmp (Arg[1],"WIDTH"))//infile
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						int width = FreeImage_GetWidth (hDIB);
						FreeImage_Unload(hDIB);
						itoa (width,OutLoc,10);
						goto Rtnl;
					}
				}
				else if (!stricmp (Arg[1],"HEIGHT"))//infile
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						int height = FreeImage_GetHeight (hDIB);
						FreeImage_Unload(hDIB);
						itoa (height,OutLoc,10);
						goto Rtnl;
					}
				}
				else if (!stricmp (Arg[1],"THUMBNAIL"))//infile,maxwidth,outfile,opt
				{
					HDIB32 hDIB = BMPHandleFromEXT (Arg[2]); 

					if (hDIB)
					{
						int width = atoi (Arg[3]);
						HDIB32 hDibOut = FreeImage_MakeThumbnail (hDIB,width,TRUE);

						if (hDibOut)
						{
							GMFIBMPHandleToEXT (Arg[4],hDibOut,atoi(Arg[5]));
							FreeImage_Unload(hDIB);
							FreeImage_Unload(hDibOut);
							goto RtnTrue;
						}
						FreeImage_Unload(hDIB);
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
							FreeImage_Unload(hDIB1);
							FreeImage_Unload(hDIB2);
							goto Rtnrtn;
						}
						FreeImage_Unload(hDIB1);
						FreeImage_Unload(hDIB2);
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
				
				hDIB32 = LoadDIB32(Arg[2],FALSE);
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
						rtn = DisplayBMInRect32 (CurReport->hDC,hDIB32,rect,TRUE);
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
			goto RtnFalse;
		}
		case 646: //$GMEDIT(file,TorF(create if new))
		{
			LPSTR pcmd;
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!ExistFile(Arg[1]))
			{
				if (atob(Arg[2]))
				{
					HFILE fid = GSSiOpenFile(Arg[1], 0, OF_CREATE);
					GSSiClose(fid);
				}
				else
					goto RtnFalse;
			}
			pcmd = malloc(MAX_PATH * 2);
			sprintf(pcmd, "$SESSION(CREATE,GMEdit /GMEdit %s)", Arg[1]);
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
			nArgs = GetFunArgs(Args, Arg, 10, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			rtn = SQLiteCmd(nArgs,Arg);
			itoa(rtn, OutLoc, 10);
			goto Rtnl;
		}

		case 701: /* $LOADVIS(visibility_file,Optional VPName) Load visibility file */
		{				
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

			SaveVP = CurView;      
			SetCurView ( SetVPFromName (Arg[2],&Err));  
			rtn = LoadVisList (Arg[1]);
			SetCurView ( SaveVP);
			if (rtn)
            	goto RtnTrue;
            else
				goto RtnFalse;
		}
		
		case 702: /* $LOADPIK(pickability_file) Load visibility file */
		{				
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			
			SaveVP = CurView;  
			SetConfig(1);
    
			SetCurView ( SetVPFromName (Arg[2],&Err)); 
			rtn = 1;
			if (!_fstricmp (Arg[1],"SAME"))
				SetPickSame (CurView);
			else  
				rtn = LoadPickList (Arg[1]);
			if (CurView->ID != SaveVP->ID)
				SetCurView ( SaveVP);
			if (rtn)
            	goto RtnTrue;
            else
				goto RtnFalse;
		}
		
		case 707: /* $LOADRDF (redef file,VPName(opt)) Load viewport redef file */
		{				
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			
			if (Pick || !Display)
				goto RtnTrue;
			SetCurView ( SetVPFromName (Arg[2],&Err)); 
			if (!*Arg[1])
			{
				RemoveVPRedef ();
            	goto RtnTrue;
            }
			
			if (LoadDisplayRedefFile (Arg[1]))
			{
				ConfigChangesMade = TRUE;
            	goto RtnTrue;
			}
            else
				RemoveVPRedef ();
			goto RtnTrue;
		}

		case 711: /* $LOADCFG (config file) Load config file */
		{	
			BOOL 	SaveTrackingStatus = TrackingStatus, ForceOpen;

			HaltMapDisplay (TRUE,TRUE);	
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen);   
			ForceBounds = FALSE;
			if (!SaveZooms (0))
			    DestroySavedZooms ();
			if (!*Arg[1])
			{
				BOOL	StartInNew, RetainZoom, LinkZoom;
				LPSTR	pBS,pBS1;
				char	modulePath[MAX_PATH];
				
				_fstrcpy (Arg[1],Arg[2]);
				if ((pEnd = _fstrrchr (Arg[2],'\\')))
					*pEnd = 0;
				if (!GSSiGetGMCName (hWndMain,Arg[1],Arg[2],"Load GeoMaster Configuration",&StartInNew,&RetainZoom,&LinkZoom,Arg[3],Arg[4]))
			        goto RtnFalse;
			    ForceBounds = RetainZoom;   
			    if (StartInNew)
			    {   
					GSSiGetTempFileName (0,"gmc",0,Arg[6]);  
					Fid = GSSiOpenFile (Arg[6],0,OF_CREATE);
					if (LinkZoom)
						sprintf (strchr(Arg[1],0),"(%ld)",(ULONG)hWndMain);
					BigWrite (Fid,(HPSTR)Arg[1],MAX_PATH,-1); 
					if (RetainZoom && hSavedZooms)
					{   
						long	len=GlobalSize (hSavedZooms);
					    LPSHORT pNumSavedViews = (LPSHORT)GlobalLock (hSavedZooms); 
					    
				    	BigWrite (Fid,(HPSTR)&len,4,-1);
						BigWrite (Fid,(HPSTR)pNumSavedViews,len,-1);
						GlobalUnlock (hSavedZooms);
					}
					GSSiClose (Fid);
					EscapeFunction (TRUE);
					if ((pBS1 = strrchr (Arg[6],'\\')))
						*pBS1 = 0;
					pBS = strrchr (Arg[6],'\\')+1;
					if (pBS1)
						*pBS1 = '\\';
					GetModuleFileName(NULL,modulePath,MAX_PATH);
	           		sprintf (Arg[5],"$EXECUTE(%s %s)",modulePath,pBS);
	           		//sprintf (Arg[5],"$EXECUTE([%%DL]gmloader.exe %s)",pBS);
	           		ProcessText (Arg[5]); 
	           		goto RtnTrue;
			    }
		    }
			if (!ExistFile (Arg[1]))
			{
				DestroySavedZooms ();
            	goto RtnFalse; 
			}
            ExpandText (Arg[2]);
            ForceOpen = atob (Arg[2]);
            GPSTracking (0);
    		_fstrcpy (CfgName,Arg[1]); 
    		CFGOpenTrackingStatus = SaveTrackingStatus;
			CheckForContinue(TRUE, 0);
		//	SetWindowText (hWndMain,"Switch"); 
			if (InAccel || ForceOpen)
				SendMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, -99);
			else
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, -99);
			goto RtnTrue;
		}
		
		case 703: /* $MAKEMAP(COORD,lat,long,scale,bmname)
							 (CITY,cityname,scale,bmname)
							 (BOUNDS,bounds,bmname)	*/   
			if (MakeMap (Args))
				goto RtnTrue;
			else
				goto RtnFalse;
		
		case 704: /* $WHEREAT(lat,long) */
			if (WhereAt (Args))
				goto RtnTrue;
			else
				goto RtnFalse;

		case 705: /* $POINTER(x1,y1,x2,y2)*/
		{	double	rval;
			LPSTR	lpEnd; 
			DPOINT	p1,p2;
			
			if (!CurView)
				goto RtnFalse;
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 4)
				goto RtnFalse;
			p1.x = atof (Arg[1]);
			p1.y = atof (Arg[2]);
			p2.x = atof (Arg[3]);
			p2.y = atof (Arg[4]);
			SetCurView ( SetVPFromName (Arg[6],&Err));  
			SimplePointer (CurView->hDC, &p1, &p2,5,-30,-15,atoi(Arg[5]));
			goto RtnTrue;
		} 
		  
		case 706:	/* $HLTAREA(SAVE,name) saves hlt area to file */   
					/* $HLTAREA(LOAD,name) loads hlt area from file */     
					// $HLTAREA(CLEAR)
					// $HLTAREA(SET,PICKED,n,hTran(opt))
					// $HLTAREA(SET,ITEM,TAGorRefno,use pickability(TorF))
		{	double	rval;
			int		l, len, usePick=-1; 
			LPSTR	lpOut; 
			char	fillchar;
			LPSTR	pFile;
			
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
		
			if (!_fstricmp (Arg[1],"CLEAR")) 
			{
				if (!CurrentConfig)
					ii = 1;
				ClearPolyOff(atob(Arg[2]));
				goto RtnTrue;
			}
			
			if (!_fstricmp (Arg[1],"SET"))
			{
				if (!_fstricmp (Arg[2],"PICKED"))
				{   
					SelectAreaToOffsetFile (atoi(Arg[3])-1,atof(Arg[3]),(HANDLE)atoi(Arg[4]));  
				} 
				else if (!_fstricmp (Arg[2],"ITEM"))
				{
					if ((lpColon = _fstrchr (Arg[3],':')))
						*lpColon++=0;
					else 
					{
						Refno = atol (Arg[3]); 
						Arg[3] = 0;
					}
					if (atob(Arg[4]))
						usePick = -101;
					
	            	if (!PickByRefno (Refno,Arg[3],lpColon,usePick))
						goto RtnFalse; 
					Dist = atobasedist(Arg[4],&Err);
					if (Dist) 
					{
						if (!OffsetPickedArea (0,Dist))
							goto RtnFalse;
					}
					else	
						SelectAreaToOffsetFile(0, 0, 0);
				} 
				else if (!_fstricmp (Arg[2],"HLTLIST"))
				{
					long	StartRef=LONG_MIN; 
					BOOL	SaveAutoClearOffset=AutoClearOffset;
					
		    		nlong = BT_NUM_IN_INDEX (hHighlight); 
                    AutoClearOffset = FALSE;
					while (!BT_FIND (hHighlight,(LPSTR)&StartRef,BT_FIRST,BT_GT,(LPSTR)&HighlightData))
					{
						PickList[0]=HighlightData.PD;
						SelectAreaToOffsetFile(0, atof(Arg[3]), 0);
					}
					AutoClearOffset = SaveAutoClearOffset; 
				} 
				goto RtnTrue; 
			}
			  
			if (!_fstricmp (Arg[1],"BOUNDS"))
			{ 
				int	AreaNum=1, nPoints, nPoly;
				double	Offset;
				HANDLE	hArea;
				int	Type;
				MNMXCORD	Bounds;

				DBoundsInit (&Bounds);

				if (!hAreaOffFile) 
					goto RtnFalse;
				while (hArea = GetNextHighlightArea (AreaNum++,0,&Type,&nPoints,&nPoly,0,&Offset,0))
				{
					LPMNMXCORD pRect = GlobalLock (hArea);
					InflateBounds (pRect,Offset);
					AddMinMaxD (&Bounds,pRect);
					GSSiGlobUlFree (&hArea);
				}
				boundstoa (OutLoc,&Bounds);
				goto Rtnl;
			}

			if (!_fstricmp (Arg[1],"SAVE"))
			{   
				if (!*Arg[2])
					goto RtnFalse;
				GSSiRemove (Arg[2]);
				if (!hAreaOffFile) 
					goto RtnTrue;
				pFile = GlobalLock (hAreaOffFile); 
				copyfile (Arg[2],pFile,FALSE,0,0,0,0,0,0);
				GlobalUnlock (hAreaOffFile);
				goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"LOAD"))
			{   
				if (!*Arg[2])
					goto RtnFalse;
				if (!hAreaOffFile) 
				{
					hAreaOffFile = GSSiGlobAlloc ( 882,GMEM_MOVEABLE,256);
					pFile = GlobalLock (hAreaOffFile); 
					GSSiGetTempFileName (0,"gm",0,pFile);
				}
				else 
				{
					pFile = GlobalLock (hAreaOffFile); 
					GSSiRemove (pFile);
				}
				copyfile (pFile,Arg[2],FALSE,0,0,0,0,0,0);
				GlobalUnlock (hAreaOffFile);
				HaveAreaOffFile (1);
				if (*Arg[3])
					ChangeAreaOffset (atof(Arg[3]));
				goto RtnTrue;
			}
           	goto RtnFalse;
			
		} 
        
		case 641: // $GETVAL(prompt,initval,type:def=C,cancelval)
        case 728: /* $GETIVAL (prompt,storvar(opt),AutoIncValue) Get integer value*/
        case 709: /* $GETFVAL (prompt,storvar(opt),initval) Get floating point value*/
		case 708: /* $GETCVAL (prompt,storvar(opt),ValueListPathname(opt),dropdownstyle(Y-default)or N for LB style,Sorted(y/n)) Get character value*/
		case 904: /* $GETMLCVAL (prompt,storvar(opt)) Get multiline character value*/
		{	
			HANDLE	hMem2;
			LPSTR	lpstr, lpstr2, lpSave;
			BOOL	DropDown=TRUE, Sorted=TRUE;
			
			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			hMem2 = GSSiGlobAlloc ( 883,GHND,4096);
			lpstr = GlobalLock(hMem2); 
			lpstr2 = lpstr + 2048;
			if (FunID == 641)
				strcpy (lpstr2,Arg[2]);
			else if (nArgs > 1) 
			{
				sprintf (lpstr2,"[%s]",Arg[2]);
				ExpandText (lpstr2); 
			}
			else
				lpstr2 = 0;  
			if (nArgs > 3)
				DropDown = atob (Arg[4]);
			if (nArgs > 4)
				Sorted = atob (Arg[5]);
			if (FunID == 728)
			{
				long	AutoInc = atol (Arg[3]);
				rtn = GetTextString (GetFocus(),lpstr,2048,Arg[1],0,lpstr2,AutoInc,DropDown,Sorted);
				 
			}
			else if (FunID == 904)
	   	  		rtn = GetTextStringML (GetFocus(),lpstr,2048,Arg[1],lpstr2);
			else if (FunID == 641)
			{
	   	  		rtn = GetTextString (GetFocus(),lpstr,2048,Arg[1],"",lpstr2,0,1,0);
				if (rtn)
					strcpy (OutLoc,lpstr);
				else if (*Arg[4])
					strcpy (OutLoc,Arg[4]);
				else
					strcpy (OutLoc,Arg[2]);
				GSSiGlobUlFree (&hMem2); 
				goto Rtnl;
			}
			else
	   	  		rtn = GetTextString (GetFocus(),lpstr,2048,Arg[1],Arg[3],lpstr2,0,DropDown,Sorted);
	   	  	if (rtn)
	   	  	{
				l = _fstrlen(lpstr);
				_fstrncpy (OutLoc,lpstr,l+1);  
				if (Arg[2])
				{
				/*	if (lpstr2)
						SetGlobalValue(Arg[2],lpstr2);
					else*/
						SetGlobalValue(Arg[2],lpstr);
				}

			}
			else
			{   
				if (lpstr2)
					_fstrcpy (OutLoc,lpstr2);
				else
					*OutLoc = 0;
				ContinueProcessing = FALSE;  
           		PostMessage(hWndMain, GF_CLEAR_FUN_STACK, 0, 0L);
			}
			GSSiGlobUlFree (&hMem2); 
			goto Rtnl;
		} 
		
		case 710: /* $LINESYM(cursymorpar (default all)) set line symbol variables */
		{   
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (SelectSymbol(CurView->hWnd, 2, Arg[1], 1))
				goto RtnTrue;
			else
				goto RtnFalse;
		}
		
		case 505: // $TRNTO(Transformation file,Point)  transform point from 'from' coords to 'to' coords
		case 720: // $TRNFROM(Transformation file,Point)  transform point from 'to' coords to 'from' coords
			CvtDir = 1;
			if (FunID == 720)
				CvtDir = 2;
		{	 
			double	X,Y, OutX, OutY;
			short	n, Type; 
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			Type = atoi (Arg[3]);
			if (Type < 1 || Type > 3)
				Type = 1;
			
		    nPoints = GetPointsFromList (Arg[2],&hPoints);  
		    *OutLoc = 0;
			if (!(hTran=LoadTranFile(Arg[1],CvtDir,Type,0,0)))
			{
				GSSiGlobFree (&hPoints);
				_fstrcpy (OutLoc,"Invalid Transformation File");
				goto Rtnl;
			}
		    if (!hPoints)
		    	goto Rtnl;
			pointstoa (OutLoc,nPoints,hPoints,hTran);
			GSSiGlobFree (&hPoints);
			CloseTRANS2 (&hTran);
			goto Rtnl;
		}
		case 506: // $CVTTO(convert file,Point) convert point between projections 
		case 721: // $CVTFROM(convert file,Point) convert point between projections  
			CvtDir=1;
			if (FunID==506)
				CvtDir=2;
		{	 
			short	n; 
			HANDLE	hPoints;
			HPDPOINT	pPoint;  
			short	nPoints;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
		    nPoints = GetPointsFromList (Arg[2],&hPoints);  
		    *OutLoc = 0;
		    if (!hPoints)
		    	goto Rtnl;
		    pPoint = (HPDPOINT)GlobalLock (hPoints);
			for (n=0;n<nPoints;n++)
			{   
				if (n)
					_fstrcat (OutLoc," ");
				if (ConvertPoint (Arg[1],&pPoint[n],CvtDir))
				{
					_fstrcpy (OutLoc,"Invalid Projection File");      
					GSSiGlobUlFree (&hPoints);
					goto Rtnl;
				}
				dpointtoa (_fstrrchr(OutLoc,0),&pPoint[n]); 
			}
			GSSiGlobUlFree (&hPoints);
			goto Rtnl;
		}
		
		case 722: /* $AREASYM() set area symbol variables */
		{   
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (SelectSymbol(CurView->hWnd, 3, Arg[1], 1))
				goto RtnTrue;
			else
				goto RtnFalse;
		}
		
		case 723: // $GETPATH(R W D or C (creates with no overwrite prompt),Extension,title(opt),storvar(opt),startdir) Get pathname
			      // $GETPATH(A,path) gets full actual path - if cached displays cache name
		{	
			HANDLE	hTemp;
			LPSTR	str, lpSave;    
			BOOL	rtn,ResetSubDL;
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			_fstrupr (Arg[1]);  
			if (*Arg[1] ==  'A')
			{
				OFSTRUCTGM	OFStruct;
				HFILE		Fid = GSSiOpenFile (Arg[2],&OFStruct,OF_READ);

				*OutLoc = 0;
				if (Fid != HFILE_ERROR)
					strcpy (OutLoc,OFStruct.szPathName);
				GSSiClose (Fid);
				goto Rtnl;
			}
			hTemp = GSSiGlobAlloc ( 888,GHND,256);
			str = GlobalLock (hTemp);
			_fstrcpy (str,Arg[5]);
			if (*str && !*LastChr (str) != '\\')
				strcat (str,"\\");

			ResetSubDL = GetGlobalBVal ("[%SUBDL]");
			SetGlobalValue("%SUBDL","N");
			switch (*Arg[1])
			{
				case 'R':
                	rtn = GetFileName4 (GetFocus(),str,0,Arg[2],Arg[3],Arg[4]);
                	goto TestGPRtn;
				case 'W': 
					OverWritePrompt = FALSE;
				case 'C':
                	rtn = GetSaveName3 (GetFocus(),str,0,Arg[2],Arg[3],Arg[4]);
                	break; 
				case 'D':
				{
					LPSTR VarName = Arg[4];
					if (VarName)
					{
						if (*VarName)
						{
							sprintf(str, "[%s]", VarName);
							ExpandText(str);
							strcpy(Arg[6], str);
						}
						else
							VarName = 0;
					}

					rtn = GetFolderName(GetFocus(), Arg[6], str, Arg[3]);

					//rtn = GetSaveName3 (GetFocus(),str,0,Arg[2],Arg[3],Arg[4]);
					if (rtn)
					{
						if (FileType(str) == 1)
							GSSiRemove(str);
						else if (VarName)
							SetGlobalValue(VarName, str);
					}
				}
                	break; 
                default:
                	rtn=FALSE;
            }
TestGPRtn:	if (ResetSubDL)
				SetGlobalValue("%SUBDL","Y");
			if (rtn)
				_fstrcpy (OutLoc,str); 
			else
			{
				ContinueProcessing = FALSE;  
           		PostMessage(hWndMain, GF_CLEAR_FUN_STACK, 0, 0L);
           		*OutLoc = 0;
			}
			GSSiGlobUlFree (&hTemp);
			goto Rtnl;
		} 
		
		case 724: // $NULLMAP(pathname,CoordOpt,R to replace existing file)
		{	
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			switch (*Arg[2])
			{         
				default: 
					Bounds = atobounds (Arg[2],&Err);
					if (Err)
						goto RtnFalse;
					break; 
				case 'v':
				case 'V':
					{
						LPSTR pEndPar,pPar = strchr (Arg[2],'(');
						
						if (pPar)
						{
							pPar++;
							pEndPar = strrchr (pPar,')');
							if (pEndPar)
								*pEndPar = 0;
							SetCurView (SetVPFromName (pPar,&Err));
						}
						Bounds = CurView->WBounds;
						CurView = SaveVP;
					}
					break;
				case 'R':
				case 'r':
					Bounds = ZoomBoxRect;
					break;
			}
			ExpandText (Arg[3]);
			if (ExistFile (Arg[1]) && _fstricmp ("R",Arg[3]))
				goto RtnFalse;
	        if (CreateNewMap (Arg[1],&Bounds,0,0,0,0,0,0,FALSE))
				goto RtnTrue;
			else
            	goto RtnFalse;
		} 
		
		case 725: // $MAKEDIR(pathname) make directory
		{   
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (!makedirectories(Arg[1], TRUE, FALSE))
				goto RtnFalse;
			goto RtnTrue;
		}
		
		case 726: // $ENLARGE(width,factor) enlarge portion of screen
		{   
			short	factor,width;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			width = atoi(Arg[1]);
			factor = atoi (Arg[2]); 
			EnlargeScreen (factor,width);
			goto RtnTrue;
		}
		
		case 727:	// $INFOBOX(LOAD,pathname)load infobox  
					// $INFOBOX(CREATE,pathname,Point,optional infobox center point,optional move factor)load infobox or
					// $INFOBOX(CREATE,pathname,ITEM,pickeditemnum,optional infobox center point)
					// $INFOBOX(CREATE,pathname,ZOOM,optional infobox center point)
		{   
			short	factor,width,item=-1;
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			if (!_fstricmp (Arg[1],"LOAD")) 
			{
				if (LoadInfoBox (Arg[2],&TAGBox))
					goto RtnTrue;
			}  
			if (!_fstricmp (Arg[1],"DELETE")) 
			{
				if (DeleteTAGByName (Arg[2]))
					goto RtnTrue;
			}  
			if (!_fstricmp (Arg[1],"CREATE")) 
			{   
				if (!_fstricmp (Arg[3],"ITEM"))    
				{
					item = atoi (Arg[4])-1;
					Point = PickList[item].PickedPoint;
					SetConfig (PickList[item].ConfigID);
					SetViewport (PickList[item].ViewID);
				}
				else if (!_fstricmp (Arg[3],"POINT")) 
				{
					SetCurView ( SetVPFromName (Arg[5],&Err));  
					Point = atopt (Arg[4],&Err); 
				}
				else if (!_fstricmp (Arg[3],"ZOOM"))    
					item = -2;
				else if (nArgs > 2)
					Point = atopt (Arg[3],&Err); 
				else
					Point = MinMaxMidPointD (&CurView->WBounds);
				if (CreateTAGBoxFromFile (CurView->hDC,Point,Arg[2],item,atof (Arg[6])))
					goto RtnTrue;
			}  
			goto RtnFalse;
		}
		case 729: // $OPENAPP(pathname) executes app associated with file
		{   
		   	HINSTANCE	hI;
		   	
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (!ExistFile(Arg[1]))
			{
				GSSiMessageBox (Arg[1],"Unable to Access Application File",MB_ICONEXCLAMATION,0);
				goto RtnFalse;
			} 
			else
			{
				hI=ShellExecute (hWndMain,0,Arg[1],0,0,SW_SHOWMAXIMIZED);
				if (DisplayShellExError ((UINT)hI,Arg[1]))
					goto RtnFalse;
				else
					goto RtnTrue; 
			}
		}
		case 730: // $SYMNAME (symnum,from(opt)) from=4 forces to get from file not dict
		{	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			SymNum = atoi(Arg[1]);
			if (*Arg[2]) 
				GetSymbolName (SymNum,OutLoc,0,atoi(Arg[2]),0);
			else
				GetDictSymName (SymNum,OutLoc);
			goto Rtnl;   
		}
		
		case 731: // $SYMDESC (symnum)
		{	
			short SymNum;			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			SymNum = atoi(Arg[1]);
			GetDictSymDescription (SymNum,OutLoc);
			goto Rtnl;   
		}
		
		case 732: // $AUTOINC(prefix,udi) returns autoinc udi or same udi if no autoinc
		{   
			short	factor,width;
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			_fstrcpy(OutLoc, Arg[2]);
		    LoadTAGDef ();   
			if (NumTAGDef)
			{ 
				LPTAGDEF    lpTAGDef;
				short	i, len; 
				LPSTR	pBeg, pEnd;
				double	CurVal; 
				char	SaveEnd;
					            
				lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
				for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
				{
				    if (!_fstricmp (Arg[1],lpTAGDef->Prefix))
				    {
				    	if (lpTAGDef->IncLen)
				    	{
				    		l=_fstrlen (Arg[2]); 
				    		if (lpTAGDef->IncBeg)
				    		{
				    			pBeg = Arg[2] + (lpTAGDef->IncBeg-1);
				    			pEnd = pBeg + lpTAGDef->IncLen; 
				    			len = lpTAGDef->IncLen;
				    		}
				    		else
				    		{   
				    			len = min (l,lpTAGDef->IncLen);
				    			pEnd = _fstrchr (Arg[2],0);
				    			pBeg = Arg[2] + (l - len);
				    		}         
				    		SaveEnd = *pEnd;
				    		*pEnd = 0;      
				    		CurVal = atof (pBeg);
				    		if (lpTAGDef->IncNDP)
				    		;
				    		else
				    			IWRITEZ (IDNINT(CurVal+1),pBeg,len);
				    		*pEnd = SaveEnd; 
				    		_fstrcpy (OutLoc,Arg[2]);
				    	}
				    	break;
				    }
				}
				GlobalUnlock (hTAGDef);
			}
			goto Rtnl;   
		}                                                           

		case 733: // $GLOBALS (?)
		{	
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			DisplayFieldList(GetFocus(), 0, 0, 3, 0);
			goto Rtnl;   
		}

		case 734:// $FILEFIT (filelist,exclusionlist,fittype,projection,OutMap,symbol)
		{
			short	FitType;
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 5)
				goto RtnFalse;
			FitType = atoi (Arg[3]);
		 	rtn = LinfitFilePoints (Arg[1],Arg[2],FitType,Arg[4],Arg[5],Arg[6]);
		 	if (rtn)
		 		goto RtnTrue;
		 	goto RtnFalse;
		}

		case 735:// $EPMACRO (prepickmacro,bpmacro,epmacro,postpickmacro)
		{
			short	FitType;
			
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			rtn = EndPointMacros(Arg[1], Arg[2], Arg[3], Arg[4]);
		 	if (rtn)
		 		goto RtnTrue;
		 	goto RtnFalse;
		}

		case 736: // $MESSAGE(message,button opt,pos) Title is blank unless arg1 contains | to sep message from title (i.e $MESSAGE(message|title,opt)
		{
			char space[2]=" ";
			LPSTR VB;
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			rtn = IDOK;
			if ((VB = strrchr (Arg[1],'|')))
				*VB++ = 0;
			else
				VB = space;
			if (InServerMode)
			{
				LogServerActivity (Arg[1]);
			}
			else
			switch (atoi(Arg[2])) 
			{
				case 1:
					rtn = GSSiMessageBox(Arg[1],VB,MB_YESNO,Arg[3]); 
					break;
				case 2:
					rtn = GSSiMessageBox(Arg[1],VB,MB_ABORTRETRYIGNORE,Arg[3]); 
					break;
				default:
					rtn = GSSiMessageBox(Arg[1],VB,MB_OK,Arg[3]); 
			}
			SetCurView ( SaveVP);
			switch (rtn)
			{
				case IDOK:
					goto RtnTrue;
				case IDYES:
					goto RtnTrue;
				case IDRETRY:
					CloseBufferedMacros ();
					CloseAllRequestedFiles(FALSE); 
   					CacheAlreadyChecked (0,0,0);
					goto RtnTrue;
				case IDNO:
					goto RtnFalse;
				case IDABORT:
					ContinueProcessing = FALSE;
			}
			goto RtnTrue;  
		}

		case 737: /* $SAVEPIK(pickability_file,description,optional VP name) Save pickability file */  
			PickVis=TRUE;
			goto SaveVis;
		case 738: /* $SAVEVIS(pickability_file,description,optional VP name) Save pickability file */  
			PickVis=FALSE;
SaveVis:
		{				
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			SaveVP = CurView;      
			SetCurView ( SetVPFromName (Arg[3],&Err));  
			rtn = SaveVisFile (Arg[1],PickVis,Arg[2]);
			SetCurView ( SaveVP);
			if (rtn)
            	goto RtnTrue;
            else
				goto RtnFalse;
		}
		
		case 739: // $ATPRINT(commands) infobox actions at print time
		{				
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (Printing && InDrawTAG)
			{
				InAtPrint = TRUE;
				ExpandText (Arg[1]);
				InAtPrint = FALSE; 
			}
			goto Rtnl;  
		}

		case 740: //$TCPOPEN(ipaddress,port,VARNAMEforsocket,terminator,inputprocesscommand,closeprocesscommand,restartcommand)
		{	
			USHORT	port;  
			SOCKET	sock;
						
			nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 6)
				goto RtnFalse;
			port = atol (Arg[2]);
			SetGlobalValueLong (Arg[3],0);    
			if (OpenTCPIPSocket (hWndMain,Arg[1],port,&sock,Arg[4],Arg[5],Arg[6],Arg[7],TRUE)<0)
				goto RtnFalse;
			SetGlobalValueLong (Arg[3],sock);    
			goto RtnTrue;
		} 
		  
		case 741: //$TCPSEND(socket,string,sendnullterm,waitseconds)
		{	
			SOCKET	socket;
						
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			socket = atol (Arg[1]);
			rtn = SendTCPIPString (socket,Arg[2],atob(Arg[3]),(UINT)atol(Arg[4]));  
			ProcessTCPData (socket);
			ltoa ((long)rtn,OutLoc,10);
			goto Rtnl;
		}
		
		case 742: // $GMDFIND(file,setkey,setval(opt),partialmatchnum(opt),index(opt)) 
				  // ex: $GMDFIND(file.gmd,KEY1=A;KEY2=B)  
				  //	 $GMDFIND(fileid,KEY1=A;KEY2=B,[val]=[fileval];[val2]=[fileval2],3)
		{	 
			short	index;

			nArgs = GetFunArgs (Args,Arg,-5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse; 
			ExpandText (Arg[1]);
			ExpandText (Arg[4]);
			n = atoi (Arg[4]);
			index = atoi (Arg[5]);
			if (FindGMDRecord (Arg[1],Arg[2],Arg[3],n,index))
				goto RtnTrue; 
			else
            	goto RtnFalse;
			
		}  
		break;  
		
		case 743: // $UMREFNO(intref) 
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			Refno = atol(Arg[1]);
			sprintf (OutLoc,"%.2f",(((double)Refno) - ZERO$)/100); 
			goto Rtnl;
		}  
		break;    
		
		case 744: // $CONVERT(AREA,from units(system),to units(sqrfeet),value) convert units
		{   
			double	InVal, OutVal;
			
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 4)
				goto RtnFalse;
			OutVal = -999999999;
			if (!_fstricmp (Arg[2],"SYSTEM"))
			{
			    if (!_fstricmp (Arg[1],"LENGTH"))
			    {   
					InVal = atof (Arg[4]);
			    	for (n=0;n<5;n++)
			    		if (!_fstricmp (DistUnitOpts[n],Arg[3]))
			    			OutVal = ConvertDist (InVal,n+1);
			    }
			    else if (!_fstricmp (Arg[1],"AREA"))
			    {   
					InVal = atof (Arg[4]);
			    	for (n=0;n<6;n++)
			    		if (!_fstricmp (AreaUnitOpts[n],Arg[3]))
			    			OutVal = ConvertArea (InVal,n+1);
			    } 
				sprintf (OutLoc,"%f",OutVal); 
			}
		    else
		    {   
				GetDistAndUnits (Arg[2],&OutVal,&n1,TRUE);  
				GetDistAndUnits (Arg[3],&OutVal,&n2,TRUE);
				n1++;
				n2++;  
				if (!_fstricmp (Arg[1],"POINT"))
				{
					Point = atopt (Arg[4],&Err); 
					Point.x = ConvertDist2 (Point.x,n1,n2); 
					Point.y = ConvertDist2 (Point.y,n1,n2); 
					sprintf (OutLoc,"%f %f",Point.x,Point.y);
				}
				else
				{
					InVal = atof (Arg[4]);
					OutVal = ConvertDist2 (InVal,n1,n2); 
					sprintf (OutLoc,"%f",OutVal); 
				}
 
			}
			goto Rtnl;
        }
		
		case 745: // $ADJTIME(seconds,ADJUSTKEY) adjust time to first sec of previous month, day or year
				  // ex: $ADJTIME([%SYS_CLOCK],MONDAY) adjusts to first second of prev (or current) Monday
		{	 
			
			struct	tm	thistime;   
			short	i;
            
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			systime = max(0, atol(Arg[1]));
			thistime = *localtime (&systime); 
			thistime.tm_hour = 0;
			thistime.tm_min = 0;
			thistime.tm_sec = 0;
			thistime.tm_isdst = -1;
			systime = mktime (&thistime);
			if (!_fstricmp (Arg[2],"MONTHBEGIN"))
			{ 
				thistime.tm_mday = 1;
				systime = mktime (&thistime);
			}
			if (!_fstricmp (Arg[2],"MONTHEND"))
			{
				thistime.tm_mday = 15;
				systime = mktime (&thistime);
				systime += 3600L * 24L * 30L;
				thistime = *localtime (&systime); 
				thistime.tm_hour = 0;
				thistime.tm_min = 0;
				thistime.tm_sec = 0;
				thistime.tm_mday = 1;
				thistime.tm_isdst = -1;
				systime = mktime (&thistime); 
				systime--;
			}
			else
//check for day of week
			for (i=0;i<7;i++)
			{
				if (!_fstricmp (Arg[2],DOW[i]))
				{   
					if (thistime.tm_wday >= i)
						systime -= (long)(thistime.tm_wday - i) * 86400L;
					else
						systime -= (long)(7-i + thistime.tm_wday) *86400L; 
					break;
				}
			}
			ltoa ((long)systime,OutLoc,10);
			goto Rtnl;
		}  
		break; 
		
		case 746: // $COPYMAP(toname,fromname,conversion)
		{   
			LPSTR	pEnd2, pFile2;
			
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			pFile = Arg[4] + 2048;
			pFile2 = pFile + 2048;
			
			if (!makedirectories (Arg[1],FALSE,FALSE))  
				goto RtnFalse;
			if (!copyfile (Arg[1],Arg[2],FALSE,0,0,0,0,0,0))
				goto RtnFalse;
			_fstrcpy (pFile,Arg[2]);
			_fstrcpy (pFile2,Arg[1]);
			{
				_fstrupr (pFile);
				_fstrupr (pFile2);
				if ((pEnd = _fstrstr (pFile,".PLT")) && (pEnd2 = _fstrstr (pFile2,".PLT")))
				{
					_fstrcpy (pEnd,".RIN");
					_fstrcpy (pEnd2,".RIN");
					if (ExistFile (pFile))
						copyfile (pFile2,pFile,FALSE,0,0,0,0,0,0);
					else
						GSSiRemove (pFile2);  
					_fstrcpy (pEnd,".TIN");
					_fstrcpy (pEnd2,".TIN");
					if (ExistFile (pFile))
						copyfile (pFile2,pFile,FALSE,0,0,0,0,0,0);
					else
						GSSiRemove (pFile2);  
				}
			}
			_fstrcpy (MapCopyProjection,Arg[3]);
			ConvertFileCoordinates (Arg[1],0,0,0);    
			*MapCopyProjection = 0;
			goto RtnTrue;
		}
		break; 
		
		case 747: // $SYMCOPY(symdic name,symname,parent(opt),newname(opt),fromsymnum(opt),tosymnum(opt)) copies symbol from another symbol dict
		{   
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			rtn = CopySymbolFromDict (Arg[1],Arg[2],Arg[3],Arg[4],atoi(Arg[5]),atoi(Arg[6]));
			if (rtn)
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 748: // $EXECUTE(command,MIN or MAX(default))
		{   
			UINT hI; 
			UINT	ShowVal=SW_SHOW;
			
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstrnicmp(Arg[2], "MIN", 3))
				ShowVal = SW_SHOWMINIMIZED;	
			else if (!_fstrnicmp (Arg[2],"MAX",3))
				ShowVal = SW_SHOWMAXIMIZED;	
			else if (!_fstrnicmp (Arg[2],"FIND",3))
			{
				UINT	hI=(UINT)FindExecutable (Arg[1],CurDir,OutLoc);

				DisplayShellExError (hI,Arg[1]);
				goto Rtnl;
			}
			//MessageBox (0,Arg[1],0,MB_OK);	
			_getcwd (CurDir,MAX_PATH);  
			hI = WinExec (Arg[1],ShowVal);
			if (hI <32)
			{
				DisplayShellExError (hI,Arg[1]);
				goto RtnFalse;  
			}
			goto RtnTrue;
		}
		
		case 749: //$CDUNITS(2,DMS,1)
		{				
			
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			*OutLoc = 0;
			for (n=0;n<*pNumViewports;n++)
			{
				if (pViewports[n]->pTheme)
				{ 
					if (pViewports[n]->pTheme->ID == PF_COORD_DISPLAY)
					{   
						if (!_fstricmp (Arg[2],"DM"))
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1]=3;
						else if (!_fstricmp (Arg[2],"DMS")) 
						{
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1]=4;
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Precision[1]=atoi(Arg[3])+1;
						} 
						else if (!_fstricmp (Arg[2],"DD")) 
						{
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1]=5;
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Precision[1]=atoi(Arg[3])+1;
						} 
						else if (!_fstricmp (Arg[2],"MGRS")) 
						{
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Units[1]=8;
							((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->Precision[1]=atoi(Arg[3])+1;
						}
						if (*Arg[4])
							_fstrcpy (((LPCOORDINATEDISPLAY)pViewports[n]->pTheme)->LineID[1],Arg[4]);
						break;
					} 
				}
			}
			goto Rtnl;
		} 
		
		case 750: // $LOADTIN(infile,outfile)
		{   
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);

			if (LoadTIN (Arg[1],Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
		}

		case 751: // $SYMTYPE(symname) 
		{	 
			short	itype;
			char	CTypes[4][2]={"p","P","L","A"};
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (*Arg[1] == '#')
				n = atol (Arg[1]+1);
			else
				n = GetDictSymbolNumber (Arg[1]);
			itype = GetDictSymbolType (n);
            if (itype > 3 || itype < 0)
            	*OutLoc = 0;
            else
            	_fstrcpy (OutLoc,CTypes[itype]);
			goto Rtnl;
		}  
		break;    
		
		case 752: // $REPLACE(str,oldstr,newstr) 
		{	 
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen);  
			_fstrcpy (OutLoc,Arg[1]);
			REPLAC (OutLoc,Arg[2],Arg[3],4096); 
			goto Rtnl;
		}  
		
		case 753: // $SYMPLOT(Parent,outfile,format) 
		{	 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (CreateSymbolPlot(Arg[1], Arg[2], Arg[3]))
		    	goto RtnTrue;
		    goto RtnFalse;
		}
		
		case 754: // $LOADDTM(GRID,infile,outfile)
		{   
			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse; 
			if (!_fstricmp (Arg[1],"GRID"))
			{
				if (LoadGRIDDTM (Arg[2],Arg[3]))
					goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"AREA"))
			{
				if (LoadAREADTM (Arg[2],Arg[3]))
					goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"LIDAR"))
			{
				if (LoadLIDARDTM (Arg[2],Arg[3],Arg[4]))
					goto RtnTrue;
			} 
			else if (!_fstricmp (Arg[1],"ERDAS"))
			{
				if (LoadERDASDem ())
					goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 755: // $SYMDICT(COMPRESS) 
		{	 
			HANDLE hSymbol;
			int isym;
		
			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen); 
			rtn = FALSE;
			if (nArgs < 1)
				goto RtnFalse; 
			if (!stricmp (Arg[1],"COMPRESS"))
				CompressSymDict ();
			else if (!stricmp (Arg[1],"DELETE"))
			{
				if (!stricmp (Arg[2],"SYMBOL"))//$SYMDICT(DELETE,SYMBOL,symnum,close(boolean))
				{
					n = atoi (Arg[3]);
					if (DeleteSymbol (n,atob(Arg[2])))
						goto RtnTrue;
					goto RtnFalse;
				}
			}  
			else if (!stricmp (Arg[1],"COPY"))
			{
				if (!stricmp (Arg[2],"SYMBOL"))
					rtn = CopySymbolFromDict (Arg[3],Arg[4],Arg[5],Arg[6],atoi(Arg[7]),atoi(Arg[8]));
				if (!stricmp (Arg[2],"PARENT"))
					rtn = CopyParentFromDict (Arg[3],Arg[4],Arg[5],Arg[6]);
			}
			else if (!stricmp (Arg[1],"GAPS"))
			{
				HFILE fidOut = GSSiOpenFile (Arg[2],0,OF_CREATE);
				LPSTR str=Arg[8];
				int fromSym = atoi (Arg[3]);
				int toSym = atoi (Arg[4]);

				n = 0;
				if (fidOut != HFILE_ERROR)
				{
					sprintf (str,"GAPSYMNUM");
					fputstring (str,fidOut);
					for (isym = fromSym;isym < toSym;isym++)
					{
						if ((hSymbol = GetDictSymDesc (isym,0)))
						{
							DestroySymbol (hSymbol);
						}
						else
						{
							itoa (isym,str,10);
							fputstring (str,fidOut);
						}
					}
					GSSiClose (fidOut);
				}
				itoa (n,OutLoc,10);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"LIST"))//$SYMDICT(LIST,outfile,PARENT,parentname)
											  //$SYMDICT(LIST,outfile,GAPS,fromsymnum,tosymnum
			{
				HFILE fidOut = GSSiOpenFile (Arg[2],0,OF_CREATE);
				LPSTR str=Arg[8];
				
				n = 0;
				if (fidOut != HFILE_ERROR)
				{
					sprintf (str,"SYMNUM\tSYMNAME\tSYMTYPE");
					fputstring (str,fidOut);
					if (!stricmp (Arg[3],"PARENT"))
					{
						int iparsym = GetSymbolNum (Arg[4]);
						if (iparsym > 0)
						{
							for (isym=1;isym<=NumSymbols;isym++)
							{
								if ((hSymbol = GetDictSymDesc (isym,0)))
								{
									LPSYMBOL pSymbol = (LPSYMBOL)GlobalLock (hSymbol);
									if (pSymbol->Parent == iparsym)
									{
										sprintf (str,"%i\t%s\t%i",pSymbol->Number,pSymbol->Name,pSymbol->Type);
										fputstring (str,fidOut);
									}
									GlobalUnlock (hSymbol);
									DestroySymbol (hSymbol);
								}
							} 

						}
					}
					GSSiClose (fidOut);
				}
				itoa (n,OutLoc,10);
				goto Rtnl;
			}
			goto Rtnrtn;
		}  
		break;  

		case 756: /* $ARCDIST(point1,point2) */ 
		{	double dist;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Point = atopt(Arg[1], &Err);
			Point2 = atopt (Arg[2],&Err);
			dist = ArcDistance (Point,Point2);
			sprintf (OutLoc,"%f",dist);
			goto Rtnl;
		}  
		
		case 757: //$LOADBMP(BMPFile,Infile)
		{	
			long	ICmd;
			
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen);  
			if (BMPFileFromEXT (Arg[2],Arg[1],1)) 
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 758: //$ADDAREA(file.plt,TAG,SYMBOLNAME,points)  
		{
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen);  
			if (AddAreaToMap (Arg[1],Arg[2],Arg[3],Arg[4],0,0,0,0,0,0,0,0,0,0))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 759: //$DOWNAME(downum)  
		{
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen); 
			i = atoi (Arg[1]);
			if (i > 0 && i < 8)
				_fstrcpy (OutLoc,DOW[i-1]);
			else
				*OutLoc = 0;
			goto Rtnl;
		}
		
		case 760: //$ADDLINE(file.plt,TAG,SYMBOLNAME,points)  
		{
			nArgs = GetFunArgs (Args,Arg,-4,&hMem, pBrkPt, bpOffset, bpLen);  
			hMem2 = GSSiGlobAlloc ( 913,GMEM_MOVEABLE,USHRT_MAX);
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			ExpandText (Arg[3]);
			Arg1 = GlobalLock(hMem2);
			_fstrcpy (Arg1,Arg[4]); 
			ExpandText (Arg1);
			rtn = AddAreaToMap (Arg[1],Arg[2],Arg[3],Arg1,1,0,0,0,0,0,0,0,0,0);
			GSSiGlobUlFree (&hMem2);
			if (rtn)
				goto RtnTrue;
			goto RtnFalse;
		} 
		
		case 761: //$NUMROWS(fileid)  
		{
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
	        
	        nlong = 0;
	        if (!_fstricmp (Arg[1],"HLTLIST"))
		    	nlong = BT_NUM_IN_INDEX (hHighlight); 
            else if (FilePathHandle) 
            {
				FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);  
				lpFileHandle = &FilePathPtr->FileHandle;
	            
				for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
				{
					if (*lpFileHandle)
					{
						SQLPtr = (LPOPENSQLDATA)GlobalLock (*lpFileHandle);
						if (!_fstricmp (SQLPtr->IDName,Arg[1]))
							nlong = NumSQLRows (*lpFileHandle); 
						GlobalUnlock (*lpFileHandle);
					} 
				}
				GlobalUnlock (FilePathHandle);
            }
			ltoa (nlong,OutLoc,10);
			goto Rtnl;
		}
		
		case 762: //$PICKCPT(point,filelist,ignorename,maxdist)  
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 4)
				goto RtnFalse;
			Point = atopt (Arg[1],&Err);
			RVal = atof (Arg[4]);
			PickNearestCPTFile (Point,Arg[2],Arg[3],RVal,OutLoc);
			goto Rtnl; 
				
		case 763: //$DISPLAY(ITEM,refno or TAG,Viewport Name)  
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if ((lpColon = _fstrchr (Arg[2],':')))
				*lpColon++=0;
			else 
			{
				Refno = atol (Arg[2]); 
				Arg[2] = 0;
			}  
			SetCurView ( SetVPFromName (Arg[3],&Err));  
            if (PickByRefno (Refno,Arg[2],lpColon,-100))
            {   
		    	ProcessPickedItem (NumPicked-1,TRUE); 
				goto RtnTrue;
			}
			goto RtnFalse; 
				
		case 764: //$GMDCOPY(gmdfile,fromkey,tokey)  
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				goto RtnFalse;
			if (GMDCopyRecord (Arg[1],Arg[2],Arg[3]))
				goto RtnTrue; 
			else
            	goto RtnFalse;
				
		case 765: //$VEHICLE(DEFINE,id,description,symname,color,size)  
			nArgs = GetFunArgs (Args,Arg,9,&hMem, pBrkPt, bpOffset, bpLen); 
			
			if (!_fstricmp (Arg[1],"DISPLAYIN"))
			{
				int	iview;

				SetCurView ( SetVPFromName (Arg[2],&Err));  
				for (iview=0;iview<nAVLVP;iview++)
				{
					if (CurView == AVLViewports[iview])
						goto HaveVP;
				}
				AVLViewports[nAVLVP++] = CurView;
				DisplayAllVehicles (FALSE,TRUE); 
HaveVP:;
				goto RtnTrue;
			}

			if (!_fstricmp (Arg[1],"BOUNDS"))
			{
				Bounds = GetVehicleBounds (Arg[2]);
				boundstoa (OutLoc,&Bounds); 
				goto Rtnl;
			}

			if (!_fstricmp (Arg[1],"UPDATE"))
			{	 
				if (!_fstrnicmp (Arg[3],"Invalid",7))
					goto RtnFalse;
				Point = atopt (Arg[3],&Err);  
				rtn = SetVehicleLoc (Arg[2],Point,(short)atof(Arg[4]),(short)atof(Arg[5]),atoi(Arg[6]),atob(Arg[7]),TRUE,atoi(Arg[8]),atob(Arg[9]));
				goto Rtnrtn;
			}

			if (!_fstricmp (Arg[1],"SETSTATUS"))
			{
				rtn = SetVehicleStatus (Arg[2],atoi(Arg[3]),FALSE,atob(Arg[4]));
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"SETFENCESTATUS"))
			{
				rtn = SetVehicleStatus (Arg[2],atoi(Arg[3]),TRUE,atob(Arg[4]));
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"UPDATEINFO"))
			{
				rtn = DisplayVehicleInfo (Arg[2],FALSE);
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"SETCOLOR"))
			{
				rtn = SetVehicleColor (Arg[2],atoi(Arg[3]));
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"SETSPEED"))
			{
				rtn = SetVehicleStatus (Arg[2],atoi(Arg[3]),FALSE,atob(Arg[4]));
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"FLASH"))
			{
				rtn = FlashVehicle (Arg[2]);
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"DISPLAY"))
			{
				UpdateAllVehicles(TRUE);
				goto RtnTrue;
			}
			if (!_fstricmp (Arg[1],"TRACK"))
			{
				rtn = SetVehicleTrackColor (Arg[2],atoi(Arg[3]));
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"REMOVE"))
			{
				rtn = RemoveVehicle (Arg[2]);
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"DEFINE"))
			{
				if (nArgs < 6)
					goto RtnFalse;
				rtn = DefineVehicle (Arg[2],Arg[8],Arg[3],Arg[4],Arg[5],Arg[6],Arg[7]);
				goto Rtnrtn;
			}
			if (!_fstricmp (Arg[1],"SPEED"))
			{
				if (!stricmp (Arg[3],"GET"))
				{
					RVal = GetVehicleSpeed (Arg[2]);
					ftoa (OutLoc,RVal);
					goto Rtnl;
				}
				goto RtnFalse;
			}
			if (!_fstricmp (Arg[1],"DRIVER"))
			{
				if (!stricmp (Arg[3],"SET"))
				{
					rtn = SetVehicleDriver (Arg[2],Arg[4]);
					goto Rtnrtn;
				}
				if (!stricmp (Arg[3],"GET"))
				{
					GetVehicleDriver (Arg[2],OutLoc);
					goto Rtnl;
				}
				goto RtnFalse;
			}
			if (!_fstricmp (Arg[1],"HEADING"))
			{
				if (!stricmp (Arg[3],"GET"))
				{
					RVal = GetVehicleHeading (Arg[2]);
					ftoa (OutLoc,RVal);
					goto Rtnl;
				}
				goto RtnFalse;
			}
			if (!_fstricmp (Arg[1],"HISTORY"))
			{
				if (!stricmp (Arg[2],"CLEAR"))
				{
					ClearVehicleHistory (atoi(Arg[3]));
					goto RtnTrue;
				}
				else if (!stricmp (Arg[2],"SETMAX"))
				{
					int maxtime = atoi (Arg[3]);

					if (!maxtime)
						maxtime = -USHRT_MAX;
					else
						maxtime = -maxtime;
					ClearVehicleHistory (maxtime);
					goto RtnTrue;
				}
				else if (!stricmp (Arg[2],"SETSEL"))
				{
					rtn = SetVehicleHistorySelection (atoi(Arg[3]));
					goto Rtnrtn;
				}
				else if (!stricmp (Arg[3],"GET"))
				{
					rtn = GetVehicleHistory (Arg[2],Arg[4],atob(Arg[5]));
					goto Rtnrtn;
				}
				else
				{
					HWND hWnd;
					
					strcpy (VehicleHistoryID,Arg[2]);
					hWnd = CreateDialog (hInst, (LPSTR)"VEHICLE_HISTORY", hWndMain, (DLGPROC)VEHICLE_HISTORYMsgProc);
					ShowWindow (hWnd,SW_SHOW);
					goto RtnTrue;
				}
			} 
			if (!_fstricmp (Arg[1],"MONITOR"))
			{
				if (*Arg[2])
					rtn = CreateProgMonMap (Arg[2],Arg[3]);
				else
					rtn = DialogBox(hInst, (LPSTR)"PROGRESS_MONITORING", hWndMain, PROGRESS_MONITORINGMsgProc);
				goto Rtnrtn; 
			} 
			if (!_fstricmp (Arg[1],"TIMERDIALOG"))
			{
				if (!stricmp (Arg[2],"START"))
					rtn = StartVehTimeMenu (CurView->hWnd);
				else if (hWndVehTime && VehReplayFid == HFILE_ERROR)
					SetDlgItemText (hWndVehTime,IDC_VEHTIME,Arg[3]);
				goto Rtnrtn; 
			} 
			if (!_fstricmp (Arg[1],"STATUS"))
			{
				if (!stricmp (Arg[2],"CREATE"))
				{
					if (hWndVehStatus)
						rtn = TRUE;
					else
					{
						HWND	hWndDlg;
						LPVIEWPORT	SaveVP;

						SetCurView ( SetVPFromName (Arg[3],&Err));  
						SaveVP = CurView;
						StartVehInfoMenu (SaveVP);
					}
				}
				else if (!stricmp (Arg[2],"SHOW"))
				{
					if (hWndVehStatus)
					{
						int	Control=IDC_VEHDISPLAY_ALL;

						if (!stricmp (Arg[3],"ACTIVE"))
							Control = IDC_VEHDISPLAY_ACTIVE;
						else if (!stricmp (Arg[3],"INSERVICE"))
							Control = IDC_VEHDISPLAY_INSERVICE;
						else if (!stricmp (Arg[3],"AVAILABLE"))
							Control = IDC_VEHDISPLAY_AVAILABLE;
						else if (!stricmp (Arg[3],"FENCE"))
							Control = IDC_VEHDISPLAY_FENCE;
						SendDlgItemMessage (hWndVehStatus,IDC_VEHDISPLAY_ALL,BM_SETCHECK,FALSE,0L);
						SendDlgItemMessage (hWndVehStatus,IDC_VEHDISPLAY_INSERVICE,BM_SETCHECK,FALSE,0L);
						SendDlgItemMessage (hWndVehStatus,IDC_VEHDISPLAY_ACTIVE,BM_SETCHECK,FALSE,0L);
						SendDlgItemMessage (hWndVehStatus,IDC_VEHDISPLAY_AVAILABLE,BM_SETCHECK,FALSE,0L);
						SendDlgItemMessage (hWndVehStatus,IDC_VEHDISPLAY_FENCE,BM_SETCHECK,FALSE,0L);
						SendDlgItemMessage (hWndVehStatus,Control,BM_SETCHECK,TRUE,0L);
						UpdateVehicleStatusDlg ();
						rtn = TRUE;
					}
				}
				else if (!stricmp (Arg[2],"REFRESH"))
				{
					UpdateVehicleStatusDlg ();
					rtn = TRUE;
				}
				else if (!stricmp (Arg[2],"CLOSE"))
				{
					if (hWndVehStatus)
						SendMessage (hWndVehStatus,WM_CLOSE,0,0);
					rtn = TRUE;
				}

				goto Rtnrtn; 
			} 
			if (!_fstricmp (Arg[1],"CLEAR"))
			{
				SetCurView ( SetVPFromName (Arg[2],&Err));  
				rtn = ClearVehicles ();
				goto Rtnrtn;
			}
			goto RtnFalse;
				
		case 766: //$ADDITEM(file.plt,Type(P,A,L,Open,Close,TwopointCurve),TAG,SYMBOLNAME,points,size,rot,text,textsize,TextColor,OpaqueText,Shadow,HIPrecis)
		{
			HANDLE	hStuff=0, hTime=0;
			LPSHORT	pStuff=0;

			nArgs = GetFunArgs (Args,Arg,-16,&hMem, pBrkPt, bpOffset, bpLen);  
			hMem2 = GSSiGlobAlloc ( 913,GMEM_MOVEABLE,USHRT_MAX);
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			ExpandText (Arg[3]);
			ExpandText (Arg[4]);
			ExpandText (Arg[6]);
			ExpandText (Arg[7]);
			ExpandText (Arg[8]);
			ExpandText (Arg[9]);
			ExpandText (Arg[10]);
			ExpandText (Arg[11]);
			ExpandText (Arg[12]);
			ExpandText (Arg[13]);
			ExpandText (Arg[14]);
			ExpandText (Arg[15]);
			ExpandText (Arg[16]);
			Arg1 = GlobalLock(hMem2);
			_fstrcpy (Arg1,Arg[5]); 
			ExpandText (Arg1);
			if (!strnicmp (Arg[2],"CI",2))
				n = 7;
			else
			{
				Arg[2][1] = 0; 
				n = _fstrcspn (" ALPOCT",Arg[2]);
			}
			if (!n)
				goto RtnFalse;
			n--; 
			HiPrecis = TRUE;
			if (*Arg[13])
				HiPrecis = atob (Arg[13]);
			if (SetStuffFromText (Arg[14],atoi(Arg[15]),&hStuff))
				pStuff = GlobalLock (hStuff);
			hTime = SetTimeStampHandle (Arg[16]);
			rtn = AddAreaToMap (Arg[1],Arg[3],Arg[4],Arg1,n,atof(Arg[6]),atof(Arg[7]),Arg[8],atof(Arg[9]),atol(Arg[10]),atob(Arg[11]),atob(Arg[12]),pStuff,hTime);
			GSSiGlobUlFree (&hMem2);
			GSSiGlobUlFree (&hStuff);
			GSSiGlobFree (&hTime);
			if (rtn)
				goto RtnTrue;
			goto RtnFalse;
		} 
		
		case 767: //$TCPPARM(socket,TIMER,seconds,timerstring)
		{	
			SOCKET	socket;
						
			nArgs = GetFunArgs (Args,Arg,-4,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				goto RtnFalse; 
			ExpandText (Arg[1]);
			ExpandText (Arg[2]); 
			ExpandText (Arg[3]);
			socket = atol (Arg[1]); 
			if (!_fstricmp (Arg[2],"TIMER"))
			{ 
				if (SetupSocketTimer (socket,atol(Arg[3]),Arg[4]))
					goto RtnTrue;
			}
			if (!_fstricmp (Arg[2],"TERM"))
			{ 
				if (!socket)
					socket = CurrentServerSocket;
				if (SetupSocketTerminator (socket,Arg[3]))
					goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 768: //$DIRPATH(DESKTOP or DESKTOPDIR or MY DOCUMENTS or ALLUSERAPPDATA or APPDATA or LOCALAPPDATA,subdir(opt - will be created))  
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			_fstrcpy (OutLoc,Arg[1]);
			if (!GetSpecialDirectory (OutLoc))
			{
				if (*Arg[2])
				{
					sprintf (strchr (OutLoc,0),"\\%s",Arg[2]);
					makedirectories (OutLoc,TRUE,FALSE);
				}
				goto Rtnl;
			}
			else 
            	goto RtnFalse;

		case 769: //$NUMERIC(VAL,start,length) 
		{
			int	starting_at,for_how_long;

			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			starting_at = atoi (Arg[2]);
			for_how_long = atoi (Arg[3]);
			if (!starting_at)
				starting_at = 1;
			if (!for_how_long)
				for_how_long = strlen (Arg[1]) - starting_at + 1;
			if (NCHK(Arg[1],starting_at,for_how_long))
				goto RtnTrue;
			goto RtnFalse;
		}
		case 770: //$FILEPOS(GET,FILEID,CURRENT|RECORD|LAST
				  //		(SET,FILEID,FIRST|NEXT|PRIOR|LAST
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			nlong = FilePos (Arg[1],Arg[2],Arg[3],atoi(Arg[4]));
			ltoa (nlong,OutLoc,10);
			goto Rtnl;  
		case 771: //$FILELEN(Pathname)
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			nlong = GSSiLength (Arg[1]);
			ltoa (nlong,OutLoc,10);
			goto Rtnl;  

		case 772: //$TCPFILE(GET,socket,ToFile,FromFile,Delete)
				  //$TCPFILE(SEND,FromFile,Delete)
				  //$TCPFILE(REQUEST,FromFile,BlockSize)
				  //$TCPFILE(SAVE,ToFile,Length)
		{	
			SOCKET	socket;
						
			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			if (!stricmp (Arg[1],"GET"))
			{
				socket = atol (Arg[2]);
				rtn = TCPGetFile (socket,Arg[3],Arg[4],atob(Arg[5]));
			}
			else if (!stricmp (Arg[1],"SEND"))
			{
				rtn = TCPSendFile (Arg[2],500,atob(Arg[3]));
			}
			else if (!stricmp (Arg[1],"REQUEST"))
			{
				rtn = TCPSendFile (Arg[2],atoi(Arg[3]),FALSE);
			}
			else if (!stricmp (Arg[1],"SAVE"))
			{
				rtn = TCPSaveFile (Arg[2],atoi(Arg[3]));
			}
			goto Rtnrtn;
		}
		case 773: //$TOOLBAR(LOAD,FLOAT,Pathname,height,nperrowfloating,pos,DPoint,Scale,vpID)
				  //$TOOLBAR(LOAD,DOCK,Pathname,
			//$TOOLBAR(RELOAD,CURRENT(defalut) or ALL)
			//$TOOLBAR(REDISPLAY,CURRENT(defalut) or ALL)
			nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "LOAD"))
			{
				Point = atopt(Arg[7], &Err);
				RVal = atof(Arg[8]);

				rtn = LoadToolbar(CurView->hWnd, Arg[3], Arg[2], atoi(Arg[4]), atoi(Arg[5]), Arg[6], TRUE, FALSE,&Point,RVal,atoi(Arg[9]));
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			if (!stricmp(Arg[1], "HOVER"))
			{
				rtn = SetToolbarHoverCmd(atoi(Arg[2]), Arg[3]);
			}
			if (!stricmp(Arg[1], "DESTROY"))
			{
				rtn = DestroyCurrentToolbar();
			}
			if (!stricmp(Arg[1], "RELOAD"))
			{
				rtn = ReloadToolbar(Arg[2]);
			}
			if (!stricmp(Arg[1], "REDISPLAY"))
			{
				rtn = RedisplayToolbar(Arg[2]);//not working yet
			}
			goto Rtnrtn;
		case 774: //$NETWORK(FALSEINT,STREETLIST
				  //$NETWORK(FALSEINT,LOAD
				  //$NETWORK(FALSEINT,NEXTREF
				  //$NETWORK(FALSEINT,CLEAR
			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			if (!stricmp (Arg[1],"FALSEINT"))
				rtn = FalseIntFunctions (Arg[2],Arg[3],OutLoc); 
			goto Rtnl;

		case 775: //$LOADMAP()
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			FastMapCopy = TRUE;
			if (*Arg[1])
				strcpy (Arg[6],Arg[1]);
			else
				GSSiGetTempFileName (0,"gm",0,Arg[6]); 
			FastMapCopyHltOnly = atob (Arg[2]);
			FastMapCopyFid = GSSiOpenFile (Arg[6],0,OF_CREATE);
			SetViewport (*pCommandViewport);
			FastMapCopynRecs = 0;
			GetWindowText (CurView->hWnd,Arg[5],255);
			RedisplayViewport (TRUE,TRUE);
			FastMapCopyHltOnly = FALSE;
			SetWindowText (CurView->hWnd,Arg[5]);
			FastMapCopy = FALSE;
			GSSiClose (FastMapCopyFid);
			if (!*Arg[1])
				CopySelectedRecords (Arg[6],TRUE,FALSE,0,FALSE);
			goto RtnTrue;

		case 776: //$AZDIFF(az1,az2)
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			ftoa (OutLoc,DeltaAZ (atof(Arg[1]),atof(Arg[2])));
			goto Rtnl;

		case 777: //$PROCESS(CREATE,program,commandline,newwindowrect,waitforprocesstoend,maxwait,startindir)
				  //$PROCESS(STOP,
				  //$PROCESS(GETWINDOW
			{
				nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
				*OutLoc = 0;
				if (!stricmp (Arg[1],"CREATE"))
				{
					STARTUPINFO si;
					PROCESS_INFORMATION pi;
					DWORD	CRFlags=0;
					RECT	rect;
			
					CloseAllRequestedFiles (FALSE);
					ZeroMemory( &si, sizeof(si) );
					si.cb = sizeof(si);
					ZeroMemory( &pi, sizeof(pi) ); 
					rect = atorect (Arg[4],&Err);
					if (!Err)
					{
						si.dwX = rect.left;
						si.dwY = rect.top;
						si.dwXSize = RECTWIDTH (&rect);
						si.dwYSize = RECTHEIGHT (&rect);
						si.dwFlags = STARTF_USEPOSITION|STARTF_USESIZE;
					}
					if (!atob (Arg[5]))
						CRFlags = DETACHED_PROCESS;
					else
						CRFlags = CREATE_NEW_CONSOLE;
					ExpandText(Arg[3]);
					if (!*Arg[7])
					{
						strcpy (Arg[7],"[%DL]");
						ExpandText (Arg[7]);
						*LastChr (Arg[7]) = 0;
					}
					if(CreateProcess(0,Arg[3], 
										NULL,             // Process handle not inheritable. 
										NULL,             // Thread handle not inheritable. 
										FALSE,            // Set handle inheritance to FALSE. 
										CRFlags,		  // creation flags. 
										NULL,             // Use parent's environment block. 
										Arg[7],             // Use parent's starting directory. 
										&si,              // Pointer to STARTUPINFO structure.
										&pi )             // Pointer to PROCESS_INFORMATION structure.
						) 
					{
						BOOL TimedOut;
						int	 MaxWait = atol (Arg[6]);

						WaitForInputIdle (pi.hProcess,INFINITE);
						ltoa ((long)pi.hProcess,OutLoc,10);
						if (atob (Arg[5]))
						{
							while (WaitForProcessToEnd (pi.dwProcessId,&MaxWait));
							if (MaxWait)
								goto RtnTrue;
							else
								goto RtnFalse;
						}
					}
					else
					{
						ltoa (-((int)GetLastError ()),OutLoc,10);
					}
				}
				else if (!stricmp (Arg[1],"STOP"))
				{
					//TerminateProcess (pi.hProcess,0);
					//CloseHandle( pi.hProcess );
					//CloseHandle( pi.hThread );
				}
				else if (!stricmp (Arg[1],"GETWINDOW"))
				{
					HANDLE	hProcess = (HANDLE)atol(Arg[2]);
					DWORD	ProcessID = GetProcessId(hProcess);
					HWND	hWnd,hWndPar;
					char	txt[256];

					WaitForInputIdle(hProcess,2000);
					hWnd = FindWindowByProcessID (ProcessID,Arg[3]);
					hWndPar = GetParent (hWnd);
					GetWindowText (hWnd,txt,256);
					while (hWndPar)
					{
						hWnd = hWndPar;
						GetWindowText (hWnd,txt,256);
						hWndPar = GetParent (hWnd);
					}
					ltoa ((long)hWnd,OutLoc,10);
				}
				goto Rtnl;

			}

		case 778: //$SESSION(CREATE,commandline,newwindowrect,startupzoom)
				  //$SESSION(STOP,hwnd
				  //$SESSION(COMMAND,hwnd
			{
				char modulePath[MAX_PATH];

				nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen); 
				*OutLoc = 0;
				if (!stricmp (Arg[1],"CREATE"))
				{
					STARTUPINFO si;
					PROCESS_INFORMATION pi;
					DWORD	CRFlags=0;
					RECT	rect;
					char	startIn[MAX_PATH] = "[%DL]";
					LPSTR	pstartIn = startIn;
					//MNMXCORD zoomBounds;
			
					CloseAllRequestedFiles (FALSE);
					GetModuleFileName(NULL,modulePath,MAX_PATH);
					ZeroMemory( &si, sizeof(si) );
					si.cb = sizeof(si);
					ZeroMemory( &pi, sizeof(pi) ); 
					rect = atorect (Arg[3],&Err);
					if (!Err)
					{
						si.dwX = rect.left;
						si.dwY = rect.top;
						si.dwXSize = RECTWIDTH (&rect);
						si.dwYSize = RECTHEIGHT (&rect);
						si.dwFlags = STARTF_USEPOSITION|STARTF_USESIZE|STARTF_USESHOWWINDOW ;
					}
					//zoomBounds = atobounds(Arg[4],ierr);
					CRFlags = DETACHED_PROCESS;
					// for mapserver addd BELOW_NORMAL_PRIORITY_CLASS
					//ExpandText(Arg[2]);
					if (*TestFileLocation)
						sprintf (strchr(Arg[2],0)," [%%TESTDL]=%s;",TestFileLocation);
					ExpandText(startIn);
					if (!*startIn)
						pstartIn = NULL;
					if(CreateProcess(modulePath,Arg[2], 
										NULL,             // Process handle not inheritable. 
										NULL,             // Thread handle not inheritable. 
										FALSE,            // Set handle inheritance to FALSE. 
										CRFlags,		  // creation flags. 
										NULL,             // Use parent's environment block. 
										pstartIn,             // Use parent's starting directory. 
										&si,              // Pointer to STARTUPINFO structure.
										&pi )             // Pointer to PROCESS_INFORMATION structure.
						) 
					{
						BOOL TimedOut;
						int	 MaxWait = atol (Arg[5]);
						DWORD	ProcessID = GetProcessId(pi.hProcess);

						//Wait (1000);

						WaitForInputIdle (pi.hProcess,INFINITE);
						hWnd = FindWindowByProcessID (ProcessID,"");
						ltoa ((long)hWnd,OutLoc,10);
						if (atob (Arg[5]))
						{
							while (WaitForProcessToEnd (pi.dwProcessId,&MaxWait));
							if (MaxWait)
								goto RtnTrue;
							else
								goto RtnFalse;
						}
					}
					else
					{
						ltoa (-((int)GetLastError ()),OutLoc,10);
					}
				}
				else if (!stricmp (Arg[1],"STOP"))
				{
					HWND hProcessWnd = (HWND)atol (Arg[2]);
					//TerminateProcess (pi.hProcess,0);
					//CloseHandle( pi.hProcess );
					//CloseHandle( pi.hThread );
					SendConnectedProcessMessage (hProcessWnd,GF_END_PROCESS,0,0);
					goto RtnTrue;
				}
				else if (!stricmp (Arg[1],"COMMAND"))
				{
					HWND hProcessWnd = (HWND)atol (Arg[2]);
					SendConnectedProcessCommand (hProcessWnd,Arg[2]);
					goto RtnTrue;
				}
				else if (!stricmp(Arg[1], "PATH"))
				{
					GetModuleFileName(NULL, OutLoc, MAX_PATH);
				}
				goto Rtnl;

			}

		case 779: //$COMPOSE(Title,InMessage(opt),SendMacro)
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (ComposeMessage (hWndMain,Arg[1],Arg[2],Arg[3]))
				goto RtnTrue;
			goto RtnFalse;

		case 780: //$BATTERY(EXISTS)
				  //$BATTERY(REMAINING)
				  //$BATTERY(CHARGING)
			{
				SYSTEM_POWER_STATUS sps;
				BOOL rtn=GetSystemPowerStatus (&sps);


				nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
				if (!stricmp (Arg[1],"EXISTS"))
				{
					if (sps.BatteryFlag == 128)
						strcpy (OutLoc,"0");
					else
						strcpy (OutLoc,"1");
				}
				else if (!stricmp (Arg[1],"REMAINING"))
				{
					itoa (sps.BatteryLifePercent,OutLoc,10);
				}
				else
				{
					if (sps.BatteryFlag < 128 && sps.BatteryFlag & 8)
						strcpy (OutLoc,"0");
					else
						strcpy (OutLoc,"1");
				}
				goto Rtnl;
			}

		case 781: //$GEOCODE(FORWARD,)
			//$GEOCODE(REVERSE,point,format)
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (!stricmp(Arg[1], "REVERSE"))
				ReverseGeocodeCommand(nArgs - 1, &Arg[1], OutLoc);
			else if (!stricmp(Arg[1], "ALLTYPES"))
				GeocodeAlltypes(hWndMain, OutLoc, Arg[2]);
			goto Rtnl;
		case 782: //$ADDRESS(SET,(I2orI4orR4orR8),address,value)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "SET"))
			{
				if (!stricmp(Arg[2], "I4"))
				{
					LPINT pI4 = (LPINT)atoi(Arg[3]);
					*pI4 = atoi(Arg[4]);
					goto RtnTrue;
				}
			}
			goto RtnFalse;
		}
		case 783: //$TESTENV(STORE,testdir)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "STORE"))
			{
				rtn = StoreTestToProduction(Arg[2], 1);
				goto Rtnrtn;
			}
			goto RtnFalse;
		}

		case 784: //$COPYDIR(TODIR,FROMDIR,T or F replace,T or F display status window)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (!FileType(Arg[2]) == 2)
			{
				sprintf(OutLoc, "%s is not a directory", Arg[2]);
				goto Rtnl;
			}
			if (FileType(Arg[1]) && !atob (Arg[3]))
			{
				sprintf(OutLoc, "%s already exists", Arg[1]);
				goto Rtnl;
			}
			rtn = CopyDirectory(Arg[1], Arg[2], atob(Arg[3]), Arg[4]);
			goto Rtnrtn;
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
		SetConfig (SaveCfg);
		if (*pNumViewports)
			SetCurView ( SaveVP);
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

		
			
