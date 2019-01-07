#include "graphint.h"

#include "gmextern.h"

static LPREORG	pReorg;
static BOOL	CursorIsGlobal=FALSE;
static long	MaxSegsPerQuad=200;
static long	NumSegsPerQuad;
static short	NumQuadLevels;
static short	MinReorgDesc=20;
static char	ReorgName[128];
static HANDLE	hReorgBuf=0;
static long	lReorgBuf;
static long	TotReorgDesc=0;
static HANDLE	hReorgDesc=0;
static HANDLE	hDescBlockReorg=0;
static long	NumMapSegments=0;
static short	NumReorgDesc=0;
static short	WantDesc;
static HFILE	ReorgDescFile=0;
static HANDLE	hReorgDescFile=0;
static short	LastWhich=-1;
static LPDESCBLOCK	pDescBlock;
static RECT	PreIconicMainRect, PreIconicConfigDisplayRect;
static RECT	CurrentBufferRect={0,0,0,0}; 
static BOOL	AlreadyProcessed;
void SetIconicRect (RECT MR,RECT CDR)
{   
	return;
	PreIconicMainRect = MR;
	PreIconicConfigDisplayRect = CDR;
	return;
}

/*BOOL DisplayHelp (HWND hWnd,LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (790);
#endif
{   HINSTANCE HI;

	HI = ShellExecute (hWnd,0,File,0,0,SW_SHOWMAXIMIZED);
	if (HI < 32)
{
#if ENABLETRACE
GSSiExitProg (790);
#endif
		return FALSE;
}
	else
{
#if ENABLETRACE
GSSiExitProg (790);
#endif
		return TRUE;
}
#if ENABLETRACE
}
#endif
}*/

HANDLE GetMultiFile (HWND hWnd,LPSTR Ext,LPSTR StartDir,LPLONG pTotFiles)
#if ENABLETRACE
{GSSiEnterProg (791);
#endif
{
	short	nRc; 
	LPSTR	pDir;
	DLGPROC lpfnMULTIFILEMsgProc;  
	char	CurDir[MAX_PATH];
	int		SaveDrive;     
	HANDLE	hName=0;
	LPSTR	pName;   
	OFSTRUCTGM	OFStruct;
	
  	_getcwd (CurDir,MAX_PATH); 
  	SaveDrive = _getdrive();
	hDir = GSSiGlobAlloc ( 495,GMEM_MOVEABLE,256);
	pDir = GlobalLock (hDir);  
	_fstrcpy (pDir,StartDir);
	GlobalUnlock (hDir);
	_fstrcpy (MFExt,Ext);
	hName = GSSiGlobAlloc ( 496,GMEM_MOVEABLE,256);
	pName = GlobalLock (hName);
	GSSiGetTempFileName (0,"gmm",0,pName);
    GlobalUnlock (hName);  
    hMFName = hName;
	lpfnMULTIFILEMsgProc = MakeProcInstance((DLGPROC)MULTIFILEMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"MULTIFILE", hWnd, (DLGPROC)lpfnMULTIFILEMsgProc);
	FreeProcInstance(lpfnMULTIFILEMsgProc); 
	GSSiGlobUlFree (&hDir);
    _chdir (CurDir);
    _chdrive (SaveDrive);
    if (!nRc)
    	GSSiGlobFree (&hName);  
    *pTotFiles = TotFiles;
{
#if ENABLETRACE
GSSiExitProg (791);
#endif
	return hName;
}
#if ENABLETRACE
}
#endif
}

    

HDC HaveScreenBuffer (LPRECT pRect)
{
	if (!hDCScreenBuffer)
		return 0;
	if (pRect)
		*pRect = CurrentBufferRect;
	return hDCScreenBuffer;
}

HDC ScreenBufferDC (HWND hWnd,HDC hDC)
{   
	RECT	Rect;
	HBITMAP	hbmpOld;
	static	HBITMAP	hbmpOrig=0;
	HDC		hDCMain;
	int		ii;
	
	if (hWnd == (HWND)1)
	{ 
		if (hbmpOrig)
		{
			hbmpOld = SelectObject (hDCScreenBuffer,hbmpOrig);
			GSSiDeleteObject (&hbmpOld); 
			DeleteDC (hDCScreenBuffer);  
			hDCScreenBuffer = 0;
			hbmpOrig = 0;
			SetConfig(1);
			if (*pNumViewports)
			{
				SetViewport(*pCommandViewport);
				hDCMain = GetDC(hWndMain);
				SetupViewports(CurView->hWnd, hDCMain, 0, MainRect, 0);
				ReleaseDC(hWndMain, hDCMain);
			}
		}
		return 0;
	}

	if (!BufferedScreen)
		return hDC;    
	GetClientRect(hWnd, &Rect);
	if (MapServer)
	{
		hWnd = GetDesktopWindow();
		hDCMain = GetDC(hWnd);
	}
	else
		hDCMain = GetDC(hWndMain);
	if (!hDCScreenBuffer)
	{
		CurrentBufferRect.left=CurrentBufferRect.right=CurrentBufferRect.top=CurrentBufferRect.bottom=0;
		hDCScreenBuffer = CreateCompatibleDC(hDCMain);    
	}
	if (MapServer || (!IsIconic (hWndMain) && !EqualRect (&Rect,&CurrentBufferRect)))
	{
		BITMAP bm;
		CurrentBufferRect = Rect;
		hBitmapScreenBuffer = CreateCompatibleBitmap (hDCMain,RECTWIDTH(&CurrentBufferRect),   
															  RECTHEIGHT(&CurrentBufferRect)); 
		if (dbug)
		{
			char mess[128];
			HBITMAP hcbm = CreateCompatibleBitmap(hDCMain, 10, 10);
			sprintf(mess, "%i %i %i %i", Rect.left, Rect.top, Rect.right, Rect.bottom);
			MessageBox(0, mess, "", MB_OK);
			GetObject(hcbm, sizeof(BITMAP), &bm);
			sprintf(mess, "SB bitmap %i %i %i %i %i %i %i", CurView->ID, bm.bmHeight, bm.bmWidth, bm.bmBitsPixel, (int)CurView->hDC, (int)hDCMain, (int)hDCScreenBuffer);
			MessageBox(0, mess, "", MB_OK);
			GetObject(hBitmapScreenBuffer, sizeof(BITMAP), &bm);
			sprintf(mess, "SB bitmap %i %i %i %i %i %i %i", CurView->ID, bm.bmHeight, bm.bmWidth, bm.bmBitsPixel, (int)CurView->hDC, (int)hDCMain, (int)hDCScreenBuffer);
			MessageBox(0, mess, "", MB_OK);
		}
		hbmpOld = SelectObject(hDCScreenBuffer, hBitmapScreenBuffer);
		if (hbmpOrig)
			GSSiDeleteObject (&hbmpOld);
		else
			hMapServerBM = hbmpOrig = hbmpOld;
	}
	if (hDCMain == hDC)  
	{
		SelectClipRgn (hDC,0);
		SelectClipRgn (hDCScreenBuffer,0);
		ii=BitBlt(hDCScreenBuffer, 0, 0, Rect.right-Rect.left+1,
                             Rect.bottom-Rect.top+1,
               hDC, Rect.left,Rect.top, SRCCOPY); 
        SetIconicRect (MainRect,ConfigDisplayRect);
    	//ShowWindow (hWndMain,SW_HIDE);//SHOWMINIMIZED); 
    }
	if (MapServer)
		ReleaseDC(hWnd, hDCMain);
	else
	    ReleaseDC (hWndMain,hDCMain); 
	return hDCScreenBuffer;
}

