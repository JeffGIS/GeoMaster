#include "graphint.h"   
#include "pnet.h"
#include "commctrl.h"

#define PROMPTWINDOWCLASS	"PromptWindowClass"
#define MAXWINDOWS			256
#define DISPLAY_TIMER_ID	1
#define FADE_TIMER_ID		2
#define DISPLAY_TIMER_DURATION 3000
#define FADE_TIMER_DURATION 30


BOOL	ddbug=FALSE;
typedef struct {
				short	SymNum,  
						Lev,
						Up,
						Next,
						Down, 		// 0 if not parent
						AllSome;
				char	Name[34],
						Desc[18];
				} SYMLIST;
typedef SYMLIST		FAR	*LPSYMLIST;

typedef struct {
				HWND hWnd;
				HDC	 hDCMem;
				RECT InRect;
				int	 nWin;
				HWND hWindows[MAXWINDOWS];
				} GSWWSTRUCT;
typedef GSWWSTRUCT	*LPGSWWSTRUCT;


extern	BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam); 

#include "gmextern.h"

#include <sys\types.h>
#include <sys\stat.h>         

static	BOOL	HaveDummyDelete=FALSE;
static	char	LastPromptText[256]=""; 
static	HANDLE	hPromptList=0;
static	short	nPromptList;
static	short	lNewTAG;  
static	PICKDATA	SavedPickList[MAXPICKITEMS+3];  
static	short	nSavePickList=0;
static	char	SysMess[256];
static	OFSTRUCTGM	OFStruct;
static	LPSTR	fgstat, origstr;
static	char	text[4096];
static	int		nwin;
static	BOOL	HavePromptText;
static	BOOL	InFade;


