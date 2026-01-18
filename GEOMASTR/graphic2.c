#include "graphint.h"
#include "extrndb.h" 

#include "gmextern.h"

static	double	RoundTo[8]={0,0.1,0.01,0.001,0.0001,0.00001,0.000001,0.0000001}; 
static BOOL	TrackOutsideVP=FALSE;
static BOOL	HltDupRefs=FALSE;
static BOOL	MoveShapePoints=FALSE;
static LPVOID	PassiveFunPTR[32];
static char	HighlightFile[MAX_PATH]="",HighlightFile2[MAX_PATH]="";  
static DPOINT	LastDPoint;
static POINT	LastPoint;
#define MAX_PICK_BOX 32
#define PICK_BOX_MACRO_LEN 512
static int		numPickBoxes=0;
static RECT		pickBoxRect[MAX_PICK_BOX];
static int		pickBoxVPID[MAX_PICK_BOX];
static char		pickBoxMacro[MAX_PICK_BOX][PICK_BOX_MACRO_LEN];

BOOL PickBoxAdd(int VPID, RECT rect, LPSTR macro)
{
	BOOL rtn = FALSE;

	if (numPickBoxes < MAX_PICK_BOX)
	{
		rtn = TRUE;
		pickBoxRect[numPickBoxes] = rect;
		pickBoxVPID[numPickBoxes] = VPID;
		strncpy(pickBoxMacro[numPickBoxes++], macro, PICK_BOX_MACRO_LEN);
	}
	return rtn;
}

void PickBoxesDestroy(VPID)
{
	numPickBoxes = 0;
}

BOOL ProcessPickBoxes (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (113);
#endif
{   int	i, ID, InfoBoxID;
	short	VPID;
	POINT	CursorPoint, CurrentPos;
	LPVIEWPORT	SaveView = CurView;
	static	HANDLE	hLastBox = 0;
	char	str[256];
	BOOL	rtn = FALSE;

	if (!CurrentConfig || !HavePaint || CursorIsLocked)
		goto Exit;
	CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
	if (Message == WM_LBUTTONDOWN)
	{
		for (i = 0; i < numPickBoxes; i++)
		{
			if (PtInRect(&pickBoxRect[i], CursorPoint))
			{
				ProcessText(pickBoxMacro[i]);
				rtn = TRUE;
			}
		}
	}
	Exit:
	CurView = SaveView;
{
#if ENABLETRACE
	GSSiExitProg(113);
#endif
	return rtn;
}

#if ENABLETRACE
}
#endif
}