BOOL SetupViewports (HWND hWnd, HDC hDC, short CurParent, RECT Rect,int Band)
#if ENABLETRACE
{GSSiEnterProg (793);
#endif
{   short iv, st,Pass=0;
    LPVIEWPORT SaveCurView; 
    RECT	NewRect;
    double	pw, ph;  
    BOOL	rtn=FALSE, ShrinkToFit=FALSE; 
    long	rect_width, rect_height, destH, destW;
	LPRECT	pRect=0;

	if (!CurParent)
		ii=1;
	if (Band < 0)
	{
		pRect = &Rect;
		Band = 0;
	}
    
    SaveCurView = CurView;  
Top:
    for (Pass=0;Pass<2;Pass++)
    {
	    if (!CurParent)
	    {   
	    	hDC = ScreenBufferDC (hWnd,hDC);   
	    	SetROP2 (hDC,R2_COPYPEN);
	    	SetCurView ( pViewportsD[0]);
	    	if (CurView->WidthType == 1)
	    	{
				if (CurView->ShowFullScreen)
					UseFullScreen (hWnd,pRect);
		    	if ((!Printing || CurView->AutoSize) || FileMode || ShrinkToFit) //fit to window using margins
		    	{
					double	w = CurView->DesiredWidth;
					double	h = CurView->DesiredHeight;
					RECT	SavePromptRect;
					char	SavePrintPrompt[512];

					if (!Pass && PrinterHeight)
					{
						w = PrinterWidth;
						h = PrinterHeight;
					}
					rect_width = (long)Rect.right - (long)Rect.left + 1;
					rect_height = (long)Rect.bottom - (long)Rect.top + 1;
					pw = w / rect_width;
					ph = h / rect_height; 
					NewRect = Rect;
					if (ph > pw)
					{
						destH = rect_height;
						destW = (IDNINT(destH * (w /h )));
		         		NewRect.left = (Rect.left + (rect_width - destW) / 2); 
		         		NewRect.right = NewRect.left + destW -1;
		         	}
		         	else 
					{
						destW = rect_width;
						destH = (IDNINT(destW * (h /w)));
						NewRect.top = (Rect.top + (rect_height - destH) / 2);
						NewRect.bottom = NewRect.top + destH;
					} 
					if (!Pass && PrinterHeight)
					{
						int	rw = NewRect.right - NewRect.left;
						int	rh = NewRect.bottom - NewRect.top;

						NewRect.left += IDNINT ((PrinterMarginLeft / PrinterWidth) * rw);
						NewRect.right -= IDNINT ((PrinterMarginRight / PrinterWidth) * rw);
						NewRect.top += IDNINT ((PrinterMarginTop / PrinterHeight) * rh);
						NewRect.bottom -= IDNINT ((PrinterMarginBottom / PrinterHeight) * rh);
					}
					CurView->Width = CurView->Height = 0; 
					Rect = NewRect;
					SavePromptRect = PromptRect;
					GetGlobalCVal ("[%PRINTPROMPT]",SavePrintPrompt,0);
					SetGlobalValue ("%PRINTPROMPT",""); 
		    		SetMainRect (0,hDC,&NewRect,6);
					SetGlobalValue ("%PRINTPROMPT",SavePrintPrompt);
					PromptRect = SavePromptRect;
		    	}
		    }
	    } 
	    for (iv = 0; iv<*pNumViewports; iv++)
	    {
	        SetCurView ( pViewportsD[iv]);
	        CurView->hWnd = hWnd;
	        CurView->hDC = hDC;
	        if (CurView->Parent == CurParent)
	        {   
	        	if (CurView->Parent && CurView->DisplayInParent)
	        	{
	        		CurView->TagPoint.x = CurView->TagPoint.y = 0;
	        		CurView->Width = CurView->Height = 100;
	        	}
	        	if (CurView->Parent && CurView->ShrinkParent)
	        	{
	        		if (Pass)
	        			st = 1;
	        		else
	            		st = SetupViewport (pViewports[CurView->Parent-1]->Rect,ShrinkToFit,Band);
	            }
	            else 
	            	st = SetupViewport (Rect,ShrinkToFit,Band); 
	            if (!st)
	            	goto Exit;
	            if (!iv && (!pViewportsD[0]->Type || !_fstricmp (pViewportsD[0]->Name,"Format")) && pViewportsD[0]->DesiredWidth)
		    		DevicePixelsPerInch = ((double)pViewportsD[0]->DrawRect.right-pViewportsD[0]->DrawRect.left)/pViewportsD[0]->DesiredWidth;
	            if (st < 0)
	            {
	            	ShrinkToFit = TRUE;
	            	goto Top;
	            }
	            if (!SetupViewports (hWnd, hDC, CurView->ID,CurView->DrawRect,0)) //ch 020506
	            	goto Exit;
	        }
	    } 
	    if (!Pass)
	    for (iv = 0; iv<*pNumViewports; iv++)
	    {
	        SetCurView ( pViewportsD[iv]);  
	        if (VPIsActive (CurView->ID))
	        {
		        if (CurView->Parent && CurView->Parent == CurParent && CurView->ShrinkParent) 
		        { 
		        	RECT	NewRect; 
			        	
		        	SubtractRect (&NewRect,&pViewports[CurParent-1]->Rect,&CurView->Rect);
		        	if (NewRect.left != pViewports[CurParent-1]->Rect.left)
		        		pViewports[CurParent-1]->Rect.left = NewRect.left+1;
		        	if (NewRect.right != pViewports[CurParent-1]->Rect.right)
		        		pViewports[CurParent-1]->Rect.right = NewRect.right-1;
		        	if (NewRect.top != pViewports[CurParent-1]->Rect.top)
		        		pViewports[CurParent-1]->Rect.top = NewRect.top+1;
		        	if (NewRect.bottom != pViewports[CurParent-1]->Rect.bottom)
		        		pViewports[CurParent-1]->Rect.bottom = NewRect.bottom-1;
		        	//pViewports[CurParent-1]->DrawRect = NewRect;
	        		pViewports[CurParent-1]->DrawRect = PctRect (pViewports[CurParent-1]->Rect,-(max(0,pViewports[CurParent-1]->Margin)));
			        if (pViewports[CurParent-1]->BorderPct >= 0)
			        	InflateRect (&pViewports[CurParent-1]->DrawRect,-1,-1); 
			        Rect = pViewports[CurParent-1]->ScreenRect = pViewports[CurParent-1]->DrawRect;
		        } 
			}  
		}
	}
    if ((Printing || GetGlobalBVal ("[%NOTOOLBAR]")) && !CurParent)
    	RemoveToolbarViewports (hWnd,hDC);
    rtn = TRUE; 
Exit:
    SetCurView ( SaveCurView);
{
#if ENABLETRACE
GSSiExitProg (793);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

void AdjustWBoundsToTile (void)
{   
	RECT	ClientRect; 
	MNMXCORD Bounds;
	
	/*if (InVirtualPrint)
	{  
   		ClientRect.left = ClientRect.top = 0;
   		ClientRect.bottom = VirtualPageHeight; 
   		ClientRect.right = VirtualPageWidth; 
	}
	else*/
	{
		if (!CurrentConfig || !InPlotView || !FormatWidth)
			return;  
		if (IsRectEmpty (&ConfigDisplayRect))
			GetClientRect (CurView->hWnd,&ClientRect);
		else
			ClientRect = ConfigDisplayRect; 
	}
	WinRectToBounds (&ClientRect, &Bounds);
	CurView->WBounds.xmn = max (CurView->WBounds.xmn,Bounds.xmn);
	CurView->WBounds.ymn = max (CurView->WBounds.ymn,Bounds.ymn);
	CurView->WBounds.xmx = min (CurView->WBounds.xmx,Bounds.xmx);
	CurView->WBounds.ymx = min (CurView->WBounds.ymx,Bounds.ymx);
	return;
}  

void AdjustMainRect (HWND hWnd,HDC hDC,LPRECT Rect)
{   
	double factor=1;  
	double	w;
	long	RectW, RectH, Shift;
	
	MainClipRect = *Rect;
	if (!CurrentConfig || !InPlotView || !FormatWidth)
		return; 
	PlotPageWidth = Rect->right - Rect->left;
	PlotPageHeight = Rect->bottom - Rect->top; 
 	w = GetGlobalDVal2 ("[%SCREENWIDTH]",(double)GetDeviceCaps(hDC, HORZSIZE) * MFT/100); 
    w *= (Rect->right - Rect->left) / (double)GetDeviceCaps(hDC,HORZRES); 
    factor = FormatWidth/w;  
	Rect->right = Rect->left + (Rect->right - Rect->left) * factor;
	Rect->bottom = Rect->top + (Rect->right - Rect->left) * (FormatHeight/FormatWidth); 
	PlotScrollWidth = (Rect->right - Rect->left) - PlotPageWidth;
	PlotScrollHeight = (Rect->bottom - Rect->top) - PlotPageHeight;
	if (PlotScrollWidth > 0)
		SetScrollRange(hWnd, SB_HORZ, 0, PlotScrollWidth, FALSE);  
	else
		ShowScrollBar(hWnd, SB_HORZ,FALSE);
	if (PlotScrollHeight > 0)
		SetScrollRange(hWnd, SB_VERT, 0, PlotScrollHeight, FALSE);  
	else
		ShowScrollBar(hWnd, SB_VERT,FALSE);   
	
	if (InPlotView == 1)
	{   
		sVPos = PlotScrollHeight/2;
		sHPos = PlotScrollWidth/2;
		InPlotView = 2;
        SetScrollPos(hWnd, SB_VERT, sVPos, TRUE);
        SetScrollPos(hWnd, SB_HORZ, sHPos, TRUE);
	}
	RectW = Rect->right - Rect->left;
	RectH = Rect->bottom - Rect->top;
	if (PlotScrollWidth > 0) 
	{
		Shift = ((double)sHPos / PlotScrollWidth) * (RectW-PlotPageWidth);
		Rect->left -= Shift;
		Rect->right -= Shift; 
	}
	if (PlotScrollHeight > 0)
	{
		Shift = ((double)sVPos / PlotScrollHeight) * (RectH-PlotPageHeight);
		Rect->top -= Shift;
		Rect->bottom -= Shift;
	}
	return;
}
static float deviceToScreenFactor = 1;
double AdjustWidth(double width)
{
	double rtn = width;

	if (rtn < 0)
	{
		if (PRJ_UNITS[1] == 4)
		{
			DPOINT Pt = NewLatLong(CurView->MidPointW.y, CurView->MidPointW.x, width, HALFPI / 2);

			rtn = ldistp(CurView->MidPointW, Pt)* BaseDistToWinDist;
		}
		else
			rtn = -width * BaseDistToWinDist;
	}
	else
		rtn *= DeviceToScreenFactor();
	return rtn;
}
float DeviceToScreenFactor(void)
{
	return deviceToScreenFactor;
}
float setDeviceToScreenFactor(float v)
{
	float rtn = deviceToScreenFactor;
	deviceToScreenFactor = v;
	if (v != 1.0)
		ii = 1;
	return rtn;
}
void SetMainRect (HWND hWnd, HDC hDC, LPRECT RectIn,int From)
#if ENABLETRACE
{GSSiEnterProg (796);
#endif
{   
	static int ScreenRes=0;   
   	short	PRHeight;  
   	RECT	Rect;
	
	if (FileMode)  
		DeviceRes = ScreenRes;
	else
	{
		if (!ScreenRes)
			ScreenRes = GetDeviceCaps(hDC, LOGPIXELSX);  
		if (Printing)
			DeviceRes = (double)GetDeviceCaps(hDCPrinter, LOGPIXELSX);
		else
			DeviceRes = GetDeviceCaps(hDC, LOGPIXELSX);
	} 
	if (MemMap)
		setDeviceToScreenFactor(DeviceToScreenFactorMemMap);
	else if (Printing && PrinterIsVirtual)
    	setDeviceToScreenFactor((double)VirtualPrintDPI/(double)ScreenRes);
	else if (ScreenRes)
    	setDeviceToScreenFactor( (double)DeviceRes/(double)ScreenRes);
    else
    	setDeviceToScreenFactor(1);
	if (RectIn) 
	{   
		Rect =*RectIn;  
		Rect.left += DeviceRes * GetGlobalDVal2 ("[%MARGIN_LEFT]",0.0);
		Rect.right -= DeviceRes * GetGlobalDVal2 ("[%MARGIN_RIGHT]",0.0);
		Rect.top += DeviceRes * GetGlobalDVal2 ("[%MARGIN_TOP]",0.0);
		Rect.bottom -= DeviceRes * GetGlobalDVal2 ("[%MARGIN_BOTTOM]",0.0);
		LastMainRect = Rect;
		MainRect = Rect;
	} 
    else if (MemMap) 
    {
        MainRect.left=0;
        MainRect.top=0;
        MainRect.bottom = MemMapHeight;
        MainRect.right = MemMapWidth;
    }
    else if (SetDimensions)
    	MainRect = SetDimRect;
    else if (hWnd && !CurrentConfig || From == 1 || IsRectEmpty (&ConfigDisplayRect))
    {   
    	if (IsIconic (hWndMain))
    	{
    		MainRect = PreIconicMainRect;
    		ConfigDisplayRect = PreIconicConfigDisplayRect;
    	}
    	else if (hWnd)
    	{
	    	RECT	MRect;
	    	
	        GetClientRect(hWnd, &MRect);
			if (DoScreenPrompt)
			{   
	        	
	        	PromptRect = MRect;
	        	PRHeight = GetGlobalLVal2 ("[%PROMPTHEIGHT]",14); 
	        	PromptRect.top = PromptRect.bottom-PRHeight;
	        	MRect.bottom -= (PromptRect.bottom - PromptRect.top + 1);
			}
			if (AdjustMainRectToMenus (&MRect,&ConfigDisplayRect))
		    	MainRect = ConfigDisplayRect;
			else
			{
				MainRect = MRect;
				_fmemset (&ConfigDisplayRect,0,sizeof(RECT));
				MainRect.right--;
				MainRect.bottom--;
			}
    		PreIconicMainRect = MainRect;
    		PreIconicConfigDisplayRect = ConfigDisplayRect;
	    }
    }
    else 
	{
		if (PrinterHeight)
		{
			double	w = ConfigDisplayRect.right - ConfigDisplayRect.left, wnew;
			double	h = ConfigDisplayRect.bottom - ConfigDisplayRect.top, hnew;

			if (h * PrinterWidth / PrinterHeight < w)
			{
				wnew = h * PrinterWidth / PrinterHeight;
				ConfigDisplayRect.left += (w - wnew)/2;
				ConfigDisplayRect.right -= (w - wnew)/2;
			}
			else
			{
				hnew = w * PrinterHeight / PrinterWidth;
				ConfigDisplayRect.top += (h - hnew)/2;
				ConfigDisplayRect.bottom -= (h - hnew)/2;
			}
		}
    	MainRect = ConfigDisplayRect;
	}
	if (CurrentConfig && !RectIn)
	{
		if (From > 0)
			RemoveToolBarsFromMainRect (&MainRect);
		ConfigDisplayRect = MainRect; //problem
	}
	if (!RectIn)
		AdjustMainRect (hWnd,hDC,&MainRect);  
	if (!Printing && CurrentConfig)
	{   
		RECT	SaveRect = MainRect;
		
		setDeviceToScreenFactor(DeviceToScreenFactor()*ScreenWindowFactor);
		if (hWnd)
		{
			MainRect.left *= ScreenWindowFactor;
			MainRect.top *= ScreenWindowFactor;
			MainRect.right = MainRect.left + (SaveRect.right - SaveRect.left) * ScreenWindowFactor;
			MainRect.bottom = MainRect.top + (SaveRect.bottom - SaveRect.top) * ScreenWindowFactor;
		}
	}
    if (Printing && !MemMap)
    {   
    	HANDLE	hMem=GSSiGlobAlloc ( 498,GMEM_MOVEABLE,256);
    	LPSTR	txt = GlobalLock (hMem);
    	
    	GetGlobalCVal ("[%PRINTPROMPT]",txt,0);
    	if (*txt)
    	{
        	PromptRect = MainRect;
        	PRHeight = DeviceToScreenFactor() * GetGlobalLVal2 ("[%PRINTPROMPTHEIGHT]",12); 
        	PromptRect.top = PromptRect.bottom-PRHeight;
        	MainRect.bottom -= (PromptRect.bottom - PromptRect.top + 1);
    	}
		if (RectIn) 
			*RectIn = MainRect; 
		GSSiGlobUlFree (&hMem);
		MainClipRect = MainRect;
    } 
    if (MemMap)
		MainClipRect = MainRect;
    	
	MaxDimension = max (MainRect.right-MainRect.left,MainRect.bottom - MainRect.top);
	ShadowInc = IDNINT ((MaxDimension * ShadowPct)/100); 
    if ( (!pViewportsD[FormatViewport]->Type || !_fstricmp (pViewportsD[FormatViewport]->Name,"Format")) && pViewportsD[FormatViewport]->DesiredWidth)
    {   
    	if (PrinterHeight > 0 && pViewportsD[0]->AutoSize && (!ShowVirtualPrintAreas || Printing))
    	{  
			BOOL disable=TRUE;

	    	pViewportsD[0]->DesiredWidth = PrinterWidth - PrinterMarginLeft - PrinterMarginRight;
	    	pViewportsD[0]->DesiredHeight = PrinterHeight - PrinterMarginTop - PrinterMarginBottom; 
			pViewportsD[0]->TagPoint.x = 0; 
			pViewportsD[0]->TagPoint.y = 0; 
	    	if (!disable && pViewportsD[0]->WBoundsWhenSaved.xmx && pViewportsD[0]->NumFiles)
	    	{
	    		double factor1 = (pViewportsD[0]->WBoundsWhenSaved.xmx - pViewportsD[0]->WBoundsWhenSaved.xmn) /
	    					    (pViewportsD[0]->WBoundsWhenSaved.ymx - pViewportsD[0]->WBoundsWhenSaved.ymn); 
	    		double factor2 = pViewportsD[0]->DesiredWidth / pViewportsD[0]->DesiredHeight;
	    		
	    		if (factor1 < factor2)
	    			pViewportsD[0]->DesiredWidth = pViewportsD[0]->DesiredHeight* factor1;
	    		else
	    			pViewportsD[0]->DesiredHeight = pViewportsD[0]->DesiredWidth / factor1;
	    	}
	    	if (!Printing)
	    	{
				setDeviceToScreenFactor(1.0);
	    		/*RECT	WindRect;  
				double fach, facw,sw = GetGlobalDVal2 ("[%SCREENWIDTH]",(double)GetDeviceCaps(hDC, HORZSIZE) * MFT/100);
					    		
	    		GetClientRect (GetDesktopWindow(),&WindRect);  
				if (!IsRectEmpty(&ConfigDisplayRect))
					WindRect = ConfigDisplayRect;
				facw = (double)(pViewportsD[0]->Rect.right - pViewportsD[0]->Rect.left) / (double)(WindRect.right - WindRect.left);
				fach = (double)(pViewportsD[0]->Rect.top - pViewportsD[0]->Rect.bottom) / (double)(WindRect.top - WindRect.bottom);
				if (fach > facw)
				{
					sw *= fach;
					setDeviceToScreenFactor(DeviceToScreenFactor() * sw / pViewportsD[0]->DesiredHeight);
				}
				else
				{
					sw *= facw;
					setDeviceToScreenFactor(DeviceToScreenFactor() * sw / pViewportsD[0]->DesiredWidth);
				}*/
				//DeviceToScreenFactor() *= (double)(MainRect.right - MainRect.left)/(ScreenWindowFactor*(double)(WindRect.right - WindRect.left));
	    	}
	    }
	    else
			GMEnableMenuItem(hWndMain, IDM_PLOTVIEW, MF_BYCOMMAND | MF_ENABLED);
    	DevicePixelsPerInch = ((double)pViewportsD[0]->DrawRect.right-pViewportsD[0]->DrawRect.left)/pViewportsD[0]->DesiredWidth;
    }
    else
    	DevicePixelsPerInch = (double)GetDeviceCaps(hDC, LOGPIXELSX);
  
{
#if ENABLETRACE
GSSiExitProg (796);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL PointInMainRect (POINT pnt)
#if ENABLETRACE
{GSSiEnterProg (797);
#endif
{   
	BOOL rtn=TRUE;
	
    if (pnt.x < MainRect.left   ||
        pnt.x > MainRect.right  ||
        pnt.y < MainRect.top    ||
        pnt.y > MainRect.bottom) rtn=FALSE;
{
#if ENABLETRACE
GSSiExitProg (797);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

double DistToRect (LPDPOINT pWPoint, POINT PickPoint,mnmxCor MinMax)
#if ENABLETRACE
{GSSiEnterProg (798);
#endif
{               
	double	Dist; 
	DPOINT	p;
	DPOINT	wPoint;
	POINT	Point;

	if (CurView->FileProjectionType)
		ProjectMinMax (&MinMax);
	if (PickPoint.x >= MinMax.xmn &&
		PickPoint.x <= MinMax.xmx &&
		PickPoint.y >= MinMax.ymn &&
		PickPoint.y <= MinMax.ymx)
	{
		Dist = 0;
{
#if ENABLETRACE
GSSiExitProg (798);
#endif
		return Dist;
}
	}
	if (PickPoint.x >= MinMax.xmn &&
		PickPoint.x <= MinMax.xmx &&
		PickPoint.y > MinMax.ymx) 
	{   
		p.x = PickPoint.x;
		p.y = MinMax.ymx;
	} 
	else if (PickPoint.x >= MinMax.xmn &&
		PickPoint.x <= MinMax.xmx &&
		PickPoint.y < MinMax.ymn) 
	{   
		p.x = PickPoint.x;
		p.y = MinMax.ymn;
	} 
	else if (PickPoint.y >= MinMax.ymn &&
		PickPoint.y <= MinMax.ymx &&
		PickPoint.x < MinMax.xmn) 
	{   
		p.y = PickPoint.y;
		p.x = MinMax.xmn;
	} 
	else if (PickPoint.y >= MinMax.ymn &&
		PickPoint.y <= MinMax.ymx &&
		PickPoint.x > MinMax.xmx) 
	{   
		p.y = PickPoint.y;
		p.x = MinMax.xmx;
	} 
	else if (PickPoint.x < MinMax.xmn &&
		PickPoint.y < MinMax.ymn) 
	{   
		p.x = MinMax.xmn;
		p.y = MinMax.ymn;
	} 
	else if (PickPoint.x < MinMax.xmn &&
		PickPoint.y > MinMax.ymx) 
	{   
		p.x = MinMax.xmn;
		p.y = MinMax.ymx;
	} 
	else if (PickPoint.x > MinMax.xmx &&
		PickPoint.y > MinMax.ymx) 
	{   
		p.x = MinMax.xmx;
		p.y = MinMax.ymx;
	} 
	else if (PickPoint.x > MinMax.xmx &&
		PickPoint.y < MinMax.ymn) 
	{   
		p.x = MinMax.xmx;
		p.y = MinMax.ymn;
	} 
//	UnProjectFilePtD (&p);

	wPoint = FilePtToBasePtD (p); 
	if (PeopleNet && CurView->FileProjectionType)
		Dist = ldistp (wPoint,*pWPoint) * MetersPerDegree;
//	if (PeopleNet)
//	{
//		Dist = ldistp (wPoint,*pWPoint);   
//		Dist = idist (PickPoint,DPointToPoint(p))*FileDistToBaseDist;
//		Dist *= FileDistToBaseDist;
//	}
	else 
	{
//		wPoint = FilePtToBasePtD (p); 
		Dist = GetBaseDist (&wPoint,pWPoint);  
	}
{
#if ENABLETRACE
GSSiExitProg (798);
#endif
	return Dist;
}
#if ENABLETRACE
}
#endif
}  

int	PickNearestItems (DPOINT PickPointBase,BOOL SelectVis)
#if ENABLETRACE
{GSSiEnterProg (799);
#endif
{	MNMXCORL	SaveBounds;
	MNMXCORD	SaveWBounds, SaveNewBounds;
	
	POINT		PickPoint;
	LPVISLIST	SaveVis;
	BOOL		SavePick, start, First, SavePrint=Printing;
	double		NearDist; 
	DPOINT		NearPoint;
	short		ii;
	
	SaveVis = CurVis;
	SaveBounds = CurView->Bounds;
	SaveWBounds = CurView->WBounds; 
	SaveNewBounds = CurView->NewBounds;
	CurView->WBounds.xmn = PickPointBase.x - 0.000001;
	CurView->WBounds.xmx = PickPointBase.x + 0.000001;
	CurView->WBounds.ymn = PickPointBase.y - 0.000001;
	CurView->WBounds.ymx = PickPointBase.y + 0.000001;  
    SetLocalProjection (&CurView->WBounds);
    CurView->PassID = 0; 
    SavePick = Pick;
	Pick=TRUE;
	CurView->CurFile=-1;
	PrevLayerVP=0;
	if (SelectVis)
		SelectVisList (TRUE);
//	IncrementFile ();
	Printing = TRUE;
	PickName[0] = '\0'; 
	NearDist = DBL_MAX;
	First = TRUE;
	while (GetNextViewportFile (FALSE))
	{   
		
		if (PltType < 5)
		{   
			if (First)
				NearDist = PickLimit;   
			First = FALSE;  
			if (!FastPick ||
				FileNum != FastPickFileNum ||
				FileInIndex != FastPickFII)
			{
				if (FastPick)
				{
					FastPickFileNum = FileNum; 
					FastPickFII = FileInIndex;
				}
				DisplayPlotInit(CurView->hWnd,TRUE);
				if (!OpenMap (CurView->hWnd, (HDC)1))
					goto NotOpen;
			} 
			else 
			{
			    GSSillseek(FidMap,GraphicsOffset,0);
			    LoadQuadTree();  
			}			 
			_fstrcpy (PickName,PltName);
			PickPoint = BasePtToFilePt(PickPointBase);
			start = TRUE;
			ContinuationOffset = -1;
	        while (FindLowestSegment(start,PickPoint,&PickPointBase,&NearDist))
	        {   
	        	start = FALSE;
				ContinuationOffset = -1;
				GSSillseek (FidMap,CurrentSeg,0);    
	        	while (PickNearestItem (PickPoint,&PickPointBase,&NearDist,&NearPoint))
			    {
				    GSSillseek (FidMap,ContinuationOffset,0); 
		            CurrentSeg=ContinuationOffset;
			    } 
/*				if (!ContinuePicking (QuitOnMouseMove)) 
					goto Exit;*/
	        	
	        }
	        CloseMap (FALSE);
	NotOpen:; 
		}
	}
	PickDispersedPoints (PickPointBase,0,&NearDist); 
	PickVehicles (PickPointBase,PickAp,0);  
Exit:
    Printing = SavePrint;  
    if (Printing)
    	ii=1;
	Pick=SavePick;
	CurView->Bounds = SaveBounds;
	CurView->WBounds = SaveWBounds; 
	CurView->NewBounds = SaveNewBounds;
	CurVis = SaveVis;
{
#if ENABLETRACE
GSSiExitProg (799);
#endif
	return (NumPicked);
}

#if ENABLETRACE
}
#endif
}

BOOL FindLowestSegment (BOOL start, POINT PickPoint, LPDPOINT pPickPointBase, LPDOUBLE pNearDist)
#if ENABLETRACE
{GSSiEnterProg (800);
#endif
{
    short     desc,ii;
    LPDESCBLOCK pDescBlock; 
	BOOL	rtn;  

    if (FidMap == HFILE_ERROR || !hQuadTree)
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
    	return (FALSE); 
}

    if (MinFileTime > TimeRangeEnd || MaxFileTime < TimeRangeBeg)
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
    	return FALSE;
}
    if (start)
    	FindLowestSegmentStart (PickPoint);
    if (ContinuationOffset>=0)
    {
        CurrentSeg=ContinuationOffset;
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
        return (TRUE);
}
    }
    if (!hDescBlock)
    {
        rtn = FindLowestSegment2(PickPoint, pPickPointBase, pNearDist); 
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
        return rtn;
}
    }
Next:  
    if (idescblock >= NumDescBlocks)
    {
        rtn = FindLowestSegment2(PickPoint, pPickPointBase, pNearDist); 
        if (!rtn)
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
            return (FALSE);  
}
        idescblock = 0;
    } 
    pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
    pDescBlock += (QuadOff * MaxQuadType * NumDescBlocks + QuadTypeNext* NumDescBlocks + idescblock++); 
    desc = pDescBlock->Desc;
    CurrentSeg = pDescBlock->Offset;
    GlobalUnlock (hDescBlock); 
    if (desc <= 0)
    {
        idescblock=100;  
        if (desc < 0) goto Next;
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
        return (TRUE); 
}
    }
    if (!GetVisibility(desc))
        goto Next;
{
#if ENABLETRACE
GSSiExitProg (800);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL FindLowestSegment2 (POINT PickPoint, LPDPOINT pPickPointBase, LPDOUBLE pNearDist)
#if ENABLETRACE
{GSSiEnterProg (801);
#endif
{
	LPLONG pQuadOff;
	LPQUAD	pQuad;
	HPSTR	pQuadTree, pQuadOffset;  
	long	FromOff;

	pQuadTree = GlobalLock (hQuadTree);
	pQuadOffset  = GlobalLock (hQuadOffset);

S10:pQuadOff  = (LPLONG) (pQuadOffset + (QuadLevelAt-1)*4);
	QuadOff   = labs(*pQuadOff);
	pQuad	  = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
S20: 
	FromOff = QuadOff;
	if (pQuad->Next[0] == NULLOFF && pQuad->Next[1] == NULLOFF)
	{   
		QuadTypeNext++;
		if (QuadTypeNext<MaxQuadType)
		{	
	        if (!CurVis->WantType[QuadTypeNext] && !(QuadTypeNext == 1 && CurVis->WantType[2])) goto S20;
			if (pQuad->TypeOffset[QuadTypeNext]<0) goto S20; 
			QuadOffLoc = (long)(LPSTR)&pQuad->TypeOffset[QuadTypeNext] - (long)(LPSTR)pQuadTree + 18;
			CurrentSeg=pQuad->TypeOffset[QuadTypeNext];
			GlobalUnlock (hQuadTree);
			GlobalUnlock (hQuadOffset);
{
#if ENABLETRACE
GSSiExitProg (801);
#endif
			return (TRUE);
}
		} 
		else  
		{
			QuadTypeNext=-1;
			goto S40;       
		}
	}
	else if (pQuad->Next[0] == NULLOFF)
	{
		QuadOff = pQuad->Next[1]; 
		pQuad->Next[1] = NULLOFF;
		QuadLevelAt++;   
	}
	else
	{
		QuadOff = pQuad->Next[0];   
		pQuad->Next[0] = NULLOFF;
		QuadLevelAt++;   
	}
	
S30:pQuadOff  = (LPLONG) (pQuadOffset + (QuadLevelAt-1)*4);
	*pQuadOff = QuadOff;
	pQuad = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg); 
	if (DistToRect (pPickPointBase,PickPoint,pQuad->MinMax) <= *pNearDist)
		goto S20; 
	FromOff = QuadOff;
S40:QuadLevelAt--;
	if (!QuadLevelAt)
	{   
		GlobalUnlock (hQuadTree);
		GlobalUnlock (hQuadOffset);
{
#if ENABLETRACE
GSSiExitProg (801);
#endif
		return (FALSE);
}
	} 
	pQuadOff  = (LPLONG) ( pQuadOffset + (QuadLevelAt-1)*4);
	QuadOff   = labs(*pQuadOff);
	pQuad = (LPQUAD) (pQuadTree + ((long)(QuadOff)-1)*LenQuadSeg); 
	if (pQuad->Next[0] == FromOff)
		pQuad->Next[0] = NULLOFF;
	if (pQuad->Next[1] == FromOff)
		pQuad->Next[1] = NULLOFF;
	goto S20;
#if ENABLETRACE
}
#endif
} 

BOOL FindLowestSegmentStart (POINT PickPoint)
#if ENABLETRACE
{GSSiEnterProg (802);
#endif
{	LPLONG pQuadOff;
	LPQUAD	pQuad;
	HPSTR		pQuadTree, pQuadOffset,pQuadOffsetSave;  
	long	QuadOff, MaxQuadLevelAt=0, MaxQuadOff;  
	HANDLE	hQuadOffsetSave;

    if (FidMap == HFILE_ERROR || !hQuadTree)
{
#if ENABLETRACE
GSSiExitProg (802);
#endif
    	return (FALSE);
}
    hQuadOffsetSave=GSSiGlobAlloc ( 594,GHND,(DWORD)((MaxQuadLevel+1)*sizeof(long)));
    pQuadOffsetSave=GlobalLock (hQuadOffsetSave);
	pQuadTree = GlobalLock (hQuadTree);
	pQuadOffset  = GlobalLock (hQuadOffset); 
	QuadLevelAt = 1;

S10:pQuadOff  = (LPLONG) (pQuadOffset + (QuadLevelAt-1)*4);
	QuadOff   = labs(*pQuadOff);
	pQuad	  = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg);
S20:
	if (pQuad->Next[0] == NULLOFF)
		goto Exit; 
	else
	{
		QuadOff = pQuad->Next[0];
		if (QuadLevelAt > MaxQuadLevelAt)
		{
			MaxQuadLevelAt = QuadLevelAt;
			MaxQuadOff = QuadOff; 
			hmemmove ((HPSTR)pQuadOffsetSave,pQuadOffset,((long)MaxQuadLevel+1)*sizeof(long));
		}
		QuadLevelAt++;   
	}
	
S30:pQuadOff  = (LPLONG) (pQuadOffset + (QuadLevelAt-1)*4);
	*pQuadOff = QuadOff;
	pQuad = (LPQUAD) (pQuadTree + ((long)(QuadOff-1))*LenQuadSeg); 
	if (PtInMinMax(PickPoint,pQuad->MinMax	))
		goto S20;
S40:QuadLevelAt--;
	if (!QuadLevelAt)
	{   
		QuadLevelAt++; 
		goto Exit;
	} 
	pQuadOff  = (LPLONG) ( pQuadOffset + (QuadLevelAt-1)*4);
	QuadOff   = *pQuadOff;
	pQuad = (LPQUAD) (pQuadTree + ((long)(labs(QuadOff)-1))*LenQuadSeg);
	if (QuadOff < 0)
//		goto Exit; 
		goto S40;
	*pQuadOff = -QuadOff;
	QuadOff = pQuad->Next[1];
	if (QuadOff == NULLOFF)
		goto Exit;
//		goto S40;
	QuadLevelAt++;
	goto S30;  
	
Exit: 
	QuadLevelAt = MaxQuadLevelAt;
	hmemmove ((HPSTR)pQuadOffset,pQuadOffsetSave,((long)MaxQuadLevel+1)*sizeof(long));
	GSSiGlobUlFree (&hQuadOffsetSave);
	GlobalUnlock (hQuadTree);
	GlobalUnlock (hQuadOffset);
{
#if ENABLETRACE
GSSiExitProg (802);
#endif
	return (TRUE);
}
	
#if ENABLETRACE
}
#endif
} 

BOOL PtInMinMax (POINT SegPoint,mnmxCor MinMax)
#if ENABLETRACE
{GSSiEnterProg (803);
#endif
{
	 POINT	Point;
	 
	 if (IgnoreBounds || PickDeletes)
{
#if ENABLETRACE
GSSiExitProg (803);
#endif
	 	return TRUE;
}
	 if (CurView->FileProjectionType)
		ProjectMinMax (&MinMax);
	 if (SegPoint.x < MinMax.xmn ||
		SegPoint.x > MinMax.xmx ||
		SegPoint.y < MinMax.ymn ||
		SegPoint.y > MinMax.ymx)
{
#if ENABLETRACE
GSSiExitProg (803);
#endif
		return FALSE;
}
	 else
{
#if ENABLETRACE
GSSiExitProg (803);
#endif
		return TRUE;
}
#if ENABLETRACE
}
#endif
}
 

BOOL PickNearestItem (POINT PickPoint, LPDPOINT pPickPointBase, LPDOUBLE pNearDist, LPDPOINT NearPoint)
#if ENABLETRACE
{GSSiEnterProg (804);
#endif
{	int		idesc, ItemLen, st;   
	WORD	nRead, nBytes;
	LPLONG	pRefno;
	long	remlen, PointSize;
	LPSHORT	pDesc=0;
	LPBYTE	Pcode;
	HPPOINTS lpPoints;
	HPDPOINT	lpPointsD;
	HANDLE	hMem=0; 
	HANDLE	hpltBuf;
	LPSHORT	ipnt;
	BOOL	Deleted;
	int		ltag, len;
	char	str[32];  
	static	long	DebugSeg = 11796, debugref=43310;
	short	ii;
	LPSTR	BeginSeg, lpTAG, pTXChar, lpColon;  
	BOOL	Visible=TRUE, TextIsVisible=TRUE, RefIsVisible=TRUE, DescIsVisible=TRUE;
	clock_t		starttime;
	HPSTR	pCoords;  
	LPINT	pPartLen; 
	BOOL	rtn=FALSE;
	static	POINTS	LinkPoint;
	static	DPOINT	LinkPointD;
	static	long	PolyBufferLen;
	static	BOOL	SkipToNextHeader=FALSE; 
	static	COLORREF	RouteColor;
    
	InGraphicsProcessor = TRUE;
    if (DoTime)
    	starttime=GetTickCount();  
    if (CurrentSeg == DebugSeg)
    	ii=1;
    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
    ContinuationOffset = -1; 
    if (!nBytes)
{
#if ENABLETRACE
GSSiExitProg (804);
#endif
    	return rtn;  
}
    hpltBuf = GSSiGlobAlloc ( 499,GMEM_MOVEABLE,(DWORD)nBytes);
    ipnt = (LPSHORT)GlobalLock (hpltBuf); 
	BeginSeg = (LPSTR) ipnt;
    nRead = BigRead (FidMap,(HPSTR)ipnt,nBytes);  
    nBlocksRead++;
    lBlocksRead+=nRead;
    if (DoTime)
    	MapIOTime+=GetTickCount()-starttime;
    if (nRead != nBytes) 
    {   
       	ProcessInvalidRecord (0,0,2);
{
#if ENABLETRACE
GSSiExitProg (804);
#endif
        return rtn;
}
    }  
        while (*ipnt != 0)
        {   Pcode = (LPBYTE) ipnt;
        	CurElementPnt = ipnt++; 
			CurElement=(LPSTR) ipnt - (LPSTR)(BeginSeg +2);
			HiPrecis = FALSE; 
			PointSize = 4;

            if (SkipToNextHeader && (*Pcode != 12 && *Pcode != 92 && *Pcode != 13))
				SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);            
		    else  switch (*Pcode)
	        {   
	            case 4: /* put line */
		 		{
					nPnts = 2;
		 		    lpCurPoints = (LPPOINTS) ipnt;
 		    		ipnt = ipnt + nPnts * 2;  
 		    		CurrentType = GF_LINE;
 		    		goto ProcessPolyLine;
				}
	            break;

	            case 41: /* put line */
		 		{
					nPnts = 2;
                	HiPrecis = TRUE;
                	PointSize = 16;
		 		    lpDCurPoints = (LPDPOINT) ipnt;
 		    		ipnt = ipnt + nPnts * 8;  
 		    		CurrentType = GF_LINE;
 		    		goto ProcessPolyLine;
				}
	            break;

 				case 151: //Route Area (not closed)
					RouteOffset = *(LPDOUBLE)ipnt;
					ipnt += 4;
					RouteColor = *(LPCOLORREF)ipnt;
					ipnt += 2;
					break;

                case 51:
                	HiPrecis = TRUE;
                	PointSize = 16;
	            case 5: /* put area */

		 		{   
		 			ipnt++; 
		 		    nPnts = *ipnt;
		 		    ipnt++;
		 		    if (HiPrecis)
		 		    {
			 		    lpDCurPoints = (HPDPOINT)ipnt;
			 		    ipnt = ipnt + nPnts * 8;
			 		}
			 		else
			 		{
			 		    lpCurPoints = (HPPOINTS)ipnt;
			 		    ipnt = ipnt + nPnts * 2;
			 		}
				    if (hCoords) 
				    {
				    	nCoords += nPnts;  
				    	GlobalUnlock (hCoords);
				    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
					    pCoords = GlobalLock (hCoords);
					    pCoords += lCoords;
				    	if (HiPrecis)
				    	{
						    BufWrite (&pCoords,&lCoords,(HPSTR)lpDCurPoints,nPnts*PointSize);  
						    GlobalUnlock (hCoords);
						    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
						}
						else
						{
						    BufWrite (&pCoords,&lCoords,(HPSTR)lpCurPoints,(long)nPnts*PointSize);  
						    GlobalUnlock (hCoords);
						    lpCurPoints = (HPPOINTS)GlobalLock (hCoords); 
						}
					    nPnts = nCoords;
				    }
		 		    CurrentType = GF_AREA;
		 		    if (Visible)
		 		    {
						if (nPoly)
						{
							HPPOINTS	lpPoints3; 
							HPDPOINT	lpDPoints3; 
							long	lbuf=0;
							
							pCoords = GlobalLock (hPolyBuffer);
							pCoords += ((long)nPolyPoints*PointSize);
							pPartLen = (LPINT)GlobalLock (hPolyPartLen);
							pPartLen+=iPoly;
							*pPartLen = nPnts;
							GlobalUnlock (hPolyPartLen);
							if (HiPrecis)
							{  
								BufWrite (&pCoords,&lbuf,(HPSTR)lpDCurPoints,(long)nPnts*PointSize); 
								if (!iPoly)
									LinkPointD = *lpDCurPoints;
								else
								{
									lpDPoints3 = (HPDPOINT)pCoords;
									*lpDPoints3 = LinkPointD;
									nPolyPoints++; 
								}
							}
							else
							{
								BufWrite (&pCoords,&lbuf,(HPSTR)lpCurPoints,(long)nPnts*PointSize); 
								if (!iPoly)
									LinkPoint = *lpCurPoints;
								else
								{
									lpPoints3 = (HPPOINTS)pCoords;
									*lpPoints3 = LinkPoint;
									nPolyPoints++; 
								}
							}
							nPolyPoints += nPnts; 
							iPoly++;   
							GlobalUnlock (hPolyBuffer);
						}           
						if (iPoly == nPoly)      
						{    
							if (nPoly)
							{   
								if (HiPrecis)
									lpDCurPoints = (HPDPOINT)GlobalLock (hPolyBuffer);
								else    
									lpCurPoints = (HPPOINTS)GlobalLock (hPolyBuffer);    
								nPnts = nPolyPoints;
							} 
							if (PickPerim == 1)
								goto ProcessPolyLine2; 
							if (HiPrecis)
								st = PickPolygonD (lpDCurPoints,nPnts,nPoly,hPolyPartLen,0,0);
							else
								st = PickPolygon (lpCurPoints,nPnts,0);
							if (st)
							{
								if (hPolyBuffer)
								{
									GSSiGlobUlFree (&hPolyBuffer);
									GSSiGlobFree (&hPolyPartLen);
									nPoly = 0; 
								}
							}
							else
								goto ProcessPolyLine2;
						}
					}
					GSSiGlobUlFree (&hCoords);
				}
				break;

                case 61:
                	HiPrecis = TRUE;
                	PointSize = 16;
	            case 6: /* put polyline */

		 		{
		 		    nPnts = *ipnt;
		 		    ipnt++;
		 		    if (HiPrecis)
		 		    {
		 		    	lpDCurPoints = (HPDPOINT) ipnt;
			 		    ipnt = ipnt + nPnts * 8;
			 		}
			 		else
			 		{        
			 		    lpCurPoints = (HPPOINTS) ipnt;
			 		    ipnt = ipnt + nPnts * 2;
			 		}
		 		    CurrentType = GF_POLYLINE;
ProcessPolyLine:    if (TSize > 0)
						CurrentType = GF_TEXT; 
					if (!GetTypeVisibility(TYPE_LINECURVE))
						break;
ProcessPolyLine2:
					if (Visible & !AlreadyProcessed)
		 		    {   
		 		    	short	Type=2;
		 		    	if (CurrentType == GF_AREA)
		 		    		Type = 3;
		 		    	if (HiPrecis)
							PickNearPolylineD (Type,lpDCurPoints,nPnts,0,pPickPointBase,pNearDist,&CurrentItemMinMax);
						else
							PickNearPolyline (Type,lpCurPoints,nPnts,0,PickPoint,pPickPointBase,pNearDist,&CurrentItemMinMax); 
					}
					if (hPolyBuffer)
					{
						GSSiGlobUlFree (&hPolyBuffer);
						GSSiGlobFree (&hPolyPartLen);
						nPoly = 0;
					}
				}
				break;

				case 7: /* block minmax */
				{	pMinMax = (LPMINMAX) ipnt;
					ipnt += 4;
                }
                break;

                case 8:	/*	description */
		 		{
	    			CurDescLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
	    			pDesc = ipnt;  
		 		    idesc = *ipnt++;
		 			SetSymNum (idesc);
//		 			DescScan(idesc,0);
                	DescIsVisible = GetVisibility (idesc); 
       				if (!pRefno)
       					break;
       ProcessRefno:
       				Visible = ProcessRefAndTAG (DescIsVisible,lpTAG,ltag);
                }
                break;

                case 9:	/*	refno	*/
                {   
	    			CurTAGLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
                	pRefno =(LPLONG) ipnt;
                	CurrentRefno = *pRefno;  
                	if (HighlightMultRefs)
                		CurrentRefno = HighlightMultRefs++;
                	ipnt += 2;    
                	ltag = *++Pcode;   
               		lpTAG = (LPSTR)ipnt;
					ipnt = (LPSHORT) (lpTAG + ltag + ltag%2);   
					if (pDesc)
						goto ProcessRefno;
			    }
                break;    
                
	            case 10: /* street number */
		 		{   
	    			CurSNamesLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
		 			_fmemmove (CurStreetNumbers,(LPLONG)ipnt,16);
					ipnt+=(2*4);
				}
	            break;
                
                case 93:
                	ItemIsRemoved = TRUE;
                	goto ProcessHeader;
                case 92: /* deleted item */
                	Deleted = TRUE;  
					ItemIsDeleted = TRUE;  
					ItemIsRemoved = FALSE;              	
                	goto ProcessHeader;
				case 12: /* item minmax */ 
					ItemIsDeleted = Deleted = FALSE;
	ProcessHeader:
				{	pMinMax = (LPMINMAX)ipnt;
					CurrentItemMinMax = *pMinMax;
					ItemSeg = CurrentSeg;
					CurrentItem=(LPSTR) ipnt - (LPSTR)(BeginSeg +2);
					ipnt += 4;
					ItemLen = abs(*ipnt);
					ipnt++;      
					TSize = 0; 
					_fmemset (CurStreetNumbers,0,16);
					pRefno=0;
					pDesc=0;
					InitRecord (0);
               		Visible=DescIsVisible=TextIsVisible=RefIsVisible=TRUE;
					AlreadyProcessed=FALSE;
					if (Deleted || DistToRect (pPickPointBase,PickPoint,*pMinMax) >= *pNearDist)
					{	
						remlen = nBytes - ((LPSTR) ipnt - BeginSeg); 
						if (ItemLen >= remlen)
							ipnt++;
						ipnt+=ItemLen;   
                        nBlocksOut++;
                        if (!ItemLen)
                        	SkipToNextHeader=TRUE;
					}
					else
					{
			 			CurrentiPen = *++Pcode;
			 			/*if (CurrentiPen)
			 				CurrentPen = pens[CurrentiPen];
			 			if (!CurrentPen)
			 				CurrentPen = pens[0];  */
			 			nBlocksIn++;
                       	SkipToNextHeader=FALSE;
			 		}
                }
                break;

				case 13: /* continuation offset */
				{	ContinuationOffset = *(LPLONG)ipnt; 
					ipnt += 2;
					*ipnt = 0;
					if (ContinuationOffset >=0)  
						rtn = TRUE; 
					else
						SkipToNextHeader=FALSE;
                }
                break;

				case 14: /* text size */
				{	TSize = *ipnt;
					ipnt++;
                	TextIsVisible = GetTextVisibility (TSize,1);
                	if (DescIsVisible && TextIsVisible && RefIsVisible)
                		Visible=TRUE;
                	else
                		Visible=FALSE;
                }
                break;

				case 15: /* point coordinates */
				{	 
	        		lpPoints = (LPPOINTS)ipnt;
	        		ipnt += 3;   
	        		AlreadyProcessed=TRUE;
	        		if (Visible)
						PickNearPoint (lpPoints,0,ipnt,PickPoint,pPickPointBase,pNearDist,&CurrentItemMinMax);
                }
                break;

				case 16: /* line coordinates */
				{	 
					lpCurPoints = (HPPOINTS)ipnt;
	        		ipnt += 4;     
	        		nPnts = 2;
	        		goto PickBaseLine;
                }
                break;

                case 172:
                {
					HPDPOINT	pDPoint;
					int		st;
					
                	HiPrecis = TRUE;
                	ipnt++;
					pDPoint = (LPDPOINT)ipnt;ipnt+=8;
					BP = *pDPoint;
					pDPoint = (LPDPOINT)ipnt;ipnt+=8;
					POC = *pDPoint;
					pDPoint = (LPDPOINT)ipnt;ipnt+=8;
					EP = *pDPoint;
		 		    CurrentType = GF_CURVE;
		 		    {
		 		    	DPOINT dPoint = MidPointD(BP,EP);
		 		    	PTRot = getazd (&BP,&EP); 
/*        				if (Pick)
        					TXLoc = BasePtToFilePt (dPoint); 
        				else  */
							TXLoc = dPoint; 
	    				HaveTXLoc = TRUE;  
	    			}
					if (Visible && GetTypeVisibility(TYPE_LINECURVE))  
					{   
						double	BackAZ;
						 
			 		    hMem = GSSiGlobAlloc ( 500,GMEM_MOVEABLE,(long)4096*16);
			 		    lpDCurPoints = (HPDPOINT)GlobalLock (hMem);   
			 		    pDPoint = lpDCurPoints;
			 		    nPnts = 0;
	                    st =  CurvePointsD(&BP,&POC,&EP, &nPnts, &pDPoint,&BackAZ,4090,DisplayCurveFactor,1);
						AlreadyProcessed=TRUE; 
						PickNearPolylineD (4,lpDCurPoints,nPnts,0,pPickPointBase,pNearDist,&CurrentItemMinMax);
						GSSiGlobUlFree (&hMem);
					} 
                }
                break;  
                
				case 17: /* curve coordinates */
				{	
					DPOINT	BP,POC,EP; 
					int		st;
					
					BP.x = *ipnt++;
					BP.y = *ipnt++;
					POC.x = *ipnt++; 
					POC.y = *ipnt++; 
					EP.x = *ipnt++; 
					EP.y = *ipnt++;  
		 		    hMem = GSSiGlobAlloc ( 501,GMEM_MOVEABLE,USHRT_MAX);
		 		    lpCurPoints = (LPPOINTS)GlobalLock (hMem);   
		 		    lpPoints = lpCurPoints;
		 		    nPnts = 0;
                    st =  CurvePointsS(&BP,&POC,&EP, &nPnts,  &lpPoints, CurveExpansionFactor,USHRT_MAX/4);
				}
	 PickBaseLine:
	        		AlreadyProcessed=TRUE;
	        		if (Visible)
						PickNearPolyline (2,lpCurPoints,nPnts,0,PickPoint,pPickPointBase,pNearDist,&CurrentItemMinMax);
					GSSiGlobFree (&hMem); 
					
                break;

				case 18: /* text character */
				{	 
	        		ipnt += 4; 
                }
                break; 
                 
				case 19: /* grahpics text */
				{	 
					int		nchar, ltext;
					LPGRTEXTHEADER	pGRTextHeader;
					LPSTR	pText; 
					HANDLE	hText = GSSiGlobAlloc ( 502,GHND,512);
					LPSTR	pText2 = GlobalLock (hText);
				    
				    pGRTextHeader = (LPGRTEXTHEADER)ipnt;
				    ipnt += sizeof(GRTEXTHEADER) / 2;  
				    nchar = pGRTextHeader->lText;  
				    pText = (LPSTR)ipnt;   
				    ipnt += nchar/2;
				    _fstrncpy (pText2,pText,nchar);  
					SetGlobalValue2 (hTEXT,pText2,0);
					GSSiGlobUlFree (&hText);
				    SetTextLocVars (&TLSet);
                	if (DescIsVisible && CurVis->WantType[2] && HaveTXLoc && (CurView->PassID || Pick)) 
                	{
						ProcessTextObject (0,pGRTextHeader,pText,nchar,BeginSeg,0,pPickPointBase,pNearDist,&CurrentItemMinMax);
					}
                }
                break;  
                
                
	        	case 20:  
	        		ipnt += 2; 
	        		CurPointAZ = PTRot = *(LPFLOAT)ipnt;
	        		ipnt += 2;
	        		lpPoints = (HPPOINTS)ipnt;
	        		ipnt += 2; 
	        		HaveTXLoc = TRUE;
 		    		CurrentType = GF_POINT; 
        			CurPointLocD = FilePtToBasePt (POINTStoPOINT(*lpPoints)); 
	        		if (Visible && GetTypeVisibility(TYPE_POINT))
						PickNearPoint (lpPoints,CurPointAZ,ipnt,PickPoint,pPickPointBase,pNearDist,&CurrentItemMinMax);
	        	break; 
	        	
	        	case 127:
	        		PointSize = 16;
	        	case 27: // Multipolygon indicator
	        	{
	        		short	n; 
	        		long	ln=(long)MAX_POLY_POINTS*PointSize;
	        		
					GSSiGlobFree (&hPolyBuffer); 
					GSSiGlobFree (&hPolyPartLen);  
	        		nPoly = n = *ipnt++; 
	        		iPoly = 0;
	        		PolyBufferLen = 2 * (nPoly - 1);    //is this needed - yes for addition of linkpoints
	        		while (n--)
	        			PolyBufferLen += *(LPWORD)ipnt++; 
	        		if (!hCurvePoints)
	        			ln = (long)PolyBufferLen*PointSize;
	        		hPolyBuffer = GSSiGlobAlloc ( 308,GMEM_MOVEABLE,ln);  
	        		hPolyPartLen = GSSiGlobAlloc ( 309,GMEM_MOVEABLE,(long)nPoly*sizeof(int));  
	        		nPolyPoints = 0;
                }
	        	break;
	        	
	        	case 227: // Multipolygon indicator
	        		PointSize = 16;
				case 228:
	        	{
	        		int		n; 
	        		long	ln=(long)MAX_POLY_POINTS*PointSize;
	        		
					GSSiGlobFree (&hPolyBuffer); 
					GSSiGlobFree (&hPolyPartLen);  
	        		nPoly = n = *(LPINT)ipnt++;
					ipnt++;
	        		iPoly = 0;
	        		PolyBufferLen = sizeof(int) * (nPoly - 1);    //is this needed - yes for addition of linkpoints
	        		while (n--)
					{
	        			PolyBufferLen += *(LPINT)ipnt++;
						ipnt++;
					}
	        		if (!hCurvePoints)
	        			ln = (long)PolyBufferLen*PointSize;
	        		hPolyBuffer = GSSiGlobAlloc ( 308,GMEM_MOVEABLE,ln);  
	        		hPolyPartLen = GSSiGlobAlloc ( 309,GMEM_MOVEABLE,(long)nPoly*sizeof(int));  
	        		nPolyPoints = 0;
                }
	        	break;
	        	
	        	case 31: /* from and to street */
	        	{  
	        		_fmemmove (&FromStreet,ipnt,4);
	        		_fmemmove (&ToStreet,ipnt+6,4);
	        		ipnt += 12;
	        	}
	        	break;  
	        	
	        	case 32: /* address data: totlen/2, ZIPL(5),ZIPR(5),COUNTYL(3),COUNTYR(3)
	        	         							addlengths(i:4,i:4,i:4,i:4) Addresses(fraddl,toaddl,fraddr,toaddr) */
	       		{   
	       			LPSTR	pAddData;
	       			if (hPNAddData)
	       			{
	       				pAddData = GlobalLock (hPNAddData);
	       				_fmemmove (pAddData,(LPVOID)(ipnt+1),*ipnt*2); 
	       				GlobalUnlock (hPNAddData);
	       			}
	        		ipnt += *ipnt+1;	
	        	}
	        	break;   
	        	
	        	case 120:  
	        		ipnt += 4;
	        		CurPointAZ = PTRot = *(LPDOUBLE)ipnt;
	        		ipnt += 4;
	        		lpPointsD = (LPDPOINT)ipnt;
	        		ipnt += 8; 
 		    		CurrentType = GF_POINT; 
        			CurPointLocD = *lpPointsD; 
	        		if (Visible && GetTypeVisibility(TYPE_POINT))
						PickNearPointD (lpPointsD,CurPointAZ,ipnt,PickPoint,pPickPointBase,pNearDist,&CurrentItemMinMax);
	        	break;

				case 135:
					PointSize = 16;
			    case 35: /* point array */
			
				{   nPnts = *ipnt;
				    ipnt++; 
				    if (hCoords) 
				    {
				    	nCoords += nPnts;
				    	GlobalUnlock (hCoords);
				    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
				    }
				    else 
				    {
				    	hCoords = GSSiGlobAlloc ( 311,GMEM_MOVEABLE,(long)nPnts*PointSize);
				    	nCoords = nPnts;
				    	lCoords = 0;
				    }
				    pCoords = GlobalLock (hCoords);
				    pCoords += lCoords;
				    BufWrite (&pCoords,&lCoords,(HPSTR)ipnt,nPnts*PointSize); 
				    GlobalUnlock (hCoords);
				    if (PointSize == 16)
					    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
					else
					    lpCurPoints = (HPPOINTS)GlobalLock (hCoords); 
				    ipnt += nPnts * PointSize/2;
				}
				break;   
				
	        		
				case 37: 
				{
					long	StartTime, EndTime;
					
					StartTime = *(LPLONG)ipnt;
					ipnt+=2;
					EndTime = *(LPLONG)ipnt;
					ipnt+=2; 
					if (StartTime > TimeRangeEnd || EndTime < TimeRangeBeg)
						Visible = FALSE;
				}
				break;
	        	
				case 40: /* command string */
				{                    
					LPSTR	pString, pEnd;
					
	    			CurGCmdStringLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
					lGCmdString = *ipnt++; 
					GSSiGlobFree (&hGCmdString);   
					hGCmdString = GSSiGlobAlloc ( 503,GMEM_MOVEABLE,4096);
					pString = GlobalLock (hGCmdString);
					_fstrncpy (pString,(LPSTR)ipnt,lGCmdString);
					pEnd = pString + lGCmdString;
					*pEnd = 0; 
					if (Visible)
					switch (ProcessGCmdStrings)
					{                          
						case 0:
							break;
						case 1:
							if (Pick)
								break; 
							goto DoCmd;
						case 2:
							if (!Pick)
								break;
						case 3: 
				DoCmd:  
						InGRCmd = TRUE;
						ExpandText (pString); 
						InGRCmd = FALSE;
						_fstrncpy (pString,(LPSTR)ipnt,lGCmdString); 
					}
					ipnt += lGCmdString/2; 
					GlobalUnlock (hGCmdString);
				}
				break;	
				
	            default: 
	            	SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);
                break;
			}
        } 
    GSSiGlobUlFree (&hpltBuf);
    if (DoTime)  
    	TotDisplayTime+=(GetTickCount()-starttime);
	InGraphicsProcessor = FALSE;

{
#if ENABLETRACE
GSSiExitProg (804);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL PickNearPolyline (short Type,HPPOINTS lpPointsIn,long nPnts,int PolyID,POINT PickPoint,
						LPDPOINT pPickPointBase, LPDOUBLE pNearDist, LPMINMAX pMinMax)
#if ENABLETRACE
{GSSiEnterProg (805);
#endif
{   
	HPPOINTS lpPoints=lpPointsIn;
	HANDLE Handle;
	POINT	LastPoint, Lastpt, IntPoint;
	DPOINT	NearPoint, BeginPoint, EndPoint, Point1,Point2, NodePoint,FromPoint,ToPoint;
	DWORD	i, NearPointID=0;
	double	MinNodeDist=DBL_MAX, MinDist=DBL_MAX, dist1, dist2, LineLen, TotDist=0, TotDistW=0, PCT=0, NearLinDist, IncDist;
	BOOL	First=TRUE;   
	double	PickY, PickX, AZ1, AZ2, AZ[3], IntX, IntY, IntAZ, TurnAZ;
	MNMXCORD Rect;
	POINT	BeginPointFile,EndPointFile, PickedPointFile;	
	
	PickX = PickPoint.x;
	PickY = PickPoint.y;
	
	NearPoint.x = lpPoints->x;
	NearPoint.y = lpPoints->y;
	BeginPointFile = POINTStoPOINT(*lpPoints);
	BeginPoint = FilePtToBasePt(BeginPointFile);
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		Lastpt = POINTStoPOINT(*lpPoints);
		ProjectFilePtS (lpPoints);
		Point2 = FilePtToBasePtNP(POINTStoPOINT(*lpPoints));
		dist1 = ldistp (*pPickPointBase,Point2);
		if (dist1 < MinNodeDist)
		{
			MinNodeDist = dist1;
			NodePoint = Point2;
			NearPointID = i;
		}
		if (!First) 
		{   
			TotDistW += ldistp (Point1,Point2);
            LineLen = idist (LastPoint,POINTStoPOINT(*lpPoints));
			AZ1 = getaz (LastPoint,POINTStoPOINT(*lpPoints)); 
			AZ2 = LTWOPI (AZ1 + HALFPI);
			if (LINSEC ((double)lpPoints->x,(double)lpPoints->y,AZ1,
			 		 PickX,PickY,AZ2,&IntX,&IntY) == 1)
			{   
				IntPoint.x = IntX;
				IntPoint.y = IntY; 
				IntAZ = getaz (IntPoint,PickPoint);
				TurnAZ = DeltaAZ (AZ1,IntAZ);
				if (LDIST (IntX,IntY,(double)lpPoints->x,(double)lpPoints->y) > LineLen ||
					LDIST (IntX,IntY,(double)LastPoint.x, (double)LastPoint.y)> LineLen)
				{ 
					dist1 = idist (PickPoint,LastPoint);
					dist2 = idist (PickPoint,POINTStoPOINT(*lpPoints));
					if (dist1 > dist2)
					{
						if (dist2 < fabs (MinDist))
						{
							MinDist = DSIGN (dist2,TurnAZ);
							NearPoint.x = lpPoints->x;
							NearPoint.y = lpPoints->y;
							NearLinDist = TotDist + LineLen;   
							AZ[1] = AZ1;
						}
					}
					else 
					{
						if (dist1 < fabs(MinDist))
						{
							MinDist = DSIGN (dist1,TurnAZ);
							NearPoint.x = LastPoint.x;
							NearPoint.y = LastPoint.y;
							NearLinDist = TotDist;   
							AZ[1] = AZ1;
						}
					}
				}
				else
				{
					dist1 = LDIST (PickX,PickY,IntX,IntY);
					if (dist1 < fabs (MinDist))
					{
						MinDist = DSIGN (dist1,TurnAZ);
						NearPoint.x = IntX;
						NearPoint.y = IntY; 
						IncDist = idist (LastPoint,IntPoint);
						NearLinDist = TotDist + IncDist;   
						AZ[1] = AZ1;
					}
				}
			}
			TotDist += LineLen;
		} 
		else
			First = FALSE;
		LastPoint = POINTStoPOINT(*lpPoints);
		Point1 = Point2;
	}
	EndPointFile = Lastpt;
	EndPoint = FilePtToBasePt(Lastpt);
	if (nPnts > 1)
	{   
		DPOINT Point=FilePtToBasePtNP(POINTStoPOINT(lpPointsIn[1]));
		AZ[0] = getazd (&BeginPoint,&Point);  
		Point=FilePtToBasePtNP(POINTStoPOINT(lpPointsIn[nPnts-2]));
		AZ[2] = getazd (&Point,&EndPoint);
	} 
	else
		AZ[0]=AZ[1]=AZ[2]=0;
	GetItemMinMax (pMinMax,&Rect); 
	PickedPointFile.x = IDNINT(NearPoint.x);
	PickedPointFile.y = IDNINT(NearPoint.y);
//	ProjectFilePt (&PickedPointFile);
//	if (PeopleNet && idist (PickPoint,PickedPointFile)*MetersPerDegree >= *pNearDist)
//		goto Exit;
//	UnProjectFilePtD (&NearPoint);
 
	NearPoint = FromPoint = ToPoint = FilePtToBasePtD(NearPoint); 
	MinDist = ldistp (NearPoint,*pPickPointBase) * MetersPerDegree;
	if (PeopleNet && MinDist >= *pNearDist)
		goto Exit;
	if (TotDist)
		PCT = NearLinDist/TotDist; 
	PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,PolyID,
				   -Type,PCT,-MinDist,AZ,TotDistW,CurrentItem,CurElement,BeginPoint,EndPoint,NearPoint,
				   NodePoint,NearPointID,
				   0,&Rect,BeginPointFile,EndPointFile,PickedPointFile,nPnts,NULL_ELEV,&FromPoint,&ToPoint);
	if (NumPicked == MaxPick)
		*pNearDist = min (*pNearDist,fabs(PickList[0].OffDist));///FileDistToBaseDist);
Exit:
{
#if ENABLETRACE
GSSiExitProg (805);
#endif
	return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL PickNearPolylineD (short Type,HPDPOINT lpPoints,long nPnts,int PolyID,
						 LPDPOINT pPickPointBase, LPDOUBLE pNearDist, LPMINMAX pMinMax)
#if ENABLETRACE
{GSSiEnterProg (806);
#endif
{
	HANDLE Handle;
//	LPPOINT	lpNewPoints, lpPntNew;
	DPOINT	IntPoint;
	DPOINT	LastPoint, NearPoint, BeginPoint, EndPoint, Point1,Point2, NodePoint, Lastpt,FromPoint,ToPoint;
	DWORD	i, NearPointID;
	double	MinNodeDist=DBL_MAX, MinDist=DBL_MAX, dist1, dist2, LineLen, TotDist=0, TotDistW=0, PCT=0, NearLinDist, IncDist;
	BOOL	First=TRUE;   
	double	PickY, PickX, AZ1, AZ2, AZ[3], IntX, IntY, IntAZ, TurnAZ;
	MNMXCORD Rect;
	POINT	BeginPointFile,EndPointFile, PickedPointFile;	
	
	PickX = pPickPointBase->x;
	PickY = pPickPointBase->y;
	
	if (nPnts > 1)
	{
		AZ[0] = getazd (&lpPoints[0],&lpPoints[1]);
		AZ[2] = getazd (&lpPoints[nPnts-2],&lpPoints[nPnts-1]);
	} 
	else
		AZ[0]=AZ[1]=AZ[2]=0;
	NearPoint = BeginPoint = *lpPoints;
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		Lastpt = *lpPoints;
		Point2 = *lpPoints;
		dist1 = ldistp (*pPickPointBase,Point2);
		if (dist1 < MinNodeDist)
		{
			MinNodeDist = dist1;
			NodePoint = Point2;
			NearPointID = i;
		}
		if (!First) 
		{   
			TotDistW += ldistp (Point1,Point2);
            LineLen = ldistp (LastPoint,*lpPoints);
			AZ1 = getazd (&LastPoint,lpPoints); 
			AZ2 = LTWOPI (AZ1 + HALFPI);
			if (LINSEC ((double)lpPoints->x,(double)lpPoints->y,AZ1,
			 		 PickX,PickY,AZ2,&IntX,&IntY) == 1)
			{
				IntPoint.x = IntX;
				IntPoint.y = IntY; 
				IntAZ = getazd (&IntPoint,pPickPointBase);
				TurnAZ = DeltaAZ (AZ1,IntAZ);
				if (LDIST (IntX,IntY,(double)lpPoints->x,(double)lpPoints->y) > LineLen ||
					LDIST (IntX,IntY,(double)LastPoint.x, (double)LastPoint.y)> LineLen)
				{ 
					dist1 = ldistp (*pPickPointBase,LastPoint);
					dist2 = ldistp (*pPickPointBase,*lpPoints);
					if (dist1 > dist2)
					{
						if (dist2 < fabs (MinDist))
						{
							MinDist = DSIGN (dist2,TurnAZ);
							NearPoint = *lpPoints;
							NearLinDist = TotDist + LineLen;   
							AZ[1] = AZ1;
						}
					}
					else 
					{
						if (dist1 < fabs (MinDist))
						{
							MinDist = DSIGN (dist1,TurnAZ);
							NearPoint = LastPoint;
							NearLinDist = TotDist;   
							AZ[1] = AZ1;
						}
					}
				}
				else
				{
					dist1 = LDIST (PickX,PickY,IntX,IntY);
					if (dist1 < fabs (MinDist))
					{
						MinDist = DSIGN (dist1,TurnAZ);
						NearPoint.x = IntX;
						NearPoint.y = IntY; 
						IncDist = ldistp (LastPoint,IntPoint);
						NearLinDist = TotDist + IncDist;   
						AZ[1] = AZ1;
					}
				}
				TotDist += LineLen; 
			}
		} 
		else
			First = FALSE;
		LastPoint = *lpPoints;
		Point1 = Point2;
	}
	EndPoint = Lastpt;
	if (pMinMax)
	{
		GetItemMinMax (pMinMax,&Rect); 
		PickedPointFile = BasePtToFilePt (NearPoint);
		BeginPointFile = BasePtToFilePt (BeginPoint);
		EndPointFile = BasePtToFilePt (EndPoint);
		if (TotDist)
			PCT = NearLinDist/TotDist;   
		FromPoint = ToPoint = NearPoint;
		PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,PolyID,
				   Type,PCT,-MinDist,AZ,TotDistW,CurrentItem,CurElement,BeginPoint,EndPoint,NearPoint,
				   NodePoint,NearPointID,
				   0,&Rect,BeginPointFile,EndPointFile,PickedPointFile,nPnts,NULL_ELEV,&FromPoint,&ToPoint);
		if (NumPicked == MaxPick)
			*pNearDist = min (*pNearDist,fabs(PickList[0].OffDist));
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (806);
#endif
	return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

LPSHORT PickNearPointD (LPDPOINT lpPoints,double PointAZ,LPSHORT ipnt,
					  POINT PickPoint, LPDPOINT pPickPointBase, LPDOUBLE pNearDist, LPMINMAX pMinMax)
#if ENABLETRACE
{GSSiEnterProg (807);
#endif
{
	DPOINT	NearPoint, BeginPoint, EndPoint,FromPoint,ToPoint;     
	POINT	FilePoint;
	int	i;
	double	MinDist;
	double	PickY, PickX, IntX, IntY, AZ[3]={PointAZ,PointAZ,PointAZ};
	MNMXCORD Rect;
	
	if (!PickPoints)
{
#if ENABLETRACE
GSSiExitProg (807);
#endif
		return FALSE;
}
	PickX = PickPoint.x;
	PickY = PickPoint.y;
	
	FilePoint = BasePtToFilePt (*lpPoints);
	NearPoint = FromPoint = ToPoint = *lpPoints;
	GetItemMinMax (pMinMax,&Rect);
	MinDist = GetBaseDist (&NearPoint,pPickPointBase);
	if (PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,0,
				   1,0,MinDist,AZ,0,CurrentItem,CurElement,
				   NearPoint,NearPoint,NearPoint,NearPoint,0,0,&Rect,FilePoint,FilePoint,FilePoint,1,NULL_ELEV,&FromPoint,&ToPoint))
	{
		if (NumPicked == MaxPick)
			*pNearDist = min (*pNearDist,fabs(PickList[0].OffDist));
	}
{
#if ENABLETRACE
GSSiExitProg (807);
#endif
	return (ipnt);
}
#if ENABLETRACE
}
#endif
} 

LPSHORT PickNearPoint (LPPOINTS lpPoints,double PointAZ,LPSHORT ipnt,
					 POINT PickPoint, LPDPOINT pPickPointBase, LPDOUBLE pNearDist, LPMINMAX pMinMax)
#if ENABLETRACE
{GSSiEnterProg (808);
#endif
{
	DPOINT	NearPoint, BeginPoint, EndPoint,FromPoint,ToPoint;
	int	i;
	double	MinDist;
	double	AZ[3]={PointAZ,PointAZ,PointAZ};
	MNMXCORD Rect; 
	POINT	PickedPointFile = POINTStoPOINT(*lpPoints);
	
	if (!PickPoints)
{
#if ENABLETRACE
GSSiExitProg (808);
#endif
		return FALSE;
}
	
	NearPoint = FromPoint = ToPoint = FilePtToBasePt(POINTStoPOINT(*lpPoints));
	GetItemMinMax (pMinMax,&Rect); 
	ProjectFilePt (&PickedPointFile);
//	MinDist = GetBaseDist (&NearPoint,pPickPointBase); 
	MinDist = ldistp (NearPoint,*pPickPointBase) * MetersPerDegree;
	if (PeopleNet && MinDist >= *pNearDist)
		goto Exit;
	PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,0,
				   -1,0,MinDist,AZ,0,CurrentItem,CurElement,
				   NearPoint,NearPoint,NearPoint,NearPoint,0,0,&Rect,PickedPointFile,PickedPointFile,PickedPointFile,1,NULL_ELEV,&FromPoint,&ToPoint);
	if (NumPicked == MaxPick)
		*pNearDist = min (*pNearDist,PickList[0].OffDist);
Exit:
{
#if ENABLETRACE
GSSiExitProg (808);
#endif
	return (ipnt);
}
#if ENABLETRACE
}
#endif
} 

double GetBaseDist (LPDPOINT p1, LPDPOINT p2)
#if ENABLETRACE
{GSSiEnterProg (809);
#endif
{   
	clock_t	starttime;
	double	d;
	
//	starttime=clock(); 
	if (PRJ_TYPE[1] == 0)
		d = ArcDistance(*p1,*p2);
	else
		d = ldistp (*p1,*p2);
//	DistTime += clock()-starttime;
{
#if ENABLETRACE
GSSiExitProg (809);
#endif
	return (d);
}
#if ENABLETRACE
}
#endif
}

BOOL ReorgFileOpen (LPSTR PltFile)
#if ENABLETRACE
{GSSiEnterProg (810);
#endif
{               
	OFSTRUCTGM OFStruct;
	LPSTR	lpExt;
	LPSTR	pFile;
	
	if (!ReorgFile)
{
#if ENABLETRACE
GSSiExitProg (810);
#endif
		return FALSE;  
}
	_fstrcpy (ReorgName,PltFile);  
	ExpandText (ReorgName);
	lpExt =_fstrrchr (ReorgName,'.'); 
	if (!lpExt)
{
#if ENABLETRACE
GSSiExitProg (810);
#endif
		return FALSE; 
}
	*lpExt = 0;
	_fstrcat (ReorgName,".reo");
	CloseAllRequestedFiles(FALSE);
	KeepFilesOpen = FALSE;
	ReorgfileFID = GSSiOpenFile (ReorgName,&OFStruct,OF_CREATE);
	hReorgBuf = GSSiGlobAlloc ( 504,GMEM_FIXED,USHRT_MAX); 
	lReorgBuf = 0; 
	hReorgDescFile = GSSiGlobAlloc ( 505,GMEM_MOVEABLE,256);
	pFile = GlobalLock (hReorgDescFile);
	GSSiGetTempFileName (0,"gm",0,pFile); 
	ReorgDescFile=GSSiOpenFile (pFile,&OFStruct,OF_CREATE);  
	GlobalUnlock (hReorgDescFile);
{
#if ENABLETRACE
GSSiExitProg (810);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ReorgFileClose (LPSTR PltFile)
#if ENABLETRACE
{GSSiEnterProg (811);
#endif
{   
	short	Signature, Version;
	short		len, DummyRec;
	long		DescBlockOffset, MinMaxLoc;
	mnmxCor		MinMax, NewFileMNMX;	
	char	OldName[MAX_PATH];
	PRIMEOFFSETS NewPrimeOffs;
	LPSTR	pDescBlock, pFile;    
	LPSHORT	pItemLen;
	long	dblen, dbn=0;
	
	if (!ReorgFile || ReorgfileFID == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (811);
#endif
		return FALSE;
}

    MinMaxInit (&NewFileMNMX);
	
	ReorgOut2 (0,-1);
	if (NewQuadFID)
	{    
		GSSillseek (ReorgfileFID,NewPrimeOffset,0);
		BigRead (ReorgfileFID,(HPSTR)&len,2);
		BigRead (ReorgfileFID,(HPSTR)&NewPrimeOffs,sizeof(PRIMEOFFSETS));
		NewPrimeOffs.GraphicsOffset = BuildNewQuadTree (ReorgfileFID,NewQuadFID);
		NewPrimeOffset = GSSillseek (ReorgfileFID,0,2); 
		BigWrite (ReorgfileFID,(HPSTR)&len,2,-1);
		BigWrite (ReorgfileFID,(HPSTR)&NewPrimeOffs,sizeof(PRIMEOFFSETS),-1);
		DescBlockOffset = 0;
	}
	else
	{ 
		DescBlockOffset = GSSillseek (ReorgfileFID,0,2);
		BigWrite (ReorgfileFID,(HPSTR)&DescBlockLen,4,-1);
		pDescBlock = GlobalLock (hDescBlockReorg);
		BigWrite (ReorgfileFID,pDescBlock,DescBlockLen,-1);   
		GlobalUnlock (hDescBlockReorg);  
	}
	GSSiGlobFree (&hDescBlockReorg);
    BigWrite (ReorgfileFID,(HPSTR)&NewPrimeOffset,4,-1);
	GSSillseek (ReorgfileFID,NewPrimeOffset,0);
	BigRead (ReorgfileFID,(HPSTR)&len,2);
	BigRead (ReorgfileFID,(HPSTR)&NewPrimeOffs,sizeof(PRIMEOFFSETS));
	NewPrimeOffs.DescBlockOffset = DescBlockOffset;
	GSSillseek (ReorgfileFID,NewPrimeOffset,0);
	BigWrite (ReorgfileFID,(HPSTR)&len,2,-1);
	BigWrite (ReorgfileFID,(HPSTR)&NewPrimeOffs,sizeof(PRIMEOFFSETS),-1);
	GSSillseek (ReorgfileFID,0,2);
    MinMaxLoc = GSSillseek(FidMap,(LONG)-(2+12),2);
    BigRead (FidMap,(HPSTR)&MinMax,8);               
    BigWrite (ReorgfileFID,(HPSTR)&MinMax,8,-1);
    BigRead (FidMap,(HPSTR)&Signature,2);       
    BigWrite (ReorgfileFID,(HPSTR)&Signature,2,-1);
    BigRead (FidMap,(HPSTR)&Version,2); 
    Version = max (Version,6); 
    if (Version > 5)
    	Version = 8;          
    BigWrite (ReorgfileFID,(HPSTR)&Version,2,-1);
    Version = 0;
    BigWrite (ReorgfileFID,(HPSTR)&Version,2,-1);

	GSSiClose (ReorgfileFID);  
	ReorgfileFID = HFILE_ERROR;   
	GSSiGlobFree (&hReorgBuf); 
	
	CloseMap(FALSE);
	CloseAllRequestedFiles(FALSE);
	_fstrcpy(OldName, PltFile);
	ExpandText (OldName);
	GSSiRemove (OldName);
	GSSiRename  (ReorgName,OldName);                      
	GSSiClose (ReorgDescFile); 
	pFile = GlobalLock (hReorgDescFile);
	GSSiRemove (pFile);
	  
	GSSiGlobUlFree (&hReorgDescFile);
	if (NewQuadFID)
	{   
		short	id, Type;
		long	len;
		long	Offset, MinTime,MaxTime,RecLen; 
		HPSTR	pBuf, tbuf;
		LPSTR	Rec;    
		BOOL	First;
		HANDLE	hBuf=0, hRec; 
		LPITEM	ItemHeader;  
		OFSTRUCTGM	OFStruct;
		long	TotLen, loc;    
		struct	{short	link;
				 long	Offset;
				 short	zero;} Trailer;
		long	idebug=641180,ii;
        static	long debugii=34654;
        //remove old RIN and TIN files
		{  
			char	File[128];
			LPSTR	pEnd;
			
			_fstrcpy (File,OldName);
			_fstrupr (File);
			if ((pEnd = _fstrstr (File,".PLT")))
			{
				_fstrcpy (pEnd,".RIN");
				GSSiRemove (File);  
				_fstrcpy (pEnd,".TIN");
				GSSiRemove (File);  
			}
		}

		Trailer.link=13;
		Trailer.Offset=-1;
		Trailer.zero=0;
		ReorgFile = FALSE;    
		WantQuadTree2 = TRUE;
		if (!OpenMap (CurView->hWnd,0))
{
#if ENABLETRACE
GSSiExitProg (811);
#endif
			return(FALSE); 
}
		TotLen = GSSillseek (NewQuadFID,0,2);
		GSSillseek (NewQuadFID,0,0); 
		MaxTime = 0;
		MinTime = LONG_MAX;  
		GSSiGlobFree (&hFileSymList);
		while (ContinueProcessing && BigRead (NewQuadFID,(HPSTR)&len,4))
		{
			hBuf = GSSiGlobAlloc ( 506,GMEM_MOVEABLE,min(len+1024,MAXREORGBUF));
			pBuf = GlobalLock (hBuf);
			ii=GSSillseek (NewQuadFID,0,1);
			if (ii==debugii)
				ii=1;  
			BigRead (NewQuadFID,(HPSTR)&Type,2);
			BigRead (NewQuadFID,pBuf,len); 
			ItemHeader = (LPITEM)pBuf; 
			if (len < 32012)
				ItemHeader->Len = (len-12)/2 + len%2;
			FoundInvalidRec = FALSE;
			UpdateDateRange (pBuf,len,&MinTime,&MaxTime); //also checks for invalid recs and sets type of new symbols
			if (FoundInvalidRec)
			{
				GSSiGlobUlFree (&hBuf); 
				ReorgBadRecs++;    
				goto NextRec;
			}
			RecomputeMinMax (&ItemHeader->MinMax,pBuf,len,0,&DummyRec);  
			if (ItemHeader->MinMax.xmn <= ItemHeader->MinMax.xmx &&
				(ItemHeader->MinMax.xmn != SHRT_MIN ||  
				 ItemHeader->MinMax.xmx != SHRT_MAX ||
				 ItemHeader->MinMax.ymn != SHRT_MIN ||
				 ItemHeader->MinMax.ymx != SHRT_MAX))
				AddMinMax (&NewFileMNMX,&ItemHeader->MinMax); 
			dblen = len;  
			if (dbn == 1101)
				dbn = 1101;
			dbn++;
			AddStreetNumber (pBuf,&len); 
			ConvertTAG (pBuf,&len); 
			tbuf = pBuf + len;
			if (DummyRec == 2)
			{ 
				id = 24;
				_fmemmove (tbuf,&id,2);
				len += 2; 
				tbuf += 2;
				ItemHeader->Len++;
			}
			_fmemmove (tbuf,&Trailer,8);
			len += 8;      
			GlobalUnlock (hBuf);
    		pBuf = GlobalLock (hBuf); 
		    First = TRUE;
		    MinMax = ItemHeader->MinMax;  
		    hRec = 0; 
		    while (SplitRec ((HPSHORT*)&pBuf,&len,(LPSHORT*)&Rec,&RecLen,&hBuf,&hRec,TRUE))
		    {   
		    	if (First && len > 0)
		    	{
					ItemHeader = (LPITEM)Rec; 
					ItemHeader->Len = 0;
		    	}
				loc = GetFileConnectOffset(Type,-1,MinMax,(short)RecLen,-1,Rec); 
				if (First)
				{
					First = FALSE;
					CurrentItem = loc - CurrentSeg -2;
					ItemSeg = CurrentSeg;
				}    
			    GSSiGlobUlFree (&hBuf);
			}
NextRec:			
			loc = GSSillseek (NewQuadFID,0,1);  
			if (loc == idebug)
				ii=1;
			PctBox (GetDlgItem(ReorghWnd,ReorgStatus), TotLen, loc,0);
        } 
	    GSSiGlobUlFree (&hBuf); 
        GSSiClose (NewQuadFID);   
	    MinMaxLoc = GSSillseek(FidMap,(LONG)-(2+12),2); 
	    if (ReorgUpdateBounds)
		{
			int	inc = ((int)NewFileMNMX.xmx - (int)NewFileMNMX.xmn) / 200;

			NewFileMNMX.xmx += inc;
			NewFileMNMX.xmn -= inc;
			
			inc = ((int)NewFileMNMX.ymx - (int)NewFileMNMX.ymn) / 200;

			NewFileMNMX.ymx += inc;
			NewFileMNMX.ymn -= inc;

	    	BigWrite (FidMap,(HPSTR)&NewFileMNMX,8,-1);
        }
		pFile = GlobalLock (hNewQuadFile);
		NewQuadFID = GSSiOpenFile (pFile,&OFStruct,OF_CREATE);
		GlobalUnlock (hNewQuadFile);
		WantQuadTree2 = FALSE;
        if (MinTime < LONG_MAX && ReorgUpdateTime)
        {
			GSSillseek (FidMap,PrimeOffset,0);
			BigRead (FidMap,(HPSTR)&len,2);
			BigRead (FidMap,(HPSTR)&NewPrimeOffs,sizeof(PRIMEOFFSETS));
			GSSillseek (FidMap,PrimeOffset,0);
		    NewPrimeOffs.MinTime=MinTime;
		    NewPrimeOffs.MaxTime=MaxTime;
			BigWrite (FidMap,(HPSTR)&len,2,-1);
			BigWrite (FidMap,(HPSTR)&NewPrimeOffs,sizeof(PRIMEOFFSETS),-1);
        }
        ReorgFile=TRUE;
	    ConvertFileBounds (FidMap); 
	}
	GSSiGlobFree (&hSymConversionTable);  
    if (hSetNewSymType)
    {   
    	LPSHORT	pNum=(LPSHORT)GlobalLock (hSetNewSymType);
    	short	Type, Num, SymNum,TemplateSymbol;  
    	char	Name[64], str[256];  
    	HANDLE	hSymbol, hTempSymbol;
    	LPSYMBOL	CurSymbol, TempSymbol;
    	
    	while (*pNum)
    	{   
    		Type = 1; 
    		SymNum = *pNum++;
    		Num = *pNum++;
    		if (*pNum > Num) 
    		{
    			Type = 2; 
    			Num = *pNum;
    		}
    		pNum++;
    		if (*pNum > Num) 
    		{
    			Type = 3; 
    			Num = *pNum;
    		} 
    		pNum++;
    		if (Num && Type == 3)
    		{
				char	NewSymbolTemplate[64]={"[%NEW_AREA_SYMBOL_TEMPLATE]"};

				hSymbol = GetDictSymDesc (SymNum,0);
				CurSymbol = GlobalLock (hSymbol);
				ExpandText (NewSymbolTemplate);
 				TemplateSymbol = GetDictSymbolNumber (NewSymbolTemplate); 
				if (TemplateSymbol)
				{
					hTempSymbol = GetDictSymDesc (TemplateSymbol,0); 
					TempSymbol = (LPSYMBOL)GlobalLock (hTempSymbol); 
					TempSymbol->Parent = CurSymbol->Parent;
					TempSymbol->Number = CurSymbol->Number;
					strcpy (TempSymbol->Name,CurSymbol->Name);
					GlobalUnlock (hSymbol);
					DestroySymbol (hSymbol);  
					hSymbol = hTempSymbol;
				}
				else
					CurSymbol->Type = Type;
				GlobalUnlock (hSymbol);
				ReplaceSymbol (SymNum,hSymbol);
				DestroySymbol (hSymbol);  
			}
	    }
		GSSiGlobUlFree (&hSetNewSymType);  
    }
	
{
#if ENABLETRACE
GSSiExitProg (811);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetMapTime (LPSTR FileName,LPSTR MinMax)
{
	PRIMEOFFSETS PrimeOffs;
	short	len;
	int	rtn=-1;

	strcpy (PltName,FileName);
	if (OpenMap (CurView->hWnd,(HDC)1))
	{
		GSSillseek (FidMap,PrimeOffset,0);
		BigRead (FidMap,(HPSTR)&len,2);
		BigRead (FidMap,(HPSTR)&PrimeOffs,sizeof(PRIMEOFFSETS));
		GSSillseek (FidMap,PrimeOffset,0);
		if (!stricmp (MinMax,"MIN"))
			rtn = PrimeOffs.MinTime;
		else
			rtn = PrimeOffs.MaxTime;
	    CloseMap(FALSE);
	}
	return rtn;
}

BOOL SetMapMinMaxTime (LPSTR FileName,long MinTime,long MaxTime)
{
	PRIMEOFFSETS PrimeOffs;
	short	len;
	BOOL	rtn=FALSE;

	if (!MaxTime)
		MaxTime = LONG_MAX;
	strcpy (PltName,FileName);
	if (OpenMap (CurView->hWnd,0))
	{
		GSSillseek (FidMap,PrimeOffset,0);
		BigRead (FidMap,(HPSTR)&len,2);
		BigRead (FidMap,(HPSTR)&PrimeOffs,sizeof(PRIMEOFFSETS));
		GSSillseek (FidMap,PrimeOffset,0);
		PrimeOffs.MinTime=MinTime;
		PrimeOffs.MaxTime=MaxTime;
		BigWrite (FidMap,(HPSTR)&len,2,-1);
		BigWrite (FidMap,(HPSTR)&PrimeOffs,sizeof(PRIMEOFFSETS),-1);
		rtn = TRUE;
	    CloseMap(FALSE);
	}
	return rtn;
}

long ReorgOut (LPSTR rec, WORD len)
#if ENABLETRACE
{GSSiEnterProg (812);
#endif
{   
	long	loc;
	
	if (!ReorgFile || ReorgfileFID == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (812);
#endif
		return FALSE;                       
}
	loc = GSSillseek (ReorgfileFID,0,1);
	BigWrite (ReorgfileFID,(HPSTR)&len,2,-1);
	BigWrite (ReorgfileFID,(HPSTR)rec,len,-1);
{
#if ENABLETRACE
GSSiExitProg (812);
#endif
	return loc;
}
#if ENABLETRACE
}
#endif
}   

void ReorgOut2 (HPSTR rec, long len)
#if ENABLETRACE
{GSSiEnterProg (813);
#endif
{   
	static	HANDLE	hRec=0;
	LPSTR	lpBuf;
	long	loc;  
	struct	{
				short	ID;
				long	Next;
				short	Term;
			}	ContinuationBlock; 
	static	HANDLE	hbuf=0;
	static	long	lbuf=0;
	static	HPSTR	pbuf;
	int		idesc;
	
	if (len<0)
	{   
		if (lbuf)
		{
			GlobalUnlock (hbuf);  
			pbuf = GlobalLock (hbuf);
			BigWrite (NewQuadFID,pbuf,lbuf,-1); 
		}
		lbuf=0; 
		GSSiGlobUlFree (&hbuf);  
    		GSSiGlobFree (&hRec);
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
		return;
}
	}
	if (NewQuadFID)
	{   
		if (!len)
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
			return;   
}
		if (!hbuf)
		{
			NumMapSegments=0; 
			MaxNewType=0; 
			hbuf = GSSiGlobAlloc ( 507,GMEM_MOVEABLE,MAXREORGBUF);
			pbuf = GlobalLock (hbuf);
			lbuf = 0;
		} 
		else
			NumMapSegments++;  
		if (len+lbuf+6>MAXREORGBUF)
		{
			GlobalUnlock (hbuf);  
			pbuf = GlobalLock (hbuf);
			if (BigWrite (NewQuadFID,pbuf,lbuf,-1) != lbuf)
			{   
				GSSiClose (NewQuadFID);
				NewQuadFID = 0;  
				ReorgFile = FALSE;
				GSSiClose (ReorgFile);
				HaltMapDisplay(FALSE,FALSE);
				GSSiMsgBox( hWndMain, "Error writing TEMP file - probably due to full disk","Reorg Aborted", MB_ICONEXCLAMATION,0);
			}
			lbuf = 0;
		}
		_fmemmove (pbuf,&len,4);
		pbuf+=4;
		_fmemmove (pbuf,&QuadTypeNext,2); 
		MaxNewType = max (MaxNewType,QuadTypeNext);
		pbuf+=2;
		hmemmove (pbuf,rec,len);
		pbuf+=len;  
		lbuf+=len+6;
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
		return;
}
	}
	if (!ReorgFile || ReorgfileFID == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
		return;  
}
	if (!ReorgDescFile)
	{
		ReorgOut3 (rec,len);
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
		return;                     
}
	}
	if (len)     
	{   
		idesc = GetRecDesc (rec,len);
		AddReorgDesc (idesc);
		BigWrite (ReorgDescFile,(HPSTR)&len,4,-1);
		BigWrite (ReorgDescFile,rec,len,-1); 
	}
	else
	{ 
		short	n=NumDescBlocks;
		long	l,ii;  
		HPSTR	lpRec; 
		BOOL	Done=FALSE, Any;
		
		if (!hReorgDesc)
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
			return;  
}
	    PctBox (GetDlgItem(ReorghWnd,IDC_STATUS), NumQuadSegs, NumQuadSegsProcessed,1);
		if (!hRec)
			hRec = GSSiGlobAlloc ( 508,GMEM_MOVEABLE,MAXREORGBUF);
		lpRec = GlobalLock (hRec);
		
		for (n=0;n<NumDescBlocks;n++)
		{   
			pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlockReorg);
			pDescBlock += ((long)QuadOff * (long)MaxQuadType * (long)NumDescBlocks +(long) QuadTypeNext* (long)NumDescBlocks + n); 
			if (Done)
				pDescBlock->Desc = -1;
			else 
			{
				pDescBlock->Desc = GetDescBlockDesc (n);
				if (!pDescBlock->Desc)
					Done = TRUE;
				pDescBlock->Offset = GSSillseek (ReorgfileFID,0,1);
				GSSillseek (ReorgDescFile,0,0);    
				Any = FALSE;
				while (BigRead (ReorgDescFile,(HPSTR)&l,4) == 4)
				{   
					BigRead (ReorgDescFile,lpRec,l);
					if (WantReorgRec (GetRecDesc(lpRec,l),n))
					{
						Any = TRUE;
						ReorgOut3 (lpRec,l);
					}
				} 
				if (Any)
					ReorgOut3 (lpRec,0);
				else 
				{
					Done = TRUE;
					pDescBlock->Desc = -1;
				}
			}
			GlobalUnlock (hDescBlockReorg);
		}
		GSSiGlobUlFree (&hRec);
		AddReorgDesc (-1); 
		GSSiChangeLength (ReorgDescFile,0); 
		GSSillseek (ReorgDescFile,0,0);
	}
{
#if ENABLETRACE
GSSiExitProg (813);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}    

void AddReorgDesc (int desc)
#if ENABLETRACE
{GSSiEnterProg (814);
#endif
{   
	int		i;               
	
	if (desc < 0)
	{
		GSSiGlobFree (&hReorgDesc);
		hReorgDesc = 0;  
		LastWhich = -1; 
		TotReorgDesc = 0;
{
#if ENABLETRACE
GSSiExitProg (814);
#endif
		return;
}
	} 
	if (!hReorgDesc)
	{
		hReorgDesc = GSSiGlobAlloc ( 509,GMEM_MOVEABLE,USHRT_MAX);
		NumReorgDesc = 0;
	}  
	TotReorgDesc++;
	pReorg = (LPREORG)GlobalLock (hReorgDesc);
	for (i=0;i<NumReorgDesc;i++,pReorg++)
	{
		if (desc == pReorg->Desc)
		{                   
			pReorg->Num++;
			goto Exit;
		}
	}
	pReorg->Desc = desc;
	pReorg->Num = 1; 
	NumReorgDesc++;
Exit:
	GlobalUnlock (hReorgDesc);
{
#if ENABLETRACE
GSSiExitProg (814);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void AllocateDescBlocks(void)
#if ENABLETRACE
{GSSiEnterProg (815);
#endif
{          
	LPDESCBLOCK	pDescBlock;  
	long		i;
	
	DescBlockLen = sizeof(DESCBLOCK) * NumQuadSegs * (long)MaxQuadType * (long)NumDescBlocks * 4;
	hDescBlockReorg = GSSiGlobAlloc ( 510,GMEM_MOVEABLE,DescBlockLen); 
	pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlockReorg);
	for (i=0;i<NumQuadSegs*(long)MaxQuadType*(long)NumDescBlocks;i++,pDescBlock++)
		pDescBlock->Desc=-1;
	GlobalUnlock (hDescBlockReorg);
{
#if ENABLETRACE
GSSiExitProg (815);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void LoadDescBlocks (long	Offset)
#if ENABLETRACE
{GSSiEnterProg (816);
#endif
{   
	LPDESCBLOCK	pDescBlock;
	short	ii;  
	
   	hDescBlock = 0;    
   	if (!Offset || !WantDescBlock)
{
#if ENABLETRACE
GSSiExitProg (816);
#endif
   		return;
}
    GSSillseek(FidMap,Offset,0); 
    BigRead (FidMap,(HPSTR)&DescBlockLen,4); 
    if (DescBlockLen > 30000)
    	ii=1;
    if (DescBlockLen)
    {
		hDescBlock = GSSiGlobAlloc ( 511,GMEM_MOVEABLE,DescBlockLen); 
		pDescBlock = (LPDESCBLOCK)GlobalLock (hDescBlock);
		BigRead (FidMap,(HPSTR)pDescBlock,DescBlockLen);
		GlobalUnlock (hDescBlock);
    }
{
#if ENABLETRACE
GSSiExitProg (816);
#endif
   	return;  
}
#if ENABLETRACE
}
#endif
}

int GetDescBlockDesc (int which)
#if ENABLETRACE
{GSSiEnterProg (817);
#endif
{
	long	MaxNum, SaveNum; 
	int		SaveDesc, i;
	LPREORG	pReorg1, MaxReorg;   

	pReorg1 = (LPREORG)GlobalLock (hReorgDesc);
	if (which) 
	{
		pReorg = pReorg1 + (which-1);
		pReorg->Num = 0;
	}
	if (which >= (NumDescBlocks-1) || TotReorgDesc < MinReorgDesc)
	{
		WantDesc = 0; 
		pReorg = pReorg1 + which;
		pReorg->Desc = 0;
		GlobalUnlock (hReorgDesc);
{
#if ENABLETRACE
GSSiExitProg (817);
#endif
		return 0;
}
	}
	MaxReorg = pReorg1;
	pReorg = pReorg1; 
	WantDesc = pReorg->Desc;
	MaxNum = pReorg++->Num;  
	for (i=1;i<NumReorgDesc;i++,pReorg++)
	{
		if (pReorg->Num > MaxNum)
		{
			MaxReorg = pReorg;
			MaxNum = pReorg->Num;
			WantDesc = pReorg->Desc;
		}
	}
	TotReorgDesc-=MaxNum; 
	pReorg = pReorg1 + which;
	SaveDesc = pReorg->Desc;
	SaveNum = pReorg->Num;
	*pReorg = *MaxReorg;
	MaxReorg->Desc = SaveDesc;
	MaxReorg->Num = SaveNum; 
	GlobalUnlock (hReorgDesc); 
	
{
#if ENABLETRACE
GSSiExitProg (817);
#endif
	return WantDesc;
}
#if ENABLETRACE
}
#endif
}

BOOL WantReorgRec (int desc, int which)
#if ENABLETRACE
{GSSiEnterProg (818);
#endif
{
	static	WhichDesc;
	int		i, SaveDesc;
	                 
	if (!hReorgDesc)
{
#if ENABLETRACE
GSSiExitProg (818);
#endif
		return TRUE; 
}
	pReorg = (LPREORG)GlobalLock (hReorgDesc);
	for (i=0;i<which;i++,pReorg++)
	{
		if (desc == pReorg->Desc)
			goto RtnFalse;
	}
	if (!pReorg->Desc || desc == pReorg->Desc)
	{
		GlobalUnlock (hReorgDesc);
{
#if ENABLETRACE
GSSiExitProg (818);
#endif
		return TRUE;
}
	}
RtnFalse:
	GlobalUnlock (hReorgDesc);
{
#if ENABLETRACE
GSSiExitProg (818);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 
     
void ReorgOut3 (HPSTR rec, long len)
#if ENABLETRACE
{GSSiEnterProg (819);
#endif
{   
	LPSTR	Rec; 
	HANDLE	hRec=0, hBuf;
	long	RecLen, lBuf;
	HPSTR	pBuf;
	int		ii;
	
	if (len < 32000)
	{
		ReorgOut4 (rec,len);
{
#if ENABLETRACE
GSSiExitProg (819);
#endif
		return;
}
	}  
	if (CurrentRefno == 1007241)
		ii=1;
	lBuf = len;
	hBuf = GSSiGlobAlloc ( 512,GHND,lBuf+32);   
	pBuf = GlobalLock (hBuf);
	hmemmove (pBuf,rec,len);    
	GlobalUnlock (hBuf);
	pBuf = GlobalLock (hBuf);
    while (SplitRec ((HPSHORT*)&pBuf,&lBuf,(LPSHORT*)&Rec,&RecLen,&hBuf,&hRec,FALSE))
    {   
    	ReorgOut4 (Rec,RecLen);
	    GSSiGlobUlFree (&hBuf);
	}
{
#if ENABLETRACE
GSSiExitProg (819);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ReorgOut4 (HPSTR rec, long len)
#if ENABLETRACE
{GSSiEnterProg (820);
#endif
{   
	LPSTR	lpBuf;
	long	loc;  
	struct	{
				short	ID;
				long	Next;
				short	Term;
			}	ContinuationBlock; 
	
	lpBuf = GlobalLock (hReorgBuf);
	lpBuf += lReorgBuf;
	if (!len)
	{    
		if (!lReorgBuf)
		{
			GlobalUnlock (hReorgBuf);
{
#if ENABLETRACE
GSSiExitProg (820);
#endif
			return;
}
		}
		ContinuationBlock.ID = 13;
		ContinuationBlock.Next = -1;
		ContinuationBlock.Term = 0;
		_fmemmove (lpBuf,&ContinuationBlock,sizeof(ContinuationBlock));
		lReorgBuf += sizeof(ContinuationBlock);
		GlobalUnlock (hReorgBuf);
		lpBuf = GlobalLock (hReorgBuf);
		loc = ReorgOut (lpBuf,(WORD)lReorgBuf);
		lReorgBuf = 0;
		if (QuadOffLoc)
		{
			GSSillseek (ReorgfileFID,ReorgStartQuad+QuadOffLoc,0);
			BigWrite (ReorgfileFID,(HPSTR)&loc,4,-1);
		} 
		QuadOffLoc = 0;
		GSSillseek (ReorgfileFID,0,2);
		GlobalUnlock (hReorgBuf);
{
#if ENABLETRACE
GSSiExitProg (820);
#endif
		return;
}
	}
	if (len + lReorgBuf > 32500)
	{	
		ContinuationBlock.ID = 13;
		ContinuationBlock.Next = 0;
		ContinuationBlock.Term = 0;
		_fmemmove (lpBuf,&ContinuationBlock,sizeof(ContinuationBlock));
		lReorgBuf += sizeof(ContinuationBlock);
		GlobalUnlock (hReorgBuf);
		lpBuf = GlobalLock (hReorgBuf);
		loc = ReorgOut (lpBuf,(WORD)lReorgBuf);
		lReorgBuf = 0;
		ContinuationBlock.Next = GSSillseek (ReorgfileFID,0,1);
		GSSillseek (ReorgfileFID,-6,2);
		BigWrite (ReorgfileFID,(HPSTR)&ContinuationBlock.Next,4,-1);  
		if (QuadOffLoc)
		{
			GSSillseek (ReorgfileFID,ReorgStartQuad+QuadOffLoc,0);
			BigWrite (ReorgfileFID,(HPSTR)&loc,4,-1);
		} 
		QuadOffLoc = 0;
		GSSillseek (ReorgfileFID,0,2);
	}   
	_fmemmove (lpBuf,rec,(size_t)len);
	lReorgBuf += len;
	GlobalUnlock (hReorgBuf);
{
#if ENABLETRACE
GSSiExitProg (820);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

long BuildNewQuadTree (HFILE ReorgFileFID,HFILE NewQuadFID)
#if ENABLETRACE
{GSSiEnterProg (821);
#endif
{
    LPQUAD	AtQuad,UpQuad, pQuad; 
	long	LenQuadSeg, MaxQuadLevel,MaxQuadType,LenQuad, NumQuadSegs, MaxQuadLength;
	long	NewLoc=0;  
	long	lMem;  
	int		i, i2, st;  
	HANDLE	hLadder, hbuf, hQuad;
	typedef struct {short	Level;
				    USHORT  Loc, Up;
				    short	SplitCoord;} LADDER;
	typedef LADDER	FAR	*LPLADDER;
	LPLADDER	pLadder, pLastLadder;
	short	Type, iloc;  
	long	len;
	long	Offset,loc=0,TotLen; 
	HPSTR	buf;
	LPITEM	ItemHeader;  
	OFSTRUCTGM	OFStruct;
	BTVARDESC	BTVar[2];   
	char	BTFile[128], ReorgDir[128];
	HANDLE	hBT1, hBT2;   
	short	X,Y;  
	struct	{short	Coord;
			 long	Unique;
			} BTKey;
	long	Unique, irec, endrec, TotQuadSegs, Skip=10,ii;  
	USHORT	NextLoc, AtLoc;
	short   Coord, SaveCoord, EndCoord, StartCoord;   
	//BTHEAD	BTHead;
	LPSTR	pDot; 
	DWORD	Err;
	
	if (NewLoc || NumMapSegments<100)
	{
		NewLoc=AddSimpleQuadTree (ReorgFileFID);	
{
#if ENABLETRACE
GSSiExitProg (821);
#endif
		return NewLoc;
}
	}
	GSSiGetTempFileName (0,"gm",0,(LPSTR)ReorgDir); 
	if ((pDot = _fstrrchr (ReorgDir,'.')))
		*pDot = 0;
	GSSiMakeDir (ReorgDir,&Err);
	MaxQuadType = max (2,MaxNewType)+1; 
	MaxQuadLength = (long)USHRT_MAX * GetGlobalLVal2 ("[%QUADLENGTH]",1);
Top:
	LenQuadSeg = 8 + 4 + 4 * MaxQuadType; 
	MaxQuadLevel=1; 
	NumQuadSegs=1;  
	TotQuadSegs=1;
	NumSegsPerQuad = NumMapSegments;
	if (NumMapSegments >40000)
		Skip=10;
	else if (NumMapSegments>10000)
		Skip=5;
	else
		Skip=2;

	MaxSegsPerQuad=50;   
	
	while (NumSegsPerQuad > MaxSegsPerQuad)
	{   
		NumQuadSegs*=2;  
		TotQuadSegs+=NumQuadSegs;
		MaxQuadLevel++; 
		NumSegsPerQuad=NumMapSegments/NumQuadSegs;
		LenQuad = TotQuadSegs * LenQuadSeg; 
		if (LenQuad > MaxQuadLength/2)
			goto MaxQuad;
	}  
MaxQuad:  
	if (LenQuadSeg < 32 && LenQuad > USHRT_MAX)
	{
		MaxQuadType = 5;
		goto Top;
	}
	NewLoc = GSSillseek (ReorgFileFID,0,1);	
    lMem = LenQuad + 4*4;
    BigWrite (ReorgFileFID,(HPSTR)&lMem,2,-1);
    BigWrite (ReorgFileFID,(HPSTR)&LenQuadSeg,4,-1); 
    BigWrite (ReorgFileFID,(HPSTR)&MaxQuadLevel,4,-1);
    BigWrite (ReorgFileFID,(HPSTR)&MaxQuadType,4,-1);
    BigWrite (ReorgFileFID,(HPSTR)&LenQuad,4,-1);    
    
    hQuad = GSSiGlobAlloc ( 513,GHND,lMem + sizeof(QUAD));
    pQuad = (LPQUAD)GlobalLock (hQuad);
    hLadder = GSSiGlobAlloc ( 514,GHND,sizeof(LADDER)*MaxQuadLevel);
    pLadder = (LPLADDER)GlobalLock (hLadder); 
    pLadder->Loc = 1;  
    pLadder->Level=1;
    pLadder->Up = 0;
    
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0; 
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=2; 
	sprintf (BTFile,"%s\\level%i.btr",ReorgDir,pLadder->Level);
	BT_CREATE (BTFile, 2, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBT2 = BT_OPEN (BTFile, 0, BT_WRITE, 0);
	TotLen = GSSillseek (NewQuadFID,0,2);   
	GSSillseek (NewQuadFID,0,0);   
	Unique = 0;
	while (BigRead (NewQuadFID,(HPSTR)&len,4))
	{
		BigRead (NewQuadFID,(HPSTR)&Type,2);
		hbuf = GSSiGlobAlloc ( 515,GMEM_MOVEABLE,len);
		buf = GlobalLock (hbuf);  
		BigRead (NewQuadFID,buf,len);
		if (!(Unique%Skip))
		{ 
			ItemHeader = (LPITEM)buf; 
			X = ((long)ItemHeader->MinMax.xmn + (long)ItemHeader->MinMax.xmx)/2;
			Y = ((long)ItemHeader->MinMax.ymn + (long)ItemHeader->MinMax.ymx)/2;
			BTKey.Coord = X;
			BTKey.Unique = Unique;
			BT_PUT (hBT2,(LPSTR)&BTKey,(LPSTR)&Y);  
			loc = GSSillseek (NewQuadFID,0,1);   
			PctBox (GetDlgItem(ReorghWnd,ReorgStatus), TotLen, loc,0);

		}
	    GSSiGlobUlFree (&hbuf);
		Unique++;  
    } 
    
	NextLoc = 2;  
    AtQuad = pQuad;
	AtQuad->MinMax.xmn = SHRT_MIN;
	AtQuad->MinMax.xmx = SHRT_MAX;
	AtQuad->MinMax.ymn = SHRT_MIN;
	AtQuad->MinMax.ymx = SHRT_MAX;
	AtQuad->Next[0] = NextLoc++;
	AtQuad->Next[1] = NULLOFF;
	for (i=0;i<MaxQuadType;i++)
		AtQuad->TypeOffset[i]=-1;
	StartCoord = SHRT_MIN;
	//GetBTHeader (hBT2,&BTHead);
	endrec = BT_NUM_IN_INDEX (hBT2)/2; 
	hBT1 = hBT2;
	iloc = 0;
	
GoDown:
	UpQuad = AtQuad;
	AtQuad = (LPQUAD)((HPSTR)pQuad + ((long)(UpQuad->Next[iloc]-1))*LenQuadSeg);
	AtLoc = UpQuad->Next[iloc]; 
	if (pLadder->Level+1 < MaxQuadLevel)
	{
		sprintf (BTFile,"%s\\level%i.btr",ReorgDir,pLadder->Level+1);
		BT_CREATE (BTFile, 2, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hBT2 = BT_OPEN (BTFile, 0, BT_WRITE, 0); 
	}
	else
	{
		BTFile[0]=0;
		hBT2 = 0;
	}
	irec = 0;
	BTKey.Unique = 0;
	BTKey.Coord = StartCoord;
	st = BT_FIND (hBT1,(LPSTR)&BTKey,BT_FIRST,BT_GE,(LPSTR)&Coord);
	while (!st)
	{
		if (irec == endrec)
			EndCoord = BTKey.Coord;
		else if (irec > endrec)
		{
			if (BTKey.Coord != EndCoord)
				goto EndSplit;
		}
		irec++;
		if (hBT2)
		{
			SaveCoord = Coord;
			Coord = BTKey.Coord;
			BTKey.Coord = SaveCoord; 
			BT_PUT (hBT2,(LPSTR)&BTKey,(LPSTR)&Coord); 
		} 
		st = BT_FIND (hBT1,(LPSTR)&BTKey,BT_NEXT,BT_ANY,(LPSTR)&Coord);		
		CheckForContinue(TRUE, 0);
	}    
EndSplit:   
	if (!irec)
		ii=0;
	if (AtLoc == UpQuad->Next[0])
		pLadder->SplitCoord = BTKey.Coord; 
	BT_CLOSE (hBT1);
	if (pLadder->Level%2)
	{   
		AtQuad->MinMax = UpQuad->MinMax;
		if (AtLoc == UpQuad->Next[0])
			AtQuad->MinMax.xmx = pLadder->SplitCoord-1; 
		else
			AtQuad->MinMax.xmn = pLadder->SplitCoord; 
	}
	else
	{
		AtQuad->MinMax = UpQuad->MinMax;
		if (AtLoc == UpQuad->Next[0])
			AtQuad->MinMax.ymx = pLadder->SplitCoord-1;
		else
			AtQuad->MinMax.ymn = pLadder->SplitCoord;
	} 
	if (AtQuad->MinMax.xmx<=AtQuad->MinMax.xmn ||
		AtQuad->MinMax.ymx<=AtQuad->MinMax.ymn)
		ii=1;
	for (i=0;i<MaxQuadType;i++)
		AtQuad->TypeOffset[i]=-1;
	AtQuad->Next[1] = NULLOFF;
	if (pLadder->Level+1 == MaxQuadLevel)  
	{
		AtQuad->Next[0] = NULLOFF;    
		UpQuad->Next[1] = NextLoc++;
		AtQuad = (LPQUAD)((HPSTR)pQuad + ((long)(UpQuad->Next[1]-1))*LenQuadSeg); 
		*AtQuad = *UpQuad;
		AtQuad->Next[0] = NULLOFF;    
		AtQuad->Next[1] = NULLOFF;    
		if (pLadder->Level%2)
			AtQuad->MinMax.xmn = pLadder->SplitCoord; 
		else
			AtQuad->MinMax.ymn = pLadder->SplitCoord;
		if (AtQuad->MinMax.xmx<=AtQuad->MinMax.xmn ||
			AtQuad->MinMax.ymx<=AtQuad->MinMax.ymn)
			ii=1;
		goto GoUp;
	}
	else
	{   
		StartCoord = SHRT_MIN;
		//GetBTHeader (hBT2,&BTHead);
		endrec = BT_NUM_IN_INDEX (hBT2)/2; 
		hBT1 = hBT2;
		AtQuad->Next[0] = NextLoc++;  
		pLastLadder = pLadder;
		pLadder++;
		pLadder->Level = pLastLadder->Level + 1;
		pLadder->Up = pLastLadder->Loc;
		pLadder->Loc = AtLoc; 
		iloc = 0;
		goto GoDown;
	}
	
GoUp: 
	if (BTFile[0])
		GSSiRemove (BTFile);
	if (pLadder->Level > 1)
		pLadder--;    
	AtQuad = (LPQUAD)((HPSTR)pQuad + ((long)(pLadder->Loc-1))*LenQuadSeg); 
	AtLoc = pLadder->Loc;
	if (AtQuad->Next[1] == NULLOFF) 
	{
		AtQuad->Next[1] = NextLoc++;  
		iloc = 1;
		StartCoord = pLadder->SplitCoord;  
		endrec = LONG_MAX;  
		sprintf (BTFile,"%s\\level%i.btr",ReorgDir,pLadder->Level);
		hBT1 = BT_OPEN (BTFile, 0, BT_READ, 0); 
		goto GoDown;                     
	}
	if (pLadder->Level > 1) 
	{
		sprintf (BTFile,"%s\\level%i.btr",ReorgDir,pLadder->Level);
		goto GoUp;
	}	
	ii=1;
	AtQuad = (LPQUAD)((HPSTR)pQuad + ii*LenQuadSeg); 
	BigWrite (ReorgFileFID,(HPSTR)pQuad,lMem,-1);
	i2 = 0;  
	BigWrite (ReorgFileFID,(HPSTR)&i2,2,-1); 
	GSSiGlobUlFree (&hLadder);
	GSSiGlobUlFree (&hQuad);
	DeleteDirAndContents (ReorgDir);
{
#if ENABLETRACE
GSSiExitProg (821);
#endif
	return NewLoc;
}
#if ENABLETRACE
}
#endif
}

void SetPickGlobalsFromThemeHighlightData (LPTHEMEHIGHLIGHTKEY pThemeHighlightKey,LPTHEMEHIGHLIGHTDATA pThemeHighlightData)
{
	SetGlobalValueLong ("%PICKED_REFNO",pThemeHighlightKey->Refno); 
	SetGlobalValueLong ("%PICKED_CLASS",pThemeHighlightKey->Class); 
	SetGlobalValueLong ("%PICKED_TYPE",pThemeHighlightData->Type); 
	SetGlobalValueLong ("%PICKED_TYPE",pThemeHighlightData->Type); 
	SetGlobalValueBounds ("%PICKED_BOUNDS",&pThemeHighlightData->Bounds);
	return;
}


void SetPickGlobals (int item)
#if ENABLETRACE
{GSSiEnterProg (822);
#endif
{   
	DPOINT	DPoint; 
	char	OutStr[256],Text[1024];
	MNMXCORD	Rect; 
	LPVIEWPORT	SaveVP=CurView;
    
    FileNum = PickList[item].FileNum;
    SetPickGlobalsCalled = TRUE; 
	SetConfig (PickList[item].ConfigID);
	SetViewport(PickList[item].ViewID);
 	SetGlobalValue ("%PICKED_VIEWPORT",CurView->Name); 
    if (FileNum >= 0 && FileNum < CurView->NumFiles) 
    {
    	GetPickName (item);
    	_fstrupr (PickName);
		SetGlobalValue ("%PICKED_FILE",PickName);   
		SetGlobalValue ("%PICKED_LAYER",CurView->FileID[FileNum]);   
	}
	else
	{
		*PickName = 0;
		SetGlobalValue ("%PICKED_LAYER","");   
		SetGlobalValue ("%PICKED_FILE",""); 
	}
	SetGlobalValueLong ("%PICKED_REC",PickList[item].Segment); 
	SetGlobalValueLong ("%PICKED_LAYERNUM",FileNum); 
	SetGlobalValueLong ("%PICKED_ITEM",item+1); 
	SetGlobalValueLong ("%PICKED_REFNO",PickList[item].Refno); 
	SetGlobalValueLong ("%PICKED_MSLINK",PickList[item].MSLink); 
	SetIntRefno (PickList[item].Refno);
	SetGlobalValueLong ("%PICKED_TYPE",PickList[item].Type); 
	SetGlobalValueLong ("%PICKED_HASTEXT",(long)PickList[item].HasText); 
	GetGlobalVal (hTEXT,Text,0); 
	SetGlobalValue ("%PICKED_TEXT",Text); 
	
	SetGlobalValue ("%PICKED_PREFIX",PickList[item].Prefix); 
	strncpy0 (CurrentPrefix,PickList[item].Prefix,MAX_PREFIX_LEN); 
	SetGlobalValue ("%PICKED_UDI",PickList[item].UDI); 
	strncpy0 (CurrentUDI,PickList[item].UDI,MAX_UDI_LEN);
	SetGlobalValueLong ("%PICKED_SYMNUM",IDNINT(PickList[item].Desc)); 
	CurrentDesc = PickList[item].Desc;
    SetGlobalValue ("%PREFIX",PickList[item].Prefix);  
	SetUDIValue (PickList[item].Prefix,PickList[item].UDI); 
	if (ConvertPickedItems)
		SetGlobalValueReal ("%PICKED_LENGTH",ConvertDist(PickList[item].Length,OutDistUnits));
	else
		SetGlobalValueReal ("%PICKED_LENGTH",PickList[item].Length);
	SetGlobalValueReal ("%PICKED_PCT",PickList[item].PCT);
	if (ConvertPickedItems)
		SetGlobalValueReal ("%PICKED_OFFDIST",ConvertDist(PickList[item].OffDist,OutDistUnits));
	else
		SetGlobalValueReal ("%PICKED_OFFDIST",PickList[item].OffDist);
	SetGlobalValueReal ("%PICKED_AZ",PickList[item].PPAZ);
	SetGlobalValueReal ("%PICKED_BPAZ",PickList[item].BPAZ);
	SetGlobalValueReal ("%PICKED_EPAZ",PickList[item].EPAZ);
	if (ConvertPickedItems)
		SetGlobalValueReal ("%PICKED_AREA",ConvertArea (PickList[item].Area,OutAreaUnits));  
	else
		SetGlobalValueReal ("%PICKED_AREA",PickList[item].Area);  
	SetGlobalValueLong ("%PICKED_NUMPOINTS",PickList[item].NumPoints); 
	SetGlobalValueLong ("%PICKED_NEARPOINTID",PickList[item].NearPoint); 
	DPoint = PickList[item].BeginPoint;
	if (ConvertPickedItems)
		ConvertCoord(&DPoint,1,3);
	CurBP = DPoint;
	SetGlobalValueReal ("%PICKED_BP_X",DPoint.x);
	SetGlobalValueReal ("%PICKED_BP_Y",DPoint.y); 
	DPoint = PickList[item].EndPoint;
	if (ConvertPickedItems)
		ConvertCoord(&DPoint,1,3);  
	CurEP = DPoint;
	SetGlobalValueReal ("%PICKED_EP_X",DPoint.x);
	SetGlobalValueReal ("%PICKED_EP_Y",DPoint.y);
	DPoint = PickList[item].PickedPoint;
	if (ConvertPickedItems)
		ConvertCoord(&DPoint,1,3); 
	SetGlobalValueReal ("%PICKED_POINT_X",DPoint.x);
	SetGlobalValueReal ("%PICKED_POINT_Y",DPoint.y);
	SetGlobalValueReal ("%PICKED_POINT_Z",PickList[item].Elev);
	DPoint = PickList[item].NodePoint;
	if (ConvertPickedItems)
		ConvertCoord(&DPoint,1,3); 
	CurMP = DPoint;
	SetGlobalValueReal ("%PICKED_NODEPOINT_X",DPoint.x);
	SetGlobalValueReal ("%PICKED_NODEPOINT_Y",DPoint.y);
	SetGlobalValueReal ("%PICKED_CHORDDIST",ConvertDist(ldistp(PickList[item].FromPoint,PickList[item].ToPoint),OutDistUnits));
	if (ConvertPickedItems)
		ConvertRectCoord (&Rect,&PickList[item].Rect,1,3);
	else
		Rect = PickList[item].Rect;
	sprintf (OutStr,"%.14lg %.14lg %.14lg %.14lg",Rect.xmn,Rect.ymn,Rect.xmx,Rect.ymx);
    SetGlobalValue ("%PICKED_BOUNDS",OutStr);
	CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (822);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}    

long GetActualItemLength (LPSHORT ipnt)
#if ENABLETRACE
{GSSiEnterProg (823);
#endif
{
	short	ItemLen,ii;
	long	len=0;
	LPBYTE	Pcode;
	BOOL	First=TRUE; 
	LPSTR	StartPnt=(LPSTR)ipnt; 
	HANDLE	hSeg=0;
         
         
    while (*ipnt != 0)
    {   Pcode = (LPBYTE)ipnt;
       	CurElementPnt = ipnt++; 
        	

	    switch (*Pcode)
        {   
			case 93: //removed record
            case 92:
			case 12: /* item minmax */
			{	
				if (First)
				{
					First = FALSE;
					ipnt += 4;
					ItemLen = abs(*ipnt);
					ipnt++;  
					len += ((LPSTR)ipnt - StartPnt);
					StartPnt = (LPSTR)ipnt;
				}
				else
				{   
	Exit:           GSSiGlobUlFree (&hSeg);
{
#if ENABLETRACE
GSSiExitProg (823);
#endif
					return len;
}
				}
            }
            break;

			case 13: /* continuation offset */
			{	
				short	nBytes, nRead;
				long	ContinueOff;
				
				ContinueOff = *(LPLONG)ipnt;
				if (ContinueOff >=0)
				{   
					GSSiGlobUlFree (&hSeg);
				    GSSillseek (FidMap,ContinueOff,0);  
				    if (BigRead (FidMap,(HPSTR)&nBytes,2) != 2)
				    	goto ErrExit;
				    if (nBytes<=0)
				        goto ErrExit;  
				    hSeg = GSSiGlobAlloc ( 516,GMEM_MOVEABLE,(DWORD)nBytes);
				    ipnt = (LPSHORT)GlobalLock (hSeg); 
				    nRead = BigRead (FidMap,(HPSTR)ipnt,nBytes);  
				    if (nRead != nBytes) 
				    {
	ErrExit:   
				       	ProcessInvalidRecord (0,0,2);
{
#if ENABLETRACE
GSSiExitProg (823);
#endif
				        return FALSE;
}
				    } 
		        	StartPnt = (LPSTR)ipnt;
				} 
				else
					goto Exit;
            }
            break;  
            
	    	case 33: /* jump to long rec and back */
	    	{
				ContinuationOffset = *(LPLONG)ipnt; 
				ipnt += 2;    
				JumpBackSeg = CurrentSeg;
//				JumpBackElement = ipnt - BeginSeg;
//				JumpBackElement++;
			}
			break; 
					
	    	case 34: /* jump to long rec and back */
	    	{
				ContinuationOffset = JumpBackSeg; 
//				StartElement = JumpBackElement;
				*ipnt = 0;	
			}
			break; 
					
	        default:
	        	SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);
	        	len += ((LPSTR)ipnt - StartPnt);   
	        	StartPnt = (LPSTR)ipnt;
	        break;

		}
    }
{
#if ENABLETRACE
GSSiExitProg (823);
#endif
	return len;
}
#if ENABLETRACE
}
#endif
}  


short LoadCompleteItem (HPSTR pSeg,LPSHORT ipnt,long ActualLength)
#if ENABLETRACE
{GSSiEnterProg (824);
#endif
{ 
	short	ItemLen,ii, rtn=0;
	long	len=0;
	LPBYTE	Pcode;
	BOOL	First=TRUE; 
	LPSTR	StartPnt=(LPSTR)ipnt; 
	HANDLE	hSeg=0;
	long	PieceLen;
         
         
    while (*ipnt != 0)
    {   Pcode = (LPBYTE)ipnt;
       	CurElementPnt = ipnt++; 
        	

	    switch (*Pcode)
        {   
			case 93: //removed record
            case 92:
			case 12: /* item minmax */
			{	
				if (First)
				{
					First = FALSE;
					ipnt += 4;
					ItemLen = abs(*ipnt);
					ipnt++; 
					PieceLen = (LPSTR)ipnt - StartPnt; 
					len += PieceLen;
					_fmemmove (pSeg,StartPnt,(size_t)PieceLen);
					pSeg+=PieceLen;
					StartPnt = (LPSTR)ipnt;
				}
				else
				{   
	Exit:           GSSiGlobUlFree (&hSeg);
{
#if ENABLETRACE
GSSiExitProg (824);
#endif
					return rtn;
}
				}
            }
            break;

			case 13: /* continuation offset */
			{	
				short	nBytes, nRead;
				long	ContinueOff;
				
				rtn = 0;
				ContinueOff = *(LPLONG)ipnt;
				if (ContinueOff >=0)
				{   
					GSSiGlobUlFree (&hSeg);
				    GSSillseek (FidMap,ContinueOff,0);  
				    if (BigRead (FidMap,(HPSTR)&nBytes,2) != 2)
				    	goto ErrExit;
				    if (nBytes<=0)
				        goto ErrExit;  
				    hSeg = GSSiGlobAlloc ( 517,GMEM_MOVEABLE,(DWORD)nBytes);
				    ipnt = (LPSHORT)GlobalLock (hSeg); 
				    nRead = BigRead (FidMap,(HPSTR)ipnt,nBytes);  
				    if (nRead != nBytes) 
				    {
	ErrExit:   
				       	ProcessInvalidRecord (0,0,2);
{
#if ENABLETRACE
GSSiExitProg (824);
#endif
				        return 0;
}
				    } 
		        	StartPnt = (LPSTR)ipnt;
				} 
				else
					goto Exit;
            }
            break;  
            
	    	case 33: /* jump to long rec and back */
	    	{
				ContinuationOffset = *(LPLONG)ipnt; 
				ipnt += 2;    
				JumpBackSeg = CurrentSeg;
//				JumpBackElement = ipnt - BeginSeg;
//				JumpBackElement++;
			}
			break; 
					
	    	case 34: /* jump to long rec and back */
	    	{
				ContinuationOffset = JumpBackSeg; 
//				StartElement = JumpBackElement;
				*ipnt = 0;	
			}
			break; 
					
	        default:
	        	SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);
				PieceLen = (LPSTR)ipnt - StartPnt; 
				len += PieceLen;
				_fmemmove (pSeg,StartPnt,(size_t)PieceLen);
				pSeg+=PieceLen;
	        	StartPnt = (LPSTR)ipnt;
	        break;

		}
    }
{
#if ENABLETRACE
GSSiExitProg (824);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void SimplePointer(HDC hDC, LPDPOINT p1, LPDPOINT p2, short width, short ToPointOffset, short TipWidth, COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (825);
#endif
{
	POINT	P1, P2;
	HPEN	ArrowPen;

	P1 = BasePtToWinPt(p1);
	P2 = BasePtToWinPt(p2);
	SetDisplayMode(hDC, GF_TEXTMODE);
	ArrowPen = CreatePen(PS_SOLID, (short)IDNINT(width*DeviceToScreenFactor()), Color);
	DrawPointerLine(hDC, P1, P2, ArrowPen, ArrowPen, TipWidth, ToPointOffset);
	DeleteObject(ArrowPen);
	{
#if ENABLETRACE
		GSSiExitProg (825);
#endif
		return;
	}
#if ENABLETRACE
}
#endif
}

double RotateTextForEasyRead(double TXRot)
{
	if (TXRot > HALFPI && TXRot < 3 * HALFPI)
	{
		TXRot = LTWOPI(TXRot + PY);
	}
	return TXRot;
}

void DimensionLine(HDC hDC, LPDPOINT p1, LPDPOINT p2, short opt, COLORREF Color, int units, LPSTR format, double txtsize)
#if ENABLETRACE
{
	GSSiEnterProg(825);
#endif
	{
		POINT	P1, P2, P;
		HPEN	ArrowPen;
		int		width = 2;
		int		TipWidth = 2;
		int		ToPointOffset = 10;
		double	dist = ldistpp(p1, p2);
		double  textsize = 18;
		double	az = LTWOPI(getazd(p1, p2));// +HALFPI);
		int h;
		DPOINT p;
		char	txt[64];
		int		oldBKMode;
		COLORREF oldTextColor;
		char defaultformat[] = "%.1f";

		if (txtsize > 0)
			textsize = txtsize;
		az = RotateTextForEasyRead(az);
		units = max(1, units);
		if (!format || !*format)
			format = defaultformat;
		if (p1->x < 0 || p2->x < 0)
			return;
		p.x = (p1->x + p2->x) / 2;
		p.y = (p1->y + p2->y) / 2;
		dist = ConvertDist(dist, units);
		P1 = BasePtToWinPt(p1);
		P2 = BasePtToWinPt(p2);
		P = BasePtToWinPt(&p);
		SetDisplayMode(hDC, GF_TEXTMODE);
		oldBKMode = SetBkMode(hDC, OPAQUE);
		oldTextColor = SetTextColor(hDC, Color);
		ArrowPen = CreatePen(PS_SOLID, (short)IDNINT(width*DeviceToScreenFactor()), Color);
		DrawDimensionLine(hDC, P1, P2, ArrowPen, ArrowPen,10,0);
	/*	DWORD DispText(HDC hDC, BOOL GetExtents, int left, int right, int y, int symsize, int hJust, int vJust, double InSize, double SymbolSizeFactor, double SymbolTextFactor,
			int InWeight, BOOL Italic, double AZIN, LPSTR InText, int shield, BOOL TestRect,
			BOOL Shadow, COLORREF ShadowColor,long RemoveColor,long nPnts,HANDLE hAreaPoints,HANDLE hAreaAccelerator,
			int nPoly,HANDLE hPolyPartLen,short UseHalfTone,LPSTR ActualText,short MinSize,LPTHEME CurTheme,LPRECT pTextRect,LPRECT pFullRect,LPRECT pFlagRect)
			*/
		sprintf(txt, format, dist);
		h = textsize * DeviceToScreenFactor();
		DispText(hDC, FALSE, P.x, P.x,P.y, 0, 2, 2, h, 1, 1, 2, FALSE, az, txt, 0, FALSE, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		SetBkMode(hDC, oldBKMode);
		SetTextColor (hDC,oldTextColor);
		DeleteObject(ArrowPen);
		{
#if ENABLETRACE
			GSSiExitProg(825);
#endif
			return;
		}
#if ENABLETRACE
	}
#endif
}






short SetPickAp(LPBOOL pUseSymbolWidth)
#if ENABLETRACE
{GSSiEnterProg (828);
#endif
{   
    POINT   WinPT1, WinPT2;
    DPOINT  WPoint1, WPoint2; 
    
    if (pUseSymbolWidth)
    	*pUseSymbolWidth = TRUE;
    if (UseUserPickAp)
	    PickApW = UserPickAP; 
    else
    	PickApW = SystemPickAp; 
    PickAp = 0;
    if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (828);
#endif
    	return (PickAp);
}
    if (!PickApW)
{
#if ENABLETRACE
GSSiExitProg (828);
#endif
    	return (PickAp);
}
    WinPT1.x = (CurView->Rect.left + CurView->Rect.right)/2;
    WinPT1.y = (CurView->Rect.top + CurView->Rect.bottom)/2;
    if (PickApW > 0) 
    {
        PickAp = (short)IDNINT (PickApW); 
        WinPT2.x = WinPT1.x + PickAp;
        WinPT2.y = WinPT1.y; 
        WPoint1 = WinPtToBasePt(WinPT1);
        WPoint2 = WinPtToBasePt(WinPT2);
        PickApW = ldistp (WPoint1,WPoint2);
    }
    else 
    {   
    	if (pUseSymbolWidth)
    		*pUseSymbolWidth = FALSE;
        PickApW = -PickApW / MetersPerDegree; 
        WPoint1 = WinPtToBasePt(WinPT1);
        WPoint2.x = WPoint1.x + PickApW;
        WPoint2.y = WPoint1.y;
        WinPT2 = BasePtToWinPt (&WPoint2);
        PickAp = (short)IDNINT (idist (WinPT1,WinPT2));
        PickAp = max (PickAp,1);
    } 
{
#if ENABLETRACE
GSSiExitProg (828);
#endif
    return (PickAp);
}
#if ENABLETRACE
}
#endif
}



void SetTAGDialog (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (830);
#endif
{   
	short	nRc;
	DLGPROC lpfnSETTAGMsgProc;
	
	lpfnSETTAGMsgProc = MakeProcInstance((DLGPROC)SETTAGMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SETTAG", hWnd, lpfnSETTAGMsgProc);
	FreeProcInstance(lpfnSETTAGMsgProc);
{
#if ENABLETRACE
GSSiExitProg (830);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}



BOOL GetTAGForSymbol (LPSTR SymName,LPSTR Prefix,LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (832);
#endif
{ 
	LPTAGDEF	lpTAGDef; 
	short		i;  
	BOOL		rtn=FALSE;

	*Prefix = 0;
	if (!_fstrcmp (SymName,"PEN1") && NumTAGDef > 0) 
	{
		_fstrcpy (Prefix,"AIRPHOTO");
		lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
	    for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
	    if (!_fstrcmp (lpTAGDef->Prefix,Prefix))
	    {    
	    	_fstrcpy (UDI,lpTAGDef->CurUDI);
	    	rtn = TRUE;
	    }
		GlobalUnlock (hTAGDef);  
	}
{
#if ENABLETRACE
GSSiExitProg (832);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL UpdateTAGValue (HWND hWndDlg)
#if ENABLETRACE
{GSSiEnterProg (833);
#endif
{   
	char	Prefix[10], NewUDI[66]; 
	short	i;
	LPTAGDEF   lpTAGDef;
		
	GetDlgItemText (hWndDlg,IDC_TAGPREFIX,Prefix,10);
	GetDlgItemText (hWndDlg,IDC_TAGUDI,NewUDI,66);
	if (!*Prefix)
{
#if ENABLETRACE
GSSiExitProg (833);
#endif
		return TRUE;
}
	lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
    for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
    if (!_fstrcmp (lpTAGDef->Prefix,Prefix))
    {    
    	_fstrcpy (lpTAGDef->CurUDI,NewUDI);
    }
	GlobalUnlock (hTAGDef);       
	
{
#if ENABLETRACE
GSSiExitProg (833);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 





void SetSYMDialog (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (836);
#endif
{   
	short	nRc;
	DLGPROC lpfnSETSYMMsgProc;
	
	lpfnSETSYMMsgProc = MakeProcInstance((DLGPROC)SETSYMMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SETSYM", hWnd, lpfnSETSYMMsgProc);
	FreeProcInstance(lpfnSETSYMMsgProc);
{
#if ENABLETRACE
GSSiExitProg (836);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}





BOOL SetRDFColorAndWidth (int idesc,LPSHORT pNewOb,LPSTR WantType,COLORREF color,int width)
{   
	char	str[66];  
	short	Parent, type, wanttype,newob;
	BOOL	IsPar;
	char	TypeNames[5][6]={"POINT","LINE","AREA","TEXT","ALL"};
	
	if (!GetSymbolName (idesc,str,&Parent,0,&IsPar))
		return FALSE;
	if (IsPar)
	{
		HANDLE	hChildren=0;
		short	nChildren; 
		LPSHORT	Child; 
		
		GetSymDictChildren (idesc,&nChildren,&hChildren,-1,TRUE);
		if (hChildren)
		{  
			Child = (LPSHORT)GlobalLock (hChildren);
			while (nChildren--)
				SetRDFColorAndWidth (*Child++,pNewOb,WantType,color,width);
			GSSiGlobUlFree (&hChildren); 
			return TRUE; 
		}
		else
			return FALSE;
	} 
	type = GetDictSymbolType (idesc);
	for (wanttype=0;wanttype<5;wanttype++)
		if (!_fstricmp (WantType,TypeNames[wanttype]))
		{
			wanttype++;
			goto Set; 
		}
	wanttype = 5;
Set:
	if (*pNewOb == -1)
	{
   		*pNewOb = NextNewObject(FALSE);
   		if (*pNewOb == -1)
   			return FALSE;  
		if (wanttype == 4)
		{
			CurView->NewObjectSetTextColor[*pNewOb] = TRUE;
			CurView->NewObjectTextColor[*pNewOb].rgbtRed = GetRValue (color);
			CurView->NewObjectTextColor[*pNewOb].rgbtGreen = GetGValue (color);
			CurView->NewObjectTextColor[*pNewOb].rgbtBlue = GetBValue (color);
	   	}
		else if (wanttype == 2)
	       	SetClassLineType (*pNewOb,wanttype,width,0,0,color);
		else if (wanttype == 5)
			SetClassColor (*pNewOb,3,color);
		else
			SetClassColor (*pNewOb,wanttype,color);
	}
	if (*pNewOb == -2)
		CurView->NewObjectMap[idesc]=0; 
	else if (wanttype == 5 || type == wanttype)
		CurView->NewObjectMap[idesc]=*pNewOb+1; 
	return TRUE;
}









 



double GetTextHeadSize (LPGRTEXTHEADER pGRTextHeader)
#if ENABLETRACE
{GSSiEnterProg (845);
#endif
{
    double THeight;
    
	switch (pGRTextHeader->HeightPrecision)
	{
		case 0:  
			THeight = (double)pGRTextHeader->Height;
			break;
		case 1:
			THeight = (double)pGRTextHeader->Height/10;
			break;
		case 2:
			THeight = (double)pGRTextHeader->Height/100;
			break;
		case 3:
			THeight = (double)pGRTextHeader->Height/1000;
			break;
	} 
{
#if ENABLETRACE
GSSiExitProg (845);
#endif
	return THeight;
}
#if ENABLETRACE
}
#endif
}

void SetTextHeadSize (LPGRTEXTHEADER GRTextHeader,double dHeight)
#if ENABLETRACE
{GSSiEnterProg (846);
#endif
{
   	UINT	isize;
   	
	if (dHeight < 64)
	{
		GRTextHeader->HeightPrecision = 3;
		isize = IDNINT (dHeight * 1000);
	}    
	else if (dHeight < 640)
	{
		GRTextHeader->HeightPrecision = 2;
		isize = IDNINT (dHeight * 100);
	}    
	else if (dHeight < 6400)
	{
		GRTextHeader->HeightPrecision = 1;
		isize = IDNINT (dHeight * 10);
	}  
	else  
		isize = IDNINT(dHeight);
	GRTextHeader->Height = isize; 
{
#if ENABLETRACE
GSSiExitProg (846);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


 

BOOL RemoveDummyDelete (int Item)
{   
	PICKDATA SavePickList = PickList[Item];
	short	ii;
	
	if (PickByRefno(PickList[Item].Refno,0,0,-1))
		DeletePickedItem (0,12,93);
	return TRUE;
}



BOOL DeleteHighlightedItems (short Function)
#if ENABLETRACE
{GSSiEnterProg (852);
#endif
{   
	HIGHLIGHTDATA	HighlightData;
	short pos=BT_FIRST;
	long	Loc,Ref;
	//BTHEAD	BTHead;  
	char	Mess[128], Type[12]="Delete"; 
	short	OldType=12,NewType=92; 
	HCURSOR hcurSave;  
						
	
	if (Function == GF_UNDELETE_HIGHLIGHTED_ITEMS && !ShowDeletedOpt) 
{
#if ENABLETRACE
GSSiExitProg (852);
#endif
		return FALSE;
}
	
	if (!hHighlight)
	{   
		GSSiMsgBox (GetFocus(),"No items highlighted",0,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (852);
#endif
		return FALSE;
}
	}
	if (Function == GF_UNDELETE_HIGHLIGHTED_ITEMS || ShowDeletedOpt)
	{
		OldType = 92;
		NewType = 12;
		_fstrcpy (Type,"Un-delete");
	}
	else if (Function ==  GF_REMOVE_HIGHLIGHTED_ITEMS)
		NewType = 93;
	//GetBTHeader (hHighlight,&BTHead); 
	if (GetGlobalBVal2("[%DeletePrompt]",TRUE))
	{
		if (BT_NUM_IN_INDEX (hHighlight) > 1)
			sprintf (Mess,"%s these %ld records?",Type,BT_NUM_IN_INDEX (hHighlight)); 
		else
			sprintf (Mess,"%s this record?",Type);
		if (GSSiMsgBox(hWndMain,Mess,"Verify Delete", MB_YESNO,0) != IDYES)
{
#if ENABLETRACE
GSSiExitProg (852);
#endif
			return FALSE;
}
	}
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
    while (!BT_FIND (hHighlight,(LPSTR)&Ref,pos,BT_ANY,(LPSTR)&HighlightData))
    {
    	pos=BT_NEXT;
    	if (HighlightData.Show)
    	{   
	   		PickList[0]=HighlightData.PD;
			SetConfig (PickList[0].ConfigID);
    		SetViewport (PickList[0].ViewID);
			PickList[0]=HighlightData.PD; 
			if (PickList[0].Blocked)
				RemoveDummyDelete (0);
			else if (OldType == 12 && NewType == 92)
				DeletePickedItem2 (0);  
			else
				DeletePickedItem (0,OldType,NewType);
		}
	} 
	GSSiSetCursor (hcurSave);
	ClearHighlightList(FALSE);
{
#if ENABLETRACE
GSSiExitProg (852);
#endif
	return TRUE;      		                 
}
#if ENABLETRACE
}
#endif
} 

 

BOOL CreateSpecial (void)
#if ENABLETRACE
{GSSiEnterProg (854);
#endif
{	BTVARDESC	BTVar[1];
    char	SpecialFile[144];
    
	if (hSpecial)
{
#if ENABLETRACE
GSSiExitProg (854);
#endif
		return FALSE;
}
	GSSiGetTempFileName (0,"gms",0,(LPSTR)SpecialFile);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (SpecialFile, sizeof(SPECIALDATA), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hSpecial = BT_OPEN (SpecialFile, 0, BT_WRITE, 0);
{
#if ENABLETRACE
GSSiExitProg (854);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void ClearSpecial (void)
#if ENABLETRACE
{GSSiEnterProg (855);
#endif
{
	if (!hSpecial)
{
#if ENABLETRACE
GSSiExitProg (855);
#endif
		return;
}
	BT_CLOSEANDDELETE (&hSpecial);
{
#if ENABLETRACE
GSSiExitProg (855);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL AddSpecial (long Ref,int Type,COLORREF Color,int Width)
#if ENABLETRACE
{GSSiEnterProg (856);
#endif
{
	SPECIALDATA	SpecialData;
	
	if (!hSpecial)
{
#if ENABLETRACE
GSSiExitProg (856);
#endif
		return FALSE;
}
	SpecialData.Type = Type;
	SpecialData.Color = Color;
	SpecialData.Width = Width;
	BT_PUT (hSpecial,(LPSTR)&Ref,(LPSTR)&SpecialData);
{
#if ENABLETRACE
GSSiExitProg (856);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void SetPickCursor (BOOL InPick)
#if ENABLETRACE
{GSSiEnterProg (857);
#endif
{   
	HCURSOR	hCursor;
	
	if (InPick) 
		SetCurs ((HCURSOR)1,FALSE);  
	else
		SetCurs (OldCursor,FALSE);  
{
#if ENABLETRACE
GSSiExitProg (857);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

HCURSOR VPCursor (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (858);
#endif
{   
	POINT	CursorPoint;
	LPVIEWPORT	SaveView; 
	short	iview, SaveCFG;  
	HCURSOR	hcursor;
	
	if (idTimer)
{
#if ENABLETRACE
GSSiExitProg (858);
#endif
		return (hDrawingCursor);
}
	if (hCursor == hWaitCursor || CursorIsGlobal)
{
#if ENABLETRACE
GSSiExitProg (858);
#endif
		return (hCursor);  
}
		
	GetCursorPos (&CursorPoint); 
	ScreenToClient (hWnd,&CursorPoint);
    SaveView = CurView;
    SaveCFG = CurrentConfig;
	SetConfigFromCursor (CursorPoint);
	iview = *pNumViewports; 
	hcursor = 0;
    while (iview--)
    {   SetCurView ( pViewportsD[iview]); 
        if (CurViewActive() && !CurView->DisplayInParent)
        	if (PtInRect (&CurView->Rect,CursorPoint))
        {   
        	hcursor = CurView->hCursor;    
        	if (hcursor == (HCURSOR)1)
        		hcursor = CurPickCursor; 
        	else if (hcursor == (HCURSOR)2)
        		hcursor = hDigCursor; 
        	else if (!hcursor)
        		hcursor = DefaultCursor;
		    goto Exit;
        }
    } 
Exit: 
	SetConfig (SaveCFG);
    SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (858);
#endif
    return hcursor;
}
#if ENABLETRACE
}
#endif
} 

void SetCurs (HCURSOR hCurs,BOOL Global)
#if ENABLETRACE
{GSSiEnterProg (859);
#endif
{   
	LPCMDSTRING    pCmdStr;
	short	ii;
	LPVIEWPORT	SaveVP = CurView;
    
    if (!CurView)
    {
    	GSSiSetCursor (hCurs);
		return;
	}
	if (CurView->DisplayInParent && CurView->Parent)
		SetViewport (SaveVP->Parent);
    if (hCurs == CurPickCursor)
    	ii=1;
	CursorIsGlobal = Global; 
	if (!Global && CurView->FunStackHandle)
	{
    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
		pCmdStr->hCursor = hCurs;
		GlobalUnlock (CurView->FunStackHandle);
    }
	if (!hCurs)
	{
		GSSiSetCursor (DefaultCursor);  
		if (!CursorIsGlobal)
			CurView->hCursor = 0;
		hCursor = 0;
		CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (859);
#endif
		return;     
}
	}
	if (CursorIsGlobal)     
		hCursor = hCurs;
	else 
	{   
//		CurView->LastCursor = CurView->hCursor;
		CurView->hCursor = hCurs; 
		if (hCurs == (HCURSOR)1)
			hCurs = CurPickCursor;  
		if (hCurs == (HCURSOR)2)
			hCurs = hDigCursor;  
	}             
	GSSiSetCursor (hCurs);
	CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (859);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ResetLastCursor (void)
#if ENABLETRACE
{GSSiEnterProg (860);
#endif
{          
	SetCurs (CurView->LastCursor,FALSE);
{
#if ENABLETRACE
GSSiExitProg (860);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL SetNewRefno (LPSTR EditFile,long Refno)
#if ENABLETRACE
{GSSiEnterProg (861);
#endif
{
    HFILE	Fid;
	char	str[MAX_PATH]; 
	char	NewRefnoPath[MAX_PATH]; 
	OFSTRUCTGM	OFStruct;
	LPSTR	lpBS;
	
	_fstrcpy (str,EditFile);
	ExpandText (str);
	_fullpath (NewRefnoPath,str,MAX_PATH);
	lpBS = _fstrrchr (NewRefnoPath,'\\');
	*lpBS = 0;
	_fstrcat (NewRefnoPath,"\\nextref.txt");
	GSSiRemove (NewRefnoPath);  
	SetGlobalValueLong ("%INITIALREFNO",Refno); 
	Refno = GetNewRefno(EditFile,0,0,0,0);
{
#if ENABLETRACE
GSSiExitProg (861);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
} 

BOOL IntersectRefs(LPSTR RefOrTAG1,LPSTR RefOrTAG2,LPSTR NearPointC,LPSTR OutVarName)
{
	BOOL rtn=FALSE;
	HANDLE	hPoly[2];
	HPDPOINT	pPolyPoints[2];
	long	npnts[2];
 	double	ExtendDist=10000; 
	double	D1,D2, Extd[2];
	double	AZ; 
	short	pos,i, type[2]; 
	DPOINT	PickPointBase, IntPoint;
	BOOL	Err;

	PickPointBase = atopt (NearPointC,&Err);

	if (*RefOrTAG1 == '(')
	{
		RefOrTAG1++;
		*LastChr (RefOrTAG1) = 0;
	    npnts[0] = GetPointsFromList (RefOrTAG1,&hPoly[0]);
		type[0] = 2;
	}
	else if (PickByTagOrRefno(RefOrTAG1,-1))
	{
	    type[0] = PickList[0].Type;
		if (!GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts[0],&hPoly[0],TRUE))
			return FALSE;
    }
	else
		return FALSE;
	if (*RefOrTAG2 == '(')
	{
		RefOrTAG2++;
		*LastChr (RefOrTAG2) = 0;
	    npnts[1] = GetPointsFromList (RefOrTAG2,&hPoly[1]);
		type[1] = 2;
	}
	else if (PickByTagOrRefno(RefOrTAG2,-1))
	{
	    type[1] = PickList[1].Type;
		if (!GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts[1],&hPoly[1],TRUE))
		{
			GSSiGlobFree (&hPoly[0]);
			return FALSE;
		}
    }
	else
	{
		GSSiGlobFree (&hPoly[0]);
		return FALSE;
	}
	pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]);
	pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);  
	Extd[0] = Extd[1] = 0;
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
	if (rtn)
		CurrentPoint = IntPoint;
	GSSiGlobUlFree (&hPoly[0]);
	GSSiGlobUlFree (&hPoly[1]); 

	return rtn;
}