short OkToContinue2 (BOOL ForceCheck);
short OkToContinue (BOOL ForceCheck)
#if ENABLETRACE
{GSSiEnterProg (871);
#endif
{   
	short	rtn;
	
	if (MemMap || Printing || RunFromCache || GetGlobalBVal2 ("[%IGNORESYSMSG]",FALSE))
{
#if ENABLETRACE
GSSiExitProg (871);
#endif
		return TRUE;
}
	while ((rtn = OkToContinue2 (ForceCheck)) == 2)
    {
      DLGPROC lpfnWAITMESSAGEMsgProc;
      short	nRc;
	              
	  HaltMapDisplay(FALSE,FALSE); 
	  CloseAllRequestedFiles (FALSE);
	  ODBCTerminate (TRUE);
      lpfnWAITMESSAGEMsgProc = MakeProcInstance((DLGPROC)WAITMESSAGEMsgProc, hInst);
      nRc = DialogBox(hInst, (LPSTR)"WAITMESSAGE", hWndMain, lpfnWAITMESSAGEMsgProc);
      FreeProcInstance(lpfnWAITMESSAGEMsgProc);
	  if (!nRc)
{
#if ENABLETRACE
GSSiExitProg (871);
#endif
	  	return FALSE;
}
	  ForceCheck = TRUE;
	
    }
{
#if ENABLETRACE
GSSiExitProg (871);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

short OkToContinue2 (BOOL ForceCheck)
#if ENABLETRACE
{GSSiEnterProg (872);
#endif
{   
	DWORD	now, elapsed;
	static	DWORD	last_check=0;     
	long	last_msg_time, msg_time;
	int	ii, l;    
	OFSTRUCTGM	OFStruct;
	char	str[128], macro[256];
	BOOL	SaveKFO	= KeepFilesOpen;
	
	now = GetTickCount();
	elapsed = now - last_check;  
	if (!PeopleNet && (elapsed > max_check_time || ForceCheck))
	{
		char	MessageFile[MAX_PATH]="[%DATA_LOC]System Message.txt", type[36];   
		char	UserIni[MAX_PATH]="[%USERDIR]geomastr.ini";
		HFILE	Fid;
		struct _stati64    statmsg; 
		BOOL	SaveAC = AllowCache;
		
		ExpandText (MessageFile);
		ExpandText (UserIni);
//		GetShortPathName (MessageFile,128);
		AllowCache = FALSE;
		KeepFilesOpen = FALSE;
		Fid = GSSiOpenFile (MessageFile,&OFStruct,OF_READ); 
		AllowCache = SaveAC;
		if (Fid == HFILE_ERROR)
		{
			last_check = now;
			KeepFilesOpen = SaveKFO;
{
#if ENABLETRACE
GSSiExitProg (872);
#endif
			return 1;
}
		} 
		fgetstring (type,sizeof(type)-4,Fid);
		l=BigRead (Fid,GMmess,sizeof(GMmess)-1); 
		GMmess[l]= 0;
		ii=GSSifstat (Fid,&statmsg);
		GSSiClose2 (&Fid);
		switch (*type)
		{    
			case 'U':
			case 'u':
				KeepFilesOpen = SaveKFO;
				return 3;
			case 'A':
			case 'a':
 				setDoPaint( FALSE);
				HaltMapDisplay(FALSE,TRUE);
				GSSiMsgBox( GetFocus(),GMmess,"GeoMaster System Message", MB_OK|MB_ICONEXCLAMATION,0);
 				BlowOut(0,0);
 			case 'M':
 			case 'm':  
 			{
				last_check = now;   
				GetPrivateProfileString("User", "LastSysMsg","0",str,sizeof(str),UserIni);
    			last_msg_time = atol(str);  
    			sprintf (macro,"$MACRO([%%DL]macros\\System Message Macro.txt,%ld)",last_msg_time);
    			ExpandText (macro); 
    			msg_time = atol (macro);
    			if (msg_time <= last_msg_time)
				{
					KeepFilesOpen = SaveKFO;
//    			if (last_msg_time == statmsg.st_mtime)
{
#if ENABLETRACE
GSSiExitProg (872);
#endif
    				return 1;
}
				}
    			ltoa (msg_time,str,10);
    			WritePrivateProfileString("User", "LastSysMsg",str,UserIni);
 				setDoPaint( FALSE);
//				GSSiMsgBox( GetFocus(),GMmess,"GeoMaster System Message", MB_OK|MB_ICONEXCLAMATION);
				setDoPaint( TRUE);
				KeepFilesOpen = SaveKFO;
{
#if ENABLETRACE
GSSiExitProg (872);
#endif
 				return 1;  
}
 			}
 			case 'S':
 			case 's': 
				HaltMapDisplay(FALSE,TRUE);
				GSSiMsgBox( GetFocus(),GMmess,"GeoMaster System Message", MB_OK|MB_ICONEXCLAMATION,0);
				KeepFilesOpen = SaveKFO;
{
#if ENABLETRACE
GSSiExitProg (872);
#endif
 				return 0;
}
 			case 'W':
 			case 'w': 
			KeepFilesOpen = SaveKFO;
{
#if ENABLETRACE
GSSiExitProg (872);
#endif
 				return 2;
}
 		}

	}
	KeepFilesOpen = SaveKFO;
{
#if ENABLETRACE
GSSiExitProg (872);
#endif
	return TRUE;
}
	
#if ENABLETRACE
}
#endif
}

 

short GetCDDriveForVolWait (LPSTR VolLabel,LPSTR Drive)
#if ENABLETRACE
{GSSiEnterProg (874);
#endif
{   
	DLGPROC lpfnWAITMESSAGEMsgProc;
	short	nRc;
	BOOL	SaveDP = DoPaint(), SaveDH = DisableHalt;
	
	if (GetCDDriveForVol (VolLabel,Drive))
{
#if ENABLETRACE
GSSiExitProg (874);
#endif
		return 1; 
}
	if (StopCDPrompts)
{
#if ENABLETRACE
GSSiExitProg (874);
#endif
		return -1;
}
	sprintf (GMmess,"Please load the CD named %s",VolLabel);
	_fstrcpy (WaitForVolLabel,VolLabel);
	HaltMapDisplay(FALSE,TRUE);  
	setDoPaint( FALSE); 
	DisableHalt = TRUE;
	lpfnWAITMESSAGEMsgProc = MakeProcInstance((DLGPROC)WAITMESSAGECDMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"WAITMESSAGECD", hWndMain, lpfnWAITMESSAGEMsgProc);
	FreeProcInstance(lpfnWAITMESSAGEMsgProc);
	setDoPaint( SaveDP);
	DisableHalt = SaveDH; 
	_fstrcpy (Drive,WaitForDrive);
{
#if ENABLETRACE
GSSiExitProg (874);
#endif
	return nRc; 
}
#if ENABLETRACE
}
#endif
}
 


BOOL SelectDrive (int Type,LPSTR Drive)
#if ENABLETRACE
{GSSiEnterProg (876);
#endif
{
	DLGPROC	lpfnSELECTDRIVEMsgProc;
	short		nRc; 
    
    WantDriveType = Type;              
	lpfnSELECTDRIVEMsgProc = MakeProcInstance((DLGPROC)SELECTDRIVEMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SELECTDRIVE", hWndMain, lpfnSELECTDRIVEMsgProc);
	FreeProcInstance(lpfnSELECTDRIVEMsgProc);  
    if (nRc)
    	_fstrcpy (Drive,WaitForDrive);
{
#if ENABLETRACE
GSSiExitProg (876);
#endif
    return nRc;
}
#if ENABLETRACE
}
#endif
}

 

BOOL EditSym (short Item,short NewDesc) 
{   
	short	ID;  
	char	SymName[64]; 
	HFILE	Fid;
	
	if (!NewDesc)
		return FALSE;
	if (!CurView->UpdateFile || !_fstricmp (EditName,PickName))
	{
	    CurDescLoc=0;
		ProcessPickedItem (Item,FALSE);        		
       	if (CurDescLoc)
       	{   
    	    DisableHalt = TRUE;
		       		
			if (NewDesc>0)
			{
				Fid = GSSiOpenFile (PickName,0,OF_READWRITE);
				if (Fid != HFILE_ERROR)
				{
					GSSillseek (Fid,CurDescLoc,0);
					BigRead (Fid,(HPSTR)&ID,2);
					if (ID == 8)
						BigWrite (Fid,(HPSTR)&NewDesc,2,-1);
					GSSiClose2 (&Fid);
					if (!GetSymbolName (NewDesc,SymName,0,2,0))
					{   
						HANDLE	hSymDesc=0;
						short	NumSyms=0;
								
			    		_fstrcpy (PltName,PickName);
			    		PltType = 2;
						AddToSymList (NewDesc,&NumSyms,&hSymDesc); 
						AddSymToMap (NumSyms,hSymDesc,0,0); 
		                DestroySymList (&NumSyms,&hSymDesc);
					} 
				}
			}
		}
	}	
	else
	{
		UpdateItem=8; 
		NewSymbol = NewDesc;
		UpdateRecord (0,NewSymbol,0,0,0,0,1,-1);
	}  
	return TRUE;
}

BOOL EditTAG (short Item,LPSTR NewTAGVal)  
{
	BOOL	SaveDisableHalt, SaveFTI=ForceTAGIndex, OpenedTAGList; 
	short	ltag;
   	short	ID, ls, TAGHead, ID2;  
   	HFILE	Fid; 
	char	NewPrefix[10], NewUDI[66]; 
	LPSTR	pColon;
    
    _fstrcpy (NewTAG,NewTAGVal);
    lNewTAG = _fstrlen (NewTAG);   		
	if (!CurView->UpdateFile)
		*EditName = 0;
	else
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
	if ((pColon = _fstrchr (NewTAG,':')))
	{
		*pColon = 0;
		strncpy0 (NewPrefix,NewTAG,8);
		*pColon++ = ':';
		_fstrcpy (NewUDI,pColon); 
		sprintf (NewTAG,"%s:%s",NewPrefix,NewUDI);
		lNewTAG = strlen (NewTAG);
	}
	else
		return FALSE;
	SaveDisableHalt = DisableHalt;
    DisableHalt = TRUE;
	if (!CurView->UpdateFile || !_fstricmp (EditName,PickName))
	{   
		_fstrcpy (PltName,PickName);
		if (lNewTAG <= CurlTAG)
		{
		    ForceRefIndex = ForceTAGIndex = TRUE;  
			OpenedTAGList = OpenTAGIndex (FALSE,StoreTAGBounds,0);
			DeleteFromTAGList (PickList[Item].Prefix,PickList[Item].UDI,PickList[Item].Refno);
			if (OpenedTAGList)
				CloseTAGIndex ();
			ForceTAGIndex = SaveFTI;		
			Fid = GSSiOpenFile (PickName,0,OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{
				GSSillseek (Fid,CurTAGLoc,0);
				BigRead (Fid,(HPSTR)&ID2,2); 
				ID = LOBYTE (ID2);
				ltag = HIBYTE (ID2);
				if (ID == 9)
				{
					GSSillseek (Fid,4,1);
					BigWrite (Fid,(HPSTR)NewTAG,CurlTAG,-1);
				}
				GSSiClose2 (&Fid);
			} 
			SaveFTI = ForceTAGIndex;
			ForceTAGIndex = ForceRefIndex = TRUE;
			ProcessPickedItem (Item,0);        		
			ForceTAGIndex = ForceRefIndex = SaveFTI; 
		}
		else
		{   
			UpdateItem=9;
			UpdateRecord (Item,PickList[Item].Desc,NewPrefix,NewUDI,0,0,0,-1);
		}
	} 
	else
	{   
		UpdateItem=9;
		UpdateRecord (Item,PickList[Item].Desc,NewPrefix,NewUDI,0,0,1,-1);
	}
   	DisableHalt = SaveDisableHalt;

	return TRUE; 
}

 

 





void InitDlgPrompts (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (882);
#endif
{
	if (hWnd)
		PromptFocus = hWnd;
	else
		PromptFocus = hWndMain;
{
#if ENABLETRACE
GSSiExitProg (882);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetDlgPrompt (HWND hWnd,UINT Prompt, UINT More)
#if ENABLETRACE
{GSSiEnterProg (883);
#endif
{   
	LPDWORD	pPrompt;
	
	if (!hWnd)
{
#if ENABLETRACE
GSSiExitProg (883);
#endif
		return;
}
	if (!hPromptList)
	{
		hPromptList = GSSiGlobAlloc ( 643,GMEM_MOVEABLE,256*3*sizeof(DWORD));
		nPromptList = 0;  
	}
	pPrompt = (LPDWORD)GlobalLock (hPromptList); 
	if (!pPrompt)
{
#if ENABLETRACE
GSSiExitProg (883);
#endif
		return;
}
	pPrompt += (3 * nPromptList); 
	*pPrompt++ = (DWORD)hWnd;
	*pPrompt++ = Prompt;
	*pPrompt = More;
	nPromptList++;
	GlobalUnlock (hPromptList);
{
#if ENABLETRACE
GSSiExitProg (883);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ClearDlgPrompts (void)
#if ENABLETRACE
{GSSiEnterProg (884);
#endif
{   
	GSSiGlobFree (&hPromptList);
{
#if ENABLETRACE
GSSiExitProg (884);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

DWORD GetDlgItemPrompt (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (885);
#endif
{   
	LPDWORD	pPrompt; 
	UINT	Prompt, More; 
	short	n;
	
	if (!hPromptList)
{
#if ENABLETRACE
GSSiExitProg (885);
#endif
		return 0;
}
	pPrompt = (LPDWORD)GlobalLock (hPromptList);  
	n = nPromptList;
	while (n--)
	{ 
		if (*pPrompt++ == (DWORD)hWnd)
		{
			Prompt = *pPrompt++;
			More = *pPrompt;
			GlobalUnlock (hPromptList);
{
#if ENABLETRACE
GSSiExitProg (885);
#endif
			return MAKELONG (Prompt,More);	
}
		}
		pPrompt+=2;
	}
	GlobalUnlock (hPromptList);
{
#if ENABLETRACE
GSSiExitProg (885);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
} 

BOOL CALLBACK GSWWEnumWndProc(HWND hWnd,LONG lParam)
{

	LPGSWWSTRUCT	pGSWWStruct = (LPGSWWSTRUCT)lParam;

	if (hWnd != pGSWWStruct->hWnd && IsWindowVisible (hWnd))
	{
		RECT rect, intRect;

		GetWindowRect (hWnd,&rect);
		if (IntersectRect (&intRect,&rect,&pGSWWStruct->InRect))
			pGSWWStruct->hWindows[pGSWWStruct->nWin++] = hWnd;
	}
	return TRUE;
}

HDC GetScreenWithoutWindow (HWND hWnd,LPRECT pRect,HBITMAP *phBMOld)
{
	GSWWSTRUCT	GSWWStruct;
	HDC	hDCWnd = GetDC (hWnd);
	HBITMAP	hBM;

	GSWWStruct.hDCMem = CreateCompatibleDC (hDCWnd);
	curProgID = 10012;
	hBM = CreateCompatibleBitmap (hDCWnd,RECTWIDTH(pRect),RECTHEIGHT(pRect));
	curProgID = -1;
	*phBMOld = SelectObject (GSWWStruct.hDCMem,hBM);
	GSWWStruct.hWnd = hWnd;
	GSWWStruct.InRect = *pRect;
	GSWWStruct.nWin=0;
	EnumWindows (GSWWEnumWndProc,(LPARAM)&GSWWStruct); 
	while (GSWWStruct.nWin--)
	{
		HDC	hDC = GetWindowDC (GSWWStruct.hWindows[GSWWStruct.nWin]);
		int	destx, desty, srcx, srcy;
		RECT	rect, intRect;

		GetWindowRect (GSWWStruct.hWindows[GSWWStruct.nWin],&rect);
		IntersectRect (&intRect,&rect,pRect);

		destx = max (0,intRect.left - pRect->left);
		desty = max (0,intRect.top - pRect->top);
		srcx  = max (0,intRect.left - rect.left);
		srcy  = max (0,intRect.top - rect.top);
		BitBlt (GSWWStruct.hDCMem,destx,desty,RECTWIDTH(&intRect),RECTHEIGHT(&intRect),hDC,srcx,srcy,SRCCOPY);

		ReleaseDC (GSWWStruct.hWindows[GSWWStruct.nWin],hDC);
	}
	
	return GSWWStruct.hDCMem;
	
}

LONG FAR PASCAL PromptWndProc(HWND hWnd, int Message, WPARAM wParam, LONG lParam)
{
	RECT	rect;
	static	int fadeout, incfade, blend;
#define NUMFADE	32


	switch (Message)
   {
		case WM_DESTROY:
			KillTimer (hWnd, 1);
			KillTimer (hWnd,2);
			hWndPrompt = 0;
			return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
		
		case WM_COMMAND:
			switch(LOWORD(wParam))
            {
			case IDOK:
				fadeout = NUMFADE;
				incfade = 150 / NUMFADE;
				blend = 0;
				SetTimer(hWnd,DISPLAY_TIMER_ID,DISPLAY_TIMER_DURATION,0); 
				//ShowWindow (hWnd,SW_SHOW);
				InvalidateRect (hWnd,0,TRUE);
				break;
			case IDCANCEL:
				KillTimer (hWnd,1);
				break;
			}
			
			return 0;

		case WM_TIMER:
			if (wParam == DISPLAY_TIMER_ID)
			{
				KillTimer(hWnd, wParam);
				InFade = TRUE;
				SetTimer(hWnd,FADE_TIMER_ID,20,0); 
			}
			else
			{
				if (!fadeout)
				{
					KillTimer (hWnd,wParam);
					ShowWindow (hWnd,SW_HIDE);
					InFade = FALSE;
				}
				else
				{
					BLENDFUNCTION bf;
					RECT	rect, rectM;
					HDC		hDCScreen;
					HDC		hDC = GetWindowDC (hWnd);
					HBITMAP	hBMOld, hBM;
					int	ii;

					GetWindowRect (hWnd,&rect);
					rectM = rect;
					hDCScreen = GetScreenWithoutWindow (hWnd,&rect,&hBMOld);
					ScreenRectToClientRect (hWndMain,&rectM);
					fadeout--;
					blend += incfade;
					bf.BlendOp = AC_SRC_OVER;
					bf.BlendFlags = 0;
					bf.AlphaFormat = 0;
					bf.SourceConstantAlpha = blend;
					ii=AlphaBlend(hDC,0,0,RECTWIDTH(&rect),RECTHEIGHT(&rect),hDCScreen, 0,0,RECTWIDTH(&rect),RECTHEIGHT(&rect), bf);
	//							hDCScreen,rectM.left,rectM.top,RECTWIDTH(&rect),RECTHEIGHT(&rect),bf);
					hBM = SelectObject (hDCScreen,hBMOld);
					GSSiDeleteObject (&hBM);
					DeleteDC (hDCScreen);
					ReleaseDC (hWnd,hDC);
				}
			}
			return 0;

		case WM_PAINT:
			if (fadeout == NUMFADE)
			{
				char	str[1024];
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hWnd,&ps);
				HFONT	hOldFont = SelectObject (hDC,GetStockObject (ANSI_VAR_FONT));

				GetWindowText (hWnd,str,1023);
				GetClientRect (hWnd,&rect);
				FillRect (hDC,&rect,GetStockObject (WHITE_BRUSH));
				InflateRect (&rect,-1,-1);
				DrawText (hDC,str,strlen(str),&rect,DT_LEFT);
				SelectObject (hDC,hOldFont);
				EndPaint (hWnd,&ps);
				InFade = FALSE;
			}
			else
				InFade = TRUE;
			return 0;
		case WM_ERASEBKGND:
			{
				HDC hDC=GetDC (hWnd);

				GetClientRect (hWnd,&rect);
				FillRect (hDC,&rect,GetStockObject (WHITE_BRUSH));
				ReleaseDC (hWnd,hDC);
			}
			return 1;

		case WM_EXITSIZEMOVE:
			return DefWindowProc(hWnd, Message, wParam, lParam);

		case WM_MOVING:
			return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}


BOOL RegisterPromptClass(BOOL UnRegister)
{
    WNDCLASS  wc; 
    static	Called=FALSE;

	if (UnRegister)
	{
		 if (Called)
		 {
			 UnregisterClass(PROMPTWINDOWCLASS, hInst);
			 Called = FALSE;
		 }
		 return TRUE;
	}
    if (Called) return TRUE;
    Called = TRUE;
    wc.style = CS_DBLCLKS|CS_SAVEBITS|CS_DROPSHADOW;
    wc.lpfnWndProc = (WNDPROC)PromptWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = PROMPTWINDOWCLASS;

    return (RegisterClass(&wc));
}

void MovePromptWindow (HWND hWnd,LPARAM lParam,int w, int h)
{
	RECT	rectW;
	POINT	midpt, pt;

	GetWindowRect (GetDesktopWindow(),&rectW);
	midpt = RectMid (&rectW);
	if (lParam < INT_MAX)
	{
		pt = 	POINTStoPOINT(MAKEPOINTS(lParam));
		ClientToScreen (hWnd,&pt);
	}
	else
		GetCursorPos (&pt);
	if (pt.x > midpt.x)
		pt.x -= w;
	if (pt.y > midpt.y)
		//MoveWindow (hWndPrompt,pt.x,pt.y-h*2,w,h,TRUE);
		SetWindowPos (hWndPrompt,0,pt.x,pt.y-h*2,w,h,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
	else
		//MoveWindow (hWndPrompt,pt.x,pt.y+h*2,w,h,TRUE);
		SetWindowPos (hWndPrompt,0,pt.x,pt.y+h*2,w,h,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
	return;
}

void CreatePromptWindow (LPSTR PromptText)
{
	static	int ii=0;
	HBITMAP	SavehFullWindowBitMap = hFullWindowBitMap;

	return;
	hFullWindowBitMap = 0;
	if (ii++ > 100)
		ii=0;
	if (PromptText)
	{
		HDC	hDC = GetDC (hWndMain);
		HFONT	hOldFont = SelectObject (hDC,GetStockObject (ANSI_VAR_FONT));
		SIZE	txSize;
		POINT	pt;

		GetTextExtentPoint32 (hDC,PromptText,strlen (PromptText),&txSize);
		SelectObject (hDC,hOldFont);
		ReleaseDC (hWndMain,hDC);
		GetCursorPos (&pt);
	    if (!hWndPrompt)
		{

			RegisterPromptClass (FALSE);
			hWndPrompt = CreateWindowEx(0, PROMPTWINDOWCLASS, (LPSTR) NULL,
										WS_BORDER|WS_POPUP,
										pt.x, pt.y, txSize.cx+6,txSize.cy+5,
										hWndMain, (HMENU) NULL, hInst, NULL); 
			SetWindowText (hWndPrompt,PromptText);
			MovePromptWindow (0,INT_MAX,txSize.cx+6,txSize.cy+5);
			HavePromptText = TRUE;
		}
		else if (!*PromptText)
		{
			KillTimer(hWndPrompt, DISPLAY_TIMER_ID);
			KillTimer(hWndPrompt, FADE_TIMER_ID);
			SetTimer(hWndPrompt,DISPLAY_TIMER_ID,FADE_TIMER_DURATION,0);
			InFade = TRUE;
			HavePromptText = FALSE;
		}
		else
		{
			SetWindowText (hWndPrompt,PromptText);
			KillTimer(hWndPrompt, DISPLAY_TIMER_ID);
			KillTimer(hWndPrompt, FADE_TIMER_ID);
			SetTimer(hWndPrompt,DISPLAY_TIMER_ID,DISPLAY_TIMER_DURATION,0);
			MoveWindow (hWndPrompt,pt.x,pt.y,txSize.cx+6,txSize.cy+5,TRUE);
			HavePromptText = TRUE;
			PostMessage(hWndPrompt, WM_COMMAND, IDOK, 0L);
			MovePromptWindow (0,INT_MAX,txSize.cx+6,txSize.cy+5);
			ShowWindow (hWndPrompt,SW_SHOW);
		}
	}
	else if (HavePromptText)
	{
		KillTimer(hWndPrompt, DISPLAY_TIMER_ID);
		KillTimer(hWndPrompt, FADE_TIMER_ID);
		SetTimer(hWndPrompt,DISPLAY_TIMER_ID,FADE_TIMER_DURATION,0);
		InFade = FALSE;
		HavePromptText = FALSE;
	}
//	else
//		ShowWindow (hWndPrompt,SW_HIDE);

	hFullWindowBitMap = SavehFullWindowBitMap;
	return;
}

void MovePromptMessage (HWND hWnd,LPARAM lParam)
{
	POINT	pt, midpt;
	static	int	ii=0;

	if (ii++ > 100)
		ii = 0;

	if (hWndPrompt  && IsWindowVisible(hWndPrompt))
	{
		RECT	rectP;

		GetWindowRect (hWndPrompt,&rectP);
		MovePromptWindow (hWnd,lParam,RECTWIDTH(&rectP),RECTHEIGHT(&rectP));
		if (HavePromptText && !InFade)
		{
			KillTimer(hWndPrompt, DISPLAY_TIMER_ID);
			KillTimer(hWndPrompt, FADE_TIMER_ID);
			SetTimer(hWndPrompt,DISPLAY_TIMER_ID,DISPLAY_TIMER_DURATION,0);
		}
	}
	return;
}

void DisplayPromptText (HDC hDC,LPSTR txtIN)
#if ENABLETRACE
{GSSiEnterProg (886);
#endif
{   
	HFONT hFont, OldFont;  
	RECT	Rect=PromptRect;
	short	FontSize;  
	BOOL	GotDC=FALSE;
	LPSTR	txt=txtIN;

	if (!CurrentConfig && txtIN && *txtIN)
		return;
	if (!txt)
		txt = LastPromptText;

	if (!hDC)
	{
		hDC=GetDC(hWndMain);   
		GotDC=TRUE;
	}
	FontSize=PromptRect.bottom - PromptRect.top - 2;
	SaveDC (hDC);
    SetDisplayMode (hDC, GF_SCREENMODE);
    SetBkMode(hDC, TRANSPARENT);  
  	SelectClipRgn (hDC,0);   
  	SetTextColor (hDC,0); 
	if (!Printing)
		FillRect(hDC, &Rect, GetStockObject(LTGRAY_BRUSH));
	else
		ii = 1;
	if (*txt)
	{
		Rect.left+=2;
		hFont = CreateFont(FontSize, 0, 0, 0, FW_BOLD, 
    						0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
		OldFont = SelectObject (hDC,hFont);
		DrawText (hDC,txt,_fstrlen(txt),&Rect,DT_LEFT); 
		SelectObject (hDC,OldFont);
		DeleteObject (hFont);
		if (strcmp (txt,LastPromptText))
			CreatePromptWindow (txt);
	}
	else
		CreatePromptWindow (0);
	if (!txtIN)
		CreatePromptWindow (0);
	else if (*txt)
		strcpy (LastPromptText,txt);
	RestoreDC (hDC,-1);	
	if (GotDC)
		ReleaseDC (hWndMain,hDC); 
{
#if ENABLETRACE
GSSiExitProg (886);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetPromptDlg (UINT PromptID)
#if ENABLETRACE
{GSSiEnterProg (887);
#endif
{
	SetPrompt2 (0,PromptID);
{
#if ENABLETRACE
GSSiExitProg (887);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}   

BOOL SetErrorProcessing (LPSTR ErrString)
#if ENABLETRACE
{GSSiEnterProg (888);
#endif
{   
	short	l;   
	LPCMDSTRING    pCmdStr;   
	LPSTR	pStr;
	
	if (CurView)
	{   
		if (CurView->FunStackHandle)
		{
			pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
			GSSiGlobFree (&pCmdStr->hError);
			l=_fstrlen (ErrString); 
			if (l)
			{
				pCmdStr->hError = GSSiGlobAlloc ( 644,GMEM_MOVEABLE,l+1);
				pStr = GlobalLock (pCmdStr->hError); 
				_fstrcpy (pStr,ErrString);
				GlobalUnlock (pCmdStr->hError); 
			}
			GlobalUnlock (CurView->FunStackHandle); 
{
#if ENABLETRACE
GSSiExitProg (888);
#endif
			return TRUE;
}
	    }
	}
{
#if ENABLETRACE
GSSiExitProg (888);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}     

BOOL ProcessErrorString (void)  
#if ENABLETRACE
{GSSiEnterProg (889);
#endif
{
	if (CurView)
	{
		if (CurView->FunStackHandle)
		{
			LPCMDSTRING pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
			HANDLE	hError = pCmdStr->hError;
			
			GlobalUnlock (CurView->FunStackHandle); 
		    if (hError)
		    {
		    	LPSTR pErrStr=GlobalLock (hError);  
		    	long	size = GlobalSize (hError);
		    	HANDLE	hNewError = GSSiGlobAlloc (0,GMEM_MOVEABLE,size);
		    	LPSTR	pNewError = GlobalLock (hNewError);
		    	
		    	hmemmove (pNewError,pErrStr,size);
		    	GlobalUnlock (hError);
		    	ProcessText (pNewError); 
		    	GSSiGlobUlFree (&hNewError);
		    }
		}
	}
{
#if ENABLETRACE
GSSiExitProg (889);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
void SetPrompt (UINT PromptID,BOOL SetVP)
#if ENABLETRACE
{GSSiEnterProg (890);
#endif
{   
	LPCMDSTRING    pCmdStr;   

	if (CurView)
	{   
		if (CurView->FunStackHandle && SetVP)
		{
			pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
			if (pCmdStr->Prompt != PRMT_USERPROMPT)   
				pCmdStr->Prompt = PromptID;
			else
				PromptID = PRMT_USERPROMPT;
			GlobalUnlock (CurView->FunStackHandle);
	    }
		SetPrompt2 (CurView->CurrentFunction,PromptID);
	}
	else
		SetPrompt2 (0,PromptID);  
{
#if ENABLETRACE
GSSiExitProg (890);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void RedisplayLastPrompt (void)
#if ENABLETRACE
{GSSiEnterProg (891);
#endif
{
	if (!DoScreenPrompt)
{
#if ENABLETRACE
GSSiExitProg (891);
#endif
		return;
}
	DisplayPromptText (0,LastPromptText);
{
#if ENABLETRACE
GSSiExitProg (891);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL GetLastPrompt (LPSTR lp)
{
	if (!strcmp (lp,LastPromptText))
		return FALSE;
	strcpy (lp,LastPromptText);
	return TRUE;
}

void SetSysMess (LPSTR mess)
{
	if (mess)
	{
		if (!strnicmp (mess,"Click here to ",14))
			mess += 14;
		strcpy (SysMess,mess);
		ExpandText (SysMess);
		SetPrompt2 (0,PRMT_SYSMESS);
	}
	else
		DisplayPromptText (0,0);
	return;
}

void SetPrompt2 (short Function,UINT PromptID)
#if ENABLETRACE
{GSSiEnterProg (892);
#endif
{   
	static	UINT	DebugPrompt=5076;
	short	ii;     
	LPSTR	lphf, lpsc;
	LPSTR	str, txt;
	HANDLE	hStr=0;
	static	char	funstr[40];
	static	short	LastFun=-1, LastPrompt=0;
    HWND	hPar = GetParFocus();
    
    if (Printing || !DoScreenPrompt || (hPar != PromptFocus && hPar != hWndMain))
{
#if ENABLETRACE
GSSiExitProg (892);
#endif
    	return; 
}
    if (!PromptID)  
    {
		DisplayPromptText (0,"");
		LastPrompt = 0;    
{
#if ENABLETRACE
GSSiExitProg (892);
#endif
    	return;  
}
    }
    hStr = GSSiGlobAlloc ( 645,GMEM_MOVEABLE,1024);
    str = GlobalLock (hStr); 
    txt = str + 512;
       
    if (PromptID == DebugPrompt)
    	ii=1;
	if (PromptID == PRMT_SYSMESS)
		strcpy (str,SysMess);
	else if (!LoadString(hInst, PromptID, str, 256))
		*str=0; 
	ExpandText (str);  
	if (*str == '#')
	{   
		_fstrcpy (FullBM,"[%INDIR]help\\legend1.bmp");
		ExpandText (FullBM);
		ShowFullBM(TRUE,0,0);  
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (892);
#endif
		return; 
}
	}
	if ((lpsc = _fstrchr (str,';')))
		*lpsc++ = 0; 
	if ((lphf = _fstrchr (str,'^')))
	{   
		
		*lphf++ = 0; 
		_fstrcpy (CurHelpTopic,lphf);
	}
	if ((PromptID == LastPrompt && (PromptID != PRMT_SYSMESS && PromptID != PRMT_USERPROMPT)) && Function == LastFun) 
	{
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (892);
#endif
		return;		
}
	}
	if (Function && WantFunNames)
	{
		
		if (Function != LastFun)
			GetFunName (Function,funstr);  
		sprintf (txt,"%s: %s",funstr,str);
		DisplayPromptText (0,txt);
	}
	else
		DisplayPromptText (0,str);
	LastFun = Function; 
	LastPrompt = PromptID;
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (892);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

UINT GetMenuPrompt (UINT wParam)
#if ENABLETRACE
{GSSiEnterProg (893);
#endif
{  
	UINT	rtn=0;
	
	if (wParam >= 64000) /* pickmacro*/
	{
		int	item, irec;
		         	
		item = (wParam - 64000) / 256;
		irec = (wParam - 64000) % 256; 
		//         	ProcessPickedItems (hWnd,item, irec);
	
	}
	else if (wParam > 60000) /* Viewports menu used for activate/deactivate */
	{
	
	} 
    else if (wParam >= 58000) /* User commands */
	{  
		int	CmdID;
		         	
		CmdID = wParam - 58000;  
		//ExecuteUserCmd (CmdID);
	}
	else switch (wParam)
	{
		case IDM_VIEW:
			 break;
        case IDM_NEWSYM:
        	 break;
        case IDM_ABOUT: 
        	 rtn = PRMT_ABOUT;
        	 break;
        case IDM_REORG_BTREE:
        	 break;
        case IDM_CITY_EXTRACT:
        	 break;  
        case IDM_SAVE_CONFIG:
        	 break;  
        case IDM_VAN_SMALL: 
             break;
        case IDM_VAN_MEDIUM: 
             break;
        case IDM_VAN_LARGE: 
             break; 
        case IDM_MISC: 
         	 break;
        case IDM_OPEN_DIGITIZER:
        	rtn = PRMT_OPEN_DIGITIZER;
        	break;
        case IDM_CLOSE_DIGITIZER:
        	rtn = PRMT_CLOSE_DIGITIZER;
        	break; 
    }
{
#if ENABLETRACE
GSSiExitProg (893);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}





UINT GetFunStackPrompt(HCURSOR *phCursor)
#if ENABLETRACE
{GSSiEnterProg (897);
#endif
{   
	UINT	Prompt=0;  
	LPCMDSTRING    pCmdStr;
	
    if (CurView->FunStackHandle)
    { 
    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
    	Prompt = pCmdStr->Prompt;  
    	if (phCursor)
    		*phCursor = pCmdStr->hCursor;
    	GlobalUnlock (CurView->FunStackHandle);
    } 
{
#if ENABLETRACE
GSSiExitProg (897);
#endif
	return Prompt;
}
#if ENABLETRACE
}
#endif
}

void SetFunStackPrompt(UINT Prompt)
#if ENABLETRACE
{GSSiEnterProg (898);
#endif
{
	LPCMDSTRING    pCmdStr;
    if (CurView->FunStackHandle)
    { 
    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
    	pCmdStr->Prompt = Prompt;
    	GlobalUnlock (CurView->FunStackHandle);
    } 
{
#if ENABLETRACE
GSSiExitProg (898);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


BOOL ReadObject (HFILE *Fid, BOOL UpdateTarget,LPVOID *RtnAdd,short WantID)
#if ENABLETRACE
{GSSiEnterProg (900);
#endif
{
    short ObjectID,Version,nVars;   
	USHORT	Length;
	long	ii=0, StartLoc, lMem;
    LPVIEWPORT  SaveView;
    HANDLE  handle, hMem;
    LPCOORDINATEDISPLAY CD; 
    LPSTR	pObject, pVName, pVVal, pTemp;
    char    FoundLit=0;
	TAGBOX	TAGBox;
	LPMNMXCORD	pRect; 
	HPDPOINT	pPoint;
    
    if (GSSilread (*Fid,&ObjectID,2) != 2)
		goto RtnFalse;
	if (ObjectID ==28052) //config file signature
		goto RtnFalse;
    if (ObjectID == OB_CONFIGDESCRIPTION || ObjectID == OB_CONFIGIMAGE)
		goto RtnFalse;
    if (ObjectID < 0) 
    	BlowOut ("Invalid object in configuration file",0);
    if (WantID && ObjectID != WantID) 
    {
    	GSSillseek (*Fid,-2,1);  
		goto RtnFalse;
    }
    GSSilread (*Fid,&handle,2);
    GSSilread (*Fid,&Version,2);
    StartLoc = GSSillseek (*Fid,-6,1);
    
    switch (ObjectID)
    {   
    	case OB_SAVESIZEPOS:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			if (Length == sizeof(RECT16))
			{
				RECT16 InitWindowRect16;

				GSSilread (*Fid,&InitWindowRect16,sizeof(RECT16));
				InitWindowRect = Rect16ToRect32 (InitWindowRect16);
			}
			else
				GSSilread (*Fid,&InitWindowRect,sizeof(RECT));
			SaveCfgSizePos = TRUE;
			if (!IsRectEmpty(&InitWindowRect))
			{
				MoveWindow(hWndMain, InitWindowRect.left, InitWindowRect.top,
					InitWindowRect.right - InitWindowRect.left,
					InitWindowRect.bottom - InitWindowRect.top, FALSE);
			}
			goto RtnTrue;
			
    	
    	case OB_SAVEPRINTSETUP:
			ReadPrintSetupData (*Fid);
			goto RtnTrue;
    	
    	case OB_SAVEMENUNAME:
    		hMem = GSSiGlobAlloc ( 646,GMEM_MOVEABLE,256);
			pTemp = GlobalLock (hMem); 
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			GSSilread (*Fid,pTemp,Length);
			if (!IgnoreSavedMenu)
				_fstrcpy (MenuCFGName,pTemp);  
			GSSiGlobUlFree (&hMem);
			goto RtnTrue;
    	case OB_EMBEDMENUS:
    	{   
    		long	Length;
    		
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,4);
			EmbededMenuLoc = GSSillseek (*Fid,0,1);
			GSSillseek (*Fid,Length,1);
			goto RtnTrue;
		}
			
    	case OB_TOOLBARS:
    	{   
    		long	Length;
			RECT OriginalWindowRect = { 0 };

			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,4);
			if (Version > 1)
				GSSilread(*Fid, &OriginalWindowRect, sizeof(RECT));
			LoadToolbarsInConfig (*Fid,OriginalWindowRect);
			goto RtnTrue;
		}  
			
    	case OB_SAVEINFOBOX:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			while (GSSilread(*Fid,&TAGBox,sizeof(TAGBOX)) ==  sizeof(TAGBOX))
			    SaveTAG(0);
			goto RtnTrue;
    		
    	case OB_SAVEMASK:  
    	{
    		short	nParts=0;
    		
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			if (Version < 3)
			{
				USHORT ishort;

				GSSilread (*Fid,&ishort,sizeof(USHORT)); 
				CurView->NumMaskPoints = ishort;
			}
			else
				GSSilread (*Fid,&CurView->NumMaskPoints,sizeof(int));  
			if (Version > 1)
				GSSilread (*Fid,&CurView->NumMaskAreaParts,2); 
			else
				CurView->NumMaskAreaParts = 0;
			if (CurView->NumMaskAreaParts)
				nParts = CurView->NumMaskAreaParts + 1;
			GSSiGlobFree (&CurView->hMaskArea);
		    CurView->hMaskArea = GSSiGlobAlloc ( 647,GMEM_MOVEABLE,sizeof(MNMXCORD)+(long)CurView->NumMaskPoints*sizeof(DPOINT)+nParts*sizeof(int));
		    pRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
			GSSilread (*Fid,pRect,sizeof(MNMXCORD));
			pRect++;
			pPoint = (HPDPOINT) pRect; 
			BigRead (*Fid,(HPSTR)pPoint,(long)CurView->NumMaskPoints*(long)sizeof(DPOINT)+nParts*sizeof(int)); 
			GlobalUnlock (CurView->hMaskArea); 
			if (!CurrentConfig)
				GSSiGlobFree (&CurView->hMaskArea);
			goto RtnTrue;
    	}	
    	case OB_SAVEBACKGROUND:  
    	{
    		short	nParts=0;
    		
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&CurView->numBackgroundAreaPoints,4);  
			GSSilread (*Fid,&CurView->numBackgroundAreaParts,2); 
			if (CurView->numBackgroundAreaParts)
				nParts = CurView->numBackgroundAreaParts + 1;
			GSSiGlobFree (&CurView->hBackgroundArea);
		    CurView->hBackgroundArea = GSSiGlobAlloc ( 647,GMEM_MOVEABLE,(long)CurView->numBackgroundAreaPoints*sizeof(DPOINT)+nParts*sizeof(int));
		    pPoint = (LPDPOINT) GlobalLock (CurView->hBackgroundArea);
			BigRead (*Fid,(HPSTR)pPoint,(long)CurView->numBackgroundAreaPoints*(long)sizeof(DPOINT)+nParts*sizeof(int)); 
			GlobalUnlock (CurView->hBackgroundArea); 
			if (!CurrentConfig)
				GSSiGlobFree (&CurView->hBackgroundArea);
			goto RtnTrue;
    	}	
    	case OB_SAVEGLOBALS:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&nVars,2);
			GSSilread (*Fid,&lMem,4); 
			hMem = GSSiGlobAlloc ( 648,GMEM_MOVEABLE,lMem+1024);
			pVName = GlobalLock (hMem); 
			pTemp = pVName + lMem;
			BigRead (*Fid,pVName,lMem);
			while (nVars--)
			{
				pVVal = _fstrchr (pVName,0);  
				pVVal++;  
				FoundLit = *pVVal++; 
	if (!_fstricmp (pVName,"%NODENAME"))
		ii=0;
				if (_fstricmp (pVName,"%SYM_DICT") && 
					_fstricmp (pVName,"%DATA_LOC") &&
					_fstricmp (pVName,"%DL") &&
					GetVarSaveStatus (pVName) ) 
				{
					if (!_fstrncmp (pVVal,"[%DL]",5))
					{
						_fstrcpy (pTemp,"[%DL]"); 
						ExpandText (pTemp); 
						_fstrcat (pTemp,&pVVal[5]);
						SetGlobalValue3 (pVName,pTemp,0,(BOOL)FoundLit); 
					}
					else if (*pVName != '~')
						SetGlobalValue3 (pVName,pVVal,0,(BOOL)FoundLit);
				}
				pVName = _fstrchr (pVVal,0);
				pVName++;
			}
			GSSiGlobUlFree (&hMem);
			goto RtnTrue;
    		
    	case TR_ATTIMPORT:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
			GSSiGlobFree (&hAttImport);
    		hAttImport = GSSiGlobAlloc ( 649,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hAttImport);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hAttImport);
			goto RtnTrue;
    		
    	case TR_STREET_SEG_FIELDS:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hStreetSegFields = GSSiGlobAlloc ( 650,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hStreetSegFields);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hStreetSegFields);
			goto RtnTrue;
    		
    	case TR_IMPORT_LIMITS:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hImportLimits = GSSiGlobAlloc ( 651,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hImportLimits);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hImportLimits);
			goto RtnTrue;
    		
    	case TR_IMPORT_FILTER:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hImportFilter = GSSiGlobAlloc ( 652,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hImportFilter);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hImportFilter);
			goto RtnTrue;
    		
    	case TR_GRTEXT:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hGRText = GSSiGlobAlloc ( 653,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hGRText);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hGRText);
			goto RtnTrue;
    		
    	case TR_GRCOMMAND:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hGRCommand = GSSiGlobAlloc ( 654,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hGRCommand);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hGRCommand);
			goto RtnTrue;
    		
    	case TR_TIMESTAMP:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hTimeStamp = GSSiGlobAlloc ( 655,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hTimeStamp);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hTimeStamp);
			goto RtnTrue;
    		
    	case TR_IMPORT_REFNO:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hImportRefno = GSSiGlobAlloc ( 656,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hImportRefno);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hImportRefno);
			goto RtnTrue;
    		
    	case TR_IMPORT_SETUP:
			GSSilread (*Fid,&ObjectID,2);
			GSSilread (*Fid,&Version,2);
			GSSilread (*Fid,&Length,2);
			
    		hImportPreSet = GSSiGlobAlloc ( 657,GMEM_MOVEABLE,Length); 
    		pObject = GlobalLock (hImportPreSet);
    		GSSillseek (*Fid,StartLoc,0);
			GSSilread (*Fid,&ObjectID,2);
    		GSSilread (*Fid,pObject,Length);
    		GlobalUnlock (hImportPreSet);
			goto RtnTrue;
    		
    	case OB_HIGHLIGHTLIST:
{
#if ENABLETRACE
GSSiExitProg (900);
#endif
			return (ReadHighlightListFromConfig (Fid));
}
		case GF_NORTH_ARROW_THEME:
			ii=1;
        case GF_SINGLE_NONNUM_VALUE_THEME:
        case GF_CITY_THEME:
        case GF_SINGLE_VALUE_THEME:
        case GF_TWO_VALUE_THEME:
        case GF_MULT_BITMAPS_THEME: 
        case GF_CRIME_THEME:  
        case GF_NETMARKER_THEME: 
        case GF_DOCUMENTS_THEME: 
        case GF_STREET_TEXT_THEME: 
        case GF_STREET_ADDRESS_THEME: 
        case GF_CONTEST_THEME:
		case GF_TRANSFORM_THEME:  
		case GF_HOTSPOT_THEME:
		case GF_BOUNDS_DISPLAY_THEME:
		case GF_DISTANCE_THEME:
		case GF_COORDGRID_THEME: 
		case GF_DYNAMIC_SEG_THEME:
		case GF_PROFILE_THEME:
		case GF_PROFILE_LINK_THEME:
		case GF_2D_THEME:   
		case GF_COMPARE_VIEWPORTS_THEME:
		case GF_GRAPHICS_FUNCTION_THEME: 
		case GF_POLYINFO_THEME:
        case GF_CACHE_DISPLAY_THEME:     
        case GF_TIME_DISPLAY_THEME: 
        case GF_POINT_IN_AREA_THEME:
		case GF_OFFSETAREA_THEME:
		case GF_AREA_IN_MASK_THEME:
		case GF_CONNECTION_LINE_THEME:
    
            handle=GSSiGlobAlloc ( 658,GHND,sizeof(THEME));
            CurTheme=(LPTHEME)GlobalLock(handle); 
            switch (Version)
            {   
            	case 0:
            	{
            		HANDLE	handle_v0 = GSSiGlobAlloc ( 659,GHND,sizeof(THEME_v0));
            		LPTHEME_v0	pTheme_v0=(LPTHEME_v0)GlobalLock (handle_v0);
            		GSSilread (*Fid,pTheme_v0,sizeof(THEME_v0)); 
            		ConvertThemeV0toV1 (CurTheme,pTheme_v0);
            		GSSiGlobUlFree (&handle_v0); 
            	}
            		break;
            	case 1:
            	{
            		HANDLE	handle_v1 = GSSiGlobAlloc ( 659,GHND,sizeof(THEME_V1));
            		LPTHEME_V1	pTheme_v1=(LPTHEME_V1)GlobalLock (handle_v1);
            		GSSilread (*Fid,pTheme_v1,sizeof(THEME_V1)); 
            		ConvertThemeV1toV2 (CurTheme,pTheme_v1);
            		GSSiGlobUlFree (&handle_v1); 
            	}
            		break;
				case 2:
				case 3:
            	default:
            		GSSilread (*Fid,CurTheme,sizeof(THEME)); 
            		break;
            }
            CurTheme->handle = handle; 
            CurTheme->Config = CurrentConfig;   
			CurTheme->CompareDC = 0;
            CurTheme->hDelayedFont = 0;
			CurTheme->nLabelLines = 0;
			CurTheme->hhLabelLines = 0;
            CurTheme->FidDelayedText = HFILE_ERROR;
			CurTheme->hDisperseFileName = 0; 
			CurTheme->hDisperseFile = 0;
			CurTheme->hHighlightFileName = 0; 
			CurTheme->hHighlightFile = 0;
            CurTheme->hThemeDB = 0;
            CurTheme->ScatterFile[0]=0; 
            CurTheme->hScatterFile = 0;
            CurTheme->FidAreas = HFILE_ERROR;
            CurTheme->HotSpotData.hGrid = 0;  
            CurTheme->HotSpotData.hMask = 0;  
            CurTheme->HotSpotData.hTranBaseToHotSpot = 0; 
            CurTheme->hHotSpotBitmap = 0;  
            CurTheme->PointInAreaBrush = 0;
            CurTheme->PointInAreaPen = 0; 
            CurTheme->Pass = 0;   
			CurTheme->hVisList = CurTheme->hVisList2 = 0;
			CurTheme->NoDataBrush = CurTheme->InvalidDataBrush = 0;
            CurTheme->NumAreas;
            _fmemset (CurTheme->ClassBrush,0,sizeof(CurTheme->ClassBrush));
            _fmemset (CurTheme->ClassPen,0,sizeof(CurTheme->ClassPen));
            if (UpdateTarget)
            {
                SaveView = CurView;
				CurTheme->DisplayViewport = CurView->ID;
                SetCurView ( pViewports[CurTheme->TargetViewport-1]); 
				CurTheme->TargetViewport = CurView->ID;
                if (CurView)
					AddThemeToVP (CurView,CurTheme);
                if (CurTheme->ID == GF_TRANSFORM_THEME)
                {
	                SetCurView ( pViewports[CurTheme->ReScan-1]); 
	                if (CurView)
						AddThemeToVP (CurView,CurTheme);
	            }
                SetCurView (SaveView); 
            }
            
            if (CurTheme->ID == GF_DISTANCE_THEME)
            {  
            	char	Value[32]; 
            	short	i;
            	
 				GetGlobalCVal ("[%SCALEBARUNITS]",Value,0);
 				if (*Value)
 				{
 					short	n=atoi (Value);
					if (!n)
					for (i = 6;i;i--)
						if (!_fstrnicmp (Value,DistUnitOpts[i-1],_fstrlen(Value)))
						{
							n = i;
							break;           
						}
        			CurTheme->ValConv = n; 
		        }
            }
			if (!CurTheme->ExpressionConverted && CurTheme->ID == GF_SINGLE_VALUE_THEME)
			{
				int	ff=CurTheme->FieldFun;
				CurTheme->ExpressionConverted = TRUE;
				CurTheme->FieldFun = 0;
				switch (ff)
				{
				case 6:
					CurTheme->FieldFun = 1;
					break;
				case 1:
					break;
				}
			}
            if (CurTheme->ID == GF_SINGLE_VALUE_THEME || CurTheme->ID == GF_TWO_VALUE_THEME)
            { 
                if (CurTheme->Recompute)
                {
                    CurTheme->WantDataPass = TRUE;
                    if (CurTheme->ClassType != 3)
                    	CurTheme->ComputeClassBoundaries = TRUE;                    
                }
            }
            if (CurTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME)
            {
				LPSTR pRecs;
                HANDLE  hRecs;
                short     len; 
				int		ClassNo;
                long    NumRecs;
                LPSTR   lpVal;  
                char	Key[210];
				short	lenrecread;
				int		ClassLen=2;
                
	        	CurTheme->Recompute = FALSE;
	            CurTheme->WantDataPass = FALSE;
                NumRecs = 0;                    
                ii=GSSilread(*Fid,&NumRecs,4);
				CurTheme->numPreloadedValues = NumRecs;
                if (Version >3)
					ClassLen = 4;
                if (Version >2)
					ii=GSSilread(*Fid,&lenrecread,2);
              // if (NumRecs || CurTheme->AutoClassDef)
                {
                    CurTheme->hScatterFile = CreateClassValueList(CurTheme->hScatterFile);
					if (CurTheme->FieldFun && Version < 3) 
					{           // temp code 12/11/99
						len =  GetGlobalLVal2 ("[%EXLEN]",128); 
						if (len < 128)
							SetGlobalValueLong ("%EXLEN",128);
					}
					else 
						len = GetBTKeyLen(CurTheme->hScatterFile);    
//                  		len = GetValListFieldLen (&CurTheme->Field[0]); 
                  	len = min (len,200);
					if (Version < 3)
						lenrecread = len;
					if (NumRecs > 0)
					{
						hRecs = GSSiGlobAlloc (1759,GMEM_MOVEABLE,NumRecs*(ClassLen+lenrecread));
						pRecs = GlobalLock(hRecs);  
						BigRead (*Fid,pRecs,NumRecs*(ClassLen+lenrecread));
						while (NumRecs--)
						{
							long	ClassNo;

							lpVal = pRecs; 
							pRecs += lenrecread;
							if (ClassLen == 2)
							{
								LPSHORT lpint = (LPSHORT) pRecs;
								ClassNo = *lpint++;
								pRecs = (LPSTR) lpint;
							}
							else
							{
								LPINT lpint = (LPINT) pRecs;
								ClassNo = *lpint++;
								pRecs = (LPSTR) lpint;
							}
							lpVal[lenrecread]=0;
							strncpy (Key,lpVal,len);
							BT_PUT(CurTheme->hScatterFile,(LPSTR)Key,(LPSTR)&ClassNo);
						}
						GSSiGlobUlFree (&hRecs);
					}
					BT_CLOSE(CurTheme->hScatterFile);
					CurTheme->hScatterFile = 0;
				}
            }
            if (CurTheme->ID == GF_STREET_TEXT_THEME) 
            {
			    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
				pStreetData->hNameFile1 = pStreetData->hNameFile2 = 0;
				*pStreetData->NameFile1 = *pStreetData->NameFile2 = 0;
           }
            if (CurTheme->ID == GF_DOCUMENTS_THEME)
            {
                short     NumDocs;
                LPSHORT   pNumDocs;
                LPSTR   pName; 
                
                GSSilread(*Fid,&NumDocs,2);
   				GSSiGlobFree (&hDesiredDocs);

                if (NumDocs)
                {                             
                    hDesiredDocs = GSSiGlobAlloc ( 660,GHND,sizeof(short)+14*NumDocs);
                    pNumDocs = (LPSHORT)GlobalLock (hDesiredDocs);
                    *pNumDocs = NumDocs;  
                    pNumDocs++;
                    pName = (char *)pNumDocs; 
                    GSSilread (*Fid,pName,NumDocs*14);
                    GlobalUnlock (hDesiredDocs);
                }
            } 
            
            *RtnAdd = CurTheme;
			goto RtnTrue;
            break;
            
        case PF_COORD_DISPLAY:
        
            handle=GSSiGlobAlloc ( 661,GHND,sizeof(THEME));
            CD=(LPCOORDINATEDISPLAY)GlobalLock(handle);
            switch (Version)
			{
			case 101:
            	ConvertCDV101To102 (*Fid,CD);
				break;
			case 102:
            	ConvertCDV102To103 (*Fid,CD);
				break;
			case 103:
			{
				COORDINATEDISPLAY_V103 CD103;

            	GSSilread (*Fid,&CD103,sizeof(COORDINATEDISPLAY_V103)); 
				ConvertCD_V103_to_V104 (CD,&CD103);
			}
			break;
			default:
            	GSSilread (*Fid,CD,sizeof(COORDINATEDISPLAY));
			//	GSSillseek (*Fid,-2,1);
			}
            CD->handle = handle;  
            CD->hSurf = 0;
            AddPassiveFun (CD);
            
            *RtnAdd = CD;
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (900);
#endif
            return TRUE;
}
            break;
    } 
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (900);
#endif
    return FALSE;
}

#if ENABLETRACE
}
#endif
}

void CloseObject (LPVOID pObject)
#if ENABLETRACE
{GSSiEnterProg (901);
#endif
{   OFSTRUCTGM    OFStruct;
    HANDLE  handle;
    
    CurTheme = pObject;
    if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (901);
#endif
    	return;
}
    switch (CurTheme->ID)
    {
        case GF_SINGLE_VALUE_THEME:
        case GF_SINGLE_NONNUM_VALUE_THEME:
        case GF_CITY_THEME:
        case GF_TWO_VALUE_THEME:
        case GF_MULT_BITMAPS_THEME:  
        case GF_CRIME_THEME: 
        case GF_NETMARKER_THEME:  
        case GF_DOCUMENTS_THEME:  
        case GF_CONTEST_THEME:
        case GF_TRANSFORM_THEME: 
		case GF_BOUNDS_DISPLAY_THEME:
		case GF_DISTANCE_THEME:
		case GF_COORDGRID_THEME:
		case GF_DYNAMIC_SEG_THEME:
		case GF_PROFILE_THEME:
		case GF_PROFILE_LINK_THEME:
		case GF_2D_THEME:   
		case GF_COMPARE_VIEWPORTS_THEME:
		case GF_GRAPHICS_FUNCTION_THEME:   
		case GF_TIME_DISPLAY_THEME:  
		case GF_POINT_IN_AREA_THEME:
		case GF_NORTH_ARROW_THEME:
		case GF_OFFSETAREA_THEME:
		case GF_AREA_IN_MASK_THEME:
		case GF_CONNECTION_LINE_THEME:
            
			ClearCompareDC (CurTheme,TRUE);
		  	if (CurTheme->FidDelayedText > 0)
		  		CloseAndDeleteFile (&CurTheme->FidDelayedText); 
			if (CurView)
				SelectObject (CurView->hDC,GetStockObject(SYSTEM_FONT)); 
		  	GSSiDeleteObject (&CurTheme->hDelayedFont);
			GSSiDeleteObject (&CurTheme->PointInAreaBrush);
			GSSiDeleteObject (&CurTheme->PointInAreaPen);
		    CloseThemeDataFile (TRUE);
			if (CurTheme->ID == GF_OFFSETAREA_THEME)
			{
				GSSiClose2 (&CurTheme->FidAreas);
				CurTheme->FidAreas = HFILE_ERROR;
				GSSiRemoveAndClear (CurTheme->ScatterFile); 
			}
			if (CurTheme->ID == GF_POINT_IN_AREA_THEME)
			{
				GSSiClose2 (&CurTheme->FidAreas);
				CurTheme->FidAreas = HFILE_ERROR;
				GSSiRemoveAndClear (CurTheme->ScatterFile); 
			    if (ExistFile (CurTheme->DataFile))
			    {   
			    	char	IndexFile[256];
			    	LPSTR pDot;
			    	
			    	_fstrcpy (IndexFile,CurTheme->DataFile);
		    		GSSiRemove (CurTheme->DataFile);
			    	pDot = _fstrrchr (IndexFile,'.');
			    	if (pDot)
			    	{
			    		_fstrcpy (pDot,".in1");
			    		GSSiRemove (IndexFile); 
			    		pDot = _fstrrchr (IndexFile,'\\');
			    		if (pDot)
			    		{
			    			*pDot = 0; 
			    			GSSiRemoveDir (IndexFile);
			    		}
			    	}
			    }
			}
			else
			{
	            if (CurTheme->hScatterFile)
					BT_CLOSEANDDELETE (&CurTheme->hScatterFile);
				else if (*CurTheme->ScatterFile)
					GSSiRemoveAndClear (CurTheme->ScatterFile); 
			}
			CurTheme->ScatterFile[0] = 0;
			DeleteThemeHighlightFile ();
			DeletePointDispersionFile ();
	FreeTheme: 
			DestroyThemePens (CurTheme);
			handle = CurTheme->handle;
			GSSiGlobUlFree (&handle); 
			CurTheme = 0;
			break;  
    	case PF_COORD_DISPLAY: 
			{
				LPCOORDINATEDISPLAY pCD=(LPCOORDINATEDISPLAY)CurTheme;

				DestroyThemePens (CurTheme);
				handle = pCD->handle;
				GSSiGlobUlFree (&handle); 
				CurTheme = 0;
			}
			break;
		case GF_STREET_TEXT_THEME:
	    	GSSiGlobFree (&CurTheme->hScatterFile);
            {
			    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
				GSSiRemoveAndClear (pStreetData->NameFile2);
            }
			DeleteThemeHighlightFile ();
            
        case GF_STREET_ADDRESS_THEME: 
	    	GSSiGlobFree (&CurTheme->hScatterFile);
			
    	case GF_POLYINFO_THEME:
        case GF_CACHE_DISPLAY_THEME:
	    	goto FreeTheme; 
	    
        case GF_HOTSPOT_THEME:
	    	GSSiGlobFree (&CurTheme->HotSpotData.hGrid);
			GSSiGlobFree (&CurTheme->HotSpotData.hMask);
			CloseTRANS2 (&CurTheme->HotSpotData.hTranBaseToHotSpot); 
			GSSiDeleteObject (&CurTheme->hHotSpotBitmap);
		    CloseThemeDataFile (TRUE);
	    	goto FreeTheme; 

    } 
{
#if ENABLETRACE
GSSiExitProg (901);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
BOOL AddSymToMap (short NumSyms,HANDLE hSymDesc,short NumPens,LPPENDESC pPenDesc)
#if ENABLETRACE
{GSSiEnterProg (902);
#endif
{   
    LPSYMDESC pSymDesc;
    char    Name[256];  
    mnmxCor     MinMax;  
    long    NewPrimeOffset, MemLen,ii;
    HANDLE  hMem;
    LPSHORT   pInt;  
    short	nbytes;
    HANDLE	hSpace=0, hPar=0, hSym=0;    
    short	nPar=0, nSym=0, NumNew=0;
    LPSTR	Space;
    short     	lMem, Signature, Version, i2, i, n, NumParent=0, ndesc,idesc;  
    LPSHORT		ipnt;
    short SymNameLen=32, iparent;
    PRIMEOFFSETS NewPrimeOffs;
    LPSYMBOL    pSymbol;
    BOOL	parent=FALSE, Opened;   
    char	Drive[4], Dir[256],name[34],Ext[8], TempName[256], DescName[34]; 
typedef    struct	{short	desc,parent;
    		 char	Name[32];} SYMENTRY;
typedef	SYMENTRY	FAR	*LPSYMENTRY;
	LPSYMENTRY	pPar, pSym, pEnt;
	
	if (!NumSyms)
{
#if ENABLETRACE
GSSiExitProg (902);
#endif
		return TRUE;
}
    if (FidMap != HFILE_ERROR)
    	Opened = FALSE;
    else  
	{   
		Opened = TRUE;		
		if (PltType == 4)
		{ 
			return FALSE;
		}
		else
		{ 
	    	if (!OpenMap (0,0))
{
#if ENABLETRACE
GSSiExitProg (902);
#endif
    			return FALSE;
}
    	}
    }
   	if (MapVersion < 8)
{
#if ENABLETRACE
GSSiExitProg (902);
#endif
   		return FALSE; 
}
   	ii=GSSillseek (FidMap,-18,2);
   	BigRead (FidMap,(char *)&NewPrimeOffset,4);
    BigRead (FidMap,(char *)&MinMax,8);
    BigRead (FidMap,(char *)&Signature,2);
    BigRead (FidMap,(char *)&Version,2);
   	GSSillseek (FidMap,NewPrimeOffset+2,0); 
   	BigRead (FidMap,(HPSTR)&NewPrimeOffs,sizeof(NewPrimeOffs));
	if (NumSyms < 0)
	{
		NumSyms = -NumSyms;
	    NewPrimeOffs.UsedDescOffset = GSSillseek (FidMap,-18,2); 
		WriteSymList (FidMap, NumParent, NumSyms,hSymDesc);
		NewPrimeOffset = GSSillseek (FidMap,0,1);    
		i2 = sizeof(NewPrimeOffs);
		BigWrite (FidMap,(HPSTR)&i2,2,-1);
		BigWrite (FidMap,(HPSTR)&NewPrimeOffs,sizeof(NewPrimeOffs),-1);
		BigWrite (FidMap,(HPSTR)&NewPrimeOffset,4,-1);
		BigWrite (FidMap,(HPSTR)&MinMax,8,-1);
		Signature = 32349;       
		BigWrite (FidMap,(HPSTR)&Signature,2,-1);
		Version = 8;            
		BigWrite (FidMap,(HPSTR)&Version,2,-1);
		BigWrite (FidMap,(HPSTR)&Version,2,-1); 
	}
	else
	{
		GSSillseek(FidMap,NewPrimeOffs.UsedDescOffset,0);
		BigRead (FidMap,(HPSTR)&nbytes,2);
		hSpace = GSSiGlobAlloc ( 662,GMEM_MOVEABLE,(DWORD)nbytes);
		Space = GlobalLock (hSpace);
		ipnt = (LPSHORT)Space;
		BigRead (FidMap,Space,(WORD)nbytes);
    
		MemLen = USHRT_MAX;
		hSym = GSSiGlobAlloc ( 663,GMEM_MOVEABLE,MemLen);  
		pSym = (LPSYMENTRY)GlobalLock (hSym);
		hPar = GSSiGlobAlloc ( 664,GMEM_MOVEABLE,MemLen);
		pPar = (LPSYMENTRY)GlobalLock (hPar);   
		pEnt = pSym;
		ipnt++;                
	moredesc:
		ndesc = *ipnt;
		ipnt++;
		for (i=0;i<ndesc;i++)
		{	
			idesc=abs (*ipnt);
			ipnt++;
			iparent=*ipnt;
			ipnt++;
    		_fmemmove (DescName,ipnt,SymNameLen); 
    		DescName[SymNameLen]=0;
    		Truncate (DescName);
    		if (!_fstricmp(DescName,"ALL"))
    			iparent=0;  
    		if (parent)
    			nPar++;
    		else
    			nSym++;
			pEnt->desc=idesc;
			pEnt->parent=iparent;
			_fstrncpy (pEnt++->Name,DescName,32);
    		ipnt+=(SymNameLen/2);
		}
		if (!parent)
			pEnt = pPar;
		parent = TRUE;  
		if (ndesc)
			goto moredesc;
		GSSiGlobUlFree (&hSpace);
			
		n = NumSyms; 
		NumParent = 0;
		pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
		while (n--)
		{   
			if (!pSymDesc->Handle)
				pSymDesc->Handle = GetDictSymDesc (pSymDesc->Number,1); 
			pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle);
			if (!pSymbol->Type) 
			{   
        		for (i=0,pEnt=pPar;i<nPar;i++,pEnt++)
        			if (!_fstricmp(pEnt->Name,pSymbol->Name))
        				goto SkipAdd;
        		pEnt = pPar + nPar;
				nPar++;
				NumNew++;            
			}
			else
			{
        		for (i=0,pEnt=pSym;i<nSym;i++,pEnt++)
        			if (!_fstricmp(pEnt->Name,pSymbol->Name))
        				goto SkipAdd;
        		pEnt = pSym + nSym;
				nSym++; 
				NumNew++;           
			}
			pEnt->desc=pSymDesc->Number;
			pEnt->parent=pSymbol->Parent;
			_fstrncpy (pEnt++->Name,pSymbol->Name,32);
	SkipAdd:
			GlobalUnlock (pSymDesc->Handle);
    		DestroySymbol (pSymDesc->Handle); 
    		pSymDesc++->Handle = 0;
		}
		GlobalUnlock (hSymDesc);
    
		if (NumNew)
		{
			NewPrimeOffs.UsedDescOffset = GSSillseek (FidMap,-18,2); 
			lMem = (1+nSym+nPar)*36+2+2+2+2+2;  
			BigWrite (FidMap,(HPSTR)&lMem,2,-1);
			i2 = 11; 
			BigWrite (FidMap,(HPSTR)&i2,2,-1);
			BigWrite (FidMap,(HPSTR)&nSym,2,-1);  
			pEnt = pSym;
			n=nSym;
			while (n--)
				BigWrite (FidMap,(HPSTR)pEnt++,sizeof(SYMENTRY),-1);
			pEnt = pPar;
			BigWrite (FidMap,(HPSTR)&nPar,2,-1);
			n=nPar;
			while (n--)
				BigWrite (FidMap,(HPSTR)pEnt++,sizeof(SYMENTRY),-1);
			i2 = 0;  
			BigWrite (FidMap,(HPSTR)&i2,2,-1); 
			BigWrite (FidMap,(HPSTR)&i2,2,-1); 
			
			NewPrimeOffset = GSSillseek (FidMap,0,1);    
			i2 = sizeof(NewPrimeOffs);
			BigWrite (FidMap,(HPSTR)&i2,2,-1);
			if (NewPrimeOffs.Code202 != 202)
			{
				NewPrimeOffs.Code202 = 202;                 
				NewPrimeOffs.MinTime=0;
				NewPrimeOffs.MaxTime=LONG_MAX; 
			}
			BigWrite (FidMap,(HPSTR)&NewPrimeOffs,sizeof(NewPrimeOffs),-1);
			BigWrite (FidMap,(HPSTR)&NewPrimeOffset,4,-1);
			BigWrite (FidMap,(HPSTR)&MinMax,8,-1);
			Signature = 32349;       
			BigWrite (FidMap,(HPSTR)&Signature,2,-1);
			Version = 8;            
			BigWrite (FidMap,(HPSTR)&Version,2,-1);
			BigWrite (FidMap,(HPSTR)&Version,2,-1); 
			if (Opened)
	    		CloseMap (FALSE);
			Opened = FALSE;
			LastQuadOff=-2; //need to be set when anything but GetFileConnectOffset writes to file
		}
	}
Exit:
	if (Opened)
		CloseMap (FALSE);
    GSSiGlobUlFree (&hSym);
    GSSiGlobUlFree (&hPar);
{
#if ENABLETRACE
GSSiExitProg (902);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

short GetBaseRecordType (short Type)
#if ENABLETRACE
{GSSiEnterProg (903);
#endif
{
	switch (Type)
	{
		case 1: 
		case GF_POINT:
		case GF_LINE:
		case GF_CURVE:
		case GF_POLYLINE:
		case 2:      
		case 4:
		case 5:
			Type = 1;
			break;
		case 3:       
		case GF_AREA:
			Type = 0;    
			break;
		case GF_TEXT:
			Type = 2;
			break;
	}
{
#if ENABLETRACE
GSSiExitProg (903);
#endif
	return Type;
}
#if ENABLETRACE
}
#endif
}

BOOL TypeIsPolyline(short Type)
{
	BOOL rtn = FALSE;
	switch (Type)
	{
	case GF_LINE:
	case GF_CURVE:
	case GF_POLYLINE:
	case 2:
	case 4:
	case 5:
		rtn = TRUE;
		break;
	}

	return rtn;
}

BOOL TypeIsPolygon(short Type)
{
	BOOL rtn = FALSE;
	switch (Type)
	{
	case 3:
	case GF_AREA:
		Type = 3;
		rtn = TRUE;
		break;
	}

	return rtn;
}


short GetHighlightType (short Type)
#if ENABLETRACE
{GSSiEnterProg (903);
#endif
{
	switch (Type)
	{    
		case 1:
		case GF_POINT:
			Type = 1;
			break; 
		default: 
		case 2:
		case GF_LINE:
		case GF_POLYLINE:
			Type = 2;
			break; 
		case 3:
		case GF_AREA:
			Type = 3;     
			break;
		case 4:
		case GF_TEXT:
			Type = 4;
			break;
		case 5:
		case GF_CURVE:
			Type = 5;
			break;
		case 6:
		case GF_DELETE:
			Type = 6;
			break;
	}
{
#if ENABLETRACE
GSSiExitProg (903);
#endif
	return Type;
}
#if ENABLETRACE
}
#endif
}

BOOL UpdateRecord (int Item,int NewDesc,LPSTR NewPrefix, LPSTR NewUDI,LPLINEDESCRIPTION pNewLineDesc, LPAREADESCRIPTION pNewAreaDesc,int WhichFile,int OutItem)
#if ENABLETRACE
{GSSiEnterProg (904);
#endif
{   
//Which file = 0 if same as picked, 1 if EditFile, 2 if delete from existing and copy to edit file
    HANDLE	hBuf, hRec;
    HPSTR	pBuf; 
    BOOL	First, SaveUROT;
	LPSTR	Rec;
	long	RecLen, loc, Offset, item_len,ii;
	short	Type, id, UpFile;
	mnmxCor		MinMax;  
	LPITEM	pRecHeader;
	LPITEM	ItemHeader;  
    LPVIEWPORT	SaveView = CurView;  
    PICKDATA   PickSave;
	static	long	ndel=0;   
	static	long	debugref=1000057;

	
    if (!GetPickName (Item))  
    {
		UpdateItem = 0;
{
#if ENABLETRACE
GSSiExitProg (904);
#endif
        return FALSE;  
}
    }
    PickSave = PickList[Item];
	SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID);
    Type = GetBaseRecordType (PickList[Item].Type); 
    _fstrcpy (PltName,PickName);
	hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
	lUpdateBuf = 0;  
	KeepTranFileToBase = (HANDLE)1; 
	if (PickList[Item].Refno == debugref)
		ii=1;
	if (PickList[Item].Type != 2) //dummy delete or deleted item      
    	ii=1;
	if (PickList[Item].IsDeleted) //dummy delete or deleted item      
	{
		ndel++;
		UpdateItem = 24;
	}
	CopyRec = TRUE;   
	HaveDummyDelete = FALSE;
	ProcessPickedItem (Item,FALSE); 
	CopyRec = FALSE; 
	if (!lUpdateBuf)
		ii=1;  
  
/*	if (UpdateItem == 24)
	{   
		short	id=24;
		
		if (HaveDummyDelete)
		{   
		    pBuf = (HPSTR)GlobalLock (hUpdateBuf); 
		    pRecHeader = (LPITEM)pBuf; 
		    pRecHeader->Marker = 12; 
			GlobalUnlock (hUpdateBuf);			
		}  
		else
		{
			UpdateItem = 0;
		    GSSiGlobFree (&hUpdateBuf);
			CloseTRANS2 (&KeepTranFileToBase);
		    SetCurView ( SaveView); 
		    return TRUE;
		}
	} */ 
	if (UpdateItem == 24)
	{   
		short	id=24;
		
	    pBuf = (HPSTR)GlobalLock (hUpdateBuf); 
	    pRecHeader = (LPITEM)pBuf; 
	    pRecHeader->Marker = 12; 
		if (!HaveDummyDelete)
		{
			pBuf += lUpdateBuf;   
			BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&id,2);
		}
		GlobalUnlock (hUpdateBuf);			
	}  
	UpdateItem = 0;

	
/*	if (pNewAreaDesc)
	{
		id = 22;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&pNewAreaDesc->Pattern,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&pNewAreaDesc->ForeColor,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)&pNewAreaDesc->BackColor,4);
	}
	if (pNewLineDesc)
	{
		if (HiPrecis)
			id = 121;
		else
			id = 21;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&pNewLineDesc->Color,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)&pNewLineDesc->Width,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&pNewLineDesc->Style,2);
	}*/
	
	UpFile = PickList[Item].FileNum; 
	if (UpFile == CurView->UpdateFile - 1)
		WhichFile = 0;
   	FileInIndex=0;
    if (!WhichFile)
		DeletePickedItem (Item,12,92);
	else if (CurView->UpdateFile)
	{   
		if (hChronoIndex)
		{ 
			if (!GetChronoIndexedEditFile (PltName,GRStartTime, GREndTime))
				goto RtnFalse;
			PltType = 2; 
		}
		else
		{
		   	_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
		   	PltType = CurView->FileType[CurView->UpdateFile-1]; 
		}
	   	UpFile = CurView->UpdateFile-1; 
	}
	else  
		goto RtnFalse;
	if (!OpenMap (CurView->hWnd,0))
	{   
RtnFalse:
	    ForceRefIndex = ForceTAGIndex = FALSE; 
	    GSSiGlobFree (&hUpdateBuf); 
		if (KeepTranFileToBase == (HANDLE)1)
			KeepTranFileToBase = 0;
		else
			CloseTRANS2 (&KeepTranFileToBase);
	    UpdateItem = 0; 
	    SetCurView ( SaveView); 
		if (hChronoIndex)
			GSSiMsgBox (0,"No file found in chronological index for specified time",PltName,MB_ICONEXCLAMATION,0);
		else
			GSSiMsgBox (0,"Unable to open update file for write access",PltName,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (904);
#endif
		return(FALSE); 
}
	}  
    ForceRefIndex = ForceTAGIndex = TRUE;  
    SaveUROT = UseRefOrTAGIndex;
    UseRefOrTAGIndex = FALSE;
	//if (!hChronoIndex)
	{
   		_fstrcpy (PltName,CurView->lpFiles[UpFile]);
   		PltType = CurView->FileType[UpFile]; 
	}
    OpenRefIndex (FALSE);
    pBuf = (HPSTR)GlobalLock (hUpdateBuf); 
    pRecHeader = (LPITEM)pBuf;  
    item_len = lUpdateBuf/2 - 6;
	if (item_len > 16000)
		item_len=0;
	else
		item_len = -item_len;
    pRecHeader->Len = item_len; 
    pBuf += lUpdateBuf;
	id=13;
	BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&id,2);
	Offset = -1;
	BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&Offset,4);
	id = 0;
	BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&id,2);  
	GlobalUnlock (hUpdateBuf);
    pBuf = GlobalLock (hUpdateBuf); 
    hTranCopyFile = KeepTranFileToBase; 
	RecomputeMinMax (&MinMax,pBuf,lUpdateBuf,0,0); 
	hTranCopyFile = 0;
	if (KeepTranFileToBase == (HANDLE)1)
		KeepTranFileToBase = 0;
	else
		CloseTRANS2 (&KeepTranFileToBase);
	ItemHeader = (LPITEM)pBuf; 
    ItemHeader->MinMax = MinMax;
    First = TRUE;  
    hRec = 0; 
	hBuf = hUpdateBuf;
	hUpdateBuf = 0;
    while (SplitRec (&(HPSHORT)pBuf,&lUpdateBuf,&(LPSHORT)Rec,&RecLen,&hBuf,&hRec,TRUE))
    {
		loc = GetFileConnectOffset(Type,NewDesc,MinMax,(short)RecLen,-1,Rec); 
		if (First)
		{
			First = FALSE;
			CurrentItem = loc - CurrentSeg -2;
			ItemSeg = CurrentSeg;
		}    
	    GSSiGlobUlFree (&hBuf);
	}
   	CurrentRefno=PickList[Item].Refno;
   	BuildRefIndex(TRUE,FALSE);
   	if (NewPrefix && *NewPrefix)	
		BuildTAGIndex (NewPrefix,NewUDI,0,CurrentRefno,FALSE);
	CloseMap (TRUE); 
	{
		HANDLE	hSymDesc=0;
		short	NumSyms=0;  
		AddToSymList (NewDesc,&NumSyms,&hSymDesc); 
		AddSymToMap (NumSyms,hSymDesc,0,0); 
        DestroySymList (&NumSyms,&hSymDesc);
    }
    ForceRefIndex = ForceTAGIndex = FALSE;    
    UseRefOrTAGIndex = SaveUROT;

    UpdateItem = 0;
    PickList[Item] = PickSave;   
 	if (WhichFile == 2)
		DeletePickedItem (Item,12,92); 
    SetCurView ( SaveView);  
    if (OutItem > -1)
    {
    	PickList[OutItem]=PickSave; 
    	PickList[OutItem].FileNum = UpFile;
    	PickList[OutItem].Segment = ItemSeg;
		PickList[OutItem].Offset = CurrentItem;
    }
{
#if ENABLETRACE
GSSiExitProg (904);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL UpdateRecordCopy (int Option)
#if ENABLETRACE
{GSSiEnterProg (904);
#endif
{   
// Option (1 = open and process record, 2 = close, 0=process record)
    HANDLE	hBuf, hRec;
    HPSTR	pBuf; 
    static	BOOL	SaveUROT;
	BOOL	First;
	LPSTR	Rec;
	long	RecLen, loc, Offset, item_len,ii;
	short	Type, id;
	static	short	UpFile;
	mnmxCor		MinMax;  
	LPITEM	pRecHeader;
	LPITEM	ItemHeader;  
    LPVIEWPORT	SaveView = CurView;  
    PICKDATA   PickSave;
	static	long	ndel=0;   
	static	long	debugref=1000057;
	static	HANDLE	hSymDesc=0;
	static	short	NumSyms=0;  
	BOOL	Deleted;
	int		ref;

	
	if (!lUpdateBuf)
		ii=1;  
  
	UpdateItem = 0;
	
   	switch (Option)
	{
	case 1:
	{
		KeepTranFileToBase = (HANDLE)1; 
		hSymDesc=0;
		NumSyms=0;
		FileInIndex=0;
		if (!CurView->UpdateFile)
			goto RtnFalse;
		_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
		PltType = CurView->FileType[CurView->UpdateFile-1]; 
		UpFile = CurView->UpdateFile-1; 
		ForceRefIndex = ForceTAGIndex = TRUE; 
		if (!OpenMap (CurView->hWnd,0))
		{   
	RtnFalse:
			ForceRefIndex = ForceTAGIndex = FALSE; 
			GSSiGlobFree (&hUpdateBuf); 
			if (KeepTranFileToBase == (HANDLE)1)
				KeepTranFileToBase = 0;
			else
				CloseTRANS2 (&KeepTranFileToBase);
			UpdateItem = 0; 
			SetCurView ( SaveView); 
			if (hChronoIndex)
				GSSiMsgBox (0,"No file found in chronological index for specified time",PltName,MB_ICONEXCLAMATION,0);
			else
				GSSiMsgBox (0,"Unable to open update file for write access",PltName,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (904);
#endif
			return(FALSE); 
}
		} 
	}
	case 0:
		ForceRefIndex = ForceTAGIndex = TRUE;  
		SaveUROT = UseRefOrTAGIndex;
		UseRefOrTAGIndex = FALSE;
		_fstrcpy (PltName,CurView->lpFiles[UpFile]);
		PltType = CurView->FileType[UpFile]; 
	//    OpenRefIndex (FALSE);
		pBuf = (HPSTR)GlobalLock (hUpdateBuf); 
		pRecHeader = (LPITEM)pBuf;  
		item_len = lUpdateBuf/2 - 6;
		if (item_len > 16000)
			item_len=0;
		else
			item_len = -item_len;
		pRecHeader->Len = item_len; 
		pBuf += lUpdateBuf;
		id=13;
		BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&id,2);
		Offset = -1;
		BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&Offset,4);
		id = 0;
		BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&id,2);  
		GlobalUnlock (hUpdateBuf);
		pBuf = GlobalLock (hUpdateBuf); 
		hTranCopyFile = KeepTranFileToBase; 
		CurrentRefno = INT_MAX;
		if (RecomputeMinMax (&MinMax,pBuf,lUpdateBuf,0,0))
			Deleted = FALSE;
		else
			Deleted = TRUE;
		ref = CurrentRefno;
		if (ref == 148069)
			ii=1;
		hTranCopyFile = 0;
		ItemHeader = (LPITEM)pBuf; 
		ItemHeader->MinMax = MinMax;
		First = TRUE;  
		hRec = 0; 
		hBuf = hUpdateBuf;
		hUpdateBuf = 0;
		while (SplitRec (&(HPSHORT)pBuf,&lUpdateBuf,&(LPSHORT)Rec,&RecLen,&hBuf,&hRec,TRUE))
		{
			loc = GetFileConnectOffset(LevelTypeFromPickType(PickTypeFromSysType(CurrentType)),CurrentDesc,MinMax,(short)RecLen,-1,Rec); 
			if (First)
			{
				First = FALSE;
				CurrentItem = loc - CurrentSeg -2;
				ItemSeg = CurrentSeg;
			}    
			GSSiGlobUlFree (&hBuf);
		}
		AddToSymList (CurrentDesc,&NumSyms,&hSymDesc);
		CurrentRefno = ref;
	   	if (CurrentRefno != INT_MAX)
			BuildRefIndex(TRUE,Deleted);
		else
			ii=1;
		UseRefOrTAGIndex = SaveUROT;
		break;
	case 2:
		if (KeepTranFileToBase == (HANDLE)1)
			KeepTranFileToBase = 0;
		else
			CloseTRANS2 (&KeepTranFileToBase);
		CloseMap (TRUE); 
		{
			AddSymToMap (NumSyms,hSymDesc,0,0); 
			DestroySymList (&NumSyms,&hSymDesc);
		}
		ForceRefIndex = ForceTAGIndex = FALSE;    
		UseRefOrTAGIndex = SaveUROT;

		UpdateItem = 0;
		break;
	}
   SetCurView ( SaveView);  
{
#if ENABLETRACE
GSSiExitProg (904);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SkipSubRec (LPBYTE Pcode,HPSHORT *ipnt,BOOL ReturnError)
#if ENABLETRACE
{GSSiEnterProg (905);
#endif
{	short	idesc, ItemLen, TSize, ltag;
	long	*pRefno;
	long	remlen; 
	static	long	LastRef;    
	static	WORD	nPnts;
	BOOL	rtn=TRUE;

    switch (*Pcode)
    {   
		case 1:
			(*ipnt) += 6;
		break;
		
		case 25:			
	    case 2: /* pen up */
		{
			(*ipnt)++;
			(*ipnt)++;
			/*pu++;*/
		}
	    break;
	
	    case 3: /* set pen */
		{    
			(*ipnt)++;
		}
	    break;
	
	    case 4: /* put line */
		{
			nPnts = 2;    
			CurElementType = GF_LINE;
			goto DoPolyline;
		}
	    break;
	
	    case 41: /* put line */
		{
			nPnts = 2;
		    *ipnt = (*ipnt) + nPnts * 8;
			CurElementType = GF_LINE;
		}
	    break;
	
	    case 5: /* put area */
		{
			(*ipnt)++;
			CurElementType = GF_AREA;
		    nPnts = *(*ipnt);
		    (*ipnt)++;
		    *ipnt = (*ipnt) + nPnts * 2;
		}
		break;
	
	    case 6: /* put polyline */
	
		{   nPnts = *(*ipnt);
			CurElementType = GF_POLYLINE;
		    (*ipnt)++;
	DoPolyline://lpCurPoints = (HPPOINT) (*ipnt);
		    *ipnt = (*ipnt) + nPnts * 2;
		}
		break;
	
	    case 151: /* put area offset*/
			(*ipnt)+=7;
			break;

	    case 51: /* put area */
		{
			(*ipnt)++;
			CurElementType = GF_AREA;
		    nPnts = *(*ipnt);
		    (*ipnt)++;
		    *ipnt = (*ipnt) + nPnts * 8;
		}
		break;
	
	    case 61: /* put polyline */
	
		{   nPnts = *(*ipnt);
		    (*ipnt)++;
//	        lpDCurPoints = (HPDPOINT) (*ipnt);
		    *ipnt = (*ipnt) + nPnts * 8;
			CurElementType = GF_POLYLINE;
		}
		break;
	
		case 7: /* block minmax */
		{	
			*ipnt += 4;
	    }
	    break;
	
	    case 8:	/*	description */
		{   
		 	CurrentDesc = **ipnt;
			(*ipnt)++;
	    }
	    break;
	
	    case 9:	/*	refno	*/
	    {   
	    	short ii;   
	    	static	long	debugrefno=1007241;
	    	  
           	pRefno =(LPLONG) *ipnt;
           	CurrentRefno = LastRef = *pRefno; 
           	if (CurrentRefno == debugrefno)
           		ii=1;
        	(*ipnt) += 2; 
        	ltag = *++Pcode;
        	if (ltag) 
        	{   LPSTR	lpTAG;
                	
        		lpTAG = (LPSTR)(*ipnt);
				(*ipnt) = (LPSHORT) (lpTAG + ltag + ltag%2);
			}  
	    }
	    break;
	
	    case 10:/*	street numbers	*/
	    {   (*ipnt) += (2*4);
	    }
	    break;
	
        case 11: /* get used description list*/
        {   short	ndesc, i;
                
moredesc:   ndesc = *(*ipnt);
        	ipnt++;
        	for (i=0;i<ndesc;i++)
        	{	ipnt++;
        		ipnt++;
            	ipnt+=4; 
            }
        	if (ndesc) goto moredesc;
        }
        break;
        
        case 93: //removed record
        case 92: /* deleted record */
		case 12: /* item minmax */
		{	
						    
			(*ipnt) += 4;
			ItemLen = abs(**ipnt);
			(*ipnt)++;
	    }
	    break;
	
		case 13: /* continuation offset */
		{	ContinuationOffset = *(LPLONG)(*ipnt);
			(*ipnt) += 2;
			**ipnt = 0;
	    }
	    break;
	
		case 14: /* text size */
		{	(*ipnt)++;
	    }
	    break;
	
		case 15: /* point symbol */
			(*ipnt) += 3;
		break; 
		
		case 16: /* line symbol */
			(*ipnt) += 4;
		break; 
		
		case 172: // HiPrecis Curve
			(*ipnt)++;
			(*ipnt) += 24;
			CurElementType = GF_LINE;
		break;
		
		case 171:
			(*ipnt)++;
		case 17: /* curve symbol */
			(*ipnt) += 6;
			CurElementType = GF_LINE;
		break;
		
		case 18: /* text character*/
			(*ipnt) += 5;
		break;
		
		case 19: // graphics text
		{ 
			short		nchar;
			LPGRTEXTHEADER	pGRTextHeader;
			LPSTR	pText;
			
		    pGRTextHeader = (LPGRTEXTHEADER)*ipnt;
		    (*ipnt) += sizeof(GRTEXTHEADER) / 2;  
		    nchar = pGRTextHeader->lText;  
		    (*ipnt) += nchar/2;
		}
		break;   
		
		case 191: //text pointer
			(*ipnt) += 38/2;
		break;
		
		case 192: //text pointer (UltiMap Style)
			(*ipnt) += 50/2;
		break;
		
		case 20: /* point symbol with size and real rot*/
			(*ipnt) += 6;  
			CurElementType = GF_POINT;
		break;
		
		case 120: // hiprecis point
			(*ipnt) += 16;
			CurElementType = GF_POINT;
		break;
		
		case 121:    
    	case 21: /* set pen color, width and style*/
    	{   
    		(*ipnt) += 4; 
    	}
    	break;
	        	
    	case 22: /* set brush pattern color and forecolor*/
    	{   
    		(*ipnt) += 5; 
    	}
    	break;
	        	
    	case 23: /* clear temp pen and brush */   
    	case 24: // dummy delete
    	break; 
		
		case 127:
    	case 27: // Multipolygon indicator
    	case 28: // Curve Point ID's
    	{
    		short	n;   
    		
    		n = *(*ipnt);
		    (*ipnt)++;
    		while (n--)
    			(*ipnt)++;
        }
        break;
		case 227:
    	case 228:  
    	{
    		int	n;   
    		
    		n = *(LPINT)(*ipnt);
		    (*ipnt)++;
		    (*ipnt)++;
    		while (n--)
			{
    			(*ipnt)++;
    			(*ipnt)++;
			}
        }
        break;
        
        case 32: //extra peoplenet data
		case 30: /* intersection data */
			(*ipnt) += **ipnt+1;
		break;

		case 31: /* intersection data */
			(*ipnt) += 12;
		break; 
		
	    case 35: /* point array */
	
		{   nPnts = *(*ipnt);
		    (*ipnt)++;
		    *ipnt = (*ipnt) + nPnts * 2;
		}
		break;   
		
	    case 135: /* Dpoint array */
	
		{   nPnts = *(*ipnt);
		    (*ipnt)++;
		    *ipnt = (*ipnt) + nPnts * 8;
		}
		break;   
		
		case 36: /* null code */
		break;   
		
		case 37: // time stamp
			(*ipnt) += 4;
		break;
	        	
	    case 38: /* Elevation array */
	
		{   nPnts = *(*ipnt);
		    (*ipnt)++;
		    *ipnt = (*ipnt) + nPnts * 2;
		}
		break;   
		
		case 40: /* command string */
		{                    
			lGCmdString = *(*ipnt);
		    (*ipnt)++;
			(*ipnt) += lGCmdString/2; 
		}
		break;	
	        	
        case 101:	/*	used description offset	*/
        {   
        	(*ipnt) += 2;
        }
        break;

	    case 102: /* color palette offset */
        {   
        	(*ipnt) += 2;
        }
        break;

        case 103: /* transformation point offset */
        {   
        	(*ipnt) += 2;
        }
        break;
        
        case 201: /* desc block offset */
        case 200: /* quad tree offset */
        {   
        	(*ipnt) += 2;
        }
        break;

        case 202: /* date range */
        {   
        	(*ipnt) += 4;
        }
        break;

	    default:
			rtn = FALSE;
			if (ReturnError)
				break;
            ProcessInvalidRecord ((short)*Pcode,*ipnt,4);
	    break;
	
	}
{
#if ENABLETRACE
GSSiExitProg (905);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL CopySubRec (LPBYTE Pcode,HPSHORT *ipnt)
#if ENABLETRACE
{GSSiEnterProg (906);
#endif
{	short	idesc, ItemLen, TSize, ltag;
	int		n;
	long	*pRefno;
	long	remlen; 
	static	long	LastRef;    
	static	WORD	nPnts;
    HPSTR	pBuf;
    short	PointSize=4;  
    BOOL	rtn = FALSE;
	HPSTR	pCoords; 
	LPINT	pMultiPolygon; 
	static	int	nPoly, iPoly; 
	LPINT	pPartLen; 
	LPSHORT	pCurvePoints;
	static	DPOINT	LinkPointD; 
	static	POINTS	LinkPoint;
	static	long	PolyBufferLen;
	static	HANDLE	hPolyBuffer=0;
	static	HANDLE	hPolyPartLen=0;
	static	HPPOINTS	lpCurPoints;
	static	HPDPOINT	lpDCurPoints;


    
    pBuf = GlobalLock (hUpdateBuf); 
    pBuf += lUpdateBuf;
    switch (*Pcode)
    {   
		case 1:
			(*ipnt) += 6;  
			BufWrite(&pBuf,&lUpdateBuf,Pcode,2+6*2);			
		break;
		
		case 25:			
	    case 2: /* pen up */
		{
			(*ipnt)++;
			(*ipnt)++;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+2*2);			
			/*pu++;*/
		}
	    break;
	
	    case 3: /* set pen */
		{    
			(*ipnt)++;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+1*2);			
		}
	    break;
	
	    case 4: /* put line */
		{
			nPnts = 2;  
			*Pcode = 6;
			goto DoPolyline;
		}
	    break;
	    
	    case 41: /* put line */
		{
			nPnts = 2;
	    	PointSize = 16; 
	    	*Pcode = 61;
			goto DoPolyline;
		}
	    break;
	    
	    case 151: /* put area offset*/
			(*ipnt) += 7;  
			BufWrite(&pBuf,&lUpdateBuf,Pcode,2+4*2+2*2);	
			break;

	    case 51:
	    	PointSize = 16;
	    case 5: /* put area */
	
			(*ipnt)++;
		    nPnts = *(LPWORD)(*ipnt);
		    (*ipnt)++; 
			goto DoPolyline;
	    
	    case 61:
	    	PointSize = 16;
	    case 6: /* put polyline */
	
		{
		    nPnts = *(LPWORD)(*ipnt);
		    (*ipnt)++; 
DoPolyline:	
			if (PointSize == 4)
				lpCurPoints = (HPPOINTS) (*ipnt);
			else
				lpDCurPoints = (HPDPOINT) (*ipnt);
		    *ipnt += nPnts * PointSize/2; 
		    if (hCoords) 
		    {
		    	nCoords += nPnts;
		    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
			    pCoords = GlobalLock (hCoords);
			    pCoords += lCoords;
		    	if (PointSize == 16)
		    	{
				    if (UpdateItem != 24)
						BufWrite (&pCoords,&lCoords,(HPSTR)lpDCurPoints,nPnts*PointSize);  
				    GlobalUnlock (hCoords);
				    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
				}
				else
				{
				    if (UpdateItem != 24)
				    	BufWrite (&pCoords,&lCoords,(HPSTR)lpCurPoints,(long)nPnts*PointSize);  
				    GlobalUnlock (hCoords);
				    lpCurPoints = (HPPOINTS)GlobalLock (hCoords); 
				}
			    nPnts = nCoords;
		    }
			if (nPoly && hPolyBuffer)
			{   
				long	lbuf=0;
				
				pCoords = GlobalLock (hPolyBuffer);
				pCoords += ((long)nPolyPoints*PointSize);
				pPartLen = (LPINT)GlobalLock (hPolyPartLen);
				pPartLen+=iPoly;
				*pPartLen = nPnts;
				GlobalUnlock (hPolyPartLen);
				if (PointSize == 16)
				{  
					if (UpdateItem != 24)
						BufWrite (&pCoords,&lbuf,(HPSTR)lpDCurPoints,(long)nPnts*PointSize);
					if (!iPoly)
						LinkPointD = *lpDCurPoints;
					else
					{
						*(HPDPOINT)pCoords = LinkPointD;
						nPolyPoints++; 
					}
				}
				else
				{
					if (UpdateItem != 24)
						BufWrite (&pCoords,&lbuf,(HPSTR)lpCurPoints,(long)nPnts*PointSize); 
					if (!iPoly)
						LinkPoint = *lpCurPoints;
					else
					{
						*(HPPOINTS)pCoords = LinkPoint;
						nPolyPoints++; 
					}
				}
				nPolyPoints += nPnts; 
				iPoly++;   
				GlobalUnlock (hPolyBuffer);
			}           
			if (iPoly == nPoly)
			{
				if (*Pcode == abs(UpdateItem)) //negative updateitem forces data to hiprecis
				{   
					if (UpdateItem < 0)
						PointSize = 16;
					GSSiGlobFree (&hPolyBuffer); 
					nPnts = nPolyPoints = nUpdatePolyPoints;   
					hPolyBuffer = hUpdatePoly; 
					hUpdatePoly = 0;
					GSSiGlobFree (&hPolyPartLen);
					hPolyPartLen = hUpdateMultiPolygon; 
					hUpdateMultiPolygon = 0;
					nMultiPolygon = nUpdateMultiPolygon;
					hUpdateMultiPolygon = 0;   
					GSSiGlobFree (&hCurvePoints);
					hCurvePoints = hUpdateCurvePoints;
					nCurvePoints = nUpdateCurvePoints;
				} 
				if (hCurvePoints)
				{   
					short	pcode = 28;
						
					pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
					if (UpdateItem != 24)
					{
						BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&pcode,2);			
						BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&nCurvePoints,2);			
						BufWrite(&pBuf,&lUpdateBuf,(HPSTR)pCurvePoints,nCurvePoints*2);	
					}		
					GlobalUnlock (hCurvePoints);
				}
				if (hPolyPartLen)
				{   
					short	pcode = 228;
						
					pMultiPolygon = (LPINT)GlobalLock (hPolyPartLen);
					if (PointSize == 16)
						pcode = 227;
					if (UpdateItem != 24)
					{
						BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&pcode,2);			
						BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&nMultiPolygon,4);			
						BufWrite(&pBuf,&lUpdateBuf,(HPSTR)pMultiPolygon,nMultiPolygon*4);
					}			
					GlobalUnlock (hPolyPartLen);
					nPoly = nMultiPolygon;	
				}
				else
					nPoly = 1;
				if (hPolyBuffer)
				{
					if (PointSize == 4)
						lpCurPoints = (HPPOINTS) GlobalLock (hPolyBuffer);
					else
						lpDCurPoints = (HPDPOINT) GlobalLock (hPolyBuffer);
				}
				for (iPoly = 0; iPoly<nPoly; iPoly++)
				{   
					if (nPoly > 1)
					{
						pMultiPolygon = (LPINT)GlobalLock (hPolyPartLen); 
						pMultiPolygon += iPoly;
						nPnts = *pMultiPolygon;
						GlobalUnlock (hPolyPartLen);
					}
					switch (*Pcode)
					{   
						case 4: 
							if (PointSize == 16)
								*Pcode = 41;
						case 41:
							if (UpdateItem != 24)
								BufWrite(&pBuf,&lUpdateBuf,Pcode,2);
							break;
						case 6:
							if (PointSize == 16)
								*Pcode = 61;
						case 61: 
							if (nPnts == 2)
							{
								if (PointSize == 16)
									*Pcode = 61;
								else
									*Pcode = 6;
							}
							if (UpdateItem != 24)
							{
								BufWrite(&pBuf,&lUpdateBuf,Pcode,2);
								BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&nPnts,2);
							}			
							break;
						case 5:
							if (PointSize == 16)
								*Pcode = 51;
						default:
							if (UpdateItem != 24)
							{
								BufWrite(&pBuf,&lUpdateBuf,Pcode,2+1*2);			
								BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&nPnts,2);
							}			
					}	
					if (PointSize == 16) 
					{                                        
						if (UpdateItem != 24)
							BufWrite(&pBuf,&lUpdateBuf,(HPSTR)lpDCurPoints,(long)nPnts*(long)PointSize);  
						lpDCurPoints += nPnts;
						if (iPoly)
							lpDCurPoints++;
					}
					else
					{
						if (UpdateItem != 24)
							BufWrite(&pBuf,&lUpdateBuf,(HPSTR)lpCurPoints,(long)nPnts*(long)PointSize);  
						lpCurPoints += nPnts;
						if (iPoly)
							lpCurPoints++;
					}
				}
				GSSiGlobFree (&hPolyPartLen);
				GSSiGlobUlFree (&hPolyBuffer); 
			}
			GSSiGlobUlFree (&hCoords);
		}
		break;
	
		case 7: /* block minmax */
		{	
			(*ipnt) += 4;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+4*2);			
	    }
	    break;
	
	    case 8:	/*	description */
		{   
			short	SymNum = *(*ipnt);
			
			if (UpdateItem == 8) 
				SymNum = NewSymbol;
			BufWrite(&pBuf,&lUpdateBuf,Pcode,2);			
			(*ipnt)++;
			BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&SymNum,2);			
	    }
	    break;
	
	    case 9:	/*	refno	*/
	    {   
	    	short ii;
	    	LPSHORT	pcode=(LPSHORT)Pcode;  
	    	BYTE	byte;
	    	  
           	pRefno =(LPLONG) *ipnt;
			if (ChangeRefnoTo != LONG_MIN)
				*pRefno = ChangeRefnoTo;
           	CurrentRefno = LastRef = *pRefno; 
        	(*ipnt) += 2; 
        	ltag = *++Pcode;
        	if (ltag) 
        	{   LPSTR	lpTAG;
                	
        		lpTAG = (LPSTR)(*ipnt);
				(*ipnt) = (LPSHORT) (lpTAG + ltag + ltag%2);
			}
			if (UpdateItem == 9)
			{   
				byte = 9;
				BufWrite(&pBuf,&lUpdateBuf,&byte,1);	
				byte = lNewTAG;		
				BufWrite(&pBuf,&lUpdateBuf,&byte,1);			
				BufWrite(&pBuf,&lUpdateBuf,(HPSTR)pRefno,4);			
				BufWrite(&pBuf,&lUpdateBuf,NewTAG,lNewTAG + lNewTAG%2);			
			}
			else  
				BufWrite(&pBuf,&lUpdateBuf,(HPSTR)pcode,2+2*2+(ltag + ltag%2));			
	    }
	    break;
	
	    case 10:/*	street numbers	*/
	    {   (*ipnt) += (2*4);
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+8*2);			
	    }
	    break;
	
        case 11: /* get used description list*/
        {   short	ndesc, i;
                
moredesc:   ndesc = *(*ipnt);
        	ipnt++;
        	for (i=0;i<ndesc;i++)
        	{	ipnt++;
        		ipnt++;
            	ipnt+=4; 
            }
        	if (ndesc) goto moredesc;
        }
        break;
         
        case 93: 
        case 92: /* deleted record */
		case 12: /* item minmax */
		{	
						    
			GSSiGlobFree (&hCoords);
			GSSiGlobFree (&hPolyPartLen);			
			GSSiGlobFree (&hPolyBuffer); 
			GSSiGlobFree (&hCurvePoints); 
			nPoly = iPoly = 0;
			if (HaveFirstHeader)    
			{
				if (ProcessSingleItem) 
					**ipnt = 0; 
				else if (FastMapCopy)
				{
					GlobalUnlock (hUpdateBuf);
					pBuf = GlobalLock (hUpdateBuf);

					if (lUpdateBuf)
					{
						BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
						BigWrite (FastMapCopyFid,pBuf,lUpdateBuf,-1);
						lUpdateBuf = 0;
						FastMapCopynRecs++;
						if (!(FastMapCopynRecs % 100))
						{
							char	mess[64];

							sprintf (mess,"%i records copied",FastMapCopynRecs);
							SetWindowText (CurView->hWnd,mess);
						}
					}
					(*ipnt) += 4;
					ItemLen = abs(**ipnt);
					(*ipnt)++;
					BufWrite(&pBuf,&lUpdateBuf,Pcode,2+5*2);
				}
			}
			else  
			{
				if (CopyRec != 2)
					HaveFirstHeader=TRUE;
				else
				{
					lUpdateBuf = 0;
				}
				(*ipnt) += 4;
				ItemLen = abs(**ipnt);
				(*ipnt)++;
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+5*2);
			}
	    }
	    break;
	
		case 13: /* continuation offset */
		{	ContinuationOffset = *(LPLONG)(*ipnt);
			(*ipnt) += 2;
			**ipnt = 0;   
			if (ContinuationOffset >=0) 
				rtn = TRUE;
	    }
	    break;
	
		case 14: /* text size */
		{	(*ipnt)++;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+1*2);			
	    }
	    break;
	
		case 15: /* point symbol */
			(*ipnt) += 3;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+3*2);			
		break; 
		
		case 16: /* line symbol */
			(*ipnt) += 4;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+4*2);			
		break; 
		
		case 172: 
		{
			HPDPOINT	pBP,pPOC,pEP;
			(*ipnt)++; 
			pBP = (HPDPOINT)(*ipnt);
			(*ipnt) += 8;
			pPOC = (HPDPOINT)(*ipnt);
			(*ipnt) += 8;
			pEP = (HPDPOINT)(*ipnt);
			(*ipnt) += 8;
			if (UpdateItem == 172) 
			{
				HPDPOINT pUpdatePoints=(HPDPOINT)GlobalLock (hUpdatePoly);
				
				*pBP = pUpdatePoints[0];
				*pPOC = pUpdatePoints[1];
				*pEP = pUpdatePoints[2];
				GlobalUnlock (hUpdatePoly); 
			}
			BP = *pBP;
			POC = *pPOC;
			EP = *pEP; 
			if (UpdateItem != 24) 
			{
				if (ConvertCurvesToPolylines)
				{   
					HANDLE	hMem = GSSiGlobAlloc ( 307,GMEM_MOVEABLE,USHRT_MAX);
		 		    HPDPOINT lpPoints = (HPDPOINT)GlobalLock (hMem); 
		 		    HPDPOINT pFirstPoint=lpPoints; 
		 		    double	BackAZ; 
		 		    long	nPnt;   
		 		    short	icode=61;
					
		 		    nPnt = 0;
                    CurvePointsD(&BP,&POC,&EP, &nPnt,  &lpPoints, &BackAZ,USHRT_MAX/4,CurveExpansionFactor,1);
                    nPnts = (USHORT)nPnt;
					BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&icode,2);
					BufWrite(&pBuf,&lUpdateBuf,(HPSTR)&nPnts,2);
					BufWrite(&pBuf,&lUpdateBuf,(HPSTR)pFirstPoint,nPnts*sizeof(DPOINT));
					GSSiGlobUlFree (&hMem);
				}
				else
					BufWrite(&pBuf,&lUpdateBuf,Pcode,2+2+6*8);
			}
		}			
		break;
		
		case 171:
			(*ipnt)++;
		case 17: /* line symbol */
			(*ipnt) += 6;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+6*2);			
		break;
		
		case 18: /* text character*/
			(*ipnt) += 5;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+5*2);			
		break;
		
		case 19: // graphics text
		{ 
			short	nchar;
			LPGRTEXTHEADER	pGRTextHeader;
			LPSTR	pText;
	    	BYTE	byte;
			
		    pGRTextHeader = (LPGRTEXTHEADER)*ipnt;
		    (*ipnt) += sizeof(GRTEXTHEADER) / 2;  
		    nchar = pGRTextHeader->lText;  
		    (*ipnt) += nchar/2; 
			if (UpdateItem == 19)
			{   
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2);	
				pGRTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);	
		    	nchar = pGRTextHeader->lText;  
				BufWrite(&pBuf,&lUpdateBuf,(HPSTR)pGRTextHeader,(sizeof(GRTEXTHEADER) / 2+nchar/2)*2);
				GlobalUnlock (hPickedTextHeader);	
            }
            else if (UpdateItem != 24)
            {
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+(sizeof(GRTEXTHEADER) / 2+nchar/2)*2);			
			}
		}
		break;
		
		case 191: //text pointer  
			(*ipnt) += 38/2;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+38);			
		break;
		
		case 192: //text pointer (UltiMap Style)  
			(*ipnt) += 50/2;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+38);			
		break;
		
		case 120: // point symbol (hi precis) 
			if (UpdateItem == 20)
			{
	    		double PTRot,CurPointSize = *(LPDOUBLE)*ipnt;
        		(*ipnt) += 4;
        		PTRot = *(LPDOUBLE)*ipnt;
        		(*ipnt) += 4;
        		lpDCurPoints = (LPDPOINT)*ipnt; 
        		*lpDCurPoints = NewPointD;
        		(*ipnt) += 8;  
			}
			else if (UpdateItem == 201)
			{   
				double	CurPointSize = *(LPDOUBLE)*ipnt;
	    		LPDOUBLE pRot;
        		(*ipnt) += 4;
        		pRot = (LPDOUBLE)*ipnt; 
        		AdjustPointRotation (pRot);
        		(*ipnt) += 4;
        		lpDCurPoints = (LPDPOINT)*ipnt; 
        		(*ipnt) += 8;  
			}
			else if (UpdateItem == 202)
			{   
				double	PTRot; 
	    		LPDOUBLE pSize = (LPDOUBLE)*ipnt;

				*pSize *= NewPointSizeFactor;
        		(*ipnt) += 4;
        		PTRot = *(LPDOUBLE)*ipnt; 
        		(*ipnt) += 4;
        		lpDCurPoints = (LPDPOINT)*ipnt; 
        		(*ipnt) += 8;  
			}
			else
				(*ipnt) += 16;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+16*2);			
		break;
		
		case 20: /* point symbol with size and real rot*/
			if (UpdateItem == 20)
			{
	    		float PTRot,CurPointSize = *(LPFLOAT)*ipnt;
        		(*ipnt) += 2;
        		PTRot = *(LPFLOAT)*ipnt;
        		(*ipnt) += 2;
        		lpCurPoints = (LPPOINTS)*ipnt; 
        		*lpCurPoints = POINTtoPOINTS(BasePtToFilePt (NewPointD));
	        	(*ipnt) += 2;  
	        }
	        else	
				(*ipnt) += 6;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+6*2);			
		break;
		
		case 121:    
    	case 21: /* set pen color, width and style*/
    	{   
    		(*ipnt) += 4; 
    	}
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+4*2);			
    	break;
	        	
    	case 22: /* set brush pattern color and forecolor*/
    	{   
    		(*ipnt) += 5; 
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+5*2);			
    	}
    	break;
	    case 24: //dummy delete 
	    	HaveDummyDelete = TRUE;   	
    	case 23: /* clear temp pen and brush */
			BufWrite(&pBuf,&lUpdateBuf,Pcode,2);			
    	break; 
		
		case 127: 
			PointSize = 16;
    	case 27: // Multipolygon indicator
    	{
    		nMultiPolygon = n = nPoly = *(*ipnt);
		    (*ipnt)++;
		    GSSiGlobFree (&hPolyPartLen);
			GSSiGlobFree (&hPolyBuffer); 
            hPolyPartLen = GSSiGlobAlloc ( 666,GMEM_MOVEABLE,nMultiPolygon*sizeof(int));
            pMultiPolygon = (LPINT)GlobalLock (hPolyPartLen);  
            PolyBufferLen = 2 * (nPoly - 1);
    		while (n--) 
    		{   
    			PolyBufferLen += *(LPWORD)(*ipnt);
    			*pMultiPolygon = *(LPWORD)(*ipnt)++;
    		}
           	GlobalUnlock (hPolyPartLen);
       		hPolyBuffer = GSSiGlobAlloc ( 667,GMEM_MOVEABLE,(long)PolyBufferLen*PointSize);  
       		nPolyPoints = 0;
        }
        break;

		case 227: 
			PointSize = 16;
    	case 228: // Multipolygon indicator
    	{
    		nMultiPolygon = n = nPoly = *(LPINT)(*ipnt);
		    (*ipnt)++;
		    (*ipnt)++;
		    GSSiGlobFree (&hPolyPartLen);
			GSSiGlobFree (&hPolyBuffer); 
            hPolyPartLen = GSSiGlobAlloc ( 666,GMEM_MOVEABLE,nMultiPolygon*sizeof(int));
            pMultiPolygon = (LPINT)GlobalLock (hPolyPartLen);  
            PolyBufferLen = 2 * (nPoly - 1);
    		while (n--) 
    		{   
    			PolyBufferLen += *(LPINT)(*ipnt);
    			*pMultiPolygon = *(LPINT)(*ipnt)++;
			    (*ipnt)++;
    		}
           	GlobalUnlock (hPolyPartLen);
       		hPolyBuffer = GSSiGlobAlloc ( 667,GMEM_MOVEABLE,(long)PolyBufferLen*PointSize);  
       		nPolyPoints = 0;
        }
        break;

    	case 28: // Curve Point ID's
    	{
    		short	n; 
	        		
			GSSiGlobFree (&hCurvePoints); 
    		nCurvePoints = n = *(*ipnt); 
		    (*ipnt)++;
    		hCurvePoints = GSSiGlobAlloc ( 668,GMEM_MOVEABLE,(nCurvePoints+1)*sizeof(short));
    		pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
    		while (n--)
    			*pCurvePoints++ = *(*ipnt)++;  
    		*pCurvePoints = -1;
    		GlobalUnlock (hCurvePoints);
    	}
    	break;
        
        case 32: //extra Peoplenet data
		case 30: /* intersection data */ 
		{
			short	n;  
			n = **ipnt + 1;
			(*ipnt) += **ipnt+1;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+n*2);			
		}
		break;

		case 31: /* intersection data */
			(*ipnt) += 12;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+12*2);			
		break; 
		
		case 135:
    		PointSize = 16;
	    case 35: /* point array */
	
		{   nPnts = *(*ipnt);
		    (*ipnt)++;

		    if (hCoords) 
		    {
		    	nCoords += nPnts;
		    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
		    }
		    else 
		    {
		    	hCoords = GSSiGlobAlloc ( 669,GMEM_MOVEABLE,(long)nPnts*PointSize);
		    	nCoords = nPnts;
		    	lCoords = 0;
		    }
		    pCoords = GlobalLock (hCoords);
		    pCoords += lCoords;
		    if (UpdateItem != 24)
		    	BufWrite (&pCoords,&lCoords,(HPSTR)(*ipnt),nPnts*PointSize); 
		    GlobalUnlock (hCoords);
		    *ipnt = (*ipnt) + nPnts * PointSize/2;
		}
		break;   
		
		case 36: /* null code */
//			BufWrite(&pBuf,&lUpdateBuf,Pcode,2);			
		break;
	        	
		case 37: /* block minmax */
		{	
			GRStartTime = *(LPLONG)(*ipnt);
			(*ipnt) += 2;
			GREndTime = *(LPLONG)(*ipnt);
			(*ipnt) += 2;
			BufWrite(&pBuf,&lUpdateBuf,Pcode,2+4*2);			
	    }
	    break;
	
	    case 38: /* elevation array */
	
		{  	PointSize = 4;
			nPnts = *(*ipnt);
		    (*ipnt)++;
		    *ipnt = (*ipnt) + nPnts * PointSize/2;
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+2+nPnts*PointSize);			
		}
		break;   
		
		case 40: /* command string */
		{                    
			lGCmdString = *(*ipnt);
		    (*ipnt)++;
			(*ipnt) += lGCmdString/2; 
			if (UpdateItem != 24)
				BufWrite(&pBuf,&lUpdateBuf,Pcode,2+2+lGCmdString);			
		}
		break;	
	        	
        case 101:	/*	used description offset	*/
        {   
        	(*ipnt) += 2;
        }
        break;

	    case 102: /* color palette offset */
        {   
        	(*ipnt) += 2;
        }
        break;

        case 103: /* transformation point offset */
        {   
        	(*ipnt) += 2;
        }
        break;
        
        case 201: /* desc block offset */
        case 200: /* quad tree offset */
        {   
        	(*ipnt) += 2;
        }
        break;

	    default:
            ProcessInvalidRecord ((short)*Pcode,*ipnt,4);
	    break;
	
	}  
	GlobalUnlock (hUpdateBuf);
{
#if ENABLETRACE
GSSiExitProg (906);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

void cwCenterBelowCursor(HWND hWnd, short top)
#if ENABLETRACE
{GSSiEnterProg (907);
#endif
{
 POINT      pt, CursorPt;
 RECT       swp;
 RECT       rParent;
 int        iwidth;
 int        iheight;

 /* get the rectangles for the parent and the child                     */
 GetWindowRect(hWnd, &swp);
 GetClientRect(hWndMain, &rParent);
 GetCursorPos(&CursorPt);
 /* calculate the height and width for MoveWindow                       */
 iwidth = swp.right - swp.left;
 iheight = swp.bottom - swp.top;

 /* find the center point and convert to screen coordinates             */
// pt.x = (rParent.right - rParent.left) / 2;
// pt.y = (rParent.bottom - rParent.top) / 2;
// ClientToScreen(hWndMain, &pt);

 /* calculate the new x, y starting point                               */
// pt.x = pt.x - (iwidth / 2);
// pt.y = pt.y - (iheight / 2);
 pt.x = CursorPt.x - (iwidth / 2);
 
 if(pt.x < rParent.left) pt.x = rParent.left;
 if(pt.x + iwidth > rParent.right)
     pt.x = rParent.right - (iwidth);
 pt.y = CursorPt.y;
 /* top will adjust the window position, up or down                     */
 if(top)
   pt.y = pt.y + top;

 /* move the window                                                     */
 MoveWindow(hWnd, pt.x, pt.y, iwidth, iheight, FALSE);
#if ENABLETRACE
}
#endif
}

void cwCenterInVP (HWND hWnd, short VPID)
#if ENABLETRACE
{GSSiEnterProg (452);
#endif
{
 POINT      pt;
 RECT       swp;
 RECT       rParent;
 int        iwidth;
 int        iheight; 
 HWND		hPWnd;  
 BOOL		IsClient=TRUE;
 
 if (VPID <1 || VPID > *pNumViewports)
 	return;
 /* get the rectangles for the parent and the child                     */
 GetWindowRect(hWnd, &swp); 
 rParent = pViewports[VPID-1]->DrawRect;

 /* calculate the height and width for MoveWindow                       */
 iwidth = swp.right - swp.left;
 iheight = swp.bottom - swp.top;

	 /* find the center point and convert to screen coordinates             */
	 pt.x = (rParent.right + rParent.left) / 2;
	 pt.y = (rParent.bottom + rParent.top) / 2; 
	 if (IsClient)
	 	ClientToScreen(hWndMain, &pt);


 /* calculate the new x, y starting point                               */
 pt.x = max (0,pt.x - (iwidth / 2));
 pt.y = max (0,pt.y - (iheight / 2));


 /* move the window                                                     */
 SetWindowPos(hWnd,0,pt.x, pt.y, iwidth, iheight, SWP_NOZORDER);
{
#if ENABLETRACE
GSSiExitProg (452);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayPickedItems (HWND hWnd,int NumPickedIn,BOOL UseMenus, LPSTR Cmd,LPSTR File,BOOL SavePickList)
#if ENABLETRACE
{GSSiEnterProg (908);
#endif
{	int		item, NumPicked=abs(NumPickedIn);
    HFILE	FidLast=HFILE_ERROR;  
    HANDLE	hStr;
    char	SymName[64],DPIMacro[256],PMFile[MAX_PATH];
    BOOL	rtn=FALSE; 
    LPSTR	lpDesc, lpSymbol,lpAction;     
    HMENU	Menus[MAXPICKITEMS+3], PickMenu;
    long	loc;
    BOOL	TopOnlyOpt, SingleOpt, DescTAGOpt;
    UINT	CmdID; 
    POINT	position; 
    int		irec, imenu=1,NumAction, TotAction,SingleItem,SingleRec;  
    LPVIEWPORT	lpV, SaveVP=CurView; 
    HANDLE	hNames=0;
	int		nNames;
	LPSTR	pNames, str,textx, txt, pIncludePath; 
	BOOL	WantThisItem, HaveCancel=FALSE;   
	int		i;
    
    if (!SavePickList)
    	nSavePickList = 0;

    if (!NumPicked)
{
#if ENABLETRACE
GSSiExitProg (908);
#endif
    	return rtn;
}
	if (NumPicked < 1 || NumPicked > MAXPICKITEMS)
	{
		char Mess[64];
		itoa (NumPicked,Mess,10);
		MessageBox (0,"Invalid pick item in DisplayPickedItems",Mess,MB_ICONEXCLAMATION);
{
#if ENABLETRACE
GSSiExitProg (908);
#endif
    	return rtn;
}
	}
	GetGlobalCVal ("[%DPIMacro]",DPIMacro,"");
    if (File)
		FidPM = GSSiOpenFile (File,&OFStruct,OF_READ);  
    else
    { 
		SetConfig (PickList[NumPicked-1].ConfigID);
	    lpV=GetVP (PickList[NumPicked-1].ViewID);  
		GetPMName(lpV->PickMacroFile,PMFile);
		FidPM = GSSiOpenFile (PMFile,&OFStruct,OF_READ);  
	}
    if (FidPM == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (908);
#endif
    	return rtn;  
}
	if (File)
		AddToMacroStack(2, 0, File, 0, 0);
	else
		AddToMacroStack(2, 0, PMFile, 0, 0);

    hStr = GSSiGlobAlloc ( 670,GMEM_MOVEABLE,2048*4);
    str=GlobalLock (hStr);
	origstr = str;
    textx=str+2048;
	txt=textx+2048;
    pIncludePath=txt+2048;  
    if (File)  
    	_fstrcpy (LastPMFile,File);
    else
    	_fstrcpy (LastPMFile,OFStruct.szPathName);
    fgstat = fgetstring (PMstr,1020,FidPM);
	while (fgstat && PMstr[0]<'A')
	{
		fgstat = fgetstring (PMstr,1020,FidPM);
	}
	TopOnlyOpt = (PMstr[0]=='Y');
	DescTAGOpt = (PMstr[1]=='Y');
	if (NumPickedIn < 0)
		DescTAGOpt = FALSE;
	SingleOpt = (PMstr[2]=='Y');    
	loc = GSSillseek (FidPM,0,1);
    fgetstring (PMstr,1020,FidPM);
	while (PMstr[0]<'!')
	{
		loc = GSSillseek(FidPM,0,1);
		fgetstring (PMstr,1020,FidPM);
	}
	item=NumPicked;
	TotAction = 0; 
	if (UseMenus)
	{
		PickMenu = CreatePopupMenu(); 
		if (GetDebug ())
		{
			sprintf (str,"Edit Menu");
			AppendMenu (PickMenu,MF_ENABLED|MF_STRING,IDM_PROCESSTEXT,str);
		}
		Menus[0] = PickMenu;
	}
	while (item--)	
	{  
		char	GlobFile[MAX_PATH];
		LPSTR	pBS;

   		SetConfig (PickList[item].ConfigID);
   		SetViewport (PickList[item].ViewID);
		*PickName = 0;
		if (item > MAXPICKITEMS)
			MessageBox (0,"Invalid item",0,MB_ICONEXCLAMATION);
		SetPickGlobals (item);
//		if (dbug)
//		{
//			strncpy0 (GlobFile,PickName,250);
//			AppendFile2 ("c:\\gmavl\\pndump.txt",GlobFile);
//		}
		strcpy (GlobFile,PickName);
		ExpandText (GlobFile);
//		if (dbug)
//			AppendFile2 ("c:\\gmavl\\pndump.txt",GlobFile);
		if ((pBS = strrchr (GlobFile,'\\')))
		{

			*(++pBS) = 0;
			strcat (GlobFile,"global.ini");
			LoadGlobalInit (GlobFile,FALSE);
		}
		if (PickList[item].Desc < 0) 
		{
			nNames = 1;
			pNames = SymName; 
			hNames = 0;
			_fstrcpy (SymName,PickList[item].Prefix);
		}
		else 
		{   
			short SaveVisListOpt = VisListOpt;
			
			VisListOpt = 0;
			nNames = GetSymbolNames (PickList[item].Desc,&hNames,-(PickList[item].FileNum+1));  
			pNames = GlobalLock (hNames);
			VisListOpt = SaveVisListOpt;
		}
	    GSSillseek (FidPM,loc,0); 
	    NumAction=0;
	    if (DescTAGOpt)                                          
	    {   
	    	if (!_fstricmp (PickList[item].Prefix,"%VIEWPORT"))
	    		_fstrcpy (txt,"Viewport Options");
	    	else if (!_fstricmp (PickList[item].Prefix,"%DISTDSP"))
			{
				if (!stricmp (PickList[item].UDI,"Line"))
	    			sprintf (txt,"Dimension Line Options");
				else
	    			sprintf (txt,"Dimension Area Options");
			}
	    	else if (!_fstricmp (PickList[item].Prefix,"%DTM"))
	    		sprintf (txt,"Surface Options for %s",PickList[item].UDI);
			else if (*DPIMacro)
			{
				sprintf (txt,"$MACRO(%s,%s,%s,%s)",DPIMacro,pNames,PickList[item].Prefix,PickList[item].UDI); 
				ExpandText (txt);
			}
	    	else
	    	{
		    	sprintf (txt,"%s (%s:%s)",
		    				 pNames,PickList[item].Prefix,PickList[item].UDI); 
			}
	    	if (UseMenus)
				Menus[imenu++] = CreatePopupMenu();
	    }          
	    irec = -1;
Top:
	    while (fgetstring (PMstr,1020,FidPM))
	    {   
	    	if (!_fstrnicmp (PMstr,"INCLUDE ",8))
	    	{ 
	    		_fstrcpy (pIncludePath,&PMstr[8]);
	    		ExpandText (pIncludePath);
	    		if (*pIncludePath)
	    		{
		    		FidLast = FidPM;
					FidPM = GSSiOpenFile (pIncludePath,&OFStruct,OF_READ);  
					if (FidPM == HFILE_ERROR)
					{
						FidPM = FidLast;
						FidLast = HFILE_ERROR;
					}
					else
						AddToMacroStack(5, 0, pIncludePath, 0, 0);
					goto Top;
				}
	    	}
	    	if (PMstr[0]<'!') goto GetNextLine;
	    	irec++;
			if ((lpDesc = MatchLev(PMstr,',')))
			{
				*lpDesc++ = 0;
				_fstrcpy (text,PMstr);
				ExpandText (text);
				if (*text == '-')
				{
					if (_fstricmp(&text[1], PickList[item].Prefix))
						WantThisItem = FALSE;
					else
						WantThisItem = TRUE;
				}
				else if (*text == '|')
				{
					if (!strnicmp(&text[1], "POLYLINE|", 9) && TypeIsPolyline(PickList[item].Type))
						WantThisItem = TRUE;
					else if (!strnicmp(&text[1], "POLYGON|", 9) && TypeIsPolygon(PickList[item].Type))
						WantThisItem = TRUE;
					else if (!strnicmp(&text[1], "HIGHLIGHTED|", 9) && RefnoInHighlightList(PickList[item].Refno))
						WantThisItem = TRUE;
					else
						WantThisItem = FALSE;
				}
				else
					WantThisItem =  NameInSymList (text,nNames,pNames) || (*PickList[item].Prefix != '%' && !_fstricmp(text,"ALL"));
				if (WantThisItem)
				{           
					if ((lpAction=MatchLev(lpDesc,',')))
					{   
						SingleItem = item;
						SingleRec = irec;
						*lpAction++ = 0;  
						if (!_fstricmp(lpAction,"DOCUMENTS"))
						{
							DisplayDocumentList (hWnd,
												 PickList[NumPicked-1].Prefix,
												 PickList[NumPicked-1].UDI,item,Menus[imenu-1]);
							NumAction++; 
							TotAction++;
						} 
						else if (!_fstricmp(lpAction, "SEPARATOR"))
						{
							if (UseMenus)
							{
								AppendMenu(Menus[imenu - 1], MF_SEPARATOR, 0,0);
							}
						}
						else if (_fstricmp(lpAction, "NULL"))
                        {   
                        	HANDLE	hTxt=GSSiGlobAlloc ( 671,GMEM_MOVEABLE,2048);
                        	LPSTR	pTxt=GlobalLock (hTxt);
                        	
                        	if (SavePickList)
                        	{   
                        		nSavePickList = NumPicked;
                        		for (i=0;i<nSavePickList;i++)
    								SavedPickList[i] = PickList[i];  
                                 SavePickList = FALSE;
                            }
                        	rtn = TRUE;
							//CmdID = item * 256 + irec + 64000; 
							CmdID = MAKELONG (item+64000,irec);
							_fstrcpy (pTxt,lpDesc);
							ExpandText (pTxt);  
							if (*pTxt)
							{
								if (UseMenus)
								{						
									AppendMenu (Menus[imenu-1],MF_ENABLED|MF_STRING,CmdID,pTxt);    
									if (imenu == 1 && !_fstricmp (pTxt,"Cancel"))
										HaveCancel = TRUE;
								}
								else 
								{
									SetGlobalValue ("%ZOOMSELECTIONTEXT",pTxt);
									if (Cmd)
									{
										if (!_fstrnicmp (lpAction,"COMMAND,",8))
										{
											lpAction += 8;
											_fstrcpy (Cmd,lpAction);
										}
									}
									item = 0;//shows only topmost item
								}
								NumAction++; 
								TotAction++;
							} 
							GSSiGlobUlFree (&hTxt);
						}
					}
				}
			}
GetNextLine:;			 
		}
		if (FidLast != HFILE_ERROR)
		{
			GSSiClose2 (&FidPM);
			FidPM = FidLast; 
			FidLast = HFILE_ERROR;
			goto Top;
		} 
	    if (DescTAGOpt && UseMenus)                                          
	    {   
			if (NumAction)
				AppendMenu (PickMenu,MF_POPUP,(UINT)Menus[imenu-1],txt);
			//else
			//	AppendMenu (PickMenu,MF_ENABLED|MF_STRING,0,txt);
		}
		NumAction = 0;
		if (TopOnlyOpt)
			item=0;
		GSSiGlobUlFree (&hNames);
	} 
	GSSiClose2 (&FidPM);
	if (UseMenus && ((!SingleOpt && TotAction == 1) || (!hWnd && TotAction)))
		ProcessPickedItems (hWndMain,SingleItem,SingleRec);
	else if (hWnd && UseMenus)
	{
	   	GetCursorPos (&position);  
	   	if (GetMenuItemCount(PickMenu))
	   	{   
			HANDLE	hScreen;
			HDC		hDC = GetDC (hWnd);
			RECT	Rect;

	   		if (!HaveCancel)
	   			AppendMenu (PickMenu,MF_ENABLED|MF_STRING,0,"Cancel");
	   		setDoPaint( FALSE);
			GetClientRect (hWnd,&Rect);
			hScreen = SaveScreen2 (hWnd,hDC,Rect,0,0);
	  		TrackPopupMenu (PickMenu,TPM_CENTERALIGN|TPM_VCENTERALIGN,position.x,position.y,0,hWnd,0);
	    	RestoreScreen2 (hDC, hScreen,0,FALSE);
	    	DestroySavedScreen (&hScreen,0);
			ReleaseDC (hWnd,hDC);
	  	}
	}
	if (UseMenus)
  		while (imenu)
	  		DestroyMenu (Menus[(imenu--)-1]); 
	GSSiGlobUlFree (&hStr);
	GSSiGlobUlFree (&hNames);
	CurView = SaveVP; 
{
#if ENABLETRACE
GSSiExitProg (908);
#endif
	return rtn; 
}
	                                     
#if ENABLETRACE
}
#endif
}   

BOOL ItemInPickMacro (int Item)
#if ENABLETRACE
{GSSiEnterProg (909);
#endif
{	 

    HANDLE	hStr;
    char	SymName[128], PMFile[MAX_PATH];
    BOOL	rtn=FALSE; 
    LPSTR	lpDesc, lpSymbol,lpAction;     
    LPVIEWPORT	lpV; 
    HANDLE	hNames=0;
	short	nNames;
	LPSTR	pNames, text; 
	BOOL	WantThisItem;   
	long	loc;  
	LPVIEWPORT	SaveVP=CurView;
    
	if (Item < 0 || Item > MAXPICKITEMS)
	{
		char Mess[64];
		itoa (Item,Mess,10);
		MessageBox (0,"Invalid pick item in ItemInPickMacro",Mess,MB_ICONEXCLAMATION);
{
#if ENABLETRACE
GSSiExitProg (909);
#endif
    	return rtn;
}
	}
	SetConfig (PickList[Item].ConfigID);
    lpV=GetVP (PickList[Item].ViewID);
	GetPMName(lpV->PickMacroFile,PMFile);
	FidPM = GSSiOpenFile (PMFile,0,OF_READ);
    if (FidPM == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (909);
#endif
    	return rtn;
}
 	SetConfig (PickList[Item].ConfigID);
	SetViewport (PickList[Item].ViewID);
	SetPickGlobals (Item);
    hStr = GSSiGlobAlloc ( 672,GMEM_MOVEABLE,4096+1024);
    text=GlobalLock (hStr);
    if (!fgetstring (PMstr,1020,FidPM))
		goto Exit;
	while (PMstr[0]<'A')
	{
		if (!fgetstring (PMstr,1020,FidPM))
			goto Exit;
	}
	loc = GSSillseek (FidPM,0,1);
    if (!fgetstring (PMstr,1020,FidPM))
		goto Exit;
	while (PMstr[0]<'!')
	{
		loc = GSSillseek(FidPM,0,1);
		if (!fgetstring (PMstr,1020,FidPM))
			goto Exit;
	}
	if (PickList[Item].Desc < 0) 
	{
		nNames = 1;
		pNames = SymName; 
		hNames = 0;
		_fstrcpy (SymName,PickList[Item].Prefix);
	}
	else 
	{
   		SetConfig (PickList[Item].ConfigID);
	    SetViewport (PickList[Item].ViewID);
		nNames = GetSymbolNames (PickList[Item].Desc,&hNames,3);  
		pNames = GlobalLock (hNames);
	}
    GSSillseek (FidPM,loc,0); 
    while (fgetstring (PMstr,1020,FidPM))
    {   
    	if (PMstr[0]<'!')
			goto GetNextLine;
		if ((lpDesc = MatchLev(PMstr,',')))
		{
			*lpDesc++ = 0;
			_fstrcpy (text,PMstr);
			ExpandText (text);
			if (*text == '-')
			{
				if (_fstricmp (&text[1],PickList[Item].Prefix))
					WantThisItem = FALSE;
				else
					WantThisItem = TRUE;
			}
			else
				WantThisItem =  NameInSymList (text,nNames,pNames) ||!_fstricmp(text,"ALL");
			if (WantThisItem)
			{ 
				rtn = TRUE;
				goto Exit;          
			}
		}
GetNextLine:;			 
	}  
Exit:
	GSSiClose2 (&FidPM);
	GSSiGlobUlFree (&hStr);
	GSSiGlobUlFree (&hNames); 
	CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (909);
#endif
	return rtn; 
}
	                                     
#if ENABLETRACE
}
#endif
}   

BOOL ProcessPickedItems (HWND hWnd,int item, int irec)
#if ENABLETRACE
{GSSiEnterProg (910);
#endif
{	int nRc,ii,i;
    HFILE	Fid, FidLast=HFILE_ERROR; 
    HANDLE	hSTR=GSSiGlobAlloc ( 673,GMEM_MOVEABLE,4096+1024+1024+4096);
    LPSTR	str=GlobalLock(hSTR);
    LPSTR	txt=str+1024;
    LPSTR	NextLine=txt+1024; 
    LPSTR	pIncludePath = NextLine+1024;
    BOOL	rtn=FALSE; 
    LPSTR	lpSymbol,lpAction, lpNext, lpHOUSE, lpSTREET, lpCITY, lpVIEWPORT, lpLAYER;     
    int		rec; 
    long	CurLoc;
    short	Layer;   
	OFSTRUCTGM	OFStruct;
    LPVIEWPORT	SaveVP=CurView;
    
    for (i=0;i<nSavePickList;i++)
    	PickList[i] = SavedPickList[i];
    nSavePickList = 0;
	Fid = GSSiOpenFile (LastPMFile,&OFStruct,OF_READ);
    if (Fid == HFILE_ERROR)
    {
		GSSiGlobUlFree (&hSTR);  
{
#if ENABLETRACE
GSSiExitProg (910);
#endif
    	return FALSE;
}
    }
    if (item)
    	PickList[0] = PickList[item]; //put into slot 0 for GF functions using GF_USEPICKED message
    NumPicked = 1;
	SetConfig (PickList[0].ConfigID);
    SetViewport (PickList[0].ViewID);
	hIntData = GSSiGlobAlloc ( 674,GHND,1024); 
	hPNAddData = GSSiGlobAlloc ( 675,GHND,1024); 
	ProcessPickedItem (item,FALSE);
	{
		char	GlobFile[MAX_PATH];
		LPSTR	pBS;

		strcpy (GlobFile,PickName);
		ExpandText (GlobFile);
		if ((pBS = strrchr (GlobFile,'\\')))
		{

			*(++pBS) = 0;
			strcat (GlobFile,"global.ini");
			LoadGlobalInit (GlobFile,FALSE);
		}
    }
	
    fgetstring (str,1020,Fid); while (*str<'!') {fgetstring (str,1020,Fid);} 
    rec = -1; 
Top:
    while (rec<irec)
    {   
    	if (!_fstrnicmp (str,"INCLUDE ",8))
    	{  
    		_fstrcpy (pIncludePath,&str[8]);
    		ExpandText (pIncludePath);
    		if (*pIncludePath)
    		{
	    		FidLast = Fid;
				Fid = GSSiOpenFile (pIncludePath,&OFStruct,OF_READ);  
				if (Fid == HFILE_ERROR)  
				{
					Fid = FidLast;
					FidLast = HFILE_ERROR;
				} 
			}
			else
				rec++;
    	}
    	else
    		rec++;
NextLine:
    	if (fgetstring (str,1020,Fid))
    	{
    		if (*str<'!')
    			goto NextLine;
    	}
    	else if (FidLast != HFILE_ERROR)
    	{   
    		GSSiClose2 (&Fid);
    		Fid = FidLast;
    		FidLast = HFILE_ERROR;
			goto NextLine;
    	}
    	else
    	{
    		GSSiClose2 (&Fid);
    		goto RtnFalse;
    	}
    } 
    
ProcessNextLine:
	if (!_fstrnicmp (str,"INCLUDE ",8))
	{  
		_fstrcpy (pIncludePath,&str[8]);
		ExpandText (pIncludePath);
		if (*pIncludePath)
		{
    		FidLast = Fid;
			Fid = GSSiOpenFile (pIncludePath,&OFStruct,OF_READ);  
			if (Fid == HFILE_ERROR)  
			{
				Fid = FidLast;
				FidLast = HFILE_ERROR;
				*str = 0;
			} 
			else
		    	if (!fgetstring (str,1020,Fid))
					*str = 0;
		}
	}
	CurLoc = GSSillseek (Fid,0,1);
	if (!fgetstring (NextLine,1020,Fid))
		NextLine[0]=0;
	GSSiClose2 (&Fid);  
	if (FidLast != HFILE_ERROR)
		GSSiClose2 (&FidLast);
	if (*str=='+')
	{
		lpAction=str;
		while (*(++lpAction)<'!');
	}
	else
	{
		if ((lpDesc= MatchLev(str,',')))
		{
			*lpDesc++=0;
			lpAction = MatchLev (lpDesc,',');
			*lpAction++=0;
		}
		else
			goto RtnFalse;    
	}
	lpNext=_fstrchr(lpAction,','); 
	if (lpNext) *lpNext = 0;    
	
	SetPickGlobals (item);
    
	if (!_fstricmp (lpAction,"HIGHLIGHT")) 
	{    
		     AddToHighlightList (PickList[item].Refno,&PickList[item],TRUE);
			 ShowPickedItem (hWndMain, item);  
    }
    
	else if (!_fstricmp (lpAction,"BASIC")) 
	{    
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in Database parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpDB++=0;
		if (!(lpSQL = _fstrchr(lpDB,',')))
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in SQL parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpSQL++=0;
		lpUpdateFieldList = 0;
		lpAutoUpdateFieldList = 0;
		displayRect.left = displayRect.right = 0;
	    {
	 	   DLGPROC lpfnIDENTIFYMsgProc;
	         
	        BasicDisplayItem=item; 
		    lpfnIDENTIFYMsgProc = MakeProcInstance((DLGPROC)IDENTIFYMsgProc, hInst);
		    nRc = DialogBox(hInst, (LPSTR)"IDENTIFY", hWndMain, lpfnIDENTIFYMsgProc);
		    FreeProcInstance(lpfnIDENTIFYMsgProc);
	
	    } 
    }
	else if (!_fstricmp (lpAction,"GETVALS")) 
	{   
		HANDLE hSQL=0;
		LPSTR	lpCMD;
		 
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in Database parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpDB++=0;
		if (!(lpSQL = _fstrchr(lpDB,',')))
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in SQL parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpSQL++=0;
		if ((lpCMD = _fstrchr(lpSQL,',')))
		{
			*lpCMD++=0;
		    if (!OpenDataFile (lpDB,lpSQL,BT_READ,&hSQL))
				goto RtnFalse;
			ExpandText (lpCMD);  
		    CloseDataFile (TRUE, &hSQL);
		}
    }
	else if (!_fstricmp (lpAction,"REPORT")) 
	{    
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in report name parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpDB++=0;
		if (!(lpSQL = _fstrchr(lpDB,',')))
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in viewport parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpSQL++=0; 
		Report (lpDB, lpSQL,PickList[item].Prefix,PickList[item].UDI,PickList[item].Refno,FALSE,FALSE);
	} 
	else if (!_fstricmp (lpAction,"INFOBOX")) 
	{    
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in infobox pathname parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpDB++=0;
		if (!CreateTAGBoxFromFile (CurView->hDC,PickList[item].PickedPoint,lpDB,item,0))
			goto RtnFalse;
	} 
	else if (!_fstricmp (lpAction,"DIALOG")) 
	{    
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in dialog name parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpDB++=0;
		if (!(lpSQL = _fstrchr(lpDB,',')))
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in SQL parameter", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpSQL++=0; 
		if ((lpInsert = _fstrchr(lpSQL,',')))
		*lpInsert++=0; 
		EditDynamicDialog (CurView->hWnd,lpDB, lpSQL, lpInsert);
	} 
	else if (!_fstricmp (lpAction,"COMMAND")) 
	{    
		lpDB = lpNext;
		if (!lpDB)
		{
	   		 GSSiMsgBox( GetFocus(), str,"No command string", MB_OK|MB_ICONEXCLAMATION,0);
			 goto RtnFalse;
		}
		*lpDB++=0;   
		CreateUndoPoint (lpDesc);
		AddGraphicsCmd (CurView->hWnd,lpDB,FALSE,0); 		
//		ExpandText (lpDB);
	} 
/*	else if (!_fstricmp (lpAction,"CITYTOUR")) 
	{    
		lpLAYER = lpNext; 
		if (!lpLAYER)
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in CITYTOUR data layer parameter", MB_OK|MB_ICONEXCLAMATION);
			 goto RtnFalse;
		} 
		lpLAYER++;
		if (!(lpVIEWPORT = _fstrchr(lpLAYER,',')))
		{
	   		 GSSiMsgBox( GetFocus(), str,"Error in Viewport parameter", MB_OK|MB_ICONEXCLAMATION);
			 goto RtnFalse;
		}
		*lpVIEWPORT++=0;  
		for (Layer=0;Layer<CurView->NumFiles;Layer++)
		{
			if (!_fstricmp (lpLAYER,CurView->FileID[Layer]))
				break;
		}  
 		if (Layer >= CurView->NumFiles)
		{
	   		 GSSiMsgBox( GetFocus(), lpLAYER,"Invalid layer name", MB_OK|MB_ICONEXCLAMATION);
			 goto RtnFalse;
		}
 		StartGEOSPANvp = CurView->ID;
		GEOSPANAtPoint (PickList[item].PickedPoint,Layer,lpVIEWPORT);
    } 
	else if (!_fstricmp (lpAction,"GEONET")) 
	{    
		GEOSPANAtNetwork (&CurPath,PickList[item].Refno,PickList[item].PCT,PickList[item].Length,PickList[item].OffDist,FALSE);
    }  */
    
	else if (!_fstricmp (lpAction,"TIGER")) 
	{    
    	GetPickName (item);
		DisplayTIGERName (PickList[item].Refno);
    }  
	else if (!_fstricmp (lpAction,"PEOPLENET")) 
	{    
		DisplayPNData (PickList[item].Refno,hIntData,hPNAddData);
    }  
	else if (!_fstricmp (lpAction,"NULL")) 
		ii=1;
	
	if (NextLine[0]=='+')
	{
		char	PMFile[MAX_PATH];

		GetPMName(CurView->PickMacroFile,PMFile);
		Fid = GSSiOpenFile (PMFile,&OFStruct,OF_READ);
		GSSillseek (Fid,CurLoc,0);
		fgetstring (str,1020,Fid);
		goto ProcessNextLine;
	} 
	GSSiGlobFree (&hIntData);
	GSSiGlobFree (&hPNAddData);
	GSSiGlobUlFree (&hSTR); 
	CurView = SaveVP; 
{
#if ENABLETRACE
GSSiExitProg (910);
#endif
	return TRUE; 
}

RtnFalse:
	GSSiGlobFree (&hIntData);  
	GSSiGlobFree (&hPNAddData);
	GSSiGlobUlFree (&hSTR);  
{
#if ENABLETRACE
GSSiExitProg (910);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}







BOOL MoveViewport (short NewVPID)
#if ENABLETRACE
{GSSiEnterProg (913);
#endif
{
	UINT	iv;
	short	OldID, FirstRenumberedVP, LastRenumberedVP, VPIDChange;  
	short	NewID[MAX_VIEWPORTS];  
	LPVIEWPORT	SaveVP[MAX_VIEWPORTS];
	HANDLE		SavehVP[MAX_VIEWPORTS];
	
	OldID = CurView->ID;
	if (NewVPID == OldID)
{
#if ENABLETRACE
GSSiExitProg (913);
#endif
		return FALSE;
}
	if (NewVPID < CurView->ID)
	{
		FirstRenumberedVP= NewVPID;
		LastRenumberedVP = CurView->ID-1;
		VPIDChange = 1;
	}
	else
	{
		FirstRenumberedVP= CurView->ID+1;
		LastRenumberedVP = NewVPID;
		VPIDChange = -1;
	}
    for (iv=0;iv<*pNumViewports;iv++)
    {
		if (pViewports[iv]->ID >= FirstRenumberedVP &&
			pViewports[iv]->ID <= LastRenumberedVP)
			NewID[iv] =  pViewports[iv]->ID + VPIDChange;
		else
			NewID[iv] =  pViewports[iv]->ID;   
	}
	NewID[CurView->ID-1] = NewVPID;
    for (iv=0;iv<*pNumViewports;iv++)
    {   
    	pViewports[iv]->ID = NewID[pViewports[iv]->ID-1];
		if (pViewports[iv]->ZoomTarget)
			pViewports[iv]->ZoomTarget = NewID[pViewports[iv]->ZoomTarget-1];  
		if (pViewports[iv]->Parent)
			pViewports[iv]->Parent = NewID[pViewports[iv]->Parent-1];  
		if (pViewports[iv]->BoundsDisplayID)
			pViewports[iv]->BoundsDisplayID = NewID[pViewports[iv]->BoundsDisplayID-1];    
		if (pViewports[iv]->BoundsDisplayVP)
			pViewports[iv]->BoundsDisplayVP = NewID[pViewports[iv]->BoundsDisplayVP-1]; 
		if (pViewports[iv]->LinkedTo)
			pViewports[iv]->LinkedTo = NewID[pViewports[iv]->LinkedTo-1]; 
		if (pViewports[iv]->OnPrintAddSpaceToVP)
			pViewports[iv]->OnPrintAddSpaceToVP = NewID[pViewports[iv]->OnPrintAddSpaceToVP-1];
		if (pViewports[iv]->pTheme)
		{ 
			if (pViewports[iv]->pTheme->TargetViewport) 
				pViewports[iv]->pTheme->TargetViewport = NewID[pViewports[iv]->pTheme->TargetViewport-1];
			if (pViewports[iv]->pTheme->DisplayViewport) 
				pViewports[iv]->pTheme->DisplayViewport = NewID[pViewports[iv]->pTheme->DisplayViewport-1];
		}
    } 
    _fmemmove (SaveVP,pViewports,sizeof(LPVIEWPORT)*MAX_VIEWPORTS);  
    _fmemmove (SavehVP,hViewports,sizeof(HANDLE)*MAX_VIEWPORTS);  
    for (iv=0;iv<*pNumViewports;iv++)
    {
        pViewports[NewID[iv]-1] = SaveVP[iv];
        hViewports[NewID[iv]-1] = SavehVP[iv];
    }  
    *pCommandViewport = NewID[*pCommandViewport-1];
	UpdateInfoBoxVPID (NewID);
{
#if ENABLETRACE
GSSiExitProg (913);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

  

void ValidVP (void)
#if ENABLETRACE
{GSSiEnterProg (915);
#endif
{
	if (!CurView)
		SetViewport(*pCommandViewport);
{
#if ENABLETRACE
GSSiExitProg (915);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}	

BOOL ValidStartupFunction (short Function)
#if ENABLETRACE
{GSSiEnterProg (916);
#endif
{
	switch (Function)
	{
		case 0:
		case GF_THEME_ACTIVATE:
		case GF_THEME_DEACTIVATE:
		case GF_MOVESIZE_VIEWPORT:
{
#if ENABLETRACE
GSSiExitProg (916);
#endif
			return FALSE; 
}
		
		default:
{
#if ENABLETRACE
GSSiExitProg (916);
#endif
		    return TRUE;
}
	}
#if ENABLETRACE
}
#endif
}

void RedisplayWindow (void)
#if ENABLETRACE
{GSSiEnterProg (917);
#endif
{
	setDoPaint(TRUE);
	PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
{
#if ENABLETRACE
GSSiExitProg (917);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}	 

BOOL GetThemeSymbolList (short Parent, HANDLE hDBList)
#if ENABLETRACE
{GSSiEnterProg (918);
#endif
{                             
    LPSYMBOL    pSymbol;
    BOOL	IsPar; 
	char	Value[128], SymName[128];
	short	nItem, nPar=0;	
	HANDLE	hSymbol, hPar = GSSiGlobAlloc ( 676,GMEM_MOVEABLE,4096); 
	LPSHORT	pPar;
    int   i, ClassNo, ClassZero=0; 
    HANDLE      hBT;
    HCURSOR hcurSave;
	LPVISLIST	SaveVis=CurVis;  
	LPVIEWPORT	SaveView=CurView;
	
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	SetCurView ( pViewports[CurTheme->TargetViewport-1]);
	GSSiGlobFree (&CurTheme->hVisList);
	CurTheme->hVisList = SetThemeVisList (Parent);
	GetDictSymbolNumber ("CIRCLE"); // opens %SYM_DICT 
	CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
	for (i=1;i<3201;i++)
	{ 
		if (GetVisibility (i))
		{
			GetSymbolName (i,SymName,0,0,&IsPar); 
			if (!IsPar)
			{
	            _fstrncpy (Value,SymName,(size_t)GetBTKeyLen(hDBList));
	            if (BT_FIND(hDBList,Value,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
	                BT_PUT (hDBList,Value,(LPSTR)&ClassZero);  
	        }
        } 
    } 
	CurVis = SaveVis;  
	SetCurView ( SaveView);
    GSSiGlobUlFree (&CurTheme->hVisList);
    GSSiSetCursor (hcurSave); 
{
#if ENABLETRACE
GSSiExitProg (918);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 
                                       
BOOL DisplayHLTData (LPLONG pStartRef)
#if ENABLETRACE
{GSSiEnterProg (919);
#endif
{
    HIGHLIGHTDATA	HighlightData;
    short	st;
    LPVIEWPORT		SaveVP;  
    HANDLE			hSaveVP;  
    BOOL	AP;
	LPVISLIST	SaveVis=CurVis; 
	HANDLE	hVisList; 
	BOOL	SaveIgnorePrevLayers = IgnorePrevLayers;
    
	if (!hHighlight)
{
#if ENABLETRACE
GSSiExitProg (919);
#endif
		return FALSE;
}
	if (!GetGlobalBVal2 ("[%SHOWSELECTED]",TRUE) && !DisplayOnlyHLT)
{
#if ENABLETRACE
GSSiExitProg (919);
#endif
		return FALSE;
}
	SetGlobalValue ("%SHOWSELECTED","");
	hSaveVP = GSSiGlobAlloc ( 677,GMEM_MOVEABLE,sizeof(VIEWPORT));
	SaveVP = (LPVIEWPORT)GlobalLock (hSaveVP); 
	GSSiDeleteObject (&CurView->hRgn);
	*SaveVP = *CurView;	
	AP = SetAutoPan (FALSE);
	hVisList=GSSiGlobAlloc ( 678,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->WantType[8] = 1;
	ShowValue (CurView->hDC,TRUE);
	IgnorePrevLayers = FALSE;
	OpenPrevLayers (FileNum,LayerID);
//	if (SaveVis)
//    	CurVis->WantType[7] = SaveVis->WantType[7];
	st = BT_FIND (hHighlight,(LPSTR)pStartRef,BT_FIRST,BT_GE,(LPSTR)&HighlightData);
	while (!st && CheckForContinue (TRUE,0))
	{   
		if (CurView->ID == HighlightData.PD.ViewID)// &&
//			((HighlightData.PD.Type == 3 && (CurView->PassID == 2 || !CurView->PassID) && CurVis->WantType[0]) ||
//			 ((HighlightData.PD.Type == 1 || HighlightData.PD.Type == 2 || HighlightData.PD.Type == 4 || HighlightData.PD.Type == 5) && (CurView->PassID == 3 || !CurView->PassID) && (CurVis->WantType[1] || CurVis->WantType[2])) ))
//			 (HighlightData.PD.Type == 4 && (CurView->PassID == 3 || !CurView->PassID) && CurVis->WantType[2]) ))
		{   
			if (hDisplayedHighlightedRefs)
			{
				long	Refno = *pStartRef;
				short	Dummy;

				if (!BT_FIND(hDisplayedHighlightedRefs,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
					goto NextRef;
			}
			if (RectInWBounds (&HighlightData.PD.Rect,0))
			{
				PickList[0]=HighlightData.PD; 
				ShowPickedItem (CurView->hWnd,-1); 
				ShowValue (CurView->hDC,FALSE);
			}
		} 
NextRef:
		st = BT_FIND (hHighlight,(LPSTR)pStartRef,BT_FIRST,BT_GT,(LPSTR)&HighlightData);
	} 
	ShowValue (CurView->hDC,FALSE);
	ClosePrevLayers (); 
	IgnorePrevLayers = SaveIgnorePrevLayers;
	GSSiGlobUlFree (&hVisList);
	CurVis = SaveVis;
	*CurView = *SaveVP;
	SetAutoPan (AP);
	GSSiGlobUlFree (&hSaveVP);
	SetGlobalValue ("%SHOWSELECTED","Y");
{
#if ENABLETRACE
GSSiExitProg (919);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL CreateReorgBackupFile (LPSTR PltName)
#if ENABLETRACE
{GSSiEnterProg (920);
#endif
{   
	char	File[206],dir[256],name[34],ext[8],drive[34],mess[256];
	LPSTR	pBUDir, pChr;
	
	if (!hReorgBUDir)
{
#if ENABLETRACE
GSSiExitProg (920);
#endif
		return TRUE;
}
	_fstrcpy (File,PltName);
	ExpandText (File);   
	_splitpath (File,drive,dir,name,ext);
	pBUDir = GlobalLock (hReorgBUDir);
	pChr = LastChr (drive); 
	if (*pChr == '\\'|| *pChr == ':')
	{
		*pChr-- = 0; 
		if (*pChr == '\\' || *pChr == ':')
			*pChr = 0;  
	}
	sprintf (File,"%s\\%s%s%s%s",pBUDir,drive,dir,name,ext);
	GlobalUnlock (hReorgBUDir);
	if (ExistFile (File))
{
#if ENABLETRACE
GSSiExitProg (920);
#endif
		return TRUE;
}
	if (makedirectories (File,FALSE,FALSE)) 
	{
		if (copyfile (File,PltName,FALSE,0,0,0,0,0,0))
{
#if ENABLETRACE
GSSiExitProg (920);
#endif
			return TRUE; 
}
	}
	sprintf (mess,"Unable to create backup file: %s",File);
	GSSiMsgBox (GetFocus(),mess,0,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (920);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

 

BOOL GetBinFileBounds (LPSTR InFile,LPMNMXCORD lpMinMax)
#if ENABLETRACE
{GSSiEnterProg (922);
#endif
{
    short       NumFiles;
    LPSHORT     lpNumFiles;       
    LPFILELISTENTRY lpEntry; 
    MNMXCORD	FileBounds;  
    char        File[MAX_PATH], drive[8], dir[MAX_PATH];   
    HANDLE		hBinFileList;
            
    _fstrcpy (File,InFile);
    ExpandText (File);
    _splitpath (File,drive,dir,0,0);
    sprintf (File,"%s%sglobal.ini",drive,dir);
	LoadGlobalInit (File,FALSE);
	GetViewportScale (CurView->hDC); 
    sprintf (File,"%s%sfilelist.bin",drive,dir);
    if (!(hBinFileList = LoadBinaryFileList (InFile,File)))
{
#if ENABLETRACE
GSSiExitProg (922);
#endif
    	return FALSE;
}
    lpEntry = (LPFILELISTENTRY)GlobalLock (hBinFileList); 
    FileBounds = lpEntry->MinMax;
    if (AdjustFileBounds (CurView->ID,CurView->CurFile,&FileBounds,0))
        AddMinMaxD (lpMinMax,&FileBounds); 
/*    lpNumFiles = (LPSHORT)lpEntry;
    NumFiles = *lpNumFiles;   
    lpEntry++; 
    while (NumFiles--)
    {
	    FileBounds = lpEntry->MinMax;
	    if (AdjustFileBounds (CurView->ID,CurView->CurFile,&FileBounds,0))
	        AddMinMaxD (lpMinMax,&FileBounds); 
        lpEntry++;            
    }*/
    GSSiGlobUlFree (&hBinFileList);
{
#if ENABLETRACE
GSSiExitProg (922);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

void SetCursorPosGM (int x,int y,BOOL DisplayLinkedOnly)
#if ENABLETRACE
{GSSiEnterProg (923);
#endif
{   
	LPVIEWPORT	SaveVP = CurView; 
	DPOINT	BasePoint, WinBasePoint;
	POINT	WinPoint, CurPoint;
	RECT	Rect, NewRect;
	short	xat,yat; 
	static	short	lastx=SHRT_MAX, lasty=SHRT_MAX;
	short	LinkedCursorWidth=12;    
	HPEN	hPen,hOldPen, hPen2;
	BOOL	AlignWithRoute=FALSE, ShowCurs;

	if (BackgroundTask)
		return;
	IgnoreMouseMove=TRUE;
	if (!DisplayLinkedOnly)
	{   
		SetCursor (hCursor);
		SetCursorPos (x,y);
		SetCursor (hCursor); 
		ddbug=TRUE;
	}
	RemoveLinkedCursors ();
	if (!CurView || !DisplayLinkedCursors)
{
#if ENABLETRACE
GSSiExitProg (923);
#endif
		return;                
}
	WinPoint.x = x;
	WinPoint.y = y;  
	ScreenToClient (CurView->hWnd,&WinPoint);
	if (!PtInRect (&CurView->ScreenRect,WinPoint))
{
#if ENABLETRACE
GSSiExitProg (923);
#endif
		return;
}
	CurView->LastCursorPos = WinPoint;
	CurPoint = WinPoint;
	BasePoint = ScreenPtToBasePt (WinPoint);  
	DisplayProfileLink (&BasePoint);
	if (!CurView->LinkedTo)
{
#if ENABLETRACE
GSSiExitProg (923);
#endif
		return; 
}
	if (TranFilePoint (1,&BasePoint) == 2)
{
#if ENABLETRACE
GSSiExitProg (923);
#endif
		return; 
}
	
    InLinkedList(-1);
    InLinkedList(CurView->ID);
    
	SaveDC (CurView->hDC);
  	SetDisplayMode (CurView->hDC, GF_SCREENMODE);
  	SelectClipRgn (CurView->hDC,0);   
    while (CurView->LinkedTo && !InLinkedList(CurView->LinkedTo))
    {   
		SetViewport (CurView->LinkedTo); 
		if (!CurViewActive())
			goto NextVP;
		WinBasePoint = BasePoint; 
		if (SaveVP->hProfileRoute[0])
		{   
			DisplayProfileLoc (&CurPoint,&BasePoint, SaveVP,FALSE);
			if (SaveVP->ProfileInCrossSection)
		   		WinBasePoint = PointAtDistOnPoly (SaveVP->ProfileCrossSection,2,BasePoint.x,0,0);  
			else
			{
				HPDPOINT	pRoute=(HPDPOINT)GlobalLock (SaveVP->hProfileRoute[0]);
				
/*				if (SaveVP->pTheme && SaveVP->pTheme->ProfileAlignmentOption)
					WinBasePoint = PointAtScreenXOnPoly(pRoute, SaveVP->nProfileRoute[0], BasePoint.x,&SaveVP->LastProfileAZ);
				else*/
		   			WinBasePoint = PointAtDistOnPoly (pRoute,SaveVP->nProfileRoute[0],BasePoint.x,&SaveVP->LastProfileAZ,0);
		   		SaveVP->LastProfileDist = BasePoint.x;  
		   		SaveVP->LastProfilePoint = WinBasePoint;
		   		GlobalUnlock (SaveVP->hProfileRoute[0]); 
		   	} 
		}
		else
		{
			if (TranFilePoint (2,&WinBasePoint) == 2)
				goto NextVP;
		}
		ShowCurs = DisplayProfileLink (&WinBasePoint);
		WinPoint = BasePtToScreenPt (&WinBasePoint); 
		Rect.left = max (0,WinPoint.x-LinkedCursorWidth-3);
		Rect.right = WinPoint.x+LinkedCursorWidth+3;
		Rect.top = max(0,WinPoint.y-LinkedCursorWidth-3);
		Rect.bottom = WinPoint.y+LinkedCursorWidth+3;
		if (ShowCurs && IntersectRect (&NewRect,&Rect,&CurView->ScreenRect))
		{   
			CurView->LastCursorPos = WinPoint;
			CurView->LinkedCursorHandle = SaveScreen2 (CurView->hWnd,CurView->hDC,Rect,CurView,0);
		    hPen = CreatePen(PS_SOLID,5,0);
		    hOldPen = SelectObject (CurView->hDC,hPen);
			MoveToEx (CurView->hDC,WinPoint.x,WinPoint.y-LinkedCursorWidth,0);
			LineTo (CurView->hDC,WinPoint.x,WinPoint.y+LinkedCursorWidth);
			MoveToEx (CurView->hDC,WinPoint.x-LinkedCursorWidth,WinPoint.y,0);
			LineTo (CurView->hDC,WinPoint.x+LinkedCursorWidth,WinPoint.y);
		    hPen2 = CreatePen(PS_SOLID,3,RGB(255,255,0));
		    SelectObject (CurView->hDC,hPen2);
			DeleteObject (hPen);
			MoveToEx (CurView->hDC,WinPoint.x,WinPoint.y-LinkedCursorWidth,0);
			LineTo (CurView->hDC,WinPoint.x,WinPoint.y+LinkedCursorWidth);
			MoveToEx (CurView->hDC,WinPoint.x-LinkedCursorWidth,WinPoint.y,0);
			LineTo (CurView->hDC,WinPoint.x+LinkedCursorWidth,WinPoint.y);
			if (hOldPen)
				SelectObject (CurView->hDC,hOldPen);
			DeleteObject (hPen2);
		}
NextVP:;
	}
	RestoreDC (CurView->hDC,-1); 
Exit:
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (923);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void RemoveLinkedCursors (void)
#if ENABLETRACE
{GSSiEnterProg (924);
#endif
{   
	LPVIEWPORT	SaveVP=CurView;
	short	iview;
	
    for (iview = 0;iview < *pNumViewports; iview++)
    {
		SetCurView ( pViewports[iview]); 
		RestoreScreen2 (CurView->hDC, CurView->LinkedCursorHandle,0,FALSE);
	    DestroySavedScreen (&CurView->LinkedCursorHandle,0);
	}
    SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (924);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayTransparency (void)
{
    BLENDFUNCTION bf;
	BOOL	rtn=TRUE;
	static	BOOL blt=FALSE;

	if (CurView->transParency && CurView->hTransparencyBitmap)
	{
		HDC hDC = CurView->originalDC;//GetDC (CurView->hWnd);
		int w=RECTWIDTH(&CurView->DrawRect);
		int h=RECTHEIGHT(&CurView->DrawRect);
		BITMAP bm;

		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = (0xff * CurView->transParency) / 100;  // half of 0xff = 50% transparency 
		bf.AlphaFormat = AC_SRC_ALPHA;   // use source alpha  
		SaveDC (hDC);
		SetGraphicsMode(hDC, GM_COMPATIBLE);
     	SetMapMode     (hDC, MM_TEXT );
		SetWindowOrgEx  ( hDC, 0, 0,0 );
		SetViewportOrgEx( hDC, 0, 0,0 );    
	  	SelectClipRgn ( hDC,0);
		SaveDC (CurView->hTransparentDC);
		SetGraphicsMode(CurView->hTransparentDC, GM_COMPATIBLE);
     	SetMapMode     (CurView->hTransparentDC, MM_TEXT );
		SetWindowOrgEx  (CurView->hTransparentDC, 0, 0,0 );
		SetViewportOrgEx(CurView->hTransparentDC, 0, 0,0 );    
	  	SelectClipRgn (CurView->hTransparentDC,CurView->hTransparentNullRgn);
		GetObject (CurView->hTransparencyBitmap,sizeof(bm), (LPSTR)&bm);
		if (bm.bmBitsPixel == 32)
		{
			unsigned int irow, icol;
			int lenBits = bm.bmWidthBytes * bm.bmHeight;
			LPBYTE	pBits = malloc (lenBits);
			GetBitmapBits (CurView->hTransparencyBitmap,lenBits,pBits);
			for (irow=max(0,CurView->DrawRect.top);irow<min(CurView->DrawRect.bottom,bm.bmHeight);irow++)
			{
				LPRGBQUAD pclr=(LPRGBQUAD)(pBits+bm.bmWidthBytes*irow);
				int firstCol = max(0,CurView->DrawRect.left);
				
				pclr += firstCol;

				for (icol=firstCol;icol<min(bm.bmWidth,CurView->DrawRect.right);icol++)
				{
					if (pclr->rgbBlue == 0 && pclr->rgbGreen == 0 && pclr->rgbRed == 0)
						pclr->rgbReserved = 0;
					else
						pclr->rgbReserved = 255;
					pclr++;
				}
			}
			SetBitmapBits (CurView->hTransparencyBitmap,lenBits,pBits);
			free (pBits);
		}
		if (blt)
		{
			if (!BitBlt (hDC,CurView->DrawRect.left,CurView->DrawRect.top,w,h, 
							CurView->hTransparentDC, CurView->DrawRect.left,CurView->DrawRect.top, SRCCOPY))
							ii = GetLastError ();
		}
		else
		{
			RECT	rect, intRect;
			GetClientRect (CurView->hWnd,&rect);
			IntersectRect (&intRect,&rect,&CurView->DrawRect);
			w = RECTWIDTH (&intRect);
			h = RECTHEIGHT (&intRect);
			/*{
				HBITMAP hbm = SaveScreen (hDC,CurView->DrawRect);
				SaveBitmap (hbm,"c:\\temp\\blendpre.bmp",0,0);
				GSSiDeleteObject (&hbm);
			}*/
			if (!AlphaBlend(hDC,CurView->DrawRect.left,CurView->DrawRect.top,w,h, 
							CurView->hTransparentDC,
							CurView->DrawRect.left,CurView->DrawRect.top,
							w,h,
							//CurView->transparencyBitmapWidth,CurView->transparencyBitmapHeight,
							bf))
			{
				int er = GetLastError ();
				char message[256];
				GetSystemErrMessage (er,message);
				rtn = FALSE;
			}
		}
/*			{
				HBITMAP hbm = SaveScreen (hDC,CurView->DrawRect);
				SaveBitmap (hbm,"c:\\temp\\blendpost.bmp",0,0);
				GSSiDeleteObject (&hbm);
			}*/
		RestoreDC (hDC,-1);
		RestoreDC (CurView->hTransparentDC,-1);
		GdiFlush ();
		//ReleaseDC (CurView->hWnd,hDC);
	}
	return TRUE;
}

BOOL SetTransparency (int tranValue)
{
	if (!CurView)
		return FALSE;
	if (tranValue < 0)
	{
		if (CurView->hTransparencyBitmap)
		{
			SelectObject (CurView->hTransparentDC,CurView->origTransparencyBitmap);
			DeleteObject (CurView->hTransparencyBitmap);
			CurView->hDC = CurView->originalDC;
			DeleteDC (CurView->hTransparentDC);
			GSSiDeleteObject(&CurView->hTransparentNullRgn);
			CurView->hTransparencyBitmap = 0;
			CurView->hTransparentDC = 0;
		}
		CurView->transParency = 0;
		return TRUE;
	}
	if (tranValue > 100)
		return FALSE;
	DisplayTransparency ();
	SetTransparency (-1);
	if (tranValue)
	{
		RECT rect;
		HDC hDC = GetDC (CurView->hWnd);
		BITMAP bm;

		if (hDC)
		{
			GetClientRect (CurView->hWnd,&rect);  
			curProgID = 10013;
			CurView->transparencyBitmapWidth = RECTWIDTH(&rect);
			CurView->transparencyBitmapHeight = RECTHEIGHT(&rect);
			CurView->hTransparencyBitmap = CreateCompatibleBitmap (hDC,CurView->transparencyBitmapWidth,CurView->transparencyBitmapHeight);
			curProgID = -1;
			if (CurView->hTransparencyBitmap)
			{
				CurView->transParency = tranValue;
				CurView->hTransparentDC = CreateCompatibleDC (hDC);
				CurView->hTransparentNullRgn = CreateRectRgn (rect.left,rect.top,rect.right,rect.bottom);
				CurView->originalDC = CurView->hDC;
				GetObject (CurView->hTransparencyBitmap,sizeof(bm), (LPSTR)&bm);
				CurView->origTransparencyBitmap = SelectObject (CurView->hTransparentDC,CurView->hTransparencyBitmap);
				CurView->hDC = CurView->hTransparentDC;
				FillRect (CurView->hDC,&CurView->DrawRect,GetStockObject (BLACK_BRUSH));
			}
			ReleaseDC (CurView->hWnd,hDC);
		}
	}
	return TRUE;
}

int SelectVPClipRgn (HRGN hrgn)
{
	if (!CurView)
		return 0;
	if (CurView->hDC == CurView->hTransparentDC)
		return SIMPLEREGION;
	return SelectClipRgn (CurView->hDC,hrgn);
}


	