BOOL ContinuePicking (BOOL QuitOnMMove)
#if ENABLETRACE
{GSSiEnterProg (697);
#endif
{
	MSG     msg = { 0 };
	BOOL	IsAccel;  
	short	ii;
	
	if (!QuitOnMMove)
{
#if ENABLETRACE
GSSiExitProg (697);
#endif
		return TRUE;
}
	while (GSSiPeekMessage(&msg,0,WM_MOUSEMOVE,WM_MOUSEMOVE,PM_REMOVE))
	{
#if ENABLETRACE
		SetLastMessage(-1 * (long)msg.message, msg.wParam);
#endif
		
	    if (msg.message == WM_MOUSEMOVE)
	    {
	    	POINT	MovePoint = POINTStoPOINT(MAKEPOINTS (msg.lParam)); 
    		if (idist (MovePoint,LastMovePoint)<6)
    			goto RtnTrue;
	    	goto RtnFalse;              
	    }
		if (!(IsAccel=TranslateAccelerator(hWndMain, hAccelTable, &msg)))
        {
	        TranslateMessage(&msg);
	        DispatchMessage(&msg);
	    } 
		if (IsAccel || msg.message == WM_QUIT || msg.message == WM_COMMAND)
		{
		    if (IsAccel)
		    	PostMessage (msg.hwnd,msg.message,msg.wParam,msg.lParam);
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (697);
#endif
			return FALSE;
}
		}
	}
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (697);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void DisplayFramePoints(LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (103);
#endif
{   
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE	hDB, hBT;
	long	Offset, Frame;
	int		len, st, w;
	struct {
			int		product;
			long	X,Y;
			int		speed,
					azimuth;
			} data; 
	HBRUSH	BkBrush, ScatterBrush;
	HPEN	DotPen, OldPen;
	POINT	WinPoint;
	DPOINT	BasePoint;


	w=2;
	if (Printing) w=6;
	DotPen = CreatePen (PS_SOLID,w,CurTheme->ScatterColor);
	OldPen = SelectObject (CurView->hDC,DotPen);
			
	hDB = OpenGWDatabase (Name,BT_READ);
    if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (103);
#endif
    	return ;
}
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	hBT = lpGWDHead->BTHandle[0];
	st = BT_FIND (hBT,(LPSTR)&Frame,BT_FIRST,BT_ANY, (LPSTR)&Offset);
    while (!st)
    {
    	GSSillseek (lpGWDHead->Fid,Offset,0);
       	BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
       	BigRead (lpGWDHead->Fid,(HPSTR)&data,len);
       	BasePoint.x = data.X;
       	BasePoint.y = data.Y;
       	if (PtInWBounds(&BasePoint))
       	{
	       	WinPoint = BasePtToWinPt (&BasePoint);
			MoveToEx (CurView->hDC,WinPoint.x,WinPoint.y,0);
			LineTo (CurView->hDC,WinPoint.x,WinPoint.y); 
		}
		st = BT_FIND (hBT,(LPSTR)&Frame,BT_NEXT,BT_ANY, (LPSTR)&Offset);
    }
	SelectObject (CurView->hDC,OldPen);
	DeleteObject (DotPen);
    GlobalUnlock (hDB);
    CloseGWDatabase (hDB);
			
{
#if ENABLETRACE
GSSiExitProg (103);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}    



LPTHEME	AddTheme (int ThemeID)
#if ENABLETRACE
{GSSiEnterProg (105);
#endif
{	HANDLE handle;
	LPTHEME	pTheme;

	if (ThemeID == GF_SAVEPOLY_THEME)
		DestroySavedPolys ();
	handle = GSSiGlobAlloc (  56,GHND,sizeof(THEME));
	pTheme = (LPTHEME) GlobalLock (handle);
	pTheme->ID = ThemeID;
	pTheme->IsActive = TRUE;
	pTheme->VPDisplayed = TRUE;
	pTheme->handle = handle;
	AddThemeToVP (CurView,pTheme);
{
#if ENABLETRACE
GSSiExitProg (105);
#endif
	return (pTheme);
}
#if ENABLETRACE
}
#endif
}

void DeleteTheme (LPTHEME pTheme)
#if ENABLETRACE
{GSSiEnterProg (106);
#endif
{	HANDLE handle;

	handle = pTheme->handle;
	CurView->NumThemes--;
	GSSiGlobUlFree (&handle);
{
#if ENABLETRACE
GSSiExitProg (106);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ClosePassiveFunctions (void)
{
	UINT i;
	LPCOORDINATEDISPLAY	CD;
	int	ID;
	
	for (i=0;i<NumPassiveFun;i++)
	{
		CurPassiveFun = PassiveFunPTR[i];
		ID = *(LPSHORT) CurPassiveFun;

		switch (ID)
		   {
		   	case PF_COORD_DISPLAY:
				CD = CurPassiveFun; 
				DTMClose (&CD->hSurf);
		   		break;
		   }
	}
	return;
}

void AddPassiveFun (LPVOID Ptr)
#if ENABLETRACE
{GSSiEnterProg (112);
#endif
{
	PassiveFunPTR[NumPassiveFun]=Ptr;
	NumPassiveFun++;
{
#if ENABLETRACE
GSSiExitProg (112);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


void SetConfigFromCursor (POINT CursorPoint)
{
    
    if (!NumViewportsArray[0])
    	return; 
    if (InDisplayProcessing || idTimer)
    	return;
	if (PtInRect (&ClientRect,CursorPoint))
	{
		if (PtInRect (&MainClipRect,CursorPoint))
		{
			if (!CurrentConfig)
				SetConfig (1);
		}
		else
		{
			if (CurrentConfig)
				SetConfig (0);
		}
	}
	else
	{
		if (!CurrentConfig)
			SetConfig (1);
	}
	return;
}
         
BOOL ProcessPassiveFunctions (HWND hWnd,UINT Message, WPARAM wParam,LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (113);
#endif
{   int	i, ID;
	POINT	CursorPoint, CurrentPos;
	LPVIEWPORT	SaveView=CurView; 
	int		DataRectID;
	BOOL	rtn=FALSE;
	BOOL	CordDispOnly=FALSE;
	static	int	lastMessage;
	static	WPARAM	lastwParam;
	static	lastlParam;


	if (Message == GF_REDISPLAYCOORD)
	{
		Message = lastMessage;
		wParam  = lastwParam;
		lParam  = lastlParam;
		CordDispOnly = TRUE;
	}
	else if (Message == WM_MOUSEMOVE)
	{
		lastMessage = Message;
		lastwParam  = wParam;
		lastlParam  = lParam;
		MovePromptMessage (hWnd,lParam);
	}
   	if (!HavePaint || InDisplayProcessing)
		goto Exit;
    if (Message == WM_MOUSEMOVE && CursorIsLocked)
    {   
    	if (GetFocus() == hWndMain)   
    	SetViewport (LockedCursorVP+1000);
        if (CurViewActive())
        {   
		    CursorPoint = BasePtToScreenPt (&CurrentPoint);
		    if (!PtInRect (&CurView->ScreenRect,CursorPoint))
				ZoomToPointAndDist (CurrentPoint, 0,FALSE);
		    else
		    { 
	            ClientToScreen (CurView->hWnd,(LPPOINT)&CursorPoint); 
	            GetCursorPos (&CurrentPos); 
	            if (CurrentPos.x != CursorPoint.x ||
	            	CurrentPos.y != CursorPoint.y) 
			    	SetCursorPosGM (CursorPoint.x,CursorPoint.y,0); 
			}
		}
		goto Exit;
    }
    
    if (!DisableMarginPan && !idTimer)
	for (i=0;i<NumPassiveFun;i++)
	{
		CurPassiveFun = PassiveFunPTR[i];
		ID = *(LPSHORT) CurPassiveFun;

		switch (ID)
		   {
		   	case PF_COORD_DISPLAY:
		   		DisplayCursorCoordinate (hWnd,Message,wParam,lParam);
		   		break;
		   }
	}
    if (!CordDispOnly && Message == WM_MOUSEMOVE && !DisableMarginPan && !idTimer && !hEnlargedScreen)
    {   
    	short	iview, InfoBoxID, VPID, endvp=0;
		
    	CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		SetConfigFromCursor (CursorPoint);
		RemoveLinkedCursors ();
	    SaveView = CurView; 
	    for (iview = 0; iview<*pNumViewports; iview++)  
	    {
	    	SetCurView ( pViewportsD[iview]);
	    	if (CurView->DisplayedFullScreen)
	    	{   
	    		endvp = iview; 
	    		iview++;
	    		break;
	    	}
	    } 
		if ((InfoBoxID = PtInInfoBoxRect (CursorPoint,&VPID)))
		{ 
			if (SelectTAG(InfoBoxID-1))   
			{
				if (TAGBox.Version > 2)
				{
					RECT	CloseRect=TAGBox.rect;

					SetViewport (VPID);
					CloseRect.top += 1;
					CloseRect.right -= 1;
					CloseRect.bottom = CloseRect.top + 15;  
					CloseRect.left = CloseRect.right - 15;
					if (PtInRect (&CloseRect,CursorPoint))
					{
				  		SelectClipRgn (CurView->hDC,0);
						InvertRect (CurView->hDC,&CloseRect);
					}
					CurView = SaveView;
				}
			}

			if (InfoBoxEditTimer)
				KillTimer (hWndMain,InfoBoxEditTimer);
			if (GetGlobalBVal2 ("[%AUTOEDITINFOBOX]",TRUE))
			{
				InfoBoxEditTimer = INFOBOXEDITTIMERID;  
				EditInfoBox = InfoBoxID;
				SetTimer(hWndMain, INFOBOXEDITTIMERID, (UINT)GetGlobalLVal2 ("[%AUTOPICKDELAY]",500), (TIMERPROC) 0);  
			}
			else 
			{
				SetPrompt (PRMT_INFOBOXEDIT,FALSE);    
				SetViewport (VPID); 
				goto Exit;
			}
		}
		else if (EditInfoBox)
		{
           	 KillTimer(hWndMain,INFOBOXEDITTIMERID);    
			 InfoBoxEditTimer = 0;  
           	 EditInfoBox = 0;
        }
	    while (iview-- > endvp)
	    {   SetCurView ( pViewportsD[iview]); 
	        if (!CurView->DisplayInParent && CurViewActive())
	        	if (PtInRect (&CurView->Rect,CursorPoint))
	        	{   
					int		ii;
	        		
					if (PtInRect (&CurView->ScreenRect,CursorPoint))
					{
						if (LastVP == CurView)
							ii=1;
						else if (LastVP && LastVP != CurView)
						{
							DisplayProfileLoc (0,0,0,FALSE);
						    NotifyFunction (LastVP,GF_EXIT_VIEWPORT);  
							LastVP = SetLastVP (CurView);
							NotifyFunction (LastVP,GF_ENTER_VIEWPORT);
							//BackgroundUpdateMessage ("!REDISPLAY!");
						}   
						else
						{
							LastVP = SetLastVP (CurView); 
							NotifyFunction (LastVP,GF_ENTER_VIEWPORT); 
						}
                        if (CurView->hProfileRoute[0])
                        {   
                        	if (CurView->ProfileInCrossSection)
								SetPrompt (PRMT_PROFILE2,FALSE);
                        	else
								SetPrompt (PRMT_PROFILE1,FALSE);
						}    
						ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
						SetCursorPosGM  (CursorPoint.x,CursorPoint.y,TRUE);
						DataRectID = PtInDataDisplayRect (CursorPoint);
						if (DataRectID != CurDataRectID)
						{
							if (CurDataRectID > -1)
							{
								InvertDataDisplayRect (CurDataRectID,CurDataRectVP,FALSE);
							}
							if (DataRectID > -1)
							{
								InvertDataDisplayRect (DataRectID,CurView,TRUE);
							}
							CurDataRectID = DataRectID;
							CurDataRectVP = CurView;
						}
//						SetFocus (hWnd);
					}
					else if (CurView->MarginPan) 
					{   
					    NotifyFunction (LastVP,GF_EXIT_VIEWPORT);  
						SetPrompt (PRMT_PANPOINT,FALSE);    
				        GSSiSetCursor (DefaultCursor);  
					 	SetCurView (0);   
						LastVP = 0;  
						rtn = TRUE;
					}
					goto Exit;
				}
	    }
	    NotifyFunction (LastVP,GF_EXIT_VIEWPORT);  
	    LastVP = 0;
		SetPrompt (0,FALSE);
	    SetCurView ( 0); 
		RemoveLinkedCursors ();
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (113);
#endif
	return rtn;
}

#if ENABLETRACE
}
#endif
}

BOOL SelectZoomMacro (HWND hWnd)
{
	DLGPROC lpfnSELECTITEMSMsgProc;
	BOOL 	rtn;

	lpfnSELECTITEMSMsgProc = MakeProcInstance((DLGPROC)SELECTAVMsgProc, hInst);
	rtn = DialogBox(hInst, (LPSTR)"SELECTONEITEM",hWnd, (DLGPROC)lpfnSELECTITEMSMsgProc);
	FreeProcInstance(lpfnSELECTITEMSMsgProc); 
	
	return rtn;
}
         
BOOL ProcessCloseIcon (HWND hWnd,UINT Message, WPARAM wParam,LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (113);
#endif
{   int	i, ID,InfoBoxID;
	short	VPID;
	POINT	CursorPoint, CurrentPos;
	LPVIEWPORT	SaveView=CurView;   
	static	HANDLE	hLastBox=0;
	char	str[256];
	BOOL	rtn=FALSE;

   	if (!CurrentConfig || !HavePaint || CursorIsLocked)
		goto Exit;
   	CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
    if (Message == WM_LBUTTONDOWN &&
		(InfoBoxID = PtInInfoBoxRect (CursorPoint,&VPID)))
	{ 
		if (SelectTAG(InfoBoxID-1))   
		{
			RECT	CloseRect=TAGBox.rect;

			SetViewport(VPID);
			CloseRect.top += 1;
			CloseRect.right -= 1;
			CloseRect.bottom = CloseRect.top + 15;  
			CloseRect.left = CloseRect.right - 15;
			if (PtInRect (&CloseRect,CursorPoint))
			{
				TAGBox.ViewportID = -TAGBox.ViewportID;
				SaveTAG(InfoBoxID);
				RedisplayViewport(FALSE,FALSE); 
				rtn = TRUE;
				goto Exit;
			}
		}
	}

    if ((Message == WM_LBUTTONDOWN ||
		 Message == WM_LBUTTONUP ||
		 Message == WM_RBUTTONDOWN ||
		 Message == WM_MOUSEMOVE||
		 Message == WM_F1DOWN) && !DisableMarginPan && !hEnlargedScreen)
    {   
    	short	iview, InfoBoxID, VPID, endvp=0;
		
		if (!InDisplayProcessing && CurView &&  hLastBox)
		{
			RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
			DestroySavedScreen (&hLastBox,0);
		}
    	CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		//SetConfigFromCursor (CursorPoint);
	    for (iview = 0; iview<*pNumViewports; iview++)  
	    {
	    	SetCurView ( pViewportsD[iview]);
	    	if (CurView->DisplayedFullScreen)
	    	{   
	    		endvp = iview; 
	    		iview++;
	    		break;
	    	}
	    } 
	    while (iview-- > endvp)
	    {   SetCurView ( pViewportsD[iview]); 
	        if (!CurView->DisplayInParent && CurViewActive()) 
	        {
	        	if (CurView->CloseIcon)
	        	{
		        	if (PtInRect (&CurView->CloseIconRect,CursorPoint))
		        	{   
						InPan = FALSE;
		        		if (Message == WM_LBUTTONUP)
		        		{
							if (CurView->pTheme && CurView->pTheme->ID == PF_COORD_DISPLAY)
							{
								LPCOORDINATEDISPLAY pCD = (LPCOORDINATEDISPLAY)CurView->pTheme;
								if (pCD->active == 1)
									pCD->active = 0;
								else
									pCD->active = 1;
								CurView = SaveView;
								IgnoreLbutton = TRUE;
								DisplayCloseIcon();
							}
							else
							{
								CurView->Active = FALSE;
								PickBoxesDestroy(CurView->ID);
								ProcessText(CurView->VPCloseCmd);
								if (CurView->DisplayedFullScreen)
									MakeVPFullScreen (CurView->ID,0);
								CurView = SaveView;
								HaltMapDisplay(FALSE,FALSE); 
								IgnoreLbutton = TRUE;
								setDoPaint(TRUE);
								PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
							}
						}
						else
						{
					        SetCurs (LoadCursor (hInst,IDC_ARROW),FALSE);
						}
						if (CurView->pTheme)
						{
							if (CurView->pTheme->ID == PF_COORD_DISPLAY)
								DisplayCloseIcon();
						}
						CurView = SaveView;
						rtn = TRUE;
						goto Exit;

		        	}
		        }
	        	if (CurView->MinMaxIcon)
	        	{
		        	if (PtInRect (&CurView->MinMaxIconRect,CursorPoint))
		        	{   
						InPan = FALSE;
		        		if (Message == WM_LBUTTONUP)
		        		{
							MakeVPFullScreen (CurView->ID,!CurView->DisplayedFullScreen);
						    CurView = SaveView;
							HaltMapDisplay(FALSE,FALSE); 
							IgnoreLbutton = TRUE;
							setDoPaint(TRUE);
							PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L); 
						}
						else
						{
					        SetCurs (LoadCursor (hInst,IDC_ARROW),FALSE);
						}
						CurView = SaveView;
						rtn = TRUE;
						goto Exit;
		        	}
		        }
	        	if (CurView->AutoVisIcon)
	        	{

		        	if (PtInRect (&CurView->AutoVisIconRect,CursorPoint))
		        	{   
						if (LastVP)
			    			NotifyFunction (LastVP,GF_EXIT_VIEWPORT);
		        		if (Message == WM_LBUTTONDOWN)
		        		{   
							RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
							DestroySavedScreen (&hLastBox,0);
			        		CurView->DisableZoomMacro = !CurView->DisableZoomMacro; 
			        		DisplayCloseIcon ();
							if (!CurView->DisableZoomMacro)
							{
								setDoPaint(TRUE);
								PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
							}
						    CurView = SaveView;
							HaltMapDisplay(FALSE, FALSE);
							IgnoreLbutton = TRUE;  
						}
		        		else if (Message == WM_RBUTTONDOWN) 
		        		{   
							RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
							DestroySavedScreen (&hLastBox,0);
							HaltMapDisplay(FALSE,TRUE); 
		        			if (SelectZoomMacro (hWndMain))
		        			{
		        				CurView->DisableZoomMacro = FALSE;
								setDoPaint(TRUE);
								PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L); 
							}
		        		}
						else if (!InDisplayProcessing)
						{
							char	AVName[64],OnOff[2][4]={"On","Off"};
					    	POINT	ButtonPoint = POINTStoPOINT(MAKEPOINTS(lParam));
 
					    	LPSTR	pBS; 
					    	
		        			HelpID = IDH_AUTOVISIBILITY;
					        SetCurs (LoadCursor (hInst,IDC_ARROW),TRUE);
							_fstrcpy (str,CurView->VisName);
							ExpandText (str);
							if ((pBS = _fstrrchr (str,'\\')))
								*pBS++ = 0;
							else
								pBS = str;
							_fstrcpy(AVName,pBS);   
							if ((pBS = _fstrrchr (AVName,'.')))
								*pBS = 0;  
							sprintf (str,"Left click to turn %s - Right click to select a different autovisibility file",OnOff[!CurView->DisableZoomMacro]);   
							SetSysMess (str);
							ButtonPoint.x = CurView->AutoVisIconRect.right;
							ButtonPoint.y = CurView->AutoVisIconRect.bottom + 18;
//							hLastBox = YellowTextBox (CurView->hWnd,str,ButtonPoint,0,0,FALSE,0);
						}
						CurView = SaveView;
						rtn = TRUE;
						goto Exit;
		        	}
		        }
		    }
	    }   
	}
Exit:
	CurView = SaveView;
{
#if ENABLETRACE
GSSiExitProg (113);
#endif
	return rtn;
}

#if ENABLETRACE
}
#endif
}

void DisplayCursorCoordinate (HWND hWnd,UINT Message, WPARAM wParam,LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (114);
#endif
{	
	POINT	CursorPoint;
	DPOINT	BasePoint;
	LPVIEWPORT	SaveView; 
	BOOL	HaveVP = FALSE;
	short	iview;
	LPCOORDINATEDISPLAY	CD;
	LPVIEWPORT	DisplayView; 
	
	if (!CurrentConfig || !CurPassiveFun ||InDisplayProcessing)
{
#if ENABLETRACE
GSSiExitProg (114);
#endif
		return;
}
	CD = CurPassiveFun; 
	if (CD->ID != PF_COORD_DISPLAY)
{
#if ENABLETRACE
GSSiExitProg (114);
#endif
		return;
}
	DisplayView = pViewports[CD->DisplayViewport-1];
	if (!DisplayView || !DisplayView->Active)
{
#if ENABLETRACE
GSSiExitProg (114);
#endif
		return;
}
	if (!CD->active)
	{
		//CD->active = -1;
		SaveView = CurView;
		SetCurView(DisplayView);
		HDC hDC = GetDC(DisplayView->hWnd);
		SaveDC(hDC);
		SetDisplayMode(hDC, GF_TEXTMODE);
		GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE, FALSE);
		SelectClipRgn(hDC, CurView->hRgn);
		GSSiDeleteObject(&CurView->hRgn);

		FillRectColor(CurView->hDC, &CurView->ScreenRect, CurView->BackGroundColor);
		LPSTR pTxt = malloc(256);
		strcpy(pTxt, "Check the box to the right to display coordinates");
		COLORREF ShadowColor = CurView->BackGroundColor;
		RECT rect = CurView->DrawRect;
		SIZE txSize;
		GetTextExtentPoint32(hDC, pTxt, _fstrlen(pTxt), &txSize);
		TextOut(hDC, rect.left+(RECTWIDTH(&rect)- txSize.cx)/2, rect.top+(RECTHEIGHT(&rect)-txSize.cy)/2, pTxt, _fstrlen(pTxt));
		//DrawTextInRect(CurView->hDC, pTxt, &rect, ShadowColor, 12, 14);
		free(pTxt);
		DisplayCloseIcon();
		RestoreDC(hDC, -1);
		CurView = SaveView;
		return;
	}
    if (Message != WM_MOUSEMOVE || CD->active < 0)
{
#if ENABLETRACE
GSSiExitProg (114);
#endif
    	return; 
}                   
    SaveView = CurView;
    CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
	iview = *pNumViewports;
    while (iview--)
    {
		SetCurView ( pViewportsD[iview]); 
		if (CD->TargetViewport != CurView->ID)
			continue;
		if (CurView->Type != VIEWPORT_TYPE_CONTAINER && CurView->Type != VIEWPORT_TYPE_FORMAT)
		{
			if ((HaveVP <= 0 || !CurView->Type) && CurViewActive())
			{
				if (PtInRect(&CurView->ScreenRect, CursorPoint))
				{
					if (CurView->nProfileRoutes)
					{
						BasePoint = WinPtToBasePt(CursorPoint);
						//if (TranFilePoint (1,&BasePoint) != 2) 
						{
							if (DisplayProfileInfo(&BasePoint, CursorPoint))
								goto Exit;
						}
					}
					else if (CurView->hTranScreenToBase)
					{
						BasePoint = ScreenPtToBasePt(CursorPoint);
						DisplayCoordinate(&BasePoint, CursorPoint);
						if (CurView->Type)
						{
							HaveVP = 1;
							break;
						}
						else
							HaveVP = -1;
					}
					else if (CurView->hTranVPToBase)
					{
						DPOINT	Point = PointToDPoint(CursorPoint);

						if (CurView->hTranScreenToVP)
							Point = TranPoint(&Point, CurView->hTranScreenToVP);
						BasePoint = WinPtToBasePtD(&Point);
						DisplayCoordinate(&BasePoint, CursorPoint);
						if (CurView->Type)
						{
							HaveVP = 1;
							break;
						}
						else
							HaveVP = -1;
					}
					else
						break;
				}
			}
		}
    }
    if (!HaveVP) 
    	DisplayCoordinate (0,CursorPoint);
Exit:
    SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (114);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

void DisplayCoordinate2 (LPDPOINT pPoint,POINT CursorPoint)
#if ENABLETRACE
{GSSiEnterProg (115);
#endif
{
	UINT	i,ID;

	if (!pPoint)
	{
		pPoint = &LastDPoint;
		CursorPoint = LastPoint;
	}
	for (i=0;i<NumPassiveFun;i++)
	{
		CurPassiveFun = PassiveFunPTR[i];
		ID = *(LPSHORT) CurPassiveFun;

		switch (ID)
		{
		   	case PF_COORD_DISPLAY:   
		   		DisplayCoordinate (pPoint,CursorPoint);
{
#if ENABLETRACE
GSSiExitProg (115);
#endif
		   		return;
}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (115);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void DisplayCoordinate (LPDPOINT pPoint,POINT CursorPoint)
#if ENABLETRACE
{GSSiEnterProg (116);
#endif
{	
	LPCOORDINATEDISPLAY	CD;
	char	Text[256]; 
	DWORD	TextExtent;
	LPVIEWPORT	DisplayView, SaveView; 
	DPOINT	OutPoint;  
	static	DPOINT	CurWorldPoint, CurFormatPoint; 
	static	short	LastVPType=-1;  
	BOOL	ShowWorldCoord = FALSE;
	HFONT	Font, OldFont=0;
	COLORREF	OldColor; 
	short	SaveSize;
	int		x, y, st; 
	double	Elevation;
	LPDOUBLE	pElevation;
	SIZE	txSize;
	HDC		hDC;
	
	if (!CurPassiveFun)
{
#if ENABLETRACE
GSSiExitProg (116);
#endif
		return;
}
	CD = CurPassiveFun;  
	if (CD->ID != PF_COORD_DISPLAY || CD->active <= 0)
{
#if ENABLETRACE
GSSiExitProg (116);
#endif
		return;
}

	DisplayView = pViewports[CD->DisplayViewport-1];
	if (!DisplayView)
{
#if ENABLETRACE
GSSiExitProg (116);
#endif
		return;
}
    if (!DisplayView->Active)
{
#if ENABLETRACE
GSSiExitProg (116);
#endif
    	return;
}
    
	x=DisplayView->DrawRect.left+1;
	y=DisplayView->DrawRect.top;

    SaveView=CurView;
    SetCurView (DisplayView);
	hDC = GetDC(DisplayView->hWnd);
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	SelectClipRgn (hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn); 
  	if (CD->Refresh || !pPoint || (!SaveView->Type && !LastVPType))
		FillRectPoly (hDC,&DisplayView->DrawRect,ConvertColor(CurView->BackGroundColor,-1)); 
    SaveSize = CD->Font.lfHeight;
	CD->Font.lfHeight = -min (abs(ConvertFontHeightFromPCTofVP (CD->Font.lfHeight,CD->DisplayViewport)),
							 DisplayView->DrawRect.bottom-DisplayView->DrawRect.top - 1);
    if (*CD->Font.lfFaceName)
    	Font = CreateFontIndirect(&CD->Font); 
    else
    	Font = 0;   
    CD->Font.lfHeight = SaveSize;
    if (Font)
	    OldFont = SelectObject(hDC, Font);
	OldColor = SetTextColor (hDC,CD->FontColor);
    SetBkMode(hDC, OPAQUE);  
    SetBkColor (hDC,CurView->BackGroundColor);
	if (pPoint)
	{   
		LastDPoint = *pPoint;
		LastPoint = CursorPoint;
	
		if (SaveView->Type)
		{
			ShowWorldCoord = TRUE;
			CurWorldPoint = *pPoint;
		}
		else
		{
			if (LastVPType)
				ShowWorldCoord = TRUE;
			CurFormatPoint = *pPoint; 
		}
		LastVPType = SaveView->Type;
		OutPoint = CurWorldPoint; 
		if (*CD->ElevSurface && !CD->hSurf)
			CD->hSurf = DTMOpen (CD->ElevSurface,DBL_MAX,BT_READ,0,0);
       	pElevation = 0;
        if (CD->DisplayElevation && CD->hSurf)
        {   
        	short	EUnits = PRJ_UNITS[1]-1;
        	
//        	if (CD->Units[0] == 2)
//        		EUnits = 1;
        	Elevation = NGIELV_bci (OutPoint,CD->hSurf,EUnits);  
        	if (Elevation < DBL_MAX)
        		pElevation = &Elevation;
        }

		if (CD->DisplayLine[0])
		{   

			SetCoordText (Text,CD->LineID[0],OutPoint,CD->ElevationID,pElevation,(short)PRJ_UNITS[1],CD->Units[0],CD->Precision[0],CD->Commas[0],
							   CD->ElevUnits,CD->ElevPrecision,CD->ElevCommas,CD->MultiLine); 
			GetTextExtentPoint32 (hDC,Text, _fstrlen(Text),&txSize);
			if (ShowWorldCoord)
				TextOut(hDC, x, y, Text, _fstrlen(Text));
			y += txSize.cy;    
			pElevation = 0;
		}
		if (CD->DisplayLine[1])
		{
			st = ConvertCoord (&OutPoint,1,2);     
			if (!st)
			{
				SetCoordText (Text,CD->LineID[1],OutPoint,CD->ElevationID,pElevation,(short)PRJ_UNITS[2],CD->Units[1],CD->Precision[1],CD->Commas[1],
							   	   CD->ElevUnits,CD->ElevPrecision,CD->ElevCommas,CD->MultiLine); 
				GetTextExtentPoint32 (hDC,Text, _fstrlen(Text),&txSize);
				if (ShowWorldCoord)
				{
				if (CD->MultiLine)
					DrawText(hDC,Text, strlen(Text),&DisplayView->DrawRect, DT_LEFT);
				else
					TextOut(hDC, x, y, Text, _fstrlen(Text));
				}
				y += txSize.cy;
			}
		}
		if (CD->DisplayLine[2])
		{
			_fstrcpy (Text,"[%ALT_PROJECTION]");
			ExpandText (Text); 
			if (*Text)
			{   
				if (!_fstricmp (Text,"SCREEN"))
				{
					sprintf (Text,"%6i %6i",CursorPoint.x,CursorPoint.y);
					st = 0;
				}
				else
				{
					OutPoint.x=pPoint->x/**MFT*/;
					OutPoint.y=pPoint->y/**MFT*/; 
					st = ConvertCoord (&OutPoint,1,3);     
					if (!st)
						SetCoordText (Text,CD->LineID[2],OutPoint,0,0,(short)PRJ_UNITS[3],CD->Units[2],CD->Precision[2],CD->Commas[2],
										   CD->ElevUnits,CD->ElevPrecision,CD->ElevCommas,CD->MultiLine); 

					else
					{
						ConvertCoordError(st); 
						goto Exit;
					}
				} 
				GetTextExtentPoint32 (hDC,Text, _fstrlen(Text),&txSize);
				if (!st && ShowWorldCoord)
					TextOut(hDC, x, y, Text, _fstrlen(Text));
				y += txSize.cy;
			}
		}
		if (CD->DisplayLine[3])
		{
			SetCoordText (Text,CD->LineID[3],CurFormatPoint,0,0,(short)PRJ_UNITS[2],CD->Units[3],CD->Precision[3],CD->Commas[3],
							   CD->ElevUnits,CD->ElevPrecision,CD->ElevCommas,CD->MultiLine); 
			TextOut(hDC, x, y, Text, _fstrlen(Text));
			GetTextExtentPoint32 (hDC,Text, _fstrlen(Text),&txSize);
			y += txSize.cy;
		}
	}
Exit: 
	if (Font)
	{
	    if (OldFont)
	    	SelectObject(hDC, OldFont);
	    DeleteObject(Font);
	}
	if (CurView)
	{	
		DisplayCloseIcon();
		SetTextColor (hDC,OldColor);
		RestoreDC (hDC,-1); 
	}
    SetCurView (SaveView);
	ReleaseDC(DisplayView->hWnd, hDC);

{
#if ENABLETRACE
GSSiExitProg (116);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetCoordText (LPSTR Text,LPSTR ID,DPOINT Point,LPSTR ElevationID,LPDOUBLE pElevation,short FromUnits,short ToUnits,short Precision, BOOL Commas,short ElevToUnits,short ElevPrecision, BOOL ElevCommas,BOOL MultiLine)
#if ENABLETRACE
{GSSiEnterProg (117);
#endif
{   
	char	str1[256], str2[256], str3[256]="",fmt[32]="%3iD %2iM %2ldS";
	short	Deg,Min,ii;
	double	Sec;  
	
	Precision--; 
	Precision = min (7,max(Precision,0));
	if (Precision)
		sprintf (&fmt[11],"%i.%ifS",Precision+3,Precision);
	ElevPrecision--; 
	ElevPrecision = min (7,max(ElevPrecision,0));
	if (FromUnits == 1 && ToUnits == 2)
	{
		Point.x *= FTM;
		Point.y *= FTM;
	}
	else if (FromUnits == 2 && ToUnits == 1)
	{
		Point.x *= MFT;
		Point.y *= MFT;
	}
	if (ToUnits == 3)
	{
		if (GetDMS (Point.y,&Deg,&Min,&Sec))
		{   
			sprintf (&str1[1],"%3iD %6.3fM",abs(Deg),(double)Min+Sec/60);
			if (Deg < 0)
				*str1 = 'S';
			else
				*str1 = 'N'; 
		}
		else
			*str1 = 0;
		if (GetDMS (Point.x,&Deg,&Min,&Sec))
		{
			sprintf (&str2[1],"%3iD %6.3fM",abs(Deg),(double)Min+Sec/60);
			if (Deg < 0)
				*str2 = 'W';
			else
				*str2 = 'E';
		}
		else
			*str2 = 0;
	}
	else if (ToUnits == 4)
	{
		if (GetDMS (Point.y,&Deg,&Min,&Sec))
		{   
			if (Precision)
				sprintf (&str1[1],fmt,abs(Deg),Min,Sec);
			else
				sprintf (&str1[1],fmt,abs(Deg),Min,IDNINT(Sec));
			if (Deg < 0)
				*str1 = 'S';
			else
				*str1 = 'N'; 
		}
		else
			*str1 = 0;
		if (GetDMS (Point.x,&Deg,&Min,&Sec))
		{
			if (Precision)
				sprintf (&str2[1],fmt,abs(Deg),Min,Sec);
			else
				sprintf (&str2[1],fmt,abs(Deg),Min,IDNINT(Sec));
			if (Deg < 0)
				*str2 = 'W';
			else
				*str2 = 'E';
		}
		else
			*str2 = 0;
	}
	else if (ToUnits == 5)
	{
		sprintf (fmt,"%%%i.%if Deg",Precision+4,Precision);
		{   
			sprintf (&str1[1],fmt,fabs(Point.y));
			if (Point.y < 0)
				*str1 = 'S';
			else
				*str1 = 'N'; 
		}
		{
			sprintf (&str2[1],fmt,fabs(Point.x));
			if (Point.x < 0)
				*str2 = 'W';
			else
				*str2 = 'E';
		}
	}
	else if (ToUnits == 8)
	{   
		*str2 = 0;
		Precision = 5;
		if (ConvertGeodeticToMGRS (&Point.y,&Point.x,Precision,str1))
			*str1 = 0;
	}
	else
	{ 
		_fstrcpy (str1,ValueConv (Point.x,1,RoundTo[Precision],Commas));
		_fstrcat (str1,"E");
		_fstrcpy (str2,ValueConv (Point.y,1,RoundTo[Precision],Commas));
		_fstrcat (str2,"N"); 
	}
	if (pElevation) 
	{
		double	Elev = *pElevation;
		
		if (FromUnits == 1 && ElevToUnits == 2)
			Elev *= FTM;
		else if ((FromUnits == 2 || FromUnits == 4)&& ElevToUnits == 1)
			Elev *= MFT;

		sprintf (str3,"%s%s",ElevationID,ValueConv (Elev,1,RoundTo[ElevPrecision],ElevCommas)); 
	}
	else
	{
		_fmemset (str3,' ',64); 
		str3[63] = 0;
	}
	if (MultiLine)
		sprintf (Text,"%s%s\r\n%s\r\n%s",ID,str1,str2,str3);  
	else
		sprintf (Text,"%s%s  %s %s",ID,str1,str2,str3);  
/*	if (hWndAddWaypoint)
	{
		NewCoordPoint = Point;
    	PostMessage (hWndAddWaypoint,WM_COMMAND,IDC_UPDATECOORD, 0L);
	} */
{
#if ENABLETRACE
GSSiExitProg (117);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

BOOL DisplayProfileInfo (LPDPOINT pPoint,POINT CursorPoint)
#if ENABLETRACE
{GSSiEnterProg (118);
#endif
{	
	LPCOORDINATEDISPLAY	CD;
	char	Text[128]; 
	DWORD	TextExtent;
	LPVIEWPORT	DisplayView, SaveView; 
	DPOINT	OutPoint;
	HFONT	Font, OldFont;
	COLORREF	OldColor;
	double	Slope,Dist, Elev;
	short	SaveSize;
	int		x, y, st; 
	BOOL	rtn = FALSE;
	HDC		hDC;
	
	if (!CurrentConfig || !CurPassiveFun)
{
#if ENABLETRACE
GSSiExitProg (118);
#endif
		return FALSE;
}
	CD = CurPassiveFun;  
	if (CD->ID != PF_COORD_DISPLAY || CD->active < 0)
{
#if ENABLETRACE
GSSiExitProg (118);
#endif
		return FALSE;
}

	DisplayView = pViewports[CD->DisplayViewport-1];
	if (!DisplayView)
{
#if ENABLETRACE
GSSiExitProg (118);
#endif
		return FALSE;
}
    if (!DisplayView->Active)
{
#if ENABLETRACE
GSSiExitProg (118);
#endif
    	return FALSE;
}
    
	x=DisplayView->DrawRect.left+1;
	y=DisplayView->DrawRect.top;

    SaveView=CurView;
    SetCurView (DisplayView);
	hDC = GetDC(DisplayView->hWnd);
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	SelectClipRgn (hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn); 
  	if (CD->Refresh || !pPoint)
  	{
		FillRectPoly (hDC,&DisplayView->DrawRect,ConvertColor(CurView->BackGroundColor,-1));  
	}
    SaveSize = CD->Font.lfHeight;
	CD->Font.lfHeight = -min (abs(ConvertFontHeightFromPCTofVP (CD->Font.lfHeight,CD->DisplayViewport)),
							 DisplayView->DrawRect.bottom-DisplayView->DrawRect.top - 1);
    if (*CD->Font.lfFaceName)
    	Font = CreateFontIndirect(&CD->Font); 
    else
    	Font = 0;   
    CD->Font.lfHeight = SaveSize;
    if (Font)
	    OldFont = SelectObject(hDC, Font);
	OldColor = SetTextColor (hDC,CD->FontColor);
    SetBkMode(hDC, OPAQUE);  
    SetBkColor (hDC,CurView->BackGroundColor);
	if (pPoint)
	{   
		if (GetProfileElevAndSlope (pPoint->x,SaveView->hProfileElev[0],SaveView->nProfileElev[0],&Elev,&Slope,DBL_MAX))
		{   
			double	d=pPoint->x;
			
			if (SaveView->ProfileInCrossSection)
				d -= CurTheme->CrossSectionWidth[0];
			Dist = ConvertDist (d,SaveView->ProfileDistUnits);
			sprintf (Text,"Dist = %.3f %s, Elevation = %.1f feet, Slope = %.1f percent                ",Dist,_fstrlwr(DistUnitOpts[SaveView->ProfileDistUnits-1]),Elev * MFT,Slope); 
			TextOut(hDC, x, y, Text, _fstrlen(Text)); 
			rtn = TRUE;
		}
	} 
	if (Font)
	{
	    SelectObject(hDC, OldFont);
	    DeleteObject(Font);
	}	
	SetTextColor (hDC,OldColor);
	RestoreDC (hDC,-1);
	ReleaseDC (DisplayView->hWnd,hDC);
    SetCurView (SaveView);

{
#if ENABLETRACE
GSSiExitProg (118);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 




LPVOID BoundsDisplayInit (int Type, int DisplayVP, int TargetVP)
#if ENABLETRACE
{GSSiEnterProg (120);
#endif
{   HANDLE Handle;
	LPBOUNDSDISPLAY lpBoundsDisplay;

	Handle = GSSiGlobAlloc (  68,GHND,sizeof(THEME));
	lpBoundsDisplay = (LPBOUNDSDISPLAY)GlobalLock(Handle); 
	lpBoundsDisplay->ID = PF_BOUNDS_DISPLAY;
	lpBoundsDisplay->Handle = Handle;
	lpBoundsDisplay->DisplayVP = DisplayVP;
	lpBoundsDisplay->TargetVP = TargetVP;
	lpBoundsDisplay->Type = Type;
{
#if ENABLETRACE
GSSiExitProg (120);
#endif
	return (lpBoundsDisplay);
}
#if ENABLETRACE
}
#endif
}

void BoundsDisplayShow (LPVOID lpV, BOOL *First)
#if ENABLETRACE
{GSSiEnterProg (121);
#endif
{	LPBOUNDSDISPLAY lpBoundsDisplay;
	LPVIEWPORT	SaveVP=CurView;
	LPVISLIST	SaveVis=CurVis;
//	MNMXCORD	WBounds;
	POINT		BegPoint, EndPoint;
	DPOINT		WPoint;
	HPEN		ArrowPen;
	short		SaveNum, SaveDVI, SaveFile,i;
	DPOINT		BoundsPoly[5];

	if (!lpV)
{
#if ENABLETRACE
GSSiExitProg (121);
#endif
		return;  
}
	SaveFile = CurView->CurFile;
//	WBounds = CurView->WBounds;
	RectToDPoints (&CurView->ScreenRect,BoundsPoly);
	for (i=0;i<4;i++)
	{
		BoundsPoly[i] = TranPoint (&BoundsPoly[i],CurView->hTranScreenToVP);
		BoundsPoly[i] = TranPoint (&BoundsPoly[i],CurView->hTranVPToBase);
	}
	BoundsPoly[4] = BoundsPoly[0];
	lpBoundsDisplay = lpV; 
	SetCurView (pViewports[lpBoundsDisplay->DisplayVP-1]);
	if (!CurViewActive() || !lpBoundsDisplay->DisplayVP || !lpBoundsDisplay->TargetVP) 
	{
		SetCurView ( SaveVP); 
{
#if ENABLETRACE
GSSiExitProg (121);
#endif
		return;
}
	}
	CurView->BoundsDisplayVP = lpBoundsDisplay->TargetVP;
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);

    
/*     CurView->NewBounds = WBounds;
     SetBounds (CurView->hWnd,0);
     SaveNum=NumViewportsToDisplay;
     SaveDVI = DisplayViewID;
     NumViewportsToDisplay = -1;
     DisplayViewID=CurView->ID-1;

     if (CurView->Bitmap)
     	DeleteObject (CurView->Bitmap);
     CurView->Bitmap = 0;
     CurView->PassID=0;
     if (DisplayViewport (CurView->hWnd,CurView->hDC))
     ImediateProcessing (TRUE);
     DisplayViewID = SaveDVI;
     NumViewportsToDisplay=SaveNum;
*/    
	if (*First) lpBoundsDisplay->SavedScreen = 0;
	*First = FALSE;
	ShowZoomArea (CurView->hDC,BoundsPoly,
				               &lpBoundsDisplay->SavedScreen,
				               &lpBoundsDisplay->SavedRect,
							   lpBoundsDisplay->BoxPoints);


	RestoreDC (CurView->hDC,-1);
	SetCurView ( SaveVP); 
	CurView->CurFile = SaveFile;
	CurVis = SaveVis;
{
#if ENABLETRACE
GSSiExitProg (121);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


void BoundsDisplayDestroy (LPVOID lpV)
#if ENABLETRACE
{GSSiEnterProg (124);
#endif
{	LPBOUNDSDISPLAY lpBoundsDisplay;
	HANDLE	Handle;

	if (!lpV)
{
#if ENABLETRACE
GSSiExitProg (124);
#endif
		return;
}
	lpBoundsDisplay = lpV;
	Handle = lpBoundsDisplay->Handle;
	GSSiGlobUlFree (&Handle);
{
#if ENABLETRACE
GSSiExitProg (124);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void ShowZoomArea (HDC hDC,LPDPOINT BoundsPoly,HBITMAP *SavedScreen, RECT *SavedRect, LPPOINT Points)
#if ENABLETRACE
{GSSiEnterProg (125);
#endif
{	HPEN	hWidePen, hOldPen, hArrowPen;
	int		BoxWidth;
	int		BoxLineWidth=3 * DeviceToScreenFactor(), ArrowLineWidth=4 * DeviceToScreenFactor();
	int		ArrowLen;
	RECT	Rect;
	int		i,RegionType;   
	DPOINT	WPoint;
	POINT MinPoint, MaxPoint;
	MNMXCORD	WBounds;
    
/*	if (*SavedScreen)
	{
		RestoreScreen (CurView->hDC,*SavedScreen,*SavedRect);
		DeleteObject (*SavedScreen);
	}*/ 
	GetPolyBoundsD2 (BoundsPoly,5,&WBounds,TYPE_AREA);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn); 
//	if (!Printing)
	{   
		if (!ScreenIsRegistered (CurView->Bitmap,CurView->BitmapID))
			CurView->Bitmap = 0;
		else if (CurView->Transparent || (!CurView->LinkedTo && CurView->Bitmap && !EqualRect(&CurView->Rect,&CurView->BitmapRect)))
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
		if ((Printing || !CurView->Bitmap) && !CurView->LinkedTo)
		{   
			BOOL	SavePrinting = Printing;
			BOOL	SaveIDP;
			
		    CurView->CurZoomAreaRef = 0; 
		    Printing = FALSE;  
		    InShowZoomArea = TRUE;
			SaveIDP = InDisplayProcessing;
	    	RedisplayViewport(2,FALSE);
			InDisplayProcessing = 0;
			DisplayAllVehicles (FALSE,FALSE);
			InDisplayProcessing = SaveIDP;
		    InShowZoomArea = FALSE;
	    	Printing = SavePrinting;
	    	if (!Printing)
	    	{
		        CurView->Bitmap = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
		        CurView->BitmapRect = CurView->Rect;  
		    }
		}
		else if (CurView->Bitmap && EqualRect(&CurView->Rect,&CurView->BitmapRect))
		{
			RestoreScreen2 (CurView->hDC,CurView->Bitmap,CurView->BitmapID,FALSE);
			SavePreVehicleVPDisplay ();
		}
		else if (!CurView->LinkedTo) 
		{
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
{
#if ENABLETRACE
GSSiExitProg (125);
#endif
	        return;
}
	    }
	}
    if (Display && !Pick)
        DisplayTAGs(hDC);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn); 
	hWidePen = CreatePen (PS_SOLID,BoxLineWidth,RGB(0,0,0));
	hArrowPen = CreatePen (PS_SOLID,ArrowLineWidth,RGB(0,0,0));
	hOldPen = SelectObject (hDC,hWidePen);
  	WPoint.x = WBounds.xmn;
  	WPoint.y = WBounds.ymn;
  	MinPoint = BasePtToScreenPt (&WPoint);
  	WPoint.x = WBounds.xmx;
  	WPoint.y = WBounds.ymx;
  	MaxPoint = BasePtToScreenPt (&WPoint);
    ArrowLen = ArrowLineWidth * 4;
	Points[0].x=MinPoint.x;
	Points[0].y=MinPoint.y;
	Points[1].x=MinPoint.x;
	Points[1].y=MaxPoint.y;
	Points[2].x=MaxPoint.x;
	Points[2].y=MaxPoint.y;
	Points[3].x=MaxPoint.x;
	Points[3].y=MinPoint.y;
	Points[4]=Points[0];
	Points[6].x = (MinPoint.x + MaxPoint.x) / 2;
	Points[6].y = (MinPoint.y + MaxPoint.y) / 2;
	if (Points[6].x < (CurView->DrawRect.left + CurView->DrawRect.right) /2)
		Points[5].x = Points[6].x + ArrowLen;
	else
		Points[5].x = Points[6].x - ArrowLen;
	if (Points[6].y < (CurView->DrawRect.top + CurView->DrawRect.bottom) /2)
		Points[5].y = Points[6].y + ArrowLen;
	else
		Points[5].y = Points[6].y - ArrowLen;


	BoxWidth = min(MaxPoint.x-MinPoint.x,MinPoint.y-MaxPoint.y);
	if (BoxWidth <= ArrowLineWidth)
		DrawPointerLine (CurView->hDC,
		                Points[5],
		                Points[6],
		                hArrowPen,
		                hArrowPen,
		                (int) (ArrowLineWidth*2.5),0);
	else
	{
		for (i=0;i<5;i++)
			Points[i] = BasePtToScreenPt (&BoundsPoly[i]);
		Polyline (hDC,Points,5);
	}
	SelectObject (hDC,hOldPen);
	DeleteObject (hWidePen);
	DeleteObject (hArrowPen);
	ShowBufferedScreen (TRUE,FALSE,CurView->ID,0);
	DisplayAllVehicles (FALSE,FALSE);
//    DisplayAllVehicles (FALSE);
{
#if ENABLETRACE
GSSiExitProg (125);
#endif
	return;
}

#if ENABLETRACE
}
#endif
}
void ThemePickImage (POINT MousePoint)
#if ENABLETRACE
{GSSiEnterProg (126);
#endif
{	HBRUSH	BkBrush;
	int		iclass;
	RECT	Rect;

	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (126);
#endif
		return;
}
	CurTheme = CurView->pTheme;
	if (!CurTheme->IsActive)
{
#if ENABLETRACE
GSSiExitProg (126);
#endif
		return;
}
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{	if (PickThemeClass(iclass,MousePoint))
		{   _fstrcpy (FullBM,CurTheme->ClassBM[iclass]);
			ShowFullBM(FALSE,0,0);
{
#if ENABLETRACE
GSSiExitProg (126);
#endif
			return;
}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (126);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL DisplayNetMarkerThemeLegend(BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (127);
#endif
{
	NETMARKERSTHEMEKEY	NetMarkersThemeKey;
	double		EndMP, Val, EndVal, MP, AZ, PCT, Length, MPInc=1, MPMod,MMsize=0.1, D,F;
	long		Refno;
	short		MPDir, st, bw, n;
	MARKERVAL	NetMarker, MinMarker, MaxMarker;
	DPOINT		MarkerPoint; 
	char		txt[64], Name[64];   
	BOOL		TextOnEven, Rtn=FALSE, Opened; 
	COLORREF	color, OldColor; 
	DPOINT		BPoint;
	static		LPTHEME	NMTheme;
	static		NETMARKERSTHEMEKEY	NetMarkersThemeKeyLast; 
	static		BOOL	HaveInit=FALSE;
	DWORD		TextDisplayed=0;  
	short		Dir;
	
	if (Init)
	{
		NMTheme = CurTheme; 
		NetMarkersThemeKeyLast.Path = LONG_MIN;
		NetMarkersThemeKeyLast.StartMP = 0;
		AddTextRect(0); 
		HaveInit = TRUE;
{
#if ENABLETRACE
GSSiExitProg (127);
#endif
		return TRUE;
}
	} 
	if (!HaveInit)
{
#if ENABLETRACE
GSSiExitProg (127);
#endif
		return FALSE;
}
	CurTheme = NMTheme;
	NetMarkersThemeKey = NetMarkersThemeKeyLast;
    SetViewport (CurTheme->TargetViewport); 
                               
    TextOnEven = TRUE;
    
    if (CurView->WBounds.xmx - CurView->WBounds.xmn > 12000)
    {	
    	MPInc = 100;
/*    	Rtn = FALSE;
    	goto Exit;  */
    }
    else if (CurView->WBounds.xmx - CurView->WBounds.xmn > 8000)
    	MPInc = 10;
    else if (CurView->WBounds.xmx - CurView->WBounds.xmn > 4000)
    	MPInc = 1;
    else   
    {
    	MPInc = 0.1;   
	    if (CurView->WBounds.xmx - CurView->WBounds.xmn < 2000) 
	    	TextOnEven = FALSE;
	}	    
//    MPInc = -1;	
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
	TextOnEven = FALSE;
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn); 
  	if (*PenCOLOR>0)
  		color = *PenCOLOR;
  	else
  		color = 0; 
	OldColor = SetTextColor (CurView->hDC,ConvertColor(color,CurTheme->UseHalfTone));
    SetBkMode(CurView->hDC, TRANSPARENT);

	CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_READ, 0);
	if (!BT_FIND (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey,BT_FIRST,BT_GT,
				  (LPSTR)&EndMP))
	{   
		NetMarkersThemeKeyLast = NetMarkersThemeKey;
	    
		if (GetMenuState(GetMenu(hWndMain), IDM_SHOW_MILEPOINTS, MF_BYCOMMAND) != MF_CHECKED)  
			goto Next;
		if (!GetMilePointFromMP (NetMarkersThemeKey.Path,1,NetMarkersThemeKey.StartMP, &MinMarker,&Dir))
			goto Next;
		if (!GetMilePointFromMP (NetMarkersThemeKey.Path,1,EndMP, &MaxMarker,&Dir))
			goto Next;
		if (MinMarker.Value > MaxMarker.Value)
		{
			NetMarker = MinMarker;
			MinMarker = MaxMarker;
			MaxMarker = NetMarker;
		} 
		if (MPInc < 0)
		{  
			EndVal = MaxMarker.Value;  
			MinMarker.Value -= 0.0000001;
			if (!GetNextMilePoint (NetMarkersThemeKey.Path,1, &MinMarker))
				goto Next; 
			Val = MinMarker.Value;
		}
		else
		{
			Val =ceil (MinMarker.Value/MPInc) * MPInc;    
			EndVal = floor (MaxMarker.Value/MPInc) * MPInc; 
		}
		EndVal += 0.0000001;
		while (Val <= EndVal)   
		{   
			NetMarker = MinMarker;
			NetMarker.Value = Val; 
			if (MPInc > 0.2)          
				sprintf (txt,"%c%c%.0f%c%c",NetMarker.Prefix[0],NetMarker.Prefix[1],NetMarker.Value,
										  NetMarker.Suffix[0],NetMarker.Suffix[1]);
			else if (MPInc > 0)  
				sprintf (txt,"%c%c%.1f%c%c",NetMarker.Prefix[0],NetMarker.Prefix[1],NetMarker.Value,
										  NetMarker.Suffix[0],NetMarker.Suffix[1]); 
			else 
				sprintf (txt,"%c%c%.5f%c%c",NetMarker.Prefix[0],NetMarker.Prefix[1],NetMarker.Value,
										  NetMarker.Suffix[0],NetMarker.Suffix[1]); 
			
			if (MPInc < 0)
				bw = 2;
			else
			{							  
				MPMod = fmod (Val,1.0); 
				if (fabs (MPMod -1.0) < MPInc/20)
					MPMod = 0;
				if (TextOnEven && MPMod > MPInc/20)   
				{
					bw = 1;
					txt[0]=0;
				}
				else
					bw = 2; 
			}
	    	MP = GetMPFromMilePoint (NetMarkersThemeKey.Path,1,&NetMarker,&MPDir);
	    	if (MP >= 0)
	    	{
		    	GetCoordFromMP (NetMarkersThemeKey.Path,MP,&MarkerPoint,&AZ,&PCT,&Length,&Refno); 
				DisplayMarkers = TRUE;  
				if (MPDir < 0)
					AZ = LTWOPI (AZ + PY);
				DisplayMarker (MarkerPoint,bw,txt,MMsize,AZ+HALFPI,0,TRUE,FALSE,0,0,0,0,0);
				DisplayMarkers = FALSE; 
			}
			if (MPInc < 0)
			{
				if (!GetNextMilePoint (NetMarkersThemeKey.Path,1, &MinMarker))
					goto Next; 
				Val = MinMarker.Value; 
			}
			else
				Val += MPInc;        
		} 
	Next: 
		D=0.5;
		n=1;
		OldColor = SetTextColor (CurView->hDC,color);
		if (GetTrueStreetName (NetMarkersThemeKey.Path,Name,CurState,0))
		while (!TextDisplayed && n<8)
		{   
			F = D;
			if (n++%2) 
			{
				F = 1.0-F;   
				D *= 0.5;
			}
		    MP = NetMarkersThemeKey.StartMP + (EndMP-NetMarkersThemeKey.StartMP)*F; 
		    GetCoordFromMP (NetMarkersThemeKey.Path,MP,&BPoint,&AZ,&PCT,&Length,&Refno); 
			{   
				POINT	Point; 
				int		symbol; 
				float	SizeFactor,TextFactor; 
								
				Point = BasePtToWinPt (&BPoint);
				symbol=ShieldType (Name,&SizeFactor,&TextFactor); 
				if (symbol)   
				{   
					COLORREF	OldColor;
									
	//				OldColor = SetTextColor (CurView->hDC,StreetTextColor[SNSelectData.Layer]);
				    SetBkMode (CurView->hDC,TRANSPARENT);
					TextDisplayed=DispText (CurView->hDC,FALSE,Point.x-10,Point.x+10, Point.y, 0,2,1,
						  StreetTextSize,SizeFactor,TextFactor,
						  100, FALSE,0,Name,symbol,TRUE,0,0,-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,0,0,0); 
	//				SetTextColor (CurView->hDC,OldColor);
			    }
			}
		}
		Rtn = TRUE;
	}
    CloseNetLinkAndRef (Opened);   
	SetTextColor (CurView->hDC,OldColor); 
	BT_CLOSE (CurTheme->hScatterFile);
	CurTheme->hScatterFile = 0;    
Exit:
    Opened = FALSE;
	if (!Rtn) 
	{
		HaveInit = FALSE;
		AddTextRect(0);
	}
{
#if ENABLETRACE
GSSiExitProg (127);
#endif
	return Rtn;
}
#if ENABLETRACE
}
#endif
}

void DisplayBMThemeLegend()
#if ENABLETRACE
{GSSiEnterProg (128);
#endif
{   int		xmargin, ymargin;
	long	h, w;
	int		iclass;
	RECT	ClassColorBox;

    CurTheme->NumClass = CurTheme->NumDesiredClass;    
    if (!CurTheme->NumClass)
{
#if ENABLETRACE
GSSiExitProg (128);
#endif
    	return;
}
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	w = (((long)CurTheme->InfoBox.right - CurTheme->InfoBox.left)
		- (CurTheme->NumClass+1) * xmargin) / CurTheme->NumClass;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,ConvertColor(CurTheme->BGColor,CurTheme->UseHalfTone));
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,RGB(255,255,255));
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,ConvertColor(CurTheme->TitleBoxBG,CurTheme->UseHalfTone));

	/* Display the color boxes */
	ClassColorBox = CurTheme->InfoBox;
	ClassColorBox.bottom = CurTheme->InfoBox.bottom;
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{
		ClassColorBox.left = CurTheme->InfoBox.left + (iclass+1) * xmargin + iclass * w;
		ClassColorBox.right = ClassColorBox.left + w;
		DisplayBMFileInRect (CurView->hDC,(LPSTR)&CurTheme->ClassBM[iclass],ClassColorBox,TRUE);
		CurTheme->ClassClrBox[iclass] = ClassColorBox;
	}


{
#if ENABLETRACE
GSSiExitProg (128);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
BOOL PtInBounds (POINT SegPoint)
#if ENABLETRACE
{GSSiEnterProg (129);
#endif
{
	if (SegPoint.x < CurView->FileBounds.xmn ||
		SegPoint.x > CurView->FileBounds.xmx ||
		SegPoint.y < CurView->FileBounds.ymn ||
		SegPoint.y > CurView->FileBounds.ymx)
{
#if ENABLETRACE
GSSiExitProg (129);
#endif
		return (FALSE);
}
	else
{
#if ENABLETRACE
GSSiExitProg (129);
#endif
		return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL PtInDrawRect (POINT SegPoint)
#if ENABLETRACE
{GSSiEnterProg (130);
#endif
{
	if (SegPoint.x < CurView->DrawRect.left ||
		SegPoint.x > CurView->DrawRect.right ||
		SegPoint.y < CurView->DrawRect.top ||
		SegPoint.y > CurView->DrawRect.bottom)
{
#if ENABLETRACE
GSSiExitProg (130);
#endif
		return (FALSE);
}
	else
{
#if ENABLETRACE
GSSiExitProg (130);
#endif
		return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL PtInScreenRect(POINT SegPoint)
#if ENABLETRACE
{
	GSSiEnterProg(130);
#endif
	{
		if (SegPoint.x < CurView->ScreenRect.left ||
			SegPoint.x > CurView->ScreenRect.right ||
			SegPoint.y < CurView->ScreenRect.top ||
			SegPoint.y > CurView->ScreenRect.bottom)
		{
#if ENABLETRACE
			GSSiExitProg(130);
#endif
			return (FALSE);
		}
		else
		{
#if ENABLETRACE
			GSSiExitProg(130);
#endif
			return (TRUE);
		}
#if ENABLETRACE
	}
#endif
}
RECT PctRect (RECT Rect, float Pct)
#if ENABLETRACE
{GSSiEnterProg (131);
#endif
{   int	xinc, yinc;

	xinc = IDNINT(((Rect.right - Rect.left) * Pct) / 100);
	yinc = IDNINT(((Rect.bottom - Rect.top) * Pct) / 100);
	xinc = min (xinc,yinc);

    InflateRect (&Rect,xinc,xinc);
{
#if ENABLETRACE
GSSiExitProg (131);
#endif
	return(Rect);
}
#if ENABLETRACE
}
#endif
}
int SetDisplayChar (HDC hDC,int Type,long iref,int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (132);
#endif
{
	int rtn; 
	static	BOOL	LastRtn;   
	int		st,ii;  
	short	Dummy;
	SPECIALDATA	SpecialData;
	
	if (ForceRefIndex || ForceTAGIndex)
{
#if ENABLETRACE
GSSiExitProg (132);
#endif
		return FALSE; 
}
	
	if (DisplayOnlyNonHLT)
	{
    	if (hHighlight)
    	{
	    	if (!(st=BT_FIND (hHighlight,(LPSTR)&iref,BT_FIRST,BT_EQ,0))) 
	    	{
				rtn = FALSE;
				goto Exit;
			}
		}
	}
	if (CurTheme && (CurTheme->MinSize || CurTheme->AppendTotArea) && Type == GF_AREA && HiPrecis && nPnts > 0)
		curItemSQMeters = fabs(ComputeAreaAreaD(lpDCurPoints, nPnts, &curItemPerim));
	SetGlobalValueReal("%CURITEMSQMETERS", curItemSQMeters);
	if (curItemSQMeters > 0 && curItemSQMeters < 1)
		ii = 1;
	
	NumDynSegPointsRemaining = 0;
	NumDynSegPoints = nPnts;

	if (Type != GF_PIXEL && !InDynamicSegmentation && !hDynamicSeg &&
		!ProcessSingleItem && !ProcessAllElements && iref && iref == LastRef)
	{ 
		rtn = LastRtn;
	}
	else
	{   
		rtn = SetDisplayChar2 (hDC,Type,iref,desc,TAG,UDI);
		LastRef = iref; 
		LastRtn = rtn;
		if (rtn < 0)
{
#if ENABLETRACE
GSSiExitProg (132);
#endif
			return rtn; 
}
	}
    st = 31;
    if (hDC && CurView->PassID) 
    {
    	if (hSpecial)
    	{
	    	if (!(st=BT_FIND (hSpecial,(LPSTR)&iref,BT_FIRST,BT_EQ,(LPSTR)&SpecialData)))
			{
				SpecialThisItem = 1;
				SpecialWidth = SpecialData.Width;
				SpecialColor = SpecialData.Color;
				switch (SpecialData.Type)
				{   
					case 1:
					{
		        		int	pwidth;           
				        SelectObject(hDC, GetStockObject(BLACK_PEN));
					    if (HiPrecis)
					    	pwidth = SpecialData.Width;
					    else
			 				pwidth = IDNINT(WidthFactor*SpecialData.Width);  
					    if (hSpecialPen)
					    	DeleteObject (hSpecialPen);
						hSpecialPen = CreatePen(PS_SOLID,pwidth,SpecialData.Color);
		        		hOldPen = SelectObject (hDC,hSpecialPen);
		        	}
		        	break;
		        	case 2: 
		        	{
				        SelectObject(hDC, GetStockObject(BLACK_BRUSH));
					    if (hSpecialBrush)
					    	DeleteObject (hSpecialBrush);
						hSpecialBrush = CreateSolidBrush(SpecialData.Color);
		        		hOldBrush = SelectObject (hDC,hSpecialBrush);
					}
		        	break;
	        	}
	        	
			}
		}
		   
    	if (st && hRedLine)
    	{
	    	if (!(st=BT_FIND (hRedLine,(LPSTR)&iref,BT_FIRST,BT_EQ,(LPSTR)&SpecialData)))
			{
			}
		}
		if (CurItemHLTShow > -1)
		{
			if (CurItemHLTShow) 
			{   
				if (Display && DisplayHLTPattern)
				switch (Type)
				{	
					case GF_POINT:   
						HighlightPointSym=TRUE;
						break;
					case GF_AREA:
						SetAreaHighlight(hDC);
						break;
					case GF_LINE:   
					case GF_TEXT:
					case GF_POLYLINE:
					case GF_CURVE:
						SetLineHighlight(hDC,CurItemHLTShow); 
					case GF_PIXEL:
						break;
				}  
				HighlightThisItem = ItemIsHighlighted = CurItemHLTShow;
				rtn=TRUE;
				if (Type != GF_PIXEL && hDisplayedHighlightedRefs && rtn)
					BT_PUT (hDisplayedHighlightedRefs,(LPSTR)&iref,(LPSTR)&desc);

			}
			else
				rtn=FALSE;
		}
	} 
Exit:
	if (Type != GF_PIXEL && hDisplayedRefs && rtn)
		BT_PUT (hDisplayedRefs,(LPSTR)&iref,(LPSTR)&desc);
{
#if ENABLETRACE
GSSiExitProg (132);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

int SetDisplayChar2 (HDC hDC,int Type,long iref,int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (133);
#endif
{   int st,ii; 
	
    
	if (desc && Type != GF_AREA && !CurVis->WantType[8] && QuadTypeNext<2)
		if (GetInVisibility(desc))
{
#if ENABLETRACE
GSSiExitProg (133);
#endif
			return(-1);
}
    st = SetCharsFromTheme (hDC,Type,iref,desc,TAG,UDI);
    if (st >=0)
{
#if ENABLETRACE
GSSiExitProg (133);
#endif
    	return(st);
}
    if (CurView->NewObjectMap[desc]) 
    {   
		LPNEWOBJECT	pObj=&CurView->NewObject[CurView->NewObjectMap[desc]-1];
        short	UseHalfTone=0;
        
        if (CurView->NewObjectMap[desc] < CurView->HalfToneNewObjectStart)
        	UseHalfTone = -1;
    	OpenNewObjects();
    	if (!pObj->Handle)
    		goto SetPen;
    	else if (Display) 
    	{   
    		if (Type == GF_AREA)
    		{
    			if (pObj->Type == 1)
					SelectObject(hDC,GetStockObject(NULL_BRUSH));
    			else
					SelectObject(hDC,GetStockObject(NULL_PEN));
			}
    		CurrentPen =  pObj->Handle;
   			SelectObject (hDC,CurrentPen); 
   			HaveVarFillColor = TRUE;  
   			if (Type == GF_AREA && pObj->Type == 3)
   			{
				BYTE		PatByt = pObj->Width;
				PATBYTE		PatByte;  
				
				_fmemmove (&PatByte,&PatByt,sizeof(PATBYTE));
				if (PatByte.Pattern && !useGDIPlus)
				{
				    SetBkColor (CurView->hDC,RGB(255,255,255));
				    if (PatByte.Transparent)
						SetROP2(CurView->hDC,R2_MASKPEN);
					GlobalColors[0] = RGB(pObj->R, pObj->G, pObj->B);
				}
				else
					GlobalColors[0] = RGBI(pObj->R, pObj->G, pObj->B, pObj->Width);
   			}
			else
			{
				GlobalColors[0] = RGB(pObj->R,
					pObj->G,
					pObj->B);
				if (pObj->Width < 0)
					ItemSymbolWidth = -(pObj->Width);
				else
					ItemSymbolWidth = (pObj->Width + 1);
				SetTextColor(hDC, ConvertColor(RGB(pObj->R, pObj->G, pObj->B), UseHalfTone));
			}
   		}
   	}
    else 
SetPen:
    if (Type != GF_AREA && Display) 
    {
    	if (HiPrecis && PenFromUMList && pens[0])
    		SelectObject (hDC,pens[0]); 
    	else if (CurrentPen)
        	SelectObject (hDC,CurrentPen);
    }
{
#if ENABLETRACE
GSSiExitProg (133);
#endif
	return (TRUE);
}
#if ENABLETRACE
}
#endif
}  

BOOL DoDynamicFixedSegmentation (int Type,long iref,BOOL DynSegDone)
{   
	static	long	LastRef; 
	
	if (DynSegDone)
		return TRUE; 
	if (Type < 0) 
	{
		LastRef = LONG_MAX; 
		return TRUE;
	}
	if (Type == GF_LINE && HiPrecis && CurTheme->DynSegMaxFixedIncrement)
	{   
		if (iref != LastRef)
		{
			double	DLen = GetPolyLengthD (lpDCurPoints,nPnts);
			long	nSegs = (DLen - P_TOL) / CurTheme->DynSegMaxFixedIncrement + 1;
			
			CurTheme->DynSegFixedIncrement = DLen / nSegs + P_TOL;
		}
		LastRef = iref;	
		NumDynSegPointsRemaining = 0;
		NumDynSegPoints = nPnts;  
		if (GetPolyLengthD (lpDCurPoints,nPnts) > CurTheme->DynSegFixedIncrement)
		{   
			long	EndPointNum;
			DPOINT EndPoint = PointAtDistOnPoly (lpDCurPoints,nPnts,CurTheme->DynSegFixedIncrement,0,&EndPointNum);

			NumDynSegPoints = EndPointNum + 2; 
			DynSegSavePoint = lpDCurPoints[NumDynSegPoints-1]; 
			lpDCurPoints[NumDynSegPoints-1] = EndPoint;
			NumDynSegPointsRemaining = nPnts - EndPointNum;
		} 
		return TRUE;
	} 
	return FALSE;
}

int SetCharsFromTheme (HDC hDC, int Type, long iref, int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (134);
#endif
{	int	itheme;
	int	irc;   
	BOOL	DynSegDone=FALSE;
    short	FromTheme = 0, ToTheme = CurView->NumThemes;
    
    if (Highlight || !ProcessThemes || !CurView)
{
#if ENABLETRACE
GSSiExitProg (134);
#endif
    	return -1; 
}
    if (ProcessSelectedTheme)
    { 
    	FromTheme = ProcessSelectedTheme-1;
    	ToTheme = ProcessSelectedTheme;
    }
		for (itheme=FromTheme;itheme<ToTheme;itheme++)
		{	
			CurTheme = CurView->pThemes[itheme]; 
			if (CurTheme->IsActive && CurTheme->VPDisplayed)
				DynSegDone = DoDynamicFixedSegmentation (Type,iref,DynSegDone);
		}
		for (itheme=FromTheme;itheme<ToTheme;itheme++)
		{	CurTheme = CurView->pThemes[itheme]; 
			switch (ThemeTestChar (Type,iref,desc,TAG,UDI))
			{
				case 0:
{
#if ENABLETRACE
GSSiExitProg (134);
#endif
					return 0;
}
				case 1:
				{
					if (Pick && CurTheme->DispersePoints)
					{
						{
#if ENABLETRACE
							GSSiExitProg(134);
#endif
							return 0;
						}
					}
				}
				break;
				default:
					break;
			}
		}
	if (Pick)
{
#if ENABLETRACE
GSSiExitProg (134);
#endif
					return 1;
}
	if (!CurView->PassID)
	{	for (itheme=FromTheme;itheme<ToTheme;itheme++)
		{	CurTheme = CurView->pThemes[itheme];
			if (CurTheme->WantDataPass)
				ThemeSetData (Type,iref,desc, TAG, UDI);
		}
{
#if ENABLETRACE
GSSiExitProg (134);
#endif
		return(0);
}
	}
	else
	{	irc = -1;
		for (itheme=FromTheme;itheme<ToTheme;itheme++)
		{	CurTheme = CurView->pThemes[itheme];
			switch (ThemeSetChar (Type,iref,desc,TAG,UDI))
			{
				case 0:
{
#if ENABLETRACE
GSSiExitProg (134);
#endif
					return 0;
}
				case 1:  //processed and graphic characteristics set
					irc = max (1,irc);
					break;
				case 3:	 //
					irc = 3; //hotspot with passthrough set to themes
					break;
				case 2:  //processed and graphic characteristics not set
				default:
					break;
			}
		}
		if (irc == 3)
			irc = 0;
{
#if ENABLETRACE
GSSiExitProg (134);
#endif
		return(irc);
}
	}
#if ENABLETRACE
}
#endif
}   

void SaveHighlightList (LPHANDLE pH1, LPHANDLE pH2)
#if ENABLETRACE
{GSSiEnterProg (135);
#endif
{   
	*pH1 = hHighlight;
	*pH2 = hHighlight2;
	hHighlight = 0;
	hHighlight2 = 0; 
	*HighlightFile = 0;
	*HighlightFile2 = 0;
	{
		long iref,ii;
		HIGHLIGHTDATA hd;
		ii=BT_FIND (*pH1,(LPSTR)&iref,BT_FIRST,BT_ANY,(LPSTR)&hd);
		ii=1;
	}
{
#if ENABLETRACE
GSSiExitProg (135);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void RestoreHighlightList (HANDLE H1, HANDLE H2)
#if ENABLETRACE
{GSSiEnterProg (136);
#endif
{   
	if (!BT_GETPATHNAME (H1,HighlightFile))
		return;
	hHighlight = H1;
	hHighlight2 = H2; 
	BT_GETPATHNAME (hHighlight2,HighlightFile2);
	{
		long iref,ii;
		HIGHLIGHTDATA hd;
		ii=BT_FIND (H1,(LPSTR)&iref,BT_FIRST,BT_ANY,(LPSTR)&hd);
		ii=1;
	}
{
#if ENABLETRACE
GSSiExitProg (136);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

double AdjustHltArea(double area)
{
	if (AllAreasArePositive)
		return fabs(area);
	return area;
}

BOOL OpenHighlightList(LPSTR Name1, LPSTR Name2)
#if ENABLETRACE
{GSSiEnterProg (137);
#endif
{	BTVARDESC	BTVar[1];
	int	ii;
	long	iref;  
	short	pos = BT_FIRST; 
	LPSTR	pName1=HighlightFile, pName2=HighlightFile2; 
	HANDLE	hHLT;
    LPHIGHLIGHTDATA	pHighlightData;
	LPPICKDATA pPickData;
	
	if (hHighlight)   
	{
{
#if ENABLETRACE
GSSiExitProg (137);
#endif
		return TRUE;
}   
	}
	else if (hHighlight2)
		ii=1;
	
	hHLT = GSSiGlobAlloc (  70,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA));
	pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hHLT); 
	pPickData = &pHighlightData->PD; 
	if (Name1)
	{
		pName1 = Name1;
		if (!*pName1)
		{
			GSSiGetTempFileName (0,"gmh",0,(LPSTR)HighlightFile); 
			pName1=HighlightFile;
		}
	}
	else if (!*pName1)
		GSSiGetTempFileName (0,"gmh",0,(LPSTR)HighlightFile);
    
    ii=sizeof(HIGHLIGHTDATA);
	hHighlight = BT_OPEN (pName1, 0, BT_WRITE, 0);
	if (!hHighlight)
	{
//		BT_SET_PARMS (8,32,6,30); .9  (no call = .456)
//		BT_SET_PARMS (8,32,5,18);  .471
//		BT_SET_PARMS (8,32,5,10);   .471
//		BT_SET_PARMS (8,32,6,20);   .454
//		BT_SET_PARMS (16,32,10,24);  .464
//		BT_SET_PARMS (16,32,8,16);   .504 
		BT_SET_PARMS (8,32,6,21);    //.447
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (pName1, sizeof(HIGHLIGHTDATA), FALSE, -1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hHighlight= BT_OPEN (pName1, 0, BT_WRITE, 0);
		TotHLTLength=TotHLTPerim=TotHLTArea=TotHLTPoints=0;

	}
	if (Name2)  
	{
		pName2 = Name2;
		if (!*pName2)
		{
			GSSiGetTempFileName (0,"gmh",0,(LPSTR)HighlightFile2); 
			pName2=HighlightFile;
		}
	}
	else if (!*pName2)
		GSSiGetTempFileName (0,"gmh",0,(LPSTR)HighlightFile2);
	hHighlight2 = BT_OPEN (pName2, 0, BT_WRITE, 0);
	if (!hHighlight2)
	{
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (pName2, sizeof(long), FALSE, -1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hHighlight2= BT_OPEN (pName2, 0, BT_WRITE, 0);  
		HighlightSequence=0;
	}
	TotHLTPerim = 0;
	TotHLTArea = 0;  
	TotHLTLength = 0; 
	TotHLTPoints = 0;   
	DBoundsInit (&HLTBounds);
   	while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)pHighlightData))
   	{
   		pos = BT_NEXT;
		if (pPickData->Type == 3)
		{    
			TotHLTPerim+=pPickData->Length;
			TotHLTArea += AdjustHltArea(pPickData->Area);
		}
		else 
			TotHLTLength+=pPickData->Length;
	    TotHLTPoints+=pPickData->NumPoints;     
	    if (pPickData->Type != 6)
			AddMinMaxD (&HLTBounds, &pPickData->Rect);
	}  
	GSSiGlobUlFree (&hHLT);
{
#if ENABLETRACE
GSSiExitProg (137);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL RefnoInHighlightList(long Refno)
{
	HIGHLIGHTDATA	HighlightData;
	BOOL opened = FALSE;

	if (!hHighlight)
	{
		OpenHighlightList(0, 0);
		opened = TRUE;
	}
	int st = BT_FIND(hHighlight, (LPSTR)&Refno, BT_FIRST, BT_EQ, (LPSTR)&HighlightData);
	if (opened)
		CloseHighlightList();
	if (!st)
		return TRUE;
	return FALSE;
}

int AddToHighlightList (long Refno,LPPICKDATA pPickData,BOOL Show)
//return 0 if not added, 1 if added and not already in and 2 if already in
#if ENABLETRACE
{GSSiEnterProg (138);
#endif
{   long	LastRef;
	HANDLE	hPHLT = GSSiGlobAlloc (  71,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA)*2);
    LPHIGHLIGHTDATA	pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hPHLT);
    LPHIGHLIGHTDATA	pPrevHighlightData = pHighlightData + 1;
    short	st,ii;
    
	if (pPickData->Desc == 414)
		ii=1;
    if (PickAllPieces)
    	Refno = PickPieceRefno++;
	OpenHighlightList(0,0);     
//Refno  += CurState * 1000000;
//pPickData->Refno = Refno;
	if (!BT_FIND (hHighlight2,(LPSTR)&HighlightSequence,BT_LAST,BT_ANY,(LPSTR)&LastRef))
		HighlightSequence += 1000;
	else
		HighlightSequence = 1000;
	pHighlightData->Show = Show;   
	pHighlightData->Sequence = HighlightSequence; 
	pPickData->HasText = 0;
	pPickData->HasTextPointer = 0; 
	pPickData->Type = GetHighlightType (pPickData->Type);   
	if (pPickData->Type == 4)
		pPickData->HasText = 1;
//Refno = HighlightSequence;	// used to make refno unique accross files for freeway plot
//	pPickData->Refno = Refno;
	if (HighlightOnlyPoints && pPickData->Type != 1) 
	{
		GSSiGlobUlFree (&hPHLT);
{
#if ENABLETRACE
GSSiExitProg (138);
#endif
		return 0;	
} 
	} 
	if (HltAutoClear)
	{
		BT_CLEAR (hHighlight);
		BT_CLEAR (hHighlight2);
	}
	pHighlightData->PD = *pPickData;  
	if (lUpdateBuf && FastMapCopyFid != HFILE_ERROR)
	{
		LPSTR pBuf = GlobalLock (hUpdateBuf);

		HLTGraphicsPos = GSSillseek (FastMapCopyFid,0,1);
		BigWrite (FastMapCopyFid,&lUpdateBuf,4,-1);
		BigWrite (FastMapCopyFid,pBuf,lUpdateBuf,-1);
		GlobalUnlock (hUpdateBuf);
	}
	else
		HLTGraphicsPos = 0;
	pHighlightData->HLTGraphicsFilePos = HLTGraphicsPos;
	ItemAddedToHLT = TRUE;
	st = BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)pPrevHighlightData); 
	if (!st)
	{   
		if (pHighlightData->PD.Segment == pPrevHighlightData->PD.Segment &&
		 	pHighlightData->PD.Offset  == pPrevHighlightData->PD.Offset  &&
		 	pHighlightData->PD.FileNum == pPrevHighlightData->PD.FileNum &&
		 	pHighlightData->PD.FileInIndex == pPrevHighlightData->PD.FileInIndex &&
		 	pHighlightData->PD.SubFile == pPrevHighlightData->PD.SubFile)
		{
			if (pHighlightData->PD.Type == 4)
			{
				pPrevHighlightData->PD.HasText = 1;
				BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)pPrevHighlightData); 
			}
		}
		else if (pPrevHighlightData->PD.IsDeleted) 
		{   
			pHighlightData->Sequence = pPrevHighlightData->Sequence; 
			BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)pHighlightData); 
			goto Next;
		}
		else if (HltDupRefs)
		{ 
			st=1;
		}
		GSSiGlobUlFree (&hPHLT);
{
#if ENABLETRACE
GSSiExitProg (138);
#endif
		return 2;
}
	}
	BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)pHighlightData); 
	BT_PUT (hHighlight2,(LPSTR)&HighlightSequence,(LPSTR)&Refno);
	HighlightSequence+=1000; 
Next:
	if (pHighlightData->PD.Type == 3)
	{    
		TotHLTPerim+=pHighlightData->PD.Length;
		TotHLTArea += AdjustHltArea(pHighlightData->PD.Area);
	}
	else 
		TotHLTLength+=pHighlightData->PD.Length;
    TotHLTPoints+=pHighlightData->PD.NumPoints;
    if (!pHighlightData->PD.IsDeleted)
		AddMinMaxD (&HLTBounds, &pHighlightData->PD.Rect);
    if (HLTDlgWnd && !hHighlightArea) 
    {
    	PostMessage (HLTDlgWnd,WM_COMMAND,IDC_REDISPLAY, 0L);
    	PostMessage (HLTDlgWnd,WM_COMMAND,ID_GOTO_BOTTOM, 0L);
    }
	GSSiGlobUlFree (&hPHLT);
{
#if ENABLETRACE
GSSiExitProg (138);
#endif
	return 1;
}
#if ENABLETRACE
}
#endif
}

void AddToHighlightListSeq (long Refno,LPPICKDATA pPickData,long Sequence,BOOL Show)
#if ENABLETRACE
{GSSiEnterProg (139);
#endif
{
	HANDLE	hHLT = GSSiGlobAlloc (  72,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA));
	LPHIGHLIGHTDATA pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hHLT); 
	
	OpenHighlightList(0,0);
	pHighlightData->Show = Show;
	pHighlightData->Sequence = Sequence;
	pHighlightData->PD = *pPickData;
	BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)pHighlightData); 
	BT_PUT (hHighlight2,(LPSTR)&Sequence,(LPSTR)&Refno);
	HighlightSequence=max(HighlightSequence,Sequence);
	if (pPickData->Type == 3)
	{    
		TotHLTPerim+=pPickData->Length;
		TotHLTArea+=AdjustHltArea (pPickData->Area);  
	}
	else
		TotHLTLength+=pPickData->Length;
    TotHLTPoints+=pPickData->NumPoints;
	AddMinMaxD (&HLTBounds, &pPickData->Rect);
    if (HLTDlgWnd)
    	PostMessage (HLTDlgWnd,WM_COMMAND,IDC_REDISPLAY, 0L);  
    GSSiGlobUlFree (&hHLT);
{
#if ENABLETRACE
GSSiExitProg (139);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void RemoveFromHighlightList (long Refno,int mode)
#if ENABLETRACE
{GSSiEnterProg (140);
#endif
{   long	iref;
	LPPICKDATA pPickData;
	HANDLE	hHLT = GSSiGlobAlloc (  73,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA));
	LPHIGHLIGHTDATA	pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hHLT); 
	
	OpenHighlightList(0,0);  
	if (!BT_FIND(hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)pHighlightData))
	{   
		if (mode == 2)
		{
			pHighlightData->Show = mode;
			BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)pHighlightData); 
		}
		else
		{    
			pPickData = &pHighlightData->PD;
			if (pPickData->Type == 3)
			{    
				TotHLTPerim-=pPickData->Length;
				TotHLTArea-= AdjustHltArea (pPickData->Area);  
			}
			else
				TotHLTLength-=pPickData->Length;
		    TotHLTPoints-=pPickData->NumPoints;
			if (!BT_FIND(hHighlight2,(LPSTR)&pHighlightData->Sequence,BT_FIRST,BT_EQ,(LPSTR)&iref))
				BT_DELETE(hHighlight2,(LPSTR)&pHighlightData->Sequence,(LPSTR)&iref,FALSE); 
			BT_DELETE (hHighlight,(LPSTR)&Refno,(LPSTR)pHighlightData,FALSE); 
		    if (HLTDlgWnd && !mode)
		    	PostMessage (HLTDlgWnd,WM_COMMAND,IDC_REDISPLAY, 0L);
		}
	} 
	GSSiGlobUlFree (&hHLT);
{
#if ENABLETRACE
GSSiExitProg (140);
#endif
	return;          
}
#if ENABLETRACE
}
#endif
}

void CloseHighlightList (void)
#if ENABLETRACE
{GSSiEnterProg (141);
#endif
{
	BT_CLOSE (hHighlight); 
	BT_CLOSE (hHighlight2);
	hHighlight = 0;        
	hHighlight2= 0;
{
#if ENABLETRACE
GSSiExitProg (141);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

HFILE SaveHighlightListToConfig (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (142);
#endif
{
 	short	Length, Version=1, id=OB_HIGHLIGHTLIST;    
	OFSTRUCTGM	OFStruct;
	long	lFile1, lFile2, pos;
	char	File1[144], File2[144], Name[144]; 

	if (!hHighlight)	
{
#if ENABLETRACE
GSSiExitProg (142);
#endif
		return Fid;
}
	if (!GetOpenFilePathname (Fid, Name))
{
#if ENABLETRACE
GSSiExitProg (142);
#endif
		return Fid;
}
 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1); 
	BT_GETPATHNAME (hHighlight,File1);		
	BT_GETPATHNAME (hHighlight2,File2);		
	CloseHighlightList ();
	lFile1 = GSSiLength (File1);
	lFile2 = GSSiLength (File2);
	BigWrite (Fid,(HPSTR)&lFile1,4,-1);
	BigWrite (Fid,(HPSTR)&lFile2,4,-1);  
	GSSiClose2 (&Fid);
	copyfile (Name,File1,TRUE,0,0,0,0,0,0);
	copyfile (Name,File2,TRUE,0,0,0,0,0,0);
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
	GSSillseek (Fid,0,2);
	OpenHighlightList (File1,File2);
{
#if ENABLETRACE
GSSiExitProg (142);
#endif
	return Fid; 
}
#if ENABLETRACE
}
#endif
}

BOOL ReadHighlightListFromConfig (HFILE *Fid)
#if ENABLETRACE
{GSSiEnterProg (143);
#endif
{
	char	File1[144], File2[144], Name[144];
	short	ObjectID,Version;
	long	lFile1, lFile2, pos;  
	long	maxread=(long)USHRT_MAX, lr;  
	HFILE	FidOut;
	HANDLE	hBuf=GSSiGlobAlloc (0,GMEM_MOVEABLE,maxread);
	HPSTR	pbuf=GlobalLock (hBuf);

/*	if (!GetOpenFilePathname (*Fid, Name))
{
#if ENABLETRACE
GSSiExitProg (143);
#endif
		return FALSE;
}  */
	BigRead (*Fid,(HPSTR)&ObjectID,2);
	BigRead (*Fid,(HPSTR)&Version,2);
	BigRead (*Fid,(HPSTR)&Version,2);  
	BigRead (*Fid,(HPSTR)&lFile1,4);
	BigRead (*Fid,(HPSTR)&lFile2,4);
	ClearHighlightList (FALSE);
	OpenHighlightList(0,0);  
	BT_GETPATHNAME (hHighlight,File1);		
	BT_GETPATHNAME (hHighlight2,File2);		
	CloseHighlightList (); 
	pos = GSSillseek (*Fid,0,1); 
	FidOut = GSSiOpenFile (File1,0,OF_CREATE); 
	while (lFile1>0) 
	{
		lr=BigRead (*Fid,pbuf,min (lFile1,maxread));
		lFile1 -= lr;
		BigWrite (FidOut,pbuf,lr,-1);
	}
	GSSiClose2 (&FidOut);
	FidOut = GSSiOpenFile (File2,0,OF_CREATE); 
	while (lFile2>0) 
	{
		lr=BigRead (*Fid,pbuf,min (lFile2,maxread));
		lFile2 -= lr;
		BigWrite (FidOut,pbuf,lr,-1);
	}
	GSSiClose2 (&FidOut); 
	GSSiGlobUlFree (&hBuf);
//	GSSiClose2 (&*Fid);
//	copyfile (File1,Name,FALSE,pos,pos+lFile1,0,0,0,0);
//	copyfile (File2,Name,FALSE,pos+lFile1,pos+lFile1+lFile2,0,0,0,0);  
//	*Fid = GSSiOpenFile (Name,0,OF_READ);
	OpenHighlightList (File1,File2);   
	LoadedHLTFromConfig = TRUE;
{
#if ENABLETRACE
GSSiExitProg (143);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
}

BOOL CopyHighlightListToDB (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (144);
#endif
{   
	char	File1[MAX_PATH], File2[MAX_PATH]; 
	  
	if (!hHighlight)	
{
#if ENABLETRACE
GSSiExitProg (144);
#endif
		return FALSE; 
}
	BT_GETPATHNAME (hHighlight,File1);		
	BT_GETPATHNAME (hHighlight2,File2);		
	CloseHighlightList ();
	copyfile (Name,File1,FALSE,0,0,0,0,0,0);
	OpenHighlightList (File1,File2);
{
#if ENABLETRACE
GSSiExitProg (144);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL CopyHighlightList (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (144);
#endif
{   
	char	File1[MAX_PATH], File2[MAX_PATH]; 
	long	lFile1, lFile2;
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	BOOL	rtn=FALSE;
	
	if (!hHighlight)	
{
#if ENABLETRACE
GSSiExitProg (144);
#endif
		return FALSE; 
}
	BT_GETPATHNAME (hHighlight,File1);		
	BT_GETPATHNAME (hHighlight2,File2);		
	CloseHighlightList ();
	copyfile (Name,File1,FALSE,0,0,0,0,0,0);
	lFile1 = GSSiLength (Name);
	copyfile (Name,File2,TRUE,0,0,0,0,0,0);
	lFile2 = GSSiLength (Name) - lFile1;
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
	if (Fid != HFILE_ERROR)
	{
		GSSillseek (Fid,0,2);
		BigWrite (Fid,(HPSTR)&lFile1,4,-1);
		BigWrite (Fid,(HPSTR)&lFile2,4,-1);
		GSSiClose2 (&Fid);
		rtn = TRUE;
	}
	OpenHighlightList (File1,File2);
{
#if ENABLETRACE
GSSiExitProg (144);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL RecallHighlightList (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (145);
#endif
{   
	char	File1[MAX_PATH], File2[MAX_PATH];
	HFILE	Fid, Fid2;
	OFSTRUCTGM	OFStruct;
	long	lFile1,lFile2;  
	BOOL	rtn=FALSE;
	
	ClearHighlightList (FALSE);
	OpenHighlightList(0,0);  
	BT_GETPATHNAME (hHighlight,File1);		
	BT_GETPATHNAME (hHighlight2,File2);		
	CloseHighlightList (); 
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
	if (Fid != HFILE_ERROR)
	{
		GSSillseek (Fid,-8,2);
		BigRead (Fid,(HPSTR)&lFile1,4);
		BigRead (Fid,(HPSTR)&lFile2,4);
		GSSiClose2 (&Fid);
		
		copyfile (File1,Name,FALSE,0,lFile1,0,0,0,0);
		copyfile (File2,Name,FALSE,lFile1,lFile1+lFile2,0,0,0,0);
		OpenHighlightList (File1,File2); 
		rtn = TRUE;
	}
{
#if ENABLETRACE
GSSiExitProg (145);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void ClearHighlightList (BOOL Undraw)
#if ENABLETRACE
{GSSiEnterProg (146);
#endif
{   
	HIGHLIGHTDATA	HighlightData;
	
	if (Undraw)
	{   
		long	iref;
		short	pos=BT_FIRST;
		
	   	while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData))
	   	{
	   		pos = BT_NEXT;  
	   		PickList[0] = HighlightData.PD;
			RemoveFromHighlightList (PickList[0].Refno,2);
			ShowPickedItem (hWndMain,0); 
			RemoveFromHighlightList (PickList[0].Refno,0);
			ShowPickedItem (hWndMain,0); 
		}
    }
	BT_CLOSEANDDELETE (&hHighlight);
	BT_CLOSEANDDELETE (&hHighlight2);
	if (*HLTGraphicsFile)
		GSSiRemove (HLTGraphicsFile);
	*HLTGraphicsFile = 0;
	TotHLTLength=TotHLTPerim=TotHLTArea=TotHLTPoints=NumPicked=PickPieceRefno=HighlightSequence=0;  
	DBoundsInit (&HLTBounds);
    if (HLTDlgWnd)
    	PostMessage (HLTDlgWnd,WM_COMMAND,IDC_REDISPLAY, 0L);
{
#if ENABLETRACE
GSSiExitProg (146);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void UpdateClassValueList()
#if ENABLETRACE
{GSSiEnterProg (147);
#endif
{
	BTVARDESC	BTVar[3];
	int	i, ifield,ii;
	OFSTRUCTGM	OFStruct;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;  
	HANDLE	hBT;
	char	str[MAX_PATH];
	
	if (!CurTheme->hThemeDB) 
	{
		if (!OpenThemeDataFile(CurTheme->DataFile))
{
#if ENABLETRACE
GSSiExitProg (147);
#endif
			return;
}
	}					
	SQLPtr = (LPOPENSQLDATA)GlobalLock(CurTheme->hThemeDB);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);

	if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE)
	{   
		_fstrcpy (str,CurTheme->DataFile);
		_strlwr (str);
		if (_fstrstr (str,"graphics.gmd"))
		{
			if (!_fstricmp (CurTheme->Field.name,"SYMBOL_NAME"))
			{
				GetThemeSymbolList (CurTheme->SymNum,CurTheme->hScatterFile);
			}
		}
		else
			GetGMDUniqueFieldValues (FilePtr->FileHandle, CurTheme->SQL,
	                              CurTheme->Field.index, CurTheme->hScatterFile);  
	}
	else if (FilePtr->Type != GMCENSUS_DATAFILE)
		GetODBCUniqueFieldValues ((int)FilePtr->FileHandle, 0, 
	                              CurTheme->Field.name, GetValListFieldLen (&CurTheme->Field),
	                              CurTheme->hScatterFile);
    GlobalUnlock (SQLPtr->OFHandle);
    GlobalUnlock (CurTheme->hThemeDB);
{
#if ENABLETRACE
GSSiExitProg (147);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}













void CancelWindowZoom(void)
#if ENABLETRACE
{GSSiEnterProg (153);
#endif
{   
	if (DisableHalt)
{
#if ENABLETRACE
GSSiExitProg (153);
#endif
		return;
}
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (153);
#endif
		return;
}
	if (CurView->CurrentFunction==GF_ZOOM_RECT)
		ZoomRectangle (CurView->hWnd,GF_CANCEL,0,0,0);
{
#if ENABLETRACE
GSSiExitProg (153);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL MarginPan (HWND hWnd,UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (154);
#endif
{
 char key;
 POINT	MousePoint;
 static	HaveDownStroke=FALSE;
 
 if (DisableMarginPan)
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
 	return FALSE;
}
 if (!CurView->MarginPan)
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
 	return FALSE;
}
 switch (Message)
   {
   	case GF_INIT:
   		HaveDownStroke=FALSE;
   		break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    if (PtInRect (&CurView->ScreenRect,MousePoint))
	    {
	    	HaveDownStroke=FALSE;
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
	    	return FALSE;
}
	    }
	    if (!PtInRect (&CurView->Rect,MousePoint))
	    {
	    	HaveDownStroke=FALSE;
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
	    	return FALSE;
}
	    }
	    if (Message==WM_LBUTTONDOWN)
	    {
	    	HaveDownStroke=TRUE;
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
	    	return(TRUE);
}
	    }
	    if (!HaveDownStroke)
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
	    	return(FALSE);
}
		CancelWindowZoom();
		InDisplayProcessing = 2;
		CenterWindow (ScreenPtToBasePt(MousePoint),FALSE);
    	HaveDownStroke=FALSE;
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
		return TRUE;
}
		break;


    default:
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (154);
#endif
    return (FALSE);
}
#if ENABLETRACE
}
#endif
}






 






int	NextNewObject(BOOL DisplayMessage)
#if ENABLETRACE
{GSSiEnterProg (159);
#endif
{ 
	int	iobj = CurView->NumNewObjects;
	UINT	i,j;

	if (CurView->NumNewObjects >= MAX_NEW_OBJECTS)
	{  
		for (i=0;i<MAX_NEW_OBJECTS;i++)
		{
			for (j=0;j<3202;j++)
			{
				if (CurView->NewObjectMap[j] == i+1)
					goto NextObj;
			}
			iobj = i;
			goto HaveObj;
NextObj:;
		}
		if (DisplayMessage)
		{
			GSSiMessageBox (0,"Graphics Redefinition Table full for this viewport",0,MB_ICONEXCLAMATION,0);
			CurView->NumNewObjects--;  
		}
		else
{
#if ENABLETRACE
GSSiExitProg (159);
#endif
			return -1;
}
	}
HaveObj:
	CurView->NewObjectTextFactor[iobj] = 1;
	CurView->NewObjectSetTextColor[iobj] = 0;
	CurView->NewObject[iobj].Width = 0;
	if (iobj == CurView->NumNewObjects && CurView->NumNewObjects < MAX_NEW_OBJECTS)
		CurView->NumNewObjects++;
	TurnOffAutoVis(TRUE);
{
#if ENABLETRACE
GSSiExitProg (159);
#endif
	return (iobj);
}
#if ENABLETRACE
}
#endif
}

 








int	SelectPickedItem (void)
#if ENABLETRACE
{GSSiEnterProg (163);
#endif
{
	if (NumPicked > 0)
{
#if ENABLETRACE
GSSiExitProg (163);
#endif
		return (NumPicked-1);
}
	else
{
#if ENABLETRACE
GSSiExitProg (163);
#endif
		return (-1);
}
#if ENABLETRACE
}
#endif
}

void GetItemTAG (int item,LPSTR TAG, LPSTR Symbol, LPLONG Refno)
#if ENABLETRACE
{GSSiEnterProg (164);
#endif
{
	short SaveVisListOpt = VisListOpt;

	VisListOpt = 0;
	sprintf (TAG,"%s:%s",PickList[item].Prefix,PickList[item].UDI);
	GetSymbolName (PickList[item].Desc,Symbol,0,TRUE,0);
	VisListOpt = SaveVisListOpt;
	*Refno=PickList[item].Refno;
{
#if ENABLETRACE
GSSiExitProg (164);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL NameInSymList (LPSTR InName,int nNames,LPSTR pNames)
#if ENABLETRACE
{GSSiEnterProg (165);
#endif
{   
	LPSTR	pColon;
	
	while (nNames-- > 0)
	{   
		LPSTR	Name = InName;
		
		do
		{
			if ((pColon = _fstrrchr (InName,':')))
			{
				*pColon++ = 0;
				Name = pColon;
			}
			else
				Name = InName;
			if (!_fstricmp (Name,pNames))
{
#if ENABLETRACE
GSSiExitProg (165);
#endif
				return TRUE;
}       
		}
		while (Name != InName);
		pNames += 36;
	}
{
#if ENABLETRACE
GSSiExitProg (165);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

int GetSymbolNames (int Desc, LPHANDLE phNames, int From)	
#if ENABLETRACE
{GSSiEnterProg (166);
#endif
{
    short	nNames = 0, maxnames=16, parent, symnum=Desc;
    HANDLE	hNames = GSSiGlobAlloc (  74,GHND,maxnames*36);  
    char	Name[66];  
    LPSTR	pName; 
    BOOL	IsPar=FALSE;
    
    while (symnum && nNames < maxnames)
    {
		if (GetSymbolName (symnum, Name,&parent, From,0))
		{
	    	Truncate(Name); 
	    	pName = GlobalLock (hNames);
	    	pName += (36 * nNames); 
	    	if (IsPar)
	    		sprintf (pName,"(%s)",Name);
	    	else
	    		_fstrcpy (pName,Name);
	    	nNames++;
	    	GlobalUnlock (hNames); 
	    	symnum = parent;  
	    	IsPar = TRUE;
	    }
	    else
	    	symnum = 0;
    }
    *phNames = hNames;
{
#if ENABLETRACE
GSSiExitProg (166);
#endif
    return nNames;
}
#if ENABLETRACE
}
#endif
}

BOOL FillThemeFromGFMenu (void)
#if ENABLETRACE
{GSSiEnterProg (168);
#endif
{
	LPVIEWPORT	SaveVP=CurView; 
	
    SetViewport (CurTheme->TargetViewport); 
    CurTheme->NumClass = 0;
//    LoadFunctionLists (0,0,0,2);
	LoadGFFile (0,"",2,FALSE); 
	SetCurView ( SaveVP);
	
{
#if ENABLETRACE
GSSiExitProg (168);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

short GetPieSlice (POINT RadiusPoint,double Radius,int NumSlices,int iSlice,LPPOINT Points)
{
	short	np=1, ndeg=360/NumSlices,i;
	double	StartAZ = TWOPI - (((double)iSlice * TWOPI)/NumSlices);
	double	EndAZ   = TWOPI - (((double)(iSlice+1) * TWOPI)/NumSlices);
	double	IncAZ = -TWOPI/(double)360;
	
	Points[0] = RadiusPoint;
	for (i=0;i<ndeg;i++)
	{
		Points[np] = newpt (RadiusPoint,StartAZ+i*IncAZ,Radius);
		if (!SamePoint (Points[np-1],Points[np]))
			np++;
	}
	Points[np] = newpt (RadiusPoint,EndAZ,Radius);
	if (!SamePoint (Points[np-1],Points[np]))
		np++;
	return np;
}  

