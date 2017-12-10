#include "graphint.h"

#include "gmextern.h"


static HPSEGMENTDATA	SegData;
static SEGDATAGM	StreetTemplate;
static	short	FilterColorFrom=0, FilterColorTo=0;  
static HANDLE	hLastHLT=0;
static	double	FactorInc=0.0005;;  
static	nBMPColors;
static	COLORREF	FromColor[16],ToColor[16];


BOOL GFFunctionTemplate(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{
	static	BOOL	Inited = FALSE;
	static HCURSOR     OldCursor;
	static	UINT	CurrentPrompt;

	switch (Message)
	{
	case GF_INIT:
		Inited = TRUE;
		SaveFullWindowBitmap(hWnd);
		CurrentPrompt = PRMT_PANZOOM2;
		SetPrompt(CurrentPrompt, TRUE);
		SetCurs(IDc_SIZENWSE, FALSE);
		break;

	case GF_EXECUTE:
	case GF_USEPICKED:
/*		if (!HaveSizeFactor)
		{
			char	str[64];

			ftoa(str, NewPointSizeFactor);
			if (!GetTextString(hWnd, str, 32, "Enter size factor", 0, 0, 0, TRUE, TRUE))
				return GF_EXECUTE_CANCELED;
			NewPointSizeFactor = atof(str);
			if (!NewPointSizeFactor)
				return GF_EXECUTE_CANCELED;
			HaveSizeFactor = TRUE;
		}
		if (Message == GF_USEPICKED)
			NumPicked = 1;
		if (NumPicked > 0)
		{
			UpdateItem = 202;
			UpdateRecord(0, PickList[NumPicked - 1].Desc, PickList[NumPicked - 1].Prefix, PickList[NumPicked - 1].UDI, 0, 0, 1, -1);
		}
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
			*/
		return GF_INCREASE_SUCCESS_COUNT;

	case GF_ENTER_VIEWPORT:
		SetPrompt(CurrentPrompt, TRUE);
	case GF_REDRAW:
	case GF_REDRAW_CMD:
	case GF_CLEAR:
	case GF_CLEAR_CMD:
	case GF_INCREASE_SUCCESS_COUNT:
	case GF_DECREASE_SUCCESS_COUNT:
	case GF_DISPLAYMESS:
	case GF_READY_TO_PROCESS:
	case GF_EXECUTE_FINISHED:
	case GF_EXIT_VIEWPORT:
		break;
	case WM_LBUTTONUP:
	{
		DPOINT		BasePoint;
		POINT		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));


		if (CursorIsLocked)
			BasePoint = CurrentPoint;
		else
			BasePoint = ScreenPtToBasePt(MousePoint);
	}
		break;

	case WM_RBUTTONUP:
	{
	}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:  //ESC   
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
			break;
		case VK_F9:
		default:
			return FALSE;
		}
		break;

	case WM_CHAR:
	{
		switch (wParam)
		{
		case 'x':
		case 'X':
			PostMessage(hWnd, GF_COMPLETE, 0, 0L);
			break;
		default:
			return FALSE;
		}
	}
		break;

	case GF_COMPLETE:
		PostMessage(hWnd, GF_CLOSE, 0, 0L);
		break;

	case GF_CLOSE:
		RestoreFullWindowBitmap();
		ClearFullWindowBitmap(0);
		Inited = FALSE;
		return FALSE;
		break;

	default:
		return (FALSE);
	}
	return (TRUE);
}

LONG FAR PASCAL CloseWhenCursorLeavesMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam,LPINT pLeaveCounter)
{
	LONG    lRtn = 0;

	switch (Message)
	{
	case WM_INITDIALOG:
		*pLeaveCounter = 0;
		ii = SetTimer(hWndDlg, LEAVE_WINDOW_TIMER, 500, (TIMERPROC)0);
		break;
	case WM_NCDESTROY:
		KillTimer(hWndDlg, LEAVE_WINDOW_TIMER);
		break;
	case WM_TIMER:
	{
		RECT rect;
		POINT pt;

		if (wParam != LEAVE_WINDOW_TIMER)
			break;

		lRtn = 1;
		if (!(*pLeaveCounter)++)
			break;

		GetCursorPos(&pt);
		if (!IsWindowVisible(hWndDlg))
		{
			GetWindowRect(hWndDlg, &rect);
			if (PtInRect(&rect, pt))
				AnimateWindow(hWndDlg, 400, AW_BLEND);
		}
		else
		{
			GetWindowRect(hWndDlg, &rect);
			if (!PtInRect(&rect, pt))
			{
				AnimateWindow(hWndDlg, 400, AW_BLEND | AW_HIDE);
				wantnextblt();
				SetFocus(GetParent(hWndDlg));
			}
		}
		*pLeaveCounter = 0;
	}
		break;
	}

	return lRtn;
}

void FAR PASCAL SubclassCWCL(HWND hCtrl, HWND parent)
{
	FARPROC     lpOrgProc;
	DWORD		PrDat;
	HWND		hWnd;

	lpOrgProc = (FARPROC)SetWindowLong(hCtrl, GWL_WNDPROC,
		(LONG)(FARPROC)CloseWhenCursorLeavesMsgProc);
	SetProp(hCtrl, "PrHI", (HANDLE)HIWORD(lpOrgProc));
	SetProp(hCtrl, "PrLO", (HANDLE)LOWORD(lpOrgProc));
	SetProp(hCtrl, "GMPar", (HANDLE)LOWORD(parent));
	SetProp(hCtrl, "GMMar", (HANDLE)HIWORD(parent));
}

BOOL CALLBACK EnumCWCLDlg(HWND hWnd, LPARAM hWndPar)
{
	if (hWndPar)
		SubclassCWCL(hWnd,(HWND) hWndPar);
	return TRUE;
}

void CloseWhenCursorLeavesInit(HWND hWndDlg)
{
	POINT pt;
	RECT  rect;

	GetWindowRect(hWndDlg, &rect);
	pt = RectMid(&rect);
	SetCursorPos(pt.x, pt.y);
	EnumChildWindows(hWndDlg, EnumCWCLDlg, (LPARAM)hWndDlg);
	//if (!HaveTrackMouseEvent)
	{
		TRACKMOUSEEVENT EventTrack;

		EventTrack.dwFlags = TME_LEAVE;
		EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
		EventTrack.hwndTrack = hWndDlg;
		EventTrack.dwHoverTime = 0;
		TrackMouseEvent(&EventTrack);
		//HaveTrackMouseEvent = TRUE;
	}

	return;
}


BOOL ResizePoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
 	POINT	MousePoint;
 	DPOINT	BasePoint;
	static	HaveSizeFactor;

	switch (Message)
   {
   	case GF_INIT:
		HaveSizeFactor = FALSE;
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
		SetPrompt (PRMT_HIGHLIGHT,TRUE); 
   		SetPickCursor(TRUE);
       	AddLBUTTON = TRUE;
       	return GF_READY_TO_PROCESS;
      	break;
    
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
    case GF_EXECUTE:
	case GF_USEPICKED:
		if (!HaveSizeFactor)
		{
			char	str[64];

			ftoa (str,NewPointSizeFactor);
			if (!GetTextString (hWnd,str,32,"Enter size factor",0,0,0,TRUE,TRUE)) 
				return GF_EXECUTE_CANCELED;
			NewPointSizeFactor = atof (str);
			if (!NewPointSizeFactor)
				return GF_EXECUTE_CANCELED;
			HaveSizeFactor = TRUE;
		}
		if (Message == GF_USEPICKED)
			NumPicked = 1;
        if (NumPicked > 0)
        {   
			UpdateItem = 202;
			UpdateRecord (0,PickList[NumPicked-1].Desc,PickList[NumPicked-1].Prefix,PickList[NumPicked-1].UDI,0,0,1,-1);
        }
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L);
		return GF_INCREASE_SUCCESS_COUNT;

    case GF_REDRAW: 
   	case GF_REDRAW_CMD: 
   		break;
    case WM_CHAR:
		{
		char	key = wParam;

			switch (key)
			{   
			case 'A':
				default:
					return FALSE;
			}
		}
		break;
    case GF_COMPLETE:   
   
    case GF_EXECUTE_FINISHED:  
    {
	}
		break;

    default:
    	return (FALSE);
	}
    return TRUE;
} 

BOOL HaveMeterPrompts (void)
{
	if (hLastBox || hLastHLT)
		return TRUE;
	return FALSE;
}

BOOL GetMeterRect (LPRECT pRect)
{
	RECT	rect;

 	if (hLastBox)
	{ 
		LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);

		if (hLastHLT)
		{
			LPSAVESCREEN	pSaveScreen2=(LPSAVESCREEN)GlobalLock (hLastHLT);

			UnionRect (&rect,&pSaveScreen->Rect,&pSaveScreen2->Rect);
			GlobalUnlock (hLastHLT);
		}
		else
			rect = pSaveScreen->Rect;
		GlobalUnlock (hLastBox);
		*pRect = rect;
		return TRUE;
    } 
	if (hLastHLT)
	{
		LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastHLT);

		rect = pSaveScreen->Rect;
		GlobalUnlock (hLastHLT);
		return TRUE;
	}
	return FALSE;
}

void ClearMeterPrompts (HDC hDC)
{
 	if (hLastBox)
	{   
	    RestoreScreen2 (hDC, hLastBox,0,FALSE);
	    DestroySavedScreen (&hLastBox,0);
		ClearVehicleInfoRect ();
    } 
	if (hLastHLT)
	{
	    RestoreScreen2 (hDC, hLastHLT,0,FALSE);
	    DestroySavedScreen (&hLastHLT,0);
	}
//	GSSiGlobFree (&hLastCmd);
	return;
}

BOOL ZoomRectangle (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1161);
#endif
{HDC hDC;
 static short Xmn, Ymn, rw, rh;
 short Xmove, Ymove;
 static BOOL    HaveBox=FALSE, VarAspect=FALSE, Clip=TRUE;
 static POINT   LastPoint, StartPoint;
 DPOINT LastPointD, StartPointD;
 POINT  MousePoint, NewPoint, CursorPoint;
 static BOOL    HaveStartPoint, /*RButtonDown,*/ SetMove;  
 static LONG    DownLoc;
 static HCURSOR     OldCursor;  
 static	UINT	CurrentPrompt;

 switch (Message)
   {
    case GF_INIT:
        HaveStartPoint = FALSE;  
        if (Function == GF_ZOOM_VARRECT)
        	VarAspect = TRUE;
        else 
        	VarAspect = FALSE;
        Clip = TRUE;
/*      RButtonDown = FALSE;*/
        hDC = GetDC (hWnd);
        SetDisplayMode (hDC, GF_SCREENMODE);
        CurrentPrompt = PRMT_PANZOOM1;
        SetPrompt (CurrentPrompt,TRUE);
        break;

    case WM_LBUTTONDOWN:
        if (HaveStartPoint)
{
#if ENABLETRACE
GSSiExitProg (1161);
#endif
        	return (TRUE); 
}
        CurrentPrompt = PRMT_PANZOOM2;
        SetPrompt (CurrentPrompt,TRUE);
        SetCurs (IDc_SIZENWSE,FALSE); 
        StartPoint.x = LOWORD(lParam);
        StartPoint.y = HIWORD(lParam);
        LastPoint = StartPoint;
        HaveStartPoint = TRUE;
        HaveBox = FALSE;
/*          RButtonDown = FALSE;*/
        SetCurs (IDc_SIZENWSE,FALSE);
        SetMove = FALSE;
        break;
    
    case GF_INITRECT:
        StartPointD.x = ZoomBoxRect.xmx;
        StartPointD.y = ZoomBoxRect.ymx; 
        LastPoint = BasePtToScreenPt (&StartPointD);
        StartPointD.x = ZoomBoxRect.xmn;
        StartPointD.y = ZoomBoxRect.ymn; 
        NewPoint = StartPoint = BasePtToScreenPt (&StartPointD);
        CursorPoint = NewPoint;
        ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
        SetCursorPos (CursorPoint.x,CursorPoint.y);
        hDC = GetDC (hWnd);
        ShowZoomBox (hDC,StartPoint,LastPoint,&HaveBox,Clip,Function);
        ReleaseDC (hWnd,hDC);
        break;           
        
    case GF_CLIPBOX:
        Clip = FALSE;
        break;           
        
    case WM_LBUTTONUP:
         if (wParam == MK_RBUTTON)
{
#if ENABLETRACE
GSSiExitProg (1161);
#endif
         	return(TRUE);
}
         /*if (RButtonDown) retrn (TRUE);*/
         SetMove=FALSE;

    case WM_MOUSEMOVE:
        SetPrompt (CurrentPrompt,TRUE);
        if (HaveStartPoint)
        {   if (SetMove)
            {   SetMove=FALSE;
{
#if ENABLETRACE
GSSiExitProg (1161);
#endif
                return (TRUE);
}
            }
            MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
            Xmove = (short)(MousePoint.x - LastPoint.x);
            Ymove = (short)(MousePoint.y - LastPoint.y);
            if (wParam == (MK_RBUTTON|MK_LBUTTON))
            {   StartPoint.x += Xmove;
                StartPoint.y += Ymove;
                LastPoint = MousePoint;
        		SetCurs (IDc_SIZE,FALSE);
                /*OldCursor = SetCursor (hCursor);*/
            }
            else
            {   
                if (VarAspect)
                {
                    NewPoint.x = LastPoint.x + Xmove;
                    NewPoint.y = LastPoint.y + Ymove;
                }
                else
                    NewPoint = AdjustPoint (hWnd,StartPoint,LastPoint,Xmove,Ymove);
                CursorPoint = NewPoint;
                ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
                SetCursorPos (CursorPoint.x,CursorPoint.y);
                SetMove=TRUE; /* SetCursor causes cursor move - ignore it to avoid nervous zoom box */
		        SetCurs (IDc_SIZENWSE,FALSE);
                /*OldCursor = SetCursor (hCursor);*/
                LastPoint = NewPoint;
            }
			hDC = GetDC(hWnd);
			ShowZoomBox(hDC, StartPoint, LastPoint, &HaveBox, Clip, Function);
			ReleaseDC(hWnd, hDC);
			if (Message == WM_LBUTTONUP)
            {  
				RECT	Rect;
				DPOINT	DPoints[4];
				short	i;

				RectInit (&Rect);
				AddPointToRect (StartPoint,&Rect);
				AddPointToRect (LastPoint,&Rect);
				RectToDPoints (&Rect,DPoints);
				DBoundsInit (&ZoomBoxRect);
				for (i=0;i<4;i++)
				{
					DPOINT Point = TranPoint (&DPoints[i],CurView->hTranScreenToVP);
					ZoomBoxPoly[i] = TranPoint (&Point,CurView->hTranVPToBase);
					AddDPointToMinMax (&ZoomBoxPoly[i],&ZoomBoxRect);
				}
				ZoomBoxMidPoint = MinMaxMidPointD (&ZoomBoxRect);
				ZoomBoxScale = ldistp (DPoints[0],DPoints[2]) / ldistp (ZoomBoxPoly[0],ZoomBoxPoly[2]);
/*				StartPointD = WinPtToBasePt(StartPoint);
                LastPointD = WinPtToBasePt(LastPoint);
                ZoomBoxRect.xmn = min (StartPointD.x,LastPointD.x);
                ZoomBoxRect.ymn = min (StartPointD.y,LastPointD.y);
                ZoomBoxRect.xmx = max (StartPointD.x,LastPointD.x);
                ZoomBoxRect.ymx = max (StartPointD.y,LastPointD.y);*/
                if (Function == GF_ZOOM_VARRECT)  
                {
					UserBounds[0].x = ZoomBoxRect.xmn;
					UserBounds[0].y = ZoomBoxRect.ymn;
					UserBounds[1].x = ZoomBoxRect.xmx;
					UserBounds[1].y = ZoomBoxRect.ymx; 
				}
		        SetCurs (0,FALSE);
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
            }
        }
        break;
	case GF_REDRAW:
		ii=1;
		HaveBox = FALSE;
		hDC = GetDC(hWnd);
		ShowZoomBox(hDC, StartPoint, LastPoint, &HaveBox, Clip, Function);
		ReleaseDC(hWnd, hDC);
		break;
    case GF_CANCEL:
        ZoomBoxRect.xmn=2;
        ZoomBoxRect.xmx=1;
        StartPoint.x=5000;
        StartPoint.y=5000;
        LastPoint=StartPoint;
        hDC = GetDC (hWnd);
        ShowZoomBox (hDC,StartPoint,LastPoint,&HaveBox,Clip,Function);
        ReleaseDC (hWnd,hDC);
        HaveStartPoint = FALSE;
        SetCurs (0,FALSE);
        PostMessage(hWnd, GF_CLOSE,0, 0L);
        break;

    case WM_RBUTTONDOWN:
    /*if (HaveStartPoint) RButtonDown = TRUE;*/   
    	CurrentPrompt = PRMT_PANZOOM3;
        SetPrompt (CurrentPrompt,TRUE);
		SetCurs (IDc_SIZE,FALSE);
        DownLoc = lParam;
        hDC = GetDC (hWnd);
		HaveBox = FALSE;
        ShowZoomBox (hDC,StartPoint,LastPoint,&HaveBox,Clip,Function);
		HaveBox = FALSE;
        ReleaseDC (hWnd,hDC);
        break;
    case WM_RBUTTONUP:
        /*RButtonDown = FALSE;*/
    	CurrentPrompt = PRMT_PANZOOM2;
        SetPrompt (CurrentPrompt,TRUE);
		SetCurs (IDc_SIZENWSE,FALSE);
		hDC = GetDC (hWnd);
		HaveBox = FALSE;
        ShowZoomBox (hDC,StartPoint,LastPoint,&HaveBox,Clip,Function);
		HaveBox = FALSE;
        ReleaseDC (hWnd,hDC);
        if (lParam == DownLoc)
{
#if ENABLETRACE
GSSiExitProg (1161);
#endif
        	return FALSE; /* allows grap fun menu to work */
}
        break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1161);
#endif
        return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1161);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL MoveTAG (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 int Xmove, Ymove;
 int		OldMode;
 static BOOL	HaveBox=FALSE;
 static POINT	LastPoint;
 POINT	Point,	MinPoint,	MaxPoint, MousePoint;
 static BOOL	LButtonDown, HavePL;
 char	key;
 static	TAGBOX	SaveTAGBox;
 static BOOL	Restore;
 int	nRc;
 POINT	CenterPoint;
 static	BOOL	HaveDown=FALSE;   
 static	short	InitVPID;

hDC = CurView->hDC;
switch (Message)
   {
   	case GF_INIT:  
   		InitVPID = CurView->ID;
    	LButtonDown = TRUE;
		DisableMarginPan = TRUE;
    	SaveTAGBox = TAGBox; 
        SetCurs (LoadCursor (hInst,"MOVE_CURSOR"),FALSE);
   		SetPrompt (PRMT_MOVETAG,TRUE);  
    	CenterPoint = TAGPtToWinPt (TAGBox.center); 
    	LastPoint = CenterPoint;
       	Restore=TRUE; 
       	HaveDown=TRUE;
		ClientToScreen (hWnd,(LPPOINT)&CenterPoint);
    	SetCursorPosGM (CenterPoint.x,CenterPoint.y,0); 
   		break;
    
    case WM_PAINT:
    	DefWindowProc(hWnd, Message, wParam, lParam); 
    	return TRUE;
    	
    case WM_LBUTTONDOWN:  
 	    HaveDown = TRUE;
    	
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
/*       	MousePoint.x = LOWORD(lParam);
       	MousePoint.y = HIWORD(lParam);
       	Point=MousePoint;
       	LastPoint = Point;
       	HaveBox = FALSE;
       	LButtonDown = TRUE;
       	Restore=TRUE;*/  
		OldMode = SetROP2(hDC,R2_NOT);
    	DrawTAG (hWnd,hDC, TRUE,Restore);
		SetROP2(hDC,OldMode);
		SetDisplayMode (hDC, GF_SCREENMODE);
    	DestroySavedScreen (&TAGBox.before,0);
    	DrawTAG(hWnd,hDC,FALSE,TRUE);
/*    	if (TAGBox.before) 
    	{
    		DeleteObject(TAGBox.before);
    		TAGBox.before=0;
    	}*/   
    	LButtonDown = FALSE;           
		DisableMarginPan = FALSE;
		if (DoSave)
		{
        	SaveTAG(TBNum); 
        	DoSave = FALSE;
        }
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
    	break;

    case WM_MOUSEMOVE:
    	{
    		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
			OldMode = SetROP2(hDC,R2_NOT);
	    	DrawTAG (hWnd,hDC, TRUE,Restore);
	    	Restore=FALSE;
    		TAGBox.center = WinPtToTAGPt (MousePoint);
	    	DrawTAG (hWnd,hDC, TRUE,FALSE);
			SetROP2(hDC,OldMode);
    	}
       	break;

    case GF_CLOSE:
/*		hDC = CurView->hDC;
		SetDisplayMode (hDC, GF_TEXTMODE);
		TAGBox = SaveTAGBox;
		DrawTAG(hWnd,hDC,FALSE,TRUE);
	    	if (TAGBox.before) 
	    	{
	    		DeleteObject(TAGBox.before);
	    		TAGBox.before=0;
	    	}*/
		DisableMarginPan=FALSE;  
		DoSave = FALSE;
		SetViewport (InitVPID);
        return FALSE;

    case WM_RBUTTONUP:
		SetDisplayMode (hDC, GF_SCREENMODE);
    	setDoPaint( FALSE);
         {
          DLGPROC lpfnTAGEDITMsgProc;

          lpfnTAGEDITMsgProc = MakeProcInstance((DLGPROC)TAGEDITMsgProc, hInst);
          nRc = DialogBox(hInst, (LPSTR)"TAGEDIT", hWnd, lpfnTAGEDITMsgProc);
          FreeProcInstance(lpfnTAGEDITMsgProc);
         }
		RestoreTAG(hDC);
		ResetTAGBox (hDC,2);
		DrawTAG(hWnd,hDC,FALSE,TRUE);
    	if (TAGBox.before) 
	    	DestroySavedScreen (&TAGBox.before,0);
		SaveFullWindowBitmap (hWndMain);
		setDoPaint( TRUE);
       	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}
BOOL SizeTAG (HWND hWnd,  int Message, WPARAM wParam, LPARAM lParam)
{static LONG	DownPoint;
 DPOINT	BasePoint;
 POINT	MousePoint, Point1, Point2;
 RECT	Rect;
 LPVIEWPORT	SaveVP;
 char key;
 static	BOOL	HaveDown=FALSE; 
 static	int		TBNum,LastTB;
 static	BOOL	HaveBox=FALSE, IsHighlighted=FALSE;   
 static	RECT	LastRect;


  switch (Message)
   {
   	case GF_INIT:
		SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	  	SelectClipRgn (CurView->hDC,0);
   		HaveDown = FALSE;
       	AddLBUTTON = TRUE;
   		break;

    case GF_COMPLETE:
/*		CurView->NewBounds = ZoomBoxRect;*/    
		BasePoint.x = (ZoomBoxRect.xmn + ZoomBoxRect.xmx) / 2;
		BasePoint.y = (ZoomBoxRect.ymn + ZoomBoxRect.ymx) / 2;
		MousePoint = BasePtToScreenPt(&BasePoint);
       	TAGBox.center = WinPtToTAGPt (MousePoint);
       	BasePoint.x = ZoomBoxRect.xmn;
       	BasePoint.y = ZoomBoxRect.ymn;
       	Point1 = BasePtToScreenPt (&BasePoint);
       	BasePoint.x = ZoomBoxRect.xmx;
       	BasePoint.y = ZoomBoxRect.ymx;
		ZoomBoxRect.xmn = 2;
		ZoomBoxRect.xmx = 1;
       	Point2 = BasePtToScreenPt (&BasePoint); 
       	
       	if (TAGBox.CoordStyle == 2)
       		Rect = MainRect;
       	else
       		Rect = CurView->Rect;
		TAGBox.bmHeightD=
			fabs((double)(Point2.y-Point1.y)/(double)(Rect.top - Rect.bottom));
		TAGBox.bmWidthD=
			fabs((double)(Point2.x-Point1.x)/(double)(Rect.right - Rect.left));
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		ResetTAGBox (CurView->hDC,2);
		DrawTAG(hWnd,CurView->hDC,FALSE,TRUE);
		SaveTAG(TBNum);
       	break;

    case WM_MOUSEMOVE:
    	if (wParam == (MK_LBUTTON))
    	{   
		    short SaveVPID = CurView->ID; 
		    
    		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    	if ((TBNum=PickTextBox (&MousePoint)))
	    	{    
	    		if (TBNum != LastTB && IsHighlighted)
	    		{
	    			InvertRect (CurView->hDC,&LastRect); 
	    			IsHighlighted = FALSE;
	    		}
	    		InvertRect (CurView->hDC,&TAGBox.rect); 
	    		LastRect = TAGBox.rect;
	    		LastTB = TBNum;
	    		if (IsHighlighted)
	    			IsHighlighted = FALSE;
	    		else
	    			IsHighlighted = TRUE;
	    		HaveBox=TRUE;
	    	}
	    	else 
	    	{
			    if (IsHighlighted)
	    			InvertRect (CurView->hDC,&LastRect); 
	    		IsHighlighted = FALSE;
	    		HaveBox=FALSE;
	    	}
	    	SetViewport (SaveVPID);
	    }
	    break;
    case WM_LBUTTONUP:
	    if (IsHighlighted)
	    	InvertRect (CurView->hDC,&TAGBox.rect);    
	    IsHighlighted = FALSE;
	    if (!HaveDown) break;
    	HaveDown = FALSE;
    	AddGraphicsFunction (hWnd, GF_ZOOM_VARRECT,0);   
    	WinRectToBounds (&TAGBox.rect,&ZoomBoxRect);
	    SendMessage (hWnd,GF_INITRECT,0L,0);
	    SendMessage (hWnd,GF_CLIPBOX,0L,0);
		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL WindowZoom (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (90);
#endif
{
	static	short	ZoomVP;
	switch (Message)
   {
    case GF_INIT:  
		ZoomVP = CurView->ID;

    case GF_REINIT:
   		if (ScaleIsSet (TRUE))
   			return FALSE;
        CurView->ZBRect = CurView->ScreenRect;
        AddGraphicsFunction (hWnd, GF_ZOOM_RECT,0);
		BlockVehicleDisplay = 1;
        SetCurs (LoadCursor (hInst,"WINZOOM"),FALSE);
        break;

	case GF_CLOSE:
		BlockVehicleDisplay = 0;
		break;
    case GF_COMPLETE: 
		BlockVehicleDisplay = 0;
        if (ZoomBoxRect.xmn < ZoomBoxRect.xmx)  
        {
			DPOINT	DPoints[4];
			short	i;

//            CurView->NewBounds = ZoomBoxRect;
			SetViewport (ZoomVP);
			RectToDPoints (&CurView->ScreenRect,DPoints);
			ZoomBoxScale = ldistp (ZoomBoxPoly[0],ZoomBoxPoly[2])/ldistp (DPoints[0],DPoints[2]);
        	RemoveGraphicsFunction (hWnd,0);  
//			PostMessage(hWnd, GF_CLOSE,0, 0L); 
			CurView->CurZoomAreaRef = 0;
			ZoomToPointAndScale (ZoomBoxMidPoint,ZoomBoxScale,FALSE);
//	        ZoomToRect(CurView->NewBounds,FALSE);
/*            if (CurView->OrthoRes >=0 && CurView->WindowZoomedToOrtho == 1)
                CurView->WindowZoomedToOrtho = 0;  
        	ClearCurrentCD ();
            DisplayCycle++;
            SetBounds (hWnd,CurView->hDC);  
            DisplayCycle--;
		    CurView->CurZoomAreaRef = 0;
            RedisplayViewport(FALSE,FALSE);
            CurView->WindowIsZoomed = TRUE;*/
	        ZoomBoxRect.xmn=2;
	        ZoomBoxRect.xmx=1;
        }
		else
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
        break;

    default:
{
#if ENABLETRACE
GSSiExitProg (90);
#endif
        return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (90);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL BlowUp (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{	static int	BlowUpStep;
	POINT	Point, ScreenPoint1, ScreenPoint2;
	DPOINT	DPoint;
	RECT	RgnRect;
	HDC		hDC;
	HRGN	hRgn;
	int		i;

 switch (Message)
   {
   	case GF_INIT:
   		BlowUpStep = 1;
       	AddLBUTTON = TRUE;
		AddGraphicsFunction (hWnd, GF_ZOOM_VARRECT,0);
   		break;

    case GF_COMPLETE:
    	if (BlowUpStep == 1)
    	{	BlowUpRec.FromBounds = ZoomBoxRect;
			ZoomBoxRect.xmn = 2;
			ZoomBoxRect.xmx = 1;
    		BlowUpStep = 2;
		    BoundsToWinRect (&BlowUpRec.FromBounds,&CurView->ZBRect);
	        PostMessage(hWnd, GSSI_ADDGF, GF_ZOOM_RECT, CurView->ID);
	    	PostMessage (hWnd,GF_CLIPBOX,0L,0);
			DisableMarginPan = TRUE;
			IgnoreSelectVP = TRUE;
    	}
    	else if (BlowUpStep == 2)
    	{	BlowUpRec.ToBounds = ZoomBoxRect;
			ZoomBoxRect.xmn = 2;
			ZoomBoxRect.xmx = 1;
    		BlowUpStep = 0;
			DPoint.x = BlowUpRec.ToBounds.xmn;
			DPoint.y = BlowUpRec.ToBounds.ymn;
			ScreenPoint1 = BasePtToScreenPt(&DPoint);
			DPoint.x = BlowUpRec.ToBounds.xmx;
			DPoint.y = BlowUpRec.ToBounds.ymx;
			ScreenPoint2 = BasePtToScreenPt(&DPoint);
			BlowUpRec.ToWinRect.left = ScreenPoint1.x;
			BlowUpRec.ToWinRect.bottom = ScreenPoint1.y;
			BlowUpRec.ToWinRect.right = ScreenPoint2.x;
			BlowUpRec.ToWinRect.top = ScreenPoint2.y;
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		}
       	break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL PanToPoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (152);
#endif
{
 char key;
 POINT	MousePoint; 
 static	HaveDown=FALSE; 
 BOOL	Imediate;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=FALSE;  
        SetPrompt (PRMT_PANPOINT,TRUE);
        SetCurs (LoadCursor (hInst,MAKEINTRESOURCE(IDC_CENTERPOINT_CURSOR)),FALSE);
        break;
        
    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	//if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	if (Function == GF_PAN_TO_POINT_NOINT)
    		Imediate = TRUE;
    	else
    		Imediate = FALSE;
		CenterWindow (ScreenPtToBasePt(MousePoint),Imediate);
		return (GF_INCREASE_SUCCESS_COUNT); 
		break;
    
    case GF_CLOSE:
    case GF_CANCEL:
    	HaveDown = FALSE;
{
#if ENABLETRACE
GSSiExitProg (152);
#endif
    	return FALSE;
}
    	
    default:
{
#if ENABLETRACE
GSSiExitProg (152);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (152);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ShowItem (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (155);
#endif
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		SetPrompt (PRMT_SHOW,TRUE);  
        SetCurs (LoadCursor (hInst,"SHOW"),FALSE);
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        ClientToScreen (CurView->hWnd,(LPPOINT)&MousePoint);
	    SetCursorPosGM (MousePoint.x,MousePoint.y,0);
	    DisplayPickedItems (hWnd,NumPicked,TRUE,0,0,TRUE);
	    DynDlgOn(TRUE);
		ShowDynWindows ();
		setDoPaint( TRUE);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (155);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (155);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL HideItem (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (156);
#endif
{HDC hDC;
 char key; 
 long	LastPicked=LONG_MAX;
 POINT	MousePoint;
 DPOINT	BasePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	        setDoPaint( FALSE);
		    AddToHighlightList (PickList[NumPicked-1].Refno,
		    					&PickList[0],FALSE);
		    LastPicked = PickList[NumPicked-1].Refno;
		    CurView->CurZoomAreaRef = 0;
		    RedisplayViewport(FALSE,FALSE);
        }
		setDoPaint( TRUE);
		break;

    case WM_RBUTTONUP:
    	if (LastPicked<LONG_MAX)
    	{
		    AddToHighlightList (PickList[NumPicked-1].Refno,&PickList[0],TRUE);
		    CurView->CurZoomAreaRef = 0;
		    RedisplayViewport(FALSE,FALSE);
    	}
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (156);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (156);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ColorClass (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (158);
#endif
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor=lastPickedColor;
 short	idesc, newob;
 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	AddLBUTTON = TRUE;
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
   	return TRUE;
}
 }
    	
 if (ForAllVis)
 {  
 	BOOL	First=TRUE, Redisplay=FALSE;
 	
 	if (Message == WM_LBUTTONUP)
 	{
	    if(GetColor(hWnd,&NewColor))
	    {   
	    	
	    	SetCVTFromVis ();
	    	for (idesc=1;idesc<3201;idesc++) 
	    	{
				if (CurView->CurVisType[idesc])
				{   
					if (First)
					{   
						First = FALSE;
				    	newob = NextNewObject(TRUE);
				    	if (Function ==  GF_COLOR_CLASSTEXT_ALLVIS)
				    	{
				    		CurView->NewObjectSetTextColor[newob] = TRUE;
				    		CurView->NewObjectTextColor[newob].rgbtRed = GetRValue (NewColor);
				    		CurView->NewObjectTextColor[newob].rgbtGreen = GetGValue (NewColor);
				    		CurView->NewObjectTextColor[newob].rgbtBlue = GetBValue (NewColor);
				    	}
				    	else
							SetClassColor (newob,CurView->CurVisType[idesc],NewColor);
				    }
					CurView->NewObjectMap[idesc]=newob+1;   
				}
			}
		    CurView->CurZoomAreaRef = 0;
		    Redisplay=TRUE;
	    }
		PostMessage(hWnd, GF_CLOSE,0, 0L);
		if (Redisplay)
			RedisplayViewport(FALSE,FALSE);
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
		return TRUE;
}
	} 
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
	return FALSE;
}
 }


 switch (Message)
   {
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
	        setDoPaint( FALSE);
        PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;
        if (NumPicked > 0)
        {
        	if(GetColor(hWnd,&NewColor))
        	{   
        		if (!CurView->NewObjectMap[PickList[NumPicked-1].Desc])
        			newob = NextNewObject(TRUE);
        		else 
        			newob = CurView->NewObjectMap[PickList[NumPicked-1].Desc] - 1;
        		CurView->NewObjectMap[PickList[NumPicked-1].Desc] = newob + 1;
		    	if (Function ==  GF_COLOR_CLASSTEXT)
		    	{
		    		CurView->NewObjectSetTextColor[newob] = TRUE;
		    		CurView->NewObjectTextColor[newob].rgbtRed = GetRValue (NewColor);
		    		CurView->NewObjectTextColor[newob].rgbtGreen = GetGValue (NewColor);
		    		CurView->NewObjectTextColor[newob].rgbtBlue = GetBValue (NewColor);
		    	}
		    	else
	        		SetClassColor (newob,PickList[NumPicked-1].Type,NewColor);
			    CurView->CurZoomAreaRef = 0;
				ConfigChangesMade = TRUE;
			    RedisplayViewport(FALSE,FALSE);
	        }
        }
		setDoPaint( TRUE);
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL LineTypeClass (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (160);
#endif
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel,rtn=FALSE;
 
    	

 switch (Message)
   {
   	case GF_INIT:
 		if (ForAllVis)
	 	{
	    	int	idesc, newob;
		    	
	    	newob = NextNewObject(TRUE);
	        {
		    	DLGPROC lpfnLINETYPEMsgProc; 
		    	int		nRc;
			    	
				setDoPaint( FALSE); 
		        lpfnLINETYPEMsgProc = MakeProcInstance((DLGPROC)LINETYPEMsgProc, hInst);
		        nRc = DialogBox(hInst, (LPSTR)"LINETYPE", hWnd, lpfnLINETYPEMsgProc);
		        FreeProcInstance(lpfnLINETYPEMsgProc);
				setDoPaint( TRUE);
		        if (!nRc)
		        {
					PostMessage(hWnd, GF_CLOSE,0, 0L); 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (160);
																							#endif
		        	return TRUE;  
																							}
		        }
		        SetClassLineType (newob,1,NewWidth,NewStyle,NewProPen,NewColor);
	        }
		    SetCVTFromVis ();
	    	for (idesc=1;idesc<3201;idesc++) 
	    	{
				if (CurView->CurVisType[idesc]>0 && CurView->CurVisType[idesc]<3)
					CurView->NewObjectMap[idesc]=newob+1;
			}
		    CurView->CurZoomAreaRef = 0;
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		    RedisplayViewport(FALSE,FALSE);
		} 
		else
  		   	AddLBUTTON = TRUE;
	 	break;

    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
       BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;
        if (NumPicked > 0)
        {
	    	DLGPROC lpfnLINETYPEMsgProc; 
	    	int		nRc;
	    	
			setDoPaint( FALSE); 
	        lpfnLINETYPEMsgProc = MakeProcInstance((DLGPROC)LINETYPEMsgProc, hInst);
	        nRc = DialogBox(hInst, (LPSTR)"LINETYPE", hWnd, lpfnLINETYPEMsgProc);
	        FreeProcInstance(lpfnLINETYPEMsgProc);
	        if (!nRc) break;
        	{
        		if (!CurView->NewObjectMap[PickList[NumPicked-1].Desc]) 
        			CurView->NewObjectMap[PickList[NumPicked-1].Desc] = NextNewObject(TRUE)+1;
	        	SetClassColor (CurView->NewObjectMap[PickList[NumPicked-1].Desc]-1,PickList[NumPicked-1].Type,NewColor);
	        	SetClassLineType (CurView->NewObjectMap[PickList[NumPicked-1].Desc]-1,PickList[NumPicked-1].Type,
	        					  NewWidth,NewStyle,NewProPen,NewColor);
			    CurView->CurZoomAreaRef = 0;
				ConfigChangesMade = TRUE;
			    RedisplayViewport(FALSE,FALSE);
	        }
			setDoPaint( TRUE);
        }
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (160);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (160);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL CreateTAG (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
 char key;
 int	Item, nRc;
 POINT	MousePoint;
 DPOINT	BasePoint;
 static	BOOL	HaveDown=FALSE;  
 short	rtn=1;

 switch (Message)
   {
   	case GF_INIT:     
       	AddLBUTTON = TRUE;   
       	DoSave = FALSE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;
	    	UnlockCursor ();
			MousePoint = BasePtToScreenPt (&BasePoint);
	    } 
	    else
        	BasePoint=ScreenPtToBasePt (MousePoint); 
        EnlargeScreen (0,0);
        setDoPaint( FALSE);
        PickItems (hWnd,BasePoint);
        if (SelectTAGTemplate())
        
        {	 
        	 
          	 if (!CreateTAGBox (CurView->hDC,BasePoint,NumPicked-1))
          	 	break;
          	 if (TAGBox.Flags.AutoEdit)
	         {
		          DLGPROC lpfnTAGEDITMsgProc;
			
			   	  setDoPaint( FALSE);
		          lpfnTAGEDITMsgProc = MakeProcInstance((DLGPROC)TAGEDITMsgProc, hInst);
		          nRc = DialogBox(hInst, (LPSTR)"TAGEDIT", hWndMain, lpfnTAGEDITMsgProc);
		          FreeProcInstance(lpfnTAGEDITMsgProc);
	         } 
	         else
	         	nRc = 1;
          	 if (nRc)
          	 {
				 SetDisplayMode (CurView->hDC, GF_MAPMODE);
				 if (ResetTAGBox (CurView->hDC,2))
				 {
					 if (TAGBox.Flags.AutoMove)
		           	 	DrawTAG(hWnd, CurView->hDC,2,FALSE);  
		           	 else
		           	 {
		           	 	DrawTAG(hWnd, CurView->hDC,FALSE,FALSE); 
						SaveFullWindowBitmap (hWndMain);
		           	 }
		           	 if (TAGBox.Flags.AutoMove)
		           	 {  
		           	 	TBNum = 0;
					 	AddGraphicsFunction (hWnd,GF_MOVE_TAG,0); 
					 	DoSave=TRUE;
					 } 
					 else 
					 {
			        	SaveTAG(0);
						rtn = GF_INCREASE_SUCCESS_COUNT; 
					 } 
				}
		    	/* if (TAGBox.before) 
		    	 {
		    		DeleteObject(TAGBox.before);
		    		TAGBox.before=0;
		    	 }*/
			 } 
			 
        }  
//        PostMessage(hWnd, WM_KEYDOWN,0,'\x02');
		break;

    case GF_COMPLETE:
		rtn = GF_INCREASE_SUCCESS_COUNT; 
    	break;
    	
    default:
    	return (FALSE);
    }
    return (rtn);
}

BOOL PickScatterPoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (148);
#endif
{
 char key;
 POINT	MousePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
		ThemePickScatterDiagram (MousePoint);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (148);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (148);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeSVChangeColor (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1231);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		SetPrompt (PRMT_SET_CLASS_COLOR,TRUE);  
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		ThemeSVChangeClr (MousePoint);
		ThemeDisplayLegend(3,0);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1231);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1231);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL PickImage (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (712);
#endif
{
 char key;
 POINT	MousePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT: 
   		HaveDown=TRUE;
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
		ThemePickImage (MousePoint);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (712);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (712);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL PanZoomTarget (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
#if ENABLETRACE
{GSSiEnterProg (119);
#endif
{static LONG	DownPoint;  
 static	BOOL	HaveDownPoint=FALSE, HaveTimer=FALSE; 
 static	POINT	MovePoint;    
 static	HANDLE	hCmd=0; 
 static long	ValueColor1, ValueColor2;  
 static short	OriginalFunction;
 LPSTR	pCmd;
 DPOINT	PanPoint, BasePoint;
 POINT	MousePoint,ScreenPoint,ClientPoint;
 static	POINT	ButtonPoint;
 RECT	Rect;   
 HDC	hDC;
 LPVIEWPORT	SaveVP;
 char key;
 BOOL HaveBox;  
 short	ii, iopt, iview, ilayer,InfoBoxID,VPID;
 int	MinMouseMove=5; 
 static	UINT	CurrentPrompt;
 static	short	LastItem=0;     
 static	BOOL	InPanZoom=FALSE;   
 BOOL	InOnePick=FALSE, ClearFullDisplay=FALSE,SkipMenuDisplay=FALSE;   
 char	SymName[34];   
 short	parent;
 static	RECT	LastBoxRect, SelectedItemRect;
 static	int	LastClass=-1;
 double	Offset;
 BOOL	QOnMouseMove=TRUE;
  
  if (InPanZoom)
{
#if ENABLETRACE
GSSiExitProg (119);
#endif
  	return FALSE; 
} 
	OriginalFunction = Function;
	if (Function == GF_ONEPICK)
		InOnePick = TRUE;
  if (CurView->pTheme)
  { 
  	if (CurView->pTheme->ID == PF_COORD_DISPLAY)
  	{
  		if (Function == GF_RUN_COORDDISPLAY_MACRO || Function == GF_PAN_ZOOM_TARGET)  
  		{
  			Function = GF_RUN_COORDDISPLAY_MACRO;
  			goto Next;                           
  		}
  	}
  	else if (CurView->pTheme->ID == GF_DISTANCE_THEME)
  	{
  		if (Function == GF_RUN_DISTDISPLAY_MACRO || Function == GF_PAN_ZOOM_TARGET)  
  		{
  			Function = GF_RUN_DISTDISPLAY_MACRO;
  			goto Next;                           
  		}
  	} 
  	else if (CurView->pTheme->ID != GF_BOUNDS_DISPLAY_THEME)
  	{
  		if (Function == GF_RUN_LEGEND_MACRO || Function == GF_PAN_ZOOM_TARGET)  
  		{
  			Function = GF_RUN_LEGEND_MACRO;
  			goto Next;                           
  		}
  	}
  }
  if ((Function == GF_AUTOPICK || Function == GF_AUTOPICK_NOZOOM || Function == GF_PAN_ZOOM_TARGET || Function == GF_AUTO_IDENTIFY) && !CurView->BoundsDisplayVP ||
  	  Function == GF_TOOLBAR && InExpand)
{
#if ENABLETRACE
GSSiExitProg (119);
#endif
  	  	return (FALSE);
}
Next:
  InPanZoom = TRUE;
  switch (Message)
   { 
    case GF_REINIT:
   	case GF_CLOSE:
		GSSiGlobFree (&hCmd);  
   		GSSiGlobFree (&hLastCmd);
		LastClass = -1;
		ClearVehicleInfoRect ();
		if (hLastBox)
		{
			RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
			DestroySavedScreen (&hLastBox,0);
		}
		if (hLastHLT)
		{
    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
    		DestroySavedScreen (&hLastHLT,0);
    	} 
    	if (Message == GF_CLOSE)
   			goto RtnFalse;	
   		
   	case GF_INIT:
		ClearVehicleInfoRect ();
    	if (hLastBox) 
    	{   
    		RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
    		DestroySavedScreen (&hLastBox,0);  
    		GSSiGlobFree (&hLastCmd);
			if (hLastHLT)
			{
	    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
	    		DestroySavedScreen (&hLastHLT,0);
	    	}
    	}  
   	 
   		switch (Function)
   		{
   			case GF_PAN_ZOOM_TARGET:
   				CurrentPrompt = PRMT_PANZOOM1;
	   			SetCurs (0,FALSE);
   				break; 
   			case GF_AUTO_IDENTIFY:
   			case GF_AUTOPICK:
			case GF_AUTOPICK_NOZOOM:
   				CurrentPrompt = PRMT_AUTOPICK;
		        SetCurs (LoadCursor (hInst,"SHOW"),FALSE);
				GetCursorPos (&MovePoint);
				ScreenToClient (hWnd,&MovePoint); 
				if (PtInRect (&CurView->ScreenRect,MovePoint))
				{
		    		HaveTimer = SetTimer(hWnd, GF_PAN_ZOOM_TARGET, (UINT)10, (TIMERPROC) 0); 
		    	}
   				break;
   			default:
	   			CurrentPrompt = 0; 
	   			SetCurs (0,FALSE);
	   			break;
	   	}
   	    SetPrompt (CurrentPrompt,TRUE);
   	    if (Function == GF_ONEPICK)
   	    {   
			if (CurView->pTheme)
			{ 
				if (CurView->pTheme->ID == PF_COORD_DISPLAY)
						Function = GF_RUN_COORDDISPLAY_MACRO;
				else if (CurView->pTheme->ID == GF_DISTANCE_THEME)
						Function = GF_RUN_DISTDISPLAY_MACRO; 
				else if (CurView->pTheme->ID == GF_COMPARE_VIEWPORTS_THEME)
						Function = GF_SHOW_DIFFERENCE;
				else if (CurView->pTheme->ID != GF_BOUNDS_DISPLAY_THEME &&
						 CurView->pTheme->ID != GF_PROFILE_THEME &&
						 CurView->pTheme->ID != GF_PROFILE_LINK_THEME &&
						 CurView->pTheme->ID != GF_NORTH_ARROW_THEME)
						Function = GF_RUN_LEGEND_MACRO;
			}
			InOnePick = TRUE;
			GetCursorPos (&ButtonPoint);
			ScreenToClient (hWnd,&ButtonPoint); 
			goto LButUp;
		//	PostMessage(hWnd, GF_EXECUTE,0, 0L);
   	    }
   		else if (Function == GF_TOOLBAR)
   			LoadVPToolBar ();	
   		break;

    case GF_COMPLETE:
	    if (CursorIsLocked)
	    { 
			MousePoint = BasePtToScreenPt (&CurrentPoint);
	    	UnlockCursor (); // release snap if not in digitize
			SetCurs (0,FALSE);
//		    PostMessage (hWnd,WM_LBUTTONDOWN,1,MAKELONG(MousePoint.x,MousePoint.y)); 
//		    PostMessage (hWnd,WM_LBUTTONUP,0,MAKELONG(MousePoint.x,MousePoint.y)); 
		    break;
    	}
    	if (wParam != GF_ZOOM_RECT)
    		break; 
		BlockVehicleDisplay = 0;
    	EnlargeScreen (0,0);
    	HaveDownPoint = FALSE;
   		CurrentPrompt = PRMT_PANZOOM1;
        SetPrompt (CurrentPrompt,TRUE);
    	HaveBox = TRUE;
    	ShowZoomBoxClear (CurView->hDC,&HaveBox); //pViewports[11]
		if (CurView->DisplayedFullScreen) 
		{
			ClearFullDisplay = TRUE;
			MakeVPFullScreen (CurView->ID,0);
		}
    	SetCurView ( pViewports[CurView->BoundsDisplayVP-1]);
		{
			DPOINT	DPoints[4];
			RectToDPoints (&CurView->ScreenRect,DPoints);
			ZoomBoxScale = ldistp (ZoomBoxPoly[0],ZoomBoxPoly[2])/ldistp (DPoints[0],DPoints[2]);
			CurView->CurZoomAreaRef = 0;
			ZoomToPointAndScale (ZoomBoxMidPoint,ZoomBoxScale,FALSE);
		}
//		SetScaleAndMidpointFromBounds (&ZoomBoxRect,CurView->Rotation,&CurView->MidPointW,&CurView->Scale);
//		CurView->NewBounds = ZoomBoxRect;  
		ZoomBoxRect.xmn = 2;
		ZoomBoxRect.xmx = 1;
		DisplayCycle++;  
		ClearCurrentCD ();
	    SetBounds (hWnd,CurView->hDC); 
	    DisplayCycle--;
		CurView->CurZoomAreaRef = 0;
        if (ClearFullDisplay)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
		else
			PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);
//	    RedisplayViewport(FALSE,FALSE);
		CurView->WindowIsZoomed = TRUE;
       	break;
    
    case WM_RBUTTONUP:
		if (Function == GF_AUTO_IDENTIFY && hLastBox) 
		{
	    	RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
	    	DestroySavedScreen (&hLastBox,0); 
			GSSiGlobFree (&hLastCmd);
	    	if (NumPicked > 0)
	    		goto NextPickItem;
	    }
    	goto RtnFalse;
    case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
    	ButtonPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		if (Function == GF_AUTO_IDENTIFY)
		{   
			if (hLastBox)
			{   
	    		RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
	    		DestroySavedScreen (&hLastBox,0);
    		} 
			if (hLastHLT)
			{
	    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
	    		DestroySavedScreen (&hLastHLT,0);
	    	}
			if (hLastCmd)
			{
		    	pCmd = GlobalLock (hLastCmd); 
				_fmemmove (&PickList[0],pCmd,sizeof(PICKDATA));
				GSSiGlobUlFree (&hLastCmd);   
				NumPicked = 1;
				DisplayPickedItems (hWnd,-1,TRUE,0,0,FALSE);
			}
			break;
		}
    /*	if (hLastBox)
    	{
			LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);
				
			if (PtInRect (&SelectedItemRect,MovePoint))
				GlobalUnlock (hLastBox);
			else
			{
				GlobalUnlock (hLastBox);
	    		hDC=GetDC(hWnd);
	    		RestoreScreen2 (hDC, hLastBox,0,FALSE);
	    		ReleaseDC (hWnd,hDC);
	    		DestroySavedScreen (&hLastBox,0);
	    		GSSiGlobFree (&hLastCmd);  
		   		if (Function == GF_PAN_ZOOM_TARGET)
		   			CurrentPrompt = PRMT_PANZOOM1;
		   		else
		   			CurrentPrompt = 0;

    		}
    	}*/
			
	case GF_EXECUTE:
LButUp:
    	if (!hLastBox && (Function == GF_RUN_CLASS_MACRO  || InOnePick)) 
		{
			if (Function == GF_RUN_CLASS_MACRO)
			{
		   		CurrentPrompt = 0;
	    	    SetPrompt (CurrentPrompt,TRUE);
	    	    NumPicked = ClassMacro (ButtonPoint,1,&hLastBox,&LastBoxVP,&hLastCmd); 
	    	    LastVP = SetLastVP (LastBoxVP);
			}
			else if (Function == GF_RUN_COORDDISPLAY_MACRO)
			{
		   		CurrentPrompt = 0;
	    	    SetPrompt (CurrentPrompt,TRUE);
	    	    NumPicked = CoordDisplayMacro (ButtonPoint); 
			}
			else if (Function == GF_RUN_DISTDISPLAY_MACRO)
			{
		   		CurrentPrompt = 0;
	    	    SetPrompt (CurrentPrompt,TRUE);
	    	    NumPicked = DistDisplayMacro (ButtonPoint); 
			} 
			else if (Function == GF_RUN_LEGEND_MACRO)
			{
		   		CurrentPrompt = 0;
	    	    SetPrompt (CurrentPrompt,TRUE);
	    	    NumPicked = LegendMacro (ButtonPoint); 
			} 
			else if (Function == GF_SHOW_DIFFERENCE)
			{
		   		CurrentPrompt = 0;
	    	    SetPrompt (CurrentPrompt,TRUE);
	    	    NumPicked = GetValueDifference (ButtonPoint,&hLastBox,&LastBoxVP,&ValueColor1,&ValueColor2); 
			} 
			//else if ((InfoBoxID = PtInInfoBoxRect (ButtonPoint,&VPID)))
			//	ProcessInfoBoxPickMacroFile (InfoBoxID);
    	    else
	    	{
				short	SaveMaxPick = MaxPick; 
				LPVISLIST	pSavePickList1=CurView->pPickList1,pSavePickListManual=CurView->pPickListManual;
				BOOL		SaveDisableZoomMacro=CurView->DisableZoomMacro;
				HANDLE		handle;
				
				NumPicked = 0;
		   		if (Function == GF_PAN_ZOOM_TARGET)
		   			CurrentPrompt = PRMT_PANZOOM1;
	    	    SetPrompt (CurrentPrompt,TRUE);
			    HaltMapDisplay (FALSE,TRUE);
			    BasePoint=WinPtSToBasePt(POINTtoPOINTS(ButtonPoint));
			    SetPickAp(0); 
		    	if (InOnePick)
				{
		    		MaxPick = GetGlobalLVal2 ("[%ONEPICKMAXPICK]",MAXPICKITEMS); 
					MaxPick = min (MaxPick,MAXPICKITEMS-4);
					CurView->pPickList1 = 0;
					CurView->pPickListManual = 0;
					ProcessText (CurView->RButPickInit);
					if (!CurView->pPickList1)
						CurView->pPickList1 = pSavePickList1;
					if (!CurView->pPickListManual)
						CurView->pPickListManual = pSavePickListManual;
				}
		    	else
		    	{  
					MaxPick = MAXPICKITEMS-4;
					StopAtFirstInPickMacro = TRUE; 
				}
			    PickItems2 (hWnd,BasePoint,FALSE,FALSE,TRUE);
				SaveVP=CurView;  
				for (iview=0;iview<*pNumViewports;iview++)
				{
					SetCurView ( pViewportsD[iview]);
					if (CurViewActive() &&
						(CurView->DisplayInParent && CurView->Parent == SaveVP->ID) ||
						(CurView->Type == SUBVIEWPORT && CurView->Parent == SaveVP->ID))
			    		PickItems2 (hWnd,BasePoint,TRUE,FALSE,TRUE);  
			    }
			    CurView = SaveVP;
			    for (ilayer=0;ilayer<CurView->NumFiles;ilayer++)
			    	if (CurView->FileType[ilayer]==9 && CurVis && CurVis->FileIsVisible[ilayer]) 
				    {
	    			    _fmemset (&PickList[NumPicked],0,sizeof(PICKDATA));
						PickList[NumPicked].Desc = -1; 
						PickList[NumPicked].Refno = LONG_MAX;
						PickList[NumPicked].OffDist = FLT_MAX;
						PickList[NumPicked].ViewID = CurView->ID;
						PickList[NumPicked].ConfigID = CurrentConfig;
						PickList[NumPicked].FileNum = ilayer;
						_fstrcpy (PickList[NumPicked].Prefix,"%DTM");   
						_fstrcpy (PickList[NumPicked++].UDI,CurView->FileID[ilayer]);   
					}
				if (InOnePick)
				{
					if (CurView->pPickList1 != pSavePickList1)
					{
						if (CurView->pPickListManual == CurView->pPickList1)
							CurView->pPickListManual = 0;
						handle = CurView->pPickList1->hVisList;
						GSSiGlobUlFree (&handle);
						CurView->pPickList1 = pSavePickList1;
					}
					CurView->pPickListManual = pSavePickListManual;
					CurView->DisableZoomMacro = SaveDisableZoomMacro;
					SelectVisList (TRUE);
				}
			    _fmemset (&PickList[NumPicked],0,sizeof(PICKDATA));
				PickList[NumPicked].Desc = -1; 
				PickList[NumPicked].Refno = LONG_MAX;
				PickList[NumPicked].OffDist = FLT_MAX;
				PickList[NumPicked].ViewID = CurView->ID;
				PickList[NumPicked].ConfigID = CurrentConfig;
				PickList[NumPicked].FileNum = -1;
				PickList[NumPicked].PickedPoint = BasePoint;
				_fstrcpy (PickList[NumPicked++].Prefix,"%VIEWPORT");   
				StopAtFirstInPickMacro = FALSE;
				MaxPick = SaveMaxPick;
			} 
		    while (NumPicked && !hLastBox)
		    {
		    	HANDLE	hTxt=GSSiGlobAlloc (  62,GMEM_MOVEABLE,4096);
		    	LPSTR	pTxt=GlobalLock (hTxt); 
		    	BOOL	ShowMenu=FALSE; 
		    	HWND	hwnd=0;
		    	
		    	if (InOnePick)  
		    	{
		    		hwnd = hWnd;
		    		ShowMenu = TRUE;
		    	}
		    	SetGlobalValue ("%ZOOMSELECTIONTEXT","");
		    	if ((Function == GF_AUTOPICK || Function == GF_AUTOPICK_NOZOOM || Function == GF_AUTO_IDENTIFY || Function == GF_PAN_ZOOM_TARGET))
		    	{
			    	ZoomBoxRect = PickList[NumPicked-1].Rect; 
			    	ProcessPickedItem (NumPicked-1,FALSE);
					PickList[NumPicked-1].Rect = ZoomBoxRect;
			    }
				GSSiGlobFree (&hLastCmd);
		    	hLastCmd = GSSiGlobAlloc (  63,GMEM_MOVEABLE,4096);
		    	pCmd = GlobalLock (hLastCmd); 
		    	*pCmd = 0;
				if (hLastHLT)
				{
		    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
		    		DestroySavedScreen (&hLastHLT,0);
		    	}
			    if (DisplayPickedItems (hwnd,NumPicked,ShowMenu,pCmd,0,TRUE))
			    {   
			    	if (ShowMenu)
			    		NumPicked = 1;
				    if (GetGlobalCVal ("[%ZOOMSELECTIONTEXT]",pTxt,""))
				    {   
				    	MenuDisplayed = FALSE;
				    	if (StringEndsWith (pCmd,"ZOOM"))
				    	{
					   		CurrentPrompt = PRMT_YTBMESSZOOM; 
					    	ExpandText (pCmd); 
					    }
				    	else if (!_fstrnicmp (pCmd,"ZOOM",4))
					   		CurrentPrompt = PRMT_YTBMESSZOOM; 
					   	else
					   		CurrentPrompt = PRMT_YTBMESS;
				    	if (MenuDisplayed) 
				    		GSSiGlobUlFree (&hLastCmd);
				    	else  
				    	{
							hLastBox = YellowTextBox (hWnd,pTxt,ButtonPoint,0,0,FALSE,0);
							LastBoxVP = CurView;
				    	    LastVP = SetLastVP (LastBoxVP);
							{
								LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);
								
								LastBoxRect = pSaveScreen->Rect;
								GlobalUnlock (hLastBox);
							}
						}
						LastItem = NumPicked; 
	        			SetPrompt (CurrentPrompt,FALSE);
				        NumPicked = 1;
				        if (hLastCmd) 
				        	GlobalUnlock (hLastCmd);
					}
					else
						GSSiGlobUlFree (&hLastCmd);
				}
				else
					GSSiGlobUlFree (&hLastCmd);
				GSSiGlobUlFree (&hTxt);
				NumPicked--; 
			}
    	} 
        if (!hLastBox || (Function == GF_TOOLBAR || InOnePick))
        	goto LButUp2;
   		hDC=GetDC(hWnd);
		RestoreScreen2 (hDC, hLastBox,0,FALSE);
		ReleaseDC (hWnd,hDC); 
		DestroySavedScreen (&hLastBox,0);
	    HaltMapDisplay (FALSE,TRUE);

		if (hLastHLT)
		{
    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
    		DestroySavedScreen (&hLastHLT,0);
    	}
    	if (Function == GF_PAN_ZOOM_TARGET)
    	{
    		if (CurView->DisplayedFullScreen) 
    		{
    			ClearFullDisplay = TRUE;
    			MakeVPFullScreen (CurView->ID,0);
    		}
    		SetCurView ( pViewports[CurView->BoundsDisplayVP-1]);
    	}
		if (!hLastCmd)
			break;  
DoCmd:
		pCmd = GlobalLock (hLastCmd);
		if (!_fstrncmp (pCmd,"$GFLIST",6))
		{   
			GSSiGlobUlFree (&hLastCmd);
			if (wParam == MK_CONTROL)
			{
				char file[MAX_PATH];
				sprintf(file, "%s\\%s.txt", "[%DL]fundir", CurTheme->SQL);
				GMEdit(CurView->hWnd,file);
			}
			else
				DisplayGFList();
			break;
		}
		if (!CurrentConfig)
			SetConfig (1); 
		if (_fstrncmp (pCmd,"$GFCMD(",7)) 
		{
			ExpandText (pCmd); 
		}    
		else
		{   
			POINT	p = CurrentLBUTDOWNLoc;
			
			ExpandText (pCmd);     
            ClientToScreen (CurView->hWnd,(LPPOINT)&p);
			SetCursorPos (p.x,p.y);
			hAddGraphicsFun2 = hAddGraphicsFun;
			hAddGraphicsFun = 0;
	        PostMessage(hWndMain, GSSI_ADDGF, 0, CurView->ID); 
		}
		iopt = 0;   
		if (!_fstrnicmp (pCmd,"ZOOM",4))
			iopt=1;
		else if (!_fstrnicmp (pCmd,"PMF(",4))
			iopt=2;
		else if (!_fstrnicmp (pCmd,"MENU(",5))
			iopt=3;
		switch (iopt)
		{           
			case 1:
			{
				LPSTR	pParen = _fstrchr (pCmd,'(');
				
				if (pParen)
				{
					HANDLE hMem = GSSiGlobAlloc (  64,GMEM_MOVEABLE,2048);
					LPSTR pStr=GlobalLock (hMem), pEnd=MatchLev (++pParen,')');
					
					if (pEnd)
						*pEnd = 0;		
					_fstrcpy (pStr,pParen);
					ExpandText (pStr);
					GSSiGlobUlFree (&hMem);
				}   
				SetScale ("REMOVE");
				DisplayCycle++;
				CurView->NewBounds = ZoomBoxRect;
				ZoomBoxRect.xmn = 2;
				ZoomBoxRect.xmx = 1; 
				ClearCurrentCD ();
				ExpandBounds (&CurView->NewBounds,LocationOffset); 
/*			    SetBounds (hWnd,CurView->hDC);
			    DisplayCycle--;
				CurView->WindowIsZoomed = TRUE; 
				CurView->CurZoomAreaRef = 0;*/
		   		CurrentPrompt = PRMT_PANZOOM1;
		        SetPrompt (CurrentPrompt,TRUE);
				GSSiGlobUlFree (&hLastCmd);
				ZoomToRect (CurView->NewBounds,FALSE);
		      /*  if (ClearFullDisplay)
					PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
				else
					PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);*/
			}
			break;
			case 2: 
			{
				LPSTR	pNext=pCmd+4, pEnd;
				
				if ((pEnd = MatchLev (pNext,')')))
				{   
					HANDLE	hSavePM=GSSiGlobAlloc (  65,GMEM_MOVEABLE,256);
					LPSTR	pSavePM = GlobalLock (hSavePM);   
					LPVIEWPORT	pSaveVP=CurView;
					
					_fstrcpy (pSavePM,CurView->PickMacroFile);
					*pEnd = 0;
					_fstrcpy (CurView->PickMacroFile,pNext);
			   		CurrentPrompt = PRMT_YTBMESS2;
			        SetPrompt (CurrentPrompt,TRUE);
				    DisplayPickedItems (CurView->hWnd,LastItem,TRUE,0,pNext,TRUE);  
				    CurView = pSaveVP;
				    _fstrcpy (CurView->PickMacroFile,pSavePM);
				    GSSiGlobUlFree (&hSavePM); 
				}  
			} 
			break;
			case 3:
			{
				POINT	position;
				HMENU	hMenu;
				HANDLE	hPopups;
                
                pCmd += 5;
                hMenu = LoadToolBarMenu (pCmd,&hPopups); 
                if (hMenu)
                {
				   	GetCursorPos (&position);
					TrackPopupMenu (hMenu,TPM_RIGHTBUTTON,position.x,position.y,0,hWndMain,0);
					DestroyUserPopups (&hPopups); 
				}
			} 
			

			break; 
		}
		GSSiGlobUlFree (&hLastCmd);
    	break;
LButUp2:
		if (InOnePick && Function != GF_SHOW_DIFFERENCE)
			PostMessage(hWnd, GF_CLOSE,OriginalFunction, CurView->ID); 

		if (Function == GF_TOOLBAR || InOnePick)
		{ 
			ClearSelectedToolbarIcons ();
			DisplayToolbarText (ButtonPoint,0,2);
			if (hCmd)
			{
				pCmd = GlobalLock (hCmd);
				//ExpandText (pCmd);     
				iopt = 0;   
				if (!_fstrnicmp (pCmd,"MENU(",5))
					iopt=3;
				switch (iopt)
				{   
					case 0:
					{
						long	ICmd;
			
						if (*pCmd == '|') 
							ExecuteCommandString (++pCmd);
						else
						{   
							long	SaveConfigID = ConfigID;
							
							SaveVP = CurView;
							SetViewport (*pCommandViewport); 
							AddGraphicsCmd (CurView->hWnd,pCmd,FALSE,0); 
							if (ConfigID == SaveConfigID)
								SetCurView ( SaveVP);   
							else
								SetViewport (*pCommandViewport);  
						}		
					/*	ICmd = GetCmdID (pCmd); 
						if (ICmd < 0)
						{   
							LPVIEWPORT SaveVP = CurView;
							ICmd = -ICmd;
							SetViewport (CommandViewport); 
							AddGraphicsCmd (CurView->hWnd,pCmd,FALSE,0); 
							if (!ConfigLoaded)
								CurView = SaveVP;
						}	
					    else if (ICmd)
			           		PostMessage(hWndMain, WM_COMMAND, ICmd, 0L);*/
			        }
			        break;
			        
					case 3:
					{
						POINT	position;
						HMENU	hMenu;
						HANDLE	hPopups;
		                
		                pCmd += 5;
		                hMenu = LoadToolBarMenu (pCmd,&hPopups); 
		                if (hMenu)
		                {
						   	GetCursorPos (&position);
							TrackPopupMenu (hMenu,TPM_RIGHTBUTTON,position.x,position.y,0,hWndMain,0);
							DestroyUserPopups (&hPopups); 
						}
					} 
					
		
					break; 
				}  
				GSSiGlobUlFree (&hCmd);
			}
			break;
		} 
		if (HaveDownPoint && (Function == GF_AUTOPICK || Function == GF_AUTOPICK_NOZOOM ))
		{
			MovePoint = ButtonPoint;
			wParam = GF_PAN_ZOOM_TARGET;
			SkipMenuDisplay = TRUE;
			goto DoPick;
		}
	    if (!HaveDownPoint || !(Function == GF_PAN_ZOOM_TARGET)) break;
			MovePoint = ButtonPoint;
			wParam = GF_PAN_ZOOM_TARGET;
			SkipMenuDisplay = TRUE;
			goto DoPick;
/*    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	HaveDownPoint = FALSE;
	    PanPoint = ScreenPtToBasePt (MousePoint); 
		if (CurView->DisplayedFullScreen) 
		{
			ClearFullDisplay = TRUE;
			MakeVPFullScreen (CurView->ID,0);
		}
	    SetViewport (CurView->BoundsDisplayVP); 
	    ClearCurrentCD ();
		CenterWindow (PanPoint,FALSE);
        if (ClearFullDisplay)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);*/
		break;

    case WM_LBUTTONDOWN:   
NoBox:    	 
    	ButtonPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		if (Function == GF_TOOLBAR)
		{   
			GSSiGlobFree (&hCmd);
	    	if (HaveTimer)
	    		KillTimer (hWnd,GF_PAN_ZOOM_TARGET);
			BlockSocketProcessing (FALSE);
	    	HaveTimer = FALSE;         
	    	if (hLastBox) 
	    	{
	    		hDC=GetDC(hWnd);
	    		RestoreScreen2 (hDC, hLastBox,0,FALSE);
	    		ReleaseDC (hWnd,hDC); 
	    		DestroySavedScreen (&hLastBox,0);
	    		GSSiGlobFree (&hLastCmd);
	    	}  
	   		CurrentPrompt = 0;
    	    NumPicked = 0;
    	    SetPrompt (CurrentPrompt,TRUE);
			DisplayToolbarText (ButtonPoint,&hCmd,0);
			LastBoxVP = 0; 
			break;
		}
    	else if (Function != GF_PAN_ZOOM_TARGET && Function != GF_AUTOPICK && Function != GF_AUTOPICK_NOZOOM  && Function != GF_AUTO_IDENTIFY) 
    		break; 
/*        if (CurView->pTheme)
        {
	    	if (hLastBox) 
	    	{
	    		hDC=GetDC(hWnd);
	    		RestoreScreen2 (hDC, hLastBox,0);
	    		ReleaseDC (hWnd,hDC); 
	    		DestroySavedScreen (&hLastBox,0);
	    		GSSiGlobFree (&hLastCmd);
	    	}  
	    }*/
    	HaveDownPoint = TRUE;
    	DownPoint = lParam;
    	SaveVP = CurView;
    	SetCurView ( pViewports[CurView->BoundsDisplayVP-1]);
    	Rect = CurView->ScreenRect;
    	SetCurView ( SaveVP);
	  	CurView->ZBRect = Rect;
		break;
    
    case GF_ENTER_VIEWPORT:
    	break;
    	    
   	case GF_REDRAW: 
   		if (Function == GF_TOOLBAR)
   			LoadVPToolBar ();	
   		DestroySavedScreen (&hLastHLT,0);
   	case GF_CLEAR: 
    case GF_EXIT_VIEWPORT:
    	if (HaveTimer)
    		KillTimer (hWnd,GF_PAN_ZOOM_TARGET);
		if (Message != GF_CLEAR && Message != GF_REDRAW)
			BlockSocketProcessing (FALSE);
    	HaveTimer = FALSE;         
		GSSiGlobFree (&hCmd);
    	if (hLastBox && LastBoxVP == CurView) 
    	{   
    		if (Message != GF_REDRAW && Message != GF_CLEAR)
    		{
	    		hDC=GetDC(hWnd);
	    		RestoreScreen2 (hDC, hLastBox,0,FALSE);
	    		ReleaseDC (hWnd,hDC);
	    	}
    		DestroySavedScreen (&hLastBox,0);  
    		GSSiGlobFree (&hLastCmd);
    	}  
		if (hLastHLT)
		{
    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
    		DestroySavedScreen (&hLastHLT,0);
    	}
		if (Function == GF_TOOLBAR)
			ClearSelectedToolbarIcons ();
    	break;
    	    
    case WM_MOUSEMOVE: 
    {
    	MovePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	if ((hLastBox || HaveTimer) && idist (MovePoint,LastMovePoint)<6)
    		break;
    	LastMovePoint = MovePoint;
/*		if (debugvalue < 0)
			debugvalue++;
		if (debugvalue == -1)
			debugvalue = -5;*/
    	if (wParam != MK_LBUTTON)
    		CurrentLBUTDOWNLoc = MovePoint;
//        SetPrompt (CurrentPrompt,TRUE); 
		if (Function == GF_TOOLBAR)
		{   
			if (DisplayToolbarText (MovePoint,0,2))
			{    
				GSSiGlobFree (&hCmd);
				if (hLastBox) 
				{
		    		RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
		    		DestroySavedScreen (&hLastBox,0);
		    		GSSiGlobFree (&hLastCmd); 
		    	}
				ClearSelectedToolbarIcons ();
		    	DisplayToolbarText (MovePoint,0,2);
		    }
		    if (!hLastBox) 
		    {
		    	if (HaveTimer)
		    		KillTimer (hWnd,GF_PAN_ZOOM_TARGET);
				BlockSocketProcessing (FALSE);
		    	if (!InDisplayProcessing)
		    		HaveTimer = SetTimer(hWnd, GF_PAN_ZOOM_TARGET, (UINT)GetGlobalLVal2 ("[%AUTOPICKDELAY]",500), (TIMERPROC) 0); 
		    	else
		    		HaveTimer = FALSE;
            }
            break;
		}
        if (*CurView->PickMacroFile || CurView->Type == MENUVIEWPORT)
        {
	    	if (HaveTimer)
	    		KillTimer (hWnd,GF_PAN_ZOOM_TARGET);
	    	HaveTimer = FALSE;
			BlockSocketProcessing (3);
	    	if (!ScreenIsRegistered(hLastBox,0))
	    		hLastBox = 0;
/*	    	if (hLastBox)
	    	{
				LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);
				
				if (PtInRect (&SelectedItemRect,MovePoint))
					GlobalUnlock (hLastBox);
				else
				{
					GlobalUnlock (hLastBox);
		    		hDC=GetDC(hWnd);
		    		RestoreScreen2 (hDC, hLastBox,0,FALSE);
		    		ReleaseDC (hWnd,hDC);
		    		DestroySavedScreen (&hLastBox,0);
		    		GSSiGlobFree (&hLastCmd);  
			   		if (Function == GF_PAN_ZOOM_TARGET)
			   			CurrentPrompt = PRMT_PANZOOM1;
			   		else
			   			CurrentPrompt = 0;
	    		}
	    	}*/
	    	else if (Function == GF_PAN_ZOOM_TARGET)
	   			CurrentPrompt = PRMT_PANZOOM1;
	   		else
	   			CurrentPrompt = 0;
	   		if (CurrentPrompt || !hLastBox)
				SetPrompt (CurrentPrompt,TRUE);
	    }
    	if ((Function == GF_AUTOPICK ||Function == GF_PAN_ZOOM_TARGET/* || Function == GF_AUTO_IDENTIFY 5/8/2003  */) && wParam == MK_LBUTTON && HaveDownPoint)
    	{   
	    	if ((labs ((long)LOWORD(lParam) - (long)LOWORD(DownPoint))) >= MinMouseMove || 
	    		(labs ((long)HIWORD(wParam) - (long)HIWORD(DownPoint))) >= MinMouseMove)
	    	{   
	    		if (CurView->BoundsDisplayVP < 0 || CurView->BoundsDisplayVP > *pNumViewports) 
	    			CurView->BoundsDisplayVP = 0;	

	    		if (CurView->BoundsDisplayVP && pViewports[CurView->BoundsDisplayVP-1]->OrthoRes < 0 && 
	    			pViewports[CurView->BoundsDisplayVP-1]->WindowZoomedToOrtho)
	    			HaveDownPoint = FALSE;
    			else
    			{
			    	if (hLastBox) 
			    	{
			    		hDC=GetDC(hWnd);
			    		RestoreScreen2 (hDC, hLastBox,0,FALSE);
			    		ReleaseDC (hWnd,hDC);
			    		DestroySavedScreen (&hLastBox,0); 
			    		GSSiGlobFree (&hLastCmd);
			    	}
					AddGraphicsFunction (hWnd, GF_ZOOM_RECT,0);
					BlockVehicleDisplay = 1;
			       	SendMessage (hWnd,WM_LBUTTONDOWN,0L,DownPoint);
			    }
	    	}  
	    }
	    else if ((*CurView->PickMacroFile  || CurView->Type == MENUVIEWPORT)/* && !hLastBox*/)
	    {   
	    	if (HaveTimer)
	    		KillTimer (hWnd,GF_PAN_ZOOM_TARGET);
	    	HaveTimer = FALSE; 
	    	if (!InDisplayProcessing)
				HaveTimer = SetTimer(hWnd, GF_PAN_ZOOM_TARGET, (UINT)GetGlobalLVal2 ("[%AUTOPICKDELAY]",200), (TIMERPROC) 0);
        }
	}
		BlockSocketProcessing (FALSE);
    	break;
    
    case WM_TIMER: 
    {
		ii=1;
DoPick:
        if (wParam != GF_PAN_ZOOM_TARGET)
        	goto RtnFalse;
    	if (HaveTimer)
    		KillTimer (hWnd,GF_PAN_ZOOM_TARGET);
    	HaveTimer = FALSE;         
    	if (InDisplayProcessing)
    		goto RtnFalse;
		BlockSocketProcessing (4);
		GetCursorPos (&ScreenPoint); 
		MovePoint = ScreenPoint;
		ScreenToClient (hWnd,&MovePoint); 
		if (Function == GF_SHOW_DIFFERENCE)
			ii=1;
		if (Function == GF_AUTOPICK || Function == GF_AUTOPICK_NOZOOM || Function == GF_AUTO_IDENTIFY)
		{   
			if (ScreenIsRegistered(hLastBox,0))
			{   
				LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);
					
				if (PtInRect (&SelectedItemRect,MovePoint))
				{
					GlobalUnlock (hLastBox);
					BlockSocketProcessing (FALSE);
					break;
				}
				GlobalUnlock (hLastBox);
	    		RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
	    		DestroySavedScreen (&hLastBox,0);
	    		GSSiGlobFree (&hLastCmd);
				if (hLastHLT)
				{
		    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
		    		DestroySavedScreen (&hLastHLT,0);
		    	}
				UpdateAllVehicles(TRUE);

    		} 
    		else
    			hLastBox = 0; 
			CurrentPrompt = PRMT_AUTOPICK;
   			SetPrompt (CurrentPrompt,FALSE);
		} 

    	if (hLastBox && Function != GF_RUN_CLASS_MACRO) 
    	{
    		hDC=GetDC(hWnd);
    		RestoreScreen2 (hDC, hLastBox,0,FALSE);
    		ReleaseDC (hWnd,hDC);
    		DestroySavedScreen (&hLastBox,0); 
    		GSSiGlobFree (&hLastCmd);
    	}
    	if ((Function == GF_AUTOPICK || Function == GF_AUTOPICK_NOZOOM || Function == GF_AUTO_IDENTIFY || Function == GF_PAN_ZOOM_TARGET))
    	{
			short	SaveMaxPick = MaxPick; 
			long	AreaNum=0;  
			HANDLE	hHighlightArea=0,hPolyPartLen=0;
			int		nHighlightAreaPoints=0,nPoly,Type;
			
			if (PtInInfoBoxRect (MovePoint,&VPID))
			{
				BlockSocketProcessing (FALSE);
				break;  
			}
			if (!PtInRect (&CurView->DrawRect,MovePoint)) 
			{
				BlockSocketProcessing (FALSE);
				break;  
			}
	   		if (Function == GF_PAN_ZOOM_TARGET)
	   			CurrentPrompt = PRMT_PANZOOM1;
    	    SetPrompt (CurrentPrompt,TRUE);
		    HaltMapDisplay (FALSE,TRUE);
		    BasePoint=ScreenPtToBasePt(MovePoint);
		    SetPickAp(0); 
			MaxPick = GetGlobalLVal2 ("[%AUTOPICKMAXPICK]",MAXPICKITEMS); 
			MaxPick = min (MaxPick,MAXPICKITEMS-4);
			PickPointBase = BasePoint; 
			NumPicked = 0;
			while (hHighlightArea = GetNextHighlightArea (AreaNum++,0,&Type,&nHighlightAreaPoints,&nPoly,&hPolyPartLen,&Offset,0))
			{
			    LPMNMXCORD	lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea); 
			    MNMXCORD	SaveBounds;
			    HPDPOINT	pPoints;
			    BOOL		PickedOffsetLine;   
			    short		SaveCD = CurrentDesc;
			    
	            lpRect++;
	            pPoints = (HPDPOINT)lpRect; 

				SaveBounds = CurView->WBounds; 
			    DBoundsInit (&CurView->WBounds);
			    AddDPointToMinMax (&PickPointBase,&CurView->WBounds);
				InflateBounds (&CurView->WBounds,PickApW); 
				CurrentDesc = -1;  
				SetGlobalValue ("%PREFIX","%HLTAREA");
				SetGlobalValueLong ("%UDI",AreaNum);
				PickedOffsetLine = PickPolylineD (pPoints,nHighlightAreaPoints,0,2,0,0,0);
				CurView->WBounds = SaveBounds; 
				CurrentDesc = SaveCD;
			    GSSiGlobUlFree (&hHighlightArea); 
			    GSSiGlobUlFree (&hPolyPartLen); 
			    if (PickedOffsetLine)
				{
					BlockSocketProcessing (FALSE);
			    	break;
				}
            }
			if (!NumPicked)
			{
				LPVISLIST	pSavePickList1=CurView->pPickList1,pSavePickListManual=CurView->pPickListManual;
				BOOL		SaveDisableZoomMacro=CurView->DisableZoomMacro;
				HANDLE		handle;

				StopAtFirstInPickMacro = GetGlobalBVal2 ("[%STOPATFIRSTINPM]",TRUE);
				SaveVP = CurView;
				if (CurView->Type == SUBVIEWPORT)
					SetViewport (SaveVP->Parent);
				CurView->pPickList1 = 0;
				CurView->pPickListManual = 0;
				ProcessText (CurView->MeterInit);
				if (!CurView->pPickList1)
					CurView->pPickList1 = pSavePickList1;
				if (!CurView->pPickListManual)
					CurView->pPickListManual = pSavePickListManual;
		    	PickItems2 (hWnd,BasePoint,QOnMouseMove,TRUE,TRUE);
				if (CurView->pPickList1 != pSavePickList1)
				{
					if (CurView->pPickListManual == CurView->pPickList1)
						CurView->pPickListManual = 0;
					handle = CurView->pPickList1->hVisList;
					GSSiGlobUlFree (&handle);
					CurView->pPickList1 = pSavePickList1;
				}
				CurView->pPickListManual = pSavePickListManual;
				CurView->DisableZoomMacro = SaveDisableZoomMacro;
		    	CurView = SaveVP;
			    
				SaveVP=CurView;  
				for (iview=0;iview<*pNumViewports;iview++)        //pViewports[iview]
				{
					SetCurView ( pViewportsD[iview]);
					if (CurViewActive() &&
						(CurView->DisplayInParent  && CurView->Parent == SaveVP->ID) ||
						(CurView->Type == SUBVIEWPORT && CurView->Parent == SaveVP->ID))
			    		PickItems2 (hWnd,BasePoint,TRUE,FALSE,TRUE);  
			    }
			    SetCurView ( SaveVP);
				StopAtFirstInPickMacro = FALSE;
			}
			MaxPick = SaveMaxPick; 
		}
		else if (Function == GF_TOOLBAR)
		{
	   		CurrentPrompt = 0;
    	    NumPicked = 0;
    	    SetPrompt (CurrentPrompt,FALSE);
			hLastBox = DisplayToolbarText (MovePoint,0,1);
			LastBoxVP = CurView;
    	    LastVP = SetLastVP (LastBoxVP);
		}
		else if (Function == GF_RUN_CLASS_MACRO)
		{
//	   		CurrentPrompt = 0;
//    	    SetPrompt (CurrentPrompt,TRUE);
    	    NumPicked = ClassMacro (MovePoint,2,&hLastBox,&LastBoxVP,&hLastCmd); 
    	    LastVP = SetLastVP (LastBoxVP);
			if (PickList[0].Refno != LastClass)
			{
				if (hLastBox)
				{
					RestoreScreen2 (CurView->hDC, hLastBox,0,FALSE);
					DestroySavedScreen (&hLastBox,0);
				}
			}
			LastClass = PickList[0].Refno;
		}
		else if (Function == GF_RUN_COORDDISPLAY_MACRO)
		{
	   		CurrentPrompt = 0;
    	    SetPrompt (CurrentPrompt,TRUE);
    	    NumPicked = CoordDisplayMacro (MovePoint); 
		}
		else if (Function == GF_RUN_DISTDISPLAY_MACRO)
		{
	   		CurrentPrompt = 0;
    	    SetPrompt (CurrentPrompt,TRUE);
    	    NumPicked = DistDisplayMacro (MovePoint); 
		}
		else if (Function == GF_RUN_LEGEND_MACRO)
		{
	   		CurrentPrompt = 0;
    	    SetPrompt (CurrentPrompt,TRUE);
    	    NumPicked = LegendMacro (MovePoint); 
		}
		else if (Function == GF_SHOW_DIFFERENCE)
		{
	   		CurrentPrompt = 0;
    	    SetPrompt (CurrentPrompt,TRUE);
    	    NumPicked = GetValueDifference (MovePoint,&hLastBox,&LastBoxVP,&ValueColor1,&ValueColor2); 
		} 
		if (!NumPicked)
			ClearVehicleInfoRect ();
NextPickItem:
	    while (NumPicked && !hLastBox)
	    {
	    	HANDLE	hTxt=GSSiGlobAlloc (  66,GMEM_MOVEABLE,4096);
	    	LPSTR	pTxt=GlobalLock (hTxt);
	    	
	    	SetGlobalValue ("%ZOOMSELECTIONTEXT","");
	    	if (Function == GF_PAN_ZOOM_TARGET)
	    	{
		    	ZoomBoxRect = PickList[NumPicked-1].Rect; 
		    	ProcessPickedItem (NumPicked-1,FALSE);
				PickList[NumPicked-1].Rect = ZoomBoxRect;
		    }
		    else if (Function == GF_AUTO_IDENTIFY)
		    {   
		    	MNMXCORD	HRect=PickList[NumPicked-1].Rect;
		    	
	    		if (hLastHLT)
	    		{
		    		RestoreScreen2 (CurView->hDC, hLastHLT,0,FALSE);
		    		DestroySavedScreen (&hLastHLT,0);
		    	} 
		    	if (PickList[NumPicked-1].IsDispersed)
		    		AddDPointToMinMax (&PickList[NumPicked-1].BeginPoint,&HRect);
		    	if ((HaveHLTRect = BoundsToWinRect (&HRect,&HLTRect))) 
		    	{   
		    		if (IsRectEmpty (&HLTRect))
		    			InflateRect (&HLTRect,1,1);
		    		if (IntersectRect (&HLTRect,&CurView->DrawRect,&HLTRect))
		    		{
		    			InflateRect (&HLTRect,1,1);
		    			hLastHLT = SaveScreen2 (CurView->hWnd,CurView->hDC,HLTRect,0,0);
		    		} 
		    	}
		    	ForceHLT = TRUE;
		    	ProcessPickedItem (NumPicked-1,2);
		    	ForceHLT = FALSE;
		    }
		    GSSiGlobFree (&hLastCmd); 
	    	hLastCmd = GSSiGlobAlloc (  67,GMEM_MOVEABLE,4096);
	    	pCmd = GlobalLock (hLastCmd); 
	    	*pCmd = 0;  
	    	if (Function == GF_AUTO_IDENTIFY)
	    	{   
	    		short	HLTRectWidth = HLTRect.right - HLTRect.left;
	    		short	HLTRectHeight = HLTRect.bottom - HLTRect.top;
	    		long	HLTRectSize = (long)HLTRectWidth * (long)HLTRectHeight;  
	    		RECT	TBRect;
	    		long	YTBSize;
				short SaveVisListOpt = VisListOpt; 
				char	AutoID[80];
	    		
    			_fmemmove (pCmd,&PickList[NumPicked-1],sizeof(PICKDATA));
			
				VisListOpt = 0;
				if (PickList[NumPicked-1].Desc > 0)  
					GetSymbolName (PickList[NumPicked-1].Desc, SymName,&parent, -(PickList[NumPicked-1].FileNum+1),0);
				VisListOpt = SaveVisListOpt;
				if (PickList[NumPicked-1].Desc < 0)
				{
	    			sprintf (pTxt,"%s %s","Highlight Area",PickList[NumPicked-1].UDI);  
					_fstrcpy (SymName,"Highlight Area"); 
				} 
				*pTxt = 0;
				sprintf (AutoID,"[%%AUTOIDTEXT%s]",SymName); 
				if (GetGlobalCVal (AutoID,pTxt,""))
					ExpandText (pTxt);
				if ( !*pTxt)
				{   
					*AutoID = 0;
					switch (PickList[NumPicked-1].Type)
					{
						case 1:
							_fstrcpy (AutoID,"[%AUTOIDTEXTPOINT]");
							break;
						case 2:
							_fstrcpy (AutoID,"[%AUTOIDTEXTLINE]");
							break;
						case 3:
							_fstrcpy (AutoID,"[%AUTOIDTEXTAREA]");
							break;
						case 4:
							_fstrcpy (AutoID,"[%AUTOIDTEXTTEXT]");
							break;
						case 5:
							_fstrcpy (AutoID,"[%AUTOIDTEXTCURVE]");
							break;
					}
					if (*AutoID) 
						GetGlobalCVal (AutoID,pTxt,"");
					ExpandText (pTxt);
				}
				if (!*pTxt)
				{
					GetGlobalCVal ("[%AUTOIDTEXT]",pTxt,"");
					ExpandText (pTxt);
				}
				if (!*pTxt && !_fstrcmp (PickList[NumPicked-1].Prefix,"REFNO") && !SetRefno)
	    			sprintf (pTxt,"%s\r\n%s:%ld",SymName,PickList[NumPicked-1].Prefix,PickList[NumPicked-1].Refno);
	    		if (!*pTxt) 
	    			sprintf (pTxt,"%s\r\n%s:%s",SymName,PickList[NumPicked-1].Prefix,PickList[NumPicked-1].UDI);  
    			YellowTextBox (hWnd,pTxt,MovePoint,&TBRect,0,TRUE,0);  
    			YTBSize = (long)(TBRect.right - TBRect.left) * (long)(TBRect.bottom - TBRect.top);
	    		if (PickList[NumPicked-1].Desc > 0 && HLTRectSize < YTBSize * 3)
	    		{
	    			if (HLTRectWidth < HLTRectHeight)
	    			{
	    				if (abs(HLTRect.left - CurView->DrawRect.left) < abs(HLTRect.right - CurView->DrawRect.right))
		    				MovePoint.x = HLTRect.right + 4 + (TBRect.right - TBRect.left)/2;
		    			else
		    				MovePoint.x = HLTRect.left - 4 - (TBRect.right - TBRect.left)/2;
		    		}
		    		else 
	    			{
	    				if (abs(HLTRect.top - CurView->DrawRect.top) < abs(HLTRect.bottom - CurView->DrawRect.bottom))
		    				MovePoint.y = HLTRect.bottom + 2 + (TBRect.bottom - TBRect.top)/2;
		    			else
		    				MovePoint.y = HLTRect.top - 2 - (TBRect.bottom - TBRect.top)/2;
		    		} 
		    	}
				hLastBox = YellowTextBox (hWnd,pTxt,MovePoint,0,0,TRUE,0); 
				{
					LPSAVESCREEN	pSaveScreen=(LPSAVESCREEN)GlobalLock (hLastBox);
					
					LastBoxRect = pSaveScreen->Rect;
					GlobalUnlock (hLastBox);
				}
				MousePoint = MovePoint; 
		        ClientToScreen (CurView->hWnd,(LPPOINT)&MousePoint);
	    	//	SetCursorPosGM (MousePoint.x,MousePoint.y,0);
				LastBoxVP = CurView;
	    	    LastVP = SetLastVP (LastBoxVP); 
		    	GlobalUnlock (hLastCmd);
		    	if (NumPicked > 1)  
   					CurrentPrompt = PRMT_AUTOPICK3; 
   				else
	   				CurrentPrompt = PRMT_AUTOPICK2;
       			SetPrompt (CurrentPrompt,TRUE);
	    	}
		    else if (DisplayPickedItems (0,NumPicked,FALSE,pCmd,0,TRUE))
		    { 
			    if (GetGlobalCVal ("[%ZOOMSELECTIONTEXT]",pTxt,""))
			    {   
			    	MenuDisplayed = SkipMenuDisplay;
			    	if (StringEndsWith (pTxt,"(ZOOM)"))
			    	{
				    	short l=_fstrlen (pTxt);
				    	pTxt[l-6] = 0; 
						SetGlobalValue ("%ZOOMSELECTIONTEXT",pTxt);
				   		CurrentPrompt = PRMT_YTBMESSZOOM; 
				    }
			    	else if (StringEndsWith (pCmd,"ZOOM"))
			    	{
				   		CurrentPrompt = PRMT_YTBMESSZOOM; 
				    	ExpandText (pCmd); 
				    }
			    	else if (!_fstrnicmp (pCmd,"ZOOM",4))
				   		CurrentPrompt = PRMT_YTBMESSZOOM; 
				   	else
				   		CurrentPrompt = PRMT_YTBMESS;
			    	if (!MenuDisplayed) 
			    	{
						//RECT	ClientRect;
						int		w,h;
						float	move=0.5;
						POINT	MidPt,pt=BasePtToScreenPt (&PickList[NumPicked-1].PickedPoint);
						BOOL	UseVehMenu=FALSE;

						if (PickList[NumPicked-1].Type == 1 || PickList[NumPicked-1].Type == 4)
							pt=BasePtToScreenPt (&PickList[NumPicked-1].BeginPoint);
						BoundsToScreenRect (&PickList[NumPicked-1].Rect,&SelectedItemRect);
						//GetClientRect (hWnd,&ClientRect);
						//MidPt = RectMid (&ClientRect);
						MidPt = RectMid (&CurView->ScreenRect);
						if (!strncmp (pTxt,"|VM|",4))
						{
							UseVehMenu = TRUE;
							memmove (pTxt,&pTxt[4],strlen(pTxt-4));
						}
						if (UseVehMenu && hWndVehStatus)
						{
							SetVehicleInfo (pTxt,PickList[NumPicked-1].UDI,TRUE);
						}
						else
						{
							RECT	ScreenRect;

							GetWindowRect (hWndMain,&ScreenRect);
							YellowTextBox (hWnd,pTxt,pt,&LastBoxRect,(LPRECT)1,FALSE,0); 
							IntersectRect (&SelectedItemRect,&SelectedItemRect,&LastBoxRect);
							w = LastBoxRect.right - LastBoxRect.left;
							h = LastBoxRect.bottom - LastBoxRect.top;
							if (pt.x > MidPt.x)
							{
								//LastBoxRect.left -= w * move;
								//LastBoxRect.right -= w * move;
								LastBoxRect.left = max (LastBoxRect.left -  w * move,ScreenRect.left);
								LastBoxRect.right = LastBoxRect.left + w;
							}
							else
							{
								//LastBoxRect.left += w * move;
								//LastBoxRect.right += w * move;
								LastBoxRect.right = min (LastBoxRect.right +  w * move,ScreenRect.right);
								LastBoxRect.left = LastBoxRect.right - w;
							}
							if (pt.y < MidPt.y)
							{
								//LastBoxRect.top += h * move;
								//LastBoxRect.bottom += h * move;
								LastBoxRect.bottom = min (LastBoxRect.bottom +  h * move,ScreenRect.bottom);
								LastBoxRect.top = LastBoxRect.bottom - h;
							}
							else
							{
								//LastBoxRect.top -= h * move;
								//LastBoxRect.bottom -= h * move;
								LastBoxRect.top = max (LastBoxRect.top -  h * move,ScreenRect.top);
								LastBoxRect.bottom = LastBoxRect.top + h;
							}
							hLastBox = YellowTextBox (hWnd,pTxt,pt,0,&LastBoxRect,FALSE,4);  
							LastBoxVP = CurView;
			    			LastVP = SetLastVP (LastBoxVP);
						}
					}
					LastItem = NumPicked; 
        			SetPrompt (CurrentPrompt,FALSE);
			        NumPicked = 1; 
			        GlobalUnlock (hLastCmd);
					if (SkipMenuDisplay)
					{
						GSSiGlobUlFree (&hTxt);
						BlockSocketProcessing (FALSE);
						goto DoCmd;
					}
				}
				else
					GSSiGlobUlFree (&hLastCmd);
			}
			else
			{
				GSSiGlobUlFree (&hLastCmd);
			}
			GSSiGlobUlFree (&hTxt);
			NumPicked--; 
		}
    } 
		BlockSocketProcessing (FALSE);
    	break;
    
    default:
    	goto RtnFalse;
    }
   	InPanZoom = FALSE;
{
#if ENABLETRACE
GSSiExitProg (119);
#endif
    return (TRUE); 
}
RtnFalse:
	InPanZoom = FALSE;
{
#if ENABLETRACE
GSSiExitProg (119);
#endif
	return (FALSE);
}
	
#if ENABLETRACE
}
#endif
}

BOOL ThemeActivate (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1253);
#endif
{	LPVIEWPORT SaveView, DisplayView=0;
	int	iview, i, n;

 switch (Message)
   {
    case GF_EXECUTE:
    case WM_LBUTTONUP:
		HaltReport=0;
   	    if (CurView->pTheme)
   	    {
   	    	CurTheme=CurView->pTheme;
   	    	CurTheme->IsActive = TRUE; 
		    UnloadReport (&CurView->hReport);  
   	    	
			if (CurTheme->ClassType < 3 &&
			    CurTheme->ID != GF_BOUNDS_DISPLAY_THEME && 
			    CurTheme->ID != GF_DISTANCE_THEME &&
			    CurTheme->ID != GF_COORDGRID_THEME &&  
			    CurTheme->ID != GF_DYNAMIC_SEG_THEME &&
			    CurTheme->ID != GF_PROFILE_THEME &&
				CurTheme->ID != GF_PROFILE_LINK_THEME &&
				CurTheme->ID != GF_TRANSFORM_THEME && 
				CurTheme->ID != GF_POLYINFO_THEME && 
				CurTheme->ID != GF_GRAPHICS_FUNCTION_THEME &&
				CurTheme->ID != GF_OFFSETAREA_THEME &&
				CurTheme->ID != GF_AREA_IN_MASK_THEME &&
				CurTheme->ID != GF_2D_THEME)
			{   
				*CurTheme->RefValChar = 0;
		   		CurTheme->WantDataPass=TRUE;
				CurTheme->ComputeClassBoundaries=TRUE;
			}
			else
			{   
			   	CurTheme->WantDataPass=CurTheme->Recompute;//10/17/03
				CurTheme->ComputeClassBoundaries=FALSE;
			}
			SaveView=CurView;
			for (iview=0;iview<*pNumViewports;iview++)
			{
				SetCurView (pViewports[iview]);
				CurView->Display=FALSE;
			}
			SetCurView (SaveView);
			CurView->Display = TRUE; 
			if (CurTheme->TargetViewport)
			{
		    	if (CurTheme->TargetViewport > *pNumViewports)
		    		CurTheme->TargetViewport = 1;
				SetCurView (pViewports[CurTheme->TargetViewport-1]);  
				DisplayView = CurView;
				for (i=0,n=0;i<CurView->NumThemes;i++)
				{ 
					if (CurView->pThemes[i]==CurTheme)
						n++;
				}
				if (!n)
					AddThemeToVP (CurView,CurTheme);
				CurView->Display = TRUE;  
				SetCurView (SaveView);
			}

		} 
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
        if (DisplayView)    
//        	InvalidateRect (hWndMain,0,TRUE);
        	PostMessage(hWndMain, WM_COMMAND,IDM_REDISPLAYVIEWPORTS, 0L); 
		break;

   	case GF_INIT:
        AddLBUTTON = TRUE;  
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1253);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1253);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeDeActivate (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1254);
#endif
{

 switch (Message)
   {
   	case GF_INIT: 
   	{
   		LPTHEME	SaveTheme = CurTheme;
   		
   	    if (CurView->pTheme)
   	    {   
   	    	CurTheme=CurView->pTheme;  
   	    	if (SaveTheme == CurTheme)
   	    		SaveTheme = 0;
   	    	if (Function == GF_THEME_REMOVE)
   	    	{
	    		LPVIEWPORT	SaveView = CurView; 
	    		
	    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
				RemoveThemeFromVP (CurView,CurTheme);
	    		SetCurView (SaveView);
	   	    	CloseObject (CurView->pTheme);
	   	    	CurView->pTheme = 0; 
	   	    	SetCurView (SaveView);
   	    	}
   	    	else
   	    	{
	   	    	CurTheme->IsActive = FALSE;
			   	CurTheme->WantDataPass=FALSE;
				CurTheme->ComputeClassBoundaries=FALSE; 
				DeletePointDispersionFile ();
				ResetViewport (TRUE,FALSE); 
				ResetOvrlapViewport (CurView->Rect,CurView->ID);
            }
            CurTheme = SaveTheme;
		}
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
		PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L); 
	}
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1254);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1254);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeEdit (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1252);
#endif
{	int nRc=0;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_REINIT:
    case WM_LBUTTONUP:
    case GF_EXECUTE:  
   	    if (CurView->pTheme)
   	    {
   	    	CurTheme=CurView->pTheme; 
   	    	nRc = EditTheme (hWnd,0);
			switch (CurTheme->ID)
			{   
					 
				case GF_SINGLE_VALUE_THEME:    
				case GF_HOTSPOT_THEME:
				case GF_STREET_ADDRESS_THEME:
				case GF_SINGLE_NONNUM_VALUE_THEME:
		        case GF_CITY_THEME:
				case GF_TRANSFORM_THEME: 
				case GF_2D_THEME:   
				case GF_COMPARE_VIEWPORTS_THEME:
				case GF_DOCUMENTS_THEME:
				case GF_POLYINFO_THEME: 
				case GF_GRAPHICS_FUNCTION_THEME:
				case GF_CONNECTION_LINE_THEME:
			          if (nRc && CurTheme->IsActive)
			          {
						HaveCurrentLBUTTON=TRUE;
			          	AddGraphicsFunction (CurView->hWnd, GF_THEME_ACTIVATE,0);
			          }	
				break; 
				
				case GF_DISTANCE_THEME:  
				case GF_COORDGRID_THEME:
				case GF_STREET_TEXT_THEME:
			    case GF_BOUNDS_DISPLAY_THEME:     
				case GF_OFFSETAREA_THEME:
				case GF_AREA_IN_MASK_THEME:
			    case GF_PROFILE_THEME:     
				case GF_PROFILE_LINK_THEME:
	        	case GF_CONTEST_THEME:
			    case GF_NETMARKER_THEME: 
			    case GF_DYNAMIC_SEG_THEME: 
		        case GF_CACHE_DISPLAY_THEME:
			    {   // dont remove without documenting why!!
			    	if (CurView->CurrentFunction == GF_THEME_EDIT)  
			        	PostMessage(hWnd, GF_CLOSE,0, 0L); 
                }  
				break;
				
			}
		} 
		if (nRc)
		{
			RedisplayViewports(FALSE);
			{
#if ENABLETRACE
				GSSiExitProg (1252);
#endif
				return GF_INCREASE_SUCCESS_COUNT;
			}
		}
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1252);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1252);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeLoad (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1255);
#endif
{	int nRc;
	static	LPVIEWPORT	SaveVP;

 switch (Message)
   {
   	case GF_INIT: 
		SaveVP = CurView;
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_COMPLETE:
    	PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;
	
	case GF_REINIT:	
    case WM_LBUTTONUP:
    case GF_EXECUTE:   
    	if (LoadTheme (hWnd,SaveVP))
    		if (CurView->pTheme->IsActive)
    			PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
    	PostMessage(hWnd, GF_CLOSE,0, 0L); 

   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1255);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1255);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL MoveIntersection (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (104);
#endif
{
 char key;
 POINT	MousePoint;
 POINT	CursorPoint; 
 DPOINT	MousePointW;
 static	POINT	StartPoint;
 DPOINT	BasePoint;
 static	int	NumItems;
 int	xmove, ymove, ii;
 static HANDLE	hOriginalItems, hCurrentItems;
 HANDLE	hSave;
 LPTHEME	pTheme;
 BOOL	ShowMoves=GetGlobalBVal2 ("[%SHOWMOVES]",FALSE);    
 static	HANDLE	SaveH1, SaveH2; 
 static	DPOINT	SavePPB; 
 static double	SavePAPW;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		NumPicked = NumItems = 0;
   		hCurrentItems = 0;
       	SaveHighlightList (&SaveH1,&SaveH2); 
   		ClearHighlightList(FALSE); 
   		SetPrompt (PRMT_MOVEINT1,TRUE);  

   		break;
    
    	
    case WM_LBUTTONDOWN:
    	break;
    case GF_CLOSE:  
/*		DeleteSavedItems(hCurrentItems); 
		if (hCurrentItems != hOriginalItems)
			DeleteSavedItems(hOriginalItems);
		hCurrentItems = hOriginalItems = 0;*/
   		ClearHighlightList(FALSE); 
    	RestoreHighlightList (SaveH1, SaveH2);
    	return FALSE;
        break;
        
    case WM_LBUTTONUP:
    {
    	short	SaveMaxPick, i;   
    	BOOL	SaveWOSP=WantOnlyShapePoints;
    	
		i = NumPicked;
	    while (i > 0)
	    {   
	    	i--;
		    RemoveFromHighlightList (PickList[i].Refno,2);
	    	ShowPickedItem (hWndMain,i); 
		    RemoveFromHighlightList (PickList[i].Refno,0);
	    	ShowPickedItem (hWndMain,i); 
	    }
   		ClearHighlightList(FALSE); 
   		if (Message == GF_CLOSE)
{
#if ENABLETRACE
GSSiExitProg (104);
#endif
   			return FALSE;
}
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
	    setDoPaint( FALSE); 
	    if (MaxPick > 1)
	    	WantOnlyShapePoints = TRUE; 
	    else
	    	WantOnlyShapePoints = GetGlobalBVal2 ("[%MOVEONLYSHAPEPOINTS]",FALSE);
	    PickItems (hWnd,BasePoint); 
		setDoPaint( TRUE); 
		if (!NumPicked)
		{
			WantOnlyShapePoints = SaveWOSP;
			break;                         
		}
		SavePAPW = PickApW; 
		
//		if (MoveShapePoints)  
		if (WantOnlyShapePoints)
			PickPointBase = PickList[0].PickedPoint;
		else
		{
			if (ldistp (PickPointBase,PickList[0].BeginPoint) <
				ldistp (PickPointBase,PickList[0].EndPoint))
				PickPointBase = PickList[0].BeginPoint;
			else
				PickPointBase = PickList[0].EndPoint;
		}
		WantOnlyShapePoints = SaveWOSP;
	    StartPoint = BasePtToScreenPt (&PickPointBase); 
	    MousePoint = StartPoint;
        ClientToScreen (CurView->hWnd,(LPPOINT)&MousePoint);
	    SetCursorPosGM (MousePoint.x,MousePoint.y,0);
		SavePPB = PickPointBase;
		i = NumItems = NumPicked;
	    while (i > 0 && i--)
	    {
		    AddToHighlightList (PickList[i].Refno,&PickList[i],TRUE);
	    	ShowPickedItem (hWndMain,i); 
//		    RemoveFromHighlightList (PickList[i].Refno,2);
	    }
/*		hOriginalItems = CreateSaveList(NumItems);
		hCurrentItems = hOriginalItems;
		MovePickListToSaveList (hOriginalItems); */
   		SetPrompt (PRMT_MOVEINT2,TRUE);  
		RedisplayViewports(FALSE);
	}
		break;

    case WM_MOUSEMOVE:
    	if (!ShowMoves) break;
    case WM_RBUTTONUP:
    	if (!NumItems) break;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    if (CursorIsLocked)
	    { 
	    	MousePointW = CurrentPoint;
	    	UnlockCursor ();
			MousePoint = BasePtToScreenPt (&MousePointW);
			CursorPoint = MousePoint;
        	ClientToScreen (CurView->hWnd,(LPPOINT)&CursorPoint);
			SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
	    }
	    else
			MousePointW = ScreenPtToBasePt(MousePoint);   
		EnlargeScreen (0,0);
		PickPointBase = SavePPB;
	    xmovePoly = MousePoint.x - StartPoint.x;
	    ymovePoly = MousePoint.y - StartPoint.y; 
	    PolySnapPoint = MousePointW;
   		MoveOnlyPickedPoint = GetGlobalLVal2 ("[%MOVEONLYPICKEDPOINT]",1); 
   		UseUserPickAp = FALSE;
		SystemPickAp = SavePAPW;
		if (Message == WM_MOUSEMOVE)
			AdjustPolygon (-103,0,0);  
		else
			AdjustPolygon (-3,0,0);  
   		UseUserPickAp = TRUE;

/*    	if (Message == WM_RBUTTONUP || wParam == MK_RBUTTON)
    	{   
    		pTheme = AddTheme (GF_UNDRAW_THEME);
    		DrawSavedItems(hCurrentItems,0);	//Undraws current position 
    		DeleteTheme (pTheme);  
    		ClearHighlightList(FALSE);
    		if (hCurrentItems != hOriginalItems) DeleteSavedItems(hCurrentItems);
    		hCurrentItems = CreateSaveList(NumItems);
    		pTheme = AddTheme (GF_MOVE_POLY_THEME);
    		pTheme->Xmove = xmove;
    		pTheme->Ymove = -ymove;
    		pTheme->ClassPnt[0] = MousePointW;
    		DrawSavedItems(hOriginalItems,hCurrentItems);// Draws new position and saves as current
    		hSave = hOriginalItems;
    		hOriginalItems = hCurrentItems;
    		hCurrentItems = hSave;
    		DeleteTheme (pTheme);

    	}*/
    	if (Message == WM_RBUTTONUP)
    	{
/*    	    WriteSavedItems (hCurrentItems);
    		NumItems = 0;
			DeleteSavedItems(hCurrentItems);
			DeleteSavedItems(hOriginalItems);
   			hCurrentItems = 0;
   			hOriginalItems = 0;  */
   			return (GF_INCREASE_SUCCESS_COUNT); 
   		}

    	break;


    default:
{
#if ENABLETRACE
GSSiExitProg (104);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (104);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL ZoomIn (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (150);
#endif
{
 char key;
 POINT	MousePoint;
 DPOINT	CenterPt;
 MNMXCORD	MinMax;
 double	Dist;
 BOOL	ClearFullDisplay=FALSE;
 static	BOOL	HaveDown;

 switch (Message)
   {
   	case GF_INIT:  
   		if (ScaleIsSet (TRUE))
   			return FALSE;
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		SetPrompt (PRMT_ZOOMIN,TRUE);  
        SetCurs (LoadCursor (hInst,"ZOOMIN"),FALSE);
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    CenterPt = ScreenPtToBasePt(MousePoint); 
 		if (CurView->Type == 7)
 		{
 			if (CurView->DisplayedFullScreen) 
			{
				ClearFullDisplay = TRUE;
				MakeVPFullScreen (CurView->ID,0);
			}
      		SetZoomVP (CurView->BoundsDisplayVP);
      	} 
	    Dist = CurView->WBounds.xmx - CurView->WBounds.xmn;  
	    Dist/=4;
	    MinMax.xmn = CenterPt.x - Dist;
	    MinMax.xmx = CenterPt.x + Dist;
	    Dist = CurView->WBounds.ymx - CurView->WBounds.ymn;
	    Dist/=4;
	    MinMax.ymn = CenterPt.y - Dist;
	    MinMax.ymx = CenterPt.y + Dist;   
   	    CurView->CurZoomAreaRef = 0;
	    ZoomToRect (MinMax,FALSE);
		if (ClearFullDisplay)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
		return (GF_INCREASE_SUCCESS_COUNT);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (150);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (150);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayMLBitmap (int ilevel)
{
	int	OutWidth = CurView->Rect.right-CurView->Rect.left+1;  
	int	OutHeight = CurView->Rect.bottom-CurView->Rect.top+1; 
	HBITMAP hBmpOld;
	RECT	BMRect, FullBMRect;
	short	ii;
	HANDLE	hTran;
	BITMAP	bm;
	BOOL	rtn=FALSE;
	
    if (!GetObject(hbmpMultiLevel[ilevel], sizeof(BITMAP), (LPSTR) &bm))
		return FALSE;
	FullBMRect.left = FullBMRect.top = 0;
	FullBMRect.right = bm.bmWidth;
	FullBMRect.bottom = bm.bmHeight;
	hTran = STRANBoundsToRect (&MultiLevelBounds[ilevel],&CurView->Rect);
	TranBoundsToRect (hTran,&CurView->WBounds,&BMRect);
	if (RectCompletelyInRect (&CurView->Rect,&BMRect))
	{
		HDC		hdcMemMap=CreateCompatibleDC(CurView->hDC);

		if ((hBmpOld = SelectObject(hdcMemMap,hbmpMultiLevel[ilevel])))
		{
			rtn=StretchBlt (CurView->hDC,//hDCTemp,
									 CurView->Rect.left,CurView->Rect.top,//0,0,
									 OutWidth,
									 OutHeight,  
						hdcMemMap,   
									 BMRect.left,BMRect.top,
									 BMRect.right-BMRect.left+1,
									 BMRect.bottom-BMRect.top+1,
									 SRCCOPY);  
			SelectObject (hdcMemMap,hBmpOld);
		}
		DeleteDC (hdcMemMap);
	}
    CloseTRANS2 (&hTran);
	return rtn;
}			

BOOL SmoothZoom (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (150);
#endif
{
	char key;
	POINT	MousePoint;
	DPOINT	CenterPt;
	MNMXCORD	MinMax;
	double	Dist;
	BOOL	ClearFullDisplay=FALSE;
	static	BOOL	HaveDown; 
	int	i;
	HDC	hDC;
	HBITMAP	hNewBM, hbmPrev;
	long	MultiZoomDelay;
	short	level;
	RECT	BMRectStart, BMRect; 
	int	ilevel;
    MSG         msg;   
    BOOL	ZoomAgain;
    double BWidth, BHeight;    
    DPOINT	CenterPoint;
    static	MNMXCORD	CurBounds;    
    static	BOOL	ZoomUp;
	short	kstate;
	int		ii;
	static	int	WantKey, TimerType;
	static	int	NumMovesPerScreen=1000;
	static	UINT	HaveTimer=0; 
	static	POINT	MovePoint,ScreenPoint;  
	DPOINT	CenPoint, MovePointD;
	static	double	MoveDist, MoveAZ;
	double	ScreenDist;
	static	RECT	SZVPRect;
	static	BOOL	NeedRedisplay;

 switch (Message)
   {
   	case GF_INIT:  
		HaveTimer = 0;
		NeedRedisplay = FALSE;
		InPan = FALSE;
   		if (ScaleIsSet (TRUE))
   			return FALSE;
       	AddLBUTTON = FALSE;  
       	HaveDown = FALSE;
   		SetPrompt (PRMT_ZOOMIN,TRUE);  
        SetCurs (LoadCursor (hInst,"ZOOMIN"),FALSE); 
  		MultiZoomVP = CurView->ID;
		SZVPRect = CurView->Rect;
		NumMovesPerScreen = max(10,GetGlobalLVal2 ("[%SCREENSPEED]",100));
		FactorInc = GetGlobalDVal2 ("[%SMOOTHZOOMFACTOR]",0.005);
   		break;
	
	case GF_EXIT_VIEWPORT:
		InPan = FALSE;
    	if (HaveTimer)
    		KillTimer (hWnd,HaveTimer);
		break;

    case GF_CLOSE:
    case GF_CANCEL:
		InPan = FALSE;
    	if (HaveTimer)
    		KillTimer (hWnd,HaveTimer);
		for (level=0;level<NumMultiZoomLevels;level++)
			GSSiDeleteObject (&hbmpMultiLevel[level]); 
		hDC = GetDC (hWndMain);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (hWndMain,hDC); 
		MultiZoomLevel = 0;    
	    ZoomToRect (CurBounds,TRUE);
		if (NeedRedisplay)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
    	return FALSE;
    	break;
    
	case WM_LBUTTONDBLCLK:
		//if (HaveDown)
		//	break;
 	    if (HaveTimer)
	    	KillTimer (hWnd,HaveTimer);
    	MousePoint=POINTStoPOINT(MAKEPOINTS(lParam)); 
	    CenterPoint = ScreenPtToBasePt(MousePoint); 
		InPan = FALSE;
		MousePoint = LastMovePoint = RectMid (&CurView->DrawRect);
		ClientToScreen (CurView->hWnd,&MousePoint);  
		SetCursorPosGM (MousePoint.x,MousePoint.y,0);
		CenterWindow (CenterPoint,TRUE);
		break;

    case WM_RBUTTONDOWN:  
 	    if (HaveTimer)
	    	KillTimer (hWnd,HaveTimer);
    	MovePoint=POINTStoPOINT(MAKEPOINTS(lParam)); 
		if (!PtInRect (&SZVPRect ,MovePoint))
			goto RtnFalse;
		HaveTimer = SetTimer(hWnd, GF_SMOOTH_ZOOM,500, (TIMERPROC) 0);
		TimerType = 1;
		break;
    case WM_LBUTTONDOWN:
 	    if (HaveTimer)
	    	KillTimer (hWnd,HaveTimer);
    	MovePoint=POINTStoPOINT(MAKEPOINTS(lParam)); 
		if (!PtInRect (&SZVPRect ,MovePoint))
			goto RtnFalse;
		TimerType = 2;
		HaveTimer = SetTimer(hWnd, GF_SMOOTH_ZOOM,500, (TIMERPROC) 0);
		break;
DoBtnDown:
	{   
	//	if (HaveDown)
	//		break;
	    if (Message == WM_RBUTTONDOWN)
		{
			WantKey = VK_RBUTTON;
	    	ZoomUp = TRUE;
		}
		else
		{
			WantKey = VK_LBUTTON;
	    	ZoomUp = FALSE;
		}
    	HaveDown = TRUE; 
		InPan = FALSE;
NextZoom:
		InSmoothZoom = TRUE;
		if (ZoomUp)
			MultiZoomBegin (1,0,1);  
		else
			MultiZoomBegin (0,1,1);  
		InSmoothZoom = FALSE;
	}
    	break;
    	
	case GF_MULTIZOOM_END:
 	    if (HaveTimer)
	    	KillTimer (hWnd,HaveTimer);
		ZoomAgain = FALSE;
		SetViewport (MultiZoomVP);
		hDC = GetDC (hWndMain); 
	    //hDCTemp = CreateCompatibleDC(0);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (hWndMain,hDC); 
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
		SelectClipRgn (CurView->hDC,CurView->hRgn);
		GSSiDeleteObject(&CurView->hRgn);
		if (InPan)
		{
			if (GetFocus() != hWndMain)
				break;
			BWidth = CurView->WBounds.xmx - CurView->WBounds.xmn;
			BHeight = CurView->WBounds.ymx - CurView->WBounds.ymn;
			ii=0;
			while (MoveDist && InPan)
			{
				while (GSSiPeekMessage(&msg, hWnd,0,0, PM_REMOVE))
				{   
           			if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_LBUTTONDBLCLK)
						InPan = FALSE;
          			TranslateMessage(&msg);
					DispatchMessage(&msg);
           			if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_LBUTTONDBLCLK)
               			goto ExitPan;
					if (msg.message == WM_MOUSEMOVE)
					{
    					MovePoint = POINTStoPOINT(MAKEPOINTS(msg.lParam));
    					if (idist (MovePoint,LastMovePoint)>5)
							goto ExitPan;
					}
				}
				CenterPoint = dnewpt (MinMaxMidPointD (&CurView->WBounds),MoveAZ, MoveDist);
				CurView->NewBounds.xmn = CenterPoint.x - BWidth/2;
				CurView->NewBounds.ymn = CenterPoint.y - BHeight/2;
				CurView->NewBounds.xmx = CenterPoint.x + BWidth/2;
				CurView->NewBounds.ymx = CenterPoint.y + BHeight/2;  
				CurView->WBounds = CurBounds = CurView->NewBounds; 
				NeedRedisplay = TRUE;
				if (!DisplayMLBitmap (0))
					goto NextPan;
				ii++;
				if (ii > 10000)
					ii++;
			}
ExitPan:
			InPan = FALSE;
			if (msg.message == WM_MOUSEMOVE)
				goto Done;
			goto RtnTrue;
		}

		MousePoint = RectMid (&CurView->DrawRect);
		ClientToScreen (CurView->hWnd,&MousePoint);  
		//SetCursorPosGM (MousePoint.x,MousePoint.y,0);
		MultiZoomDelay = GetGlobalLVal2 ("[%MULTIZOOMDELAY]",0); 
		//hNewBM = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight);
		hdcMemMap=CreateCompatibleDC(CurView->hDC);
		{   
			int	OutWidth = CurView->Rect.right-CurView->Rect.left+1;  
			int	OutHeight = CurView->Rect.bottom-CurView->Rect.top+1; 
			int	BeginLevel=NumMultiZoomLevels, EndLevel=0, LevelInc=-1;
		    //HBITMAP	hNewBM = CreateCompatibleBitmap(hDCTemp,OutWidth,OutHeight), hbmPrev;     
		    HDIB	hDIB;
			BITMAP	bm;
				
	       // GetObject(hNewBM, sizeof(BITMAP), (LPSTR) &bm);
	        
	       // hbmPrev = SelectObject(hDCTemp, hNewBM);
	       // GetObject(hbmPrev, sizeof(BITMAP), (LPSTR) &bm);    
	        if (ZoomUp) 
	        {
	        	BeginLevel=1;
	        	EndLevel=NumMultiZoomLevels+1;
	        	LevelInc=1;
			}
			ilevel = BeginLevel;
			while (ilevel != EndLevel)
			{ 
				double	Factor=1;
				HBITMAP hBmpOld;
				DWORD	starttime, endtime, elapsetime;
				 
				if (ZoomUp)
				{
					Factor = 0.5;
					FactorInc = -fabs(FactorInc);
				}
				else
					FactorInc = fabs(FactorInc);
	            BMRectStart = BMRect = CurView->Rect;
	                
	        GetObject(hbmpMultiLevel[ilevel-1], sizeof(BITMAP), (LPSTR) &bm);
//		        	hDIB = BitmapToDIB (hbmpMultiLevel[ilevel], 0);
//                    DestroyDIB(hDIB);
                hBmpOld = SelectObject(hdcMemMap,hbmpMultiLevel[ilevel-1]); 
				starttime=GetTickCount();   
				while (Factor >= 0.5 && Factor <= 1.0)
				{ 
					int		OldMode = 0;
					POINT	pt;
					BOOL	Fast=TRUE;
						if (!Fast)
					{
						SetStretchBltMode(CurView->hDC,HALFTONE); 
	
						SetBrushOrgEx (CurView->hDC,0,0,&pt);
					}
					NeedRedisplay = TRUE;
					ii=StretchBlt (CurView->hDC,
											 CurView->Rect.left,CurView->Rect.top,//0,0,
											 OutWidth,
											 OutHeight,  
							    hdcMemMap,   
											 BMRect.left,BMRect.top,
											 BMRect.right-BMRect.left+1,
											 BMRect.bottom-BMRect.top+1,
											 SRCCOPY);  
/*					    ii=BitBlt(CurView->hDC,  
											 CurView->Rect.left,CurView->Rect.top,
											 OutWidth,OutHeight,
											 hDCTemp, 0, 0,SRCCOPY);*/  
					if (OldMode)
						SetStretchBltMode(CurView->hDC,OldMode);
					Factor -= FactorInc;    
					BMRect = FactorRect (&BMRectStart,Factor);  
					CurBounds = FactorBounds (&MultiLevelBounds[ilevel-1],Factor);
					Wait (MultiZoomDelay); 
					kstate = GetAsyncKeyState (WantKey);
					if (!kstate)
						goto Exit;
					if (GetAsyncKeyState ('F')>>1)
						FactorInc *= 1.01;
					if (GetAsyncKeyState ('S')>>1)
						FactorInc *= 0.99;

 //                   while (PeekMessage(&msg, hWnd,0,0, PM_REMOVE))
 //                   {   
//                   	if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP)
//	                    	goto Exit;
//	                   	TranslateMessage(&msg);
//						DispatchMessage(&msg);
//                    }

				}
				ilevel += LevelInc;
				endtime=GetTickCount();
				elapsetime = endtime - starttime;
/*				if (elapsetime)
					FactorInc *= ((double)elapsetime)/500;
				else
					FactorInc /= 10;*/
			}
			//SelectObject (hDCTemp,hbmPrev);
			//GSSiDeleteObject (&hNewBM);
			//DeleteDC (hDCTemp);
		}
		ZoomAgain = TRUE;
Exit:   
		MultiZoomLevel = 0;
		hNewBM = SelectObject(hdcMemMap,hbmpOld); 
		DeleteDC (hdcMemMap);
		//GSSiDeleteObject (&hNewBM);
		hdcMemMap = 0;
		for (level=0;level<NumMultiZoomLevels;level++)
			GSSiDeleteObject (&hbmpMultiLevel[level]);    
		hDC = GetDC (hWndMain); 
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (hWndMain,hDC); 
			
		CurView->WBounds = CurBounds;  
		if (ZoomAgain) 
		{
			GetCursorPos (&MousePoint); 
			ScreenToClient (CurView->hWnd,&MousePoint);
			goto NextZoom; 
		}
		break;

    case WM_MOUSEMOVE: 
    {
    	MovePoint=POINTStoPOINT(MAKEPOINTS(lParam)); 
		if (!PtInRect (&SZVPRect ,MovePoint))
		{
			InPan = FALSE;
			goto RtnFalse;
		}
		if (InPan)
			break;
		kstate = GetAsyncKeyState (VK_LBUTTON);
		kstate = kstate >> 1;
		if (kstate)
			break;
    	if ((HaveTimer == 3 && idist (MovePoint,LastMovePoint)<6) || (HaveTimer == 1 || HaveTimer == 2))
    		break;
    	LastMovePoint = MovePoint;
 	    if (HaveTimer)
	    	KillTimer (hWnd,HaveTimer);
	    HaveTimer = FALSE;         
	    if (!InDisplayProcessing)
			HaveTimer = SetTimer(hWnd, GF_SMOOTH_ZOOM,500, (TIMERPROC) 0);
		TimerType = 3;
	}
    	break;
    
    case WM_TIMER: 
    {
		MNMXCORD	SaveBounds;
		double		SaveScale;

    	if (InDisplayProcessing)
    		goto RtnFalse;
        if (wParam != GF_SMOOTH_ZOOM)
        	goto RtnFalse;
    	if (HaveTimer)
    		KillTimer (hWnd,HaveTimer);
    	HaveTimer = 0; 
        switch (TimerType)
		{
		case 1:
			Message = WM_RBUTTONDOWN;
			goto DoBtnDown;
		case 2:
			Message = WM_LBUTTONDOWN;
			goto DoBtnDown;
		}

		SetViewport (MultiZoomVP);
		GetCursorPos (&ScreenPoint); 
		MovePoint = ScreenPoint;
		ScreenToClient (hWnd,&MovePoint);
		if (!PtInRect (&SZVPRect ,MovePoint))
			goto RtnFalse;
		MovePointD = ScreenPtToBasePt (MovePoint);
		CenPoint = MinMaxMidPointD (&CurView->WBounds);
		ScreenDist = ((CurView->Bounds.xmx - CurView->Bounds.xmn) + (CurView->Bounds.ymx - CurView->Bounds.ymn))/2;
		MoveAZ = getazd (&CenPoint,&MovePointD);
		MoveDist = ldistp (CenPoint,MovePointD) / NumMovesPerScreen;
		InPan = TRUE;
NextPan:
		SaveBounds = CurView->WBounds;
		SaveScale = CurView->Scale;
       	CurView->WBounds = CurView->NewBounds = FactorBounds (&CurView->WBounds,1.25);
		CurView->Scale *= 1.25;
		InSmoothZoom = TRUE;
		MultiZoomBegin (1,0,1);
		InSmoothZoom = FALSE;
		CurBounds = CurView->WBounds = CurView->NewBounds = SaveBounds;
		CurView->Scale = SaveScale;
	}
		break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
		if (HaveTimer)
			break;
Done:
		HaltMapDisplay(FALSE,FALSE); 
		ClearFullWindowBitmap (0);
		MultiZoomLevel = 0;    
    	HaveDown = FALSE;  
		for (level=0;level<NumMultiZoomLevels;level++)
			GSSiDeleteObject (&hbmpMultiLevel[level]);    
		hDC = GetDC (hWndMain);
		hDC = ScreenBufferDC (hWnd,hDC);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (hWndMain,hDC); 
    	if (CurView->HaveOrthos)
    		ClearViewport = FALSE;
		InSmoothZoom = FALSE;
	    ZoomToRect (CurBounds,TRUE);
		NeedRedisplay = FALSE;
		if (ClearFullDisplay)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
		break;

    default:
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (150);
#endif
    	return (FALSE);
}
    }
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (150);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL CreateTiles (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (150);
#endif
{
	POINT	MousePoint;
	DPOINT	CenterPt;
	MNMXCORD	MinMax;
	double	Dist;
	static	BOOL	HaveDown; 
	int	i;
	HDC	hDC;
	HBITMAP	hNewBM, hbmPrev;
	long	MultiZoomDelay;
	short	level;
	RECT	BMRectStart, BMRect; 
	int	ilevel;
    double BWidth, BHeight;    
    DPOINT	CenterPoint;
    HDIB32	hDib32;
    char	Name[256]="c:\\testtif.tif",str[256]; 
    static	int	TileRow,TileCol, NumRows,NumCols;  
    static	long	TotFiles;  
    static	double	TileScale=1, TileBeginx,TileBeginy, TileWidth,TileHeight,TileOverlapFactor;

 switch (Message)
   {
   	case GF_INIT:  
   		if (ScaleIsSet (TRUE))
   			return FALSE;
       	AddLBUTTON = FALSE;  
       	HaveDown = FALSE;  
       	TileScale = GetGlobalDVal2 ("[%TILESCALE]",1.0);
       	TileOverlapFactor = GetGlobalDVal2 ("[%TILEOVERLAP]",1);
   		SetPrompt (PRMT_ZOOMIN,TRUE);  
        SetCurs (LoadCursor (hInst,"ZOOMIN"),FALSE); 
   		break;

    case GF_CLOSE:
    case GF_CANCEL:
		for (level=0;level<NumMultiZoomLevels;level++)
			GSSiDeleteObject (&hbmpMultiLevel[level]); 
		hDC = GetDC (hWndMain);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (hWndMain,hDC); 
	   	if (hdcMemMap)
			DeleteDC (hdcMemMap);
		hdcMemMap = 0;
		MultiZoomLevel = 0;    
    	return FALSE;
    	break;
    	  
    case WM_LBUTTONUP:
	{   
		if (MaskOffsetLine && CurView->hMaskArea)
		{ 
			LPMNMXCORD lpRect = (LPMNMXCORD) GlobalLock(CurView->hMaskArea);

			CurView->NewBounds = *lpRect; 
			GlobalUnlock (CurView->hMaskArea);
		}
		CurView->WBounds = CurView->NewBounds; 
		TileWidth = CurView->DrawRect.right - CurView->DrawRect.left + 1;
		TileHeight = CurView->DrawRect.bottom - CurView->DrawRect.top + 1;
		NumCols = 1+(CurView->WBounds.xmx - CurView->WBounds.xmn - TileScale)/(TileScale * (TileWidth-1));
		NumRows = 1+(CurView->WBounds.ymx - CurView->WBounds.ymn - TileScale)/(TileScale * (TileHeight-1));      
		TileBeginx = CurView->WBounds.xmn;
		TileBeginy = CurView->WBounds.ymn;  
		TileRow =  0;  
		TileCol = -1;   
		TotFiles = 0;
NextTile: 
		TileCol++;
		if (TileCol >= NumCols)
		{
			TileCol = 0;
			TileRow++;
			if (TileRow >= NumRows) 
			{
				SetWindowText (hWndMain,"Done");
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
				return TRUE;
			}
		}
		if (!CheckForContinue (TRUE,0))
		{
			SetWindowText (hWndMain,"Cancelled");
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
			return TRUE;
		}
		sprintf (str,"Row %i of %i: Col %i of %i",TileRow+1,NumRows,TileCol+1,NumCols);
		SetWindowText (hWndMain,str);
		CurView->WBounds.xmn = TileBeginx + TileCol * TileScale * TileWidth-TileScale;
		CurView->WBounds.xmx = CurView->WBounds.xmn + TileScale * TileWidth;
		CurView->WBounds.ymn = TileBeginy + TileRow * TileScale * TileHeight-TileScale;
		CurView->WBounds.ymx = CurView->WBounds.ymn + TileScale * TileHeight;   
		if (MaskOffsetLine && CurView->hMaskArea)
		{ 
			LPMNMXCORD lpRect = (LPMNMXCORD) GlobalLock(CurView->hMaskArea);
            DPOINT	AreaPoints[5];
            HPDPOINT	lpDpoints;
            BOOL		rtn;
            double		AreaAZ[4];
            
			CurView->NewBounds = *lpRect; 
		    lpRect++;
		    lpDpoints = (HPDPOINT) lpRect;  
		    BoundsToPoints (&CurView->WBounds,AreaPoints,AreaAZ);   
		    rtn = PolyInArea (GF_AREA,4,AreaPoints,AreaAZ,CurView->NumMaskPoints,lpDpoints,1,0,0,0);
			GlobalUnlock (CurView->hMaskArea); 
			if (!rtn)
				goto NextTile;
		}
		FactorBounds (&CurView->WBounds,TileOverlapFactor);
		CurView->NewBounds = CurView->WBounds;
		MultiZoomBegin (0,1,TileOverlapFactor);  
	}
    	break;
    	
	case GF_MULTIZOOM_END:
	{ 
		DPOINT BitmapPoint[2], WorldPoint[2];
		double	ScaleX, ScaleY;
		POINT	BPoint[2];
		
   		BitmapPoint[0].x = CurView->DrawRect.left;
   		BitmapPoint[0].y = CurView->DrawRect.top; 
   		BitmapPoint[1].x = CurView->DrawRect.right;
   		BitmapPoint[1].y = CurView->DrawRect.bottom;  
   		BPoint[0]= DPointToPoint (BitmapPoint[0]);
   		BPoint[1]= DPointToPoint (BitmapPoint[1]);
   		BitmapPoint[0].x = 0;
   		BitmapPoint[0].y = 0; 
   		WorldPoint[0] = WinPtToBasePt (BPoint[0]);
   		WorldPoint[1] = WinPtToBasePt (BPoint[1]);
   		ScaleX = ScaleY = CurView->BaseUnitsPerPixel; 
   		ScaleX = ScaleY = ldistp (WorldPoint[0],WorldPoint[1])/idist (BPoint[0],BPoint[1]); 
		SetViewport (MultiZoomVP);
        SelectObject(hdcMemMap,hbmpMultiLevel[0]); 
		{
			int	OutWidth = CurView->DrawRect.right-CurView->DrawRect.left+1;  
			int	OutHeight = CurView->DrawRect.bottom-CurView->DrawRect.top+1; 
			HDC		hDCTemp = CreateCompatibleDC(hdcMemMap);
		    HBITMAP	hNewBM = CreateCompatibleBitmap(hdcMemMap,OutWidth,OutHeight), hbmPrev;     
		    HDIB	hDIB;
//			BITMAP	bm;
			char	directory[32];
			
			TotFiles++;
			sprintf (directory,"File%0.3ld",TotFiles/1000+1);	
//	        GetObject(hNewBM, sizeof(BITMAP), (LPSTR) &bm);
	        
	        hbmPrev = SelectObject(hDCTemp, hNewBM);
//	        GetObject(hbmPrev, sizeof(BITMAP), (LPSTR) &bm);    
	                
//	        GetObject(hbmpMultiLevel[ilevel-1], sizeof(BITMAP), (LPSTR) &bm);
		    BitBlt(hDCTemp, 0,0, OutWidth,OutHeight,
				   CurView->hDC,CurView->DrawRect.left,CurView->DrawRect.top,
				   SRCCOPY); 
			SelectObject (hDCTemp,hbmPrev); 

	   		hDib32 = BitmapToDIB32 (hNewBM);  
			SetGeoTiffData (hDib32,&ScaleX,&ScaleY,&BitmapPoint[0],&WorldPoint[0]); 
			sprintf (Name,"c:\\tiles\\%s\\%4.4i%4.4i.tif",directory,TileRow+1,TileCol+1);   
			makedirectories (Name,FALSE,FALSE);
	   		SaveDIB32 (hDib32,Name,18,TIFF_ADOBE_DEFLATE); 
			GMDestroyDIB32 (hDib32); 
			GSSiDeleteObject (&hNewBM);
			DeleteDC (hDCTemp); 
			SelectObject(hdcMemMap,hbmpOld); 
			GSSiDeleteObject (&hbmpMultiLevel[0]); 
		} 
		hDC = GetDC (hWndMain);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (hWndMain,hDC); 
	   	if (hdcMemMap)
			DeleteDC (hdcMemMap);
		hdcMemMap = 0;
		
		goto NextTile; 
    }

    default:
{
#if ENABLETRACE
GSSiExitProg (150);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (150);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ZoomOut (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (149);
#endif
{
 char key;
 POINT	MousePoint;
 DPOINT	CenterPt;
 MNMXCORD	MinMax;
 double	Dist;   
 static	BOOL	HaveDown; 
 BOOL	ClearFullDisplay=FALSE;

 switch (Message)
   {
   	case GF_INIT:
   		if (ScaleIsSet (TRUE))
   			return FALSE;
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		SetPrompt (PRMT_ZOOMOUT,TRUE);
        SetCurs (LoadCursor (hInst,"ZOOMOUT"),FALSE);
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    CenterPt = ScreenPtToBasePt(MousePoint); 
 		if (CurView->Type == 7)
 		{
 			if (CurView->DisplayedFullScreen) 
			{
				ClearFullDisplay = TRUE;
				MakeVPFullScreen (CurView->ID,0);
			}
      		SetZoomVP (CurView->BoundsDisplayVP);
      	} 
	    Dist = CurView->WBounds.xmx - CurView->WBounds.xmn;
	    MinMax.xmn = CenterPt.x - Dist;
	    MinMax.xmx = CenterPt.x + Dist;
	    Dist = CurView->WBounds.ymx - CurView->WBounds.ymn;
	    MinMax.ymn = CenterPt.y - Dist;
	    MinMax.ymx = CenterPt.y + Dist; 
	    CurView->CurZoomAreaRef = 0;
	    ZoomToRect (MinMax,FALSE);
		if (ClearFullDisplay)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
		return (GF_INCREASE_SUCCESS_COUNT);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (149);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (149);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL MoveTextBox (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
 char key;
 POINT	MousePoint;
 static	BOOL	HaveDown=FALSE, HaveBox=FALSE, IsHighlighted=FALSE;   
 static	RECT	LastRect;

 switch (Message)
   {
   	case GF_INIT:
   		HaveDown=TRUE;
		DisableMarginPan=TRUE;    
		IgnoreSelectVP = TRUE;
       	AddLBUTTON = TRUE;  
   		HaveBox = FALSE;
       	if (lParam)  
       	{
			ResetTAGBox (CurView->hDC,2);   
			DoSave = TRUE;    	
       		AddGraphicsFunction (hWnd,GF_MOVE_TAG,0); 
       	}
   		break;
    
    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	DoSave = TRUE;
	    if (HaveBox)
			AddGraphicsFunction (hWnd,GF_MOVE_TAG,0);
		else if ((TBNum=PickTextBox (&MousePoint)))
			AddGraphicsFunction (hWnd,GF_MOVE_TAG,0);
		else
			DoSave = FALSE;
		break;

    case WM_MOUSEMOVE:
    	if (CurView && wParam == (MK_LBUTTON))
    	{
    		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    	if ((TBNum=PickTextBox (&MousePoint))) 
	    	{    
	    		if (TBNum != LastTB && IsHighlighted)
	    		{
	    			InvertRect (CurView->hDC,&LastRect); 
	    			IsHighlighted = FALSE;
	    		}
	    		InvertRect (CurView->hDC,&TAGBox.rect); 
	    		LastRect = TAGBox.rect;
	    		LastTB = TBNum;
	    		if (IsHighlighted)
	    			IsHighlighted = FALSE;
	    		else
	    			IsHighlighted = TRUE;
	    		HaveBox=TRUE;
	    	}
	    	else 
	    	{
			    if (IsHighlighted)
	    			InvertRect (CurView->hDC,&LastRect); 
	    		IsHighlighted = FALSE;
	    		HaveBox=FALSE;
	    	}
	    }
	    break; 
	
	case GF_CLOSE:
		DisableMarginPan=FALSE;  
		IgnoreSelectVP = FALSE; 
		DoSave = FALSE;
		return FALSE;
		    
    case GF_COMPLETE:
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
    	break;
    	
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL EditTextBox (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
 char key;
 int	nRc;
 POINT	MousePoint; 
 static	BOOL	HaveBox=FALSE, IsHighlighted=FALSE;
 static	RECT	LastRect;

 switch (Message)
   {
   	case GF_INIT:
   		if (lParam)
   		{
	   		HaveBox=TRUE;
	       	AddLBUTTON = FALSE;
			PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		}
   		else
   		{
	   		HaveBox=FALSE;
	       	AddLBUTTON = TRUE;
	       	DisableMarginPan=TRUE;
	    }
   		break;

    case WM_CHAR:
    {
		switch (wParam)
		{   
			case 'f':
			case 'F':
            	if ((TBNum=PickTextBox (0))) 
            	{
            		HaveBox=TRUE;
					PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
		    	}
            break;

            default:
            	return FALSE;
            break;
		}  
		return TRUE;
	}
    case WM_MOUSEMOVE:
    	if (wParam == (MK_LBUTTON))
    	{   
    		short	SaveVPID = CurView->ID;
			
    		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    	if ((TBNum=PickTextBox (&MousePoint))) 
	    	{    
	    		if (TBNum != LastTB && IsHighlighted)
	    		{
	    			InvertRect (CurView->hDC,&LastRect); 
	    			IsHighlighted = FALSE;
	    		}
	    		InvertRect (CurView->hDC,&TAGBox.rect); 
	    		LastRect = TAGBox.rect;
	    		LastTB = TBNum;
	    		if (IsHighlighted)
	    			IsHighlighted = FALSE;
	    		else
	    			IsHighlighted = TRUE;
	    		HaveBox=TRUE;
	    	}
	    	else 
	    	{
			    if (IsHighlighted)
	    			InvertRect (CurView->hDC,&LastRect); 
	    		IsHighlighted = FALSE;
	    		HaveBox=FALSE;
	    	} 
	    	SetViewport (SaveVPID);
	    }
	    break;
    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {
    	
	    if (IsHighlighted)
	    	InvertRect (CurView->hDC,&TAGBox.rect);  
	    IsHighlighted = FALSE;
    	if (!HaveBox) break; 
    	switch (Function)
    	{   
    		case GF_SET_TEXTBOX_FACTOR:
	        {
	        	double Factor = GetGlobalDVal2 ("[%SIZECHANGEFACTOR]",1.0);
				
				if (TAGBox.Factor)
					TAGBox.Factor *= Factor;
				else
					TAGBox.Factor = Factor; 
				TAGBox.bmWidthD = 0;
				TAGBox.bmHeightD = 0;
				RestoreTAG (CurView->hDC);
				if (ResetTAGBox (CurView->hDC,2))
					DrawTAG(CurView->hWnd,CurView->hDC,FALSE,TRUE); 
				SaveTAG(TBNum);
			} 
			break;
    		
    		case GF_EDIT_TEXTBOX:
	    	setDoPaint( FALSE);
	        {
	          DLGPROC lpfnTAGEDITMsgProc;
		
	          lpfnTAGEDITMsgProc = MakeProcInstance((DLGPROC)TAGEDITMsgProc, hInst);
	          nRc = DialogBox(hInst, (LPSTR)"TAGEDIT", hWndMain, lpfnTAGEDITMsgProc);
	          FreeProcInstance(lpfnTAGEDITMsgProc);
	        }
	        if (nRc)
	        {
				RestoreTAG(CurView->hDC);
				if (ResetTAGBox (CurView->hDC,2))
					DrawTAG(CurView->hWnd,CurView->hDC,FALSE,TRUE); 
		    	/*if (TAGBox.before) 
		    	{
		    		DeleteObject(TAGBox.before);
		    		TAGBox.before=0;
		    	}*/
				SaveTAG(TBNum);
			}
			setDoPaint( TRUE);  
			DisableMarginPan=FALSE;
			break;
			
			case GF_DELETE_TEXTBOX:
				TAGBox.ViewportID = -TAGBox.ViewportID;
				SaveTAG(TBNum);
				break; 
				
			case GF_EDIT_TEXTBOX_TEXT:
				if (GetTextStringML (hWnd,TAGBox.text,256,"Edit Text",0))
					SaveTAG(TBNum);
				break;
		}
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
    }
		break;

    case GF_COMPLETE:
    	break;
    	
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL OffsetAreaHP (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (716);
#endif
{
 POINT	MousePoint;
 DPOINT	BasePoint;  
 char	key, str[64];
 static	HaveDown=FALSE; 
 short	Item;
 BOOL	rc;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;  
		GetGlobalCVal ("[%OFFSETLINEDIST]",str,0);
		OffsetLineOffset = atobasedist (str,&rc); 
		if (Function == GF_OFFSET_AREA && fabs(OffsetLineOffset) < 0.0001)
        {   
	         GSSiMsgBox(0,"Offset distance not set", "Offset Error",MB_ICONSTOP,0);
{
#if ENABLETRACE
GSSiExitProg (716);
#endif
	         return FALSE;
}
        } 
        if (Function == GF_OFFSET_AREA) 
   			SetPrompt (PRMT_OFFSET_AREA,TRUE); 
   		else 
   			SetPrompt (PRMT_SELECT_HIGHLIGHT_AREA,TRUE);  
 
{
#if ENABLETRACE
GSSiExitProg (716);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {
        if (Message == GF_EXECUTE)
        	Item = 0;
        else
        {   
        	LPVISLIST	SaveVis = CurVis;
			VISLIST		SaveVisList;
			
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	        BasePoint=ScreenPtToBasePt(MousePoint);
			SelectVisList (TRUE); 
			SaveVisList = *CurVis;
			_fmemset (CurVis->WantType,0,sizeof(CurVis->WantType));
			CurVis->WantType[0] = 1;
	        PickItems (hWnd,BasePoint); 
	        *CurVis = SaveVisList;
	        CurVis = SaveVis;
        	if (!NumPicked)
        		break;
        	Item = NumPicked-1; 
        } 
	case GF_USEPICKED:
		GetGlobalCVal ("[%OFFSETLINEDIST]",str,0);
		OffsetLineOffset = atobasedist (str,&rc); 
		if (Message == GF_USEPICKED)
			Item = 0;
        if (Function == GF_SELECT_HIGHLIGHT_AREA)
			SelectAreaToOffsetFile(Item, OffsetLineOffset, 0);
        else
        	OffsetPickedArea (Item,OffsetLineOffset);
		DisplayPolyOff();  
		DisplayMaskArea();
        if (Function == GF_SELECT_HIGHLIGHT_AREA || Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L);
		else
{
#if ENABLETRACE
GSSiExitProg (716);
#endif
			return (GF_INCREASE_SUCCESS_COUNT); 
}


    }
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (716);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (716);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ChangeClassVis (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (162);
#endif
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 static	int	LastDesc;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT: 
       	AddLBUTTON = TRUE;
   		LastDesc=-1;   
   		HaveDown = TRUE;
   		SetPrompt (PRMT_TOGGLEVIS,TRUE);  
        SetCurs (LoadCursor (hInst,"TOGGLEVIS"),FALSE);
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
			SelectVisList (FALSE);
        	ToggleVisibility (PickList[NumPicked-1].Desc);
		    CurView->CurZoomAreaRef = 0;
		    ZoomToRect(CurView->WBounds,FALSE);
		    LastDesc = PickList[NumPicked-1].Desc;
        }
		break;

    case WM_RBUTTONUP:
        if (LastDesc > 0)
        {
			SelectVisList (FALSE);
        	ToggleVisibility (LastDesc);
		    CurView->CurZoomAreaRef = 0;
		    ZoomToRect(CurView->WBounds,FALSE);
		    LastDesc = 0;
        }
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (162);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (162);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ChangeRedefColor (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 static	COLORREF	NewColor=0;
 static	HaveDown=FALSE,First=TRUE; 
 BOOL	Redisplay=FALSE;
 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	HaveDown = TRUE;
   	First = TRUE;
   	AddLBUTTON = TRUE;
   	return GF_READY_TO_PROCESS;
 }
    	
 if (ForAllVis)
 {  
 	if (Message == WM_LBUTTONUP && HaveDown)
 	{
	    if(GetColor(hWnd,&NewColor))
	    {   
	    	int	idesc, newob;
	    	
	    	newob = NextNewObject(TRUE);
			SetClassColor (newob,1,NewColor);
	    	SetCVTFromVis ();
	    	for (idesc=1;idesc<3201;idesc++) 
	    	{
				if (CurView->CurVisType[idesc])
					CurView->NewObjectMap[idesc]=newob+1;
			}
		    CurView->CurZoomAreaRef = 0;  
		    Redisplay = TRUE;
	    }
		PostMessage(hWnd, GF_CLOSE,0, 0L);
		if (Redisplay)
	    	RedisplayViewport(FALSE,FALSE);
		return TRUE;
	} 
	return FALSE;
 }


 switch (Message)
   {
   	case GF_EXECUTE:
   		if (First)
       		if(GetColor(hWnd,&NewColor))
       	First = FALSE;
//   		AddPenRedef (PickList[0].ipen,NewColor);
		return (GF_INCREASE_SUCCESS_COUNT); 
    case WM_LBUTTONUP:
        {   
	    	if (!HaveDown) break;
	    	HaveDown = FALSE;
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	        BasePoint=ScreenPtToBasePt(MousePoint);
	        PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
			if (Message == GF_USEPICKED)
				NumPicked = 1;
	        if (NumPicked > 0)
	        {
		        setDoPaint( FALSE);
	        	if(GetColor(hWnd,&NewColor))
	        	{   
//	        		AddPenRedef (PickList[NumPicked-1].ipen,NewColor);
				    CurView->CurZoomAreaRef = 0;
				    RedisplayViewport(FALSE,FALSE);
		        }
				setDoPaint( TRUE);
	        }
			if (Message == GF_USEPICKED)
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	    }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL ChangePenColor (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor;
 static	HaveDown=FALSE;
 
 if (Message == GF_INIT)
 {
	if (!EditName[0])
	{
 		MessageBox(GetFocus(), "No Update File", 0,MB_ICONEXCLAMATION|MB_OK);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        return TRUE;
	}
   	AddLBUTTON = TRUE;
 } 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	HaveDown = TRUE;
   	return TRUE;
 }
    	
 switch (Message)
   {
         
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
	    MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	        setDoPaint( FALSE);
        	if(GetColor(hWnd,&NewColor))
        	{   
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				if (OpenMap (CurView->hWnd,0))
				{   
					int clr;
					ModifyPen = 1;//PickList[NumPicked-1].ipen;
				    GSSillseek(FidMap,ColorPaletteOffset,0);  
				    ModPenLoc=0;
				    ProcessPrimarySeg (0, 0, 0,FALSE,0,HFILE_ERROR); 
				    if (ModPenLoc)
				    {
					    GSSillseek (FidMap,ColorPaletteOffset+ModPenLoc,0);
					    clr = GetRValue(NewColor);
			 			BigWrite (FidMap,(HPSTR)&clr,2,-1); 
					    clr = GetGValue(NewColor);
			 			BigWrite (FidMap,(HPSTR)&clr,2,-1);
					    clr = GetBValue(NewColor);
			 			BigWrite (FidMap,(HPSTR)&clr,2,-1); 
			 		}
					CloseMap (FALSE);
				} 
			    CurView->CurZoomAreaRef = 0;
			    RedisplayViewport(FALSE,FALSE);
	        }
			setDoPaint( TRUE);
        }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}
BOOL ChangePenNumber (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key, str[16];
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor;
 static	HaveDown=FALSE;
 
 if (Message == GF_INIT)
 {
	if (!CurView->UpdateFile)
	{
 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        return TRUE;
	} 
	_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
	_fstrcpy (PltName,EditName);
  	AddLBUTTON = TRUE;
 } 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	HaveDown = TRUE;
   	return TRUE;
 }
    	
 switch (Message)
   {
         
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	        setDoPaint( FALSE); 
	        str[0]=0;
           	if (GetTextString (hWnd,str,sizeof(str),"Enter new pen number",0,0,0,TRUE,TRUE))
        	{   
        		int	newpen;
        		
	           	newpen = atoi (str);
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				if (OpenMap (CurView->hWnd,0))
				{   
				    GSSillseek (FidMap,PickList[NumPicked-1].Segment +
				    			 PickList[NumPicked-1].Element+2+2,0);
		 			BigWrite (FidMap,(HPSTR)&newpen,2,-1); 
					CloseMap (FALSE);
				} 
			    CurView->CurZoomAreaRef = 0;
			    RedisplayViewport(FALSE,FALSE);
	        }
			setDoPaint( TRUE);
        }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL MoveSizeViewport (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (911);
#endif
{
 HDC	hDC;
 char key;
 POINT	MousePoint; 
 RECT	MoveRect;
 static	int	Mode=0; 
 static	POINT	LastPoint; 
 static LPVIEWPORT EditView=0;  
 static	RECT	LastRect, OrigRect; 
 static BOOL	HaveRect;
 HPEN	hWidePen; 
 int	OldMode;  

 switch (Message)
   {
   	case GF_INIT:
/*		GetCursorPos (&MousePoint);
		ScreenToClient (hWnd,&MousePoint); 
		SelectViewport (MousePoint,FALSE,FALSE);*/   
    	if (EditView && EditView != CurView)
    	{   
    		GMMessageBox (MSG_MOVESIZE1,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (911);
#endif
			return FALSE; 
}
    	}
    	if (CurView->WidthType == 1)
    	{   
    		GMMessageBox (MSG_MOVESIZE2,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (911);
#endif
			return FALSE; 
}
    	}
   		EditView = CurView; 
//   		DisableMarginPan=TRUE; 
   		HaveRect = FALSE;
       	AddLBUTTON = TRUE; 
       	Mode = 0;
   		SetPrompt (PRMT_MOVESIZEVP,TRUE);  
   		break;

    case WM_MOUSEMOVE: 
    	if (!EditView)
    		break;
		SetCurView ( EditView);
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	if (wParam == (MK_LBUTTON))
    	{   
    		int	xdiff, ydiff;
    		
			hDC = GetDC (hWnd);  
		  	SetDisplayMode (hDC, GF_SCREENMODE);
		  	SelectClipRgn (CurView->hDC,0);
			hWidePen = CreatePen (PS_SOLID,3,RGB(0,0,0));
			OldMode = SetROP2(hDC,R2_NOT); 
			DrawRectPoly (hDC,&LastRect,hWidePen); 
			xdiff = MousePoint.x - LastPoint.x;
			ydiff = MousePoint.y - LastPoint.y;
			switch (Mode)
			{
				case 0:
				break;
				
				case 1:
					LastRect.left += xdiff;
					LastRect.right += xdiff;
					LastRect.top += ydiff;
					LastRect.bottom += ydiff;  
				break;
				
				case 2:
					LastRect.left += xdiff;
				break;
				
				case 3:
					LastRect.left += xdiff;
					LastRect.top += ydiff;
				break; 
				
				case 4:
					LastRect.top += ydiff;
				break;  
				
				case 5:
					LastRect.top += ydiff;
					LastRect.right += xdiff;
				break;

				case 6:
					LastRect.right += xdiff;
				break;

				case 7:
					LastRect.bottom += ydiff;
					LastRect.right += xdiff;
				break;

				case 8:
					LastRect.bottom += ydiff;
				break;
				
				case 9:
					LastRect.left += xdiff;
					LastRect.bottom += ydiff;
				break;
			}
			DrawRectPoly (hDC,&LastRect,hWidePen); 
			DeleteObject (hWidePen);
			SetROP2(hDC,OldMode); 
		    ReleaseDC (hWnd,hDC);
		    LastPoint = MousePoint; 
		    CurView->Rect = LastRect;
    	}
    	else
    	{   
    		Mode = 0;
    		if (!PtInRect (&CurView->Rect,MousePoint))
    		{
        		SetCurs (0,FALSE);
				DisableMarginPan=FALSE;
    			break; 
    		} 
    		MoveRect = CurView->Rect;
    		InflateRect (&MoveRect,-6,-6);
    		if (PtInRect (&MoveRect,MousePoint))
    		{
				Mode = 1;
    		}
    		else
    		{   double	NearDist, Dist;
    		    POINT	VPoint;
    			
    			VPoint.x = CurView->Rect.left;
    			VPoint.y = (CurView->Rect.top + CurView->Rect.bottom) / 2;
    			NearDist = idist (VPoint,MousePoint);
				Mode = 2; 
				
    			VPoint.x = CurView->Rect.left;
    			VPoint.y = CurView->Rect.top;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 3;
					NearDist = Dist;
				} 
				
    			VPoint.x = (CurView->Rect.left + CurView->Rect.right)/2;
    			VPoint.y = CurView->Rect.top;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 4;
					NearDist = Dist;
				} 
				
    			VPoint.x = CurView->Rect.right;
    			VPoint.y = CurView->Rect.top;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 5;
					NearDist = Dist;
				} 
				
    			VPoint.x = CurView->Rect.right;
    			VPoint.y = (CurView->Rect.top + CurView->Rect.bottom) / 2;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 6;
					NearDist = Dist;
				} 
				
    			VPoint.x = CurView->Rect.right;
    			VPoint.y = CurView->Rect.bottom;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 7;
					NearDist = Dist;
				} 
				
    			VPoint.x = (CurView->Rect.left + CurView->Rect.right)/2;
    			VPoint.y = CurView->Rect.bottom;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 8;
					NearDist = Dist;
				} 
				
    			VPoint.x = CurView->Rect.left;
    			VPoint.y = CurView->Rect.bottom;
    			Dist = idist (VPoint,MousePoint);
				if (Dist < NearDist)
				{
					Mode = 9;
					NearDist = Dist;
				} 
    		} 
			DisableMarginPan=TRUE;
			SetMoveCursor (Mode);
    	}
    	
       	break;

    case WM_LBUTTONDOWN: 
    	if (!EditView)
    		break;
		SetCurView ( EditView);
		LastRect = CurView->Rect;
   		LastPoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    if (HaveRect) break;
	    HaveRect = TRUE;
		hDC = GetDC (hWnd);  
		OrigRect = CurView->Rect;
	  	SetDisplayMode (hDC, GF_SCREENMODE);
	  	SelectClipRgn (CurView->hDC,0);
		hWidePen = CreatePen (PS_SOLID,3,RGB(0,0,0));
		OldMode = SetROP2(hDC,R2_NOT); 
		DrawRectPoly (hDC,&LastRect,hWidePen);
		DeleteObject (hWidePen);  
		SetROP2(hDC,OldMode);
	    ReleaseDC (hWnd,hDC);
	    
		break;

    case WM_LBUTTONUP:
		break;

    case WM_RBUTTONUP: 
    	RecomputeViewport(OrigRect);
//        RemoveGraphicsFunction (hWnd);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		EditView = 0; 
   		HaveRect = FALSE;
		setDoPaint( TRUE);      
		DisableMarginPan = FALSE;
       	SetCurs (0,FALSE);
		break;
    
    case GF_CLOSE: 
   		EditView = 0;  
   		Mode = 0;
   		HaveRect = FALSE;
		DisableMarginPan = FALSE;
       	SetCurs (0,FALSE);
		RedisplayWindow();   
{
#if ENABLETRACE
GSSiExitProg (911);
#endif
    	return FALSE;
}
    	break;
    	
    default:
{
#if ENABLETRACE
GSSiExitProg (911);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (911);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL EditViewport (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (912);
#endif
{ 

  DLGPROC lpfnVPEDITMsgProc;
  int	nRc;

  switch (Message)
  {
	   	case GF_INIT:
	       	AddLBUTTON = FALSE;
	       	if (hWnd)
				PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
	   		break;
	    
	    case GF_EXECUTE:  
	    {   
	    	DisableMarginPan = TRUE;
			lpfnVPEDITMsgProc = MakeProcInstance((DLGPROC)VPEDITMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"VPEDIT", hWndMain,lpfnVPEDITMsgProc);
			FreeProcInstance(lpfnVPEDITMsgProc);
			if (nRc)
	 			DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
			setDoPaint( TRUE);      
			DisableMarginPan = FALSE;
			RedisplayWindow();   
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		}
	    default:
{
#if ENABLETRACE
GSSiExitProg (912);
#endif
	    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (912);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL AdjustOrthoColors (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (722);
#endif
{
	LPVIEWPORT	SaveViewport;
    
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
		CloseOrthos(TRUE);
		SaveViewport=CurView;
		AdjustColors (); 
		SetCurView ( SaveViewport);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 

	    break; 
	  
	default:
{
#if ENABLETRACE
GSSiExitProg (722);
#endif
		return FALSE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (722);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL SelectOrigOrtho (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (725);
#endif
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_CHAR:
		key = wParam;
		if (key == 'S' || key == 's')
			SingleStepOrthos = TRUE;
		break;
		   
    case WM_LBUTTONUP:
    {
    	short	SaveMaxPick=MaxPick;
    	
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE); 
	    PickOrtho=TRUE;  
	    PickedOrthoName[0]=0;
		UseUserPickAp =FALSE;
		SystemPickAp = 4;	
		MaxPick=1;  
	    PickItems (hWnd,BasePoint);  
		UseUserPickAp =TRUE;  
		MaxPick = SaveMaxPick;
	    PickOrtho=FALSE;
		setDoPaint( TRUE); 
		if (PickedOrthoName[0])
		{
			LPSTR	lpColon;
			
			lpColon = _fstrchr (PickedOrthoName,':');
			if (lpColon)
				*lpColon = 0;
			else
			{
				lpColon = _fstrrchr (PickedOrthoName,'.');
				if (lpColon)
					*lpColon = 0;
			}
			_fstrcpy(CurView->OrthoDisplayName,PickedOrthoName);
			SetWindowText(hWndMain,PickedOrthoName);
		    CurView->CurZoomAreaRef = 0;
			RedisplayViewport (FALSE,FALSE);
		}
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (725);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (725);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AdjustOrthos (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (726);
#endif
{HDC hDC;
 char key; 
 short	i;
 POINT	MousePoint;
 DPOINT	BasePoint;
 static	DPOINT	OrthoPoint;
 static	HaveDown=FALSE, HaveOrthoPoint=FALSE;
 static	HANDLE	hTrackLine=0;

 switch (Message)
   {
   	case GF_INIT: 
   		OrthoAdjustVP = CurView->ID;
		SelectVisList (TRUE);
		for (i=0;i<CurView->NumFiles;i++)
		{
			if (CurVis->FileIsVisible[i] && CurView->FileType[i] == 5)
				OrthoAdjustFile = i;
		}
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
		HaveOrthoPoint=FALSE;
   		break;
    
    case GF_CLOSE:
    	OrthoAdjustVP = -1;
    	OrthoAdjustX = 0;
    	OrthoAdjustY = 0;
    	break;
    	
    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
	    if (HaveOrthoPoint)
	    {   
			CloseOrthos(TRUE);
	    	OrthoAdjustX += BasePoint.x - OrthoPoint.x;
	    	OrthoAdjustY += BasePoint.y - OrthoPoint.y;
		    CurView->CurZoomAreaRef = 0;
	    	RedisplayViewport(FALSE,FALSE);
	    	HaveOrthoPoint = FALSE;
	    	GSSiGlobFree (&hTrackLine);
	    }
	    else
	    {
	    	OrthoPoint = BasePoint;
	    	HaveOrthoPoint=TRUE;
	    }
		break;

    case WM_MOUSEMOVE:
    	if (!HaveOrthoPoint) break;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
	    TrackLine (CurView->hDC,OrthoPoint,BasePoint,&hTrackLine); 
	    break;
	    
    case WM_CHAR:
		key = wParam;
		if (key == 'U')
		{   
			double x=OrthoAdjustX, y=OrthoAdjustY;
			
			SetViewport (OrthoAdjustVP);
	    	OrthoAdjustX = 0;
	    	OrthoAdjustY = 0;
			UpdateOrthoIndexWrite (CurView->lpFiles[OrthoAdjustFile],x,y);
			Sound(GOOD_SOUND);
            RemoveGraphicsFunction (hWnd,0);
	    	GSSiGlobFree (&hTrackLine);
	        CurView->CurZoomAreaRef = 0;
	    	RedisplayViewport(FALSE,FALSE);
{
#if ENABLETRACE
GSSiExitProg (726);
#endif
	    	return TRUE;
}
		}               
		else if (key == 'D')
		{   
			
			SetViewport (OrthoAdjustVP);
	    	OrthoAdjustX = 0;
	    	OrthoAdjustY = 0;
			UpdateOrthoIndexWrite (CurView->lpFiles[OrthoAdjustFile],DBL_MAX,0);
			Sound(GOOD_SOUND);
	    	GSSiGlobFree (&hTrackLine);
	    	CurView->OrthoDisplayName[0]=0; 
//            RemoveGraphicsFunction (hWnd,0);
		    CurView->CurZoomAreaRef = 0;
			HaltMapDisplay(FALSE,TRUE);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
			PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);
//	    	RedisplayViewport(FALSE,FALSE);
{
#if ENABLETRACE
GSSiExitProg (726);
#endif
	    	return TRUE;
}
		}               
{
#if ENABLETRACE
GSSiExitProg (726);
#endif
		return FALSE;
}

    default:
{
#if ENABLETRACE
GSSiExitProg (726);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (726);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AddDocument (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{ 
    int		irec;
 	HFILE	FidNoteType; 
	OFSTRUCTGM	OFStruct;
 	char	File[128], str[132],Ext[16];
 	static	char	Name[128]; 
 	LPSTR	lpBar;  
 	BOOL	HaveDown;
 	WORD	type,rec;
 	LPSTR	lpExt;
 	static	char	CurExt[4];
 

#define	LDM_CANCEL		30001
#define LDM_EDIT_INFO	30002
#define LDM_ADD_IMAGE	30003
#define LDM_ADD_VIDEO 	30004
#define LDM_ADD_DOC 	30006 
#define LDM_ADD_WAVE	30007
#define LDM_ADD_IMAGE_CMP	30008
	
 switch(Message)
   {

	   	case GF_INIT:
	       	AddLBUTTON = TRUE;
	   		break;

	    case WM_LBUTTONDOWN:
	    	HaveDown = TRUE;          
	    	break;
		    	
	    case WM_LBUTTONUP:  
	    {
	    	POINT	MousePoint;
	    	DPOINT	BasePoint;
		    	
	    	/*if (!HaveDown) break;*/
	    	HaveDown = FALSE;
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	        BasePoint=ScreenPtToBasePt(MousePoint);
		    setDoPaint( FALSE);
		    PickItems (hWnd,BasePoint); 
		    setDoPaint( TRUE);
	        if (NumPicked > 0)
	        {
		      	POINT	position; 
		      	HMENU	EditMenu, TextMenu, BMPMenu, AVIMenu, MIIMenu, WordMenu, WAVMenu;
				char	FName[128], UDI[34];
				
				CreateDocumentDirectory (PickList[NumPicked-1].Prefix,PickList[NumPicked-1].UDI);  
	         	rec = 31000;
				FidNoteType=GSSiOpenFile("document.txt",&OFStruct,OF_READ);
				if (FidNoteType!=HFILE_ERROR)
				{
			      	GetCursorPos (&position);
			      	EditMenu = CreatePopupMenu();
			      	TextMenu = CreatePopupMenu();
			      	BMPMenu = CreatePopupMenu();
			      	AVIMenu = CreatePopupMenu();  
			      	WAVMenu = CreatePopupMenu();  
			      	MIIMenu = CreatePopupMenu();  
			      	WordMenu = CreatePopupMenu();  
					_fstrcpy(Ext,".TXT"); 
					while (fgetstring(str,128,FidNoteType))
					{   
						rec++;
						if ((lpBar = _fstrstr (str,"|"))) 
						{
							*lpBar = 0;
							lpBar++; 
							_fstrupr(lpBar);
							if (_fstrstr(lpBar,".AVI"))
						      	AppendMenu (AVIMenu,MF_ENABLED|MF_STRING,rec,str);
							else if (_fstrstr(lpBar,".WAV"))
						      	AppendMenu (WAVMenu,MF_ENABLED|MF_STRING,rec,str);
							else if (_fstrstr(lpBar,".BMP"))
						      	AppendMenu (BMPMenu,MF_ENABLED|MF_STRING,rec,str);
							else if (_fstrstr(lpBar,".CBM"))
						      	AppendMenu (BMPMenu,MF_ENABLED|MF_STRING,rec,str);
							else if (_fstrstr(lpBar,".MII"))
						      	AppendMenu (MIIMenu,MF_ENABLED|MF_STRING,rec,str);
							else if (_fstrstr(lpBar,".DOC"))
						      	AppendMenu (WordMenu,MF_ENABLED|MF_STRING,rec,str);
							else 
						      	AppendMenu (TextMenu,MF_ENABLED|MF_STRING,rec,str); 
						}
					}
					GSSiClose(FidNoteType);
					      	    
			      	AppendMenu (EditMenu,MF_ENABLED|MF_STRING,LDM_CANCEL,IADDR("Cancel",1));
			      	if (GetMenuItemCount (TextMenu)) 
				      	AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)TextMenu,"Add Other");
			      	if (GetMenuItemCount (BMPMenu))
				      	AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)BMPMenu,"Add Image");
			      	if (GetMenuItemCount (AVIMenu))
				      	AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)AVIMenu,"Add Video");  
			      	if (GetMenuItemCount (WAVMenu))
				      	AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)WAVMenu,"Add Audio");  
			      	if (GetMenuItemCount (MIIMenu))
				      	AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)MIIMenu,"Add Scanned Document");  
			      	if (GetMenuItemCount (WordMenu))
				      	AppendMenu (EditMenu,MF_ENABLED|MF_POPUP,(UINT)WordMenu,"Add Word Document");  
			      	TrackPopupMenu (EditMenu,TPM_LEFTBUTTON,position.x,position.y,0,hWnd,0);
			      	DestroyMenu (TextMenu); 
			      	DestroyMenu (BMPMenu);
			      	DestroyMenu (AVIMenu); 
			      	DestroyMenu (WAVMenu);
			      	DestroyMenu (MIIMenu); 
			      	DestroyMenu (WordMenu);
			      	DestroyMenu (EditMenu);
			    } 
	        }
	     }
			break;
	case WM_COMMAND:  
    	 if (wParam > 31000)
    	 {  
    	 	BOOL	Edit;
    	 	
         	rec = wParam - 31000; 
         	irec = 0;
			FidNoteType=GSSiOpenFile("document.txt",&OFStruct,OF_READ);
			if (FidNoteType!=HFILE_ERROR)
			{
				while (fgetstring(str,128,FidNoteType))
				{   
					irec++;
					if (irec == rec)
					{
						if ((lpBar = _fstrstr (str,"|"))) 
						{
							*lpBar = '\0';
							lpBar++; 
							_fstrupr(lpBar);
							_fstrcpy (File,lpBar); 
							if (_fstrstr(File,".AVI"))
								type = LDM_ADD_VIDEO;
							else if (_fstrstr(File,".BMP"))
								type = LDM_ADD_IMAGE;
							else if (_fstrstr(File,".CBM"))
								type = LDM_ADD_IMAGE_CMP;
							else if (_fstrstr(File,".WAV"))
								type = LDM_ADD_WAVE;
							else if (lpExt=_fstrstr(File,".")) 
							{
								_fstrcpy (CurExt,lpExt);
								type = LDM_ADD_DOC;
							}
						    PostMessage(hWnd, WM_COMMAND, type,0);
							break; 
						}
					}
				}
				GSSiClose(FidNoteType);
			}
    	 }
    	 else
         switch(wParam)
           {
			
           	case LDM_ADD_IMAGE_CMP:
           	case LDM_ADD_IMAGE:
				 if (GetFileName3 (hWnd,Name,IDS_FILTERBMP,IDS_FILEBMP))
				 {   
					 sprintf (str,"%s%s",CurDocDir,File);
					 if (wParam == LDM_ADD_IMAGE)
					 	copyfile (str,Name,FALSE,0,0,0,0,0,0);
					 else 
					 	CreateCompressedImage (str,Name);
					 _strlwr (Name); 
					 REPLAC (Name,".bmp",".wav",sizeof(Name));
					 _strlwr (str);
					 REPLAC (str,".bmp",".wav",sizeof(str));
					 copyfile (str,Name,FALSE,0,0,0,0,0,0);
					 return TRUE;
				 }
           		break;
           		
           	case LDM_ADD_VIDEO:
				 if (GetFileName3 (hWnd,Name,IDS_FILTERAVI,IDS_FILEAVI))
				 {   
					 sprintf (str,"%s%s",CurDocDir,File);
					 copyfile (str,Name,FALSE,0,0,0,0,0,0);
					 return(TRUE);
				 }
           		break;
           		
           	case LDM_ADD_WAVE:   
           		 if (GetFileName3 (hWndMain,Name,IDS_FILTERWAV,IDS_FILEWAV))
           		 {
					 sprintf (str,"%s%s",CurDocDir,File);
					 copyfile (str,Name,FALSE,0,0,0,0,0,0);
					 return(TRUE);
				 }
           		break;
           		
           	case LDM_ADD_DOC:
           	{
           		char	ExtID[64]="File Type"; 
           		_fstrcpy (Ext,CurExt);
            	 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
				 if (GetFileName3(hWndMain,Name,0,IDS_FILEDOC))
				 {   
					 sprintf (str,"%s%s",CurDocDir,File);
					 copyfile (str,Name,FALSE,0,0,0,0,0,0);
					 return(TRUE);
				 } 
			}
           		break;
           		
           	case LDM_EDIT_INFO:
	        { 
		    	char	Editor[256], mess[128];
		    	DWORD	WVer;
		    	int		WinVer, DosVer;
		    	UINT	ierr;
		    							      	 
				 sprintf (str,"%s%s",CurDocDir,File);
	        	 if (!ExistFile(str))
	        	 {   
	        	 	HFILE	InfoID;
					OFSTRUCTGM	OFStruct;
	        	 	
	        	 	InfoID = GSSiOpenFile(str,&OFStruct,OF_CREATE);
	        	 	GSSiClose(InfoID);
	        	 } 
	        	 _fstrcat (Editor," ");
	        	 _fstrcat (Editor,str);
				 {
					 if ((ierr = WinExec (Editor,SW_SHOW)) < 32)
					 {
					 	sprintf (mess,"Error loading editor: %i",(int) ierr);
					 	MessageBox (hWndMain,mess,0,0);  
					 }
					 else
					 	ShowWindow (hWndMain,SW_MINIMIZE);
				 } 
	        }
	        break;
	        
	        default:
	        	return FALSE;
	   }
	   return TRUE; 
	   
	   default:
			return FALSE;
	}
return TRUE;
}

BOOL DisplayDocument (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key; 
 long	LastPicked=LONG_MAX;
 POINT	MousePoint;
 DPOINT	BasePoint;
 static	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
   		HaveDown=TRUE;
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);    
	    setDoPaint( TRUE);
        if (NumPicked > 0)
        {
		    DisplayDocumentList (hWnd,
		    					 PickList[NumPicked-1].Prefix,
		    					 PickList[NumPicked-1].UDI,NumPicked-1,0);
        }
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL CreatePoly (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{HDC hDC;
 char key;
 static	POINTS	MousePoint;
 static	long	StartSpline;    
 //static	POINT	POCScreen;    
 static	DPOINT	POC,Point0Base;
 POINT	point;
 HPPOINT	Points;
 HPDPOINT	lpDPoint;
 HPPOINT	lpPoints, lpPoint;
 HPDPOINT	lpNewPoint,pFirstPoint;  
 HANDLE	hPoints;
 DPOINT	BasePoint, EndPoint, AtPoint, LastPoint;
 static	BOOL	HaveStart;
 int	OldMode; 
 long	i;
 BOOL	EndPointConnected;
 long	NewTLID;
 static	BOOL	MapEdit=TRUE; 
 HPEN	hTrackPen, hOldPen; 
 double	Dist;  
 static	double	TotDist;  
 static	HCURSOR	InCursor;
 static	BOOL	HaveDownButton;
 char	txt[128];
 
 if (idTimer) return FALSE;    
 if (hTempPoints)
 	Points = (HPPOINT)GlobalLock (hTempPoints);
 switch (Message)
   {
   	case GF_INIT:
   		TotDist=-1; 
   		InCreatePoly=TRUE;
   		HaveDownButton = FALSE; 
   		HavePOC = FALSE; 
   		HaveStart = FALSE;
   		Spline = GetGlobalBVal2 ("[%SPLINE]",FALSE);
   		if (!hCurPolyPoints)
   		{
   			hCurPolyPoints = GSSiGlobAlloc ( 603,GHND,MAX_DIGPOINTS*(long)sizeof(DPOINT));
   			nCurPolyPoints = 0;
   			CurPolyVP = CurView;
  		}
   		else 
   		{
	 		MessageBox(GetFocus(), "Must complete current polyline before beginning a new one", 0,MB_ICONEXCLAMATION|MB_OK);
	 		goto RtnFalse;
   		}
		InCursor=CurView->hCursor;
		SetCurs (hDigCursor,FALSE);
		GSSiGlobFree (&hTempPoints);
		hTempPoints = GSSiGlobAlloc ( 604,GMEM_MOVEABLE,USHRT_MAX); 
	 	Points = (HPPOINT)GlobalLock (hTempPoints);
   		nTempPoints = 0;
   		StartSpline = 1;  
   		GSSiGlobFree (&hNewPolyPoints);
   		NumNewPolyPoints = 0;
   		break;  
   		
   	case GF_COMPLETE:
       	HaveDownButton = FALSE;  
   		if (CursorIsLocked) 
   		{
			IgnoreMouseMove = FALSE;
			GetCursorPos (&point);
			ScreenToClient (hWnd,&point);  
		    PostMessage(hWnd, WM_MOUSEMOVE,0, MAKELPARAM (MousePoint.x,MousePoint.y));
		} 
		else
			SetCurs ((HCURSOR)2,FALSE); 
		if (wParam == GF_UNDO)
			goto DoBackspace;
		if (wParam == GF_LBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
	    	MousePoint = MAKEPOINTS(lParam); 
	    	HaveDownButton = TRUE;
			goto LButUp; 
		}
		if (wParam == GF_RBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
	    	MousePoint = MAKEPOINTS(lParam);
	    	HaveDownButton = TRUE;
			goto RButUp;
		}
		if (wParam == GF_SNAP_UNLOCK && hCurPolyPoints)
		{
			lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+(nCurPolyPoints-1); 
			CurrentPoint = *lpDPoint; 
			GlobalUnlock (hCurPolyPoints);
		} 
		if (wParam == GF_SET_POC)
		{	
			if (!hCurPolyPoints/* || (NewPolyType == 1 && nCurPolyPoints > 1)*/) 
				break;
			POC = CurrentPoint;
			//POCScreen = BasePtToScreenPt(&CurrentPoint); 
			HavePOC = TRUE;
			HaveDownButton=FALSE;  
			//if (!NewPolyType)
				break;  
			lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
			*lpDPoint = CurrentPoint;  
			GlobalUnlock (hCurPolyPoints);
			nCurPolyPoints++;
			break;			
		}
		if (wParam == GF_COPY_POLY)
		{
			GSSiGlobFree (&hCurPolyPoints);
			hCurPolyPoints = hNewPolyPoints;
			nCurPolyPoints = NumNewPolyPoints;
	    	HaveDownButton = TRUE;
			HaveStart = TRUE;
			goto RButUp;
		}
		if (hNewPolyPoints && hCurPolyPoints)
		{   
			if (!nTempPoints)
				HaveStart = FALSE;
	    	if (HaveStart && DoTrack)
				NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0);  
			nTempPoints = 2;
			lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
			nCurPolyPoints+=NumNewPolyPoints;
			lpNewPoint = (HPDPOINT)GlobalLock (hNewPolyPoints); 
			while (NumNewPolyPoints--)
			{   
				if (HaveStart)
				{
					Points[1] = BasePtToScreenPt (lpNewPoint);
					TempPolyline (CurView->hDC,Points,2,0,0);
		    		Points[0] = Points[1];
				}
				else
				{
					HaveStart = TRUE;
					Points[0] = BasePtToScreenPt (lpNewPoint);
				}
				Point0Base = *lpNewPoint;
				*lpDPoint++ = *lpNewPoint++;
			}
			GlobalUnlock (hCurPolyPoints);
        } 
		GSSiGlobUlFree (&hNewPolyPoints);
		NumNewPolyPoints = 0;
       	if (wParam == GF_AREA_FROM_HLT)  
       	{
	    	HaveDownButton = TRUE;
			goto RButUp;
       	}
   		break;	
   		
   	case GF_REDRAW:
    	FirstMoveSinceRedraw = TRUE;  
		HaveDownButton = FALSE;
    	if (nCurPolyPoints)
    	{   
    		Points[0]=Points[1]=DrawCurPoints (Spline,StartSpline,R2_NOT,0);
			Point0Base = ScreenPtToBasePt (Points[0]);;
/*			SetDisplayMode (CurView->hDC, GF_TEXTMODE);
			CurView->hRgn = CreateVPRgn (FALSE);
		  	SelectClipRgn (CurView->hDC,CurView->hRgn);
		  	DeleteObject(CurView->hRgn);
   			hPoints = GSSiGlobAlloc ( 605,GMEM_MOVEABLE,nCurPolyPoints*sizeof(POINT));
   			lpPoints = (HPPOINT)GlobalLock(hPoints);
   			lpPoint = lpPoints;
   			lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints); 
   			for (i=0;i<nCurPolyPoints;i++,lpPoint++,lpDPoint++)
   			{
   				*lpPoint = BasePtToWinPt(lpDPoint);
   			}
   			GlobalUnlock (hCurPolyPoints);
	    	lpPoint--;
	    	if (nCurPolyPoints > 1)
	    	{
		    	TempPolyline (CurView->hDC,lpPoints,(short)nCurPolyPoints,0,0);  
		    	Points[1]=*lpPoint;
		    }
	    	Points[0]=*lpPoint;
           	GSSiGlobUlFree (&hPoints);*/  
   		}
   		if (!CursorIsLocked)
			SetCurs ((HCURSOR)2,FALSE);
   		break;
    
    case WM_RBUTTONDOWN: 
    	if (wParam & MK_LBUTTON)
    		goto RtnFalse;
    case WM_LBUTTONDOWN: 
    	HaveDownButton=TRUE;
    	MousePoint = MAKEPOINTS(lParam);
    	break;
    	
    case WM_MBUTTONDOWN:   
    	break;
    	
    case WM_MBUTTONUP:
       	HaveCurrentLBUTTON=TRUE;
		CurrentLBUTDOWNLoc = POINTStoPOINT(MAKEPOINTS (lParam));
		IgnoreLbutton = FALSE;   
		AddGraphicsFunction (hWnd, GF_SET_POC,0); 
/*		if (!CursorIsLocked)
		{
			IgnoreLbutton = FALSE;   
			IgnoreSelectVP = TRUE;
			PostMessage(hWnd, WM_LBUTTONDOWN,wParam, lParam); 
			PostMessage(hWnd, WM_LBUTTONUP,wParam, lParam);
		}*/
    	break;
/*    	if (HaveLastEP)
    	{ 
			CursorIsLocked = TRUE;
	    	CurrentPoint = LastEP; 
	    	HaveDownButton = TRUE;
			if (DoTrack)
			{
				NotPolyline (CurView->hDC,Points,nTempPoints,0,0);
				Points[1] = BasePtToWinPt (&CurrentPoint);
				TempPolyline (CurView->hDC,Points,2,0,0);
			}
	    	goto LButUp;
    	}   
    	break; */
    	
    case WM_LBUTTONUP:
LButUp: 
		if (!HaveDownButton)
			break;
		if (TempLineType == 1)
	   		SetPrompt (PRMT_DIST_LINE,TRUE);
		else if (NewPolyType) 
	   		SetPrompt (PRMT_POLYLINE_NEXT,TRUE);
	   	else
	   		SetPrompt (PRMT_POLYGON_NEXT,TRUE); 
/*    	if (HavePOC && NewPolyType == 1)
    		goto RButUp; */
		HaveDownButton=FALSE;  
		SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
    	if (HaveStart && !CursorIsLocked)
    	{
			if (DoTrack)
				NotPolylineScreen (CurView->hDC,Points,(short)nTempPoints,0,0);
			if (!Spline && TempLineType != 1) 
				TempPolyline (CurView->hDC,Points,(short)nTempPoints,0,0);
		}
   		if (!hCurPolyPoints)
   		{
   			hCurPolyPoints = GSSiGlobAlloc ( 606,GHND,MAX_DIGPOINTS*(long)sizeof(DPOINT));
   			nCurPolyPoints = 0;
  			CurPolyVP = CurView;
   		}
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;  
	    	UnlockCursor ();
//			CursorIsLocked = FALSE; 
           	CreateDigCursor (CurView->hDC);
			SetCurs ((HCURSOR)2,FALSE);
	    }
	    else
		{
			Points[1] = POINTStoPOINT(MousePoint);
			if (HaveStart && MoveHorzVert == 'V')
				Points[1].x = Points[0].x;
			else if (HaveStart && MoveHorzVert == 'H')
				Points[1].y = Points[0].y;
        	BasePoint=GetButtonWorldCoord (Points[1],wParam);
		}
        EnlargeScreen (0,0);
        CurrentPoint = BasePoint;
		lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
        if (HaveStart && nCurPolyPoints)
        {   
   			double	AZ;
   			long	nNewPnts;
   			 
        	lpDPoint--;
        	if (ldistp (CurrentPoint,*lpDPoint) > P_TOL || HavePOC)
        	{   
		    	Points[0]=Points[1];
		    	Points[1] = POINTStoPOINT(MousePoint);
				if (Spline)
					nTempPoints = 0;
				else
					nTempPoints = 2;
        		ShowTempLineType (&BasePoint,lpDPoint,&TotDist);
				if (HavePOC)
				{   
					DPOINT	PC=*lpDPoint;
					double	BackAZ;
					
					lpDPoint++;
	                CurvePointsD(&PC,&POC,&CurrentPoint, &nCurPolyPoints, &lpDPoint,&BackAZ,USHRT_MAX-4,CurveChordDist,1);
					HavePOC = FALSE; 
					lpDPoint--;
		    		Points[0]=BasePtToScreenPt(lpDPoint);
					Point0Base = *lpDPoint;
		    		Points[1]=Points[0];   
					SetCurAZ (BackAZ);
				}
				else
				{
					nCurPolyPoints++; 
					AZ = getazd (lpDPoint,&BasePoint);
					SetCurAZ (AZ);
					lpDPoint++;
					*lpDPoint = BasePoint; 
				}   
			}
        }
        else
        {
    		HaveStart = TRUE;  
    		*lpDPoint = BasePoint;
    		Points[0]=BasePtToScreenPt(lpDPoint);
			Point0Base = *lpDPoint;
    		Points[1]=Points[0];  
    		if (Spline)
    			nTempPoints = 0;
    		else
	    		nTempPoints = 2;
    		nCurPolyPoints = 1;  
	    }
		GlobalUnlock (hCurPolyPoints);
		if (Message == WM_MBUTTONUP) 
			PostMessage(hWnd, WM_MOUSEMOVE,wParam, lParam); 
		break; 
		
//    case WM_RBUTTONDOWN:
//    	if (wParam!=2) goto RtnFalse;
    	/* keeps from being passed to identify function */
//		break;
		
    case WM_RBUTTONUP:
RButUp: 
//    	MousePoint = MAKEPOINT(lParam);
		if (!HaveDownButton)
			break;
		HaveDownButton=FALSE;  
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;  
	    	UnlockCursor ();
//			CursorIsLocked = FALSE; 
//			SetCurs (hDigCursor,FALSE);
	    }
	    else
		{
			Points[1] = POINTStoPOINT(MousePoint);
			if (MoveHorzVert == 'V')
				Points[1].x = Points[0].x;
			else if (MoveHorzVert == 'H')
				Points[1].y = Points[0].y;
        	CurrentPoint = BasePoint=GetButtonWorldCoord (Points[1],wParam);
		}
        EnlargeScreen (0,0);
        LastEP = BasePoint;
        HaveLastEP = TRUE;
        if (HaveStart && hCurPolyPoints)
        {   
        	BOOL	st; 
        	DPOINT	FirstPoint;
        	
			if (DoTrack)
				NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0); 
			lpDPoint = pFirstPoint = (HPDPOINT)GlobalLock(hCurPolyPoints);
			FirstPoint = *lpDPoint;
			lpDPoint += nCurPolyPoints; 
        	lpDPoint--;
       		st = ShowTempLineType (&BasePoint,lpDPoint,&TotDist);
       		lpDPoint++;
//			OldMode = SetROP2(CurView->hDC,R2_NOT); 
//			if (!st)
//				TempPolyline (CurView->hDC,Points,nTempPoints,0,0);
	    	Points[1]=POINTStoPOINT(MousePoint);
//	    	SetROP2(CurView->hDC,OldMode);
        	EndPointConnected=FALSE;
        	if (NewPolyType && nCurPolyPoints == 1 && HavePOC)
        	{
				nCurPolyPoints++;
				*lpDPoint++ = POC;
			}
			else if (HavePOC && (!NewPolyType || nCurPolyPoints > 1) && !Spline)
			{   
				DPOINT	PC=*(lpDPoint-1);
				double	BackAZ;
				
				if (!NewPolyType)
                	CurvePointsD(&PC,&POC,&FirstPoint, &nCurPolyPoints, &lpDPoint,&BackAZ,USHRT_MAX-4,CurveChordDist,1);
				else	
                	CurvePointsD(&PC,&POC,&CurrentPoint, &nCurPolyPoints, &lpDPoint,&BackAZ,USHRT_MAX-4,CurveChordDist,1);
				HavePOC = FALSE; 
				lpDPoint--;
				nCurPolyPoints--;
	    		Points[0]=BasePtToScreenPt(lpDPoint);
				Point0Base = *lpDPoint;
	    		Points[1]=Points[0];   
				SetCurAZ (BackAZ);
			}
			nCurPolyPoints++;
			*lpDPoint = BasePoint; 
       		if (TempLineType==1 && GetGlobalBVal2 ("[%SAVEDISTLINE]",TRUE))
			{
				LPLONG		pNumDistPoints;
				HPDPOINT	pDistPoints;
	
				GSSiGlobFree (&CurView->hDistanceLine);
				CurView->hDistanceLine = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
				pNumDistPoints = (LPLONG)GlobalLock (CurView->hDistanceLine);
				pDistPoints = (HPDPOINT)(pNumDistPoints+1);
				*pNumDistPoints = -nCurPolyPoints;
				hmemmove ((HPSTR)pDistPoints,(HPSTR)pFirstPoint,nCurPolyPoints*sizeof(DPOINT));
				GlobalUnlock (CurView->hDistanceLine); 
			}    
			GlobalUnlock (hCurPolyPoints);

			if (Spline)
			{   
				double	Thin = GetGlobalDVal2 ("[%SPLINETHIN]",0.1); 
				double	EndDist;
				
				lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints);
				if (wParam == 10)
				{   
					lpDPoint[nCurPolyPoints++] = lpDPoint[1];
					EndDist = ldistp (lpDPoint[0],lpDPoint[1]);
				}
				else
					EndDist = 0;	  
		        SplinePointsD (StartSpline,nCurPolyPoints,lpDPoint, &StartSpline, lpDPoint+StartSpline,CurView->BaseUnitsPerPixel,Thin,EndDist,USHRT_MAX/4);   
	        	GlobalUnlock (hCurPolyPoints); 
	        	nCurPolyPoints = StartSpline;
				lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
				*lpDPoint = BasePoint;
				GlobalUnlock (hCurPolyPoints);
				nCurPolyPoints++;
				Spline = FALSE;
			}

            NumNewPolyPoints = nCurPolyPoints;
            hNewPolyPoints = hCurPolyPoints;
            hCurPolyPoints = 0;
           	HaveStart=FALSE;
            nCurPolyPoints = 0;
           	CreateDigCursor (CurView->hDC);
		    PostMessage(hWnd, GF_CLOSE,Function,0); 
        }
        else if (AllowSinglePoint)
        {
        	NumNewPolyPoints = 1;
        	hNewPolyPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(DPOINT));
        	lpDPoint = (HPDPOINT)GlobalLock (hNewPolyPoints);
        	*lpDPoint = BasePoint;
        	GlobalUnlock (hNewPolyPoints);
		    PostMessage(hWnd, GF_CLOSE,Function,0); 
        } 
        break; 
        
    case GF_CLOSE:
    	if (HaveStart && !CursorIsLocked && DoTrack)
			NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0); 
    	if (nCurPolyPoints > 1)
    	{   
    		DrawCurPoints (Spline,StartSpline,R2_NOT,Points);
	    	nTempPoints = 0;
   		}
//        RemoveGraphicsFunction (hWnd); 
//        NumNewPolyPoints = 0;
//        hNewPolyPoints = 0;
       	GSSiGlobFree(&hCurPolyPoints);
       	HaveStart=FALSE;
       	nCurPolyPoints=0; 
       	GSSiGlobUlFree (&hTempPoints);
       	SetCurs (InCursor,FALSE);
       	InCreatePoly = FALSE;  
       	goto RtnFalse;
    	break;
        
    case WM_MOUSEMOVE: 
    	if (!HaveStart || HaveDownButton || !hCurPolyPoints) break;
		if (DoTrack)
			NotPolylineScreen (CurView->hDC,Points,(short)nTempPoints,0,0);
    	if (CursorIsLocked) 
    	{
    		AtPoint = CurrentPoint;
    		Points[1] = BasePtToScreenPt (&CurrentPoint); 
    	}
    	else 
    	{
    		Points[1] = POINTStoPOINT(MAKEPOINTS(lParam)); 
			if (MoveHorzVert == 'V')
				Points[1].x = Points[0].x;
			else if (MoveHorzVert == 'H')
				Points[1].y = Points[0].y;
    		AtPoint = ScreenPtToBasePt (Points[1]);
    	}
		lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
    	lpDPoint--; 
    	LastPoint = TrackLineBegin = *lpDPoint; 
    	TrackLineEnd = AtPoint;
    	GlobalUnlock (hCurPolyPoints);
		if (abs(TempLineType)==1)
    		ShowCurrentDist (2,&LastPoint,&AtPoint,TotDist);
    	if (HavePOC)
    	{   
    		DPOINT	BP=PointToDPoint(Points[0]);
			DPOINT	EP=BasePtToScreenPtD(&AtPoint);
			DPOINT	POCScreen = BasePtToScreenPtD(&POC);
			
    		nTempPoints = 0;
	        CurvePoints(&BP,&POCScreen,&EP, &nTempPoints,  &Points, 1,USHRT_MAX/4); 
	        GlobalUnlock (hTempPoints);
         	Points = (HPPOINT)GlobalLock (hTempPoints);
    	} 
    	else if (Spline && (nCurPolyPoints - StartSpline))
    	{   
			lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints); 
    		lpDPoint[nCurPolyPoints] = TrackLineEnd;
    		if (ldistp (lpDPoint[nCurPolyPoints-1],lpDPoint[nCurPolyPoints]))
    			nCurPolyPoints++;
        	GlobalUnlock (hCurPolyPoints);
    		nTempPoints = 0;
	        SplinePoints (StartSpline,nCurPolyPoints,hCurPolyPoints, &nTempPoints,  Points,CurView->BaseUnitsPerPixel*2,USHRT_MAX/4);   
	        nCurPolyPoints--;
	        GlobalUnlock (hTempPoints);
         	Points = (HPPOINT)GlobalLock (hTempPoints);
    	}
    	else
    		nTempPoints = 2; 
    	
    	if (DoTrack)
			NotPolylineScreen (CurView->hDC,Points,(short)nTempPoints,0,0);
		break;
		
    case WM_CHAR:
		key = wParam; 
		if (lParam & KF_UP)
			break;
		switch (key)
		{   
			case 2: //CNTL_B 
				if (nCurPolyPoints && hCurPolyPoints)
				{   
					lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints); 
					LockCursor (lpDPoint);
					GlobalUnlock (hCurPolyPoints);
					HaveDownButton = TRUE; 
					IgnoreLbutton = FALSE;   
					IgnoreSelectVP = TRUE;
//		        	SendMessage(hWnd, WM_LBUTTONUP,0, 0L); 
					HaveDownButton = TRUE;
		        	PostMessage(hWnd, WM_RBUTTONUP,10, 0L); 
		        }
		        goto RtnTrue;
		        
   			case 19: //CNTL_S 
   				if (Spline)
   				{   
					if (DoTrack)
			    	{   
						NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0);
			    		nTempPoints = 0;
				        SplinePoints (StartSpline,nCurPolyPoints,hCurPolyPoints, &nTempPoints,  Points,CurView->BaseUnitsPerPixel*2,USHRT_MAX/4);   
				        GlobalUnlock (hTempPoints);
			         	Points = (HPPOINT)GlobalLock (hTempPoints);
						NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0);
			    	} 
                    {
						HPDPOINT pPoints = (HPDPOINT)GlobalLock(hCurPolyPoints);
						HPDPOINT pEndPoint = pPoints + (nCurPolyPoints-1);
						DPOINT	 EndPoint = *pEndPoint;
						double	Thin = GetGlobalDVal2 ("[%SPLINETHIN]",GetGlobalDVal2("[%BLACKLINEWIDTH]",2)/10);
	
						lpDPoint = pPoints;  
				        SplinePointsD (StartSpline,nCurPolyPoints,lpDPoint, &StartSpline, lpDPoint+StartSpline,CurView->BaseUnitsPerPixel,Thin,0,USHRT_MAX/4);   
			        	GlobalUnlock (hCurPolyPoints); 
			        	nCurPolyPoints = StartSpline;
						pPoints = (HPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints++;
						*pPoints = EndPoint;
			        	GlobalUnlock (hCurPolyPoints); 
	   					Spline = FALSE; 
	   					nTempPoints = 2;
		    			Points[0]=Points[1]=BasePtToScreenPt(&EndPoint); 
						Point0Base = EndPoint;
		    		}
   				}
   				else 
   				{
   					Spline = TRUE;
   					StartSpline = max(nCurPolyPoints,1);
   				}
   				break;
   				
   			case 20: //CNTL_T
   				if (HaveStart) 
					NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0);
   				if (DoTrack) 
   					DoTrack = FALSE;
   				else
   					DoTrack = TRUE;
		        goto RtnTrue;
   				
		    case '\b': //Backspace 
	DoBackspace:
		    	if (!nCurPolyPoints) break;
		    	if (!FirstMoveSinceRedraw)
		    	{
					OldMode = SetROP2(CurView->hDC,R2_NOT); 
					if (DoTrack) 
						TempPolyline (CurView->hDC,Points,(short)nTempPoints,0,0);   
			    	SetROP2(CurView->hDC,OldMode);   
				}
				lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+(nCurPolyPoints-1); 
	    		Points[1]=BasePtToScreenPt(lpDPoint);
		    	nCurPolyPoints--; 
		    	if (Spline && nCurPolyPoints < StartSpline)
		    		Spline = FALSE;
		    	if (!nCurPolyPoints)
		    		HaveStart=FALSE;  
		    	else if (Spline)
		    		nTempPoints = 0;
		    	else
		    	{
					lpDPoint--; 
					CurrentPoint = *lpDPoint;
		    		Points[0]=BasePtToScreenPt(lpDPoint); 
					Point0Base = *lpDPoint;
			    	if (HavePOC)
			    	{  
				    	HavePOC = FALSE;    
				    	Points[1] = Points[0];
				    	nTempPoints = 2;
				    }
		    	}
		    	GlobalUnlock (hCurPolyPoints);
		    	FirstMoveSinceRedraw = FALSE; 
		    	if (nCurPolyPoints)
		    	{
	    			SendMessage(hWnd, WM_MOUSEMOVE, 0,
	    							  MAKELPARAM(Points[1].x,Points[1].y)); 
	    		} 
		    	
		    	goto RtnTrue;
		}
       	goto RtnFalse;

    default:
    	goto RtnFalse;
    }
RtnTrue:
	if (hTempPoints)
		GlobalUnlock (hTempPoints);

    return (TRUE);   
RtnFalse:
	if (hTempPoints)
		GlobalUnlock (hTempPoints);

    return (FALSE);   
    
}
BOOL CreateHLTArea (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{  
 char key;
 static	BOOL	Inited=FALSE;
 switch (Message)
   {
   	case GF_INIT: 
   		if (Inited)
   		{
	 		MessageBox(GetFocus(), "Must complete current highlight Area before beginning a new one", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		}
//       	AddLBUTTON = TRUE;
   		Inited = TRUE; 
		CreateDigCursor (CurView->hDC);
		SetCurs ((HCURSOR)2,FALSE);
   		SetPrompt (PRMT_POLYGON_INIT,TRUE); 
		NewPolyType = 0;
		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,PRMT_POLYGON_INIT);  
   		break;
   		
    case WM_LBUTTONUP:
		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,0);  
		if (CurView->CurrentFunction != GF_DIGITIZE_POLYLINE) 
		{
	        PostMessage(hWnd, GF_CLOSE,0, 0L);
			break;  
		}
   		SetFunStackPrompt (PRMT_POLYGON_NEXT); 
		PostMessage(hWnd, WM_LBUTTONDOWN,wParam, lParam); 
		PostMessage(hWnd, WM_LBUTTONUP,wParam, lParam);
		if (Message == WM_MBUTTONUP) 
			PostMessage(hWnd, WM_MOUSEMOVE,wParam, lParam); 
		break;   
		
    case GF_COMPLETE:
    	if (NumNewPolyPoints)
    	{   
    		HPDPOINT	lpDPoints;
    		
    		lpDPoints = (HPDPOINT)GlobalLock (hNewPolyPoints);
			if (!SameDPoint(lpDPoints, &lpDPoints[NumNewPolyPoints - 1]))
			{
				HANDLE hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, (NumNewPolyPoints+1) * sizeof(DPOINT));
				LPDPOINT pPt = GlobalLock(hPoints);
				for (int i = 0; i < NumNewPolyPoints; i++)
				{
					pPt[i] = lpDPoints[i];
				}
				pPt[NumNewPolyPoints++] = lpDPoints[0];
				GSSiGlobUlFree(&hNewPolyPoints);
				hNewPolyPoints = hPoints;
				lpDPoints = pPt;
			}
			AddAreaToOffsetFile (0,3,NumNewPolyPoints, lpDPoints,1,0,0);
        	GSSiGlobUlFree(&hNewPolyPoints);
        	NumNewPolyPoints=0;
            DisplayPolyOff();  
            DisplayMaskArea();
        }  
        PostMessage(hWnd, GF_CLOSE,0, 0L);
        break; 
        
    case GF_CLOSE: 
    	Inited = FALSE; 
        return FALSE;
        
    default:
    	return (FALSE);
    }
    return (TRUE);
} 
BOOL CreateNewPolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{  
 char key;   
 short	rtn=1;
 static	BOOL	Inited=FALSE;    
 static short	NewSymbol; 
 static	HCURSOR	InCursor;
 int	StartPrompt;
 char	str[128];
 
 switch (Message)
   {
   	case GF_INIT:  
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
		{
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
			sprintf (str,"#%i",CurView->UpdateFile);
			SetLayerVisibility (str,1);
		}
   	
   		if (Inited)
   		{
	 		MessageBox(GetFocus(), "Must complete current line before beginning a new one", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		}
   		if (NewPolyType)
   		{  
	   		if (!SetLineSymbol (&NewSymbol,0))
	   		{
		 		MessageBox(GetFocus(), "Linear symbol not set", 0,MB_ICONEXCLAMATION|MB_OK);
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	            break;
	   		} 
	   	}
	   	else 
   		{  
	   		if (!SetAreaSymbol (&NewSymbol,0))
	   		{
		 		MessageBox(GetFocus(), "Area symbol not set", 0,MB_ICONEXCLAMATION|MB_OK);
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	            break;
	   		}  
	   	} 
	   	InCursor = CurView->hCursor;
		CreateDigCursor (CurView->hDC);
		SetCurs ((HCURSOR)2,FALSE);
   		Inited = TRUE; 
   		TempLineType = -1;
		if (NewPolyType) 
	   		StartPrompt = PRMT_POLYLINE_INIT;
	   	else
	   		StartPrompt = PRMT_POLYGON_INIT;

		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,StartPrompt);
		if (CurView->CurrentFunction != GF_DIGITIZE_POLYLINE)
			break;  
/*		if (NewPolyType) 
	   		SetFunStackPrompt (PRMT_POLYLINE_NEXT);
	   	else
	   		SetFunStackPrompt (PRMT_POLYGON_NEXT); */
   		break;
   		
    case WM_MBUTTONDOWN:   
    	break;
    	
    case WM_MBUTTONUP:
    	if (HaveLastEP)
    	{ 
			CursorIsLocked = TRUE;
	    	CurrentPoint = LastEP;
	    	goto LButUp;
    	}   
    	break;
    	
    case WM_LBUTTONUP: 
LButUp:
   		if (!EditName[0])
   			break;
		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,0);
		if (CurView->CurrentFunction != GF_DIGITIZE_POLYLINE)
			break;  
		if (NewPolyType) 
	   		SetFunStackPrompt (PRMT_POLYLINE_NEXT);
	   	else
	   		SetFunStackPrompt (PRMT_POLYGON_NEXT); 
/*	   	if (hCurPolyPoints) Added to allow enlarge to be used on first point - causes problems with dig mult lines
	   		break;*/
		PostMessage(hWnd, WM_LBUTTONDOWN,wParam, lParam); 
		PostMessage(hWnd, WM_LBUTTONUP,wParam, lParam);
		if (Message == WM_MBUTTONUP) 
			PostMessage(hWnd, WM_MOUSEMOVE,wParam, lParam); 
		break;
		
    case GF_DISPLAYMESS: 
		if (NewPolyType) 
	   		SetPrompt (PRMT_POLYLINE_INIT,TRUE);
	   	else
	   		SetPrompt (PRMT_POLYGON_INIT,TRUE);
        break;
        
    case GF_COMPLETE:
		if (wParam == GF_LBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
			goto LButUp; 
		}
		if (wParam == GF_RBUTTON)
			break;
		if (NewPolyType) 
	   		SetPrompt (PRMT_POLYLINE_INIT,TRUE);
	   	else
	   	{   
	   		if (NumNewPolyPoints > 2)
	   			NumNewPolyPoints--; //remove right button point
	   		SetPrompt (PRMT_POLYGON_INIT,TRUE);                     
	   	}
    	if (NumNewPolyPoints)
    	{   
    		LPDPOINT	lpDPoints;
    		long		NewRefno; 
    		int			st;  
    		char		Prefix[128],UDI[128];
    		LPSTR		pStuff=0; 
    		HANDLE		hStuff=0;  
    		BOOL		status, Redefine=FALSE;   
    		char		SymName[34]; 
    		COLORREF	NewLineColor, NewAreaColor;
    		float		NewLineWidth;
    		short		NewLineSymbol, NewAreaSymbol; 
			double		NewRouteWidth;
    		HANDLE		hText=0, hTextTPL=0, hTime=0;
		 
    		if (!EditName[0])
    			break;
			CloseMap(TRUE);
    		_fstrcpy (PltName,EditName);
    		PltType = 2;
//			GetTAGForSymbol ("PEN1",Prefix,UDI); 
			if (NewPolyType) 
			{   
				short	Type=NewPolyType; 
				DPOINT	FirstPoint;
				
				lpDPoints =(HPDPOINT)GlobalLock (hNewPolyPoints);
				FirstPoint = *lpDPoints;
				GlobalUnlock (hNewPolyPoints);
				if (HavePOC)
					Type = 3;   
				NewRefno = GetNewRefnoNoUpdate (EditName);
				if (!SetNewMacro (2))
					goto ExitLine;
				if (!SetLineSymbol (&NewLineSymbol,0))
					goto ExitLine;
				if (!SetLineWidth (&NewLineWidth,0))
					goto ExitLine;
				if (!SetRouteWidth (&NewRouteWidth,0))
					goto ExitLine;
				if (!SetLineColor (&NewLineColor,0))
					goto ExitLine;
				if (!SetNewText (2,&hText,&hTextTPL,&FirstPoint))
					goto ExitLine;
				if (!SetNewTime (2,&hTime))
					goto ExitLine;
				_fstrcpy (Prefix,"[%NEW_LINE_PREFIX]");
				_fstrcpy (UDI,"[%NEW_LINE_UDI]"); 
				ExpandText (Prefix);
				ExpandText (UDI); 
				NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewLineSymbol,&Redefine);
				if (!SetNewStuff (2,&hStuff))
					goto ExitLine;
				if (hStuff)
					pStuff = GlobalLock (hStuff);
				if (GetGlobalBVal2 ("[%LINESONLY]",FALSE) )
				{   
					int		nPnts2=2, i;
					HANDLE	hPnts2=GSSiGlobAlloc ( 610,GMEM_MOVEABLE,2*sizeof(DPOINT));
					HPDPOINT	pPoint2=(HPDPOINT)GlobalLock (hPnts2), pNewPP=(HPDPOINT)GlobalLock (hNewPolyPoints);
					
					for (i=1;i<NumNewPolyPoints;i++)
					{   
						pPoint2[0] = pNewPP[i-1];
						pPoint2[1] = pNewPP[i];
						status=AddPolyToMap (1,&nPnts2, &hPnts2,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis,0);
						NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewLineSymbol,&Redefine);
					} 
					GlobalUnlock (hNewPolyPoints);
					GSSiGlobUlFree (&hPnts2);
				}
				else if (NewRouteWidth)
					status=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis,&NewRouteWidth);
				else
					status=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis,0);
				NewSymbol = NewLineSymbol;
				if (status && *Prefix)
				{   
					double	IncVal = GetGlobalDVal2 ("[%NEW_LINE_AUTOINC]",0);
					
					if (AutoIncTAG (Prefix,UDI,IncVal))
						SetGlobalValue ("%NEW_LINE_UDI",UDI);
				}
			}
			else  
			{   
				LPVIEWPORT SaveVP=CurView; 
				DPOINT	FirstPoint;
				
				lpDPoints =(HPDPOINT)GlobalLock (hNewPolyPoints);
				FirstPoint = *lpDPoints;
				GlobalUnlock (hNewPolyPoints);
				
				NewRefno = GetNewRefnoNoUpdate (EditName);
				if (!SetNewMacro (3))
					goto ExitLine;
				if (!SetAreaSymbol (&NewAreaSymbol,0))
					goto ExitLine;
				if (!SetAreaColor (&NewAreaColor,0))
					goto ExitLine;   
				if (!SetNewText (3,&hText,&hTextTPL,&FirstPoint))
					goto ExitLine;
				if (!SetNewTime (3,&hTime))
					goto ExitLine;
				_fstrcpy (Prefix,"[%NEW_AREA_PREFIX]");
				_fstrcpy (UDI,"[%NEW_AREA_UDI]"); 
				ExpandText (Prefix);
				ExpandText (UDI);
				SetCurView (SaveVP);
				NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewAreaSymbol,&Redefine);
				if (!SetNewStuff (3,&hStuff))
					goto ExitLine;
				if (hStuff)
					pStuff = GlobalLock (hStuff); 
				if (Redefine)
					DeleteRedefinedItem (EditName);
				status=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,NewPolyType,NewRefno,hTime,-1,NewAreaSymbol,(LPSHORT)pStuff,Prefix,UDI,NewAreaColor,-1,0,hText,hTextTPL,0,0,DigHiPrecis,0);
				NewSymbol = NewAreaSymbol;   
				if (status && *Prefix)
				{   
					double	IncVal = GetGlobalDVal2 ("[%NEW_AREA_AUTOINC]",0);
					
					if (AutoIncTAG (Prefix,UDI,IncVal))
						SetGlobalValue ("%NEW_AREA_UDI",UDI);
				}
			}
			CloseMap(TRUE);
			CloseRefIndex(FALSE);    		
    		ForceRefIndex = ForceTAGIndex = FALSE;
        	GSSiGlobFree(&hNewPolyPoints);
        	NumNewPolyPoints=0; 
        	if (status)
        	{ 
				if (!GetSymbolName (NewSymbol,SymName,0,2,0))
				{   
					HANDLE	hSymDesc=0;
					short	NumSyms=0;
					
		    		_fstrcpy (PltName,EditName);
		    		PltType = 2;
					AddToSymList (NewSymbol,&NumSyms,&hSymDesc); 
					AddSymToMap (NumSyms,hSymDesc,0,0); 
	                DestroySymList (&NumSyms,&hSymDesc);
				}
				if (!GetVisibility (NewSymbol))
					ToggleVisibility (NewSymbol);
	        	_fstrcpy (PickName,EditName);
	    		PltType = 2;  
				PickList[0].Refno = NewRefno;
				PickList[0].Desc = NewSymbol;
	    		PickList[0].ViewID = CurView->ID;
				PickList[0].ConfigID = CurrentConfig;
	        	PickList[0].Segment = ItemSeg;
	        	PickList[0].Offset = CurrentItem;  
	        	PickList[0].FileNum=CurView->UpdateFile-1;
				PickList[0].SubFile = CurView->SubFile;
			    SetViewport (PickList[0].ViewID);
	        	ProcessPickedItem(0,TRUE);
	        	rtn = GF_INCREASE_SUCCESS_COUNT;
	        }  
	        else
	        	GMMessageBox (MSG_OUTOFEDLIM,0,MB_ICONEXCLAMATION);
ExitLine: 	GSSiGlobUlFree (&hStuff);
    		GSSiGlobFree (&hText); 
		    GSSiGlobFree (&hTextTPL); 
    		GSSiGlobFree (&hTime);   
    		GetNewRefnoNoUpdate (0);
        } 
//        Inited = FALSE;
//        RemoveGraphicsFunction (hWnd);
        break; 
        
    case GF_CLOSE: 
   		Inited = FALSE; 
		SetCurs (InCursor,FALSE); 
        return FALSE;
        
    default:
    	return (FALSE);
    }
    return rtn;
}

BOOL CreateNewPoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{ 
	static	BOOL HaveLocPoint, ShowDrag; 
	DPOINT	AZPoint; 
	POINTS	MousePoint;  
	static	HCURSOR	InCursor;
	static	POINT	DragPoints[2];   
	short	rtn=FALSE, OldMode;
	static	UINT	CurrentPrompt;
	char	str[128];
	
 char key;
 switch (Message)
   {
   	case GF_INIT: 
   		HaveLocPoint = FALSE; 
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		InCursor = CurView->hCursor;
		CreateDigCursor (CurView->hDC);
		SetCurs ((HCURSOR)2,FALSE);
   		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   		_fstrcpy (PltName,EditName);
		sprintf (str,"#%i",CurView->UpdateFile);
		SetLayerVisibility (str,1);
		AddLBUTTON = TRUE; 
		ShowDrag = FALSE; 
       	CurrentPrompt = PRMT_LOCPOINT;
	    SetPrompt (CurrentPrompt,TRUE);
        break;
   	
   	case GF_REDRAW:
   		if (!HaveLocPoint)
   			break;
		DragPoints[0]=BasePtToScreenPt (&CurrentPoint);
   		break;
   		
   	case WM_MOUSEMOVE:  
   		if (!ShowDrag)
   			break; 
   		
		OldMode = SetROP2(CurView->hDC,R2_NOT); 
		TempPolyline (CurView->hDC,DragPoints,2,0,0);
    	DragPoints[1]=POINTStoPOINT(MAKEPOINTS(lParam));
		TempPolyline (CurView->hDC,DragPoints,2,0,0);
    	SetROP2(CurView->hDC,OldMode);
   		
   		break;
    case WM_LBUTTONDOWN:
    {
    	char	str[64];
    	
		SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
    	MousePoint = MAKEPOINTS(lParam);
    	if (CursorIsLocked)
	    	UnlockCursor ();
		else
	        CurrentPoint=WinPtSToBasePt(MousePoint);
	    DragPoints[0]=DragPoints[1]=POINTStoPOINT(MousePoint);
    	HaveLocPoint = TRUE;
		ExpandGlobalRaw ("%NEW_POINT_ROT",TRUE,str,64);
        if (!_fstricmp (str,"@[%DRAGAZ]"))
        {
        	CurrentPrompt = PRMT_DRAGPOINT;
		    SetPrompt (CurrentPrompt,TRUE);
        	ShowDrag=TRUE; 
        }
        else 
        	ShowDrag=FALSE; 
    }
		break;

	case GF_USECMDCOR:		
			rtn = AddNewPoint (CommandPoint,TRUE,0);
	        return rtn;

   	case GF_COMPLETE:
		if (wParam == GF_LBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
	    	MousePoint = MAKEPOINTS(lParam);
	    	HaveLocPoint = TRUE;
		}
		else
			break;
    case WM_LBUTTONUP:
    	if (HaveLocPoint)
    	{   
    		double		AZ;
    		
	       	CurrentPrompt = PRMT_LOCPOINT;
		    rtn = TRUE;
		    HaveLocPoint = FALSE; 
		    if (ShowDrag)
		    {
				OldMode = SetROP2(CurView->hDC,R2_NOT); 
				TempPolyline (CurView->hDC,DragPoints,2,0,0);
		    	SetROP2(CurView->hDC,OldMode);
		    } 
		    ShowDrag = FALSE;
	    	MousePoint = MAKEPOINTS(lParam);
	        AZPoint=WinPtSToBasePt(MousePoint); 
	        AZ = getazd (&CurrentPoint,&AZPoint);
			SetGlobalValueReal ("%DRAGAZ",AZ);
	    	UnlockCursor ();
		    EnlargeScreen (0,0);
			rtn = AddNewPoint (CurrentPoint,TRUE,0);
			if (!rtn)
	        {
	        	GMMessageBox (MSG_OUTOFEDLIM,0,MB_ICONEXCLAMATION);
	        	rtn = TRUE; 
	        }
	        SetPrompt (CurrentPrompt,TRUE);
	        return rtn;
        }
 
        break; 
        
    case GF_CLOSE: 
		SetCurs (InCursor,FALSE);
		ShowDrag = FALSE;
    	return FALSE;
        
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL DistancePolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{  
 char key;
 static	BOOL	Inited=FALSE;   
 static	short	SaveTempLineType;
 switch (Message)
   {
   	case GF_INIT:  
   		Inited = TRUE;
   		SaveTempLineType = TempLineType; 
   		TempLineType = 1;
   		if (Function == GF_SHOW_AREA)
   			TempLineType = 2;
   		if (Function == GF_SHOW_AZ)
   			TempLineType = 3;
//       	AddLBUTTON = TRUE;
   		SetPrompt (PRMT_DIST_LINEBEG,TRUE);
		CreateDigCursor (CurView->hDC);
		SetCurs ((HCURSOR)2,FALSE);
		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,PRMT_DIST_LINEBEG);  
   		break;
   		
//    case WM_LBUTTONUP: 
//		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,PRMT_DIST_LINE);  
//		break;
	
	case GF_CLOSE:
		TempLineType = SaveTempLineType;
		return FALSE;	
    case GF_COMPLETE:
    	if (NumNewPolyPoints > 1) 
    	{
			HPDPOINT	NewPolyPoints; 
			double		dist; 
			
		   	switch (Function)
	    	{
				case GF_SHOW_AZ:
					NewPolyPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
					CurrentAZ = getazd (&NewPolyPoints[0],&NewPolyPoints[NumNewPolyPoints-1]);
					GlobalUnlock (hNewPolyPoints);
				break;

	    		case GF_SHOW_DIST:
			    	ShowCurrentDist (1,0,0,0);
					NewPolyPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
					dist = ldistp (NewPolyPoints[NumNewPolyPoints-1],NewPolyPoints[0]);
					GlobalUnlock (hNewPolyPoints);
			    	if (dist > P_TOL)
			    		break;
			    case GF_SHOW_AREA:  
		    	{   
					LPLONG		pNumDistPoints;
					HPDPOINT	pDistPoints;
	
					GSSiGlobFree (&CurView->hDistanceLine);
					NewPolyPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
					NewPolyPoints[NumNewPolyPoints++] = NewPolyPoints[0];
					CurView->hDistanceLine = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
					pNumDistPoints = (LPLONG)GlobalLock (CurView->hDistanceLine);
					pDistPoints = (HPDPOINT)(pNumDistPoints+1);
					*pNumDistPoints = NumNewPolyPoints;
					hmemmove ((HPSTR)pDistPoints,(HPSTR)NewPolyPoints,NumNewPolyPoints*sizeof(DPOINT));
					GlobalUnlock (CurView->hDistanceLine); 
					GlobalUnlock (hNewPolyPoints);  
					DisplayDistanceLine (FALSE);
				}
		    	break;
		    }
		}
   		Inited = FALSE;    
   		TempLineType = SaveTempLineType;
    	GSSiGlobFree(&hNewPolyPoints);
    	NumNewPolyPoints=0; 
       	RemoveGraphicsFunction (hWnd,0);  
        PostMessage(hWnd, GF_CLOSE,0, 0L);
        break;
        
    default:
    	return (FALSE);
    }
    return (TRUE);
}
BOOL HighlightItem (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (742);
#endif
{HDC hDC;
 char key;     
 int	st,np, SaveNP;
 long	TLID;
 short	rtn=TRUE, SavePOH;
 HIGHLIGHTDATA	HighlightData;
 POINT	MousePoint;
 DPOINT	BasePoint;

 switch (Message)
   {
   	case GF_INIT:
   		switch (Function)
   		{ 
		    case GF_UNHIGHLIGHT:
		    	SetPrompt (PRMT_UNHIGHLIGHT,TRUE); 
		    	break;
		    case GF_TOGGLE_HIGHLIGHT:
			   	SetPrompt (PRMT_TOGGLEHIGHLIGHT,TRUE); 
			   	break;
		    case GF_HIGHLIGHT:
	   			SetPrompt (PRMT_HIGHLIGHT,TRUE); 
	   			break;
	   	}
   		SetPickCursor(TRUE);
       	AddLBUTTON = TRUE;
       	break;
    
    case WM_RBUTTONUP:
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
    	break;
    	
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE); 
	    SavePOH = PickOnlyHighlighted;
		PickOnlyHighlighted = 0;
	    switch (Function)
	    {
		    case GF_UNHIGHLIGHT:
		    	PickOnlyHighlighted = 1; 
		    	break;
		    case GF_HIGHLIGHT:
		    	PickOnlyHighlighted = 2; 
		    	break;
	    }
	    PickItems (hWnd,BasePoint);  
	    PickOnlyHighlighted = SavePOH;   
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;
	    np = NumPicked;
	    while (NumPicked--)
	    {
			int	SavePass = CurView->PassID;

			CurView->PassID = 4;
		    st = BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    if (st || !HighlightData.Show) 
		    {
			    AddToHighlightList (PickList[NumPicked].Refno,&PickList[NumPicked],TRUE);
			    rtn = GF_INCREASE_SUCCESS_COUNT;
			}
			else if (Function != GF_HIGHLIGHT) 
			{
			    RemoveFromHighlightList (PickList[NumPicked].Refno,2);
		    	ShowPickedItem (CurView->hWnd,NumPicked); 
			    RemoveFromHighlightList (PickList[NumPicked].Refno,0);
			    rtn = GF_INCREASE_SUCCESS_COUNT;
			}
			SaveNP = NumPicked;
	    	ShowPickedItem (hWndMain,NumPicked); 
	    	NumPicked = SaveNP;
			CurView->PassID = SavePass;
	    }
	    if (np)
			SetPickGlobals (np-1); 
		setDoPaint( TRUE);
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (742);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (742);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
} 

BOOL SetStreetName (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (744);
#endif
{HDC hDC;
 char key;     
 int	st;

 switch (Message)
   {
   	case GF_INIT: 
       	AddLBUTTON = TRUE;
   		break;  
   		
	case WM_LBUTTONUP:

   	{
		DLGPROC lpfnGETSTREETNUMMsgProc; 
		BOOL	rc;
			
		lpfnGETSTREETNUMMsgProc = MakeProcInstance((DLGPROC)GETSTREETNUMMsgProc, hInst);
		rc=DialogBox(hInst, (LPSTR)"GETSTREETNUM", hWndMain, lpfnGETSTREETNUMMsgProc);
		FreeProcInstance(lpfnGETSTREETNUMMsgProc);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 

    }
       	break;

    default:
{
#if ENABLETRACE
GSSiExitProg (744);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (744);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL CreateNetLink (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short function)
{HDC hDC;
 char key;     
 int	st, DoADD,ii;
 long	TLID, WantPath=0;
 HIGHLIGHTDATA	HighlightData;
 POINT	MousePoint;
 DPOINT	BasePoint;
 //BTHEAD	BTHead;   
 char	mess[128];  
 OFSTRUCTGM	OFStruct;
 static	long	TotLen;
 long	CurLoc, StreetNums[32], NumStreets; 
 struct {long	Ref, SNum;} RouteRefKey;  
 long	OppEndLinkID;
 long	NumInChain, nInChain;
 LPLONG	pChain;
 HANDLE	hChain=0;
 

 switch (Message)
   {
   	case GF_INIT:
   		if (function == GF_DEFINE_STREET_LINK_AUTO) 
   		{
			char	NetChainFile[MAX_PATH];

			GetTempDir (NetChainFile);
			strcat (NetChainFile,"\\NetChain.bin");
			FidChain = GSSiOpenFile (NetChainFile,&OFStruct,OF_READWRITE); 
			TotLen = GSSillseek (FidChain,0,2);
			GSSillseek (FidChain,0,0);    
			CreateStatusWind (CurView->hWnd,1,0);
			goto NextChain;
		}	
		else
			FidChain = HFILE_ERROR;
		gbUserAbort=FALSE;	
       	AddLBUTTON = TRUE;

   		break;

    case WM_LBUTTONUP: 
NextChain:
    	if (FidChain != HFILE_ERROR)
    	{    
    		BOOL	First, GotWant=FALSE;  
    		short	st, Dummy, i;
    		
    		GSSiGlobFree (&hChain);
			CurLoc = GSSillseek (FidChain,0,1);
			StatusWindowUpdate ("Auto Network Creation",CurrentStreetName, TotLen, CurLoc);
			if (!BigRead (FidChain,(HPSTR)&NumInChain,4) || gbUserAbort)
			{
				goto Exit;
			}
			BigRead (FidChain,(HPSTR)StreetNums,16);
			hChain = GSSiGlobAlloc ( 532,GMEM_MOVEABLE,NumInChain*4);
			pChain = (LPLONG)GlobalLock (hChain);
			BigRead (FidChain,(HPSTR)pChain,(size_t)(4*NumInChain)); 
			First = TRUE; 
			NumStreets=0;
			while (NumStreets < 4 && StreetNums[NumStreets])
				NumStreets++; 
			GlobalUnlock (hChain);
			if (WantPath && !GotWant)
			{
				goto NextChain;     
			}  
    	} 
    	else
    	{
    		NumStreets = 1;
    		StreetNums[0] = CurPath; 
    		hChain = 0;
    	}
    	while (NumStreets--)
    	{

    		CurPath = StreetNums[NumStreets];
	    	if (!CurPath)
	    	{   
	    		if (FidChain != HFILE_ERROR)
	    			goto NextChain;
				MessageBox(hWnd,"Street Name Not Set","", MB_ICONEXCLAMATION);  
		        PostMessage(hWnd, GF_CLOSE,0, 0L);
				break;
			} 
			if (hChain)
			{
	    		long	Sequence=0, nInChain;
	    		
				ClearHighlightList(FALSE);
				nInChain = NumInChain;
				pChain = (LPLONG)GlobalLock (hChain); 
				while (nInChain--)
				{	
		if (*pChain == -2147436847)
			ii=1;			                                      
					PickByRefno(*pChain,0,0,-1);
					Sequence += 1000;     
					AddToHighlightListSeq (*pChain++,&PickList[0],Sequence,TRUE);
				}
				GlobalUnlock (hChain);
			}
			if (CurPath == 286)
				ii=1;
			if (BT_NUM_IN_INDEX (hHighlight2))
				goto GotHighlight;

//			if (GetBTHeader (hHighlight2,&BTHead))
//				if (BTHead.BT_NUMRECS) goto GotHighlight;
			MessageBox(hWnd,"No segments highlighted","", MB_ICONEXCLAMATION);
	        PostMessage(hWnd, GF_CLOSE,0, 0L);
			break;
	GotHighlight:
			GetTrueStreetName (CurPath, CurrentStreetName, 0,0); 
	    	if (FidChain == HFILE_ERROR)
	     	{
	     		sprintf (mess,"Add these segments to '%s'?",CurrentStreetName);
			 	DoADD = MessageBox (hWnd,mess,"",MB_YESNO);
			}
			else 
				DoADD = IDYES;
	GotHighlight2:
			if (DoADD == IDYES)
		 	{   
		 		BOOL	Opened, First=TRUE, HaveGaps=FALSE; 
		 		int		st, FirstDir, SecondDir, Pass, NetDir=1, SnapEnd2, FirstFirstDir;
		 		double	Dist1, Dist2, MP, StartMP=0, TotLength, MaxGap=0, FixGap=0, GapDist, FirstLength;
		 		long	Ref,FirstRef, Sequence;  
				NETREFSKEY	NetRefsKey;
				NETREFSDATA	NetRefsData;  
		 		HIGHLIGHTDATA	HighlightData1, HighlightData2;
		 		DPOINT	FirstPoint, LastPoint, GapPoint;
		 		
		 		TotLength = 0;
		 		Pass=0;  
		 		FirstDir = 1;
	NextPass:   
		 		SecondDir = 1;
				First = TRUE;
			    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&Ref); 
			    while (!st)
			    {
			    	BT_FIND (hHighlight,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&HighlightData2);  
			    	TotLength += HighlightData2.PD.Length;
			    	if (First)
			    	{
			    		HighlightData1 = HighlightData2;
			    		if (FixGap)
			    			FirstRef = LONG_MIN; 
						else
			    			FirstRef = Ref;
			    		First = FALSE;      
			    		FirstLength = HighlightData1.PD.Length;
			    	}
			    	else 
			    	{   if (FirstRef > LONG_MIN)
			    		{   
			    			FirstDir = 2;   
			    			SecondDir = 1;
				    		Dist1 = ldistp (HighlightData1.PD.BeginPoint,HighlightData2.PD.BeginPoint);
				    		Dist2 = ldistp (HighlightData1.PD.EndPoint,HighlightData2.PD.BeginPoint);
				    		GapPoint = HighlightData1.PD.BeginPoint;
				    		if (Dist2 < Dist1)
				    		{
				    			Dist1 = Dist2;
				    			FirstDir = 1; 
				    			GapPoint = HighlightData1.PD.EndPoint;
				    		}
				    		Dist2 = ldistp (HighlightData1.PD.BeginPoint,HighlightData2.PD.EndPoint);
				    		if (Dist2 < Dist1)
				    		{
				    			Dist1 = Dist2;
				    			FirstDir = 2; 
				    			SecondDir = 2;
				    			GapPoint = HighlightData1.PD.BeginPoint;
				    		}
				    		Dist2 = ldistp (HighlightData1.PD.EndPoint,HighlightData2.PD.EndPoint);
				    		if (Dist2 < Dist1)
				    		{
				    			Dist1 = Dist2;
				    			FirstDir = 1; 
				    			SecondDir = 2;
				    			GapPoint = HighlightData1.PD.EndPoint;
				    		}
				    		if (Dist1 > NetTOL) 
				    		{
				    			MaxGap = max (MaxGap,Dist1);
				    			HaveGaps = TRUE; 
				    			DisplayMarkers = TRUE;
				    			DisplayMarker (GapPoint,3,0,0,0,0,TRUE,FALSE,0,0,0,0,0);
				    			DisplayMarkers = FALSE;           
				    		}
				    		if (NetDir > 0)
				    			MP = StartMP + HighlightData1.PD.Length;
				    		else
				    			MP = StartMP - HighlightData2.PD.Length;
				    		if (FirstDir == 1)
				    			FirstPoint = HighlightData1.PD.BeginPoint;
				    		else
				    			FirstPoint = HighlightData1.PD.EndPoint;   
				    		FirstLength = HighlightData1.PD.Length;
				    		if (Pass)
				    		{
								PutNetRefAndLink (NetworkID,CurPath,FirstRef,FirstDir,StartMP,HighlightData1.PD.Length,NetDir);
								PutNetRefAndLink (NetworkID,CurPath,Ref,SecondDir,MP,HighlightData2.PD.Length,NetDir); 
							}
							FirstFirstDir = FirstDir;
				    	}
				    	else
				    	{   
				    		if (FirstDir == 1)
				    		{
				    			Dist1 = ldistp (HighlightData1.PD.EndPoint,HighlightData2.PD.BeginPoint);
				    			Dist2 =	ldistp (HighlightData1.PD.EndPoint,HighlightData2.PD.EndPoint);
					    		GapDist = min (Dist1,Dist2);
					    		if (GapDist > NetTOL) 
					    		{   
					    			if (GapDist < FixGap)
					    			{   
					    				if (Dist1 < Dist2)
					    				{
					    					SnapPoint = AverageDPoint (HighlightData1.PD.EndPoint,
					    											  HighlightData2.PD.BeginPoint);
					    					SnapEnd2 = 1;
					    				}
					    				else
					    				{
					    					SnapPoint = AverageDPoint (HighlightData1.PD.EndPoint,
					    											  HighlightData2.PD.EndPoint);
					    					SnapEnd2 = 2;
					    				}
					    				PickList[0]=HighlightData1.PD;
					    				SnapEnd = 2; 
					    				PickList[0].PCT = 1;
					    				SnapPickedItem (0);
					    				PickList[0]=HighlightData2.PD; 
					    				SnapEnd = SnapEnd2;
					    				if (SnapEnd == 2 || SnapEnd == 4)
						    				PickList[0].PCT = 1;
						    			else
						    				PickList[0].PCT = 0;
					    				SnapPickedItem (0);
					    				if (FidChain != HFILE_ERROR)
					    				{
					    					GSSillseek (FidChain,CurLoc,0); 
					    					goto NextChain;
					    				}
					    			}
					    			else
					    			{
					    				GapPoint = HighlightData1.PD.EndPoint;
						    			MaxGap = max (MaxGap,min (Dist1,Dist2) );
						    			HaveGaps = TRUE;            
						    			DisplayMarkers = TRUE;
						    			DisplayMarker (GapPoint,3,0,0,0,0,TRUE,FALSE,0,0,0,0,0);
						    			DisplayMarkers = FALSE;  
						    		}         
					    		}
				    			if (Dist1 < Dist2)
				    				SecondDir = 1;
				    			else
				    				SecondDir = 2;
				    		}
				    		else
				    		{
				    			Dist1 = ldistp (HighlightData1.PD.BeginPoint,HighlightData2.PD.BeginPoint);
				    			Dist2 =	ldistp (HighlightData1.PD.BeginPoint,HighlightData2.PD.EndPoint);
					    		GapDist = min (Dist1,Dist2);
					    		if (GapDist > NetTOL) 
					    		{   
					    			if (GapDist < FixGap)
					    			{   
					    				if (Dist1 < Dist2)
					    				{
					    					SnapPoint = AverageDPoint (HighlightData1.PD.BeginPoint,
					    											  HighlightData2.PD.BeginPoint);
					    					SnapEnd2 = 1;
					    				}
					    				else
					    				{
					    					SnapPoint = AverageDPoint (HighlightData1.PD.BeginPoint,
					    											  HighlightData2.PD.EndPoint);
					    					SnapEnd2 = 2;
					    				}
					    				PickList[0]=HighlightData1.PD;
					    				SnapEnd = 1;
					    				PickList[0].PCT = 0;
					    				SnapPickedItem (0);
					    				PickList[0]=HighlightData2.PD;
					    				SnapEnd = SnapEnd2;
					    				if (SnapEnd == 2 || SnapEnd == 4)
						    				PickList[0].PCT = 1;
						    			else
						    				PickList[0].PCT = 0;
					    				SnapPickedItem (0); 
					    				if (FidChain != HFILE_ERROR)
					    				{
					    					GSSillseek (FidChain,CurLoc,0); 
					    					goto NextChain;
					    				}
					    			}
					    			else
					    			{
					    				GapPoint = HighlightData1.PD.BeginPoint;
						    			MaxGap = max (MaxGap,min (Dist1,Dist2) );
						    			HaveGaps = TRUE;            
						    			DisplayMarkers = TRUE;
						    			DisplayMarker (GapPoint,3,0,0,0,0,TRUE,FALSE,0,0,0,0,0);
						    			DisplayMarkers = FALSE;
						    		}           
					    		} 
					    		if (Dist1 < Dist2)
				    				SecondDir = 1;
				    			else
				    				SecondDir = 2;
				    		}
	//			    		MP += (NetDir * HighlightData1.PD.Length);
				    		if (NetDir > 0)
				    			MP += HighlightData1.PD.Length;
				    		else
				    			MP -= HighlightData2.PD.Length;
	
							if (Pass)
								PutNetRefAndLink (NetworkID,CurPath,Ref,SecondDir,MP,HighlightData2.PD.Length,NetDir);
				    	}
	                    FirstDir = SecondDir;
			    		FirstRef = LONG_MIN; 
			    		HighlightData1 = HighlightData2;
			    	}
		    		st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&Ref); 
			    } 
			    if (FirstRef > LONG_MIN)
			    {   
			    	if (Pass)
			    	{
						PutNetRefAndLink (NetworkID,CurPath,FirstRef,FirstDir,StartMP,HighlightData1.PD.Length,NetDir);
			    	}
			    	else
			    	{
						FirstPoint = HighlightData1.PD.BeginPoint;		    
						LastPoint = HighlightData1.PD.EndPoint;
					}		    
			    }   
			    
			    if (Pass)
			    {
		 			Sequence = 0;
		 			if (OppEndLinkID >= 0)
		 			{
						ClearHighlightList(FALSE);
						NetRefsKey.Path = CurPath;
						NetRefsKey.MP	= (double)OppEndLinkID * 1000000;
			    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);  	 
			    		while (!st && NetRefsKey.Path == CurPath && (long)(NetRefsKey.MP/1000000) == OppEndLinkID)
			    		{
							PickByRefno(NetRefsData.Ref,0,0,-1);
							Sequence += 1000;     
							AddToHighlightListSeq (NetRefsData.Ref,&PickList[0],Sequence,TRUE);
				    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_NEXT,BT_ANY,(LPSTR)&NetRefsData);  	 
				    	} 
				    NextDelete: 
						NetRefsKey.Path = CurPath;
						NetRefsKey.MP	= (double)OppEndLinkID * 1000000;
			    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);  	 
			    		if (!st && NetRefsKey.Path == CurPath && (long)(NetRefsKey.MP/1000000) == OppEndLinkID) 
			    		{   
			    			BT_DELETE (hBTNetRefs,(LPSTR)&NetRefsKey,(LPSTR)&NetRefsData,FALSE);
			    			goto NextDelete;
			    		}
				       	CloseNetLinkAndRef (Opened); 
		 			    goto GotHighlight2;
		 			}
		 			CloseNetLinkAndRef (Opened); 
		 		} 
		 		else
		 		{
		 			if (HaveGaps)
		 			{   
		 				/*char	Mess[256];
		 				sprintf (mess,"This section contains one or more gaps of up to %f base units. Do you want the system to close them?",
		 						 MaxGap);
		 				if (GetGlobalBVal2 ("[%FIXNETGAPS]",TRUE) ||
		 					MessageBox (hWnd,mess,0,MB_YESNO) == IDYES)
		 				{
		 					FixGap = MaxGap + 0.01;      
		 					HaveGaps = FALSE; 
		 					FirstDir = FirstFirstDir;
		 					goto NextPass;
		 				}
		 				else */ 
		 				{
					    	if (FidChain != HFILE_ERROR)
								goto NextChain;	 	
		 				    else
		 						goto Exit;
		 				}
		 			}
		 			else if (FixGap > 0)
		 			{
		 				FixGap = 0; 
		 				FirstDir = 1;
		 				goto NextPass;
					}	 			
		 			else
		 			{   
		 				DPOINT	FirstNetPoint, LastNetPoint;
		 				double	FirstMP, LastMP, StartMPEnd[8];
		 				short	NetDirEnd[8], GotEnd;
		 				
		 				Pass = 1; 
		 				OppEndLinkID = -1;
			    		if (SecondDir == 1)
			    			LastPoint = HighlightData2.PD.EndPoint;
			    		else
			    			LastPoint = HighlightData2.PD.BeginPoint;
		 				OpenNetLinkAndRef (NetworkID,TRUE,&Opened); 
						StartMP = 500000; 
						NetDir = 1;   
						GotEnd = 0;
				NextNet:
						NetRefsKey.Path = CurPath;
						NetRefsKey.MP	= StartMP - 500000;
			    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);  	 
			    		if (st || NetRefsKey.Path != CurPath)
			    		{   
							OppEndLinkID = -1;
 						    switch (GotEnd)
							{
								case 2: 
									OppEndLinkID = StartMPEnd[1]/1000000;
								case 1: 
									StartMP = StartMPEnd[0];
									NetDir = NetDirEnd[0];
								break;
							}
							FirstDir = 1;		
			    			goto NextPass;
			    		}
			    		if (!PickByRefno (NetRefsData.Ref,0,0,-1))
			    		{
			    			MessageBox(hWnd,"Cannot locate existing network segments. Rebuild reference index",0,MB_ICONEXCLAMATION);
		 					CloseNetLinkAndRef (Opened); 
			    			goto Exit;
			    		} 
			    		FirstMP = NetRefsKey.MP;
			    		if (NetRefsData.Dir == 1)
			    			FirstNetPoint = PickList[0].BeginPoint;
			    		else
			    			FirstNetPoint = PickList[0].EndPoint;   
			    		NetRefsKey.MP = (NetRefsKey.MP - fmod (NetRefsKey.MP,1000000)) + 1000000;
			    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);  	 
			    		if (st) 
				    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_LAST,BT_ANY,(LPSTR)&NetRefsData);  	 
				    	else
				    		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_PRIOR,BT_ANY,(LPSTR)&NetRefsData);  	 
			    		if (!PickByRefno (NetRefsData.Ref,0,0,-1))
			    		{
			    			MessageBox(hWnd,"Cannot locate existing network segments. Rebuild reference index",0,MB_ICONEXCLAMATION);
		 					CloseNetLinkAndRef (Opened); 
			    			goto Exit;
			    		}
				    	LastMP = NetRefsKey.MP + PickList[0].Length;
			    		if (NetRefsData.Dir == 1)
			    			LastNetPoint = PickList[0].EndPoint;
			    		else
			    			LastNetPoint = PickList[0].BeginPoint;   
			    		if (ldistp(FirstPoint,FirstNetPoint)<NetTOL)
			    		{
			    			StartMPEnd[GotEnd] = FirstMP - FirstLength; 
			    			NetDirEnd[GotEnd++] = -1;
			    		}
			    		if (ldistp(FirstPoint,LastNetPoint)<NetTOL)
			    		{
			    			StartMPEnd[GotEnd] = LastMP;
			    			NetDirEnd[GotEnd++] = 1;
			    		}
			    		if (ldistp(LastPoint,FirstNetPoint)<NetTOL)
			    		{
			    			StartMPEnd[GotEnd] = FirstMP - TotLength; 
			    			NetDirEnd[GotEnd++] = 1;
			    		}
			    		if (ldistp(LastPoint,LastNetPoint)<NetTOL)
			    		{
			    			StartMPEnd[GotEnd] = LastMP + TotLength - FirstLength; 
			    			NetDirEnd[GotEnd++] = -1;
			    		} 
			    		StartMP += 1000000;
		 				goto NextNet;
		 			}
		 		}
		 	}
		}  
   		GSSiGlobFree (&hChain);
    	if (FidChain != HFILE_ERROR)
			goto NextChain;	 	
Exit:   
   		GSSiGlobFree (&hChain);
		if (FidChain != HFILE_ERROR)
		{
			DestroyStatusWindow (0);
			GSSiClose (FidChain);
	        FidChain = HFILE_ERROR; 
		}
        PostMessage(hWnd, GF_CLOSE,0, 0L);
		
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}
BOOL DisplayNetInfo (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key;     
 int	st; 
 POINT	MousePoint;
 DPOINT	BasePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
			DLGPROC lpfnDISPLAYNETINFOMsgProc; 
			BOOL	rc;
				
			lpfnDISPLAYNETINFOMsgProc = MakeProcInstance((DLGPROC)DISPLAYNETINFOMsgProc, hInst);
			rc=DialogBox(hInst, (LPSTR)"DISPLAYNETINFO", hWndMain, lpfnDISPLAYNETINFOMsgProc);
			FreeProcInstance(lpfnDISPLAYNETINFOMsgProc);
    	} 
    	setDoPaint( TRUE);
       	break;

    case WM_KEYDOWN:
		key = wParam;
		switch (key)
		{
            case 'v':
            case 'V': //v  
				LoadVideoIndex ("INDEXES\\MN95.SF");   
				return TRUE;  

								
/*			case 'R':	
			case 'r':    
		    {     
		    	FILE	*Fid;  
		    	char	str[1024], txt[128];
		    	DPOINT	MPPoint;
				double			MPinc, PCT, MPVal, AZ;  
				char			File[]="file20.txt";
				LPSTR	lpSpace;    
				long	WantStreetNum;  
				char			snam[34], TrueName[34]="";  
				int	iroute, i, NumChecked, Dummy;     
				HCURSOR	hcurSave;   
				struct {long	Ref, SNum;} RouteRefKey;  
				BTVARDESC	BTVar[2]; 
				HANDLE	hBT;
				short	SaveMaxPick;
		
		        if (!NetUpdate) break;
				hcurSave = SetCursor(LoadCursor(0, IDC_WAIT));
			    
				UseUserPickAp =FALSE;
				SystemPickAp = 0;	
				SaveMaxPick = MaxPick;        
				MaxPick=8;  
		    	SetViewport(*pCommandViewport);
				SetDisplayMode (CurView->hDC, GF_TEXTMODE);
				CurView->hRgn = CreateVPRgn ();
			  	SelectClipRgn (CurView->hDC,CurView->hRgn);
			  	DeleteObject(CurView->hRgn);
				Fid = fopen (File,"r");
				if (!Fid)
				{
					MessageBox( GetFocus(), File,"Unable to open data file", MB_OK);
					break;
				} 
				BTVar[0].BT_VARTYP=BT_INTEGER;
				BTVar[0].BT_VARLEN=4;
				BTVar[0].BT_VAROFF=0;
				BTVar[1].BT_VARTYP=BT_INTEGER;
				BTVar[1].BT_VARLEN=4;
				BTVar[1].BT_VAROFF=4;
				BT_CREATE ("routeref.btr", 2, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
				hBT = BT_OPEN ("routeref.btr", 0, BT_WRITE, 0); 
				fgetss (str,1024,Fid);
				ProcessDelimTextHeader(str);
			     
				while (fgetss (str,1024,Fid))
				{
		          	GetDelimTextData(str); 
		          	_fstrcpy (str,"[easting]");
		          	ExpandText (str);
		          	MPPoint.x = atof (str)*FTM;
		          	_fstrcpy (str,"[northing]");
		          	ExpandText (str);
		          	MPPoint.y = atof (str)*FTM;
		          	_fstrcpy (txt,"[id1]");
		          	ExpandText (txt);  
		          	iroute = atoi(txt);
		          	_fstrcpy (snam,"ROUTE ");
		          	_fstrcat (snam,txt);  
//					StreetNum = GetStreetNumFromName (snam,1,BT_FIRST,TrueName); 
					StreetNum = AddStreetName (snam,0,"","","","");	
					if (!StreetNum)
						goto NextPoint;
				    setDoPaint( FALSE);
				    PickItems (hWnd,MPPoint); 
				    setDoPaint( TRUE);
				    while (NumPicked--)
				    {   
				    	RouteRefKey.Ref = PickList[NumPicked].Refno;
				    	RouteRefKey.SNum = StreetNum; 
				    	if (PickList[NumPicked].OffDist <= NetTOL)
				    	{
				    		if (!BT_FIND (hBT,(LPSTR)&RouteRefKey,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
				    			RouteRefKey.SNum = -StreetNum; 	
							BT_PUT (hBT,(LPSTR)&RouteRefKey,(LPSTR)&Dummy); 
						}
		          	}
		NextPoint:;          
				  }
				SetCursor (hcurSave);
		 		fclose (Fid); 
		 		BT_CLOSE (hBT);
				UseUserPickAp = TRUE; 
				MaxPick = SaveMaxPick; 
				
		    }  
					
				break;  */
/* 
			case 'X':	
			case 'x':    
		    {     
		    	FILE	*Fid;  
		    	char	str[1024], txt[128];
				char			File[]="routeref.txt";
				LPSTR	lpSpace;    
				long	WantStreetNum;  
				char			snam[34], TrueName[34]="";  
				int	iroute, i, NumChecked, Dummy, st;     
				HCURSOR	hcurSave;  
				long	Ref; 
				struct {long	Ref, SNum;} RouteRefKey;  
				BTVARDESC	BTVar[2]; 
				HANDLE	hBT;
				short	SaveMaxPick;
		
		        if (!NetUpdate) break;
				hcurSave = SetCursor(LoadCursor(0, IDC_WAIT));
			    
				Fid = fopen (File,"r");
				if (!Fid)
				{
					MessageBox( GetFocus(), File,"Unable to open data file", MB_OK);
					break;
				} 
				hBT = BT_OPEN ("routeref.btr", 0, BT_WRITE, 0); 
				fgetss (str,1024,Fid);
				ProcessDelimTextHeader(str);
			     
				while (fgetss (str,1024,Fid))
				{
		          	GetDelimTextData(str); 
		          	_fstrcpy (str,"[INT_REFNO]");
		          	ExpandText (str); 
		          	Ref = atol (str);
		          	st=BT_FIND (hBT,(LPSTR)&RouteRefKey,BT_FIRST,BT_GE,(LPSTR)&Dummy);
		          	while (!st && RouteRefKey.Ref == Ref && RouteRefKey.SNum <0)
		          	{                                    
		          		BT_DELETE (hBT,(LPSTR)&RouteRefKey,(LPSTR)&Dummy,FALSE);
			          	RouteRefKey.Ref = Ref; 
			          	RouteRefKey.SNum = LONG_MIN;
		          		st=BT_FIND (hBT,(LPSTR)&RouteRefKey,BT_FIRST,BT_GE,(LPSTR)&Dummy);
		          	}
		          	_fstrcpy (str,"[ROUTE]");
		          	ExpandText (str);
		          	_fstrcpy (snam,"ROUTE ");
		          	_fstrcat (snam,str);
		          	if (!*str || !_fstricmp (str,"00")|| !_fstricmp (str,"0"))
		          		goto NextRef;  
//					StreetNum = GetStreetNumFromName (snam,1,BT_FIRST,TrueName); 
					StreetNum = AddStreetName (snam,0,"","","","");	
					if (!StreetNum)
						goto NextRef;
			        RouteRefKey.Ref = Ref; 
			    	RouteRefKey.SNum = -StreetNum;
			    	if (RouteRefKey.Ref) 
						BT_PUT (hBT,(LPSTR)&RouteRefKey,(LPSTR)&Dummy);
		NextRef:;          
				}
				SetCursor (hcurSave);
		 		fclose (Fid); 
		 		BT_CLOSE (hBT);
			}*/				
            default: 
            	return FALSE;
		} 
       	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL DisplayNetMarkers (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key;     
 int	st; 
 BOOL	HaveDown=FALSE; 
 POINT	MousePoint;
 DPOINT	BasePoint;
 MARKERVAL	NetMarker;
 char	str[256];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
 	    MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {      
/*    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break;
	    BasePoint=WinPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
	    	DPOINT	MPPoint;
	    	BOOL	Opened, Opened2;
	    	int		st,st2;
			NETLINKSKEY	NetLinksKey; 
			NETLINKSDATA	NetLinksData;
			NETREFSKEY	NetRefsKey;
			NETREFSDATA	NetRefsData; 
			NETMARKERSKEY1	NetMarkersKey1;
			NETMARKERSKEY2	NetMarkersKey2;
			NETMARKERSDATA1 NetMarkersData1;
			NETMARKERSDATA2	NetMarkersData2;
			double			MPinc, PCT, MPVal, AZ;  
			char			File[]="file20.txt";
			char			snam[34], TrueName[34];  
			int	iroute, i, NumChecked;     
			long	Checked[32];
			LPSTR	lpSpace;
		    	        
	    	SetViewport(*pCommandViewport);
			SetDisplayMode (CurView->hDC, GF_TEXTMODE);
			CurView->hRgn = CreateVPRgn ();
		  	SelectClipRgn (CurView->hDC,CurView->hRgn);
		  	DeleteObject(CurView->hRgn); 
		  	
		 	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
	 		NetLinksKey.Ref = PickList[NumPicked-1].Refno;
	 	 	NetLinksKey.NetID = 0;
	 	 	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
	 	 	if (!st && NetLinksKey.Ref == PickList[NumPicked-1].Refno)
	 	 	{  
				NetRefsKey.Path =  NetLinksKey.Path;
				NetRefsKey.MP = NetLinksData.MP; 
				st2 = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);
				PCT = PickList[NumPicked-1].PCT;  
				if (NetRefsData.Dir == 2)
					PCT = 1.0 - PCT;
				MPinc = PCT * PickList[NumPicked-1].Length;
				NetLinksData.MP += MPinc;  
		    	GetTrueStreetName (NetLinksKey.Path,TrueName);
		 	 	sprintf (str,"%s\t%f",TrueName,NetLinksData.MP);
				if (GetMilePointFromMP (NetLinksKey.Path,1,NetLinksData.MP, &NetMarker))
					sprintf (str2,"\t%c%c%f%c%c",NetMarker.Prefix[0],NetMarker.Prefix[1],NetMarker.Value,
											   NetMarker.Suffix[0],NetMarker.Suffix[1]);
				else 
					str2[0]=0; 
				_fstrcat (str,str2);
				SendDlgItemMessage (hWndDlg,IDC_NET_INFO,LB_ADDSTRING,0,(LPARAM)str); 
		 	 	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
		 	 }  
		 	 CloseNetLinkAndRef (Opened);
		  	
			 OpenNetMarkers (NetworkID,FALSE,&Opened2);  
		     
			  while (fgetss (str,1024,Fid))
			  {
	          	GetDelimTextData(str); 
	          	_fstrcpy (str,"[fbeg_easting]");
	          	ExpandText (str);
	          	MPPoint.x = atof (str)*FTM;
	          	_fstrcpy (str,"[fbeg_northing]");
	          	ExpandText (str);
	          	MPPoint.y = atof (str)*FTM;
	          	_fstrcpy (txt,"[id1]");
	          	ExpandText (txt);  
	          	iroute = atoi(txt);
	          	_fstrcpy (snam,"ROUTE ");
	          	_fstrcat (snam,txt);  
				StreetNum = GetStreetNumFromName (snam,1,BT_FIRST,TrueName);
	          	_fstrcpy (str,"[beginmi]");
	          	ExpandText (str); 
	          	MPVal = atof (str); 
			    setDoPaint( FALSE);
			    PickItems (hWnd,MPPoint); 
			    setDoPaint( TRUE);
			    NumChecked = 0;
			    while (NumPicked--)
			    {   
					NetLinksKey.Ref = PickList[NumPicked].Refno;
					NetLinksKey.NetID = 0;
					st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
					while (!st && NetLinksKey.Ref == PickList[NumPicked].Refno)
					{   	
						for (i=0;i<NumChecked;i++)
							if (NetLinksKey.Path == Checked[i])
								goto NextPick;
						if (PickList[NumPicked].OffDist > NetTOL)
							goto NextPick;
						Checked[NumChecked++] = NetLinksKey.Path;
				    	GetTrueStreetName (NetLinksKey.Path,TrueName);
				    	lpSpace = _fstrchr (TrueName,' ');
				    	if (!lpSpace) lpSpace = TrueName;
						if (iroute && (iroute == atoi (lpSpace)))
						{
							NetRefsKey.Path =  NetLinksKey.Path;
							NetRefsKey.MP = NetLinksData.MP; 
							st2 = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);
							PCT = PickList[NumPicked].PCT;  
							AZ	= PickList[NumPicked].AZ;
							if (NetRefsData.Dir == 2)
								PCT = 1.0 - PCT;
							MPinc = PCT * PickList[NumPicked].Length;
							NetLinksData.MP += MPinc;  
							NetMarkersKey1.MarkerID=1;
							NetMarkersKey1.Path = NetLinksKey.Path;
							_fmemset (NetMarkersKey1.Prefix,' ',2);   
							_fmemset (NetMarkersKey1.Suffix,' ',2);
							NetMarkersKey1.Value = atof (str); 
							NetMarkersData1.Location = MPPoint;
							NetMarkersData1.MP = NetLinksData.MP; 
							NetMarkersKey2.MarkerID=1;
							NetMarkersKey2.Path=NetLinksKey.Path;
							NetMarkersKey2.MP=NetLinksData.MP; 
							_fmemset (NetMarkersData2.Prefix,' ',4);
							NetMarkersData2.Value = NetMarkersKey1.Value; 
							
							BT_PUT (hBTNetMarkers1,(LPSTR)&NetMarkersKey1,(LPSTR)&NetMarkersData1);
							BT_PUT (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,(LPSTR)&NetMarkersData2);
						}
						st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
					}  
	NextPick:;					          		
	          	}
			CloseNetLinkAndRef (Opened);
			CloseNetMarkers (Opened2);  
			CloseStreetNameTable ();
*/        
    	} 
    	setDoPaint( TRUE);
       	break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL DeleteNetMarker (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key;     
 int	st; 
 BOOL	HaveDown=FALSE; 
 POINT	MousePoint;
 DPOINT	BasePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
	    MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        if (NumPicked > 0)
        {
			DLGPROC lpfnDISPLAYNETINFOMsgProc; 
			BOOL	rc;
				
			lpfnDISPLAYNETINFOMsgProc = MakeProcInstance((DLGPROC)DISPLAYNETINFOMsgProc, hInst);
			rc=DialogBox(hInst, (LPSTR)"DISPLAYNETINFO", hWndMain, lpfnDISPLAYNETINFOMsgProc);
			FreeProcInstance(lpfnDISPLAYNETINFOMsgProc);
    	} 
    	setDoPaint( TRUE);
       	break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL EditPickMacro (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (749);
#endif
{HDC hDC;
 char key;     
 int	st; 

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
        { 
			char PMFile[MAX_PATH];

			GetPMName(CurView->PickMacroFile,PMFile);
		    GMEdit (hWndMain,PMFile);
        }
       	break;

	case GSSI_GMEDITCOMPLETE:
		PostMessage(hWnd, GF_CLOSE, 0, 0L);
		break;

	default:
{
#if ENABLETRACE
GSSiExitProg (749);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (749);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL ClearRedef(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (750);
#endif
{HDC hDC;
 char key;     
 int	st; 

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
        { 
		    OFSTRUCTGM OFStruct;
		     
		    GSSiRemove (CurView->DisplayRedefFile);
		    RemoveVPRedef ();
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
           	PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
			 
        }

       	break;

    default:
{
#if ENABLETRACE
GSSiExitProg (750);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (750);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ToggleFunStack (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
#if ENABLETRACE
{GSSiEnterProg (752);
#endif
{HDC hDC;
 char key;     
 int	st; 

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
        {   
        	switch (Function)
        	{
        		case GF_CLEAR_MASK:
        			ClearMaskArea();
        			break;
        		case GF_TOGGLE_FUNSTACK:
		        	if (CurView->DisplayFunStack)
		        		CurView->DisplayFunStack = FALSE;
		        	else
		        		CurView->DisplayFunStack = TRUE; 
		        	DisplayFunctionStack();
		        	break;
		    }
        }
		PostMessage(hWnd, GF_CLOSE,0, 0L); 

       	break;

    default:
{
#if ENABLETRACE
GSSiExitProg (752);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (752);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SetCommandViewport (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (751);
#endif
{HDC hDC;
 char key;     
 int	st; 

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
		*pCommandViewport = CurView->ID;
		PostMessage(hWnd, GF_CLOSE,0, 0L); 

       	break;

    default:
{
#if ENABLETRACE
GSSiExitProg (751);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (751);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL ShowPolyPoints (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (826);
#endif
{HDC hDC;
 char key;     
 int	st;
 POINT	MousePoint;
 DPOINT	BasePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    {
    	LPTHEME	pTheme;  
    	short	Item;

    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    PickItems (hWnd,BasePoint);
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;
   		SetConfig (PickList[Item].ConfigID);
	    SetViewport (PickList[Item].ViewID);
		pTheme = AddTheme (GF_SAVEPOLY_THEME);
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (Item,FALSE);        		
		ProcessSelectedTheme = 0;
		DeleteTheme (pTheme);
		GetSavedPolys ();
		if (hSavePoly) 
        {
              DLGPROC	lpfnSHOWPOLYMsgProc; 

              lpfnSHOWPOLYMsgProc = MakeProcInstance((DLGPROC)SHOWPOLYMsgProc, hInst);
//              DialogBox(hInst, (LPSTR)"SHOWPOLY", hWnd, lpfnSHOWPOLYMsgProc);
//              FreeProcInstance(lpfnSHOWPOLYMsgProc);
              CreateDialog(hInst, (LPSTR)"SHOWPOLY", hWnd, lpfnSHOWPOLYMsgProc);
        }
	}
		break;
    default:
{
#if ENABLETRACE
GSSiExitProg (826);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (826);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SaveRedefFile (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (840);
#endif
{
	char	Name[128];	
 switch (Message)
   {
   	case GF_INIT:
        if (GetSaveName2 (hWnd,Name,IDS_FILTERRDF,".RDF",IDS_FILERDF))
			SaveDisplayRedefFile (Name);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (840);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (840);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL LoadRedefFile (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (841);
#endif
{
 char	Name[128];	
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
        if (GetFileName3 (hWnd,Name,IDS_FILTERRDF,IDS_FILERDF)) 
			LoadDisplayRedefFile (Name);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (841);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (841);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL EditCmdString (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (975);
#endif
{
 char key;
 short	rtn=1;     
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;  
{
#if ENABLETRACE
GSSiExitProg (975);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {
		LPSTR	pString; 
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, ls;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

        if (Message == GF_EXECUTE)
        	Item = 0;
        else
        {
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		    PickItems (hWnd,BasePoint);
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
		} 
		lGCmdString = 0;
		GSSiGlobFree (&hGCmdString);   
		ProcessPickedItem (Item,FALSE);        		
       	if (lGCmdString && hGCmdString)
       	{   
       		BOOL	SaveDisableHalt;
       		
       		pString = GlobalLock (hGCmdString);
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
			if (GetTextString (hWnd,pString,lGCmdString-1,"Edit Graphics Command String",0,0,0,TRUE,TRUE))
			{
				Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
				if (Fid != HFILE_ERROR)
				{
					GSSillseek (Fid,CurGCmdStringLoc,0);
					BigRead (Fid,(HPSTR)&ID,2);
					BigRead (Fid,(HPSTR)&ls,2); 
					if (ID == 40 && ls == lGCmdString)
						BigWrite (Fid,pString,lGCmdString,-1);
					GSSiClose (Fid);  
					rtn = GF_INCREASE_SUCCESS_COUNT; 
				}
			}
        	DisableHalt = SaveDisableHalt;
			GlobalUnlock (hGCmdString);
       	}
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (975);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (975);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL SetTAGGF (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (834);
#endif
{
	char	Name[128];	
 switch (Message)
   {
   	case GF_INIT:
		SetTAGDialog (hWnd);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (834);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (834);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SetSYMGF (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (835);
#endif
{
	char	Name[128];	
 switch (Message)
   {
  	case GF_INIT:
		SetSYMDialog (hWnd);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (835);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (835);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ChangeDesc (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (853);
#endif
{
	static short	NewDesc;
	static	char	NewSymbolSave[80];
   	char	SymName[34];   
 char key;     
 switch (Message)
   {
    case GF_CLOSE:  
    	SetGlobalValue ("%NEWSYMBOL",""); 
{
#if ENABLETRACE
GSSiExitProg (853);
#endif
    	return FALSE;
}
    	break;
    	 
   	case GF_INIT:  
   		NewDesc = 0;
		if (!SetEditName (FALSE))
			break;
		SetPrompt (PRMT_CHANGEDESC,TRUE); 
       	GetGlobalCVal ("[%NEWSYMBOL]",NewSymbolSave,0);
       	AddLBUTTON = TRUE;
{
#if ENABLETRACE
GSSiExitProg (853);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

	case GF_USEPICKED:
       	GetGlobalCVal ("[%NEWSYMBOL]",NewSymbolSave,0);
    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, rtn;  
    	HFILE	Fid; 
		float	SymSize, SymRot;
		OFSTRUCTGM	OFStruct;
    	long	Ref; 
   		BOOL	SaveDisableHalt= DisableHalt;

        if (Message == GF_EXECUTE || Message == GF_USEPICKED)
        	Item = 0;
        else
        {
		   	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	        BasePoint=ScreenPtToBasePt(MousePoint);
	        PickItems (hWnd,BasePoint);
        	if (!NumPicked)
        		break;
        	Item = NumPicked-1; 
        	NewDesc = 0;
        }
   		if (NewDesc<=0)
   		{
			if (*NewSymbolSave)
				NewDesc = GetDictSymbolNumber(NewSymbolSave);
			else 
			{
				SetPrompt (PRMT_CHANGEDESC2,TRUE); 
	       		switch (PickList[Item].Type)
	       		{
		       		case 1:
		       		case 4:   
		       			_fstrcpy (SymName,"[%NEW_POINT_SYM]"); 
		       			ExpandText (SymName);
						NewDesc = SelectPointSymbol (hWnd,1,SymName,"All",0,0,0,FALSE);
						break;
					case 2: 
					case 5:
		       			_fstrcpy (SymName,"[%NEW_LINE_SYM]"); 
		       			ExpandText (SymName);
					    NewDesc = SelectLineSymbol (hWnd,1,SymName,0,0,FALSE);
		                break;
				                 
					case 3:
		       			_fstrcpy (SymName,"[%NEW_AREA_SYM]"); 
		       			ExpandText (SymName);
					    NewDesc = SelectAreaSymbol (hWnd,1,SymName,0,FALSE);
					    break;  
				}
			}
		}
		
		if (!NewDesc)
			return GF_EXECUTE_CANCELED;
        GetPickName (Item);   
		EditSym (Item,NewDesc);			
					
		DisableHalt = SaveDisableHalt;  
		if (Message == GF_USEPICKED)
		{
	    	SetGlobalValue ("%NEWSYMBOL","");   
			PostMessage(hWnd, GF_CLOSE,0, 0L);
		}  
		else if (Message == GF_EXECUTE) 
			break;

    }
    
    case GF_EXECUTE_FINISHED:  
    {
	}
		break;
    
    default:
{
#if ENABLETRACE
GSSiExitProg (853);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (853);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SplitSegment (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
	short	st, i;
	
 char key;     
 switch (Message)
   {
   	case GF_INIT:
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, CursorPos;
		DPOINT	BasePoint;
    	long	Ref; 
    	long	WantRef=LONG_MAX; 
    	double	dtoend, maxdtoend;

    	if (CursorIsLocked)
	    { 
			BasePoint = CurrentPoint;
	    	UnlockCursor ();
	    }
		else
		{    	
	    	MousePoint=POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint); 
		}
		if (BT_NUM_IN_INDEX (hHighlight) == 1) 
		{   
			HIGHLIGHTDATA	HighlightData;
			
         	BT_FIND (hHighlight,(LPSTR)&WantRef,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
			st = IDYES; 
		}
		{
			ClearHighlightList (FALSE);
			{
				VISLIST	SaveVis = *CurVis;

				SetTypeVisByName ("AREA",0);
				SetTypeVisByName ("TEXT",0);
				PickItems (hWnd,BasePoint);
				*CurVis = SaveVis;
			}
		    if (!NumPicked)
		    	break;  
		    maxdtoend = 0;
		    for (i=0;i<NumPicked;i++)
		    {    
		    	if (PickList[i].Refno == WantRef)
		    	{
		    		Item = i;
		    		break;
		    	}
		    	dtoend = min (PickList[i].Length*PickList[i].PCT,PickList[i].Length*(1-PickList[i].PCT));
		    	if (dtoend >= maxdtoend)
		    		Item = i;
		    }
		    if (WantRef == LONG_MAX)
		    { 
				AddToHighlightList (PickList[Item].Refno,&PickList[Item],TRUE);
			    ShowPickedItem (hWndMain,Item); 
		        GetCursorPos (&CursorPos);  
				if ((st = MessageBox (GetFocus(),"Do you wish to split this segment?","Verify split",MB_YESNO)) == IDYES)
				{
					SetCursorPosGM (CursorPos.x,CursorPos.y,0);
				    RemoveFromHighlightList (PickList[Item].Refno,2);
				    ShowPickedItem (hWndMain,Item); 
				}
			} 
	    }
	    if (st == IDYES)
		{   
			long	Newref1, Newref2;
			
			if (SplitPoly (Item,&Newref1,&Newref2))
			{   
				if (Function == GF_SPLIT_STREET_SEGMENT)
				{   
					BOOL	OpenedSeg;
					short	len;  
					long	Offset; 
					LPGWDHEADER lpGWDHead; 
					
					if (OpenStreetSegmentTable (TRUE,&OpenedSeg))
					{
						time_t	systime;     
				
						time (&systime);
						lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
						if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&PickList[Item].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
						{
						    LPSEGDATAGM	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
						    SEGDATAGM	SaveSegdata;
						    long	lAddDiff;
						    long	rAddDiff;
						    
							FillGWDData (lpGWDHead,Offset);
							SaveSegdata = *pSegdata;
						    lAddDiff = pSegdata->taddl - pSegdata->faddl;
						    rAddDiff = pSegdata->taddr - pSegdata->faddr; 
						    pSegdata->taddl = IDNINT (pSegdata->faddl + lAddDiff * PickList[Item].PCT);
						    if (pSegdata->faddl%2 == SaveSegdata.taddl && SaveSegdata.taddl%2 != pSegdata->taddl%2)
						    	pSegdata->taddl++;
						    pSegdata->taddr = IDNINT (pSegdata->faddr + rAddDiff * PickList[Item].PCT);
						    if (pSegdata->faddr%2 == SaveSegdata.taddr && SaveSegdata.taddr%2 != pSegdata->taddr%2)
						    	pSegdata->taddr++;
					        pSegdata->TLID = Newref1;    
        					pSegdata->UpdateTime = systime;
					        Offset = GSSillseek (lpGWDHead->Fid,0,2); 
					        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&Newref1,(LPSTR)&Offset);  
						  	len = sizeof(SEGDATAGM);
						   	BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
						   	BigWrite (lpGWDHead->Fid,(HPSTR)pSegdata,len,-1); 
					   	    SetGWDCurrentOffset (lpGWDHead,-1);
						    pSegdata->faddl = pSegdata->taddl + lSignof(lAddDiff);
						    pSegdata->faddr = pSegdata->taddr + lSignof(rAddDiff);  
						    pSegdata->taddl = SaveSegdata.taddl;
						    pSegdata->taddr = SaveSegdata.taddr;
        					pSegdata->UpdateTime = systime;
					        pSegdata->TLID = Newref2;
					        Offset = GSSillseek (lpGWDHead->Fid,0,2); 
					        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&Newref2,(LPSTR)&Offset);  
						  	len = sizeof(SEGDATAGM);
						   	BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
						   	BigWrite (lpGWDHead->Fid,(HPSTR)pSegdata,len,-1); 
						   	BT_DELETE (lpGWDHead->BTHandle[0],(LPSTR)&PickList[Item].Refno,(LPSTR)pSegdata,FALSE);
						}
						GlobalUnlock (hDBStreetSegments);
					    CloseStreetSegmentTable(OpenedSeg);
					 }  
				}
				ClearHighlightList (FALSE);
				PickByRefno(Newref1,0,0,-1);
				ShowPickedItem (hWndMain,0); 
				ClearHighlightList (FALSE);
				PickByRefno(Newref2,0,0,-1);
				ShowPickedItem (hWndMain,0);
			}
			else 
			{
			    RemoveFromHighlightList (PickList[Item].Refno,0);
			    ShowPickedItem (hWndMain,Item); 
			} 
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
       	}
       	else
       	{
		    RemoveFromHighlightList (PickList[Item].Refno,2);
		    ShowPickedItem (hWndMain,Item);  
		    RemoveFromHighlightList (PickList[Item].Refno,0);
		    ShowPickedItem (hWndMain,Item); 
		}
		ClearHighlightList (FALSE);
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL SplitPolygon (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
	short	st;
	DWORD	i,j,n; 
	HPDPOINT	pPoint;
	static	short	Step; 
	static	long	npnts;
	static	HANDLE	hPoly=0;
	static	PICKDATA	SavePickList;
	static	HANDLE	hSavedScreen=0;
	static	long	BitmapID;  
	static	BOOL	PolyIsClosed;
	static	BOOL	Insert;
	
 char key;     
 switch (Message)
   {
   	case GF_INIT:
    	GSSiGlobFree (&hPoly); 
   		if (!CurView->UpdateFile)
   		{
	 		GSSiMsgBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK,0);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	    Step = 1;
       	AddLBUTTON = TRUE; 
   		SetPrompt (PRMT_CHANGE_POLY1,TRUE);  
   		break;
    
    case GF_CLOSE:  
    	AllowSinglePoint = FALSE;
    	DestroySavedScreen (&hSavedScreen,0);  
    	GSSiGlobFree (&hPoly); 
    	ShowNodesRef = LONG_MAX;
		return FALSE;
   	case GF_REDRAW: 
   	case GF_REDRAW_CMD: 
   		DestroySavedScreen (&hSavedScreen,0);
   		break;
    case WM_CHAR:
		key = wParam;
		if (Step != 2)
			return FALSE;; 
		switch (key)
		{   
			case 65://A
			case 97://a
				NumSelectedNodes = 2;
				SelectedNodes[0] = 0;   
				SelectedNodes[1] = npnts;
				ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);
				goto RBut; 
			break;	
			case 73://I
			case 105: //i 
				Insert = TRUE;
				goto RBut;  
			break;
			case 82://R
			case 114://r
				Insert = FALSE; 
				goto RBut;
			break; 
			default:
				return FALSE;
		}
		break;
    case GF_COMPLETE:   
    	AllowSinglePoint = FALSE;
    	if (NumNewPolyPoints)
    	{    
    		HPDPOINT	Points, NewPoints, OldPoints;
    		
		    ClearHighlightList (FALSE); 
       		GSSiGlobUlFree (&hUpdateMultiPolygon); 
		    nUpdatePolyPoints = NumNewPolyPoints + npnts; 
       		hUpdatePoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,(nUpdatePolyPoints+1)*sizeof(DPOINT));  
       		Points = (HPDPOINT)GlobalLock (hUpdatePoly);
        	NewPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
        	OldPoints = (HPDPOINT)GlobalLock (hPoly);
        	i = n = 0; 
        	if (SelectedNodes[0] > SelectedNodes[1])
        	{ 
    			for (j=0;j<NumNewPolyPoints;j++)
    				Points[n++] = NewPoints[j]; 
    			for (j=SelectedNodes[1];j<SelectedNodes[0];j++)
    				Points[n++] = OldPoints[j]; 
        	}
        	else
        	while (i<npnts)
        	{
        		if (i == SelectedNodes[0]) 
        		{
        			if (Insert)
        				Points[n++] = OldPoints[i++];
        			for (j=0;j<NumNewPolyPoints;j++)
        				Points[n++] = NewPoints[j]; 
        			i = SelectedNodes[1] + 1;
        		}
        		else
       				Points[n++] = OldPoints[i++];
        	} 
        	nUpdatePolyPoints = n;
        	if (PolyIsClosed)
        		Points[nUpdatePolyPoints++] = Points[0];
        	GlobalUnlock (hUpdatePoly);
        	NumNewPolyPoints=0;
        	GSSiGlobUlFree (&hNewPolyPoints);
        	GSSiGlobUlFree (&hPoly); 
			PickList[0] = SavePickList;
        	if (PickList[0].Type == 2)
				UpdateItem=61;   
        	else
				UpdateItem=51;   
			UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,1,-1);
			GSSiGlobFree (&hUpdatePoly);  
        }  
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        break; 
        
    
    case WM_RBUTTONUP:
  RBut:
    	switch (Step)
    	{
    		case 1:
				if (hSavedScreen)
					RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
    			if (npnts && hPoly) 
    			{ 
					ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);
    				Step++;   
			   		SetPrompt (PRMT_CHANGE_POLY3,TRUE);  
    			} 
    			
    			break;
    		case 2:
				CreateDigCursor (CurView->hDC);
				SetCurs ((HCURSOR)2,FALSE); 
				AllowSinglePoint = TRUE;  
				AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,PRMT_CHANGE_POLY5); 
				break;
		}
		break;
		 
	case GF_USEPICKED:
		NumPicked = 1;
	    goto UsePicked;
    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, CursorPos;
		DPOINT	BasePoint;
    	long	Ref;
    	short	nLoops; 

    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
		switch (Step)
		{
			case 1:
			{
				short	SaveMaxPick = MaxPick; 
                
                Insert = FALSE;
				GSSiGlobFree (&hPoly);
				ClearHighlightList (FALSE);  
				if (hSavedScreen)
					RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
				else
			        hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&BitmapID);
				MaxPick = 5;
			    PickItems (hWnd,BasePoint);  
			    MaxPick = SaveMaxPick;
UsePicked:
			    while (NumPicked--)
			    {
			    	if (PickList[NumPicked].Type == 2 || PickList[NumPicked].Type == 3)
			    		break;
			    }
			    if (NumPicked < 0)
			    	break; 
	       		SetPrompt (PRMT_CHANGE_POLY2,TRUE);  
			    SavePickList = PickList[NumPicked];   
			    NumSelectedNodes = 0;
				AddToHighlightList (PickList[NumPicked].Refno,&PickList[NumPicked],TRUE);     
				ShowNodesRef = PickList[NumPicked].Refno;
			    ShowPickedItem (hWnd,NumPicked);  
				WantUnsplinedPoints = TRUE;  
				nLoops = GetPolyPoints2 ((LPPICKDATAHEADER)&PickList[NumPicked],0,0,0); 
				if (nLoops == 1)
					st = GetPolyPoints ((LPPICKDATAHEADER)&PickList[NumPicked],FALSE,&npnts,&hPoly);
				else
				{
					st = 0; 
					GSSiMsgBox (CurView->hWnd,"This polygon contains multiple loops",0,MB_ICONEXCLAMATION,0);
				}
				WantUnsplinedPoints = FALSE;  
				
				ClearHighlightList (FALSE);  
				if (!st)
				{
					PostMessage(hWnd, GF_CLOSE,0, 0L); 
					break;        
				}
				else
				{
					HPDPOINT	pPoints=(HPDPOINT)GlobalLock (hPoly);
					
					if (nPnts > 1 && SameDPoint (&pPoints[0],&pPoints[nPnts-1]))  
					{
						npnts--;
						PolyIsClosed = TRUE;
					}
					else
						PolyIsClosed = FALSE; 
					GlobalUnlock (hPoly);
				}
			} 
			    break; 
			    
			case 2: 
			{
				double MinDist = DBL_MAX, d; 
				DWORD	Mini;
				 
				pPoint =(HPDPOINT) GlobalLock (hPoly);
				for (i=0;i<npnts;i++)
				{
					d = ldistp (pPoint[i],BasePoint);
					if (d < MinDist)
					{
						MinDist = d;
						Mini = i;
					}
				}
				GlobalUnlock (hPoly);
				switch (NumSelectedNodes)
				{
					case 0:
						SelectedNodes[NumSelectedNodes++] = Mini;
				   		SetPrompt (PRMT_CHANGE_POLY4,TRUE);  
						break;
					case 1:
/*						if (Mini < SelectedNodes[0])    
						{   
							long	SaveNode = SelectedNodes[0];
							
							SelectedNodes[0] = Mini;
							SelectedNodes[NumSelectedNodes++] = SaveNode;
						}	
						else if (Mini > SelectedNodes[NumSelectedNodes])   */
							SelectedNodes[NumSelectedNodes++] = Mini; 
						ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);  
						goto RBut;
						break;
					case 2:
						SelectedNodes[NumSelectedNodes-1] = Mini; 
						ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);  
						Step++; 
						goto RBut;
				} 
				ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);  


			}
			break;
		}
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL ReplacePolyPoints (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
	short	st;
	DWORD	i,j,n; 
	HPDPOINT	pPoint;
	static	short	Step; 
	static	long	npnts;
	static	HANDLE	hPoly=0;
	static	PICKDATA	SavePickList;
	static	HANDLE	hSavedScreen=0;
	static	long	BitmapID;  
	static	BOOL	PolyIsClosed;
	static	BOOL	Insert;
	
 char key;     
 switch (Message)
   {
   	case GF_INIT:
    	GSSiGlobFree (&hPoly); 
   		if (!CurView->UpdateFile)
   		{
	 		GSSiMsgBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK,0);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	    Step = 1;
       	AddLBUTTON = TRUE; 
   		SetPrompt (PRMT_CHANGE_POLY1,TRUE);  
   		break;
    
    case GF_CLOSE:  
    	AllowSinglePoint = FALSE;
    	DestroySavedScreen (&hSavedScreen,0);  
    	GSSiGlobFree (&hPoly); 
    	ShowNodesRef = LONG_MAX;
		return FALSE;
   	case GF_REDRAW: 
   	case GF_REDRAW_CMD: 
   		DestroySavedScreen (&hSavedScreen,0);
   		break;
    case WM_CHAR:
		key = wParam;
		if (Step != 2)
			return FALSE;; 
		switch (key)
		{   
			case 65://A
			case 97://a
				NumSelectedNodes = 2;
				SelectedNodes[0] = 0;   
				SelectedNodes[1] = npnts;
				ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);
				goto RBut; 
			break;	
			case 73://I
			case 105: //i 
				Insert = TRUE;
				goto RBut;  
			break;
			case 82://R
			case 114://r
				Insert = FALSE; 
				goto RBut;
			break; 
			default:
				return FALSE;
		}
		break;
    case GF_COMPLETE:   
    	AllowSinglePoint = FALSE;
    	if (NumNewPolyPoints)
    	{    
    		HPDPOINT	Points, NewPoints, OldPoints;
    		
		    ClearHighlightList (FALSE); 
       		GSSiGlobUlFree (&hUpdateMultiPolygon); 
		    nUpdatePolyPoints = NumNewPolyPoints + npnts; 
       		hUpdatePoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,(nUpdatePolyPoints+1)*sizeof(DPOINT));  
       		Points = (HPDPOINT)GlobalLock (hUpdatePoly);
        	NewPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
        	OldPoints = (HPDPOINT)GlobalLock (hPoly);
        	i = n = 0; 
        	if (SelectedNodes[0] > SelectedNodes[1])
        	{ 
    			for (j=0;j<NumNewPolyPoints;j++)
    				Points[n++] = NewPoints[j]; 
    			for (j=SelectedNodes[1];j<SelectedNodes[0];j++)
    				Points[n++] = OldPoints[j]; 
        	}
        	else
        	while (i<npnts)
        	{
        		if (i == SelectedNodes[0]) 
        		{
        			if (Insert)
        				Points[n++] = OldPoints[i++];
        			for (j=0;j<NumNewPolyPoints;j++)
        				Points[n++] = NewPoints[j]; 
        			i = SelectedNodes[1] + 1;
        		}
        		else
       				Points[n++] = OldPoints[i++];
        	} 
        	nUpdatePolyPoints = n;
        	if (PolyIsClosed)
        		Points[nUpdatePolyPoints++] = Points[0];
        	GlobalUnlock (hUpdatePoly);
        	NumNewPolyPoints=0;
        	GSSiGlobUlFree (&hNewPolyPoints);
        	GSSiGlobUlFree (&hPoly); 
			PickList[0] = SavePickList;
        	if (PickList[0].Type == 2)
				UpdateItem=61;   
        	else
				UpdateItem=51;   
			UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,1,-1);
			GSSiGlobFree (&hUpdatePoly);  
        }  
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        break; 
        
    
    case WM_RBUTTONUP:
  RBut:
    	switch (Step)
    	{
    		case 1:
				if (hSavedScreen)
					RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
    			if (npnts && hPoly) 
    			{ 
					ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);
    				Step++;   
			   		SetPrompt (PRMT_CHANGE_POLY3,TRUE);  
    			} 
    			
    			break;
    		case 2:
				CreateDigCursor (CurView->hDC);
				SetCurs ((HCURSOR)2,FALSE); 
				AllowSinglePoint = TRUE;  
				AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,PRMT_CHANGE_POLY5); 
				break;
		}
		break;
		 
	case GF_USEPICKED:
		NumPicked = 1;
	    goto UsePicked;
    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, CursorPos;
		DPOINT	BasePoint;
    	long	Ref;
    	short	nLoops; 

    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
		switch (Step)
		{
			case 1:
			{
				short	SaveMaxPick = MaxPick; 
                
                Insert = FALSE;
				GSSiGlobFree (&hPoly);
				ClearHighlightList (FALSE);  
				if (hSavedScreen)
					RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
				else
			        hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&BitmapID);
				MaxPick = 5;
			    PickItems (hWnd,BasePoint);  
			    MaxPick = SaveMaxPick;
UsePicked:
			    while (NumPicked--)
			    {
			    	if (PickList[NumPicked].Type == 2 || PickList[NumPicked].Type == 3)
			    		break;
			    }
			    if (NumPicked < 0)
			    	break; 
	       		SetPrompt (PRMT_CHANGE_POLY2,TRUE);  
			    SavePickList = PickList[NumPicked];   
			    NumSelectedNodes = 0;
				AddToHighlightList (PickList[NumPicked].Refno,&PickList[NumPicked],TRUE);     
				ShowNodesRef = PickList[NumPicked].Refno;
			    ShowPickedItem (hWnd,NumPicked);  
				WantUnsplinedPoints = TRUE;  
				nLoops = GetPolyPoints2 ((LPPICKDATAHEADER)&PickList[NumPicked],0,0,0); 
				if (nLoops == 1)
					st = GetPolyPoints ((LPPICKDATAHEADER)&PickList[NumPicked],FALSE,&npnts,&hPoly);
				else
				{
					st = 0; 
					GSSiMsgBox (CurView->hWnd,"This polygon contains multiple loops",0,MB_ICONEXCLAMATION,0);
				}
				WantUnsplinedPoints = FALSE;  
				
				ClearHighlightList (FALSE);  
				if (!st)
				{
					PostMessage(hWnd, GF_CLOSE,0, 0L); 
					break;        
				}
				else
				{
					HPDPOINT	pPoints=(HPDPOINT)GlobalLock (hPoly);
					
					if (nPnts > 1 && SameDPoint (&pPoints[0],&pPoints[nPnts-1]))  
					{
						npnts--;
						PolyIsClosed = TRUE;
					}
					else
						PolyIsClosed = FALSE; 
					GlobalUnlock (hPoly);
				}
			} 
			    break; 
			    
			case 2: 
			{
				double MinDist = DBL_MAX, d; 
				DWORD	Mini;
				 
				pPoint =(HPDPOINT) GlobalLock (hPoly);
				for (i=0;i<npnts;i++)
				{
					d = ldistp (pPoint[i],BasePoint);
					if (d < MinDist)
					{
						MinDist = d;
						Mini = i;
					}
				}
				GlobalUnlock (hPoly);
				switch (NumSelectedNodes)
				{
					case 0:
						SelectedNodes[NumSelectedNodes++] = Mini;
				   		SetPrompt (PRMT_CHANGE_POLY4,TRUE);  
						break;
					case 1:
/*						if (Mini < SelectedNodes[0])    
						{   
							long	SaveNode = SelectedNodes[0];
							
							SelectedNodes[0] = Mini;
							SelectedNodes[NumSelectedNodes++] = SaveNode;
						}	
						else if (Mini > SelectedNodes[NumSelectedNodes])   */
							SelectedNodes[NumSelectedNodes++] = Mini; 
						ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);  
						goto RBut;
						break;
					case 2:
						SelectedNodes[NumSelectedNodes-1] = Mini; 
						ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);  
						Step++; 
						goto RBut;
				} 
				ShowNodePoints (CurView->hDC,0,0,hPoly,npnts,PolyIsClosed);  


			}
			break;
		}
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL DeleteItems (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short function)
#if ENABLETRACE
{GSSiEnterProg (851);
#endif
{
 switch (Message)
   {
   	case GF_INIT:  
   		DeleteHighlightedItems (function);
        PostMessage(hWnd, WM_COMMAND, GF_CANCEL, 0L);
   		break;

	case WM_COMMAND:
		if (wParam != GF_CANCEL)
{
#if ENABLETRACE
GSSiExitProg (851);
#endif
			return FALSE;
}
   		PostMessage(hWnd, GF_CLOSE,0, 0L);
		break;
    case WM_LBUTTONUP:
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (851);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (851);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 
BOOL DeleteStreetNet (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short function)
{
	char	mess[128], TrueName[66];
    BOOL		Opened;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData;
	NETREFSKEY	NetRefsKey;
	NETREFSDATA	NetRefsData; 
	NETMARKERSKEY1	NetMarkersKey1;
	NETMARKERSDATA1 NetMarkersData1;
	NETMARKERSKEY2	NetMarkersKey2;
	NETMARKERSDATA2 NetMarkersData2;
	NETINTMPKEY NetIntMPKey; 
	long	IntID;
	short	st,LinkID, CurMarkerID;
	long	NumSegs=0,NumMarkers=0;

 switch (Message)
   {
   	case GF_INIT:
   		if (!CurPath)
   		{
   			MessageBox (hWnd,"No current street name",0,MB_ICONEXCLAMATION);  
   		}
   		else
   		{
	    	GetTrueStreetName (CurPath,TrueName,0,0);
		    sprintf (mess,"Delete network for %s?",TrueName);
			if (MessageBox (hWnd,mess,"Verify Delete",MB_YESNO) == IDYES) 
			{
			 	OpenNetLinkAndRef (NetworkID,TRUE,&Opened);
		Next:  
				NetRefsKey.Path =  CurPath; 
				NetRefsKey.MP = CurLinkID * 1000000; 
				st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData); 
				LinkID = NetRefsKey.MP/1000000;
				if (!st && NetRefsKey.Path == CurPath && (LinkID == CurLinkID || CurLinkID<0))
				{
	    			BT_DELETE (hBTNetRefs,(LPSTR)&NetRefsKey,(LPSTR)&NetRefsData,FALSE); 
	    			NumSegs++;
				 	NetLinksKey.Ref = NetRefsData.Ref;
				 	NetLinksKey.NetID = NetworkID;
				 	NetLinksKey.Path = CurPath;
				 	if (!BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData))
				 		BT_DELETE(hBTNetLinks,(LPSTR)&NetLinksKey,(LPSTR)&NetLinksData,FALSE); 
				 	goto Next;
				}
			 	CurMarkerID=-1;
			 	CloseNetLinkAndRef (Opened);
				OpenNetMarkers (NetworkID,FALSE,&Opened);  
		NextMarkerID:
			 	NetMarkersKey2.MarkerID = CurMarkerID;
			 	NetMarkersKey2.Path = 1000000000;
			 	NetMarkersKey2.MP = 0;
				if (!BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData2))
				{
					CurMarkerID = NetMarkersKey2.MarkerID; 
		NextMarker:
				 	NetMarkersKey2.MarkerID = CurMarkerID;
				 	NetMarkersKey2.Path = CurPath;
				 	NetMarkersKey2.MP = CurLinkID * 1000000;
					st = BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData2);
					LinkID = NetMarkersKey2.MP/1000000;
					if (!st && NetMarkersKey2.Path == CurPath && (LinkID == CurLinkID || CurLinkID<0))
					{
						BT_DELETE (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,(LPSTR)&NetMarkersData2,FALSE);
						NetMarkersKey1.MarkerID = CurMarkerID;
						NetMarkersKey1.Path = CurPath;
						_fmemmove (NetMarkersKey1.Prefix,NetMarkersData2.Prefix,2);
						_fmemmove (NetMarkersKey1.Suffix,NetMarkersData2.Suffix,2);
						NetMarkersKey1.Value = NetMarkersData2.Value;
						if (!BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey1,BT_FIRST,BT_EQ,(LPSTR)&NetMarkersData2))
							BT_DELETE (hBTNetMarkers1,(LPSTR)&NetMarkersKey1,(LPSTR)&NetMarkersData1,FALSE);
						NumMarkers++;
						goto NextMarker;
					}
					goto NextMarkerID;
				}
				CloseNetMarkers (Opened);
				OpenNetIntersect (NetworkID,FALSE,&Opened); 
		NextInt: 
				NetIntMPKey.Path = CurPath;
				NetIntMPKey.MP = CurLinkID * 1000000;
				st = BT_FIND (hBTNetIntMP,(LPSTR)&NetIntMPKey,BT_FIRST,BT_GE,(LPSTR)&IntID);
				LinkID = NetIntMPKey.MP/1000000; 
				if (!st && NetIntMPKey.Path == CurPath && (LinkID == CurLinkID || CurLinkID<0))
				{
					BT_DELETE (hBTNetIntMP,(LPSTR)&NetIntMPKey,(LPSTR)&IntID,FALSE);
					goto NextInt;
				}
				CloseNetIntersect (Opened);
					
 				ClearSpecial(); 
				sprintf (mess,"%ld segments and %ld markers deleted",NumSegs,NumMarkers);
				MessageBox (hWnd,mess,"Delete Completed",MB_OK);
			}
		} 
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
       	break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL SelectStreetLink (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
 HDC hDC;
 char key;     
 int	st, i; 
 BOOL	HaveDown=FALSE; 
 POINT	MousePoint;
 DPOINT	BasePoint;
 NETLINKSKEY	NetLinksKey; 
 NETLINKSDATA	NetLinksData; 
 static	long	SelectedPaths[8], SelectedLinks[8];
 

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break; 
    	
    case WM_COMMAND:
    	if (wParam >= 32000)
    	{
    		i = wParam - 32000; 
    		if (i<8)
    		{   
    			CurPath = SelectedPaths[i];
    			CurLinkID = SelectedLinks[i];
				DrawStreet (CurPath,CurLinkID,RGB(255,0,255),3,TRUE,0,0,0);
    		}
           	PostMessage(hWnd, GF_CLOSE,0, 0L);
           	return TRUE;
    	}
    	return FALSE;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown) break;
    	HaveDown = FALSE;
	    MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE);
	    PickItems (hWnd,BasePoint);
        if (NumPicked--)
        {   
        	BOOL	Opened;
        	short	st;
			HMENU	PathMenu; 
			WORD	MenItem=32000; 
			POINT	position;
			char	TrueName[66];
	
			PathMenu = CreatePopupMenu();          
            i=0;
			OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
	 	 	NetLinksKey.Ref = PickList[NumPicked].Refno;
	 	 	NetLinksKey.NetID = 0;
	 	 	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
	 	 	while (!st && NetLinksKey.Ref == PickList[NumPicked].Refno)
	 	 	{   
	 	 		if (i<8)
	 	 		{	
		    		GetTrueStreetName (NetLinksKey.Path,TrueName,0,0);
		    		SelectedPaths[i] = NetLinksKey.Path;
		    		SelectedLinks[i++]= NetLinksData.MP/1000000;
					AppendMenu (PathMenu,MF_ENABLED,MenItem++,TrueName);
			 	 	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
		 	 	} 
		 	 	else
		 	 		st = 1;
	    	} 
			if (i) 
			{
			   	GetCursorPos (&position);
				TrackPopupMenu (PathMenu,TPM_RIGHTBUTTON,position.x,position.y,0,hWnd,0);
			}
			DestroyMenu (PathMenu); 
		 	CloseNetLinkAndRef (Opened);
	    	
    	} 
    	setDoPaint( TRUE);
       	break;

    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL SnapTo (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{HDC hDC;
 char key, str[128];     
 BOOL	st;
 HIGHLIGHTDATA	HighlightData;
 POINTS	MousePoint;
 POINT	CursorPoint;
 DPOINT	BasePoint, NodePoint, RP; 
 short	SavePP;
 DPOINT	SnapPoint;
 double	AZ;

 switch (Message)
   {
    case GF_DISPLAYMESS:
    {
    	UINT	Prompt;
    	switch (Function)
    	{
		    case GF_SNAP_ON:
		    	Prompt = PRMT_SNAP_ON;
		    	break;
		    case GF_SNAP_END: 
		    	Prompt = PRMT_SNAP_END;
		    	break;
		    case GF_SNAP_NODE_POINT: 
		    	Prompt = PRMT_SNAP_NODE_POINT;
		    	break;
		    case GF_SNAP_END_POINT: 
		    	Prompt = PRMT_SNAP_END_POINT;
		    	break; 
		    case GF_SNAP_CHORD_MID_POINT:
		    case GF_SNAP_MID_POINT: 
			case GF_SNAP_RADIUS_POINT:
		    	Prompt = PRMT_SNAP_ON;
		    	break; 
		    case GF_SNAP_AVE_POINT:
		    	Prompt = PRMT_SNAP_ON;
		    	break;
		    case GF_SNAP_BEGIN_POINT:    
		    	Prompt = PRMT_SNAP_BEGIN_POINT;
		    	break;
		    case GF_SET_POC:
		    case GF_SET_COORD:
		    	Prompt = GetFunStackPrompt(0);
		    	break;
		    default:
		    	Prompt = 0;
		}
        SetPrompt (Prompt,TRUE);   
   }
        break;
    case GF_CLOSE:
    	return FALSE;
   	case GF_INIT:
		if (Function == GF_UNDO) 
		{   
			PostMessage(hWnd, GF_CLOSE,0,0L); 
           	return (TRUE);
		}
		if (Function == GF_LBUTTON || Function == GF_RBUTTON) 
		{   
			GetCursorPos (&RLButLoc);
			ScreenToClient (hWnd,&RLButLoc);
			PostMessage(hWnd, GF_CLOSE,Function,0L); 
           	return (TRUE);
		}
/*		if (Function == GF_SET_COORD && CursorIsLocked) 
		{   
			UnlockCursor ();
			SetDigWorldControlPoint (CurrentPoint);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
           	return (TRUE);
		} */
		if (Function == GF_SET_POC && CursorIsLocked) 
		{   
			UnlockCursor ();
			SetDigWorldControlPoint (CurrentPoint);
			PostMessage(hWnd, GF_CLOSE,GF_SET_POC, 0L); 
           	return (TRUE);
		}
		if (Function == GF_SNAP_UNLOCK) 
		{
			UnlockCursor ();
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
			HaveCurrentLBUTTON=FALSE;
           	return (TRUE);
		}
		if (Function == GF_SNAP_DIST_AND_DIR) 
		{   
			DLGPROC lpfnDISTANDDIRMsgProc;
			HCURSOR	SaveCursor=GSSiSetCursor (LoadCursor(0, IDC_ARROW));     

			
			lpfnDISTANDDIRMsgProc = MakeProcInstance((DLGPROC)DISTANDDIRMsgProc, hInst);
			st = DialogBox(hInst, (LPSTR)"DISTANDDIR", hWnd, lpfnDISTANDDIRMsgProc);
			FreeProcInstance(lpfnDISTANDDIRMsgProc);
			GSSiSetCursor (SaveCursor);
			if (st)
			{
				LockCursor (&PandDPoint);
				goto LockPoint; 
			}
			else
			{
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
				HaveCurrentLBUTTON=FALSE;
	           	return (TRUE);
	        }
		}
		if (Function == GF_SNAP_COORD) 
		{   
           	DLGPROC lpfnLOC_COORDMsgProc;
			HCURSOR	SaveCursor=GSSiSetCursor (LoadCursor(0, IDC_ARROW));     
            
			if (GetGlobalCVal ("[%NEXTCVAL]",str,0))
			{
				LPSTR pComma = MatchLev (str,',');
				
				if (pComma)
				{
					*pComma++ = 0;  
					SetGlobalValue ("%NEXTCVAL",pComma); 
				}
				else 
					SetGlobalValue ("%NEXTCVAL",""); 
				UserSpecifiedBasePoint = atopt (str,&st);
				st = !st;
			}
			else
			{
	            InSnap = TRUE;
	            lpfnLOC_COORDMsgProc = MakeProcInstance((DLGPROC)LOC_COORDMsgProc, hInst);
	            st = DialogBox(hInst, (LPSTR)"LOC_COORD", hWnd, lpfnLOC_COORDMsgProc);
	            FreeProcInstance(lpfnLOC_COORDMsgProc); 
				InSnap = FALSE; 
			}
			GSSiSetCursor (SaveCursor);
			if (st)
			{
				LockCursor (&UserSpecifiedBasePoint);
				goto LockPoint; 
			}
			else
			{
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
				HaveCurrentLBUTTON=FALSE;
	           	return (TRUE);
	        }
		}
		if (Function == GF_SNAP_LATLONG) 
		{   
           	DLGPROC lpfnLOC_LATLONGMsgProc;
			HCURSOR	SaveCursor=GSSiSetCursor (LoadCursor(0, IDC_ARROW));     
            
			//GetCursorPos (&MousePoint);
			//ScreenToClient (hWnd,&MousePoint);
		    //UserSpecifiedBasePoint=WinPtToBasePt(MousePoint);
            InSnap = TRUE;
            lpfnLOC_LATLONGMsgProc = MakeProcInstance((DLGPROC)LOC_LATLONGMsgProc, hInst);
            st = DialogBox(hInst, (LPSTR)"LOC_LATLONG", hWnd, lpfnLOC_LATLONGMsgProc);
            FreeProcInstance(lpfnLOC_LATLONGMsgProc); 
			GSSiSetCursor (SaveCursor);
			InSnap = FALSE;
			if (st)
			{
				LockCursor (&UserSpecifiedBasePoint);
				goto LockPoint; 
			}
			else
			{
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
				HaveCurrentLBUTTON=FALSE;
	           	return (TRUE);
	        }
		}
       	AddLBUTTON = TRUE;
		if (Function == GF_SET_COORD) 
		{   
			if (!CursorIsLocked)
				AddLBUTTON = FALSE; 
		}
		SetPickCursor (TRUE);
   		break;

    case GF_COMPLETE:
/*		if (Function == GF_SET_COORD && CursorIsLocked) 
		{
			UnlockCursor ();
			SetDigWorldControlPoint (CurrentPoint);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
           	return (TRUE);
		} */ 
		if (Function == GF_SET_POC && CursorIsLocked) 
		{
			UnlockCursor ();
			SetDigWorldControlPoint (CurrentPoint);
			PostMessage(hWnd, GF_CLOSE,GF_SET_POC, 0L); 
           	return (TRUE);
		} 
		if (CursorIsLocked) 
		{
			UnlockCursor (); 
			BasePoint = CurrentPoint;
			goto HavePoint;
		}
		if (wParam == GF_LBUTTON)
		{
			MousePoint = POINTtoPOINTS(RLButLoc);
			goto HaveMousePoint; 
		}
		 
		break;
		
    case WM_LBUTTONUP:
    	MousePoint = MAKEPOINTS (lParam);     
HaveMousePoint:
	    BasePoint=WinPtSToBasePt(MousePoint);
HavePoint:
		if (Function == GF_SET_COORD) 
		{   
			if (CursorIsLocked) 
				UnlockCursor ();
			else
				CurrentPoint = BasePoint;
			SetDigWorldControlPoint (CurrentPoint);
			SetPickCursor (FALSE);
		    EnlargeScreen (0,0);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
           	return (TRUE);
		}
		if (Function == GF_SNAP_CURSOR) 
		{   
			LockCursor (&BasePoint);
			goto LockPoint; 
		}  
    	if (Function >= GF_SNAP_COLOR && Function <= GF_SNAP_BLACK_MID)
    	{   
			POINT	point;

    		if (!PickColor (BasePtToScreenPt (&BasePoint),GetGlobalLVal2("[%PICKCOLOR]",0),&point,FALSE,Function-GF_SNAP_COLOR))
    			return FALSE; 
    		SnapPoint = ScreenPtToBasePt(point);
			LockCursor (&SnapPoint);
			goto LockPoint; 
		}
		if (Function == GF_SET_POC) 
		{   
			CurrentPoint = BasePoint;
			SetDigWorldControlPoint (CurrentPoint);
			SetPickCursor (FALSE);
			PostMessage(hWnd, GF_CLOSE,GF_SET_POC, 0L); 
           	return (TRUE);
		}
		if (Function == GF_SNAP_ON || Function == GF_SNAP_NODE_POINT)
		{
			SavePP = PickPerim;
			PickPerim = 1;  
		} 
		{
			BOOL	SavePTXT = PickText;  
            
            PickText = FALSE;
            if (Function == GF_SNAP_NODE_POINT)
            	InSnapNode = TRUE;
	    	PickItems (hWnd,BasePoint);
	    	InSnapNode = FALSE;
	    	PickText = SavePTXT;
	    }
		if (Function == GF_SNAP_ON || Function == GF_SNAP_NODE_POINT)
			PickPerim = SavePP;  
	    if (!NumPicked)
	    	return FALSE;
    	NumPicked--; 
    	SetPickGlobals (NumPicked);
       	AddToSnappedList (NumPicked);
    	switch (Function)
    	{   
    		
    		case GF_SNAP_ON:  
    			LockCursor (&PickList[NumPicked].PickedPoint);
		    	SetCurAZ (PickList[NumPicked].PPAZ);
    			break;
    		case GF_SNAP_BEGIN_POINT:
    			LockCursor (&PickList[NumPicked].BeginPoint);
		    	SetCurAZ (PickList[NumPicked].BPAZ);
    			break;
    		case GF_SNAP_END_POINT:
    			LockCursor (&PickList[NumPicked].EndPoint);
		    	SetCurAZ (PickList[NumPicked].EPAZ);
    			break;
    		case GF_SNAP_RADIUS_POINT:
     			if (SysTypeFromPickType (PickList[NumPicked].Type) == GF_CURVE)    
    			{
					double	CLEN;

 					st = RCURVE(&PickList[NumPicked].BeginPoint.x,&PickList[NumPicked].BeginPoint.y,
								&PickList[NumPicked].NodePoint.x,&PickList[NumPicked].NodePoint.y,
								&PickList[NumPicked].EndPoint.x,&PickList[NumPicked].EndPoint.y,&RP.x,&RP.y,&CLEN); 
  					LockCursor (&RP);
				}
    			break;
    		case GF_SNAP_NODE_POINT: 
    			if (SysTypeFromPickType (PickList[NumPicked].Type) == GF_CURVE)    
    			{
					if (PickList[NumPicked].PCT > 0.33) 
					{
						NodePoint = PickList[NumPicked].EndPoint;
						AZ = PickList[NumPicked].EPAZ;
                    }
					else if (PickList[NumPicked].PCT > 0.67)
					{
						NodePoint = PickList[NumPicked].BeginPoint; 
						AZ = PickList[NumPicked].BPAZ;
                    }
					else
					{
						NodePoint = PickList[NumPicked].NodePoint; 
						AZ = LTWOPI ((PickList[NumPicked].BPAZ + PickList[NumPicked].BPAZ) / 2);
                    }
    			}
    			else
    			{
    				NodePoint = PickList[NumPicked].NodePoint;
					AZ = PickList[NumPicked].PPAZ;
    			}
    			LockCursor (&NodePoint);
		    	SetCurAZ (AZ);
    			break;
    		case GF_SNAP_CHORD_MID_POINT:
    			SnapPoint = MidPointD (PickList[NumPicked].FromPoint,PickList[NumPicked].ToPoint);
    			LockCursor (&SnapPoint);   
    			AZ = getazd (&PickList[NumPicked].FromPoint,&PickList[NumPicked].ToPoint);
		    	SetCurAZ (AZ);
    			break;
    		case GF_SNAP_MID_POINT:
    			SnapPoint = MidPointD (PickList[NumPicked].BeginPoint,PickList[NumPicked].EndPoint);
    			LockCursor (&SnapPoint);
    			AZ = getazd (&PickList[NumPicked].BeginPoint,&PickList[NumPicked].EndPoint);
		    	SetCurAZ (AZ);
    			break;
    		case GF_SNAP_AVE_POINT:
    		{
    			long	nPoints = 0;
    			long	npnts, i, Refno;
    			HANDLE	hPoints; 
    			short	pos=BT_FIRST; 
    			
//				if (!BT_NUM_IN_INDEX (hHighlight))
//					break;  
				SnapPoint.x = SnapPoint.y = 0;
				nPoints = 0;
//			   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
			   	{
			   		pos=BT_NEXT;
//			   		PickList[0]=HighlightData.PD;
					if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[NumPicked],FALSE,&npnts,&hPoints))
					{   
						HPDPOINT	Points=(HPDPOINT)GlobalLock (hPoints);

						if (npnts > 1 && SameDPoint (&Points[0],&Points[npnts-1]))
							npnts--;
						for (i=0;i<npnts;i++)
						{
							SnapPoint.x += Points[i].x;
							SnapPoint.y += Points[i].y;
						}
						nPoints += npnts;  
						GSSiGlobUlFree (&hPoints);
					}
		        }  
		        SnapPoint.x /= nPoints;
		        SnapPoint.y /= nPoints;
    			LockCursor (&SnapPoint); 
    		}
    			break;
		        
    		case GF_SNAP_AVE_HLTPOINT:
    		{
    			long	nPoints = 0;
    			long	npnts, i, Refno;
    			HANDLE	hPoints; 
    			short	pos=BT_FIRST; 
    			
				if (!BT_NUM_IN_INDEX (hHighlight))
					break;  
				SnapPoint.x = SnapPoint.y = 0;
				nPoints = 0;
			   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
			   	{
			   		pos=BT_NEXT;
			   		PickList[0]=HighlightData.PD;
			   		if (PickList[0].Type == 1 || PickList[0].Type == 4)
			   		{
			   			SnapPoint.x += PickList[0].BeginPoint.x;
			   			SnapPoint.y += PickList[0].BeginPoint.y; 
			   			nPoints++;
			   		}
			   		else if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[NumPicked],FALSE,&npnts,&hPoints))
					{   
						HPDPOINT	Points=(HPDPOINT)GlobalLock (hPoints);

						if (npnts > 1 && SameDPoint (&Points[0],&Points[npnts-1]))
							npnts--;
						for (i=0;i<npnts;i++)
						{
							SnapPoint.x += Points[i].x;
							SnapPoint.y += Points[i].y;
						}
						nPoints += npnts;  
						GSSiGlobUlFree (&hPoints);
					}
		        }  
		        SnapPoint.x /= nPoints;
		        SnapPoint.y /= nPoints;
    			LockCursor (&SnapPoint); 
    		}
    			break;
		        
    		case GF_SNAP_END:
    			if (ldistp (BasePoint,PickList[NumPicked].EndPoint) < 
    				ldistp (BasePoint,PickList[NumPicked].BeginPoint)) 
    			{
    				LockCursor (&PickList[NumPicked].EndPoint);   
			    	SetCurAZ (PickList[NumPicked].EPAZ);
    			}	
    			else 
    			{
    				LockCursor (&PickList[NumPicked].BeginPoint); 
			    	SetCurAZ (PickList[NumPicked].BPAZ);
    			}	
    			break; 
    	}
LockPoint:
		if (!PtInWBounds (&CurrentPoint) && AutoPan)
            CenterWindow (CurrentPoint,FALSE);
    	CursorPoint = BasePtToScreenPt (&CurrentPoint);
        ClientToScreen (CurView->hWnd,(LPPOINT)&CursorPoint);
		SetPickCursor (FALSE);
		CreateDigCursor (CurView->hDC); 
		SetCurs ((HCURSOR)2,FALSE); 
    	SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
		DisplayCoordinate2 (&CurrentPoint,CursorPoint);
//           	RemoveGraphicsFunction (hWnd); 
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL EditAddress (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (848);
#endif
{
	POINTS	MousePoint;
	short	DisplayOpt=0;
	char key;     
	EDITRECTINFO Info;
	LPGWDHEADER	lpGWDHead;
    LPSEGDATAGM	pSegdata;   
    SEGDATAGM	SaveSegdata;    
	
	
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:   
    case WM_MBUTTONUP:
    {
		LPSTR	pString; 
		short	Item, len, DoChange, ChangeLen, pos = BT_FIRST; 
		long	Offset, NewVal, Ref, FromStreet, MissSeg=0, TotLen=0, CurLoc=0;
		LPLONG	pChangeAdd;
		HIGHLIGHTDATA	HighlightData;
		LPSHORT	pChangeSpeed;
	    BOOL	First=TRUE, HaveZero, OpenedSeg=FALSE;
		short	i,j;   
   		HCURSOR	hcurSave=0; 
		char	OneWayTxt[3][3]={"<>","->","<-"};
		BOOL	SaveDoPaint=DoPaint(), SaveHBW=HaveBlockingWindow, RemoveStreet;

        if (!CurView->HaveEditRect)
        	break;
		if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
			break;
	
	
		HaveBlockingWindow = TRUE;
		setDoPaint( FALSE); 
		_fmemmove (&Info,&CurEditRect,CurView->EditRectInfoSize);
		Ref = Info.Refno;
NextRef:  
		RemoveStreet = FALSE;
		lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
		pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData; 
	    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&Offset))
	    {   
			len=FillGWDData (lpGWDHead,Offset);
			SaveSegdata = *pSegdata;
//		    CloseStreetSegmentTable();
			HaveZero = FALSE;  
			for (i=0;i<4;i++)  
			{
				if (HaveZero)
					SaveSegdata.StreetNum[i] = 0;
				if (!SaveSegdata.StreetNum[i])
					HaveZero=TRUE;
			}

		    ChangeLen = 4;
			if (Info.AddID < 4)
			{   
				DisplayOpt = 1;
				pChangeAdd = &SaveSegdata.faddl;
				pChangeAdd += Info.AddID;  
				CurSegmentAdd = *pChangeAdd; 
			    DoChange = IDNO; 
				if (Message == WM_MBUTTONUP)
					DoChange = IDNO;
				else if (Message == WM_RBUTTONUP)
				{   
					long	SaveNewAdd=NewAdd;
					
					NewAdd += CurSegmentAddInc;
					if (VerifyAddIncrement)
					{
						char	mess[128];
						sprintf (mess,"Change %ld to %ld?",CurSegmentAdd ,NewAdd);
						DoChange = GSSiMsgBox (GetFocus(),mess,"Verify Address Change",MB_YESNO,0); 
						if (DoChange != IDYES)
							NewAdd = SaveNewAdd;
					}
					else 
						DoChange = IDYES;
					NewVal = NewAdd;
				}
				else if (Message == WM_LBUTTONUP)
	            {
	                  DLGPROC	lpfnADDRESS_EDITMsgProc;
	                  short	rc; 
	
	                  lpfnADDRESS_EDITMsgProc = MakeProcInstance((DLGPROC)ADDRESS_EDITMsgProc, hInst);
	                  rc = DialogBox(hInst, (LPSTR)"ADDRESS_EDIT", hWnd, lpfnADDRESS_EDITMsgProc);
	                  FreeProcInstance(lpfnADDRESS_EDITMsgProc);
	                  if (rc)
	                  {  
	                  	NewAdd = CurSegmentAdd;
	                  	NewVal = NewAdd;
	                  	DoChange = IDYES;
	                  } 
	                  else goto CancelChange;
	            }
				
			}
			else if (Info.AddID < 8)
			{   
				DisplayOpt = 2;
				pChangeAdd = &SaveSegdata.StreetNum[0];
				pChangeAdd += (Info.AddID-4); 
				if (First)
					FromStreet = *pChangeAdd; 
				if (Message == WM_MBUTTONUP)
				{   
					j = 0;
					DoChange = IDNO;
					for (i=0;i<4;i++)
					{
						if (SaveSegdata.StreetNum[i] != FromStreet)
							SaveSegdata.StreetNum[j++] = SaveSegdata.StreetNum[i]; 
						else
							DoChange = IDYES;
					}
					if (i != j)
						SaveSegdata.StreetNum[3] = 0;
					pChangeAdd = &SaveSegdata.StreetNum[0];
					NewVal = 0;
				    ChangeLen = 0;
				}
				else if ((!First || Message == WM_RBUTTONUP) && CurPath)
				{   
					DoChange = IDYES;
					if (VerifyAddIncrement && *pChangeAdd && First)
					{
						char	mess[256], OldName[66],NewName[66]; 
							
							
						GetTrueStreetName (*pChangeAdd, OldName, 0,0);
						GetTrueStreetName (CurPath, NewName, 0,0);
						sprintf (mess,"Change %s to %s?",OldName,NewName);
						DoChange = GSSiMsgBox (GetFocus(),mess,"Verify Street Name Change",MB_YESNO,0); 
					} 
					if (DoChange == IDYES)
						NewVal = CurPath;
				}
				else if (Message == WM_LBUTTONUP) 
			   	{
					DLGPROC lpfnGETSTREETNUMMsgProc; 
					BOOL	rc;
					
					StreetNum = CurPath=*pChangeAdd;		
					lpfnGETSTREETNUMMsgProc = MakeProcInstance((DLGPROC)GETSTREETNUMMsgProc, hInst);
					rc=DialogBox(hInst, (LPSTR)"GETSTREETNUM", hWndMain, lpfnGETSTREETNUMMsgProc);
					FreeProcInstance(lpfnGETSTREETNUMMsgProc);
				    switch (rc)
				    {  
				    	case 1:
						NewVal = CurPath;
						DoChange = IDYES;
						break;
						case 2:
						RemoveStreet = TRUE;
						NewVal = CurPath;
						DoChange = IDYES;
						break;
						case 0:
						goto CancelChange;
					}
				}
				if (DoChange == IDYES && NewVal)
				{   
					if (!FromStreet)
					{
						for (i=0;i<4;i++)  
							if (SaveSegdata.StreetNum[i] == NewVal)
								DoChange = IDNO; 
					}
					else
					{   
						BOOL	AlreadyHaveStreet=FALSE;
						
						j = 0;
						DoChange = IDNO;
						for (i=0;i<4;i++)
						{
							SaveSegdata.StreetNum[j] = SaveSegdata.StreetNum[i]; 
							if (SaveSegdata.StreetNum[i] == NewVal)
							{
								if (!AlreadyHaveStreet)
									j++; 
								AlreadyHaveStreet = TRUE;
								DoChange = IDYES;
							}
							else if (SaveSegdata.StreetNum[i] == FromStreet)
							{
								if (!AlreadyHaveStreet && !RemoveStreet) 
								{
									SaveSegdata.StreetNum[i] = NewVal;
									j++; 
								}
								AlreadyHaveStreet = TRUE;
								DoChange = IDYES;
							}
							else
								j++;	
						}
						if (i != j)
							SaveSegdata.StreetNum[3] = 0;
						pChangeAdd = &SaveSegdata.StreetNum[0];
					    ChangeLen = 0;
					}
				}  
				
			} 
			else if (Info.AddID == 8)
			{   
				DisplayOpt = 4;
				pChangeSpeed = &SaveSegdata.Speed;  
				ChangeLen = 2;
				if (Message == WM_MBUTTONUP)
					DoChange = IDNO;
				else if ((!First || Message == WM_RBUTTONUP) && CurSpeed)
				{   
					DoChange = IDYES;
					if (VerifyAddIncrement && *pChangeSpeed && First)
					{
						char	mess[256], OldName[66],NewName[66]; 
							
							
						sprintf (mess,"Change %i to %i?",*pChangeSpeed,(short)CurSpeed);
						DoChange = GSSiMsgBox (GetFocus(),mess,"Verify Speed Change",MB_YESNO,0); 
					} 
					if (DoChange == IDYES)
						NewVal = CurSpeed;
				}
				else if (Message == WM_LBUTTONUP)
				{   
					char	txt[32];
					
					itoa (*pChangeSpeed,txt,10);   
					DoChange = IDNO;
			   	  	if (GetTextString (GetFocus(),txt,32,"Enter speed",0,0,0,TRUE,TRUE))  
			   	  	{
			   	  		DoChange = IDYES;
						CurSpeed = atoi (txt);
					}
					else
						goto CancelChange;
					NewVal = CurSpeed;
			    }
			}
			else if (Info.AddID == 9)
			{   
				DisplayOpt = 5;
				pChangeAdd = &SaveSegdata.TrafVol;  
				ChangeLen = 4;
				if (Message == WM_MBUTTONUP)
					DoChange = IDNO;
				else if ((!First || Message == WM_RBUTTONUP) && CurVol)
				{   
					DoChange = IDYES;
					if (VerifyAddIncrement && *pChangeAdd && First)
					{
						char	mess[256], OldName[66],NewName[66]; 
							
							
						sprintf (mess,"Change %ld to %ld?",*pChangeAdd,CurVol);
						DoChange = GSSiMsgBox (GetFocus(),mess,"Verify Traffic Volume Change",MB_YESNO,0); 
					} 
					if (DoChange == IDYES)
						NewVal = CurVol;
				}
				else if (Message == WM_LBUTTONUP)
				{   
					char	txt[32];
					
					ltoa (*pChangeAdd,txt,10);   
					DoChange = IDNO;
			   	  	if (GetTextString (GetFocus(),txt,32,"Enter traffic volume",0,0,0,TRUE,TRUE))  
			   	  	{
			   	  		DoChange = IDYES;
						CurVol = atol (txt);
					}
					else
						goto CancelChange;
					NewVal = CurVol;
			    }
			}
			else if (Info.AddID == 10)
			{   
				DisplayOpt = 7;
				pChangeSpeed = &SaveSegdata.OneWay;  
				ChangeLen = 2;
				if (Message == WM_MBUTTONUP)
					DoChange = IDNO;
				else if ((!First || Message == WM_RBUTTONUP) && CurOneWay>=0)
				{   
					DoChange = IDYES;
					if (VerifyAddIncrement && *pChangeSpeed && First)
					{
						char	mess[256], OldName[66],NewName[66]; 
							
							
						sprintf (mess,"Change %s to %s?",OneWayTxt[*pChangeSpeed],OneWayTxt[CurOneWay]);
						DoChange = GSSiMsgBox (GetFocus(),mess,"Verify One-Way Change",MB_YESNO,0); 
					} 
					if (DoChange == IDYES)
						NewVal = CurOneWay;
				}
				else if (Message == WM_LBUTTONUP)
				{   
					char	txt[32];
					
					_fstrcpy (txt,OneWayTxt[*pChangeSpeed]);   
					DoChange = IDNO;   
TryOnewayAgain:
			   	  	if (GetTextString (GetFocus(),txt,32,"Enter one-way direction (<> -> or <-",0,0,0,TRUE,TRUE))  
			   	  	{   
			   	  		short	i;
			   	  		
			   	  		for (i=0;i<3;i++)
			   	  			if (!_fstrcmp (txt,OneWayTxt[i]))
			   	  				goto GoodOneWay;
			   	  		GSSiMsgBox (GetFocus(),"Invalid one-way entry",0,MB_ICONEXCLAMATION,0);
			   	  		goto TryOnewayAgain;
GoodOneWay:
			   	 		CurOneWay = i;
			   	  		DoChange = IDYES;
					}
					else
						goto CancelChange;
					NewVal = CurOneWay;
			    }
			}
			else if (Info.AddID == 11)
			{   
				DisplayOpt = 8;
				pChangeSpeed = &SaveSegdata.Lanes;  
				ChangeLen = 2;
				if (Message == WM_MBUTTONUP)
					DoChange = IDNO;
				else if ((!First || Message == WM_RBUTTONUP) && CurLanes)
				{   
					DoChange = IDYES;
					if (VerifyAddIncrement && *pChangeSpeed && First)
					{
						char	mess[256], OldName[66],NewName[66]; 
							
							
						sprintf (mess,"Change %i to %i?",*pChangeSpeed,(short)CurLanes);
						DoChange = GSSiMsgBox (GetFocus(),mess,"Verify Lane Change",MB_YESNO,0); 
					} 
					if (DoChange == IDYES)
						NewVal = CurLanes;
				}
				else if (Message == WM_LBUTTONUP)
				{   
					char	txt[32];
					
					itoa (*pChangeSpeed,txt,10);   
					DoChange = IDNO;
			   	  	if (GetTextString (GetFocus(),txt,32,"Enter Number of Lanes",0,0,0,TRUE,TRUE))  
			   	  	{
			   	  		DoChange = IDYES;
						CurLanes = atoi (txt);
					}
					else
						goto CancelChange;
					NewVal = CurLanes;
			    }
			}
			if (DoChange==IDYES)
			{
				time_t	systime;     
	
				time (&systime);
			
			    GlobalUnlock (hDBStreetSegments);
				OpenStreetSegmentTable (TRUE,&OpenedSeg);
				lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
				pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData; 
				if (ChangeLen == 2)
					*pChangeSpeed = NewVal;
				else if (ChangeLen == 4)
					*pChangeAdd = NewVal;
				*pSegdata = SaveSegdata;  
				pSegdata->UpdateTime = systime;
				GWDReplaceRecord (lpGWDHead,len,0,Offset);
				if (PickByRefno(Ref,0,0,-1))
					UpdateEmbeddedStreetNums (0,pSegdata->StreetNum);
				if (Info.AddID >3 && Info.AddID < 8)
					AddNameToSegmentInt (Ref,NewVal,RemoveStreet);
				FillRectPoly (CurView->hDC,(LPRECT)&CurEditRect,ConvertColor(CurView->BackGroundColor,-1));
				CurView->HaveEditRect = FALSE;
				if (First)
					DisplaySegmentAdds (lpGWDHead,&Info.Refno, DisplayOpt,1,0);
				First=FALSE;
			}   
	    }
	    else
	    	MissSeg++; 
		if (!BT_FIND (hHighlight,(LPSTR)&Ref,pos,BT_ANY,(LPSTR)&HighlightData))
		{   
			if (pos == BT_FIRST)
			{
				pos = BT_NEXT;
				if ((TotLen = BT_NUM_IN_INDEX (hHighlight)) > 10)
					CreateStatusWind (CurView->hWnd,1,0);
		        else 
		        {
					hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
				}
			}
			if (TotLen > 10)
				StatusWindowUpdate ("","Multiple Segment Editing", TotLen, CurLoc++);
		    GlobalUnlock (hDBStreetSegments);
			goto NextRef; 
		}
		if (TotLen > 10)
			DestroyStatusWindow (0);
		else if (hcurSave)
		{
			GSSiSetCursor(hcurSave);
		}
		if (MissSeg)
		{
			char	mess[128];
			sprintf (mess,"%ld segments not updated due to missing record in strtseg.gmd file",MissSeg);
			GSSiMsgBox (GetFocus(),mess,"Warning",MB_OK,0);
		}
CancelChange:	    
	    if (hDBStreetSegments)
	    	GlobalUnlock (hDBStreetSegments);
	    CloseStreetSegmentTable(OpenedSeg);  
		setDoPaint( SaveDoPaint);   
		HaveBlockingWindow = SaveHBW;
	}
    	break;  
    	
    case WM_MOUSEMOVE:
		MousePoint = MAKEPOINTS(lParam);
   		EditRectTrack (MousePoint);
   		break;
   		
    case WM_CHAR:
		key = wParam;
		switch (key)
		{   
			case 'r':
			case 'R': 
				if (!CurView->HaveEditRect)
					break;
				_fmemmove (&Info,&CurEditRect,CurView->EditRectInfoSize);
				if (Info.AddID >3 && Info.AddID < 8)
					PostMessage(hWnd, WM_MBUTTONUP, 0, 0L);
{
#if ENABLETRACE
GSSiExitProg (848);
#endif
			return TRUE; 
}
			
			case 'v':
			case 'V': 
			{
				char	mess[128], CStatus[6];
				
				if (VerifyAddIncrement)
				{
					VerifyAddIncrement=FALSE;
					_fstrcpy (CStatus,"off");
				}
				else
				{
					VerifyAddIncrement=TRUE;
					_fstrcpy (CStatus,"on");
				} 
				sprintf(mess, "Verify is now %s", CStatus);
				MessageBox(hWnd, mess, 0, MB_OK);
			}
{
#if ENABLETRACE
GSSiExitProg (848);
#endif
			return TRUE; 
}
			
		}
{
#if ENABLETRACE
GSSiExitProg (848);
#endif
       	return FALSE;
}

    default:
{
#if ENABLETRACE
GSSiExitProg (848);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (848);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AddSegStreetName (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (847);
#endif
{
	
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    {
		LPSTR	pString; 
		short	Item; 
		POINTS	MousePoint;
		DPOINT	BasePoint;
    	short	ID, ls, len, i;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;
		LPGWDHEADER	lpGWDHead;
	    LPSEGDATAGM	pSegdata;  
	    long	Offset; 
	    BOOL	OpenedSeg=FALSE;

        if (!StreetNum)
        {   
        	GSSiMsgBox (GetFocus(),"Street name not set",0,MB_ICONEXCLAMATION|MB_OK,0);
        	break;
        }
    	MousePoint = MAKEPOINTS(lParam);
	    BasePoint=WinPtSToBasePt(MousePoint);
	    PickItems (hWnd,BasePoint);
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;
		ProcessPickedItem (Item,FALSE); 
       	if (!CurSNamesLoc)
       		break;	
		if (!OpenStreetSegmentTable (TRUE,&OpenedSeg))
			break;
		lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
		pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
	    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&PickList[Item].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
	    {   
			time_t	systime;     
	
			time (&systime);
			len=FillGWDData (lpGWDHead,Offset); 
			for (i=0;i<4;i++)
			{
				if (pSegdata->StreetNum[i] == StreetNum)
					break;
				if (!pSegdata->StreetNum[i])
				{   
					pSegdata->StreetNum[i] = StreetNum;
					pSegdata->UpdateTime = systime;
					GWDReplaceRecord (lpGWDHead,len,0,Offset);
			       	if (CurSNamesLoc)
		       		{   
			       		BOOL	SaveDisableHalt;
			       		
			        	SaveDisableHalt = DisableHalt;
			    	    DisableHalt = TRUE;
		       		
						Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
						if (Fid != HFILE_ERROR)
						{
							GSSillseek (Fid,CurSNamesLoc,0);
							BigRead (Fid,(HPSTR)&ID,2);
							if (ID == 10)
								BigWrite (Fid,(HPSTR)&pSegdata->StreetNum,16,-1);
							GSSiClose (Fid);
						}
			        	DisableHalt = SaveDisableHalt;
			       	}
					break;   
				}  
			}
		}
		GlobalUnlock (hDBStreetSegments);
	    CloseStreetSegmentTable(OpenedSeg);  
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (847);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (847);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL LoadMarkersFromHighlight (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{HDC hDC;
 char key;     
 int	st; 
 BOOL	HaveDown; 
 POINT	MousePoint;
 DPOINT	BasePoint;

 switch (Message)
   {
 
   	case GF_INIT:
    {     
    	FILE	*Fid;  
    	char	str[1024], txt[128];
    	DPOINT	MPPoint;
    	BOOL	Opened, Opened2;
    	int		st,st2;
		NETLINKSKEY	NetLinksKey; 
		NETLINKSDATA	NetLinksData;
		NETREFSKEY	NetRefsKey;
		NETREFSDATA	NetRefsData; 
		NETMARKERSKEY1	NetMarkersKey1, NetMarkersKey1Del;
		NETMARKERSKEY2	NetMarkersKey2,NetMarkersKey2Del;
		NETMARKERSDATA1 NetMarkersData1,NetMarkersData1Del;
		NETMARKERSDATA2	NetMarkersData2,NetMarkersData2Del;
		HIGHLIGHTDATA	HighlightData;
		double			MPinc, PCT, MPVal, AZ;  
		char			File[]="file20.txt";
		char			snam[34], TrueName[34]="", Route[32];  
		int	iroute, i, NumChecked, Dummy, SaveMaxPick;     
		long	Checked[32];
		LPSTR	lpSpace;    
		long	WantStreetNum=0, TotNum, CurLoc;
		HCURSOR	hcurSave; 
		short	pos, LinkID, CheckedLink[32], NP;
		long	MarkerRef; 
		char	RouteAndMP[256];
		LPSTR	pRandMP; 
		BOOL	WildCard;

		if (Function == GF_DELETE_MARKERS_HLT)
		{
			DeleteHighlightedMarkers ();
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
			break;
		}
    	SetViewport(*pCommandViewport);
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
		if (Function == GF_LOAD_MARKERS_HLT) 
		{
		 	if (MessageBox( GetFocus(),"Do you wish to delete current markers and load these","Verify Marker Database Recreation",
            			MB_YESNO|MB_ICONQUESTION)==IDYES) 
				CreateStatusWind (CurView->hWnd,1,0);
			else
				break;
		}
	    TotNum = BT_NUM_IN_INDEX (hHighlight);
	    CurLoc = 0;
	      if (!TotNum)
	      {
			MessageBox( GetFocus(), File,"No markers highlighted", MB_OK|MB_ICONEXCLAMATION);
	     	break;
	      } 
		  
		  UseUserPickAp =FALSE;
		  SystemPickAp = 0;	
		  SaveMaxPick = MaxPick;        
		  MaxPick=8;  
		  PickNET=TRUE;
		  
		   
		  {
			  OpenNetLinkAndRef (NetworkID,FALSE,&Opened); 
			  if (Function == GF_LOAD_SINGLE_MARKER)
			  {
				  DeleteHighlightedMarkers ();			   
				  OpenNetMarkers (NetworkID,1,&Opened2); 
			  }
			  else 
				  OpenNetMarkers (NetworkID,2,&Opened2);
		  } 
	      
	      pos = BT_FIRST;
		  while (!BT_FIND (hHighlight,(LPSTR)&MarkerRef,pos,BT_ANY,(LPSTR)&HighlightData))
		  { 
		  	pos = BT_NEXT; 
            PickList[0]=HighlightData.PD;
            ProcessPickedItem (0,FALSE);    
          	MPPoint = HighlightData.PD.BeginPoint;
		    setDoPaint( FALSE);
		    PickItems (0,MPPoint); 
		    setDoPaint( TRUE);
		    AZ = PickList[NumPicked-1].PPAZ; 
		    NP = NumPicked; 
		    pRandMP = RouteAndMP;
		    _fstrcpy (pRandMP,"[%RMP]");
		    ExpandText (pRandMP);
		    while (GetRouteAndMP (&pRandMP,Route,&MPVal))
		    {
	          	_fstrcpy (snam,"ROUTE ");
	          	_fstrcat (snam,Route); 
	          	WildCard = FALSE;
	          	if (*LastChr (snam) == '*')
	          		WildCard = TRUE;
	          	else
					StreetNum = GetStreetNumFromName (snam,1,BT_FIRST,TrueName); 
				if (!WildCard && WantStreetNum && StreetNum != WantStreetNum)
					goto NextStreet;
			    NumPicked = NP;
			    NumChecked = 0;  
			    while (NumPicked--)
			    {   
					NetLinksKey.Ref = PickList[NumPicked].Refno;
					NetLinksKey.NetID = 0;
					st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
					while (!st && NetLinksKey.Ref == PickList[NumPicked].Refno)
					{   
						LinkID = NetLinksData.MP/1000000;	
						for (i=0;i<NumChecked;i++)
							if (NetLinksKey.Path == Checked[i] && LinkID == CheckedLink[i])
								goto NextPick;
						if (PickList[NumPicked].OffDist > MarkerTOL)
							goto NextPick;  
						CheckedLink[NumChecked] = LinkID;
						Checked[NumChecked++] = NetLinksKey.Path;
				    	GetTrueStreetName (NetLinksKey.Path,TrueName,0,0);
				    	lpSpace = _fstrchr (TrueName,' ');
				    	if (!lpSpace)
				    		lpSpace = TrueName;
				    	else
				    		lpSpace++;
						if (!_fstricmp (Route,lpSpace))
						{
							NetRefsKey.Path =  NetLinksKey.Path;
							NetRefsKey.MP = NetLinksData.MP; 
							st2 = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);
							PCT = PickList[NumPicked].PCT;  
							AZ	= PickList[NumPicked].PPAZ;
							if (NetRefsData.Dir == 2)
								PCT = 1.0 - PCT;
							MPinc = PCT * PickList[NumPicked].Length;
							NetLinksData.MP += MPinc;  
							NetMarkersKey1.MarkerID=1;
							NetMarkersKey1.Path = NetLinksKey.Path;
							_fmemset (NetMarkersKey1.Prefix,' ',2);   
							_fmemset (NetMarkersKey1.Suffix,' ',2);
							NetMarkersKey1.Value = MPVal; 
							NetMarkersData1.Location = MPPoint;
							NetMarkersData1.MP = NetLinksData.MP; 
							NetMarkersKey2.MarkerID=1;
							NetMarkersKey2.Path=NetLinksKey.Path;
							NetMarkersKey2.MP=NetLinksData.MP; 
							_fmemset (NetMarkersData2.Prefix,' ',4);
							NetMarkersData2.Value = NetMarkersKey1.Value; 
							NetMarkersKey2Del = NetMarkersKey2;
							NetMarkersKey2Del.MP -= NetTOL;  
							if (!BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2Del,BT_FIRST,BT_GE,(LPSTR)&NetMarkersData2Del))
							{
								if (NetMarkersKey2Del.MarkerID == NetMarkersKey2.MarkerID &&
									NetMarkersKey2Del.Path == NetMarkersKey2.Path &&
									fabs (NetMarkersKey2Del.MP - NetMarkersKey2.MP) <= NetTOL)
								{   
									NetMarkersKey1Del = NetMarkersKey1;
									NetMarkersKey1Del.Value = NetMarkersData2Del.Value;
									BT_DELETE (hBTNetMarkers2,(LPSTR)&NetMarkersKey2Del,(LPSTR)&NetMarkersData2Del,FALSE);
									BT_DELETE (hBTNetMarkers1,(LPSTR)&NetMarkersKey1Del,(LPSTR)&NetMarkersData1Del,FALSE);
								}
							}
							BT_PUT (hBTNetMarkers1,(LPSTR)&NetMarkersKey1,(LPSTR)&NetMarkersData1);
							BT_PUT (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,(LPSTR)&NetMarkersData2);
						}
						st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
					}  
	NextPick:;					          		
	          	} 
	NextStreet:;
          	} 
          	_fstrcpy (txt,Route);
          	_fstrcat (txt," - ");
          	sprintf (str,"%.3f",MPVal);
          	_fstrcat (txt,str);
          	
			DisplayMarkers = TRUE;
			DisplayMarker (MPPoint,2,txt,0.12,AZ+HALFPI,0,FALSE,FALSE,0,0,0,0,0);
			DisplayMarkers = FALSE;           
NextMarker:;
			CurLoc++; 
			if (Function == GF_LOAD_MARKERS_HLT)         
				StatusWindowUpdate ("Loading Network Markers",TrueName, TotNum, CurLoc);
		  }
		GSSiSetCursor (hcurSave);   
 		 
 		{
			CloseNetLinkAndRef (Opened);
			CloseNetMarkers (Opened2);  
		}
		CloseStreetNameTable ();
		if (Function == GF_LOAD_MARKERS_HLT)
			DestroyStatusWindow (0);		
		UseUserPickAp = TRUE; 
		MaxPick = SaveMaxPick; 
		PickNET = FALSE;
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		}				
       	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL ProcessCmdString (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (974);
#endif
{
 char key;     
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    {
		LPSTR	pString; 
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, ls;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;
    	short	SavePGC=ProcessGCmdStrings;

        GSSiGlobFree (&hGCmdString);
	    MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    ProcessGCmdStrings=0;
	    PickItems (hWnd,BasePoint); 
	    ProcessGCmdStrings = SavePGC;
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;
	    ProcessGCmdStrings=0;
		ProcessPickedItem (Item,FALSE);  
		ProcessGCmdStrings=SavePGC;      		
       	if (hGCmdString)
       	{   
       		BOOL	SaveDisableHalt;  
       		HANDLE	handle=GSSiGlobAlloc ( 765,GMEM_MOVEABLE,1024);
       		LPSTR	pStr=GlobalLock (handle);
       		
       		pString = GlobalLock (hGCmdString);   
       		_fstrcpy (pStr,pString);
			GlobalUnlock (hGCmdString);   
			ExpandText (pStr);
			GSSiGlobUlFree (&handle);
       	}
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (974);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (974);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SetPhotoTrans (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1112);
#endif
{HDC hDC;
 char key;     
 int	st;
 HIGHLIGHTDATA	HighlightData;
 POINT	MousePoint;
 DPOINT	BasePoint;    
 static	short	nPnts; 
 float	RSQMIN;
 static	double	XPic[16],YPic[16],XBase[16],YBase[16];  
 static	POINT	LastWinPt[16][2];
 char	mess[128]; 
 short	i;   
 HANDLE	hTran; 
 POINT	Points[2]; 
 DPOINT	PicPoint; 
 static	BOOL	WaitingForPoint=FALSE, HaveClrCmd=FALSE;

 switch (Message)
   {
   	case GF_INIT:    
   		nPnts = 0;
       	AddLBUTTON = TRUE;
   		break;

   	case GF_CLEAR_CMD: 
   		HaveClrCmd = TRUE; 
   	case GF_CLEAR: 
ClearEnd: 
	{
    	LPVIEWPORT	SaveView=CurView;  
    	
		SaveDC (CurView->hDC);  
       	for (i=0;i<nPnts;i++)
       	{    
			SaveDC (CurView->hDC);  
       		if (LastWinPt[i][0].x > SHRT_MIN)
       		{   
				NotPolylineScreen (CurView->hDC,LastWinPt[i],2,0,0);
				SetCurView ( SaveView);
			}  
			SetCurView ( SaveView);
		}
		RestoreDC (CurView->hDC,-1);
	} 
	break;
	
   	case GF_REDRAW_CMD: 
   		if (!HaveClrCmd)
   			break; 
   	case GF_REDRAW:  
	{
    	LPVIEWPORT	SaveView=CurView;  
    	
        HaveClrCmd = FALSE; 
		SaveDC (CurView->hDC);  
       	for (i=0;i<nPnts;i++)
       	{    
       		PicPoint.x = XPic[i];
       		PicPoint.y = YPic[i];
			SaveDC (CurView->hDC);  
       		if (PtInWBounds (&PicPoint))
       		{   
       			Points[0] = BasePtToScreenPt (&PicPoint); 
       			LastWinPt[i][0]=Points[0];
			    BasePoint.x = XBase[i];
			    BasePoint.y = YBase[i];
				SetCurView (pViewports[*pCommandViewport-1]); 
	       		if (PtInWBounds (&BasePoint))
	       		{   
					Points[1] = BasePtToScreenPt (&BasePoint);
	       			LastWinPt[i][1]=Points[1];
					NotPolylineScreen (CurView->hDC,Points,2,0,0);  
				}
				else
	       			LastWinPt[i][0].x=SHRT_MIN;
				SetCurView ( SaveView);
			}  
			else
				LastWinPt[i][0].x = SHRT_MIN;
			SetCurView ( SaveView);
		}
		RestoreDC (CurView->hDC,-1);
	}
   		break;
   		
   		
    case GF_COMPLETE: 
    {
    	LPVIEWPORT	SaveView=CurView;  
    
    	if (!WaitingForPoint || nPnts >= 16)
    		break;  
    	XPic[nPnts] = CurrentPoint.x;
    	YPic[nPnts] = CurrentPoint.y; 
    	WaitingForPoint=FALSE;
   		PicPoint.x = XPic[nPnts];
   		PicPoint.y = YPic[nPnts];
		SaveDC (CurView->hDC);  
   		if (PtInWBounds (&PicPoint))
   		{   
   			Points[0] = BasePtToScreenPt (&PicPoint); 
   			LastWinPt[nPnts][0]=Points[0];
		    BasePoint.x = XBase[nPnts];
		    BasePoint.y = YBase[nPnts];
			SetCurView (pViewports[*pCommandViewport-1]); 
			Points[1] = BasePtToScreenPt (&BasePoint);
   			LastWinPt[nPnts][1]=Points[1];
			NotPolylineScreen (CurView->hDC,Points,2,0,0);
		} 
		SetCurView ( SaveView);
    	nPnts++;  
    }
		break;
		
    case WM_LBUTTONUP:
    	XBase[nPnts] = CurrentPoint.x;
    	YBase[nPnts] = CurrentPoint.y; 
    	WaitingForPoint = TRUE;
        AddGraphicsFunction (hWnd, GF_SET_COORD,0);
        PostMessage (hWnd,Message,wParam,lParam);
		break;
		
    case WM_RBUTTONUP:
    {   
    	char	TranFileName[MAX_PATH], FullName[MAX_PATH]; 
    	HFILE	FidTran;
		OFSTRUCTGM	OFStruct;
    	LPSTR	lpSlash;
    	
    	_fstrcpy (TranFileName,CurView->lpFiles[0]);
    	ExpandText (TranFileName);
    	_fullpath (FullName,TranFileName,sizeof(FullName));
    	if (!(lpSlash = _fstrrchr (FullName,'\\')))
    		break;
    	*lpSlash++ = 0;
    	if (_fstricmp (lpSlash,"index"))
    		break;
    	_fstrcat (FullName,"\\tran");
    	FidTran = GSSiOpenFile (FullName,&OFStruct,OF_CREATE); 
     	hTran = STRAN2 (1626,XPic,YPic,XBase,YBase,nPnts,&RSQMIN,1,0);    
     	WriteTranData (FidTran,hTran);
    	GSSiClose (FidTran);
     	GSSiGlobUlFree (&hTran);
     	sprintf (mess,"Transformation set with %i control points - RSQ = %f",nPnts,RSQMIN);
     	MessageBox (GetFocus(),mess," ",MB_OK);   
     	
       	PostMessage(hWnd, GF_CLOSE,0, 0L); 
       	goto ClearEnd;
    }
		break;    
		
    default:
{
#if ENABLETRACE
GSSiExitProg (1112);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1112);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL TrackPhotoLoc (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1113);
#endif
{HDC hDC;
 char key;     
 int	st;
 HIGHLIGHTDATA	HighlightData;
 DPOINT	BasePoint, PicPoint;   
 static	HANDLE	hTran=0;  
 static	POINT	LastPoints[2];
 static BOOL	HaveLastLine=FALSE;

 switch (Message)
   {
   	case GF_INIT:
		HaveLastLine = FALSE;
       	AddLBUTTON = TRUE;
   		break;
    
    case WM_LBUTTONDOWN:
    {   
    	char	TranFileName[MAX_PATH], FullName[MAX_PATH]; 
    	HFILE	FidTran;
		OFSTRUCTGM	OFStruct;
    	short	i;
    	LPSTR	lpSlash;
    	
    	_fstrcpy (TranFileName,CurView->lpFiles[0]);
    	ExpandText (TranFileName);
    	_fullpath (FullName,TranFileName,sizeof(FullName));
    	if (!(lpSlash = _fstrrchr (FullName,'\\')))
    		break;
    	*lpSlash++ = 0;
    	if (_fstricmp (lpSlash,"index"))
    		break;
    	_fstrcat (FullName,"\\tran");
    	FidTran = GSSiOpenFile (FullName,&OFStruct,OF_READ); 
    	hTran = ReadTranData (FidTran);   
    	GSSiClose (FidTran);
     	break;
	}    
    case WM_LBUTTONUP:
		SaveDC (CurView->hDC);  
		if (HaveLastLine)
			NotPolylineScreen (CurView->hDC,LastPoints,2,0,0);
		RestoreDC (CurView->hDC,-1);
		GSSiGlobFree (&hTran);
		HaveLastLine = FALSE;
		break;
		
    case WM_MOUSEMOVE:
    {
    	LPVIEWPORT	SaveView=CurView;  
    	POINT	Points[2];
    	
        if (wParam != MK_LBUTTON || !hTran)
        	break;
		Points[0] = POINTStoPOINT(MAKEPOINTS(lParam));
	    PicPoint=ScreenPtToBasePt(Points[0]);
	    TRANS2 (PicPoint.x,PicPoint.y,&BasePoint.x,&BasePoint.y,hTran);
		SetCurView (pViewports[*pCommandViewport-1]); 
		Points[1] = BasePtToScreenPt (&BasePoint);
		SetCurView ( SaveView);
		SaveDC (CurView->hDC);  
		if (HaveLastLine)
			NotPolylineScreen (CurView->hDC,LastPoints,2,0,0);
		NotPolylineScreen (CurView->hDC,Points,2,0,0);
		LastPoints[0]=Points[0];
		LastPoints[1]=Points[1];
		HaveLastLine = TRUE;
		RestoreDC (CurView->hDC,-1);
	}
   		break;
   		
    default:
{
#if ENABLETRACE
GSSiExitProg (1113);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1113);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL SelectStreetTemplate (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1114);
#endif
{
 long	Sequence, AddInc;    
 char key;     
 LPGWDHEADER lpGWDHead; 
 HIGHLIGHTDATA	HighlightData;
 long	TLID;  
 static	BOOL	OpenedSeg=FALSE;
 switch (Message)
   {
   	case GF_INIT:
		if (Function == GF_SET_NULL_TEMPLATE)
		{   
			_fmemset (&StreetTemplate,0,sizeof(SEGDATAGM));
		    CloseStreetSegmentTable(TRUE);  
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
           	break;
		}
		if (Function == GF_CREATE_STREET_SEGMENT)
		{   
			short	pos=BT_FIRST, len;  
			long	Offset; 
			time_t	systime;     

			time (&systime);
			
			if (!OpenStreetSegmentTable (TRUE,&OpenedSeg))
				break;
			lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
			AddInc = GetGlobalLVal ("[%ADDRESS_INC]");
			while (!BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&TLID))
			{
				pos = BT_NEXT;
		        Offset = GSSillseek (lpGWDHead->Fid,0,2);
		        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&TLID,(LPSTR)&Offset);  
			  	len = sizeof(SEGDATAGM);
			   	if (BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1)!=2)
			   		goto ErrOut; 
	   			StreetTemplate.TLID = TLID;
	   			StreetTemplate.faddl+=AddInc;
	   			StreetTemplate.faddr+=AddInc;
	   			StreetTemplate.taddl+=AddInc;
	   			StreetTemplate.taddr+=AddInc; 
	   			StreetTemplate.UpdateTime = systime;
			   	if (BigWrite (lpGWDHead->Fid,(HPSTR)&StreetTemplate,len,-1) != len)
			   		goto ErrOut; 
				BT_FIND (hHighlight,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);
				PickList[0]=HighlightData.PD;
				UpdateEmbeddedStreetNums (0,StreetTemplate.StreetNum);
			}
			GlobalUnlock (hDBStreetSegments);
		    CloseStreetSegmentTable(OpenedSeg);  
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
           	break;
        }
       	AddLBUTTON = TRUE;
	    CloseStreetSegmentTable(OpenedSeg);  
   		break; 
ErrOut:
	    CloseStreetSegmentTable(OpenedSeg);  
		break;

    case WM_LBUTTONUP:
    {
		POINT	MousePoint;
		DPOINT	BasePoint;
		short	i;
        
		if (Function != GF_SELECT_STREET_TEMPLATE)
			break;
		if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
			break;
    	MousePoint=POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);  
	    ProcessGCmdStrings=0;
	    PickItems (hWnd,BasePoint); 
	    if (!NumPicked)
	    	break;
	    i=3;
	    while (i--)
	    { 
		    AddToHighlightList (PickList[NumPicked-1].Refno,&PickList[NumPicked-1],TRUE);
	    	ShowPickedItem (hWndMain,NumPicked-1); 
	    	Sleep (50);
		    RemoveFromHighlightList (PickList[NumPicked-1].Refno,2);
	    	ShowPickedItem (hWndMain,NumPicked-1); 
	    	Sleep (50);
		    RemoveFromHighlightList (PickList[NumPicked-1].Refno,0);
	    	ShowPickedItem (hWndMain,NumPicked-1); 
	    	Sleep (50);
	    }
	    TLID = PickList[NumPicked-1].Refno;
		if (GetSegDataGM (TLID,&StreetTemplate))
			Sound(GOOD_SOUND);
		else
			Sound(BAD_SOUND);
	    CloseStreetSegmentTable(OpenedSeg);  
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1114);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1114);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL TrackLocInOtherVP (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1116);
#endif
{HDC hDC;
 char key;     
 int	st;
 HIGHLIGHTDATA	HighlightData;
 DPOINT	BasePoint, PicPoint;   
 static	HANDLE	hTran=0;  
 static	POINT	LastPoints[2];
 static BOOL	HaveLastLine=FALSE;

 switch (Message)
   {
   	case GF_INIT:
		HaveLastLine = FALSE;
       	AddLBUTTON = TRUE;
   		break;
    
    case WM_LBUTTONUP:
		SaveDC (CurView->hDC);  
		if (HaveLastLine)
			NotPolylineScreen (CurView->hDC,LastPoints,2,0,0);
		RestoreDC (CurView->hDC,-1);
		GSSiGlobFree (&hTran);
		HaveLastLine = FALSE;
		break;
		
    case WM_MOUSEMOVE:
    {
    	LPVIEWPORT	SaveView=CurView;  
    	POINT	Points[2];
    	
        if (wParam != MK_LBUTTON)
        	break;
		Points[0] = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(Points[0]);
		SetCurView (pViewports[*pCommandViewport-1]); 
		Points[1] = BasePtToScreenPt (&BasePoint);
		SetCurView ( SaveView);
		SaveDC (CurView->hDC);  
		if (HaveLastLine)
			NotPolylineScreen (CurView->hDC,LastPoints,2,0,0);
		NotPolylineScreen (CurView->hDC,Points,2,0,0);
		LastPoints[0]=Points[0];
		LastPoints[1]=Points[1];
		HaveLastLine = TRUE;
		RestoreDC (CurView->hDC,-1);
	}
   		break;
   		
   	case GF_REDRAW_CMD: 
   		HaveLastLine=FALSE;
   		break; 
   			
    default:
{
#if ENABLETRACE
GSSiExitProg (1116);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1116);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL CreateNullMap (HWND hWnd, int Message, short Function)
{ 

 switch (Message)
   {
   	case GF_INIT:    
       	AddLBUTTON = FALSE;
       	PostMessage(hWnd, GF_EXECUTE,0, 0L);
   		break;
    
    case GF_EXECUTE: 
    {
    	char	Name[MAX_PATH]="";
		short	nRc;
		DLGPROC lpfnIMPORT_LIMITSMsgProc;
		MNMXCORD	SaveEditBounds=EditBounds; 
							
		lpfnIMPORT_LIMITSMsgProc = MakeProcInstance((DLGPROC)IMPORT_LIMITSMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"IMPORT_LIMITS", hWnd, lpfnIMPORT_LIMITSMsgProc);
		FreeProcInstance(lpfnIMPORT_LIMITSMsgProc);
		if (nRc)
		{
			LPIMPORTLIMITS	lpLimits;
				 
        	lpLimits = (LPIMPORTLIMITS)GlobalLock (hImportLimits);
	    	if (Function == GF_CREATE_NULL_DIR)
	        {
				DLGPROC	lpfnNULLDIRMsgProc; 
				short		nRc;
					
				*(LPMNMXCORD)&UserBounds = lpLimits->MinMax;
				lpfnNULLDIRMsgProc = MakeProcInstance((DLGPROC)NULLDIRMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"NULLDIR", hWnd, lpfnNULLDIRMsgProc);
				FreeProcInstance(lpfnNULLDIRMsgProc);
	        }
	    	else
	    	{ 
		    	if (GetSaveName2 (hWnd,Name,IDS_FILTERPLT_ONLY,".PLT",IDS_FILEPLT))
			        CreateNewMap (Name,&lpLimits->MinMax,0,0,0,0,0,0,FALSE);
			}
    	} 
	    GSSiGlobUlFree (&hImportLimits);
    	EditBounds = SaveEditBounds;
       	PostMessage(hWnd, GF_CLOSE,0, 0L);
    }
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL IdentifyPolygons (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
    	IDPolygons ();  
        PostMessage(hWnd, GF_CLOSE,0, 0L);
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL EditText (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (879);
#endif
{
 short	rtn=1; 
 static	BOOL	HaveNewColor; 
 static	COLORREF	NewColor;   
 switch (Message)
   {
   	case GF_INIT:
		if (!HaveUpdateFile ())
		{
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
       	AddLBUTTON = TRUE;  
       	HaveNewColor = FALSE; 
       	NewColor = 0;
{
#if ENABLETRACE
GSSiExitProg (879);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;
    
    case GF_CLOSE:
    	rtn=0;
    	break;
    	
    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {             
		LPSTR	pString; 
		short	Item, NewLen; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, ls;  
    	HFILE	Fid;
		OFSTRUCTGM	OFStruct;
    	GRTEXTHEADER	GRTextHeader;
		LPGRTEXTHEADER	pPickedTextHeader;


        if (Message == GF_EXECUTE)
        	Item = 0;
        else
        {   
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		    PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
		}
	    hPickedTextHeader = GSSiGlobAlloc ( 642,GMEM_MOVEABLE,sizeof(GRTEXTHEADER)+512); 
	    pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
		lTextString = 0; 
		GSSiGlobFree (&hTextString);   
		ProcessPickedItem (Item,FALSE);        		
       	if (lTextString && hTextString)
       	{   
       		BOOL	SaveDisableHalt, HaveValue, HaveColor=FALSE;
       		char	Text[512], CColor[32]; 
       		LPSTR	pStr, pEndColor;
       		short	l;
       		
       		pString = GlobalLock (hTextString);
       		if (!_fstrnicmp (pString,"$RGB(",5))
       		{
       			HaveColor = TRUE;
       			pEndColor = _fstrchr (pString+5,')');  
       			l = (long) pEndColor - (long) pString + 1;
       			strncpy0 (CColor,pString,l); 
       			pString += l;
       			ExpandText (CColor);
       			NewColor = atol (CColor);
       		}
       		_fstrcpy (Text,pString);
			GSSiGlobUlFree (&hTextString);  
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		if (Function == GF_EDIT_TEXT)
				HaveValue = GetTextStringML (hWnd,Text,510,"Edit Text",0);
			else
			{   
				if (!HaveNewColor)
					HaveValue = GetColor (CurView->hWnd,&NewColor);
				else
					HaveValue = TRUE; 
				HaveNewColor = TRUE;
				HaveColor = TRUE;
			} 
			if (HaveValue)
			{   
				char	NewText[512];
				
				if (HaveColor)
					sprintf (NewText,"$RGB(%i,%i,%i)%s",GetRValue (NewColor),GetGValue (NewColor),GetBValue (NewColor),Text);
				else
					_fstrcpy (NewText,Text);
				Truncate (NewText); 
				NewLen = _fstrlen (NewText); 
				NewLen += NewLen % 2; 
				if (!CurView->UpdateFile || !_fstricmp (EditName,PickName))
				{
			       	if (CurTextStringLoc && NewLen <= lTextString)
					{
						Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
						if (Fid != HFILE_ERROR)
						{
							GSSillseek (Fid,CurTextStringLoc,0);
							BigRead (Fid,(HPSTR)&ID,2);
							BigRead (Fid,(HPSTR)&GRTextHeader,sizeof(GRTEXTHEADER)); 
							if (ID == 19)
								BigWrite (Fid,(HPSTR)NewText,lTextString,-1);
							GSSiClose (Fid);
						}
						ProcessPickedItem (Item,0);        		
					}
					else
					{   
						UpdateItem=19;
						pPickedTextHeader->lText = NewLen;
						pStr = (LPSTR)pPickedTextHeader;
						pStr += sizeof(GRTEXTHEADER);
						_fstrcpy (pStr,NewText);
						UpdateRecord (Item,PickList[Item].Desc,0,0,0,0,0,-1);
					} 
				}
		       	else
		       	{
					UpdateItem=19;
					pPickedTextHeader->lText = NewLen;
					pStr = (LPSTR)pPickedTextHeader;
					pStr += sizeof(GRTEXTHEADER);
					_fstrcpy (pStr,NewText);
					UpdateRecord (Item,PickList[0].Desc,0,0,0,0,1,-1);
			    }
				rtn = GF_INCREASE_SUCCESS_COUNT; 
			}
        	DisableHalt = SaveDisableHalt;
    	} 
       	GSSiGlobUlFree (&hPickedTextHeader);
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L);
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (879);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (879);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL Sponge (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (151);
#endif
{

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
		PostMessage(hWnd, GF_CLOSE,0, 0L);  
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (151);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (151);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL EditTextMultiple (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (881);
#endif
{
 char key;     
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE:  
    {
		LPSTR	pString; 
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, ls;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;
    	GRTEXTHEADER	GRTextHeader; 
    	short	pos = BT_FIRST;
    	HIGHLIGHTDATA	HighlightData; 
    	long	Refno=LONG_MIN; 
    	char	ChangeCommand[256];


		while (!BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_GT,(LPSTR)&HighlightData)) 
		{   
			pos = BT_NEXT; 
			PickList[0] = HighlightData.PD;
			CurView->PassID = 4;
			lTextString = 0; 
			GSSiGlobFree (&hTextString);   
		    hPickedTextHeader = GSSiGlobAlloc ( 642,GMEM_MOVEABLE,sizeof(GRTEXTHEADER)+512); 
			ProcessPickedItem (0,FALSE);        		
	       	if (lTextString && hTextString)
	       	{   
	       		BOOL	SaveDisableHalt;
	       		
			    GetGlobalValRaw ("%TEXTCHANGECOMMAND",ChangeCommand);
	       		pString = GlobalLock (hTextString); 
	       		SetGlobalValue ("%CURRENTTEXT",pString);
	        	SaveDisableHalt = DisableHalt;
	    	    DisableHalt = TRUE;
	    	    if (*ChangeCommand)
	    	    {
		       		ExpandText (ChangeCommand); 
					if (!CurView->UpdateFile || !_fstricmp (EditName,PickName))
					{
			       		if (_fstrlen (ChangeCommand) <= lTextString)
						{   
							Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
							if (Fid != HFILE_ERROR)
							{
								GSSillseek (Fid,CurTextStringLoc,0);
								BigRead (Fid,(HPSTR)&ID,2);
								BigRead (Fid,(HPSTR)&GRTextHeader,sizeof(GRTEXTHEADER)); 
								if (ID == 19)
									BigWrite (Fid,(HPSTR)ChangeCommand,lTextString,-1);
								GSSiClose (Fid);
							}
						}
					}
					else
					{   
						LPGRTEXTHEADER	pPickedTextHeader; 
						LPSTR	pStr;               
						short	NewLen; 
						
					    pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
						UpdateItem=19;
						NewLen = _fstrlen (ChangeCommand);
						NewLen += NewLen % 2;
						pPickedTextHeader->lText = NewLen;
						pStr = (LPSTR)pPickedTextHeader;
						pStr += sizeof(GRTEXTHEADER);
						_fstrcpy (pStr,ChangeCommand);
						UpdateRecord (0,PickList[0].Desc,0,0,0,0,1,1);  
						HighlightData.PD = PickList[1];
						GlobalUnlock (hPickedTextHeader); 
						BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)&HighlightData);
					} 

				}
	        	DisableHalt = SaveDisableHalt;
				GSSiGlobUlFree (&hTextString);  
	       	} 
			GSSiGlobFree (&hPickedTextHeader);  
	    } 
   		PostMessage(hWnd, GF_CLOSE,0, 0L); 

    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (881);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (881);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL AddEditTAG (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (878);
#endif
{ 
 static	char	NewTAGSave[80]; 
 short	rtn=1;  
 BOOL	Picked=FALSE;
 
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;  
        SetCurs (LoadCursor (hInst,"SETTAG"),FALSE);
       	GetGlobalCVal ("[%NEWTAG]",NewTAGSave,0);
{
#if ENABLETRACE
GSSiExitProg (878);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;
    
    case GF_CLOSE:  
    	SetGlobalValue ("%NEWTAG","");   
    	*NewTAGSave = 0;
{
#if ENABLETRACE
GSSiExitProg (878);
#endif
    	return FALSE;
}
    	break;
    	 
    case WM_LBUTTONUP: 
    	Picked = TRUE;
       	GetGlobalCVal ("[%NEWTAG]",NewTAGSave,0);
    	SetGlobalValue ("%NEWTAG","");   
    case GF_EXECUTE:
    {
		LPSTR	pColon; 
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
		DLGPROC lpfnADDEDITTAGMsgProc;
		int	nRc; 
		long	Refno;
    	
        if (Message == GF_EXECUTE)
        	Item = 0;
        else
        {
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		    PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
			if (Message == GF_USEPICKED)
				NumPicked = 1;
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
		} 
		Refno = PickList[Item].Refno;
		ProcessPickedItem (Item,FALSE);        		
		
		if (!*NewTAGSave)
		{				
			lpfnADDEDITTAGMsgProc = MakeProcInstance((DLGPROC)ADDEDITTAGMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"ADDEDITTAG", hWndMain, lpfnADDEDITTAGMsgProc);
			FreeProcInstance(lpfnADDEDITTAGMsgProc);
			if (!nRc)
			 	return GF_EXECUTE_CANCELED;   
			sprintf (NewTAGSave,"%s:%s",NewPrefix,NewUDI); 
		} 
		else
		{
			if ((pColon = _fstrchr (NewTAGSave,':')))
			{
				*pColon = 0;
				_fstrcpy (NewPrefix,NewTAGSave);
				*pColon++ = ':';
				_fstrcpy (NewUDI,pColon);   
			}
		}
		ExpandText (NewPrefix);
		ExpandText (NewUDI);
		sprintf (NewTAG,"%s:%s",NewPrefix,NewUDI);
		if (EditTAG (Item,NewTAG)) 
			rtn = GF_INCREASE_SUCCESS_COUNT;
		if (Picked)
		{   
			if (PickByRefno(Refno,0,0,-1))
				ProcessPickedItem (0,TRUE);        		
		} 
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L);
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (878);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (878);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL EditTextHeader (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (843);
#endif
{
 char key; 
 char	str[512];
 short	l;
 short	rtn=FALSE;
 double OldSize;
 LPGRTEXTHEADER	pPickedTextHeader;
 static	BOOL	HaveNewSettings;     
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;  
       	HaveNewSettings = FALSE;
{
#if ENABLETRACE
GSSiExitProg (843);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case WM_LBUTTONUP:
    {
		LPSTR	pString; 
		short	Item=0; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, ls;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

    	HaveNewSettings = FALSE;
     	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
    case GF_EXECUTE:
		if (Message == GF_USEPICKED || Message == GF_EXECUTE)
			NumPicked = 1;
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;
	    hPickedTextHeader = GSSiGlobAlloc ( 519,GMEM_MOVEABLE,sizeof(GRTEXTHEADER)+512); 
		ProcessPickedItem (Item,FALSE);        		
	    pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
       	if (CurTextHeaderLoc)
       	{   
       		BOOL	SaveDisableHalt;  
       		GRTEXTHEADER	GRTextHeaderOld;
       		
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
       		if ((Function == GF_EDIT_TEXT_HEADER ||Function == GF_SET_TEXT_GLOBALS) && !HaveNewSettings)
       		{
				DLGPROC	lpfnTEXTHEADEDITMsgProc;
				short	rc; 
				
				Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READ);
				if (Fid != HFILE_ERROR)
				{
					GSSillseek (Fid,CurTextHeaderLoc,0);
					BigRead (Fid,(HPSTR)&ID,2);
					if (ID == 19)
					{
						BigRead (Fid,(HPSTR)&GRTextHeader,sizeof(GRTEXTHEADER)); 
						if (Function == GF_SET_TEXT_GLOBALS)
						{ 
							BigRead (Fid,str,GRTextHeader.lText); 
							str[GRTextHeader.lText] = 0;
						}
					}
					GSSiClose (Fid);  
					if (Function == GF_SET_TEXT_GLOBALS)
					{   
						double	THeight;
						
						SetGlobalValue ("%PICKED_TEXT",str);
				    	THeight = GetTextHeadSize (&GRTextHeader);
		    			SetGlobalValueReal ("%PICKED_TEXTSIZE",THeight);
		    			SetGlobalValueDPoint ("%PICKED_TEXTLOC",TXLoc);
		    			SetGlobalValueReal ("%PICKED_TEXTAZ",TXRot);
				       	GSSiGlobUlFree (&hPickedTextHeader); 
						PostMessage(hWnd, GF_CLOSE,0, 0L);
						break; 
					}
					lpfnTEXTHEADEDITMsgProc = MakeProcInstance((DLGPROC)TEXTHEADEDITMsgProc, hInst);
					rc = DialogBox(hInst, (LPSTR)"TEXTHEADEDIT", hWnd, lpfnTEXTHEADEDITMsgProc);
					FreeProcInstance(lpfnTEXTHEADEDITMsgProc);
					if (rc)
						HaveNewSettings = TRUE;
					else 
					{
			        	DisableHalt = SaveDisableHalt;
				       	GSSiGlobUlFree (&hPickedTextHeader); 
			        	if (wParam == 1)  
{
#if ENABLETRACE
GSSiExitProg (843);
#endif
			        		return GF_EXECUTE_CANCELED;
}
			        	else
							PostMessage(hWnd, GF_CLOSE,0, 0L); 
						break; 
					}
				}
			}

			if (!CurView->UpdateFile || !_fstricmp (EditName,PickName))
			{
				Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
				if (Fid != HFILE_ERROR)
				{   
					
					GSSillseek (Fid,CurTextHeaderLoc,0);
					BigRead (Fid,(HPSTR)&ID,2);
					if (ID == 19)
					{   
						
						BigRead (Fid,(HPSTR)&GRTextHeaderOld,sizeof(GRTEXTHEADER));
						l = GRTextHeaderOld.lText; 
						OldSize = GetTextHeadSize (&GRTextHeaderOld); 
						if (Function == GF_EDIT_TEXT_HEADER)
						{
							GRTextHeaderOld = GRTextHeader;
							GRTextHeaderOld.lText = l; 
						}
						else if (Function == GF_FACTOR_TEXT)
							TextSizeFactor = GetGlobalDVal2 ("[%SIZECHANGEFACTOR]",1);
						if (TextSizeFactor > 0)
							SetTextHeadSize (&GRTextHeaderOld,OldSize*TextSizeFactor);
						GSSillseek (Fid,CurTextHeaderLoc+2,0);
						BigWrite (Fid,(HPSTR)&GRTextHeaderOld,sizeof(GRTEXTHEADER),-1); 
					}
					GSSiClose (Fid);  
				}
			}
	       	else
	       	{
				UpdateItem=19; 
				l = pPickedTextHeader->lText;
				OldSize = GetTextHeadSize (pPickedTextHeader); 
				if (Function == GF_EDIT_TEXT_HEADER)
				{
					*pPickedTextHeader = GRTextHeader;
					pPickedTextHeader->lText = l;
					if (TextSizeFactor > 0)
					{ 
						SetTextHeadSize (pPickedTextHeader,OldSize*TextSizeFactor);
					}
				}
				else if (Function == GF_FACTOR_TEXT)
				{
					TextSizeFactor = GetGlobalDVal2 ("[%SIZECHANGEFACTOR]",1);
					SetTextHeadSize (pPickedTextHeader,OldSize*TextSizeFactor);
				}
				UpdateRecord (Item,PickList[0].Desc,0,0,0,0,1,-1);
		    }
        	DisableHalt = SaveDisableHalt;
			rtn = GF_INCREASE_SUCCESS_COUNT; 
       	}
       	GSSiGlobUlFree (&hPickedTextHeader); 
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L);  
       	return rtn;
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (843);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (843);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}
BOOL AddTurnData (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
	TURNKEY	TurnKey;
	double	TurnCost;
	
 switch (Message)
   {
   	case GF_INIT:
    {     
    	BOOL	Opened;
    	int		st;
		HIGHLIGHTDATA	HighlightDataFrom, HighlightDataTo;
		long	TotNum, Sequence, RefnoFrom, RefnoTo;

	    TotNum = BT_NUM_IN_INDEX (hHighlight2);
	    if (!TotNum)
	    {
			MessageBox( GetFocus(), "No items highlighted",0, MB_OK|MB_ICONEXCLAMATION);
	     	break;
	    } 
		if (!OpenTurnTable (TRUE,&Opened))
			break;
		BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&RefnoFrom);
		BT_FIND (hHighlight,(LPSTR)&RefnoFrom,BT_FIRST,BT_EQ,(LPSTR)&HighlightDataFrom);
		while (!BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&RefnoTo))
		{ 
			BT_FIND (hHighlight,(LPSTR)&RefnoTo,BT_FIRST,BT_EQ,(LPSTR)&HighlightDataTo);
			if (ldistp (HighlightDataFrom.PD.BeginPoint,HighlightDataTo.PD.BeginPoint) < NetTOL) 
			{
				TurnKey.FromEnd = 1;
				TurnKey.ToEnd = 1;
			}
			else if (ldistp (HighlightDataFrom.PD.BeginPoint,HighlightDataTo.PD.EndPoint) < NetTOL) 
			{
				TurnKey.FromEnd = 1;
				TurnKey.ToEnd = 2;
			}
			else if (ldistp (HighlightDataFrom.PD.EndPoint,HighlightDataTo.PD.BeginPoint) < NetTOL) 
			{
				TurnKey.FromEnd = 2;
				TurnKey.ToEnd = 1;
			}
			else if (ldistp (HighlightDataFrom.PD.EndPoint,HighlightDataTo.PD.EndPoint) < NetTOL) 
			{
				TurnKey.FromEnd = 2;
				TurnKey.ToEnd = 2;
			} 
			else
				goto NextRef;
			TurnKey.FromRef = RefnoFrom;
			TurnKey.ToRef = RefnoTo;
			TurnKey.Time = 0;
			TurnCost = MAX_COST; 
			BT_PUT (hBTTurnTable,(LPSTR)&TurnKey,(LPSTR)&TurnCost);
NextRef:;
       	} 
		CloseTurnTable (Opened);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
	}				
       	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL OffsetPolylinesHLT (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (717);
#endif
{
 POINT	MousePoint;
 DPOINT	BasePoint;  
 char	key,str[128];
 static	HaveDown=FALSE;  
 long	TotNum, Refno, StartSeq, EndSeq;
 int	NumPoints;
 HANDLE	hPoints;     
 BOOL	rc;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
	    TotNum = BT_NUM_IN_INDEX (hHighlight2);
	    if (!TotNum)
	    {
			GSSiMsgBox( GetFocus(), "No items highlighted",0, MB_OK|MB_ICONEXCLAMATION,0);
	     	goto Exit;
	    } 
		GetGlobalCVal ("[%OFFSETLINEDIST]",str,0);
		OffsetLineOffset = atobasedist (str,&rc); 
		if (OffsetLineOffset < 0.0001)
        {   
	         GSSiMsgBox(0,"Offset distance not set", "Offset Error",MB_ICONSTOP,0);
	         goto Exit;
        }
        
        StartSeq = 0;
		hPoints = GetConnectedItems (FALSE,&NumPoints);
		set_left_side_only ();
        OffsetHighlightedLines (NumPoints,hPoints,OffsetLineOffset); 
        GSSiGlobUlFree (&hPoints);
Exit:
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (717);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (717);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL ADJUSTBITMAPCOLORSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 	 
	static	LPVIEWPORT	OrthVP; 
	short	ii;
	static	char	BitmapPath[MAX_PATH]="";
	HDIB32	hDIB;
	
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         hWndAdjustBitmapColors = hWndDlg; 
         OrthVP = CurView;
         break; 
    
    case WM_DESTROY:
         hWndAdjustBitmapColors = 0; 
    	 return 0;
    	 
    case WM_CLOSE:
		{
			int	height, width, row, col;

			if (!nBMPColors)
				break;
			hDIB = BMPHandleFromEXT (BitmapPath);
			if (!hDIB)
				break;
			GetDIBDimensionsFromHandle(hDIB,&height,&width);
			CreateStatusWind (hWndDlg,1,0);
			for (row=0;row<height;row++)
			{
				for (col=0;col<width;col++)
				{
					RGBQUAD	c;
					
					FreeImage_GetPixelColor (hDIB,col,row,&c);
					{
						double	d2,d = RGBQUADDist (c, RGBQUADFromCOLORREF (FromColor[0]));
						int	i,neari=0;
						
						for (i=1;i<nBMPColors;i++)
						{
							d2 = RGBQUADDist (c, RGBQUADFromCOLORREF(FromColor[i]));
							if (d2 < d)
							{
								d = d2;
								neari = i;
							}
						}
						c = RGBQUADFromCOLORREF (ToColor[neari]);
						FreeImage_SetPixelColor (hDIB,col,row,&c);
					}
				}
				StatusWindowUpdate (0,0,height,row);
			}
			SaveDIB32 (hDIB,BitmapPath,-1,0);
		}
		DestroyStatusWindow(0);  
	    DestroyWindow(hWndDlg); 
	    break;
	      
    case WM_COMMAND: 
    	 SetCurView ( OrthVP);
         switch(LOWORD(wParam))
         { 
			case IDOK:
				 if (GetFileName3 (hWndDlg,BitmapPath,IDS_FILTERBMP,IDS_FILEBMP))
				 { 
					 SetGlobalValue ("%PLOT",BitmapPath);
					 RedisplayViewports(FALSE);
				 }
				break;
            case IDCANCEL: 
				PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
            break; 
            
            case IDC_DISTRIBUTION:
            	ii=1;
            	break;
            	
            case IDC_SCAN: 
            {
            	HBITMAP	hBmp, hbmpScreen; 
            	HDC		hDC; 
            	HWND	hWnd;  
            	RECT	Rect; 
            	DWORD	BitMapDim;
            	UINT	width, height, w, i; 
            	long	Dist[256], MaxVal;
            	HCURSOR	hcurSave; 
            	COLORREF	color; 
            	double	Factor;  
            	POINT	Point[2];
            	
				hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
            	
				height = CurView->DrawRect.right - CurView->DrawRect.left + 1;
				width = CurView->DrawRect.bottom - CurView->DrawRect.top + 1; 
				_fmemset (Dist,0,256*sizeof(long));
				while (height--)
				{
					w = width;
					while (w--)
					{
						color = GetPixel (CurView->hDC,w,height);
						i = GetRValue (color);
						Dist[i]++; 
					}
				}
				GSSiSetCursor(hcurSave);
				MaxVal = 0; 
				for (i=0;i<256;i++)
					MaxVal = max (MaxVal,Dist[i]); 
				hWnd = GetDlgItem(hWndDlg,IDC_DISTRIBUTION);
				hDC = GetDC (hWnd);  
				GetClientRect (hWnd,&Rect);
				height = Rect.bottom - Rect.top + 1;
				width = Rect.right - Rect.left + 1;
				Factor = (double)width / (double) MaxVal;    
				for (i=0;i<256;i++)
				{
					Point[0].x = 0;
					Point[0].y = Point[1].y = i+2; 
					Point[1].x = IDNINT (Dist[i] * Factor);
					Polyline (hDC,Point,2); 
				}
				ReleaseDC (hWnd,hDC);
			}
            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL AdjustBitmapColors (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{
#define	NUMENTRIES	256  
	POINT	MousePoint;   
	char	str[128]; 
	int		row,col;
	static	COLORREF	Color;
	
 switch (Message)
   {
   	case GF_INIT:
		nBMPColors = 0;
       	AddLBUTTON = FALSE;  
       	if (!hWndAdjustBitmapColors)
	    {
		  DLGPROC lpfnADJUSTBITMAPCOLORSMsgProc;
		  
	      lpfnADJUSTBITMAPCOLORSMsgProc = MakeProcInstance((DLGPROC)ADJUSTBITMAPCOLORSMsgProc, hInst);
	      CreateDialog(hInst, (LPSTR)"ADJUSTBITMAPCOLORS", hWnd, lpfnADJUSTBITMAPCOLORSMsgProc);
	    }
   		break;
	
	case GF_CLOSE:
		if (hWndAdjustBitmapColors)
			DestroyWindow (hWndAdjustBitmapColors);
		break;

    case WM_MOUSEMOVE:
		
    	if (!hWndAdjustBitmapColors)
    		break;
		MousePoint =POINTStoPOINT(MAKEPOINTS(lParam));
		Color = GetPixel (CurView->hDC,MousePoint.x,MousePoint.y); 
		sprintf (str,"%ld %ld %ld",(long)GetRValue(Color),(long)GetGValue(Color),(long)GetBValue(Color)); 
		SetDlgItemText (hWndOrthoFilter,IDC_CVALUE2,str);  
		sprintf (str,"%ld",Color); 
		SetDlgItemText (hWndOrthoFilter,IDC_CVALUE,str);  
		if (GetGlobalCVal ("%ORTHOFILTERMACRO",str,0))
		{
			CurrentPoint = ScreenPtToBasePt (MousePoint);
			ProcessText (str);
		}
		break;
		
	case WM_CHAR:
    {
		switch (wParam)
		{   
			case 'T':
			case 't': 
				if (!nBMPColors)
					break;
				for (row=CurView->ScreenRect.top;row<=CurView->ScreenRect.bottom;row++)
					for (col=CurView->ScreenRect.left;col<=CurView->ScreenRect.right;col++)
					{
						COLORREF	c = GetPixel (CurView->hDC,col,row);
						double	d2,d = ColorDist (c, FromColor[0]);
						int	i,neari=0;
						
						for (i=1;i<nBMPColors;i++)
						{
							d2 = ColorDist (c, FromColor[i]);
							if (d2 < d)
							{
								d = d2;
								neari = i;
							}
						}
						SetPixel (CurView->hDC,col,row,ToColor[neari]);
					}
			break;

			case 'S':
			case 's':
			{
			}
			break;

            default:
               	return FALSE;
		}  
		return TRUE;
	}
    case WM_LBUTTONUP:
    {
		FromColor[nBMPColors] = Color;
	}
		break;               
		
    case WM_RBUTTONUP:
    {
		if (nBMPColors < 15)
			ToColor[nBMPColors++] = Color;
	}
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL OrthoFilterFunction (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{
#define	NUMENTRIES	256  
	POINT	MousePoint;   
	char	str[128]; 
	COLORREF	Color; 
	static	short	CurColorIndex;
	
 switch (Message)
   {
   	case GF_INIT:
		FilterColorFrom=FilterColorTo=0;
       	AddLBUTTON = FALSE;  
       	if (!hWndOrthoFilter)
	    {
		  DLGPROC lpfnORTHOFILTERMsgProc;
		  
	      lpfnORTHOFILTERMsgProc = MakeProcInstance((DLGPROC)ORTHOFILTERMsgProc, hInst);
	      CreateDialog(hInst, (LPSTR)"ORTHOFILTER", hWnd, lpfnORTHOFILTERMsgProc);
	    }
   		break;

    case WM_MOUSEMOVE:
		
    	if (!hWndOrthoFilter)
    		break;
		MousePoint =POINTStoPOINT(MAKEPOINTS(lParam));
		Color = GetPixel (CurView->hDC,MousePoint.x,MousePoint.y); 
        CurColorIndex = GetRValue(Color);
		sprintf (str,"%ld %ld %ld",(long)GetRValue(Color),(long)GetGValue(Color),(long)GetBValue(Color)); 
		SetDlgItemText (hWndOrthoFilter,IDC_CVALUE2,str);  
		sprintf (str,"%ld",Color); 
		SetDlgItemText (hWndOrthoFilter,IDC_CVALUE,str);  
		if (GetGlobalCVal ("%ORTHOFILTERMACRO",str,0))
		{
			CurrentPoint = ScreenPtToBasePt (MousePoint);
			ProcessText (str);
		}
		break;
		
	case WM_CHAR:
    {
		switch (wParam)
		{   
			case 'F':
			case 'f': 
			FilterColorFrom = CurColorIndex;
            break;

			case 'T':
			case 't': 
			FilterColorTo = CurColorIndex;
            break;  
            
            default:
               	return FALSE;
		}  
		return TRUE;
	}
    case WM_LBUTTONUP:
    {
		LPLOGPALETTE	plgpl;
		HPALETTE hpal;
		LPPALETTEENTRY lpPalEnt, lpPalEntBeg;
		UINT	i, ic;
		BYTE	red, green, blue; 
		HANDLE	handle; 
		
		if (!hGSPal)
			break;
		handle = GSSiGlobAlloc (1109,GHND,NUMENTRIES * sizeof(PALETTEENTRY));
    	lpPalEnt = lpPalEntBeg = (LPPALETTEENTRY)GlobalLock (handle);
		for (i = 0, red = 0, green = 0, blue = 0; i < NUMENTRIES;
		        i++, red++, green++, blue++)
		{   
			if (i >= min(FilterColorFrom,FilterColorTo) && i <= max(FilterColorFrom,FilterColorTo))
				lpPalEnt->peRed = lpPalEnt->peGreen = lpPalEnt->peBlue = 0;
			else
				lpPalEnt->peRed = lpPalEnt->peGreen = lpPalEnt->peBlue = 255;
			lpPalEnt++->peFlags = PC_RESERVED; 
		}
		AnimatePalette (hGSPal,0,256,lpPalEntBeg);
		GlobalUnlock (handle);
		GSSiGlobUlFree (&handle);  
	}
		break;               
		
    case WM_RBUTTONUP:
    {
		LPLOGPALETTE	plgpl;
		HPALETTE hpal;
		LPPALETTEENTRY lpPalEnt, lpPalEntBeg;
		UINT	i;
		BYTE	red, green, blue; 
		HANDLE	handle; 
		
		if (!hGSPal)
			break;
		handle = GSSiGlobAlloc (1110,GHND,NUMENTRIES * sizeof(PALETTEENTRY));
    	lpPalEnt = lpPalEntBeg = (LPPALETTEENTRY)GlobalLock (handle);
		for (i = 0, red = 0, green = 0, blue = 0; i < NUMENTRIES;
		        i++, red++, green++, blue++)
		{
			lpPalEnt->peRed = lpPalEnt->peGreen = lpPalEnt->peBlue = i;
			lpPalEnt++->peFlags = PC_RESERVED; 
		}
		AnimatePalette (hGSPal,0,256,lpPalEntBeg);
		GlobalUnlock (handle);
		GSSiGlobUlFree (&handle);  
	}
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL HighlightByClass (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1313);
#endif
{
 char key;
 POINT	MousePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP: 
    {
    	DLGPROC lpfnHIGHLIGHTCLASSMsgProc;
    	short	iclass,i; 
    	
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break; 
		DefaultHltClass = -1;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{
			if (PickThemeClass(iclass,MousePoint))
			{
				DefaultHltClass = iclass;    
				break;
			}
		} 
		setDoPaint( FALSE); 
        lpfnHIGHLIGHTCLASSMsgProc = MakeProcInstance((DLGPROC)HIGHLIGHTCLASSMsgProc, hInst);
        DialogBox(hInst, (LPSTR)"HIGHLIGHTCLASS", CurView->hWnd, lpfnHIGHLIGHTCLASSMsgProc);
        FreeProcInstance(lpfnHIGHLIGHTCLASSMsgProc);
		setDoPaint( TRUE);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1313);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1313);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL AddCirclePolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (931);
#endif
{  
 static	BOOL	HaveRP=FALSE;    
 static DPOINT	RadiusPoint; 
 static	HCURSOR	InCursor; 
 long	Radius;  
 POINT	MousePoint;
 
 if (idTimer)
{
#if ENABLETRACE
GSSiExitProg (931);
#endif
 	return FALSE;
}
 switch (Message)
   {
   	case GF_INIT:  
	   	InCursor = CurView->hCursor;
		SetCurs ((HCURSOR)2,FALSE);
   		HaveRP = FALSE; 
   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
   		break;
   		
    case WM_LBUTTONUP: 
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    if (!CursorIsLocked)
	        CurrentPoint=ScreenPtToBasePt(MousePoint); 
	    else
	    	UnlockCursor ();
	    EnlargeScreen (0,0);
    	if (HaveRP)
    	{
			long	nnewpt;

    		if (CurrentDistance == DBL_MAX)                      
    		{   
    			CurrentDistance = ldistp (RadiusPoint,CurrentPoint);
    		} 
    		hNewPolyPoints = CreateCirclePoly (RadiusPoint,CurrentDistance,&nnewpt,CurveChordDist);
			NumNewPolyPoints = nnewpt;
		    PostMessage(hWnd, GF_CLOSE,0, 0L); 
		    break;
    	}
		RadiusPoint = CurrentPoint;
   		SetPrompt (PRMT_LOCATE_RADIUS,TRUE);
   		HaveRP = TRUE;
   		CurrentDistance = DBL_MAX;
		break;
		
    case GF_CLOSE: 
		SetCurs (InCursor,FALSE); 
        return FALSE;
        
    default:
{
#if ENABLETRACE
GSSiExitProg (931);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (931);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AddRectanglePolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (931);
#endif
{  
 static	HCURSOR	InCursor; 
 POINT	MousePoint;  
 HPDPOINT	Points;
 
 if (idTimer)
{
#if ENABLETRACE
GSSiExitProg (931);
#endif
 	return FALSE;
}
 switch (Message)
   {
   	case GF_INIT:  
	   	InCursor = CurView->hCursor;
		SetCurs ((HCURSOR)2,FALSE);
   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
		NumNewPolyPoints = 0;
		AddGraphicsFunction (hWnd, GF_ZOOM_VARRECT,0);
   		break;

    case GF_COMPLETE:
    	hNewPolyPoints = GSSiGlobAlloc ( 752,GMEM_MOVEABLE,sizeof(DPOINT)*4);
    	Points = (HPDPOINT)GlobalLock (hNewPolyPoints);
    	Points[0].x = ZoomBoxRect.xmn; 
    	Points[0].y = ZoomBoxRect.ymn; 
    	Points[1].x = ZoomBoxRect.xmn; 
    	Points[1].y = ZoomBoxRect.ymx; 
    	Points[2].x = ZoomBoxRect.xmx; 
    	Points[2].y = ZoomBoxRect.ymx; 
    	Points[3].x = ZoomBoxRect.xmx; 
    	Points[3].y = ZoomBoxRect.ymn;
    	GlobalUnlock (hNewPolyPoints); 
    	NumNewPolyPoints = 4;
	    PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;
   		
    default:
{
#if ENABLETRACE
GSSiExitProg (931);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (931);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SelectDist (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (935);
#endif
{  
 static	HCURSOR	InCursor; 
 DLGPROC lpfnSELECTDISTMsgProc;  
 short	nRc;
 
 switch (Message)
   {
   	case GF_INIT:  
	   	InCursor = CurView->hCursor;
		SetCurs ((HCURSOR)2,FALSE);
//   		SetPrompt (PRMT_SELECT_DIST);
		lpfnSELECTDISTMsgProc = MakeProcInstance((DLGPROC)SELECTDISTMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"SELECTDIST", hWnd, lpfnSELECTDISTMsgProc);
		FreeProcInstance(lpfnSELECTDISTMsgProc);
	    PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;
   		
    case GF_COMPLETE:
    case GF_DISPLAYMESS: 
//   		SetPrompt (PRMT_SELECT_DIST);
        break;
        
    case GF_CLOSE: 
		SetCurs (InCursor,FALSE); 
        return FALSE;
        
    default:
{
#if ENABLETRACE
GSSiExitProg (935);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (935);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL DragDist (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (936);
#endif
{  
 char key;
 static	BOOL	HaveRP=FALSE;    
 static DPOINT	RadiusPoint; 
 static	HCURSOR	InCursor; 
 long			Radius;  
 POINT	MousePoint;
 
 switch (Message)
   {
   	case GF_INIT:  
	   	InCursor = CurView->hCursor;
		SetCurs ((HCURSOR)2,FALSE);
   		HaveRP = FALSE; 
   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
   		break;
   		
    case WM_LBUTTONUP: 
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	if (HaveRP)
    	{ 
			long	nnewpt;

    		if (CurrentDistance == DBL_MAX)                      
    		{   
    			DPOINT	BasePoint;
    			
    			BasePoint=ScreenPtToBasePt(MousePoint);
    			CurrentDistance = ldistp (RadiusPoint,BasePoint);
    		} 
    		hNewPolyPoints = CreateCirclePoly (RadiusPoint,CurrentDistance,&nnewpt,0);
			NumNewPolyPoints = nnewpt;
		    PostMessage(hWnd, GF_CLOSE,0, 0L); 
		    break;
    	}
	    if (!CursorIsLocked)
	        CurrentPoint=ScreenPtToBasePt(MousePoint);
		RadiusPoint = CurrentPoint;
   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
   		HaveRP = TRUE;
   		CurrentDistance = DBL_MAX;
		break;
		
    case GF_COMPLETE:
    case GF_DISPLAYMESS: 
		if (HaveRP) 
   			SetPrompt (PRMT_LOCATE_RADIUS,TRUE);
	   	else
	   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
        break;
        
    case GF_CLOSE: 
		SetCurs (InCursor,FALSE); 
        return FALSE;
        
    default:
{
#if ENABLETRACE
GSSiExitProg (936);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (936);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL PointsFromHLT (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (952);
#endif
{  
 char key;
 long			Radius;  
 POINT	MousePoint; 
 
 switch (Message)
   {
   	case GF_INIT:  
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
   	
   	case GF_EXECUTE:	
    {   
    	long	EndSeq;  
		BOOL	Closed=FALSE;
    	
    	if (Function == GF_AREA_FROM_HLT || Function == GF_COPY_POLY)
    		Closed = TRUE;
		if ((hNewPolyPoints = GetConnectedItems (Closed,&NumNewPolyPoints)))
		{
			HPDPOINT pPoint=(HPDPOINT)GlobalLock (hNewPolyPoints);
			if (Function != GF_COPY_POLY)
				LockCursor (&pPoint[NumNewPolyPoints-1]);
			GlobalUnlock (hNewPolyPoints);
		}
	    PostMessage(hWnd, GF_CLOSE,Function, 0L);
	} 
		break;
    default:
{
#if ENABLETRACE
GSSiExitProg (952);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (952);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL WheelZoom (int inc,int From,double Scale)//if inc == -1 returns TRUE if have timer
{
	static long		ScreenID;
	static	short	VPID;
	int		ZoomFactor=32*Scale;
	int				WheelZoomTimeout=1000,n;
	static	int		TimerID=0;
	static	int		Count;
	char			str[64];
	static	double			OrigScale;
	int	OutWidth;  
	int	OutHeight; 
	RECT	BMRect;
	HDC		hdcMemMap;
	HBITMAP	hBmpOld;
	MSG		msg;
	static	HBITMAP	hBmpScreen=0;
 
	if (GetGlobalBVal2("[%DISABLEWHEEL]", FALSE))
		return FALSE;
	if (inc == -1)
		return TimerID;
	if (!CurView)
		return FALSE;
	if (isTouchScreen)
		Scale *= 2;
	HaltMapDisplay (FALSE,FALSE);
	switch (inc)
	{
	case INT_MAX:
	case 0:
 		if (hBmpScreen)
		{
			SetViewport (VPID);
    		KillTimer (CurView->hWnd,TimerID);
			TimerID = 0;
			n = 0;
			while (n<5 && GSSiPeekMessage(&msg, CurView->hWnd,WM_TIMER,WM_TIMER, PM_REMOVE))
				n++;
   			GSSiDeleteObject (&hBmpScreen);
			BlockVehicleDisplay = 0;
			if (!inc)
				ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);
		}
		break;
	default:
		if (!hBmpScreen)
		{
			VPID = CurView->ID;
			SetDisplayMode (CurView->hDC, GF_SCREENMODE);
			if (BufferedScreen && hDCScreenBuffer)
				hBmpScreen = SaveScreen (hDCScreenBuffer,CurView->ScreenRect);
			else
				hBmpScreen = SaveScreen (CurView->hDC,CurView->ScreenRect);
			if (!From)
				TimerID = SetTimer(CurView->hWnd, GF_WHEELZOOM,WheelZoomTimeout, (TIMERPROC) 0);
			OrigScale = CurView->Scale;
			Count = 0;
		}
		SetViewport (VPID);
		hdcMemMap=CreateCompatibleDC(CurView->hDC);
        hBmpOld = SelectObject(hdcMemMap,hBmpScreen); 
		{   
			RECT	VPRect=CurView->ScreenRect, BMRect;
			HDC		hDC = GetDC(CurView->hWnd);

			if (!From)
				KillTimer (CurView->hWnd,TimerID);
			TimerID = 0;
			n = 0;
			while (n<5 && GSSiPeekMessage(&msg, CurView->hWnd,WM_TIMER,WM_TIMER, PM_REMOVE))
				n++;
			if (inc > 0)
				CurView->Scale -= CurView->Scale/ZoomFactor;
			else
				CurView->Scale += CurView->Scale/ZoomFactor;
			OutWidth  = CurView->Rect.right-CurView->Rect.left+1;
			OutHeight = CurView->Rect.bottom-CurView->Rect.top+1;
			BMRect.left = BMRect.top = 0;
			BMRect.right = OutWidth;
			BMRect.bottom = OutHeight;
			if (CurView->Scale > OrigScale)
			{
				OutWidth *= OrigScale/CurView->Scale;
				OutHeight *= OrigScale/CurView->Scale;
				VPRect = FactorRect (&VPRect,OrigScale/CurView->Scale);
				pZoomOutExclusionArea = &VPRect;
			}
			else
			{
				BMRect = FactorRect (&BMRect,CurView->Scale / OrigScale);
			}
			GSSiDeleteObject(&CurView->hRgn);  
			CurView->hRgn = CreateVPRgn(FALSE,FALSE);
			pZoomOutExclusionArea = 0;
			SelectClipRgn (hDC,CurView->hRgn);
			GSSiDeleteObject(&CurView->hRgn);  
			if (inc < 0)
			{
				HBRUSH hBrush	 = CreateSolidBrush (CurView->BackGroundColor);
				HBRUSH hOldBrush = SelectObject (hDC,hBrush);

				FillRect (hDC,&CurView->ScreenRect,hBrush);
				SelectObject (hDC,hOldBrush);
				GSSiDeleteObject (&hBrush);
				CurView->hRgn = CreateVPRgn(FALSE,FALSE);
				SelectClipRgn (hDC,CurView->hRgn);
				GSSiDeleteObject(&CurView->hRgn);  
			}
			SetStretchBltMode(hDC,COLORONCOLOR);
			StretchBlt (hDC,
									 VPRect.left,VPRect.top,
									 OutWidth,
									 OutHeight,  
						hdcMemMap,   
									 BMRect.left,BMRect.top,
									 BMRect.right-BMRect.left+1,
									 BMRect.bottom-BMRect.top+1,
									 SRCCOPY);  
			if (!From)
				TimerID = SetTimer(CurView->hWnd, GF_WHEELZOOM,WheelZoomTimeout, (TIMERPROC) 0);
			BlockVehicleDisplay = 1;
			ReleaseDC(CurView->hWnd, hDC);
		}
		SelectObject(hdcMemMap,hBmpOld); 
		DeleteDC (hdcMemMap);
		break;
	}

	return FALSE;
}

BOOL MoveScreen (int xinc,int yinc,HANDLE hSavedScreen,int ScreenID)
{   
    RECT	SaveRect;
	HRGN	NewRgn,DiffRgn;
	int		TypeRegion;
	LPSAVESCREEN	pSavedScreen;
	HDC		savehDC = CurView->hDC;
    
	if (!xinc && !yinc)
		return FALSE;
    if (!ScreenIsRegistered(hSavedScreen,ScreenID))
		return FALSE;
	CurView->hDC = GetDC(CurView->hWnd);
	pSavedScreen = (LPSAVESCREEN)GlobalLock (hSavedScreen);  
	SaveRect = pSavedScreen->Rect;  
	pSavedScreen->Rect.left+=xinc;
	pSavedScreen->Rect.right+=xinc;
	pSavedScreen->Rect.top+=yinc;
	pSavedScreen->Rect.bottom+=yinc;
    SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	NewRgn = CreateRectRgnIndirect (&pSavedScreen->Rect); 
	TypeRegion = CombineRgn (CurView->hRgn,CurView->hRgn,NewRgn,RGN_DIFF);
	DeleteObject (NewRgn);

	GlobalUnlock (hSavedScreen); 
	SelectClipRgn(CurView->hDC, CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn);
	//if (GetGlobalBVal2 ("[%SLIDESCREENCLEAR]",FALSE
	//FillRectPoly (CurView->hDC,&CurView->DrawRect,CurView->BackGroundColor);
	FillRect (CurView->hDC,&CurView->ScreenRect,GetStockObject(WHITE_BRUSH));
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	SelectClipRgn (CurView->hDC,CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn);  
	RestoreScreen2 (CurView->hDC,hSavedScreen,ScreenID,TRUE);
	pSavedScreen = (LPSAVESCREEN)GlobalLock (hSavedScreen);
	pSavedScreen->Rect = SaveRect;    
   	GlobalUnlock (hSavedScreen);
	ReleaseDC(CurView->hWnd, CurView->hDC);
	CurView->hDC = savehDC;
	return TRUE;
}

BOOL SlideScreen (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (965);
#endif
{
	static HANDLE	hSavedScreen=0; 
	static long		ScreenID; 
	static POINT	DownPoint;
	static BOOL		HaveDownPoint;  
	DPOINT	BasePointDown, BasePointUp;
	static	POINT  MousePoint;
	LPSAVESCREEN	pSavedScreen;  
	double	xmove, ymove; 
	static	short  xinc, yinc, i; 
	short	ii,rtn=1;
 
 switch (Message)
   {
    case GF_INIT:
		SetDisplayMode (CurView->hDC, GF_SCREENMODE);
		if (BufferedScreen && hDCScreenBuffer)
			hSavedScreen = SaveScreen2 (CurView->hWnd,hDCScreenBuffer,CurView->ScreenRect,CurView,&ScreenID); 
		else
			hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,CurView->ScreenRect,CurView,&ScreenID);
		HaveDownPoint = FALSE;
   		SetPrompt (PRMT_SLIDE1,TRUE);  
        SetCurs (LoadCursor (hInst,"HANDMOVE1"),FALSE);
        break;

    case GF_REDRAW:
    	DestroySavedScreen (&hSavedScreen,ScreenID);
		HaveDownPoint = FALSE;

    	break;
    case WM_MBUTTONUP:
    case WM_LBUTTONUP:
//        MousePoint = MAKEPOINT(lParam);
   		SetPrompt (PRMT_SLIDE1,TRUE);  
        SetCurs (LoadCursor (hInst,"HANDMOVE1"),FALSE); 
        BasePointDown = ScreenPtToBasePt (DownPoint);
        BasePointUp = ScreenPtToBasePt (MousePoint); 
        xmove = BasePointDown.x - BasePointUp.x;
        ymove = BasePointDown.y - BasePointUp.y;
        CurView->MidPointW.x += xmove;
        CurView->MidPointW.y += ymove;
    	DestroySavedScreen (&hSavedScreen,ScreenID);
		HaveDownPoint = FALSE;
		ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);
		rtn = GF_INCREASE_SUCCESS_COUNT; 
		if (Message == WM_MBUTTONUP)
			 PostMessage(hWnd,GF_CLOSE, 0,0); 
        break;
    
    case WM_LBUTTONDOWN:
    	DownPoint = POINTStoPOINT(MAKEPOINTS(lParam));
        MousePoint = DownPoint;
        HaveDownPoint = TRUE;
		if (BufferedScreen && hDCScreenBuffer)
			hSavedScreen = SaveScreen2 (CurView->hWnd,hDCScreenBuffer,CurView->ScreenRect,CurView,&ScreenID); 
		else
			hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,CurView->ScreenRect,CurView,&ScreenID);
   		SetPrompt (PRMT_SLIDE2,TRUE);  
        SetCurs (LoadCursor (hInst,"HANDMOVE2"),FALSE);
        break;
    
    case WM_MOUSEMOVE:
    	if (wParam != MK_LBUTTON && wParam != MK_MBUTTON)
    		break; 
        MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	
        if (ScreenIsRegistered(hSavedScreen,ScreenID))
        {   
        	
        	if (HaveDownPoint)
        	{   
        		xinc = MousePoint.x - DownPoint.x;
        		yinc = MousePoint.y - DownPoint.y;
				MoveScreen (xinc,yinc,hSavedScreen,ScreenID);
			}
        }
        else
        	ii=1;
        break;
    
    case GF_CLOSE:
    case GF_CANCEL:
    	DestroySavedScreen (&hSavedScreen,ScreenID);
        return FALSE;

    default:
{
#if ENABLETRACE
GSSiExitProg (965);
#endif
        return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (965);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}
BOOL TraverseEntry (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (157);
#endif
{HDC hDC;
 char key; 
 long	LastPicked=LONG_MAX;
 POINT	MousePoint;
 DPOINT	BasePoint;

 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,0, 0L);
   		break;

	case GF_EXECUTE:
    	HaveTravStartPoint = CursorIsLocked;
    	if (HaveTravStartPoint) 
    	{
    		TravStartPoint = CurrentPoint;
    		UnlockCursor ();
    	}  
		if (!hWndTraverseEntry)
		{
		                  
		  lpfnTRAVERSE_ENTRYMsgProc = MakeProcInstance((DLGPROC)TRAVERSE_ENTRYMsgProc, hInst);
		  hWndTraverseEntry=CreateDialog(hInst,"TRAVERSE_ENTRY",hWndMain, lpfnTRAVERSE_ENTRYMsgProc);
		}
		else
			ShowWindow (hWndTraverseEntry,SW_RESTORE);

		SendMessage(hWnd, GF_CLOSE,GF_TRAVERSE_ENTRY, CurView->ID);
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (157);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (157);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL RotateSymbols (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{ 
	static	BOOL HaveLocPoint; 
	DPOINT	AZPoint; 
	POINT	MousePoint;  
	static	POINT	DragPoints[2]; 
	static	long	SkipRef;  
	short	rtn=FALSE, OldMode;
	char	key;
   	char	str[64];
   	long	TotNum;  
   	short	pos=BT_FIRST;
	long	Refno;
	static	BOOL	UsePick;
	HIGHLIGHTDATA	HighlightData;
   	
	
 switch (Message)
   {
   	case GF_INIT: 
   		HaveLocPoint = FALSE; 
       	UsePick = FALSE;     
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
	    TotNum = BT_NUM_IN_INDEX (hHighlight2);
	    if (!TotNum)
	    {
			MessageBox( GetFocus(), "No items highlighted",0, MB_OK|MB_ICONEXCLAMATION);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
	     	break;
	    } 
		AddLBUTTON = FALSE; 
		SetPrompt (PRMT_ROTATE_SYMBOLS,TRUE); 
		SkipRef = LONG_MAX;
        break;
   	
   	case GF_REDRAW:
   		if (!HaveLocPoint)
   			break;
		DragPoints[0]=BasePtToScreenPt (&CurrentPoint);
   		break;
   		
	case GF_EXECUTE:  
	{   
		BOOL	ShowStatus = FALSE; 
		long	n = 0; 
		HCURSOR	hcurSave;
		
	    TotNum = BT_NUM_IN_INDEX (hHighlight); 
	    if (TotNum > 10)
	    	ShowStatus = TRUE;
		if (ShowStatus)
			CreateStatusWind (CurView->hWnd,1,"Rotating map data");
	    else 
	    {
			hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
		}
		pos = BT_FIRST;	
		while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
		{   
			n++;
			pos = BT_NEXT;  
			PickList[0] = HighlightData.PD; 
			UpdateItem = 201;
			if (Refno != SkipRef &&
			    (SysTypeFromPickType(PickList[0].Type) == GF_POINT || SysTypeFromPickType(PickList[0].Type) == GF_TEXT)) 
				UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,1,-1);
			if (ShowStatus)
				StatusWindowUpdate (0,0, TotNum, n);
	    } 
	    UpdateItem = 0;  
	    
	    ContinueProcessing = TRUE; 
	    CloseChronoIndex ();
		if (ShowStatus)
			DestroyStatusWindow (0);
		else
		{
			GSSiSetCursor(hcurSave);
		} 
	}   
		PointRotOpt = 0; 
		ClearHighlightList (FALSE);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;
		
   	case WM_MOUSEMOVE:  
   		if (!HaveLocPoint)
   			break; 
   		
    	MousePoint=POINTStoPOINT(MAKEPOINTS(lParam));
    	if (idist (DragPoints[0],MousePoint) < 5)
    		break;  
		OldMode = SetROP2(CurView->hDC,R2_NOT); 
		TempPolyline (CurView->hDC,DragPoints,2,0,0);
    	DragPoints[1] = MousePoint;
		TempPolyline (CurView->hDC,DragPoints,2,0,0);
    	SetROP2(CurView->hDC,OldMode);
   		
   		break;
    case WM_LBUTTONDOWN:
    {
    	
		SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    if (!CursorIsLocked)
	    { 
	        CurrentPoint=ScreenPtToBasePt(MousePoint);
	    }
	    DragPoints[0]=DragPoints[1]=MousePoint;
    	HaveLocPoint = TRUE;
    }
		break;
		
    case WM_LBUTTONUP:
    	if (HaveLocPoint)
    	{   
    		DPOINT	Point1,Point2;
    		
		    rtn = TRUE;
		    HaveLocPoint = FALSE; 
			OldMode = SetROP2(CurView->hDC,R2_NOT); 
			TempPolyline (CurView->hDC,DragPoints,2,0,0);
	    	SetROP2(CurView->hDC,OldMode);
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    	Point1 = ScreenPtToBasePt (DragPoints[0]);
	        Point2 = ScreenPtToBasePt(MousePoint); 
	        PointRotValue = getazd (&Point1,&Point2);
	        PointRotOpt = 1;
	    	UnlockCursor ();
		    EnlargeScreen (0,0);
			PostMessage(hWnd, GF_EXECUTE,0, 0L); 
        }
 
        break; 
        
    case WM_CHAR:
		key = wParam;
		switch (key)
		{
			case 'r':
			case 'R'://reverse rotation
				PointRotOpt = 2;
				PostMessage(hWnd, GF_EXECUTE,0, 0L); 
			break;
			case 's':
			case 'S'://set rotation 
				*str = 0;
				if (!GetTextString (hWnd,str,2,"Enter new rotation (use + or - increment current rotation)",0,0,0,TRUE,TRUE))
					return FALSE;
				if (*str == '+' || *str == '-')
					PointRotOpt = 3;
				else 
					PointRotOpt = 1;
	        	PointRotValue = atof (str) * RADDEG;
				PostMessage(hWnd, GF_EXECUTE,0, 0L); 
			break;  
			case 'L':
			case 'l':
				pos = BT_LAST; 
			case 'F':
			case 'f': 
			{
				long	Sequence;
				
				PointRotOpt = 1;  
				BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&Refno);
				BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);  
				SkipRef= Refno;
				PointRotValue = HighlightData.PD.PPAZ;
				PostMessage(hWnd, GF_EXECUTE,0, 0L);   
			}
			break;
			default:
				return FALSE;		
				
		}
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL EditSymbol (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1052);
#endif
{
 char key;
 short	rtn=1;
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;  
{
#if ENABLETRACE
GSSiExitProg (1052);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

        if (Message == GF_EXECUTE)
        	Item = 0;
        else
        {   
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		    PickItems (hWnd,BasePoint);
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
		}
		ProcessPickedItem (Item,FALSE);        		
       	if (CurSymSizeLoc || CurSymSizeLocD)
       	{   
       		BOOL	SaveDisableHalt;  
       		LPDOUBLE pSizeD;
       		LPFLOAT	 pSize;  
       		double	SizeFactor = GetGlobalDVal2 ("[%SIZECHANGEFACTOR]",1.0); 
       		long	Loc=max (CurSymSizeLoc,CurSymSizeLocD);
       		
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
			Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{   
				short	l;
				
				GSSillseek (Fid,Loc,0);
				BigRead (Fid,(HPSTR)&ID,2);
				if (ID == 20 && Function == GF_FACTOR_SYMBOL)
				{   
					float OldSize;
					
					BigRead (Fid,(HPSTR)&OldSize,sizeof(OldSize));
					OldSize *= SizeFactor;
					GSSillseek (Fid,Loc+2,0);
					BigWrite (Fid,(HPSTR)&OldSize,sizeof(OldSize),-1);
				}
				else if (ID == 120 && Function == GF_FACTOR_SYMBOL)
				{   
					double OldSize;
					
					BigRead (Fid,(HPSTR)&OldSize,sizeof(OldSize));
					OldSize *= SizeFactor;
					GSSillseek (Fid,Loc+2,0);
					BigWrite (Fid,(HPSTR)&OldSize,sizeof(OldSize),-1);
				}
				GSSiClose (Fid);  
				rtn = GF_INCREASE_SUCCESS_COUNT; 
			}
        	DisableHalt = SaveDisableHalt;
       	}
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1052);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1052);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL PointsFromOffset (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (953);
#endif
{  
 LPSTR	pFile;
 POINT	MousePoint;
 
 switch (Message)
   {
   	case GF_INIT: 
   		GSSiGlobFree (&hNewPolyPoints);
   		NumNewPolyPoints = 0;
	   	if (!hAreaOffFile)
{
#if ENABLETRACE
GSSiExitProg (953);
#endif
	   		return FALSE;
}
		pFile = GlobalLock (hAreaOffFile); 
	    if (!ExistFile(pFile))  
	    {	
	    	GlobalUnlock (hAreaOffFile);
{
#if ENABLETRACE
GSSiExitProg (953);
#endif
	    	return FALSE;               
}
	    }
    	GlobalUnlock (hAreaOffFile);
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
   	
   	case GF_EXECUTE:	
    {   
		int nRc, Type, nPoly;
		double	Offset;
    
		hNewPolyPoints = GetNextHighlightArea (1,0,&Type,&NumNewPolyPoints,&nPoly,0,&Offset,0);
	    PostMessage(hWnd, GF_CLOSE,0, 0L);
	} 
		break;
    default:
{
#if ENABLETRACE
GSSiExitProg (953);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (953);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL PointsFromLinfit (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1080);
#endif
{  
 char key;
 long			Radius;  
 POINT	MousePoint;
 
 switch (Message)
   {
   	case GF_INIT:  
	    if (!TotHLTPoints)
{
#if ENABLETRACE
GSSiExitProg (1080);
#endif
	    	return FALSE;
}
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
   	
   	case GF_EXECUTE:	
    {   
		hNewPolyPoints = LinfitHLTItems (&NumNewPolyPoints,Function);
	    PostMessage(hWnd, GF_CLOSE,0, 0L);
	} 
		break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1080);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1080);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SnapToIntersection (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{HDC hDC;
 char key;     
 int	st;     
 long	npnts[2],Refno; 
 BOOL	rtn;
 HPDPOINT	pPolyPoints[2];
 HANDLE	hPoly[2];
 HIGHLIGHTDATA	HighlightData;
 POINTS	MousePoint;
 POINT CursorPoint;
 DPOINT	BasePoint, IntPoint;   
 LPTHEME	pTheme;   
 static	double	ExtendDist=10000; 
 double	D1,D2, Extd[2];
 double	AZ; 
 short	pos,i, type[2]; 
 static	BOOL	HaveFirstItem;
 static	long	FirstItem;  
 static	HANDLE	SaveH1, SaveH2; 
 static	DPOINT	OrigSnapPos;
 static	BOOL	CursorWasSnapped;

 switch (Message)
   {
        
   	case GF_INIT:
   		if (Function == GF_SPLIT_AT_INTERSECTION)
   		{
   			if (!CurView->UpdateFile)
	   		{
		 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	            break;
	   		} 
	   		else
	   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	    }
		if (CurView)
			ExtendDist = ((CurView->WBounds.xmx - CurView->WBounds.xmn) + (CurView->WBounds.ymx - CurView->WBounds.ymn))/2;
        SetPrompt (PRMT_SNAPINT1,TRUE);     
		OrigSnapPos = CurrentPoint;
		CursorWasSnapped = CursorIsLocked;  
        UnlockCursor ();  
        EnlargeScreen (0,0);
		SetPickCursor (TRUE);
       	AddLBUTTON = TRUE;   
       	HaveFirstItem = FALSE;
       	SaveHighlightList (&SaveH1,&SaveH2); 

   		break;
    
    case GF_CANCEL:
    	if (CursorWasSnapped)
    		LockCursor (&OrigSnapPos);	
    	return FALSE;
    	
    case GF_CLOSE:
		ClearHighlightList (FALSE);
    	RestoreHighlightList (SaveH1, SaveH2);
    	return FALSE; 
    	
    case WM_LBUTTONUP:
    	MousePoint = MAKEPOINTS (lParam);
	    BasePoint=WinPtSToBasePt(MousePoint);
		{
			BOOL	SavePTXT = PickText;  
            
            PickText = FALSE;
	    	PickItems (hWnd,BasePoint);
	    	PickText = SavePTXT;
	    }
	    if (!NumPicked)
	    	return FALSE;
    	NumPicked--;
	    AddToHighlightList (PickList[NumPicked].Refno,&PickList[NumPicked],TRUE);
    	ShowPickedItem (hWndMain,NumPicked); 
   		if (!HaveFirstItem)
   		{
   			HaveFirstItem = TRUE;
   			FirstItem = PickList[NumPicked].Refno; 
	        SetPrompt (PRMT_SNAPINT2,TRUE);   
			SetPickCursor (TRUE);
   			break;
   		}
		SetPickCursor (FALSE);
		i=0; 
		goto GetHLT;
    case WM_RBUTTONUP:   
    	if (!HaveFirstItem)
    		break;
		i=1; 
		type[0] = 2;
		npnts[0] = 2;
        hPoly[0] = GSSiGlobAlloc ( 615,GMEM_MOVEABLE,npnts[0]*sizeof(DPOINT));
        pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]); 
        *pPolyPoints[0]++ = TrackLineBegin;
        *pPolyPoints[0] = TrackLineEnd;
        GlobalUnlock (hPoly[0]);
GetHLT:
		pos = BT_FIRST; 
	   	while (i<2 && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
	   	{
	   		pos=BT_NEXT;
	   		PickList[0]=HighlightData.PD;
   		    type[i] = PickList[0].Type;
			if (GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts[i],&hPoly[i],TRUE))
				i++;
        }
		if (i != 2)
		{
			while (i--)
				GSSiGlobFree (&hPoly[i]);
			break;
		} 
		pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]);
		pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);  
		Extd[0] = Extd[1] = 0;//pPolyPoints[0][0] pPolyPoints[0][1] pPolyPoints[0][2] pPolyPoints2[0] pPolyPoints2[1]
   		if (type[0] == 2)   
   		{
   			Extd[0] = ExtendDist;
   			ExtendPoly (npnts[0],pPolyPoints[0],ExtendDist);
   		}
   		if (type[1] == 2)  
   		{
   			Extd[1] = ExtendDist;
   			ExtendPoly (npnts[1],pPolyPoints[1],ExtendDist);
   		}
		rtn = IntersectPolys1 (type[0],type[1],npnts[0],pPolyPoints[0],0,npnts[1],pPolyPoints[1],0,0,&PickPointBase,&IntPoint,&D1,&D2,0);
		GSSiGlobUlFree (&hPoly[0]);
		GSSiGlobUlFree (&hPoly[1]); 
		
		pos = BT_FIRST; 
		i=0;
	   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
	   	{
	   		pos=BT_NEXT;
	   		PickList[i]=HighlightData.PD;
		    RemoveFromHighlightList (PickList[i].Refno,2);
	    	ShowPickedItem (hWndMain,i); 
		    RemoveFromHighlightList (PickList[i].Refno,0);
	    	ShowPickedItem (hWndMain,i++); 
		}
   		ClearHighlightList(FALSE);
		if (rtn)
		{   
			if (Function == GF_SPLIT_AT_INTERSECTION)
			{   
				long	Newref1, Newref2;
				
				PickList[0].PCT = (D1 - Extd[0]) / PickList[0].Length;
				PickList[1].PCT = (D2 - Extd[1]) / PickList[1].Length;  
				PickList[0].PickedPoint = IntPoint;
				PickList[1].PickedPoint = IntPoint;
				if (D1 > Extd[0] - P_TOL && D1 < PickList[0].Length + Extd[0] + P_TOL)
					SplitPoly (0,&Newref1,&Newref2);
				if (D2 > Extd[1] - P_TOL && D2 < PickList[1].Length + Extd[1] + P_TOL)
					SplitPoly (1,&Newref1,&Newref2);
            }
            else
            {
				LockCursor (&IntPoint);
				if (!PtInWBounds (&CurrentPoint) && AutoPan)
		            CenterWindow (CurrentPoint,TRUE);
		    	CursorPoint = BasePtToScreenPt (&CurrentPoint);
		        ClientToScreen (CurView->hWnd,(LPPOINT)&CursorPoint);
				CreateDigCursor (CurView->hDC); 
				SetCurs ((HCURSOR)2,FALSE); 
		    	SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
				DisplayCoordinate2 (&CurrentPoint,CursorPoint); 
			}
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		}
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL RelocatePoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1060);
#endif
{
 char key;
 short	rtn=1; 
 static	BOOL	HaveFirstPoint, DoTrack=TRUE, UsePick; 
 static	PICKDATA	PickData;
 static	POINT	TrackPoints[5], LastPoint; 
 static	DPOINT	OrigMidPoint, OrigPoint;  
 static	HANDLE	hSaveScreen;    
 static	double	BoundsOffLeft, BoundsOffRight, BoundsOffTop, BoundsOffBottom;
 DPOINT	NewMidPoint; 
 POINT	MousePoint;   
 MNMXCORD	Bounds; 
 RECT	Rect;
 
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE; 
       	HaveFirstPoint = FALSE; 
       	SetEditName (FALSE); 
       	UsePick = FALSE;     
       	hSaveScreen = 0;
   		break;

    case GF_CLOSE: 
    	if (HaveFirstPoint && DoTrack) 
//			NotPolyline (CurView->hDC,TrackPoints,5,0,0);    
			RestoreScreen2 (CurView->hDC, hSaveScreen,0,FALSE);
    	DestroySavedScreen (&hSaveScreen,0);
{
#if ENABLETRACE
GSSiExitProg (1060);
#endif
    	return FALSE;
}
    
    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	NewPoint;
		DPOINT	BasePoint;
   		BOOL	SaveDisableHalt, NewPointInOldQuad=FALSE;  
    	short	ID;  
    	double	XMove, YMove;

    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    goto Next;
	case GF_USEPICKED:
		UsePick=TRUE;
Next:
        if (!HaveFirstPoint)
        {   
        	if (Message == WM_LBUTTONUP)
		    	PickItems (hWnd,BasePoint);
		    else
		    	NumPicked = 1;
		    while (NumPicked--)
		    {
		    	if (PickList[NumPicked].Type == 1 || PickList[NumPicked].Type == 4)
		    		goto HavePoint;
		    }
    		break;
HavePoint:	
			GetTypeBounds (NumPicked,&Bounds,-1);
			BoundsOffLeft = PickList[NumPicked].BeginPoint.x - Bounds.xmn;	      
			BoundsOffRight = Bounds.xmx - PickList[NumPicked].BeginPoint.x;	      
			BoundsOffTop = Bounds.ymx - PickList[NumPicked].BeginPoint.y;	      
			BoundsOffBottom = PickList[NumPicked].BeginPoint.y - Bounds.ymn;	      
		    PickData = PickList[NumPicked];
		    BoundsToScreenPoly (&Bounds,TrackPoints,2); 
		    OrigMidPoint = MinMaxMidPointD (&Bounds); 
		    if (UsePick)
		    {   
		    	POINT	ScreenPoint = BasePtToScreenPt (&OrigMidPoint);  
		    	
		    	MousePoint = ScreenPoint;
				ClientToScreen (hWnd,&ScreenPoint);  
				if (!CursorIsLocked)
		 			SetCursorPosGM (ScreenPoint.x,ScreenPoint.y,0);
		    }
		    TrackPoints[4] = TrackPoints[0];
//		    if (DoTrack)
//    			NotPolyline (CurView->hDC,TrackPoints,5,0,0);
	    	LastPoint = MousePoint;
		    HaveFirstPoint = TRUE; 
		    break;
		} 
 		if (CursorIsLocked)  
	    	UnlockCursor ();   
	    EnlargeScreen (0,0);
		ScreenPolyToBounds (&Bounds,TrackPoints,4);
/*		NewMidPoint = MinMaxMidPointD (&Bounds); 
		XMove = NewMidPoint.x - OrigMidPoint.x;
		YMove = NewMidPoint.y - OrigMidPoint.y; 
		NewPointD = PickData.BeginPoint;
		NewPointD.x += XMove;
		NewPointD.y += YMove;*/  
		NewPointD = BasePoint;
		PickList[0] = PickData; 
        GetPickName (0);   
       	SaveDisableHalt = DisableHalt;
   	    DisableHalt = TRUE;
		if (!CurView->UpdateFile || !_fstricmp (EditName,PickName))
		{
			ProcessPickedItem (0,FALSE);        		
	       	if (CurSymSizeLoc || CurSymSizeLocD)
	       	{   
	       		if (NewPointInOldQuad)
	       		{
		       		LPDOUBLE pSizeD;
		       		LPFLOAT	 pSize;  
		       		long	Loc=max (CurSymSizeLoc,CurSymSizeLocD);
		       		
					if (OpenMap (CurView->hWnd,0))
					{   
						short	l;
						
						GSSillseek (FidMap,Loc,0);
						BigRead (FidMap,(HPSTR)&ID,2);
						if (ID == 20)
						{   
		    				NewPoint = BasePtToFilePt (NewPointD);
							GSSillseek (FidMap,Loc+2+4+4,0);
							BigWrite (FidMap,(HPSTR)&NewPoint,sizeof(NewPoint),-1);
						}
						else if (ID == 120)
						{                
							GSSillseek (FidMap,Loc+2+8+8,0);
							BigWrite (FidMap,(HPSTR)&NewPointD,sizeof(NewPointD),-1);
						}
						GSSiClose (FidMap); 
					}
				}
		       	else
		       	{
					UpdateItem=20;
					UpdateRecord (0,PickList[0].Desc,0,0,0,0,0,0);
			    }
	       	}
	    } 
       	else
       	{
			UpdateItem=20;
			UpdateRecord (0,PickList[0].Desc,0,0,0,0,1,0);
	    }
       	DisableHalt = SaveDisableHalt;
       	HaveFirstPoint = FALSE;
	    ShowPickedItem (hWnd,0); 
//		if (DoTrack) 
//			NotPolyline (CurView->hDC,TrackPoints,5,0,0);
		if (UsePick)
			PostMessage(hWnd, GF_CLOSE,0, 0L);
		rtn = GF_INCREASE_SUCCESS_COUNT; 
       	
    }
    	break; 
    	
    case WM_MOUSEMOVE: 
    {
    	short	xmove, ymove, i;
		RECT	IntRect;
    	
    	if (!HaveFirstPoint || !DoTrack) break;
//		NotPolyline (CurView->hDC,TrackPoints,5,0,0);    
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));  
    	NewPointD = ScreenPtToBasePt (MousePoint);
 		if (CursorIsLocked)  
 		 	MousePoint = BasePtToScreenPt(&CurrentPoint);
    	xmove = MousePoint.x - LastPoint.x;
    	ymove = MousePoint.y - LastPoint.y;
    	LastPoint = MousePoint;
    	for (i=0;i<5;i++)
    	{
    		TrackPoints[i].x += xmove;
    		TrackPoints[i].y += ymove;
    	}
//		NotPolyline (CurView->hDC,TrackPoints,5,0,0); 
		RestoreScreen2 (CurView->hDC, hSaveScreen,0,FALSE);
    	DestroySavedScreen (&hSaveScreen,0); 
    	Bounds.xmn = NewPointD.x - BoundsOffLeft;  
    	Bounds.xmx = NewPointD.x + BoundsOffRight;
    	Bounds.ymx = NewPointD.y + BoundsOffTop;
    	Bounds.ymn = NewPointD.y - BoundsOffBottom;
		BoundsToWinRect (&Bounds,&Rect);
		IntersectRect (&IntRect,&Rect,&CurView->ScreenRect);
		hSaveScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,IntRect,0,0);
		HaveNewPoint = TRUE;
		ProcessPickedItem (NumPicked,2);
		HaveNewPoint = FALSE;
	}  
		break;
		
    case WM_CHAR:
		key = wParam; 
		if (lParam & KF_UP)
			break;
		switch (key)
		{   
   			case 20: //CNTL_T 
   				if (HaveFirstPoint && DoTrack) 
					NotPolyline (CurView->hDC,TrackPoints,5,0,0);
   				if (DoTrack) 
   					DoTrack = FALSE;
   				else
   					DoTrack = TRUE;
{
#if ENABLETRACE
GSSiExitProg (1060);
#endif
		        return TRUE;
}
		}

    default:
{
#if ENABLETRACE
GSSiExitProg (1060);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1060);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL TraceSpill (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1064);
#endif
{
 char key;
 short	rtn=1; 
 static	BOOL	HaveFirstPoint, DoTrack=TRUE; 
 static	PICKDATA	PickData;
 static	POINT	TrackPoints[5], LastPoint;  
 POINT	MousePoint;
 
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE; 
       	HaveFirstPoint = FALSE; 
   		break;

    case GF_CLOSE: 
{
#if ENABLETRACE
GSSiExitProg (1064);
#endif
    	return FALSE;
}
    	
    case WM_LBUTTONUP:
    {
		short	i; 
		POINT	NewPoint;
		DPOINT	BasePoint, NextPoint, Points[2];
   		BOOL	SaveDisableHalt, NewPointInOldQuad=FALSE;  
    	short	ID,SurfSym; 
    	double	LenRadial = 100, MaxRadialMove = 50;
    	short	nRadials = 4;
    	HANDLE	hRadials[16];
		LPRADIAL	pRadial; 
		double	StartElev;

    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
    	SurfSym = GetSymbolNum ("OT1");
    	if (!SurfSym)
    		break;
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    if (!GetRadialProfiles (BasePoint,0,SurfSym,nRadials,LenRadial,hRadials))
	    	goto Exit;
	    StartElev = ElevFromRadials (nRadials,hRadials);
	    for (i=0;i<nRadials;i++)
	    {
	    	pRadial = (LPRADIAL)GlobalLock (hRadials[i]);
	    	pRadial->Elev = StartElev;
	    	GlobalUnlock (hRadials[i]);
	    }
NextMove:
	    if (!GetNextRadialPoint (&NextPoint,&StartElev,nRadials,hRadials,MaxRadialMove))
	    	goto Exit;
	    GWPolylineD (CurView->hDC,Points,2,0);
	    if (GetRadialProfiles (BasePoint,&StartElev,SurfSym,nRadials,LenRadial,hRadials))
	    	goto NextMove;
Exit:;
    }
    	break;  
    default:
{
#if ENABLETRACE
GSSiExitProg (1064);
#endif
    	return FALSE;
}
    	
    }
{
#if ENABLETRACE
GSSiExitProg (1064);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AdjustTravPoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
 char key;
 static	HBITMAP	hShowPoint=0;  
 static long	mini;
 short	rtn=1, iSym; 
 static	BOOL	HavePoint, DoTrack=TRUE; 
 static	PICKDATA	PickData;
 static	POINT	TrackPoints[5], LastPoint;  
 POINT	MousePoint;
 DPOINT	BasePoint;    
 HPDPOINT pStatus;
 
 switch (Message)
   {
   	case GF_INIT:     
   		if (!hWndTraverseEntry ||!hSnapStatus || !hSnappedPoints)
   			return FALSE;
       	AddLBUTTON = FALSE; 
       	HavePoint = FALSE;    
   		SetPrompt (PRMT_TRAVADJUST1,TRUE);  
   		break;

   	case GF_REDRAW:
    case GF_CLOSE: 
		DestroySavedScreen (&hShowPoint,0);
    	return FALSE;
    	
    case WM_RBUTTONUP:
    {    
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;
	    	UnlockCursor ();
			MousePoint = BasePtToScreenPt (&BasePoint);
	    } 
	    else
        	BasePoint=ScreenPtToBasePt (MousePoint); 
    	EnlargeScreen (0,0);
   		if (!hWndTraverseEntry ||!hSnapStatus) 
   		{
		    PostMessage(hWnd, GF_CLOSE, 0, 0L); 
			break;
		}
	    pStatus = (HPDPOINT)GlobalLock (hSnapStatus);
	    pStatus[mini] = BasePoint;
	    GlobalUnlock (hSnapStatus);	
        PostMessage(hWndTraverseEntry, WM_COMMAND, IDC_GENERATE, 1L);
		PostMessage(hWnd, GF_CLOSE,0, 0L);
   		SetPrompt (PRMT_TRAVADJUST1,TRUE);  
		HavePoint = FALSE;
    }
    	break;
    	
    case WM_LBUTTONUP:
    {   
    	if (HavePoint)
    		HavePoint = FALSE; 
    	if (!hShowPoint)
    		break;
    	HavePoint = TRUE;   
   		SetPrompt (PRMT_TRAVADJUST2,TRUE);  

    }
    	break;

    case WM_MOUSEMOVE: 
    {
		long	i; 
		DPOINT	BasePoint;
		HPDPOINT	DPoint;
		double	MinDist,d; 
		POINT	WinPoint;
        
   		if (!hWndTraverseEntry ||!hSnapStatus) 
   		{
		    PostMessage(hWnd, GF_CLOSE, 0, 0L); 
			break;
		}
        if (HavePoint)
        	break;
        if (hShowPoint)
        {
	   		HDC	hDC=GetDC(hWnd);
			RestoreScreen2 (hDC, hShowPoint,0,FALSE);
			ReleaseDC (hWnd,hDC); 
			DestroySavedScreen (&hShowPoint,0);
		}
		if (!hSnappedPoints)
			break;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
	    DPoint = (HPDPOINT)GlobalLock (hSnappedPoints);  
	    MinDist = ldistp (BasePoint,DPoint[0]);
	    mini = 0;
	    for (i=1;i<nTravPoints;i++)
	    {
	    	d = ldistp (BasePoint,DPoint[i]);
	    	if (d<MinDist)
	    	{
	    		MinDist=d;
	    		mini = i;
	    	}
	    }  
	    WinPoint = BasePtToScreenPt (&DPoint[mini]);
	    iSym = GetDictSymbolNumber ("CIRCLE");
		hShowPoint = DisplayPointItem2 (CurView->hDC,WinPoint,5,0,iSym) ;
	    GlobalUnlock (hSnappedPoints);
    }
    	break;  
    default:
    	return FALSE;
    	
    }
    return (TRUE);
} 

BOOL CreateAreaAroundPoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{  
 char key;   
 short	rtn=1;
 static short	NewSymbol; 
 static	HCURSOR	InCursor;   
 POINTS	MousePoint;  
 DPOINT	BasePoint;   
 static	HANDLE	hSavedScreen=0;
 static	long	BitmapID;
 
 switch (Message)
   {
   	case GF_INIT:  
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
   		if (!SetAreaSymbol (&NewSymbol,0))
   		{
	 		MessageBox(GetFocus(), "Area symbol not set", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		}  
   		SetPickCursor(TRUE);
       	AddLBUTTON = TRUE;  
       	break;
    
    case WM_LBUTTONUP: 
    {   
		MNMXCORD Bounds; 
    	HPDPOINT	NewPolyPoints, SegPoints;
    	short	SaveMaxPick=MaxPick, st, pos, cond;
    	HANDLE	hInt, hSegData;
    	long	NextSeg=0, Segno, TotSegs=1, StartRef, NumSegPoints;  
    	HANDLE	hSegPoints;
    	double	StartDist, Tol=GetGlobalDVal2 ("[%JUMPDIST]",0);
    	short	Direction,ii;   
		BTVARDESC	BTVar[4];
		char	File[144];  
		INTERSECTIONKEY	IntKey;
		INTERSECTIONDATA	IntData;
		HCURSOR	hcurSave; 
		BOOL	SavePPNT = PickPoints;
		BOOL	SavePTXT = PickText;  
		short	SavePP = PickPerim;
    	
    	MaxSegments = GetGlobalLVal2 ("[%MAXSEGMENTS]",MaxSegments);  
    	MaxSegments = min (MaxSegments,MAX_SEGMENTS);
		if (hSavedScreen)
		{
			RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
	        DestroySavedScreen (&hSavedScreen,BitmapID);
		} 
		NumNewPolyPoints = 0;
		GSSiGlobFree (&hNewPolyPoints);
		UseUserPickAp =FALSE;
		SystemPickAp = 0;
		PickPerim = 1;
		PickPoints = PickText = FALSE;	
		MaxPick=1;  
    	MousePoint = MAKEPOINTS(lParam);
	    BasePoint=WinPtSToBasePt(MousePoint);
	    PickItems (hWnd,BasePoint);  
		UseUserPickAp =TRUE;  
		MaxPick = SaveMaxPick;
		PickPerim = SavePP; 
		PickPoints = SavePPNT;
		PickText = SavePTXT;
        if (!NumPicked)
        	break; 

	
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
		GSSiGetTempFileName (0,"gmi",0,(LPSTR)File);
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_REAL;
		BTVar[1].BT_VARLEN=8;
		BTVar[1].BT_VAROFF=4;
		BTVar[2].BT_VARTYP=BT_REAL;
		BTVar[2].BT_VARLEN=8;
		BTVar[2].BT_VAROFF=12;
		BTVar[3].BT_VARTYP=BT_INTEGER;
		BTVar[3].BT_VARLEN=4;
		BTVar[3].BT_VAROFF=20;
		BT_CREATE (File,sizeof(IntData), FALSE, 4, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hInt= BT_OPEN (File, 0, BT_WRITE, 0); 
   		hNewPolyPoints = GSSiGlobAlloc ( 616,GHND,MAX_DIGPOINTS*(long)sizeof(DPOINT)); 
   		NewPolyPoints = (HPDPOINT)GlobalLock (hNewPolyPoints);
   		hSegData = GSSiGlobAlloc ( 617,GHND,MAX_SEGMENTS*(long)sizeof(SEGMENTDATA)); 
   		SegData = (HPSEGMENTDATA)GlobalLock (hSegData);
        Segno = NextSeg++; 
        StartDist = PickList[0].PCT * PickList[0].Length;
        if (PickList[0].OffDist < 0)
        {
        	StartDist = PickList[0].Length - StartDist;
        	SegData[Segno].Reverse = TRUE;
        }
        else
        	SegData[Segno].Reverse = FALSE;
        SegData[Segno].PD.Refno = PickList[0].Refno;
        SegData[Segno].Length = PickList[0].Length;
        SegData[Segno].FromDist = StartDist;
        SegData[Segno].Bounds = PickList[0].Rect;  
		SegData[Segno].FirstPointID = NumNewPolyPoints;
        SegData[Segno].PD = *(LPPICKDATAHEADER)&PickList[0];
        if (!FindIntersectionsWithRef (Segno,hInt,Tol,
        						  	   SegData,&TotSegs)) break;
NextTurn:        
// Get the first turn for the current segment  
		IntKey.Segno = Segno;
		IntKey.AtDist = -DBL_MAX;
		if (!(st=BT_FIND (hInt,(LPSTR)&IntKey,BT_FIRST,BT_GT,(LPSTR)&IntData)))
		{
			if (IntKey.Segno != Segno)
				st=1;
		}
		if (st) //dead end - back up to previous seg
		{ 
			if (!Segno)
				goto ErrOut;
			IntKey.Segno = SegData[Segno].FromSeg;
			IntKey.AtDist = -DBL_MAX;
			IntKey.NextSeg = -1;
			pos = BT_FIRST;
			cond = BT_GT;
			while (IntKey.NextSeg != Segno)
			{
				BT_FIND (hInt,(LPSTR)&IntKey,pos,cond,(LPSTR)&IntData);
				pos = BT_NEXT;
				cond = BT_ANY;
			} 
			BT_DELETE (hInt,(LPSTR)&IntKey,(LPSTR)&IntData,FALSE);
			Segno = SegData[Segno].FromSeg;
			NumNewPolyPoints = SegData[Segno].FirstPointID;
			goto NextTurn;
		}
		SegData[Segno].FirstPointID = NumNewPolyPoints;
		if (!Segno)
		{
			StartRef = SegData[Segno].PD.Refno;
			StartDist = IntKey.AtDist;
			IntData.OnRef = LONG_MAX; 
		}
		else
		{   
	       	GetPolyPnts (&SegData[Segno].PD,SegData[Segno].Reverse,&NumSegPoints,&hSegPoints,TRUE); 
	       	SegPoints = (HPDPOINT)GlobalLock (hSegPoints); 
	       	NumNewPolyPoints += GetPointsBetweenDist (NumSegPoints,SegPoints,
	       											  SegData[Segno].FromDist,
	       											  SegData[IntKey.NextSeg].AtDist, 
	       											  &NewPolyPoints[NumNewPolyPoints]);
            GSSiGlobUlFree (&hSegPoints);
		}
		NewPolyPoints[NumNewPolyPoints++] = IntData.IntPoint;
		if (IntData.OnRef == StartRef && fabs (IntKey.AtDist - StartDist) < P_TOL)
		{
			goto HaveClosedLoop;
		}      
		Segno = IntKey.NextSeg;
        if (FindIntersectionsWithRef (Segno,hInt,Tol,
        						  	  SegData,&TotSegs))
	        goto NextTurn; 
ErrOut:
	    GSSiMessageBox (0,"Unable to create area",0,MB_ICONEXCLAMATION,0);
        NumNewPolyPoints = 0;
HaveClosedLoop: 
		GSSiSetCursor (hcurSave); 
		GSSiGlobUlFree (&hSegData);
		if (hNewPolyPoints)
			GlobalUnlock (hNewPolyPoints);  
		BT_CLOSEANDDELETE (&hInt);   
		
	case GF_REDRAW:
    	if (NumNewPolyPoints)
    	{   
			HBITMAP hBM = LoadBitmap (hInst,MAKEINTRESOURCE(IDB_BITMAP9));
			HBRUSH	hBrush = CreatePatternBrush (hBM);
			HBRUSH	OldBrush = SelectObject (CurView->hDC,hBrush);
			
		    GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn (FALSE,FALSE);
		  	SelectClipRgn (CurView->hDC,CurView->hRgn);
		  	GSSiDeleteObject(&CurView->hRgn);
	        hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&BitmapID);
			DeleteObject (hBM); 
			NewPolyPoints = (HPDPOINT)GlobalLock (hNewPolyPoints);
			GWPolygonD (CurView->hDC,NewPolyPoints,NumNewPolyPoints,1,0,0,FALSE,TRUE,0);
			GlobalUnlock (hNewPolyPoints);  
			SelectObject (CurView->hDC,OldBrush);
			GSSiDeleteObject (&hBrush);
		}
	}
		break;
		      
    case WM_RBUTTONUP: 
    {   
		if (hSavedScreen)
		{
			RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
	        DestroySavedScreen (&hSavedScreen,BitmapID);
		} 
    	if (!NumNewPolyPoints)
    		break;
    	if (HaveRedefineData)
    	{   
    		HaveRedefineData = FALSE;
		    ClearHighlightList (FALSE); 
       		GSSiGlobUlFree (&hUpdateMultiPolygon);
		    nUpdatePolyPoints = NumNewPolyPoints;
        	hUpdatePoly = hNewPolyPoints;
        	NumNewPolyPoints=0;
        	hNewPolyPoints=0; 
			PickList[0] = RedefineData;
        	if (PickList[0].Type == 2)
				UpdateItem=61;   
        	else
				UpdateItem=51;   
			UpdateRecord (0,PickList[0].Desc,0,0,0,0,1,-1);
			GSSiGlobFree (&hUpdatePoly);  
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
        }  
    	else
    	{   
    		LPDPOINT	lpDPoints;
    		long		NewRefno; 
    		int			st;  
    		char		Prefix[128],UDI[128],SaveUDI[66];
    		LPSTR		pStuff=0; 
    		HANDLE		hStuff=0;  
    		BOOL		status, Redefine=FALSE;   
    		char		SymName[34]; 
    		COLORREF	NewLineColor, NewAreaColor;
    		float		NewLineWidth;
    		short		NewLineSymbol, NewAreaSymbol; 
    		HANDLE		hText=0, hTextTPL=0;
		 
    		if (!EditName[0]) break;
    		if (!AutoAreaID(NumNewPolyPoints,hNewPolyPoints,SaveUDI))
    			break;
    		_fstrcpy (PltName,EditName);
    		PltType = 2;
//			GetTAGForSymbol ("PEN1",Prefix,UDI); 
			{   
				LPVIEWPORT SaveVP=CurView; 
				
				NewRefno = GetNewRefnoNoUpdate (EditName);
				if (!SetNewMacro (3))
					goto ExitLine;
				if (!SetAreaSymbol (&NewAreaSymbol,0))
					goto ExitLine;
				if (!SetAreaColor (&NewAreaColor,0))
					goto ExitLine;   
				_fstrcpy (Prefix,"[%NEW_AREA_PREFIX]");
				_fstrcpy (UDI,"[%NEW_AREA_UDI]"); 
				ExpandText (Prefix);
				ExpandText (UDI);
				SetCurView (SaveVP);
				NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewAreaSymbol,&Redefine);
				if (!SetNewStuff (3,&hStuff))
					goto ExitLine;
				if (hStuff)
					pStuff = GlobalLock (hStuff); 
				if (Redefine)
					DeleteRedefinedItem (EditName);
				status=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,0,NewRefno,0,-1,NewAreaSymbol,(LPSHORT)pStuff,Prefix,UDI,NewAreaColor,-1,0,0,0,0,0,DigHiPrecis,0);
				NewSymbol = NewAreaSymbol;   
				if (status && *Prefix)
				{   
					double	IncVal = GetGlobalDVal2 ("[%NEW_AREA_AUTOINC]",0);
					
					if (AutoIncTAG (Prefix,UDI,IncVal))
						SetGlobalValue ("%NEW_AREA_UDI",UDI);
				}
			}
			if (*SaveUDI)
				SetGlobalValue ("%NEW_AREA_UDI",SaveUDI);
			CloseMap(TRUE);
			CloseRefIndex(FALSE);    		
    		ForceRefIndex = ForceTAGIndex = FALSE;
        	GSSiGlobFree(&hNewPolyPoints);
        	NumNewPolyPoints=0; 
        	if (status)
        	{ 
				if (!GetSymbolName (NewSymbol,SymName,0,2,0))
				{   
					HANDLE	hSymDesc=0;
					short	NumSyms=0;
					
		    		_fstrcpy (PltName,EditName);
		    		PltType = 2;
					AddToSymList (NewSymbol,&NumSyms,&hSymDesc); 
					AddSymToMap (NumSyms,hSymDesc,0,0); 
	                DestroySymList (&NumSyms,&hSymDesc);
				}
	        	_fstrcpy (PickName,EditName);
	    		PltType = 2;
	        	PickList[0].Segment = ItemSeg;
	        	PickList[0].Offset = CurrentItem;  
	        	PickList[0].FileNum=CurView->UpdateFile-1;
				PickList[0].SubFile = CurView->SubFile;
				PickList[0].ConfigID = CurrentConfig;
	        	PickList[0].ViewID = CurView->ID;
	        	ProcessPickedItem(0,TRUE);
	        	rtn = GF_INCREASE_SUCCESS_COUNT;
	        }  
	        else
	        	GMMessageBox (MSG_OUTOFEDLIM,0,MB_ICONEXCLAMATION);
ExitLine: 	GSSiGlobUlFree (&hStuff);
    		GSSiGlobUlFree (&hText); 
		    GSSiGlobFree (&hTextTPL); 
    		GetNewRefnoNoUpdate (0);
        } 
//        Inited = FALSE;
//        RemoveGraphicsFunction (hWnd);   
		}
        break; 
        
    case GF_CLOSE: 
		if (hSavedScreen)
		{
			RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
	        DestroySavedScreen (&hSavedScreen,BitmapID);
		} 
		NumNewPolyPoints = 0;
		GSSiGlobFree (&hNewPolyPoints);
		SetCurs (InCursor,FALSE); 
        return FALSE;
        
    default:
    	return (FALSE);
    }
    return rtn;
} 

BOOL ThemeChangeSymbol (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	BOOL	HaveDown=FALSE;
 static	BOOL	HaveInClass=FALSE;	
 char	str[260];
 int	iclass;

 switch (Message)
   {
   	case GF_INIT:
		if (lParam)
		{
			iclass = lParam;
			HaveInClass = TRUE;
			goto HaveClass;
		}
		HaveInClass = FALSE;
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    
    case WM_RBUTTONUP:
		GetGFFile (str,CurTheme->SQL,2); 
		GMEdit (hWnd,str);
    	break;
    		
    case WM_LBUTTONUP:
	{	 
		int		isym, rtn=0; 
		char	SymName[34], SaveSymDict[MAX_PATH];
		
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{  
HaveClass:
				if (*CurTheme->IconLibrary)
				{
					GetGlobalCVal ("[%SYM_DICT]",SaveSymDict,0);  
					sprintf (str,"%sfilelist.txt",CurTheme->IconLibrary);
					SetGlobalValue ("%SYM_DICT",str);
					CloseSymDict(); 
					if (OpenSymDict (OF_READ))
						rtn = SelectPointSymbol (hWnd,2,SymName,"",0,0,0,FALSE);
					SetGlobalValue ("%SYM_DICT",SaveSymDict);    
					CloseSymDict(); 
					if (rtn)
					{
						HFILE Fid;
						LPSTR	AtLoc;
						
						GetGFFile (str,CurTheme->SQL,2); 
						Fid = GSSiOpenFile (str,0,OF_READWRITE); 
                        GSSillseek (Fid,labs(CurTheme->ClassCount[iclass])-1,0);
						fgetstring (str,256,Fid);
						if ((AtLoc = _fstrchr (str,'&')))
						{ 
							GSSillseek (Fid,labs(CurTheme->ClassCount[iclass])-1+((long)AtLoc - (long)str),0);
							_fstrcpy (str,"&"); 
							_fstrcat (str,SymName);
							PadString (str,' ',13);
							BigWrite (Fid,str,13,-1);
						}
						GSSiClose (Fid);
					}
				}
				else if (*CurTheme->SymbolFont[0])
				{
					_fmemmove (CurSymbolFont,CurTheme->SymbolFont,sizeof(CurSymbolFont));
					CurTheme->ClassSymbol[iclass] = SelectFontSymbol (hWnd,CurTheme->SymSizeC,&CurTheme->ClassColor[iclass]);
					_fmemmove (CurTheme->SymbolFont,CurSymbolFont,sizeof(CurSymbolFont));
				}
				else if (CurTheme->DataType == THEMEDATATYPE_POINT)
				{
					GetDictSymName (CurTheme->ClassSymbol[iclass],SymName);
					SetGlobalValue("%NEW_POINT_SYM",SymName); 
					SetGlobalValue("%NEW_POINT_SIZE",CurTheme->SymSizeC); 
					SetGlobalValueLong("%NEW_POINT_COLOR",CurTheme->ClassColor[iclass]); 
					if (!(isym=SelectSymbol (hWnd,1,"ALL",1)))
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
						return TRUE;
}
					CurTheme->ClassSymbol[iclass]=isym;    
					GetGlobalCVal ("[%NEW_POINT_SIZE]",CurTheme->SymSizeC,0);
					CurTheme->ClassColor[iclass]=GetGlobalLVal ("[%NEW_POINT_COLOR]"); 
				}
				else if (CurTheme->DataType == THEMEDATATYPE_LINE)
				{
					GetDictSymName (CurTheme->ClassSymbol[iclass],SymName);
					SetGlobalValue("%NEW_LINE_SYM",SymName); 
					SetGlobalValue("%NEW_LINE_WIDTH",CurTheme->SymSizeC); 
					SetGlobalValueLong("%NEW_LINE_COLOR",CurTheme->ClassColor[iclass]); 
					if (!(isym=SelectSymbol (hWnd,2,"ALL",1)))
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
						return TRUE;
}
					CurTheme->ClassSymbol[iclass]=isym;    
					GetGlobalCVal ("[%NEW_LINE_SIZE]",CurTheme->SymSizeC,0);
					CurTheme->ClassColor[iclass]=GetGlobalLVal ("[%NEW_LINE_COLOR]"); 
				}
				break;
			}
		}
		ThemeDisplayLegend(3,0);
		if (HaveInClass)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
BOOL ThemeChangeFactor (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;
 char	str[260];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    
    case WM_RBUTTONUP:
		GetGFFile (str,CurTheme->SQL,2); 
		GMEdit (hWnd,str);
    	break;
    		
    case WM_LBUTTONUP:
	{	 
		int		iclass, isym, rtn=0; 
		char	SymName[34], SaveSymDict[128];
		
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   

	        	sprintf (str,"%f",CurTheme->ClassFactor[iclass]);
                if (!GetTextString (GetFocus(),str,32,"Enter symbol factor",0,0,0,TRUE,TRUE))
                	break;
                CurTheme->ClassFactor[iclass] = atof (str);

				break;
			}
		}
		ThemeDisplayLegend(3,0);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ThemeChangeWidth (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	HaveDown=FALSE;
 char	str[260];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
   		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
    	break;
    
    case WM_RBUTTONUP:
		GetGFFile (str,CurTheme->SQL,2); 
		GMEdit (hWnd,str);
    	break;
    		
    case WM_LBUTTONUP:
	{	 
		int		iclass, isym, rtn=0; 
		char	SymName[34], SaveSymDict[128];
		
    	if (!HaveDown) break;
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   
				double	Dist;
				short	Units;

	        	sprintf (str,"%f",CurTheme->ClassFactor[iclass]);
                if (!GetTextString (GetFocus(),str,32,"Enter line width (feet)",0,0,0,TRUE,TRUE))
                	break;
				GetDistAndUnits (str,&Dist,&Units,TRUE);   
				CurTheme->AbsLineWidth[iclass] = -ConvertInDist (Dist,Units+1);

				break;
			}
		}
		ThemeDisplayLegend(3,0);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 
BOOL EdgeMatchLines (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1099);
#endif
{HDC hDC;
 int	st;     
 BOOL	rtn;
 HIGHLIGHTDATA	HighlightData;
 POINT	MousePoint, CursorPoint;
 DPOINT	BasePoint, Point1, Point2, MidPoint;  
 double	d1,d2;   
 long	Refno;
 LPTHEME	pTheme;   
 char	PName1[144]; 
 HANDLE	hPnts;  
 LPDPOINT	pPoints;  
 int	npt=2; 
 short	symnum;
 short    NumSyms=0;  
 HANDLE hSymDesc=0;
  

 NumPicked = 0;    
 switch (Message)
   {
   	case GF_INIT:
		SetPickCursor (TRUE);
       	AddLBUTTON = FALSE;   
	    if (BT_NUM_IN_INDEX (hHighlight) != 2)
        {
        	GSSiMessageBox (0,"This function requires two highlighted items",0,MB_ICONEXCLAMATION,0);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
        }	
   		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
		PickList[0] = HighlightData.PD;
		GetPickName (0);  
		_fstrcpy(PName1,PickName);
	   	BT_FIND (hHighlight,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&HighlightData); 
		PickList[0] = HighlightData.PD;
		GetPickName (0);   
		if (!_fstricmp (PickName,PName1))
        {
        	GSSiMessageBox (0,"This function requires two items highlighted from different files",0,MB_ICONEXCLAMATION,0);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
        }	
   		break;
    
    case GF_CLOSE:
   		ClearHighlightList(FALSE);
		SetPickCursor (FALSE);
{
#if ENABLETRACE
GSSiExitProg (1099);
#endif
    	return FALSE; 
}
    	
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    PickItems (hWnd,BasePoint);
	    if (!NumPicked)
{
#if ENABLETRACE
GSSiExitProg (1099);
#endif
	    	return FALSE;
}
	    MidPoint = PickList[NumPicked-1].PickedPoint; 
	case WM_RBUTTONUP:
		if (!NumPicked)
		{
	   		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
			if (ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.EndPoint) < 
				ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.BeginPoint))
				Point1 = HighlightData.PD.EndPoint;
			else
				Point1 = HighlightData.PD.BeginPoint;
	   		BT_FIND (hHighlight,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&HighlightData); 
			if (ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.EndPoint) < 
				ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.BeginPoint))
				Point2 = HighlightData.PD.EndPoint;
			else
				Point2 = HighlightData.PD.BeginPoint;
			MidPoint = MidPointD (Point1,Point2);
	   	}
   		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
		if (ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.EndPoint) < 
			ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.BeginPoint))
			Point1 = HighlightData.PD.EndPoint;
		else
			Point1 = HighlightData.PD.BeginPoint;
		symnum = GetOrCreateSym (hWnd,"TRANLINE",&NumSyms,&hSymDesc,TRUE,2);
		hPnts = GSSiGlobAlloc (1064,GMEM_MOVEABLE,1024);
		PickList[0] = HighlightData.PD;
		GetPickName (0);  
		_fstrcpy(PltName,PickName);
		SetPltNameGlobals ();
		GetGlobalCVal ("[%TRANFILENAME]",PltName,0); 
		ExpandText (PltName);
		OpenMap (CurView->hWnd,CurView->hDC);
		EditBounds = CurView->FileMNMX; 
		CloseMap (FALSE);
	    Refno=GetNewRefno(PltName,0,0,0,0);   
	    pPoints = (LPDPOINT)GlobalLock (hPnts);
	    *pPoints++ = Point1;
	    *pPoints = MidPoint;
	    GlobalUnlock (hPnts);
		st = AddPolyToMap (1,&npt,&hPnts,1,Refno,0,2,symnum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);  
		CloseMap (TRUE);  
 	 	AddSymToMap (NumSyms,hSymDesc,0,0); 
   		BT_FIND (hHighlight,(LPSTR)&Refno,BT_NEXT,BT_ANY,(LPSTR)&HighlightData); 
		if (ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.EndPoint) < 
			ldistp (HighlightData.PD.PickedPoint,HighlightData.PD.BeginPoint))
			Point1 = HighlightData.PD.EndPoint;
		else
			Point1 = HighlightData.PD.BeginPoint;
		PickList[0] = HighlightData.PD;
		GetPickName (0);  
		_fstrcpy(PltName,PickName); 
		SetPltNameGlobals ();
		GetGlobalCVal ("[%TRANFILENAME]",PltName,0); 
		ExpandText (PltName);
		OpenMap (CurView->hWnd,CurView->hDC);
		EditBounds = CurView->FileMNMX; 
		CloseMap (FALSE);
	    Refno=GetNewRefno(PltName,0,0,0,0);   
	    pPoints = (LPDPOINT)GlobalLock (hPnts);
	    *pPoints++ = Point1;
	    *pPoints = MidPoint;
	    GlobalUnlock (hPnts);
		st = AddPolyToMap (1,&npt,&hPnts,1,Refno,0,2,symnum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
	   	GSSiGlobFree (&hPnts);	
		CloseMap (TRUE);  
 	 	AddSymToMap (NumSyms,hSymDesc,0,0); 
        DestroySymList (&NumSyms,&hSymDesc);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1099);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1099);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL MovePoint (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1110);
#endif
{
 char key; 
 static	long	CPL, CPLD;
 static	BOOL	HavePoint;
 short	rtn=1;
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE; 
       	HavePoint = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (1110);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, FilePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

 		if (CursorIsLocked)  
 		{
 		 	BasePoint = CurrentPoint;
	    	UnlockCursor ();
	    }
	    else
	    { 
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		} 
		EnlargeScreen (0,0);
		if (!HavePoint)
		{
		    PickItems (hWnd,BasePoint);
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
			ProcessPickedItem (Item,FALSE);
			HavePoint = TRUE;
			CPL = CurSymSizeLoc;
			CPLD = CurSymSizeLocD;
		}       		
		else
       	if (CPL || CPLD)
       	{   
       		BOOL	SaveDisableHalt;  
       		long	Loc=max (CPL,CPLD);
       		
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
			Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{   
				short	l;
				
				GSSillseek (Fid,Loc,0);
				BigRead (Fid,(HPSTR)&ID,2);  
				FilePoint = BasePtToFilePt (BasePoint);
				if (ID == 20)
				{   
					float OldSize;
					
					GSSillseek (Fid,Loc+10,0);
					BigWrite (Fid,(HPSTR)&FilePoint,sizeof(FilePoint),-1);
				}
				else if (ID == 120)
				{   
					double OldSize;
					
					GSSillseek (Fid,Loc+18,0);
					BigWrite (Fid,(HPSTR)&BasePoint,sizeof(BasePoint),-1);
				}
				GSSiClose (Fid);  
				rtn = GF_INCREASE_SUCCESS_COUNT; 
			}
        	DisableHalt = SaveDisableHalt; 
        	HavePoint = FALSE;
       	}
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1110);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1110);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL MoveOffsetLine (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1110);
#endif
{
 char key; 
 static	long	CPL, CPLD;
 static	BOOL	HavePoint;
 short	rtn=1;
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE; 
       	HavePoint = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (1110);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case WM_MOUSEMOVE:
    	{
    		POINT MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
			int	OldMode = SetROP2(CurView->hDC,R2_NOT);
			
			SetROP2(CurView->hDC,OldMode);
    	}
       	break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, FilePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

 		if (CursorIsLocked)  
 		{
 		 	BasePoint = CurrentPoint;
	    	UnlockCursor ();
	    }
	    else
	    { 
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		} 
		EnlargeScreen (0,0);
		if (!HavePoint)
		{
		    PickItems (hWnd,BasePoint);
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
			ProcessPickedItem (Item,FALSE);
			HavePoint = TRUE;
			CPL = CurSymSizeLoc;
			CPLD = CurSymSizeLocD;
		}       		
		else
       	if (CPL || CPLD)
       	{   
       		BOOL	SaveDisableHalt;  
       		long	Loc=max (CPL,CPLD);
       		
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
			Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{   
				short	l;
				
				GSSillseek (Fid,Loc,0);
				BigRead (Fid,(HPSTR)&ID,2);  
				FilePoint = BasePtToFilePt (BasePoint);
				if (ID == 20)
				{   
					float OldSize;
					
					GSSillseek (Fid,Loc+10,0);
					BigWrite (Fid,(HPSTR)&FilePoint,sizeof(FilePoint),-1);
				}
				else if (ID == 120)
				{   
					double OldSize;
					
					GSSillseek (Fid,Loc+18,0);
					BigWrite (Fid,(HPSTR)&BasePoint,sizeof(BasePoint),-1);
				}
				GSSiClose (Fid);  
				rtn = GF_INCREASE_SUCCESS_COUNT; 
			}
        	DisableHalt = SaveDisableHalt; 
        	HavePoint = FALSE;
       	}
    }
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1110);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1110);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL RefConnectOutput (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 
 switch (Message)
   {
   	case GF_INIT:
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
           
    case GF_EXECUTE:  
    	OutputRefConnect ();
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}  

BOOL SelectByClass (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1315);
#endif
{
 char key;
 POINT	MousePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP: 
    {
    	
    	SelectThemeClasses (-1);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1315);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1315);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL ChangeRefno (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1072);
#endif
{
	static long	NewRefno;
   	char	NewRefC[34]="";   
 char key;     
 switch (Message)
   {
        
   	case GF_INIT:  
   		NewRefno = LONG_MAX;
		SetEditName (TRUE);   	
       	AddLBUTTON = TRUE;
{
#if ENABLETRACE
GSSiExitProg (1072);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case GF_EXECUTE:
    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID, rtn;  

        if (Message == GF_EXECUTE)
        	Item = 0;
        else
        {
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	        BasePoint=ScreenPtToBasePt(MousePoint);
	        PickItems (hWnd,BasePoint);
        	if (!NumPicked)
        		break;
        	Item = NumPicked-1; 
        } 
        if (Function == GF_CHANGE_REFNO)
        {   
			if (NewRefno == LONG_MAX)
			{
        		if (GetTextString (hWnd,NewRefC,32,"Enter new refno",0,0,0,TRUE,TRUE))
        		{
        			NewRefno = atol (NewRefC);
				}
				else
					return GF_EXECUTE_CANCELED;
			}
			ChangePickedItemRefno (0,NewRefno); 
			NewRefno++;
		}
        else
			ChangePickedItemRefno (0,LONG_MAX);
    } 
    break;
    
    default:
{
#if ENABLETRACE
GSSiExitProg (1072);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1072);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}  

BOOL IdentifyTraverseLeg (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{    
	char	str[128];
	
 switch (Message)
   {
   	case GF_INIT:
   		if (!hWndTraverseEntry ||!hSnapStatus)
   			return FALSE; 
		if (TravDest != DEST_TEST)
		{
			GSSiMessageBox (0,"Traverse entry must be in test mode to use this function",0,MB_OK,0);
			return FALSE;
		} 
       	AddLBUTTON = TRUE; 
       	return GF_READY_TO_PROCESS;
   		break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, FilePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

 		if (CursorIsLocked)  
 		{
 		 	BasePoint = CurrentPoint;
	    	UnlockCursor ();
	    }
	    else
	    { 
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		}
		EnlargeScreen (0,0); 
	    PickItems (hWnd,BasePoint);
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;
		SendDlgItemMessage (hWndTraverseEntry,IDC_TRAVLIST,LB_SETCURSEL,(WPARAM)(PickList[Item].Refno+POBItem),0);
	    ltoa (PickList[Item].Refno,str,10);
	    SetDlgItemText (hWndTraverseEntry,IDC_CALLNO,str);
 		EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_REMOVELEG),TRUE);
 		EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_EDLEG),TRUE); 
		SetFocus (GetDlgItem(hWndTraverseEntry,IDOK));
		ShowWindow (hWndTraverseEntry,SW_RESTORE);
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL EditBrushColor (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1053);
#endif
{
 static	BOOL	HaveNewSettings;     
 short	rtn=1;
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;  
       	HaveNewSettings = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1053);
#endif
       	return GF_READY_TO_PROCESS;
}
   		break;

    case GF_EXECUTE:
    case WM_LBUTTONUP:
	case GF_USEPICKED:
    {
		short	Item=0; 
		POINT	MousePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;
    	COLORREF	NewColor;

        if (Message == WM_LBUTTONUP)
        {   
        	HaveNewSettings = FALSE;
	    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		    BasePoint=ScreenPtToBasePt(MousePoint);
		    PickItems (hWnd,BasePoint);
		    if (!NumPicked)
		    	break;  
		    Item = NumPicked-1;
		}
		ProcessPickedItem (Item,FALSE);        		
       	if (Function == GF_CHANGE_BRUSH_COLOR && CurBrushLoc)
       	{   
       		BOOL	SaveDisableHalt;  
       		long	Loc=CurBrushLoc;
       		
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
       		if (!HaveNewSettings)
       		{
				Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READ);
				if (Fid != HFILE_ERROR)
				{
					GSSillseek (Fid,Loc,0);
					BigRead (Fid,(HPSTR)&ID,2);
					if (ID == 22)
					{
						GSSillseek (Fid,2,1);
						BigRead (Fid,(HPSTR)&NewColor,4);
					}
					GSSiClose (Fid);
					
					if (GetColor (hWnd,&NewColor))
						HaveNewSettings = TRUE;
					else 
					{
			        	DisableHalt = SaveDisableHalt;
			        	if (wParam == 1)  
{
#if ENABLETRACE
GSSiExitProg (1053);
#endif
			        		return GF_EXECUTE_CANCELED;
}
			        	else
							PostMessage(hWnd, GF_CLOSE,0, 0L); 
						break; 
					}
				}
			}
			Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{   
				short	l;
				
				GSSillseek (Fid,Loc,0);
				BigRead (Fid,(HPSTR)&ID,2);
				if (ID == 22 && Function == GF_CHANGE_BRUSH_COLOR)
				{   
					GSSillseek (Fid,Loc+4,0); 
					BigWrite (Fid,(HPSTR)&NewColor,4,-1);
				}
				GSSiClose (Fid);  
				ProcessPickedItem (Item,TRUE);        		
				rtn = GF_INCREASE_SUCCESS_COUNT; 
			}
        	DisableHalt = SaveDisableHalt;
       	}
       	else if (Function == GF_CHANGE_LINE_COLOR && CurPenColorLoc)
       	{   
       		BOOL	SaveDisableHalt;  
       		long	Loc=CurPenColorLoc;
       		
        	SaveDisableHalt = DisableHalt;
    	    DisableHalt = TRUE;
       		
       		if (!HaveNewSettings)
       		{
				Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READ);
				if (Fid != HFILE_ERROR)
				{
					GSSillseek (Fid,Loc,0);
					BigRead (Fid,(HPSTR)&ID,2);
					if (ID == 21 || ID == 121)
					{
						BigRead (Fid,(HPSTR)&NewColor,4);
					}
					GSSiClose (Fid);
					
					if (GetColor (hWnd,&NewColor))
						HaveNewSettings = TRUE;
					else 
					{
			        	DisableHalt = SaveDisableHalt;
			        	if (wParam == 1)  
{
#if ENABLETRACE
GSSiExitProg (1053);
#endif
			        		return GF_EXECUTE_CANCELED;
}
			        	else
							PostMessage(hWnd, GF_CLOSE,0, 0L); 
						break; 
					}
				}
			}
			Fid = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{   
				short	l;
				
				GSSillseek (Fid,Loc,0);
				BigRead (Fid,(HPSTR)&ID,2);
				if ((ID == 21 || ID == 121) && Function == GF_CHANGE_LINE_COLOR)
				{   
					BigWrite (Fid,(HPSTR)&NewColor,4,-1);
				}
				GSSiClose (Fid);  
				rtn = GF_INCREASE_SUCCESS_COUNT; 
			}
        	DisableHalt = SaveDisableHalt;
       	}
    }
	if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
    	break;
    default:
{
#if ENABLETRACE
GSSiExitProg (1053);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1053);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
} 

BOOL SnapToPolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (934);
#endif
{  
 POINT	MousePoint;
 BOOL	WantOp;   
 static	HANDLE	SavedScreen=0; 
 static long	ScreenID=0;
 
 if (idTimer)
{
#if ENABLETRACE
GSSiExitProg (934);
#endif
 	return FALSE;
}
 switch (Message)
   {
   	case GF_INIT: 
   		WantOp = FALSE;
   		GSSiGlobFree (&hNewPolyPoints);
   		NumNewPolyPoints = 0;
		AddLBUTTON = FALSE; 
   		if (nInSnapList < 2)
		    PostMessage(hWnd, GF_CLOSE,0, 0L);
		else if (SnapList[0].Refno != SnapList[1].Refno)
		    PostMessage(hWnd, GF_CLOSE,0, 0L);
		else
		{   
			long	npnts;
			HANDLE	hPoly;
   			double		d1;
   			double		d2;
   			BOOL		Reverse;
ShowPoly:	   			
   			d1=SnapList[0].PCT * SnapList[0].Length;
   			d2=SnapList[1].PCT * SnapList[0].Length;
   			Reverse=FALSE;
   			if (d2 < d1)
   			{
   				
   				d1 = SnapList[0].Length - d1;
   				d2 = SnapList[0].Length - d2;
   				Reverse = TRUE;
   			}
			
	   		if (GetPolyPoints ((LPPICKDATAHEADER)&SnapList[0],Reverse,&npnts,&hPoly)) 
	   		{   
	   			HPDPOINT	pPoints = (HPDPOINT)GlobalLock (hPoly); 
	   			HPEN		hPen, hOldPen; 
	   			RECT		Rect;
	   			
				hNewPolyPoints = GetPolyBetweenDist (pPoints,npnts,d1,d2,&NumNewPolyPoints,FALSE,WantOp); 
				GSSiGlobUlFree (&hPoly); 
				GetPolyWRect (hNewPolyPoints,NumNewPolyPoints,&Rect);
				SavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,Rect,CurView,&ScreenID);
//				if (Reverse)
//					hNewPolyPoints = ReversePoints (npnts,hNewPolyPoints);	
				pPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
				hPen = CreatePen(PS_SOLID,3,RGB(255,0,0));
				hOldPen = SelectObject(CurView->hDC, hPen);
				GWPolylineD (CurView->hDC,pPoints,NumNewPolyPoints,0);  
				SelectObject (CurView->hDC,hOldPen);
				DeleteObject (hPen);
				GlobalUnlock (hNewPolyPoints);
				AddLBUTTON = FALSE; 
			}
			else
		    	PostMessage(hWnd, GF_CLOSE,0, 0L);                          
		    
		}
   		break;
   		
    case WM_LBUTTONUP: 
		RestoreScreen2 (CurView->hDC,SavedScreen,ScreenID,FALSE); 
    	PostMessage(hWnd, GF_CLOSE,0, 0L);                          
    	break;
   		
    case WM_RBUTTONUP: 
		RestoreScreen2 (CurView->hDC,SavedScreen,ScreenID,FALSE); 
    	WantOp = TRUE;
    	goto ShowPoly;                       
    	break;
   		
    case WM_MBUTTONUP: 
		RestoreScreen2 (CurView->hDC,SavedScreen,ScreenID,FALSE); 
    	GSSiGlobUlFree (&hNewPolyPoints);
    	NumNewPolyPoints = 0;
    	PostMessage(hWnd, GF_CLOSE,0, 0L);                          
    	break;
   		
    case GF_CANCEL: 
		RestoreScreen2 (CurView->hDC,SavedScreen,ScreenID,FALSE); 
    	GSSiGlobUlFree (&hNewPolyPoints);   
    	NumNewPolyPoints = 0;
{
#if ENABLETRACE
GSSiExitProg (934);
#endif
    	return FALSE;
}
    
    case GF_CLOSE:
		RestoreScreen2 (CurView->hDC,SavedScreen,ScreenID,FALSE); 
    	UnlockCursor ();  
    	EnlargeScreen (0,0);
{
#if ENABLETRACE
GSSiExitProg (934);
#endif
    	return FALSE;
}
    		
    default:
{
#if ENABLETRACE
GSSiExitProg (934);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (934);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

void ShowCurPoint (HWND hWnd,int CurPoint)
{
	HPDPOINT	Points;
	COLORREF	Color=RGB(0,255,200);
	BOOL		SaveDM = DisplayMarkers;

	if (!hNewPolyPoints)
		return;
	RestoreFullWindowBitmap ();
	Points = GlobalLock (hNewPolyPoints);
	DisplayMarkers = TRUE;
	DisplayMarker (Points[CurPoint],10,0,0,0,Color,FALSE,FALSE,0,0,0,0,0); 
	GlobalUnlock (hNewPolyPoints);
	DisplayMarkers = SaveDM;
	return;
}
BOOL RedefinePolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,int Function)
{  
	static	BOOL	Inited=FALSE;  
	static	HIGHLIGHTDATA	HighlightData;
	UINT	nPnts;
	long	Refno;
	static	int	CurPoint;
	HPDPOINT	Points;
	double		d, neard;
	UINT		i, neari, n;

 switch (Message)
   {
   	case GF_INIT: 
		if (BT_NUM_IN_INDEX (hHighlight) != 1) 
		{  
	ErrExit:  
			GSSiMessageBox (0,"One, and only one, polyline or area must be highlighted to redefine a polyline or polygon",0,MB_ICONEXCLAMATION,0);
			return FALSE;
		}
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
		if (HighlightData.PD.Type != 2 && HighlightData.PD.Type != 3)
			goto ErrExit;
   		if (Inited)
   		{
	 		MessageBox(GetFocus(), "Must complete current Area before beginning a new one", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		}
//       	AddLBUTTON = TRUE;
   		Inited = TRUE; 
		NewPolyType = 0;
		if (HighlightData.PD.Type == 2)
			NewPolyType = 1;
		if (Function == GF_REMOVE_POLY_POINT)
		{
			if (!GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&nPnts,&hNewPolyPoints))
				return FALSE;
			NumNewPolyPoints = nPnts;
			CurPoint = 0;
			SaveFullWindowBitmap (hWnd);
		}
		else
		{
   			SetPrompt (PRMT_DIST_LINEBEG,TRUE);
			CreateDigCursor (CurView->hDC);
			SetCurs ((HCURSOR)2,FALSE);
			AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,0); 
		}
   		break;
   		
    case WM_LBUTTONUP:
		{
			DPOINT		BasePoint;
	    	POINT		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));

   	
			if (CursorIsLocked)
	    		BasePoint = CurrentPoint;
			else
        		BasePoint=ScreenPtToBasePt (MousePoint); 
			Points = GlobalLock (hNewPolyPoints);
			neari = 0;
			neard = ldistp (BasePoint,Points[0]);
			for (i=0;i<NumNewPolyPoints;i++)
			{
				d = ldistp (BasePoint,Points[i]);
				if (d < neard)
				{
					neari = i;
					neard = d;
				}
			}
			CurPoint = neari;
			GlobalUnlock (hNewPolyPoints);
			ShowCurPoint (hWnd,CurPoint);
		}
			break;

    case WM_RBUTTONUP:
		{
			Points = GlobalLock (hNewPolyPoints);
			n = 0;
			for (i=0;i<NumNewPolyPoints;i++)
			{
				if (i != CurPoint)
					Points[n++] = Points[i];
			}
			NumNewPolyPoints = n;
			GlobalUnlock (hNewPolyPoints);
			PostMessage(hWnd, GF_COMPLETE,0, 0L); 
		}
		break;  
		
    case WM_CHAR:
	{
		switch (wParam)
		{
            case 'N':
			case 'n':
				if (CurPoint < NumNewPolyPoints-1)
					CurPoint++;
				break;
			case 'P':
			case 'p':
				if (CurPoint > 0)
					CurPoint--;
				break;
			case 'L':
			case 'l':
				CurPoint = NumNewPolyPoints - 1;
				break;
			case 'F':
			case 'f':
				CurPoint = 0;
				break;
		}
		ShowCurPoint (hWnd,CurPoint);
	}
		break;

    case GF_COMPLETE:
    	if (NumNewPolyPoints)
    	{   
		    ClearHighlightList (FALSE); 
       		GSSiGlobUlFree (&hUpdateMultiPolygon);
		    nUpdatePolyPoints = NumNewPolyPoints;
        	hUpdatePoly = hNewPolyPoints;
        	NumNewPolyPoints=0;
        	hNewPolyPoints=0; 
			PickList[0] = HighlightData.PD;
        	if (PickList[0].Type == 2)
				UpdateItem=61;   
        	else
				UpdateItem=51;   
			UpdateRecord (0,PickList[0].Desc,0,0,0,0,1,-1);
			GSSiGlobFree (&hUpdatePoly);  
        }  
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		if (Function == GF_REMOVE_POLY_POINT)
			RedisplayViewport(FALSE,FALSE);

        break; 
        
    case GF_CLOSE: 
		ClearFullWindowBitmap (0);	
		GSSiGlobFree (&hNewPolyPoints);
		Inited = FALSE; 
        return FALSE;
    	break;
        
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL EditRedefData (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (839);
#endif
{
	DLGPROC lpfnRDFEDITMsgProc; 
	short	nRc;
 switch (Message)
   {
   	case GF_INIT:
		
		lpfnRDFEDITMsgProc = MakeProcInstance((DLGPROC)RDFEDITMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"RDFEDIT", hWnd, lpfnRDFEDITMsgProc);
		FreeProcInstance(lpfnRDFEDITMsgProc);
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
   		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (839);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (839);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AddHltAreaPolyline (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1177);
#endif
{  
 static	HCURSOR	InCursor; 
 HANDLE	hHighlightArea; 
 LPMNMXCORD	lpRect;  
 int	i, nPoly,Type;
 HPDPOINT	pPoints, pOutPoints;
 double	Offset;
 
 if (idTimer)
{
#if ENABLETRACE
GSSiExitProg (1177);
#endif
 	return FALSE;
}
 switch (Message)
   {
   	case GF_INIT:  
        GSSiGlobFree (&hNewPolyPoints);
		if ((hHighlightArea = GetNextHighlightArea (0,0,&Type,&NumNewPolyPoints,&nPoly,0,&Offset,0)))
		{
		    lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea); 
            hNewPolyPoints = GSSiGlobAlloc (1262,GMEM_MOVEABLE,NumNewPolyPoints*sizeof(DPOINT)); 
            pOutPoints = (HPDPOINT)GlobalLock (hNewPolyPoints);
            lpRect++;
            pPoints = (HPDPOINT)lpRect;
            for (i=0;i<NumNewPolyPoints;i++)
            	*pOutPoints++ = *pPoints++;
            GlobalUnlock (hNewPolyPoints);
            GSSiGlobUlFree (&hHighlightArea);
        }
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
    	break;
        
    default:
{
#if ENABLETRACE
GSSiExitProg (1177);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1177);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL AutoSplinePoints (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{  
	POINTS	MousePoint;
	POINT	ScreenPoints[2];
	static	POINT	BeginPoint;   
	HPDPOINT	DPoints, pCurPoints; 
	short	Pickap;
	HPEN	hPen, hOldPen;
	COLORREF	White = RGB(255,255,255);
    
    
	if (App)
		return FALSE;
	if (idTimer)
		return FALSE;
	switch (Message)
   {
   	case GF_INIT: 
   		if (nCurPolyPoints < 2)
   			return FALSE; 
		if (DoTrack && hTempPoints && nTempPoints)
		{
		 	HPPOINT	Points = (HPPOINT)GlobalLock (hTempPoints);
			NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0); 
			GlobalUnlock (hTempPoints);
		}
   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
		pCurPoints = (HPDPOINT)GlobalLock (hCurPolyPoints);
		ScreenPoints[0] = BasePtToScreenPt (&pCurPoints[nCurPolyPoints-2]);
		ScreenPoints[1] = BasePtToScreenPt (&pCurPoints[nCurPolyPoints-1]);
    	UseUserPickAp = FALSE;
		SystemPickAp = -2 * GetGlobalDVal2 ("[%BLACKLINEWIDTH]",2);
		Pickap = SetPickAp(0);
		hPen = CreatePen (PS_SOLID,(short)IDNINT((Pickap-1)),White);
		hOldPen = SelectObject (CurView->hDC,hPen);
		Polyline (CurView->hDC,ScreenPoints,2);
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hPen);
		GlobalUnlock (hCurPolyPoints);
		if (!PickColor (ScreenPoints[1],0,&BeginPoint,FALSE,2))
		{
			UseUserPickAp = TRUE;
			return FALSE;
		}                                     
		UseUserPickAp = TRUE;
   		break;
   		
    case WM_LBUTTONUP:  
    {
		short	nextx[8]={1,1,0,-1,-1,-1,0,1};
		short	nexty[8]={0,-1,-1,-1,0,1,1,1}; 
		POINT	EndPoint;
		HANDLE	hPoints, hDPoints, hFlags;
		HPPOINT	BitPoints; 
		HPBYTE	Flags;  
		long	i, nPntsIn = nCurPolyPoints; 
		long	nBitPnts=0, AtPnt=0;
		short	x, y, pass;
		long	FirstFlag, j, k, ii;   
		long	MAXBITPOINTS=USHRT_MAX/2; 
		HPDPOINT	pNewPoints; 
		DPOINT	MidPointD; 
		double	AZ;
		double	BlackLineWidth = GetGlobalDVal2 ("[%BLACKLINEWIDTH]",2);
		double	SplinePointDist = BlackLineWidth;//GetGlobalDVal2 ("[%SPLINEPOINTDIST]",5); 
		double	dist;

    	MousePoint = MAKEPOINTS(lParam);
		UnlockCursor ();
		EnlargeScreen (0,0);
		if (!PickColor (POINTStoPOINT(MousePoint),GetGlobalLVal2("[%PICKCOLOR]",0),&EndPoint,FALSE,2))
			return FALSE; 
		hDPoints = GSSiGlobAlloc ( 611,GMEM_MOVEABLE,(nPntsIn+1024)*sizeof(DPOINT)); 
		DPoints = (HPDPOINT)GlobalLock (hDPoints);
		hPoints = GSSiGlobAlloc ( 612,GMEM_MOVEABLE,MAXBITPOINTS*sizeof(POINT)); 
		BitPoints = (HPPOINT)GlobalLock (hPoints); 
		hFlags = GSSiGlobAlloc ( 613,GHND,(long)MAXBITPOINTS*8);  
		Flags = (HPBYTE)GlobalLock (hFlags);
		pCurPoints = (HPDPOINT)GlobalLock (hCurPolyPoints);
		for (i=0;i<nPntsIn;i++)
			DPoints[i] = BasePtToWinPtD (&pCurPoints[i]);
		GlobalUnlock (hCurPolyPoints);    
		
		BitPoints[nBitPnts++] = BeginPoint;
AddBitPoint:
		if (nBitPnts < MAXBITPOINTS)
		{
			SetPixel (CurView->hDC,BitPoints[nBitPnts-1].x,BitPoints[nBitPnts-1].y,White); 
			while (AtPnt < nBitPnts)
			{
				FirstFlag = AtPnt * 8;
				for (j=0;j<8;j++)
				{
					if (!Flags[FirstFlag + j])
					{   
						Flags[FirstFlag + j] = 1;
						x = BitPoints[AtPnt].x + nextx[j];
						y = BitPoints[AtPnt].y + nexty[j];
						if (x == EndPoint.x && y == EndPoint.y)
							goto EndOfTheLine;
						if (!GetPixel (CurView->hDC,x,y))
						{
							k = nBitPnts * 8 + (j+4) % 8;
							Flags[k] = 1;
							BitPoints[nBitPnts].x = x;
							BitPoints[nBitPnts++].y = y; 
							goto AddBitPoint;
						}
					}
				}
				AtPnt++;
			}
	   		for (i=0;i<nBitPnts;i++)
	   			SetPixel (CurView->hDC,BitPoints[i].x,BitPoints[i].y,0);  
			GSSiGlobUlFree (&hPoints); 
			GSSiGlobUlFree (&hDPoints);
			GSSiGlobUlFree (&hFlags); 
			return FALSE;
		}
		else
			ii=1;
EndOfTheLine:		    
		GSSiGlobUlFree (&hDPoints);
		GSSiGlobUlFree (&hFlags); 
   		for (i=0;i<nBitPnts;i++)
   			SetPixel (CurView->hDC,BitPoints[i].x,BitPoints[i].y,0);  
		NumNewPolyPoints = min (nBitPnts,30); 
		for (pass = 0; pass < 2; pass++)
		{                                                        
			GSSiGlobFree (&hNewPolyPoints);
	   		hNewPolyPoints = GSSiGlobAlloc ( 614,GMEM_MOVEABLE,NumNewPolyPoints*sizeof(DPOINT)); 
	   		pNewPoints = (HPDPOINT)GlobalLock (hNewPolyPoints);
	   		for (i=0,k=nBitPnts/NumNewPolyPoints,j=k;i<NumNewPolyPoints-1;i++,j=k+((i*nBitPnts)/NumNewPolyPoints))
	   		{   
	   			if (pass)
	   			{
		   			if (!i)
		   				AZ = getaz (BeginPoint, BitPoints[j+k]);
		   			else if (i < NumNewPolyPoints - 2)
		   				AZ = getaz (BitPoints[j-k],BitPoints[j+k]);
		   			else
		   				AZ = getaz (BitPoints[j-k],EndPoint);
		   			BlackLineMidPoint (BitPoints[j],AZ,0,0,&MidPointD,0);
		   		}
		   		else
		   			MidPointD = PointToDPoint (BitPoints[j]);
	   			pNewPoints[i] = WinPtToBasePtD (&MidPointD); 
	   		}
			pNewPoints[NumNewPolyPoints-1] = WinPtToBasePt (EndPoint);
   			if (!pass)
   			{
				dist = GetPolyLengthD (pNewPoints,NumNewPolyPoints);  
				NumNewPolyPoints = IDNINT(dist / SplinePointDist) + 1;
			}
			else if (!Spline)
			{
				double	Thin = GetGlobalDVal2 ("[%SPLINETHIN]",0.1); 
				long	nNewPt=NumNewPolyPoints;

				ThinPoly (&nNewPt,pNewPoints,Thin);
				NumNewPolyPoints = nNewPt;
			}
	   		GlobalUnlock (hNewPolyPoints); 
	   	}
		GSSiGlobUlFree (&hPoints); 
		nTempPoints = 0;
	    PostMessage(hWnd, GF_CLOSE,0, 0L); 
	}
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
} 
BOOL SetViewportParms (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (1381);
#endif
{
 LPVIEWPORT	SaveVP, LastVP;
 	
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE:
    {
    	short	NumNewVP=2; 
    	BOOL	SplitVert=TRUE, SplitHorz=FALSE;
    	
	    switch (Function)
	    {   
			case GF_QUAD_VIEWPORT:
				NumNewVP = 4;   
				SplitVert = SplitHorz = TRUE;
	    	case GF_SPLIT_VIEWPORT:
	    	{
	    		char	NewName[32];
	    		short	end, i;
	    		double	splitgap=GetGlobalDVal2 ("[%SPLITGAP]",0.5);
	    		
				if (*pNumViewports + NumNewVP > MAX_VIEWPORTS)
				{
                 	GSSiMessageBox (0,"Maximum viewports exceeded",0,MB_ICONEXCLAMATION,0);
					break;
				}
				SaveVP = LastVP = CurView;  
				_fstrcpy (NewName,CurView->Name);
				end = min (29,_fstrlen (NewName));
                
                for (i=1;i<NumNewVP;i++)
                {
					sprintf (&NewName[end]," %i",i);
					CurView = SaveVP;
					CopyCurViewToNew (NewName);  
					CurView->LastWidth = 0;
					switch (i)
					{   
						case 1: 
							if (NumNewVP == 4)
							{
								CurView->Height = CurView->Height/2 - splitgap/2; 
								CurView->Width  = CurView->Width/2 - splitgap/2; 
								CurView->TagPoint.y += CurView->Height + splitgap;
							}
							else if (SplitVert) 
							{
								CurView->Width = CurView->Width/2 - splitgap/2; 
								CurView->TagPoint.x += CurView->Width + splitgap;  
							}
							else if (SplitHorz)
							{
								CurView->Height = CurView->Height/2 - splitgap/2; 
								CurView->TagPoint.y += CurView->Height + splitgap;
							}  
						break;
						case 2:
							CurView->Height = CurView->Height/2 - splitgap/2; 
							CurView->Width  = CurView->Width/2 - splitgap/2; 
							CurView->TagPoint.y += CurView->Height + splitgap;
							CurView->TagPoint.x += CurView->Width + splitgap;  
						break;
						case 3: 
							CurView->Height = CurView->Height/2 - splitgap/2; 
							CurView->Width  = CurView->Width/2 - splitgap/2; 
							CurView->TagPoint.x += CurView->Width + splitgap;  
						break;
					}	
					CurView->LinkedTo = LastVP->ID;
					LastVP = CurView;
				}
				CurView = SaveVP;
				CurView->LinkedTo = LastVP->ID;
				if (NumNewVP == 4)
				{
					CurView->Height = CurView->Height/2 - splitgap/2; 
					CurView->Width  = CurView->Width/2 - splitgap/2; 
				}
				else if (SplitVert) 
					CurView->Width  = CurView->Width/2 - splitgap/2; 
				else if (SplitHorz)
					CurView->Height = CurView->Height/2 - splitgap/2; 
			}	
				
			break;
			
			case GF_SET_PICKABILITY:  
				Pickability = TRUE; 
				goto SetVis;
			case GF_SET_VISIBILITY:
				Pickability = FALSE; 
		SetVis:
             {
              DLGPROC lpfnVISIBLEMsgProc;  
              char	VisDialog[2][10]={"VISIBLE","VISIBLE1"};

			  if (!CurrentConfig || !CurView)
			  {
			  	SetConfig (1);
			  	SetViewport(*pCommandViewport);
			  }
              lpfnVISIBLEMsgProc = MakeProcInstance((DLGPROC)VISIBLEMsgProc, hInst);
              DialogBox(hInst, (LPSTR)VisDialog[VisListOpt], hWnd, lpfnVISIBLEMsgProc);
              FreeProcInstance(lpfnVISIBLEMsgProc);
 
             }
		    break;

			case GF_LOAD_PICKABILITY:
				Pickability = TRUE; 
				goto SetPik;
			case GF_LOAD_VISIBILITY:
				Pickability = FALSE;
		SetPik: 
		    {
				short	nRc;
                DLGPROC lpfnVISFILESMsgProc;
                  
				lpfnVISFILESMsgProc = MakeProcInstance((DLGPROC)VISFILESMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"VISFILES", hWnd, lpfnVISFILESMsgProc);
				FreeProcInstance(lpfnVISFILESMsgProc);
		    }
		    break;
		}
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
		PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1381);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1381);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL TextSizeClass (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (158);
#endif
{HDC hDC;
 char str[128], Prompt[32];
 POINT	MousePoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 COLORREF	NewColor;
 short	idesc, newob;
 
 if (Message == WM_LBUTTONDOWN || Message == GF_INIT)
 {
   	AddLBUTTON = TRUE;
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
   	return TRUE;
}
 }
    	
if (Function == GF_SIZE_CLASSTEXT_ALLVIS)
	strcpy (Prompt,"Enter the text size factor");
else
	strcpy (Prompt,"Enter the point size factor");
if (ForAllVis)
 {  
 	BOOL	First=TRUE, Redisplay=FALSE;
 	
 	if (Message == WM_LBUTTONUP)
 	{
        *str=0;

        if(GetTextString (GetFocus(),str,128,Prompt,0,0,0,TRUE,TRUE))
        {   
        	double 	factor = atof (str);
        		
	    	SetCVTFromVis ();
	    	for (idesc=1;idesc<3201;idesc++) 
	    	{
				if (CurView->CurVisType[idesc])
				{   
					if (First)
					{   
						First = FALSE;
				    	newob = NextNewObject(TRUE);
						if (Function == GF_SIZE_CLASSTEXT_ALLVIS)
			    			CurView->NewObjectTextFactor[newob] *= factor;
						else
						{
							if (!CurView->NewObject[newob].Width)
								CurView->NewObject[newob].Width = -1;
							CurView->NewObject[newob].Width *= factor;
						}
				    }
					CurView->NewObjectMap[idesc]=newob+1;   
				}
			}
		    CurView->CurZoomAreaRef = 0;
		    Redisplay=TRUE;
	    }
		PostMessage(hWnd, GF_CLOSE,0, 0L);
		if (Redisplay)
			RedisplayViewport(FALSE,FALSE);
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
		return TRUE;
}
	} 
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
	return FALSE;
}
 }


 switch (Message)
   {
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;
        if (NumPicked > 0)
        {   
        	*str=0;
        	if(GetTextString (GetFocus(),str,128,Prompt,0,0,0,TRUE,TRUE))
        	{   
        		double 	factor = atof (str);
        		
        		if (!CurView->NewObjectMap[PickList[NumPicked-1].Desc])
        			newob = NextNewObject(TRUE);
        		else 
        			newob = CurView->NewObjectMap[PickList[NumPicked-1].Desc] - 1;
        		CurView->NewObjectMap[PickList[NumPicked-1].Desc] = newob + 1;
				if (Function == GF_SIZE_CLASSTEXT ||
					Function == GF_SIZE_CLASSTEXT_ALLVIS)
	    			CurView->NewObjectTextFactor[newob] *= factor;
				else
				{
					if (!CurView->NewObject[newob].Width)
						CurView->NewObject[newob].Width = -1;
					CurView->NewObject[newob].Width *= factor;
				}
			    RedisplayViewport(FALSE,FALSE);
	        }
        }
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (158);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL DeleteItem (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (742);
#endif
{HDC hDC;
 char key;     
 int	st,np, SaveNP;
 long	TLID;
 short	rtn=TRUE, SavePOH;
 POINT	MousePoint;
 DPOINT	BasePoint;
 short	OldType=12,NewType=92; 

 switch (Message)
   {
   	case GF_INIT:
   		switch (Function)
   		{ 
		    case GF_DELETE_ITEM:
	   			SetPrompt (PRMT_DELETE_ITEM,TRUE); 
		    	break;
		    case GF_REMOVE_ITEM:
	   			SetPrompt (PRMT_REMOVE_ITEM,TRUE); 
	   			break;
	   	}
   		SetPickCursor(TRUE);
       	AddLBUTTON = TRUE;    
       	GSSiGlobFree (&hLastDeleteList);
       	break;
    
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint);
	    setDoPaint( FALSE); 
	    PickItems (hWnd,BasePoint);  
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;  
		if (NumPicked && Function == GF_DELETE_ITEM && PickList[NumPicked-1].Blocked)
		{
			RemoveDummyDelete (NumPicked-1);
		}
	    else if (NumPicked && CurView->UpdateFile)
	    {
   			if (Function == GF_REMOVE_ITEM)
				DeletePickedItem (NumPicked-1,OldType,93);
   			else
				DeletePickedItem2 (NumPicked-1);
   		}
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (742);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (742);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL CopyPolylineWithOffset (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{  
 char key;   
 short	rtn=1;
 static	HIGHLIGHTDATA	HighlightData;
 long	Refno;
 
 switch (Message)
   {
   	case GF_INIT: 
        {
            DLGPROC lpfnCOPYLINEMsgProc;
            short	nRc; 
            
			if (BT_NUM_IN_INDEX (hHighlight) != 1) 
			{  
		ErrExit:  
				GSSiMessageBox (0,"One, and only one, polyline or area must be highlighted to redefine a polyline or polygon",0,MB_ICONEXCLAMATION,0);
				return FALSE;
			}
			BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
			if (HighlightData.PD.Type != 2 && HighlightData.PD.Type != 3)
				goto ErrExit;  
			if (!HaveUpdateFile ())
			{
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	            break;
	   		} 
	   		else
	   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
                  
			lpfnCOPYLINEMsgProc = MakeProcInstance((DLGPROC)COPYLINEMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"COPYLINE", hWnd, lpfnCOPYLINEMsgProc);
			FreeProcInstance(lpfnCOPYLINEMsgProc);
			if (!nRc)
				return FALSE;
			PostMessage(hWnd, GF_EXECUTE,0, 0L);
		}
   		break;

    case GF_EXECUTE:
		_fstrcpy (PltName,EditName);
		PltType = 2;
		{   
			int	nPnts2=2, i;
			HANDLE	hPnts2=GSSiGlobAlloc ( 624,GMEM_MOVEABLE,2*sizeof(DPOINT));
			HPDPOINT	pPoint2=(HPDPOINT)GlobalLock (hPnts2);  
			long	NewRefno;
			double	AZ = getazd (&HighlightData.PD.BeginPoint,&HighlightData.PD.EndPoint);
			
			AZ = LTWOPI(AZ - HALFPI);		
			for (i=0;i<NumPolyCopies;i++)
			{   
				pPoint2[0] = dnewpt (HighlightData.PD.BeginPoint,AZ,PolyCopyOffdist*(i+1));
				pPoint2[1] = dnewpt (HighlightData.PD.EndPoint,AZ,PolyCopyOffdist*(i+1));
				
				NewRefno = GetNewRefno (EditName,0,0,0,0);
				AddPolyToMap (1,&nPnts2, &hPnts2,1,NewRefno,0,-1,HighlightData.PD.Desc,0,HighlightData.PD.Prefix,HighlightData.PD.UDI,-1,-1,0,0,0,0,0,DigHiPrecis,0);
			} 
			GSSiGlobUlFree (&hPnts2);
		}
	    PostMessage(hWnd, GF_CLOSE,Function,0); 
        break; 
        
    default:
    	return (FALSE);
    }
    return rtn;
} 

BOOL LegendSetup (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{  
 POINT	MousePoint;  
 short pixdist = 4;
 double colordist = 5; 
 
 switch (Message)
   {
   	case GF_INIT:  
		lpfnLEGENDEDITMsgProc = (DLGPROC) MakeProcInstance((DLGPROC)LEGENDEDITMsgProc, hInst);
		CreateDialog(hInst, (LPSTR)"LEGENDEDIT",hWnd, lpfnLEGENDEDITMsgProc);
   		break;
   		
   	case WM_RBUTTONUP: 
		SetCurs ((HCURSOR)2,FALSE);
   		SetPrompt (PRMT_LOCATE_RADIUSP,TRUE);
		NumNewPolyPoints = 0;
		AddGraphicsFunction (hWnd, GF_ZOOM_VARRECT,0);
   		break;

    case GF_COMPLETE:
    	LegendBounds[iLegend] = ZoomBoxRect; 
   		break;
   		
    case WM_LBUTTONUP: 
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		GetFillPatternSignature (CurView->hDC,MousePoint,pixdist,colordist,&LegendSignature[iLegend]);
    	break;
    	
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL LegendSelect (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{  
	POINT	MousePoint;  
	FILLSIGNATURE Signature;
	
 switch (Message)
   {
   	case GF_INIT:  
		AddLBUTTON = TRUE; 
   		break;
   		
    case WM_LBUTTONUP: 
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		GetFillPatternSignature (CurView->hDC,MousePoint,1,0,&Signature);
		SelectLegend (&Signature);
    	break;
    	
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL EditCityLoc (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (152);
#endif
{
 POINT	MousePoint; 
 BOOL	Imediate; 
 static	long EditLoc=-1; 
 HFILE	Cities4FID;
 CITIESDATA4 Data;
 static	CITIESDATA4 EditData;
 DPOINT	BasePt;
 long	Loc;
 char	str[256];
 
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
       	EditLoc = -1;
        SetCurs (LoadCursor (hInst,IDC_ARROW),FALSE);
        break;
        
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		BasePt = ScreenPtToBasePt(MousePoint);
		if (EditLoc > -1)
		{ 
			Cities4FID = GSSiOpenFile ("[%CITYDATALOC]cities4.dat",0,OF_READWRITE); 
			GSSillseek (Cities4FID,EditLoc,0); 
			EditData.LatLong = BasePt;
			BigWrite (Cities4FID,(HPSTR)&EditData,sizeof(EditData),-1);
			GSSiClose (Cities4FID); 
			sprintf (str,"%s has been updated",EditData.Name);
			GSSiMsgBox (hWnd,str,"",MB_OK,0);
			EditLoc = -1;
			return (GF_INCREASE_SUCCESS_COUNT); 
		}  
		Cities4FID = GSSiOpenFile ("[%CITYDATALOC]cities4.dat",0,OF_READ);
		if (Cities4FID != HFILE_ERROR)
		{   
			double	dist,MinDist = DBL_MAX;
			
			Loc = 0;
			while (BigRead (Cities4FID,(HPSTR)&Data,sizeof(Data)) == sizeof(Data))
			{
				dist = ldistp (Data.LatLong,BasePt);
				if (dist < MinDist)
				{
					MinDist = dist;
					EditLoc = Loc; 
					EditData = Data;
				} 
				Loc = GSSillseek (Cities4FID,0,1);
			} 
			GSSiClose (Cities4FID); 
			sprintf (str,"Select new location for %s",EditData.Name);
			GSSiMsgBox (hWnd,str,"",MB_OK,0);
		}

		break;
    
    case GF_CLOSE:
    case GF_CANCEL:  
    	EditLoc = -1;
{
#if ENABLETRACE
GSSiExitProg (152);
#endif
    	return FALSE;
}
    	
    default:
{
#if ENABLETRACE
GSSiExitProg (152);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (152);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL RemovePolyLoop (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{
#define MAX_LOOPS	64
	short	st;
	DWORD	i,j,n; 
	HPDPOINT	pPoint;
	POINT	MousePoint, CursorPos;
	DPOINT	BasePoint;
	static	short	Step; 
	static	PICKDATA	SavePickList;
	static	HANDLE	hSavedScreen=0;
	static	long	BitmapID;  
	static	BOOL	PolyIsClosed;
	static	BOOL	Insert;
	static	short	nLoops=0, SelectedLoop = -1;
	static	HANDLE	hLoopPoints[MAX_LOOPS];
	static	int		nLoopPoints[MAX_LOOPS];
	
 char key;     
 switch (Message)
   {
   	case GF_INIT:
    	nLoops = 0; 
    	_fmemset (hLoopPoints,0,sizeof(hLoopPoints));
   		if (!CurView->UpdateFile)
   		{
	 		GSSiMsgBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK,0);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	    Step = 1;
       	AddLBUTTON = TRUE; 
   		SetPrompt (PRMT_CHANGE_POLY1,TRUE);  
   		break;
    
    case GF_CLOSE:  
    	AllowSinglePoint = FALSE;
    	DestroySavedScreen (&hSavedScreen,0);  
    	for (i=0;i<nLoops;i++)
    		GSSiGlobFree (&hLoopPoints[i]);
    	ShowNodesRef = LONG_MAX;
		return FALSE;
   	case GF_REDRAW: 
   	case GF_REDRAW_CMD: 
   		DestroySavedScreen (&hSavedScreen,0);
   		break;
    case GF_COMPLETE:   
/*    	AllowSinglePoint = FALSE;
    	if (NumNewPolyPoints)
    	{    
    		HPDPOINT	Points, NewPoints, OldPoints;
    		
		    ClearHighlightList (FALSE); 
       		GSSiGlobUlFree (&hUpdateMultiPolygon); 
		    nUpdatePolyPoints = NumNewPolyPoints ; 
       		hUpdatePoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,(nUpdatePolyPoints+1)*sizeof(DPOINT));  
       		Points = (HPDPOINT)GlobalLock (hUpdatePoly);
        	NewPoints = (HPDPOINT)GlobalLock (hNewPolyPoints); 
        	OldPoints = (HPDPOINT)GlobalLock (hPoly);
        	i = n = 0; 
        	if (SelectedNodes[0] > SelectedNodes[1])
        	{ 
    			for (j=0;j<NumNewPolyPoints;j++)
    				Points[n++] = NewPoints[j]; 
    			for (j=SelectedNodes[1];j<SelectedNodes[0];j++)
    				Points[n++] = OldPoints[j]; 
        	}
        	else
        	while (i<npnts)
        	{
        		if (i == SelectedNodes[0]) 
        		{
        			if (Insert)
        				Points[n++] = OldPoints[i++];
        			for (j=0;j<NumNewPolyPoints;j++)
        				Points[n++] = NewPoints[j]; 
        			i = SelectedNodes[1] + 1;
        		}
        		else
       				Points[n++] = OldPoints[i++];
        	} 
        	nUpdatePolyPoints = n;
        	if (PolyIsClosed)
        		Points[nUpdatePolyPoints++] = Points[0];
        	GlobalUnlock (hUpdatePoly);
        	NumNewPolyPoints=0;
        	GSSiGlobUlFree (&hNewPolyPoints);
        	GSSiGlobUlFree (&hPoly); 
			PickList[0] = SavePickList;
			UpdateItem=51;   
			UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,1,-1);
			GSSiGlobFree (&hUpdatePoly);  
        } */ 
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
        break; 
        
    
    case WM_RBUTTONUP:
  RBut:
		if (hSavedScreen)
			RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
		if (nLoops < 2) 
			break;
		{
			HANDLE	hRemovedLoop;
			int		nRemovedLoopPoints;
			short	nNewLoops=0;   
			long	NewRefno;
    			
			for (i=0;i<nLoops;i++)
			{
				if (i == SelectedLoop)
				{
					hRemovedLoop = hLoopPoints[i];
					nRemovedLoopPoints = nLoopPoints[i];
				}
				else
				{
					hLoopPoints[nNewLoops] = hLoopPoints[i];
					nLoopPoints[nNewLoops++] = nLoopPoints[i];
				}
			}
			CreateUpdatePolyFromLoops (nNewLoops,nLoopPoints,hLoopPoints);
	    	for (i=0;i<nNewLoops;i++)
    			GSSiGlobFree (&hLoopPoints[i]);
			PickList[0] = SavePickList;
			UpdateItem=51;   
			UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,1,-1);
			GSSiGlobFree (&hUpdatePoly);  
       		GSSiGlobFree (&hUpdateMultiPolygon); 
			NewRefno = GetNewRefno (EditName,0,0,0,0);
			AddPolyToMap (1,&nRemovedLoopPoints, &hRemovedLoop,0,NewRefno,0,-1,PickList[0].Desc,0,0,0,-1,-1,0,0,0,0,0,TRUE,0);
			GSSiGlobFree (&hRemovedLoop); 
			nLoops = 0; 
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
       	} 
		break;
		 
	case GF_USEPICKED:
	    BasePoint=PickList[0].PickedPoint; 
		NumPicked = 1;
	    goto UsePicked;
    case WM_LBUTTONUP:
    {
		short	Item; 
    	long	Ref; 

    	for (i=0;i<nLoops;i++)
    		GSSiGlobFree (&hLoopPoints[i]);
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    BasePoint=ScreenPtToBasePt(MousePoint); 
		switch (Step)
		{
			case 1:
			{
				short	SaveMaxPick = MaxPick; 
                
                Insert = FALSE;
				ClearHighlightList (FALSE);  
				if (hSavedScreen)
					RestoreScreen2 (CurView->hDC,hSavedScreen,BitmapID,FALSE);
				else
			        hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&BitmapID);
				MaxPick = 5;
			    PickItems (hWnd,BasePoint);  
			    MaxPick = SaveMaxPick;
UsePicked:
			    while (NumPicked--)
			    {
			    	if (PickList[NumPicked].Type == 3)
			    		break;
			    }
			    if (NumPicked < 0)  
			    	break; 
	       		SetPrompt (PRMT_REMOVE_LOOP2,TRUE);  
			    SavePickList = PickList[NumPicked];   
/*			    NumSelectedNodes = 0;
				AddToHighlightList (PickList[NumPicked].Refno,&PickList[NumPicked],TRUE);     
				ShowNodesRef = PickList[NumPicked].Refno;
			    ShowPickedItem (hWnd,NumPicked); */ 
				WantUnsplinedPoints = TRUE;
				nLoops = GetPolyPoints2 ((LPPICKDATAHEADER)&PickList[NumPicked],nLoopPoints,hLoopPoints,MAX_LOOPS);
				WantUnsplinedPoints = FALSE; 
				ClearHighlightList (FALSE);  
				SelectedLoop = -1;
				for (i=0;i<nLoops;i++)  
		    	{   
			    	HBRUSH	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
			        HBRUSH	hGreenBrush = CreateSolidBrush(RGB(  0, 255,   0));
					HBRUSH	OldBrush = SelectObject (CurView->hDC,hRedBrush); 
					HPDPOINT	pLoopPoints = (HPDPOINT)GlobalLock (hLoopPoints[i]);
					
					SaveDC (CurView->hDC);
					GSSiDeleteObject(&CurView->hRgn);
					CurView->hRgn = CreateVPRgn (FALSE,FALSE);
				  	SelectClipRgn (CurView->hDC,CurView->hRgn);
				  	GSSiDeleteObject(&CurView->hRgn);
			        //hSavedScreen = SaveScreen2 (CurView->hDC, CurView->Rect,CurView,&BitmapID); 
			        if (SelectedLoop == -1 && POINT_IN_AREAD (BasePoint, nLoopPoints[i],pLoopPoints,1,0,0,0)) 
			        {
			        	SelectedLoop = i;
                    	SelectObject (CurView->hDC,hGreenBrush);
                    } 
					GWPolygonD (CurView->hDC,pLoopPoints,nLoopPoints[i],1,0,0,FALSE,TRUE,0);
					GlobalUnlock (hLoopPoints[i]);  
					SelectObject (CurView->hDC,OldBrush);
					GSSiDeleteObject (&hRedBrush);  
					GSSiDeleteObject (&hGreenBrush);  
					RestoreDC (CurView->hDC,-1);
				}
			} 
			    
			break;
		}
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL SetLayerColor (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
#if ENABLETRACE
{GSSiEnterProg (742);
#endif
{
	COLORREF	NewColor;
 	POINT	MousePoint;
 	DPOINT	BasePoint;

 switch (Message)
   {
   	case GF_INIT:
		SetPrompt (PRMT_HIGHLIGHT,TRUE); 
   		SetPickCursor(TRUE);
       	AddLBUTTON = TRUE;
       	break;
    
    case WM_LBUTTONUP:
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
        BasePoint=ScreenPtToBasePt(MousePoint);
        PickItems (hWnd,BasePoint);
	case GF_USEPICKED:
		if (Message == GF_USEPICKED)
			NumPicked = 1;
        if (NumPicked > 0)
        {   
	   		SetConfig (PickList[NumPicked-1].ConfigID);
			SetViewport (PickList[NumPicked-1].ViewID);
        	if (Function == GF_REMOVE_LAYER_COLOR)
        		CurView->HaveLayerColor[PickList[NumPicked-1].FileNum] = 0;
        	else
        	{
	        	NewColor = CurView->LayerColor[PickList[NumPicked-1].FileNum];
	        	if(GetColor(hWnd,&NewColor))
	        	{   
	        		CurView->LayerColor[PickList[NumPicked-1].FileNum] = NewColor;
	        		CurView->HaveLayerColor[PickList[NumPicked-1].FileNum] = 1;
		        }
		    } 
			Pickability = FALSE;
		    TurnOffAutoVis (TRUE);		   
        }
		if (Message == GF_USEPICKED)
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (742);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (742);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 
BOOL ThemeShowClassMembers (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
 static POINT	MousePoint;
 static	HaveDown=FALSE;
 static HANDLE hSavedScreen = 0;
 static BOOL singleStep;
 static int currentRef, currentClass;
 HDC	hDC;
 char	str[260];
 HANDLE hMacro;
 LPSTR	pMacro;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=TRUE;
		singleStep = FALSE;
   		break;

    case GF_ENTER_VIEWPORT:
		ii=1;
    	break;
  	case GF_REDRAW: 
		if (hSavedScreen)
		{
			hDC = GetDC (hWnd);
			DestroySavedScreen (&hSavedScreen,0);
			ReleaseDC (hWnd,hDC);
		}
		if (singleStep)
		{
   			LPVIEWPORT savedVP = CurView;
			SetCurView (pViewports[CurTheme->TargetViewport-1]);  
			hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,CurView->DrawRect,0,0);
			GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn(FALSE,FALSE);
			SelectClipRgn (CurView->hDC,CurView->hRgn);
			GSSiDeleteObject(&CurView->hRgn);
			ShowClassMembers (CurTheme,currentClass,MousePoint,&currentRef,0);
			SetCurView (savedVP);
		}
		break;
   	case GF_CLEAR: 
		ii=1;
		break;
	case GF_CLOSE:
		if (hSavedScreen)
		{
			hDC = GetDC (hWnd);
 			RestoreScreen2 (hDC, hSavedScreen,0,FALSE);
			DestroySavedScreen (&hSavedScreen,0);
			ReleaseDC (hWnd,hDC);
		}
		break;
    case GF_EXIT_VIEWPORT:
		ii=1;
		break;
    case WM_LBUTTONUP:
		HaveDown = FALSE;
		if (hSavedScreen && !singleStep)
		{
			hDC = GetDC (hWnd);
 			RestoreScreen2 (hDC, hSavedScreen,0,FALSE);
			DestroySavedScreen (&hSavedScreen,0);
			ReleaseDC (hWnd,hDC);
		}
   	break;
    
    case WM_RBUTTONUP:
		GetGFFile (str,CurTheme->SQL,2); 
		GMEdit (hWnd,str);
    	break;
    		
    case WM_LBUTTONDOWN:
	{	 
		int		iclass, isym, rtn=0; 
		
      	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
		if (hSavedScreen && !singleStep)
		{
			hDC = GetDC (hWnd);
 			RestoreScreen2 (hDC, hSavedScreen,0,FALSE);
			DestroySavedScreen (&hSavedScreen,0);
			ReleaseDC (hWnd,hDC);
		}
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		if (hSavedScreen)
		{
			hDC = GetDC (hWnd);
			RestoreScreen2 (hDC, hSavedScreen,0,FALSE);
			ReleaseDC (hWnd,hDC);
		}
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{  
				if (singleStep && iclass == currentClass)
				{
	    			LPVIEWPORT savedVP = CurView;

					hMacro = GSSiGlobAlloc (1783,GMEM_MOVEABLE,4096);
					pMacro = GlobalLock (hMacro);

					GetGlobalCVal ("%SHOWCLASSMEMBERMACRO",pMacro,0);
					SetCurView (pViewports[CurTheme->TargetViewport-1]);  

					GSSiDeleteObject(&CurView->hRgn);
					CurView->hRgn = CreateVPRgn(FALSE,FALSE);
					SelectClipRgn (CurView->hDC,CurView->hRgn);
					GSSiDeleteObject(&CurView->hRgn);
					currentRef++;
					if (!ShowClassMembers (CurTheme,iclass,MousePoint,&currentRef,pMacro))
					{
						hDC = GetDC (hWnd);
						RestoreScreen2 (hDC, hSavedScreen,0,FALSE);
						DestroySavedScreen (&hSavedScreen,0);
						ReleaseDC (hWnd,hDC);
						singleStep = FALSE;
					}
					GSSiGlobUlFree (&hMacro);
					SetCurView (savedVP);
				}
				else
				{
					if (CurTheme->TargetViewport)
					{
		    			LPVIEWPORT savedVP = CurView;

						currentClass = iclass;
						singleStep = (wParam == (MK_LBUTTON|MK_SHIFT));
						SetCurView (pViewports[CurTheme->TargetViewport-1]);  

		    			hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,CurView->DrawRect,0,0);
						GSSiDeleteObject(&CurView->hRgn);
						CurView->hRgn = CreateVPRgn(FALSE,FALSE);
						SelectClipRgn (CurView->hDC,CurView->hRgn);
						GSSiDeleteObject(&CurView->hRgn);
						if (singleStep)
						{
							currentRef = LONG_MIN;
							hMacro = GSSiGlobAlloc (1783,GMEM_MOVEABLE,4096);
							pMacro = GlobalLock (hMacro);
							GetGlobalCVal ("%SHOWCLASSMEMBERMACRO",pMacro,0);
						}
						else
						{
							currentRef = LONG_MAX;
							pMacro = 0;
							hMacro = 0;
						}
						if (!ShowClassMembers (CurTheme,iclass,MousePoint,&currentRef,pMacro))
						{
							hDC = GetDC (hWnd);
							RestoreScreen2 (hDC, hSavedScreen,0,FALSE);
							DestroySavedScreen (&hSavedScreen,0);
							ReleaseDC (hWnd,hDC);
							singleStep = FALSE;
						}
						GSSiGlobUlFree (&hMacro);
						SetCurView (savedVP);
					}
				}
				break;
			}
		}
		//ThemeDisplayLegend(3,0);
	}
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL TraceDownstream (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{    
	char	str[128];  
	static	short	np;
	static	DPOINT	Points[32];
	
 switch (Message)
   {
   	case GF_INIT:  
   		np=0;
       	AddLBUTTON = FALSE; 
   		break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINTS	MousePoint;
		POINT	FilePoint;
		DPOINT	BasePoint;
    	short	ID;  
    	HFILE	Fid; 
		OFSTRUCTGM	OFStruct;

 		if (CursorIsLocked)  
 		{
 		 	BasePoint = CurrentPoint;
	    	UnlockCursor ();
	    }
	    else
	    { 
	    	MousePoint = MAKEPOINTS(lParam);
		    BasePoint=WinPtSToBasePt(MousePoint);
		} 
		Points[np++] = BasePoint;
	}
		break; 
//		GetDTMGridAroundPoint (BasePoint);
//		GetDTMGridTriangleAroundPoint (BasePoint);
//		GetDTMGridTriangleSlope (
    case WM_RBUTTONUP:
    {
		FindBasinsAroundPoints ("e:\\mpls\\surfaces\\lidar.ldr",np, Points);
		break;
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL ThemeToggleClassStatus (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1232);
#endif
{
 POINT	MousePoint;
 static	BOOL	HaveDown=FALSE;
 static	int		LastClass;
 int	iclass;
 char	str[260];

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		HaveDown=FALSE;
		LastClass = -1;
   		break;

	case WM_LBUTTONDBLCLK:
		LastClass = -1;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
     	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   
				sprintf (str,"%i(%i)",GF_SET_CLASS_SYMBOL,iclass);
				AddGraphicsCmd (hWnd,str,FALSE,0);
			}
		}
		goto UndoLast;
		break;

	case WM_MBUTTONDOWN:
		RedisplayViewports (FALSE);
		break;

    case WM_RBUTTONDOWN:
		LastClass = -1;
    	HaveDown = TRUE;
		break;

    case WM_LBUTTONDOWN:
    	HaveDown = TRUE;
     	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
		LastClass = -1;
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   
				LastClass = iclass;
	        	if (CurTheme->ClassStatus[iclass])
	        		CurTheme->ClassStatus[iclass] = FALSE;
	        	else
	        		CurTheme->ClassStatus[iclass] = TRUE;
				ThemeDisplayLegend(3,0);
				break;
			}
		}
   		break;
    
    case WM_MOUSEMOVE:
    	if (!HaveDown)
			break;
     	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{  
				if (iclass == LastClass)
					break;
				LastClass = iclass;
	        	if (CurTheme->ClassStatus[iclass])
	        		CurTheme->ClassStatus[iclass] = FALSE;
	        	else
	        		CurTheme->ClassStatus[iclass] = TRUE;
				ThemeDisplayLegend(3,0);
				break;
			}
		}
   		break;
    
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
	{	 
    	if (!HaveDown) break;
UndoLast:
    	HaveDown = FALSE;
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	    
	    NumPicked = 0;
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{	
			if (PickThemeClass(iclass,MousePoint)) 
			{   
				if (iclass == LastClass)
					break;
	        	if (CurTheme->ClassStatus[iclass])
	        		CurTheme->ClassStatus[iclass] = FALSE;
	        	else
	        		CurTheme->ClassStatus[iclass] = TRUE;
	        	if (Message == WM_RBUTTONUP)
	        	{
	        		BOOL Stat=CurTheme->ClassStatus[iclass];
	        		
					for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	        			CurTheme->ClassStatus[iclass] = Stat; 
	        	}
				ThemeDisplayLegend(3,0);
				break;
			}
		}
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1232);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL SetPNParms (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 char key;     
 char	str[128];
 static POINT	MousePoint;
 DPOINT	BasePoint; 
 COLORREF	Color;
 static	BOOL	ColorLocked;

 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,0, 0L);
		ColorLocked = FALSE;
   		break;

	case GF_EXECUTE:
		if (!hWndPNParms)
		{
		                  
		  lpfnPNPARMSMsgProc = MakeProcInstance((DLGPROC)PNPARMSMsgProc, hInst);
		  hWndTraverseEntry=CreateDialog(hInst,"PNPARMS",hWnd, lpfnPNPARMSMsgProc);
		}
		else
			ShowWindow (hWndPNParms,SW_RESTORE);

		break;
    
    case WM_CHAR:
    {
		switch (wParam)
		{   
			case 'C':
			case 'c': 
				BasePoint =  ScreenPtToBasePt (MousePoint);       
				ftoa (str,BasePoint.x);
            	SetDlgItemText (hWndPNParms,IDC_LONGITUDE,str);
				ftoa (str,BasePoint.y);
            	SetDlgItemText (hWndPNParms,IDC_LATITUDE,str);
            break;

            default:
            	return FALSE;
            break;
		}  
		return TRUE;
	}
    case WM_MOUSEMOVE:   
    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam)); 
    	if (ColorLocked)
    		break;
    	hDC = GetDC (hWnd);
    	Color = GetPixel (hDC,MousePoint.x,MousePoint.y); 
    	ReleaseDC (hWnd,hDC);  
    	PostMessage(hWndPNParms, WM_COMMAND, IDC_SHOWCOLOR, Color); 
    	break;  
    case WM_LBUTTONDOWN:
    	break;
    case WM_LBUTTONUP:
    	if (ColorLocked)
    		ColorLocked = FALSE;
    	else
    		ColorLocked = TRUE;
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}