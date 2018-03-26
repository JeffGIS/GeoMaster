
//#include "perfmon.h" 
#include "graphint.h"
#include <commctrl.h> 
#include "toolbar.h" 
#include "dibapi.h"
#include "status.h"     // for StatusLine & StatusLineReady 

#include "gmextern.h"

#define TBT_STANDARDMENU	0
#define TBT_STANDARDMENU_DOCKED	3
#define TBT_ZOOMPAN	1
#define TBT_VISMENU	2

#define DMS_NOTDISPLAYED	0
#define DMS_NORMAL	3
#define DMS_DIMMED	1
#define DMS_WANTDIMMED	2

int		nToolbars = 0;
static	int		ToolbarIDCur, ToolbarX, MaxToolbarX;
static	int		nToolbarRow, rowY, nControlsInRow, MaxControlsInRow, MaxDesiredToolbarWidth;
static	BOOL	FirstToolbarPass;

typedef struct	{UINT	CntlID;
				 WNDPROC	wndProc;
				 HBITMAP	hBM;
				 int	FileLoc;
				 int	w,h; // h and w of entire toolbar with this number of controls per row
				 char	Label[80];} TOOBAR_CONTROL_INFO;
typedef TOOBAR_CONTROL_INFO	*LPTOOBAR_CONTROL_INFO;

typedef struct { HDIB32 hDib32;
				 BOOL haveTracking;
				} TEMPWINDDATA;
typedef TEMPWINDDATA *LPTEMPWINDDATA;

static	HWND	currentToolbarWnd = 0;
static	int		nextToolbarId = 1;
static	int		BestW, BestH, TotBestWH;
static	int		BestRowConfigs[MAX_TOOLBARS];
static	int		BestTBConfigs[MAX_TOOLBARS];
static	int		TotBestConfigs[MAX_TOOLBARS];
static	RECT	BestConfigRect[MAX_TOOLBARS];
static	RECT	TotBestConfigRect[MAX_TOOLBARS];
static	HWND	hWndToolbar;
static	HWND	hwndPanZoomRot=0;
static	int		iPerfmonView=0;
static	char	ToolbarPath[MAX_TOOLBARS][MAX_PATH];
HWND	ToolbarWindow[MAX_TOOLBARS];
static	int		ToolbarId[MAX_TOOLBARS];
static	BOOL	ToolbarFloating[MAX_TOOLBARS];
static	int		ToolbarType[MAX_TOOLBARS];
static	int		ToolbarFloatNumPerRow[MAX_TOOLBARS];
static	HANDLE	ToolbarHandle[MAX_TOOLBARS];
static	HBITMAP	ToolbarImage[MAX_TOOLBARS]={0};
static	HWND	ToolbarTTWindow[MAX_TOOLBARS];
//static	HHOOK	ToolbarHook=0;//[MAX_TOOLBARS]={0};
static	HHOOK	ToolbarMoveHook=0;//[MAX_TOOLBARS]={0};
static	TOOLINFO toolItem[MAX_TOOLBARS];
static	BOOL	DisplayMenuStatus[MAX_TOOLBARS]={0};
static	int		nToolbarControls[MAX_TOOLBARS];
static	int		nToolbarConfigs[MAX_TOOLBARS];
static	int		ToolbarHeight[MAX_TOOLBARS];
static	int		MaxButtonBitmapWidth[MAX_TOOLBARS];
static	int		MaxButtonBitmapHeight[MAX_TOOLBARS]; 
static	int		FirstButtonBitmapHeight[MAX_TOOLBARS];
static	BOOL	CreateConfigs[MAX_TOOLBARS];
static	int		ToolbarCurrentConfig[MAX_TOOLBARS];
static	SIZE	ToolbarConfigs[MAX_TOOLBARS][MAX_TOOLBAR_CONFIGS];
static	int		ToolbarConfigNumPerRow[MAX_TOOLBARS][MAX_TOOLBAR_CONFIGS];
static	int		nToolbarRows[MAX_TOOLBARS];
static	RECT	ToolbarRect[MAX_TOOLBARS];
static	POINT	ToolbarBeginMove[MAX_TOOLBARS];
//static	POINTS	ToolbarConfigs[MAX_TOOLBARS][MAX_TOOLBAR_CONTROLS];
static	POINTS	ToolbarPos[MAX_TOOLBARS];
static	short	ToolbarsLeft[MAX_TOOLBARS]={0};
static	short	nToolbarsLeft=0;
static	short	ToolbarsRight[MAX_TOOLBARS]={0};
static	short	nToolbarsRight=0;
static	short	ToolbarsTop[MAX_TOOLBARS]={0};
static	short	nToolbarsTop=0;
static	short	ToolbarsBottom[MAX_TOOLBARS]={0};
static	DPOINT	ToolbarDPoint[MAX_TOOLBARS];
static	int		ToolbarVPID[MAX_TOOLBARS];
static	double	ToolbarReZoomScale[MAX_TOOLBARS];
static	int		HaveTrackMouseEvent[MAX_TOOLBARS];
static	int		ToolbarPointerType[MAX_TOOLBARS];
static	short	nToolbarsBottom=0;
static	char	ToolbarHoverCmd[MAX_TOOLBARS][MAX_TOOLBAR_HOVERCMD + 1] = { 0 };
static	UINT	Buttons[MAX_TOOLBAR_BUTTONS]={IDC_BUTTON1,IDC_BUTTON2,IDC_BUTTON3,IDC_BUTTON4,IDC_BUTTON5,
							  IDC_BUTTON11,IDC_BUTTON12,IDC_BUTTON13,IDC_BUTTON14,IDC_BUTTON15,IDC_BUTTON16,IDC_BUTTON17,IDC_BUTTON18,IDC_BUTTON19,IDC_BUTTON20,
							  IDC_BUTTON21,IDC_BUTTON22,IDC_BUTTON23,IDC_BUTTON24,IDC_BUTTON25,IDC_BUTTON26,IDC_BUTTON27,IDC_BUTTON28,IDC_BUTTON29,IDC_BUTTON30,
							  IDC_BUTTON31,IDC_BUTTON32,IDC_BUTTON33,IDC_BUTTON34,IDC_BUTTON35,IDC_BUTTON36,IDC_BUTTON37,IDC_BUTTON38,IDC_BUTTON39,IDC_BUTTON40,
							  IDC_BUTTON41,IDC_BUTTON42,IDC_BUTTON43,IDC_BUTTON44,IDC_BUTTON45,IDC_BUTTON46,IDC_BUTTON47,IDC_BUTTON48,IDC_BUTTON49,IDC_BUTTON50,
							  IDC_BUTTON51,IDC_BUTTON52,IDC_BUTTON53,IDC_BUTTON54,IDC_BUTTON55,IDC_BUTTON56,IDC_BUTTON57,IDC_BUTTON58,IDC_BUTTON59,IDC_BUTTON60,
							  IDC_BUTTON61,IDC_BUTTON62,IDC_BUTTON63,IDC_BUTTON64,IDC_BUTTON65,IDC_BUTTON66,IDC_BUTTON67,IDC_BUTTON68,IDC_BUTTON69};
static	UINT	RadioButtons[MAX_TOOLBAR_RADIO_BUTTONS]={IDC_RADIO1,IDC_RADIO2,IDC_RADIO15,IDC_RADIO16,IDC_RADIO18};
static	UINT	TextControls[MAX_TOOLBAR_TEXT_INPUT]={IDC_EDIT1,IDC_EDIT9,IDC_EDIT10,IDC_EDIT11,IDC_EDIT12};
static	UINT	ComboControls[MAX_TOOLBAR_DROPDOWN]={IDC_COMBO1,IDC_COMBO2,IDC_COMBO3,IDC_COMBO4,IDC_COMBO5};
static	TBBUTTON tbButtons[] = { 
   { 0, 0,                    TBSTATE_ENABLED,   TBSTYLE_SEP,        0 }, 
   { 0, IDM_CFG1,	        TBSTATE_ENABLED,   TBSTYLE_CHECKGROUP, 0 }, 
   { 1, IDM_CFG2,	      TBSTATE_ENABLED,   TBSTYLE_CHECKGROUP, 0 }, 
   { 2, IDM_CFG3,          TBSTATE_ENABLED,   TBSTYLE_CHECKGROUP, 0 }, 
   { 3, IDM_COMMAND_INPUT,       TBSTATE_ENABLED,   TBSTYLE_CHECKGROUP, 0 }, 
   { 0, 0,                    TBSTATE_ENABLED,   TBSTYLE_SEP,        0 }, 
   { 0, 0,                    TBSTATE_ENABLED,   TBSTYLE_SEP,        0 }, 
   { 4, IDM_COMBO_FILE,       TBSTATE_ENABLED,   TBSTYLE_BUTTON,     0 } 
} ; 
static	double	Rotation = 0;
static	int		ToolbarWidthLeft = 0;
static	int		ToolbarWidthRight = 0;
static	int		ToolbarWidthTop = 0;
static	int		ToolbarWidthBottom = 0;
static	HRGN	hRgnJoyStick;
static	RECT	JoyStickRect;
static	double	JoyStickSpeed = 0.5;
static	int		radius;
static	POINT	PZRMidPoint;
static	LPVIEWPORT PZR_VP;
static	char	lastPrompt[256];
static	HWND	g_hwndTrackingTT;

static	HWND	ImageWnd=0;
static	char	PictViewerDir[MAX_PATH];
static	UINT	PictBtnBM[2][3]={IDB_LEFT_ARROW,IDB_LEFT_ARROW2,IDB_LEFT_ARROW1,IDB_RIGHT_ARROW,IDB_RIGHT_ARROW2,IDB_RIGHT_ARROW1};

static	HWND	hWndPZR=0;
static	HANDLE	hSavePZRScreen=0;
static	HANDLE	hSavePZRScreen2=0;
static	HANDLE	hSavePZRScreenNoIcons=0;
static	HDC		hDCPZMoveImage=0;
static	HBITMAP	hOldPZMoveBM=0;
static	DWORD	RastOpts[15]={SRCCOPY,SRCAND,SRCPAINT,SRCINVERT,SRCERASE,NOTSRCCOPY,NOTSRCERASE,MERGECOPY,
	           			  MERGEPAINT,PATCOPY,PATPAINT,PATINVERT,DSTINVERT,BLACKNESS,WHITENESS};
static	RECT	winzoomRect, listRect, cancelRect, findRect;
static	int		rop=0; 
#define TB_ENTRIES sizeof(tbButtons)/sizeof(tbButtons[0]) 
POINT	ToolBarStartPoint;
static	HWND	g_hwndDlgx;
static	HDWP	hWDP;
static	BOOL	ignoreMM=FALSE;

#define MAXTBROWS	8
#define MAXINTBROW	8
static	short	TBInRow[MAXTBROWS][MAXINTBROW];
static	short	nInRow[MAXTBROWS];

static	BYTE	maskBits[131][131]={0};



extern	int	AlphaBlendFactor;


BOOL OnWMNotify(HWND hWndDlg,LPARAM lParam);
BOOL CALLBACK EnumChildProcTT(HWND hwndCtrl, LPARAM lParam);
void DisplayPanZoomRot (HWND hWnd,HDC hDC,LPRECT pRect);
HANDLE FillTBRows (int nTBTot,LPSHORT TB,int nInIndexArray,LPSHORT IndexArray,int nRows,int iRow,int lastRow,LPHANDLE phOut);
void DrawBtnFocusRect(HWND BtnWnd);
LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND CursorInVisMenuWnd (POINT pt)
{
	RECT	rect;
	UINT	imen;
	HWND	rtn=0;

	if (hWndPZR)
	{
		GetWindowRect (hWndPZR,&rect);
		if (PtInRect (&rect,pt))
		{
			rtn = hWndPZR;
			goto Exit;
		}
	}
	for (imen = 0;imen < nToolbars;imen++)
	{
		HANDLE hWMH = (HANDLE)GetWindowLong (ToolbarWindow[imen],GWL_USERDATA);

		if (hWMH)
		{
			LPWINDOWMENUHEADER pWMH = GlobalLock (hWMH);

			GetWindowRect (pWMH->hWndDisplay,&rect);
			if (PtInRect (&rect,pt))
				rtn = pWMH->hWndDisplay;
			GlobalUnlock (hWMH);
		}
	}
Exit:
	return rtn;
}

void DisplayAllToolbarControls (int ToolbarID)
{
	int	i;
	LPTOOBAR_CONTROL_INFO	pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock (ToolbarHandle[ToolbarID]);

	for (i=0;i<nToolbarControls[ToolbarID];i++,pTBInfo++)
	{
		HWND hWnd = GetDlgItem (ToolbarWindow[ToolbarID],pTBInfo->CntlID);
		RECT	rect;

		GetWindowRect (hWnd,&rect);
		ScreenRectToClientRect (ToolbarWindow[ToolbarID],&rect);
		MoveWindow (hWnd,rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect),TRUE);
	}
	GlobalUnlock (ToolbarHandle[ToolbarID]);

	return;
}

void SaveToolbarImage (int ToolbarID)
{
	HDC	hDC;
	RECT	rect;
	int	w,h;

	if (ToolbarType[ToolbarID] == TBT_ZOOMPAN || ToolbarType[ToolbarID] == TBT_VISMENU)
		return;
	hDC = GetWindowDC (ToolbarWindow[ToolbarID]);
	GetWindowRect (ToolbarWindow[ToolbarID],&rect);
	w = RECTWIDTH(&rect);
	h = RECTHEIGHT(&rect);
	rect.left = rect.top = 0;
	rect.right = w;
	rect.bottom = h;
	GSSiDeleteObject (&ToolbarImage[ToolbarID]);
	if (ToolbarFloating[ToolbarID])
	{
		DisplayAllToolbarControls (ToolbarID);
		ToolbarImage[ToolbarID] = SaveScreen (hDC, rect);
	}
	ReleaseDC (ToolbarWindow[ToolbarID],hDC);
	DisplayMenuStatus[ToolbarID] = DMS_NORMAL;
	return;
}

void DisplayDimmedMenu (int ToolbarID)
{
	int	w,h;

	if (ToolbarID >= 0 && ToolbarImage[ToolbarID])
	{
		HDC	hDC;
		BLENDFUNCTION bf;
		RECT	MenuRectInMain, rect;
		int		xoff,yoff, widthMenu, heightMenu;

		GetWindowRect (ToolbarWindow[ToolbarID],&rect);
		MenuRectInMain = rect;
		ScreenRectToClientRect (hWndMain,&MenuRectInMain);
		xoff = MenuRectInMain.left;
		yoff = MenuRectInMain.top;
		widthMenu=RECTWIDTH (&MenuRectInMain);
		heightMenu=RECTHEIGHT (&MenuRectInMain);
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = 0;
		bf.SourceConstantAlpha = AlphaBlendFactor;
		hDC = GetWindowDC (ToolbarWindow[ToolbarID]);
		w = RECTWIDTH(&rect);
		h = RECTHEIGHT(&rect);
		rect.left = rect.top = 0;
		rect.right = w;
		rect.bottom = h;
		RestoreScreen (hDC,ToolbarImage[ToolbarID], rect);
	  	SelectClipRgn ( hDC,0);
		ii=AlphaBlend(hDC,0,0,widthMenu,heightMenu, 
					hDCScreenBuffer,xoff,yoff,widthMenu,heightMenu,bf);
		ReleaseDC (ToolbarWindow[ToolbarID],hDC);
		DisplayMenuStatus[ToolbarID] = DMS_DIMMED;
	}
	return;
}

HWND CreateTrackingToolTip(int toolID, HWND hDlg, LPSTR pText)
{
    // Create a tooltip.
    HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, 
                                 WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
                                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                 hDlg, NULL, hInst,NULL);

    if (!hwndTT)
    {
      return NULL;
    }

    // Set up the tool information. In this case, the "tool" is the entire parent window.
    
    toolItem[toolID].cbSize   = sizeof(TOOLINFO);
    toolItem[toolID].uFlags   = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
    toolItem[toolID].hwnd     = hDlg;
    toolItem[toolID].hinst    = hInst;
    toolItem[toolID].lpszText = LPSTR_TEXTCALLBACK;
    toolItem[toolID].uId      = (UINT_PTR)hDlg;
    
    GetClientRect (hDlg, &toolItem[toolID].rect);

    // Associate the tooltip with the tool window.
    
    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &toolItem[toolID]);	
    
    return hwndTT;
}

void PZEndMove (int ToolbarID)
{
	if (ToolbarType[ToolbarID] != TBT_ZOOMPAN)
		return;
	if (hDCPZMoveImage)
	{
		HBITMAP hBM = SelectObject (hDCPZMoveImage,hOldPZMoveBM);

		GSSiDeleteObject (&hBM);
		hOldPZMoveBM = 0;
		DeleteDC(hDCPZMoveImage);
		hDCPZMoveImage = 0;
	}
	return;
}
void PZStartMove (int ToolbarID)
{
	HWND	hWndDT = GetDesktopWindow();
	HDC		hDC;
	RECT	rect;
	HBITMAP hbmPrev, hNewBM;
    short   i,ii; 
    int		w,h;

	if (ToolbarType[ToolbarID] != TBT_ZOOMPAN)
		return;
	PZEndMove (ToolbarID);
	ShowWindow (ToolbarWindow[ToolbarID],FALSE);
	GetClientRect (hWndDT,&rect);
	w=RECTWIDTH(&rect);
	h=RECTHEIGHT(&rect);
    
	hDC = GetWindowDC (hWndDT);
    SaveDC (hDC);
	SetGraphicsMode(hDC, GM_COMPATIBLE);
    SetWindowOrgEx  ( hDC, 0, 0,0 );
    SetViewportOrgEx( hDC, 0, 0,0 );    
    SetMapMode    ( hDC, MM_TEXT );
  	SelectClipRgn ( hDC,0);
    hDCPZMoveImage = CreateCompatibleDC(hDC);
    hNewBM = CreateCompatibleBitmap(hDC,w,h);
	hOldPZMoveBM = SelectObject (hDCPZMoveImage,hNewBM);
	BitBlt(hDCPZMoveImage, 0, 0, w,h,
           hDC, 0,0, SRCCOPY);
    RestoreDC (hDC,-1); 
	ReleaseDC (hWndDT,hDC);
	ShowWindow (ToolbarWindow[ToolbarID],TRUE);

	return;
}
           
void SetPZMaskBytes (HWND hWnd,HDC hDC2)
{
	BITMAP bm;
	RGBQUAD	*pBits32,*pB32;
    RGBTRIPLE	*pBits24,*pB24;             
	GSSiCOLOR16 *pBits16,*pB16;  
	int	r,c;
	HDC hDC=GetDC (hWnd);
	RECT	rect;

	GetClientRect (hWnd,&rect);
	{
		int	w=RECTWIDTH(&rect),h=RECTHEIGHT(&rect);
		HDC		hDCMem=CreateCompatibleDC (hDC);
		HBITMAP hBMMem = CreateCompatibleBitmap (hDC,w,h);
		HBITMAP hBM = SelectObject (hDCMem,hBMMem);

		BitBlt (hDCMem,0,0,w,h,hDC2,0,0,SRCCOPY);
		SelectObject (hDCMem,hBM);
		GetObject(hBMMem, sizeof(bm), (LPSTR)&bm);
		switch (bm.bmBitsPixel)
		{
		case 32:
			pBits32 = pB32 = malloc (bm.bmWidthBytes*bm.bmHeight);
			GetBitmapBits(hBMMem,bm.bmWidthBytes * bm.bmHeight,pBits32);
			for (r=bm.bmHeight-1;r>=0;r--)
				for (c=0;c<bm.bmWidth;c++,pB32++)
					if (pB32->rgbBlue < 200 && pB32->rgbGreen > 220 && pB32->rgbRed < 200)
						maskBits[r][c] = 255;
					else
						maskBits[r][c] = 0;
			free (pBits32);
			break;
		case 24:
			pBits24 = pB24 = malloc (bm.bmWidthBytes*bm.bmHeight);
			GetBitmapBits(hBMMem,bm.bmWidthBytes * bm.bmHeight,pBits24);
			for (r=bm.bmHeight-1;r>=0;r--)
				for (c=0;c<bm.bmWidth;c++,pB24++)
					if (pB24->rgbtBlue < 200 && pB24->rgbtGreen > 220 && pB24->rgbtRed < 200)
						maskBits[r][c] = 255;
					else
						maskBits[r][c] = 0;
			free (pBits24);
			break;
		case 16:
			pBits16 = pB16 = malloc (bm.bmWidthBytes*bm.bmHeight);
			GetBitmapBits(hBMMem,bm.bmWidthBytes * bm.bmHeight,pBits16);
			for (r=bm.bmHeight-1;r>=0;r--)
				for (c=0;c<bm.bmWidth;c++,pB16++)
					if (pB16->b < 29 && pB16->g > 30 && pB16->r < 29)
						maskBits[r][c] = 255;
					else
						maskBits[r][c] = 0;
			free (pBits16);
			break;
		}
		SelectObject (hDC,hBM);
		DeleteObject (hBMMem);
		DeleteDC (hDCMem);
	}
	return;
}

void SetPZRControlRotation (double Rot)
{
	Rotation = Rot;
	if (hWndPZR)
		DisplayPanZoomRot (hWndPZR,0,0);

	return;
}

void ClearToolbarTrackEvents (HWND hWnd)
{
	UINT i;

	for (i=0;i<nToolbars;i++)
	{
		if (ToolbarFloating[i] && BufferedScreen && hDCScreenBuffer)
		{
			switch (DisplayMenuStatus[i])
			{
			case DMS_NOTDISPLAYED:
				SaveToolbarImage (i);
			case DMS_NORMAL:
				PostMessage (ToolbarWindow[i],GSSi_DimMenu,0,0);
			}
		}
	}
	if (hWnd)
		SetFocus (hWnd);
	return;
}

void DisplayAllToolbars (int Opt)
{
	int	i;
	static	int	ii=0;

	if ((int)hFullWindowBitMap == -1)
		return;
	if (ii++ == 100)
		ii=0;
	switch (Opt)
	{
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
		break;
	default: 
		return;
	}
	IgnoreActivate = TRUE;
	IgnoreWPC = TRUE;
//	DisplayPZRotImage (CheckCursorPos);
//	DisplayFloatMenu (CheckCursorPos);
	if (Opt)
		for (i=0;i<nToolbars;i++)
		{
			RECT	rect=ToolbarRect[i];

			ShowWindow (ToolbarWindow[i],SW_SHOW);
			if (DisplayMenuStatus[i] == DMS_DIMMED)
				DisplayDimmedMenu (i);
			else
			{
				GSSiDeleteObject (&ToolbarImage[i]);
				InvalidateRect (ToolbarWindow[i],0,TRUE);
			}
	//		ShowWindow (ToolbarWindow[i],SW_SHOW);
			SetWindowPos (ToolbarWindow[i],HWND_NOTOPMOST,rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect),SWP_DRAWFRAME|SWP_SHOWWINDOW);//|SWP_NOSIZE|SWP_NOMOVE);
		}
	else
		for (i=0;i<nToolbars;i++)
		{
			ShowWindow (ToolbarWindow[i],SW_HIDE);
		}
	IgnoreActivate = FALSE;
	IgnoreWPC = FALSE;
	return;
}

BOOL FillNextTBRow (int nTB,LPSHORT TB,int nRows,int iRow,int lastRow,LPHANDLE phOut)
{
	int		nNextTB=0;
	SHORT	nextTB[MAX_TOOLBARS];
	int		n,r,inr;

	for (n=0;n<nTB;n++)
	{
		for (r=0;r<iRow;r++)
		{
			for (inr=0;inr<nInRow[r];inr++)
				if (n == TBInRow[r][inr])
					goto next;
		}
		nextTB[nNextTB++] = n;
next:;
	}
	FillTBRows (nTB,TB,nNextTB,nextTB,nRows,iRow,lastRow,phOut);
	return TRUE;
}

void FillTBRowOutput (int nTB,LPSHORT TB,LPHANDLE phOut,int nRows)
{
	int	i,j;
	LPSTR	pOut=GlobalLock (*phOut);

	for (i=0;i<nRows;i++)
	{
		sprintf (strchr(pOut,0),"%2.1i",nInRow[i]);
		for (j=0;j<nInRow[i];j++)
		{
			sprintf (strchr(pOut,0),"%2.1i",TB[TBInRow[i][j]]);
		}
	}
	strcat (pOut,"$");
	GlobalUnlock (*phOut);
	return;
}

HANDLE FillTBRows (int nTB,LPSHORT TB,int nInIndexArray,LPSHORT IndexArray,int nRows,int iRow,int lastRow,LPHANDLE phOut)
{
	int i,j,k;
	int	maxPerRow = nInIndexArray - nRows + 1;
	int	lastMaxPerRow = max (maxPerRow / 2,nInIndexArray - (lastRow - iRow));
	char	str[256];
	LPSTR	pOut;
	int	rowIndex[MAXINTBROW];

	if (!*phOut)
		*phOut=GSSiGlobAlloc (1736,GHND,USHRT_MAX);
	if (nRows == 1)
	{
		for (j=0;j < nInIndexArray;j++)
			TBInRow[iRow][j] = IndexArray[j];
		nInRow[iRow] = nInIndexArray;
		FillTBRowOutput (nTB,TB,phOut,lastRow+1);
	}
	else if (nInIndexArray == nRows)
	{
		for (j=0;j < nInIndexArray;j++)
		{
			nInRow[iRow+j] = 1;
			TBInRow[iRow+j][0] = IndexArray[j];
		}
		FillTBRowOutput (nTB,TB,phOut,lastRow+1);
	}
	else while (maxPerRow && maxPerRow >= lastMaxPerRow)
	{
		nInRow[iRow] = maxPerRow;
		for (i=0;i < maxPerRow;i++)
			//TBInRow[iRow][i] = IndexArray[i];
			rowIndex[i] = i;


		i = maxPerRow-1;
		while (i > -1)
		{
			int iEnd;

			for (j=0;j < maxPerRow;j++)
				TBInRow[iRow][j] = IndexArray[rowIndex[j]];

			if (iRow == lastRow)
				FillTBRowOutput (nTB,TB,phOut,lastRow+1);
			else
				FillNextTBRow (nTB,TB,nRows-1,iRow+1,lastRow,phOut);
			
			i = maxPerRow-1;

Next:
			iEnd = nInIndexArray - maxPerRow  + i - iRow;
			if (rowIndex[i] < iEnd)
			{
				rowIndex[i]++;
				for (j=i+1,k=1;j<maxPerRow;j++)
					rowIndex[j] = rowIndex[i] + k++;
			}
			else
			{
				i--;
				if (i > -1)
					goto Next;
			}
		}
		
		maxPerRow--;
	}
	return *phOut;
}


HANDLE GetTBRows (int nTB,LPSHORT pTB)
{
	HANDLE	hAllRows=0;
	int	i, lnTot=0;
	LPSTR	pAll;
	short	TBIndex[MAX_TOOLBARS];

	for (i=0;i<nTB;i++)
		TBIndex[i] = i;

	for (i=0;i<nTB;i++)
	{
		HANDLE hTBRowsIn = 0;
		HANDLE hTBRows = FillTBRows (nTB,pTB,nTB,TBIndex,i+1,0,i,&hTBRowsIn);
		LPSTR  pTBRows = GlobalLock (hTBRows);
		int	   ln = strlen (pTBRows);

		GlobalUnlock (hTBRows);
		lnTot += ln;
		if (!i)
			hAllRows = hTBRows;
		else
		{
			hAllRows = GSSiGlobalReAlloc (1738,hAllRows,lnTot+1,GMEM_MOVEABLE);
			pAll = GlobalLock (hAllRows);
			pTBRows = GlobalLock (hTBRows);
			strcat (pAll,pTBRows);
			GlobalUnlock (hAllRows);
			GSSiGlobUlFree (&hTBRows);
		}
	}
	return hAllRows;
}

int TestFillTBRows (int nr,int ntb)
{
	short	TB[10]={0,1,2,3,4,5,6,7,8,9};
	int nTB=ntb;
	int	nRows = nr;
	HANDLE	hOut=0;

	//return 1;

	GSSiRemove ("c:\\temp\\testTB.txt");
	FillTBRows (nTB,TB,nTB,TB,nRows,0,nRows-1,&hOut);
	return 1;
}

/*void FindBestConfigOfToolbars (int WhichSide)
{
	RECT	MainRect;
	int		i,j,icfg,jcfg;

	GetClientRect (hWndMain,&MainRect);
	if (!IsRectEmpty (&PromptRect))
		MainRect.bottom = PromptRect.top - 1;

	switch (WhichSide)
	{
	case 1: //left
		{
			int minY=0, maxY = MainRect.bottom;
			int	MinLeftWidth = INT_MAX;
			POINTS	rowSize;
			int	nRows = nToolbarsLeft;

			while (nRows--)
			{
				int	maxPerRow = nToolbarsLeft - nRows;

				for (irow = 0;irow < nRows;irow++)
				{


			GetToolbarRowConfig (nInRow,ToolbarsInRow,&rowSize);

			for (i=0;i<nToolbarsLeft;i++)
			{

				for (icfg =0;icfg<nToolbarConfigs[i];icfg++)
				{
					int LeftWidth = ToolbarConfigs[i][icfg].x;
					int	LeftHeight = ToolbarConfigs[i][icfg].y;

					for (j=i+1;j<nToolbarsLeft;j++)
					{
						for (jcfg=0;jcfg<nToolbarConfigs[j];jcfg++)
						{
							LeftWidth = max (LeftWidth,ToolbarConfigs[j][jcfg].x);
							LeftHeight += ToolbarConfigs[j][jcfg].y);
						}
					}
				}
			}
		}

		break;
	}
	return;
}*/

int GetToolbarDockingStatus (int ToolbarID,LPINT pWidth)
{
	int	i;

	for (i=0;i<nToolbarsLeft;i++)
	{
		if (ToolbarsLeft[i] == ToolbarID)
		{
			*pWidth = ToolbarWidthLeft;
			return 100 + i;
		}
	}
	for (i=0;i<nToolbarsRight;i++)
	{
		if (ToolbarsRight[i] == ToolbarID)
		{
			*pWidth = ToolbarWidthRight;
			return 200 + i;
		}
	}
	for (i=0;i<nToolbarsTop;i++)
	{
		if (ToolbarsTop[i] == ToolbarID)
		{
			*pWidth = ToolbarWidthTop;
			return 300 + i;
		}
	}
	for (i=0;i<nToolbarsBottom;i++)
	{
		if (ToolbarsBottom[i] == ToolbarID)
		{
			*pWidth = ToolbarWidthBottom;
			return 400 + i;
		}
	}

	return 0;
}

void SetToolbarDockingStatus (int ToolbarID,int Status,int Width)
{
	int	i,j;

	if (Status >= 400)
	{
		i = Status - 400;
		for (j=nToolbarsBottom;j>i;j--)
			ToolbarsBottom[j] = ToolbarsBottom[j-1];
		ToolbarsBottom[i] = ToolbarID;
		ToolbarWidthBottom = Width;
		nToolbarsBottom++;
	}
	else if (Status >= 300)
	{
		i = Status - 300;
		for (j=nToolbarsTop;j>i;j--)
			ToolbarsTop[j] = ToolbarsTop[j-1];
		ToolbarsTop[i] = ToolbarID;
		ToolbarWidthTop = Width;
		nToolbarsTop++;
	}
	else if (Status >= 200)
	{
		i = Status - 200;
		for (j=nToolbarsRight;j>i;j--)
			ToolbarsRight[j] = ToolbarsRight[j-1];
		ToolbarsRight[i] = ToolbarID;
		ToolbarWidthRight = Width;
		nToolbarsRight++;
	}
	else if (Status >= 100)
	{
		i = Status - 100;
		for (j=nToolbarsLeft;j>i;j--)
			ToolbarsLeft[j] = ToolbarsLeft[j-1];
		ToolbarsLeft[i] = ToolbarID;
		ToolbarWidthLeft = Width;
		nToolbarsLeft++;
	}
	return;
}

BOOL RemoveDockedToolbar (int ToolbarID)
{
	int	i,j;

	for (i=0;i<nToolbarsLeft;i++)
	{
		if (ToolbarsLeft[i] == ToolbarID)
		{
			for (j=i;j<nToolbarsLeft;j++)
				ToolbarsLeft[j] = ToolbarsLeft[j+1];
			nToolbarsLeft--;
			return TRUE;
		}
	}
	for (i=0;i<nToolbarsRight;i++)
	{
		if (ToolbarsRight[i] == ToolbarID)
		{
			if (i < --nToolbarsRight)
				ToolbarsRight[i] = ToolbarsRight[nToolbarsRight];
			return TRUE;
		}
	}
	for (i=0;i<nToolbarsTop;i++)
	{
		if (ToolbarsTop[i] == ToolbarID)
		{
			if (i < --nToolbarsTop)
				ToolbarsTop[i] = ToolbarsTop[nToolbarsTop];
			return TRUE;
		}
	}
	for (i=0;i<nToolbarsBottom;i++)
	{
		if (ToolbarsBottom[i] == ToolbarID)
		{
			if (i < --nToolbarsBottom)
				ToolbarsBottom[i] = ToolbarsBottom[nToolbarsBottom];
			return TRUE;
		}
	}

	return FALSE;
}
void AdjustDockedToolbarsForRemovedToolbar (int ToolbarID)
{
	int	i;

	for (i=0;i<nToolbarsLeft;i++)
	{
		if (ToolbarsLeft[i] > ToolbarID)
			ToolbarsLeft[i]--;
	}
	for (i=0;i<nToolbarsRight;i++)
	{
		if (ToolbarsRight[i] > ToolbarID)
			ToolbarsRight[i]--;
	}
	for (i=0;i<nToolbarsTop;i++)
	{
		if (ToolbarsTop[i] > ToolbarID)
			ToolbarsTop[i]--;
	}
	for (i=0;i<nToolbarsBottom;i++)
	{
		if (ToolbarsBottom[i] > ToolbarID)
			ToolbarsBottom[i]--;
	}

	return;
}
BOOL ToolbarIsDocked (int ToolbarID)
{
	int	i,j;

	for (i=0;i<nToolbarsLeft;i++)
	{
		if (ToolbarsLeft[i] == ToolbarID)
			return TRUE;
	}
	for (i=0;i<nToolbarsRight;i++)
	{
		if (ToolbarsRight[i] == ToolbarID)
			return TRUE;
	}
	for (i=0;i<nToolbarsTop;i++)
	{
		if (ToolbarsTop[i] == ToolbarID)
			return TRUE;
	}
	for (i=0;i<nToolbarsBottom;i++)
	{
		if (ToolbarsBottom[i] == ToolbarID)
			return TRUE;
	}

	return FALSE;
}

BOOL SeeIfToolbarShouldBeDocked (HWND hWnd,LPPOINT pPos)
{
	int ToolbarID = GetToolbarIDFromWnd (hWnd);
	RECT	rect;
	BOOL	rtn=FALSE;
	int		i,j;
	RECT	mainRect;

	if (!GetGlobalBVal2 ("[%ALLOWTOOLBARDOCKING]",TRUE))
		return FALSE;
	GetClientRect (hWndMain,&mainRect);

	rtn = RemoveDockedToolbar (ToolbarID);
	if (ToolbarID >= 0)
	{
		GetWindowRect (hWnd,&rect);
		if (rect.left >= 5000)
		{
			int	w=RECTWIDTH(&rect),h=RECTHEIGHT(&rect);

			rect.left = pPos->x;
			rect.top  = pPos->y;
			rect.right = rect.left + w;
			rect.bottom = rect.top + h;
		}
		ScreenRectToClientRect (hWndMain,&rect);
		pPos->x = rect.left;
		pPos->y = rect.top;
		if (rect.left <= mainRect.left + ToolbarWidthLeft)
		{
			rtn = TRUE;
			if (!nToolbarsLeft)
				ToolbarsLeft[nToolbarsLeft++] = ToolbarID;
			else if (pPos->y >= ToolbarRect[ToolbarsLeft[nToolbarsLeft-1]].top)
				ToolbarsLeft[nToolbarsLeft++] = ToolbarID;
			else
			{
				for (i=0;i<nToolbarsLeft;i++)
				{
					if (pPos->y <= ToolbarRect[ToolbarsLeft[i]].top)
					{
						j = nToolbarsLeft+1;
						while (j-- > i)
							ToolbarsLeft[j] = ToolbarsLeft[j-1];
						ToolbarsLeft[i] = ToolbarID;
						nToolbarsLeft++;
						goto Exit;
					}
				}
			}
		}
		else if (rect.right >= mainRect.right - ToolbarWidthRight)
		{
			rtn = TRUE;
			if (!nToolbarsRight)
				ToolbarsRight[nToolbarsRight++] = ToolbarID;
			else if (pPos->y >= ToolbarRect[ToolbarsRight[nToolbarsRight-1]].top)
				ToolbarsRight[nToolbarsRight++] = ToolbarID;
			else
			{
				for (i=0;i<nToolbarsRight;i++)
				{
					if (pPos->y <= ToolbarRect[ToolbarsRight[i]].top)
					{
						j = nToolbarsRight+1;
						while (j-- > i)
							ToolbarsRight[j] = ToolbarsRight[j-1];
						ToolbarsRight[i] = ToolbarID;
						nToolbarsRight++;
						goto Exit;
					}
				}
			}
		}
		else if (rect.top <= mainRect.top + ToolbarWidthTop)
		{
			rtn = TRUE;
			if (!nToolbarsTop)
				ToolbarsTop[nToolbarsTop++] = ToolbarID;
			else if (pPos->x >= ToolbarRect[ToolbarsTop[nToolbarsTop-1]].left)
				ToolbarsTop[nToolbarsTop++] = ToolbarID;
			else
			{
				for (i=0;i<nToolbarsTop;i++)
				{
					if (pPos->x <= ToolbarRect[ToolbarsTop[i]].left)
					{
						j = nToolbarsTop+1;
						while (j-- > i)
							ToolbarsTop[j] = ToolbarsTop[j-1];
						ToolbarsTop[i] = ToolbarID;
						nToolbarsTop++;
						goto Exit;
					}
				}
			}
		}
		else if (rect.bottom >= mainRect.bottom - ToolbarWidthBottom)
		{
			rtn = TRUE;
			if (!nToolbarsBottom)
				ToolbarsBottom[nToolbarsBottom++] = ToolbarID;
			else if (pPos->x >= ToolbarRect[ToolbarsBottom[nToolbarsBottom-1]].left)
				ToolbarsBottom[nToolbarsBottom++] = ToolbarID;
			else
			{
				for (i=0;i<nToolbarsBottom;i++)
				{
					if (pPos->x <= ToolbarRect[ToolbarsBottom[i]].left)
					{
						j = nToolbarsBottom+1;
						while (j-- > i)
							ToolbarsBottom[j] = ToolbarsBottom[j-1];
						ToolbarsBottom[i] = ToolbarID;
						nToolbarsBottom++;
						goto Exit;
					}
				}
			}
		}
	}
Exit:
	if (rtn)
		AdjustToolbarPositions ();
	return rtn;
}

void CheckIfBestRowConfig (BOOL Verticle,int MaxScreenHW,int nLevels,LPSHORT pToolbars,LPINT levConfig)
{
	int	maxW=0, maxH=0, i;
	
	if (Verticle)
	{
		for (i=0;i<nLevels;i++)
		{
			//SetToolbarConfig (pToolbars[i],0);
			maxW = max (maxW,ToolbarConfigs[pToolbars[i]][levConfig[i]-1].cx);
			maxH = maxH + ToolbarConfigs[pToolbars[i]][levConfig[i]-1].cy;
		}
		if (maxW < BestW && maxH <= MaxScreenHW)
		{
			BestW = maxW;
			memmove (BestRowConfigs,levConfig,sizeof(int)*nLevels);
		}
	}
	else
	{
		for (i=0;i<nLevels;i++)
		{
			//SetToolbarConfig (pToolbars[i],0);
			maxH = max (maxH,ToolbarConfigs[pToolbars[i]][levConfig[i]-1].cy);
			maxW = maxW + ToolbarConfigs[pToolbars[i]][levConfig[i]-1].cx;
		}
		if (maxH < BestH && maxW <= MaxScreenHW)
		{
			BestH = maxH;
			memmove (BestRowConfigs,levConfig,sizeof(int)*nLevels);
		}
	}

	return;
}

BOOL AdjustToolbarPositions (void)
{
	RECT	rect;
	int		w, h, cw,ch, i,j,x,y, iSave;
	RECT	MainRect, rect2;
	BOOL	DoRedisplay=FALSE;
	POINT	pt={0,0};
	int		levConfig[MAX_TOOLBARS];
	int		nLevels, level;
	LPSHORT	pToolbars;
	BOOL	Verticle;
	HANDLE	hAllComb=0;
	LPSTR	pAllComb;
	int		nRows;
	int		BestTotBestWH;
	int		ToolbarID;

	if (!DisplayToolbars)
		return FALSE;
	ClientToScreen (hWndMain,&pt);

	GetClientRect (hWndMain,&MainRect);
	if (!IsRectEmpty (&PromptRect))
		MainRect.bottom = PromptRect.top - 1;

	iSave = ToolbarWidthTop;
	if (nToolbarsTop)
	{
		nLevels = nToolbarsTop;
		pToolbars = ToolbarsTop;
		Verticle = FALSE;
		BestTotBestWH = INT_MAX;
		hAllComb = GetTBRows (nLevels,pToolbars);
		pAllComb = GlobalLock (hAllComb);
		while (*pAllComb)
		{
			int	iRow = 0;
	
			TotBestWH = 0;

			while (*pAllComb != '$')
			{

				nInRow[iRow] = ldread (pAllComb,2);
				pAllComb += 2;
				for (j=0;j<nInRow[iRow];j++)
				{
					TBInRow[iRow][j] = ldread (pAllComb,2);
					pAllComb += 2;
				}
				iRow++;
			}
			pAllComb++;
			nRows = iRow;
			for (iRow = 0;iRow<nRows;iRow++)
			{
				pToolbars = TBInRow[iRow];
				BestW = BestH = INT_MAX;
				memset (levConfig,0,sizeof(levConfig));
				level = 0;
				while (level >= 0)
				{
					if (++levConfig[level] > nToolbarConfigs[pToolbars[level]])
					{
						level--;
						continue;
					}
					while (++level < nInRow[iRow])
						levConfig[level] = 1;
					CheckIfBestRowConfig (Verticle,MainRect.right-ToolbarWidthLeft-ToolbarWidthRight,nInRow[iRow],pToolbars,levConfig);
					level--;
				}
				x = ToolbarWidthLeft+1;
				for (i=0;i<nInRow[iRow];i++)
				{
					BestTBConfigs[pToolbars[i]] = BestRowConfigs[i];
					BestConfigRect[pToolbars[i]].left = x;
					BestConfigRect[pToolbars[i]].right = x+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cx;
					BestConfigRect[pToolbars[i]].top = TotBestWH + (BestH - ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cy)/2 + 1;
					BestConfigRect[pToolbars[i]].bottom = BestConfigRect[pToolbars[i]].top+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cy;
					x = BestConfigRect[pToolbars[i]].right + 2;
				}
				TotBestWH += BestH;
			}
			if (TotBestWH < BestTotBestWH)
			{
				BestTotBestWH = TotBestWH;
				memmove (TotBestConfigRect,BestConfigRect,nToolbars*sizeof(RECT));
				memmove (TotBestConfigs,BestTBConfigs,nToolbars*sizeof(int));
			}
		}
		ToolbarWidthTop = BestTotBestWH+2;
		if (ToolbarWidthTop < 0 || ToolbarWidthTop > MainRect.bottom)
			ToolbarWidthTop = 0;
		else
		for (j=0;j<nToolbarsTop;j++)
		{
			int	ToolbarID = ToolbarsTop[j];

			ToolbarCurrentConfig[ToolbarID] = TotBestConfigs[ToolbarID]-1;
			ToolBarStartPoint.x = TotBestConfigRect[ToolbarID].left;
			ToolBarStartPoint.y = TotBestConfigRect[ToolbarID].top;
			if (ToolbarFloating[ToolbarID] && ToolbarType[ToolbarID] == TBT_STANDARDMENU_DOCKED)
			{
				HWND	SaveToolbarWindow = ToolbarWindow[ToolbarID];
				DisplayToolbars = FALSE;
				ToolbarWindow[ToolbarID] = 0;
				DestroyWindow(SaveToolbarWindow);
				//DisplayToolbars = TRUE;
				ToolbarWindow[ToolbarID] = SaveToolbarWindow;
				ToolbarFloating[ToolbarID] = FALSE;
				GSSiDeleteObject (&ToolbarImage[ToolbarID]);
				ToolbarIDCur = ToolbarID;
				ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_DOCKED", hWndMain, (DLGPROC)TOOLBARMsgProc); 
				DoCreateDialogTooltip(ToolbarID);
			}
			else
			{
				ToolbarFloating[ToolbarID] = FALSE;
				SendMessage (ToolbarWindow[ToolbarID],GSSI_REINITDIALOG,0,0);
			}
		}
		GSSiGlobUlFree (&hAllComb);
	}
	else
		ToolbarWidthTop = 0;
	if (ToolbarWidthTop != iSave)
		DoRedisplay = TRUE;
	iSave = ToolbarWidthBottom;
	if (nToolbarsBottom)
	{
		nLevels = nToolbarsBottom;
		pToolbars = ToolbarsBottom;
		Verticle = FALSE;
		BestTotBestWH = INT_MAX;
		hAllComb = GetTBRows (nLevels,pToolbars);
		pAllComb = GlobalLock (hAllComb);
		while (*pAllComb)
		{
			int	iRow = 0;
	
			TotBestWH = 0;

			while (*pAllComb != '$')
			{

				nInRow[iRow] = ldread (pAllComb,2);
				pAllComb += 2;
				for (j=0;j<nInRow[iRow];j++)
				{
					TBInRow[iRow][j] = ldread (pAllComb,2);
					pAllComb += 2;
				}
				iRow++;
			}
			pAllComb++;
			nRows = iRow;
			for (iRow = 0;iRow<nRows;iRow++)
			{
				pToolbars = TBInRow[iRow];
				BestW = BestH = INT_MAX;
				memset (levConfig,0,sizeof(levConfig));
				level = 0;
				while (level >= 0)
				{
					if (++levConfig[level] > nToolbarConfigs[pToolbars[level]])
					{
						level--;
						continue;
					}
					while (++level < nInRow[iRow])
						levConfig[level] = 1;
					CheckIfBestRowConfig (Verticle,MainRect.right-ToolbarWidthLeft-ToolbarWidthRight,nInRow[iRow],pToolbars,levConfig);
					level--;
				}
				x = ToolbarWidthLeft+1;
				for (i=0;i<nInRow[iRow];i++)
				{
					BestTBConfigs[pToolbars[i]] = BestRowConfigs[i];
					BestConfigRect[pToolbars[i]].left = x;
					BestConfigRect[pToolbars[i]].right = x+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cx;
					BestConfigRect[pToolbars[i]].top = TotBestWH + (BestH - ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cy)/2 + 1;
					BestConfigRect[pToolbars[i]].bottom = BestConfigRect[pToolbars[i]].top+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cy;
					x = BestConfigRect[pToolbars[i]].right + 2;
				}
				TotBestWH += BestH;
			}
			if (TotBestWH < BestTotBestWH)
			{
				BestTotBestWH = TotBestWH;
				memmove (TotBestConfigRect,BestConfigRect,nToolbars*sizeof(RECT));
				memmove (TotBestConfigs,BestTBConfigs,nToolbars*sizeof(int));
			}
		}
		ToolbarWidthBottom = BestTotBestWH+2;
		if (ToolbarWidthBottom < 0 || ToolbarWidthBottom > MainRect.bottom)
			ToolbarWidthBottom = 0;
		else
		for (j=0;j<nToolbarsBottom;j++)
		{
			int	ToolbarID = ToolbarsBottom[j];

			ToolbarCurrentConfig[ToolbarID] = TotBestConfigs[ToolbarID]-1;
			ToolBarStartPoint.x = TotBestConfigRect[ToolbarID].left;
			ToolBarStartPoint.y = TotBestConfigRect[ToolbarID].top+MainRect.bottom-ToolbarWidthBottom;
			if (ToolbarFloating[ToolbarID] && ToolbarType[ToolbarID] == TBT_STANDARDMENU_DOCKED)
			{
				HWND	SaveToolbarWindow = ToolbarWindow[ToolbarID];
				DisplayToolbars = FALSE;
				ToolbarWindow[ToolbarID] = 0;
				DestroyWindow(SaveToolbarWindow);
				//DisplayToolbars = TRUE;
				ToolbarWindow[ToolbarID] = SaveToolbarWindow;
				ToolbarFloating[ToolbarID] = FALSE;
				GSSiDeleteObject (&ToolbarImage[ToolbarID]);
				ToolbarIDCur = ToolbarID;
				ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_DOCKED", hWndMain, (DLGPROC)TOOLBARMsgProc);
				DoCreateDialogTooltip(ToolbarID);
			}
			else
			{
				ToolbarFloating[ToolbarID] = FALSE;
				SendMessage (ToolbarWindow[ToolbarID],GSSI_REINITDIALOG,0,0);
			}
		}
		GSSiGlobUlFree (&hAllComb);
	}
	else
		ToolbarWidthBottom = 0;
	if (ToolbarWidthBottom != iSave)
		DoRedisplay = TRUE;
	iSave = ToolbarWidthLeft;
	if (nToolbarsLeft)
	{
		nLevels = nToolbarsLeft;
		pToolbars = ToolbarsLeft;
		Verticle = TRUE;
		BestTotBestWH = INT_MAX;
		hAllComb = GetTBRows (nLevels,pToolbars);
		pAllComb = GlobalLock (hAllComb);
		while (*pAllComb)
		{
			int	iRow = 0;
	
			TotBestWH = 0;

			while (*pAllComb != '$')
			{

				nInRow[iRow] = ldread (pAllComb,2);
				pAllComb += 2;
				for (j=0;j<nInRow[iRow];j++)
				{
					TBInRow[iRow][j] = ldread (pAllComb,2);
					pAllComb += 2;
				}
				iRow++;
			}
			pAllComb++;
			nRows = iRow;
			for (iRow = 0;iRow<nRows;iRow++)
			{
				pToolbars = TBInRow[iRow];
				BestW = BestH = INT_MAX;
				memset (levConfig,0,sizeof(levConfig));
				level = 0;
				while (level >= 0)
				{
					if (++levConfig[level] > nToolbarConfigs[pToolbars[level]])
					{
						level--;
						continue;
					}
					while (++level < nInRow[iRow])
						levConfig[level] = 1;
					CheckIfBestRowConfig (Verticle,MainRect.bottom-ToolbarWidthTop-ToolbarWidthBottom,nInRow[iRow],pToolbars,levConfig);
					level--;
				}
				y = ToolbarWidthTop+1;
				for (i=0;i<nInRow[iRow];i++)
				{
					BestTBConfigs[pToolbars[i]] = BestRowConfigs[i];
					BestConfigRect[pToolbars[i]].top = y;
					BestConfigRect[pToolbars[i]].bottom = y+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cy;
					BestConfigRect[pToolbars[i]].left = TotBestWH + (BestW - ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cx)/2 + 1;
					BestConfigRect[pToolbars[i]].right = BestConfigRect[pToolbars[i]].left+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cx;
					y = BestConfigRect[pToolbars[i]].bottom + 2;
				}
				TotBestWH += BestW;
			}
			if (TotBestWH < BestTotBestWH)
			{
				BestTotBestWH = TotBestWH;
				memmove (TotBestConfigRect,BestConfigRect,nToolbars*sizeof(RECT));
				memmove (TotBestConfigs,BestTBConfigs,nToolbars*sizeof(int));
			}
		}
		ToolbarWidthLeft = BestTotBestWH+2;
		if (ToolbarWidthLeft < 0 || ToolbarWidthLeft > MainRect.right)
			ToolbarWidthLeft = 0;
		else
		for (j=0;j<nToolbarsLeft;j++)
		{
			int	ToolbarID = ToolbarsLeft[j];

			ToolbarCurrentConfig[ToolbarID] = TotBestConfigs[ToolbarID]-1;
			ToolBarStartPoint.x = TotBestConfigRect[ToolbarID].left;
			ToolBarStartPoint.y = TotBestConfigRect[ToolbarID].top;
			if (ToolbarFloating[ToolbarID] && ToolbarType[ToolbarID] == TBT_STANDARDMENU_DOCKED)
			{
				HWND saveToolbarWindow = ToolbarWindow[ToolbarID];

				ToolbarWindow[ToolbarID] = 0;
				DisplayToolbars = FALSE;
				DestroyWindow(saveToolbarWindow);
				//DisplayToolbars = TRUE;
				ToolbarWindow[ToolbarID] = saveToolbarWindow;
				ToolbarFloating[ToolbarID] = FALSE;
				GSSiDeleteObject (&ToolbarImage[ToolbarID]);
				ToolbarIDCur = ToolbarID;
				ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_DOCKED", hWndMain, (DLGPROC)TOOLBARMsgProc);
				DoCreateDialogTooltip(ToolbarID);
			}
			else
			{
				ToolbarFloating[ToolbarID] = FALSE;
				SendMessage (ToolbarWindow[ToolbarID],GSSI_REINITDIALOG,0,0);
			}
		}
		GSSiGlobUlFree (&hAllComb);
	}
	else
		ToolbarWidthLeft = 0;
	if (ToolbarWidthLeft != iSave)
		DoRedisplay = TRUE;
	iSave = ToolbarWidthRight;
	if (nToolbarsRight)
	{
		nLevels = nToolbarsRight;
		pToolbars = ToolbarsRight;
		Verticle = TRUE;
		BestTotBestWH = INT_MAX;
		hAllComb = GetTBRows (nLevels,pToolbars);
		pAllComb = GlobalLock (hAllComb);
		while (*pAllComb)
		{
			int	iRow = 0;
	
			TotBestWH = 0;

			while (*pAllComb != '$')
			{

				nInRow[iRow] = ldread (pAllComb,2);
				pAllComb += 2;
				for (j=0;j<nInRow[iRow];j++)
				{
					TBInRow[iRow][j] = ldread (pAllComb,2);
					pAllComb += 2;
				}
				iRow++;
			}
			pAllComb++;
			nRows = iRow;
			for (iRow = 0;iRow<nRows;iRow++)
			{
				pToolbars = TBInRow[iRow];
				BestW = BestH = INT_MAX;
				memset (levConfig,0,sizeof(levConfig));
				level = 0;
				while (level >= 0)
				{
					if (++levConfig[level] > nToolbarConfigs[pToolbars[level]])
					{
						level--;
						continue;
					}
					while (++level < nInRow[iRow])
						levConfig[level] = 1;
					CheckIfBestRowConfig (Verticle,MainRect.bottom-ToolbarWidthTop-ToolbarWidthBottom,nInRow[iRow],pToolbars,levConfig);
					level--;
				}
				y = ToolbarWidthTop+1;
				for (i=0;i<nInRow[iRow];i++)
				{
					BestTBConfigs[pToolbars[i]] = BestRowConfigs[i];
					BestConfigRect[pToolbars[i]].top = y;
					BestConfigRect[pToolbars[i]].bottom = y+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cy;
					BestConfigRect[pToolbars[i]].left = TotBestWH + (BestW - ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cx)/2 + 1;
					BestConfigRect[pToolbars[i]].right = BestConfigRect[pToolbars[i]].left+ToolbarConfigs[pToolbars[i]][BestRowConfigs[i]-1].cx;
					y = BestConfigRect[pToolbars[i]].bottom + 2;
				}
				TotBestWH += BestW;
			}
			if (TotBestWH < BestTotBestWH)
			{
				BestTotBestWH = TotBestWH;
				memmove (TotBestConfigRect,BestConfigRect,nToolbars*sizeof(RECT));
				memmove (TotBestConfigs,BestTBConfigs,nToolbars*sizeof(int));
			}
		}
		ToolbarWidthRight = BestTotBestWH+2;
		if (ToolbarWidthRight < 0 || ToolbarWidthRight > MainRect.right)
			ToolbarWidthRight = 0;
		else
		for (j=0;j<nToolbarsRight;j++)
		{
			int	ToolbarID = ToolbarsRight[j];

			ToolbarCurrentConfig[ToolbarID] = TotBestConfigs[ToolbarID]-1;
			ToolBarStartPoint.x = TotBestConfigRect[ToolbarID].left + MainRect.right - ToolbarWidthRight;
			ToolBarStartPoint.y = TotBestConfigRect[ToolbarID].top;
			if (ToolbarFloating[ToolbarID] && ToolbarType[ToolbarID] == TBT_STANDARDMENU_DOCKED)
			{
				HWND saveToolbarWindow = ToolbarWindow[ToolbarID];

				ToolbarWindow[ToolbarID] = 0;
				DisplayToolbars = FALSE;
				DestroyWindow(saveToolbarWindow);
				//DisplayToolbars = TRUE;
				ToolbarWindow[ToolbarID] = saveToolbarWindow;
				ToolbarFloating[ToolbarID] = FALSE;
				GSSiDeleteObject (&ToolbarImage[ToolbarID]);
				ToolbarIDCur = ToolbarID;
				ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_DOCKED", hWndMain, (DLGPROC)TOOLBARMsgProc);
				DoCreateDialogTooltip(ToolbarID);
			}
			else
			{
				ToolbarFloating[ToolbarID] = FALSE;
				SendMessage (ToolbarWindow[ToolbarID],GSSI_REINITDIALOG,0,0);
			}
		}
		GSSiGlobUlFree (&hAllComb);
	}
	else
		ToolbarWidthRight = 0;
	if (ToolbarWidthRight != iSave)
		DoRedisplay = TRUE;
	
	for (ToolbarID = 0;ToolbarID < nToolbars;ToolbarID++)
	{
		if (!ToolbarIsDocked (ToolbarID) && !ToolbarFloating[ToolbarID])
			switch (ToolbarType[ToolbarID])
			{
				case TBT_STANDARDMENU_DOCKED:
				{
					HWND	SaveToolbarWindow = ToolbarWindow[ToolbarID];
					int	h,w;

					GetWindowRect (ToolbarWindow[ToolbarID],&rect);
					DisplayToolbars = FALSE;
					ToolbarWindow[ToolbarID] = 0;
					DestroyWindow(SaveToolbarWindow);
					//DisplayToolbars = TRUE;
					ToolbarWindow[ToolbarID] = SaveToolbarWindow;
					ToolbarFloating[ToolbarID] = TRUE;
					ToolbarCurrentConfig[ToolbarID] = 0;
					ToolBarStartPoint.x = rect.left;
					ToolBarStartPoint.y = rect.top;
					GSSiDeleteObject (&ToolbarImage[ToolbarID]);
					ToolbarIDCur = ToolbarID;
					ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_FLOAT", hWndMain, (DLGPROC)TOOLBARMsgProc);
					DoCreateDialogTooltip(ToolbarID);
					h = RECTHEIGHT(&ToolbarRect[ToolbarID]);
					w = RECTWIDTH(&ToolbarRect[ToolbarID]);
					ToolbarRect[ToolbarID].left = rect.left;
					ToolbarRect[ToolbarID].top = rect.top;
					ToolbarRect[ToolbarID].bottom = ToolbarRect[ToolbarID].top + h;
					ToolbarRect[ToolbarID].right = ToolbarRect[ToolbarID].left + w;
					//SetWindowPos (ToolbarWindow[ToolbarID],0,rect.left,rect.top,0,0,SWP_DRAWFRAME|SWP_SHOWWINDOW|SWP_NOSIZE);
				}
				break;
				case TBT_ZOOMPAN:
				{
					ToolbarFloating[ToolbarID] = TRUE;
					ToolbarCurrentConfig[ToolbarID] = 0;
				}
				break;
				case TBT_VISMENU:
				{
					ToolbarFloating[ToolbarID] = TRUE;
					ToolbarCurrentConfig[ToolbarID] = 0;
					FloatVisMenu (ToolbarWindow[ToolbarID]);
				}
				break;
		}
	}

	GSSiGlobUlFree (&hAllComb);
	if (DoRedisplay)
	{
		int SaveNFiles[MAX_VIEWPORTS];
		MNMXCORD SaveWBounds[MAX_VIEWPORTS];
		DPOINT	VPPoints[MAX_VIEWPORTS][4];
		RECT	SaveRect[MAX_VIEWPORTS];
		double	SaveOrthoRes[MAX_VIEWPORTS];
		int		SaveWZTO[MAX_VIEWPORTS];
		DPOINT	VPMidW[MAX_VIEWPORTS];
		POINT	VPMidS[MAX_VIEWPORTS];
		int	iview, i, mainW, mainH;
		RECT	mainRect;
		HDC		hDCMem, hDCMain = GetDC (hWndMain), hDC;
		HBITMAP	hBMMem, hBMOld, hBM;
		POINT	points[4];
		LPVIEWPORT	SaveVP = CurView;
		BOOL	DoRedraw = FALSE;
		DPOINT	ptW;
		double	mainArea;

		//RedisplayWindow ();
 		SetSysMess ("");
		hDCMem = CreateCompatibleDC (hDCMain);
		SetConfigDisplayRect ();
		GetClientRect (hWndMain,&mainRect);
		mainArea =  RectArea (&mainRect);
		mainW = RECTWIDTH(&mainRect);
		mainH = RECTHEIGHT(&mainRect);
		hBMMem = CreateCompatibleBitmap (hDCMain,mainW,mainH);
		hBMOld = SelectObject (hDCMem,hBMMem);
		BitBlt (hDCMem,0,0,mainW,mainH,hDCMain,0,0,SRCCOPY);
		if (!NumViewportsArray[0])
		{
			ClearFullWindowBitmap (0);
			FillRect (hDCMain,&mainRect,GetStockObject (WHITE_BRUSH));
		}
		ReleaseDC (hWndMain,hDCMain);
    	SetConfig (1);
		for (iview = 0;iview < *pNumViewports; iview++)
		{
			CurView = pViewportsD[iview];
			SaveNFiles[iview] = pViewportsD[iview]->NumFiles;
			if (VPIsActive (pViewportsD[iview]->ID) && VPIsMap (pViewportsD[iview]->ID))
			{
				VPMidS[iview] = RectMid (&CurView->ScreenRect);
				VPMidW[iview] = ScreenPtToBasePt (VPMidS[iview]);
				SaveOrthoRes[iview] = pViewportsD[iview]->OrthoRes;
				SaveWZTO[iview] = pViewportsD[iview]->WindowZoomedToOrtho;
				pViewportsD[iview]->OrthoRes = pViewportsD[iview]->Scale;
				pViewportsD[iview]->WindowZoomedToOrtho = 2;
				pViewportsD[iview]->NumFiles = 0;
				SaveRect[iview] =pViewportsD[iview]->ScreenRect;
				RectToPoints (&pViewportsD[iview]->ScreenRect,points);
				CloseTRANS2 (&CurView->hTranVPToBase);
				CloseTRANS2 (&CurView->hTranBaseToVP);
				for (i=0;i<4;i++)
					VPPoints[iview][i] = ScreenPtToBasePt (points[i]);
			}
		}
		PaintMap (pViewportsD[0]->hWnd,pViewportsD[0]->hDC,TRUE,NULL,0); 
		ShowBufferedScreen (TRUE,FALSE,0,0);
		hDC = ScreenBufferDC (CurView->hWnd,CurView->hDC);
		DoRedraw = FALSE;
    	SetConfig (1);
		for (iview = 0;iview < *pNumViewports; iview++)
		{
			CurView = pViewportsD[iview];
 			pViewportsD[iview]->NumFiles = SaveNFiles[iview];
			if (VPIsActive (pViewportsD[iview]->ID) && VPIsMap (pViewportsD[iview]->ID))
			{
				RECT	vpRect;

				if (CurView->HaveBounds)
				{
					CurView->NumFiles = -1;
             		SetBounds(CurView->hWnd,(HDC)1);  
					pViewportsD[iview]->NumFiles = SaveNFiles[iview];
				}
				CurView->HaveBounds = TRUE;
				RectInit (&vpRect);
				CloseTRANS2 (&CurView->hTranVPToBase);
				CloseTRANS2 (&CurView->hTranBaseToVP);
				AdjustBoundsAndDrawRectToRotation ();
				if (RectArea (&CurView->ScreenRect) > 0.3 * mainArea)
				{
					ptW = ScreenPtToBasePt (VPMidS[iview]);
					CurView->MidPointW.x -= (ptW.x - VPMidW[iview].x);
					CurView->MidPointW.y -= (ptW.y - VPMidW[iview].y);
					if (CurView->HaveBounds)
					{
						CurView->NumFiles = -1;
             			SetBounds(CurView->hWnd,(HDC)1);  
						pViewportsD[iview]->NumFiles = SaveNFiles[iview];
					}
					CurView->HaveBounds = TRUE;
					RectInit (&vpRect);
					CloseTRANS2 (&CurView->hTranVPToBase);
					CloseTRANS2 (&CurView->hTranBaseToVP);
					AdjustBoundsAndDrawRectToRotation ();
				}
				for (i=0;i<4;i++)
				{
					points[i] = BasePtToScreenPt (&VPPoints[iview][i]);
					AddPointToRect (points[i],&vpRect);
				}
				CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  				SelectClipRgn (CurView->hDC,CurView->hRgn);
  				GSSiDeleteObject(&CurView->hRgn); 
				StretchBlt (hDC,vpRect.left,vpRect.top,RECTWIDTH(&vpRect),RECTHEIGHT(&vpRect),
						    hDCMem,SaveRect[iview].left,SaveRect[iview].top,RECTWIDTH(&SaveRect[iview]),RECTHEIGHT(&SaveRect[iview]),
						    SRCCOPY);
				if (vpRect.left > CurView->ScreenRect.left ||
					vpRect.right < CurView->ScreenRect.right ||
					vpRect.top > CurView->ScreenRect.top ||
					vpRect.bottom < CurView->ScreenRect.bottom)
					DoRedraw = TRUE;
				CurView->OrthoRes = SaveOrthoRes[iview];
				CurView->WindowZoomedToOrtho = SaveWZTO[iview];
				CurView->LastWidth = 0;
			}
		}
		CurView = SaveVP;
		hBM = SelectObject (hDCMem,hBMOld);
		GSSiDeleteObject (&hBM);
		DeleteDC (hDCMem);
		DisplayToolbars = TRUE;
		ShowBufferedScreen (TRUE,FALSE,0,0);
		if (DoRedraw)
			RedisplayWindow ();
		ResetObjectCount ();
	}
	DisplayToolbars = TRUE;
	DisplayAllToolbars (2);
	return DoRedisplay;
}

int DisplayBitmapInRect (HDC hDC,RECT Rect,HBITMAP hBM,short MaintainAspect,UINT Rop)
{
	int	rc;
	HDIB32	hDIB32;

	hDIB32 = BitmapToDIB32 (hBM); 
	rc = DisplayBMInRect32_2 (hDC,hDIB32,Rect,MaintainAspect,Rop);
	GMDestroyDIB32 (hDIB32);
	return rc;
}

void DisplayJoyStick (HDC hDC,POINT Point)
{
	HBITMAP	hBM;
	RECT	rect;
	UINT	ii,rop2=0x00FE02A9;
	HDIB32	hDIB;

	DisplayPZRotImage(2);//RestoreScreen2 (hDC, hSavePZRScreen2,0,FALSE);
	hBM = LoadBitmap (hInst,"JOYSTICK");
	rect.left = Point.x - 11;
	rect.top = Point.y - 11;
	rect.right = Point.x + 11;
	rect.bottom = Point.y + 11;
//	hRgn = CreateEllipticRgn(rect.left,rect.top,rect.right,rect.bottom);
//	ii=SelectClipRgn (hDC,hRgn);
	hDIB = BitmapToDIB32 (hBM);
	DisplayTransparentBitmapInRect (hDC,hDIB, &rect,FALSE);
	DestroyDIB32 (hDIB,TRUE); 
//	DisplayBitmapInRect (hDC,rect,hBM,2,SRCCOPY);
//	SelectClipRgn (hDC,0);
//	DeleteObject (hRgn);
	DeleteObject (hBM);
	return;
}

BOOL AdjustMainRectToMenus (LPRECT MRect,LPRECT ConfigDisplayRect)
{
	if (!NumViewportsArray[0])
		*ConfigDisplayRect = *MRect;
//	InflateRect (ConfigDisplayRect,-100,-100);
	return TRUE;
}

BOOL RemoveToolBarsFromMainRect (LPRECT pRect)
{
	pRect->left		+= ToolbarWidthLeft;
	pRect->bottom	-= ToolbarWidthBottom;
	pRect->right	-= ToolbarWidthRight;
	pRect->top		+= ToolbarWidthTop;
	return TRUE;
}

BOOL TextOutWithShadow (HDC hDC,int x,int y,LPSTR txt,int l,int inc,COLORREF ShadowColor)
{
	COLORREF	SaveColor = SetTextColor (hDC,ShadowColor); 
	int			i;
	BOOL		rtn;
	int	OldMode = SetBkMode (hDC,TRANSPARENT);

	for (i=1;i<inc*DeviceToScreenFactor()+1;i++)
	{
		ExtTextOut (hDC,x+i,y+i,0,0,txt,l,0);
		ExtTextOut (hDC,x-i,y+i,0,0,txt,l,0);
		ExtTextOut (hDC,x-i,y-i,0,0,txt,l,0);
		ExtTextOut (hDC,x+i,y-i,0,0,txt,l,0);
	}
	SetTextColor (hDC,SaveColor);
	rtn = ExtTextOut (hDC,x,y,0,0,txt,l,0);
	SetBkMode (hDC,OldMode);
#if CHECKMEM
	GdiFlush ();
#endif
	return rtn;
}

int DisplayZoomInOut (HWND hWnd,HDC hDC,int which,COLORREF color)
{
	HFONT	hFont = CreateFont(10,0, 0, 0, FW_NORMAL, 
    				0, 0, 0, 0, 0, 0, 0, 0,"Arial Black");   
	HFONT	OldFont;

	SaveDC (hDC);
	OldFont  = SelectObject (hDC,hFont);
    SetTextColor(hDC, color);
	if (which)
		TextOutWithShadow (hDC,PZRMidPoint.x-15,PZRMidPoint.y-27,"Zoom In",7,0,RGB(255,255,255));
	else
		TextOutWithShadow (hDC,PZRMidPoint.x-18,PZRMidPoint.y+12,"Zoom Out",8,0,RGB(255,255,255));
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	RestoreDC (hDC,-1);
	return which+1;
}

void SelectZoomInOut (HDC hDC,int io)
{
	HFONT	hFont = CreateFont(24,0, 0, 0, FW_BOLD, 
    						0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
	HFONT	OldFont  = SelectObject (hDC,hFont);

	if (io)
	    SetTextColor (hDC, 255);
	else
		SetTextColor (hDC,0);
	if (io < 2)
		TextOutWithShadow (hDC,PZRMidPoint.x-35,PZRMidPoint.y-14,"-",1,1,RGB(255,255,255));
	if (!io || io == 2)
		TextOutWithShadow (hDC,PZRMidPoint.x+24,PZRMidPoint.y-13,"+",1,1,RGB(255,255,255));
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	return;
}

void SethDCRotation (HDC hDC,POINT Point,double Rotation)
{
	XFORM xForm;
	float	RSQMIN;
	double	XFROM[2], YFROM[2], XTO[2], YTO[2]; 
	DPOINT	Point1, Point2;
	HANDLE	hTran;

	Point1 = PointToDPoint (Point);
	Point2 = dnewpt (Point1,-Rotation,100);
	XFROM[0] = Point1.x;
	YFROM[0] = Point1.y;
	XFROM[1] = Point1.x + 100;
	YFROM[1] = YFROM[0];
	XTO[0]   = XFROM[0];
	YTO[0]   = YFROM[0];
	XTO[1]   = Point2.x;
	YTO[1]	 = Point2.y;
	hTran = STRAN2 (1658,XFROM,YFROM,XTO,YTO,2,&RSQMIN,1,0);  
	xForm =  SetXFORMFromTRANS (hTran);
	CloseTRANS2 (&hTran);
	SetGraphicsMode(hDC, GM_ADVANCED);
	SetWorldTransform(hDC, &xForm);
	return;
}

void DisplayCompassDir (HDC hDC,int Dir,POINT MidP,double rot)
{
	HFONT	hFont = CreateFont(13,0, 0, 0, FW_BOLD, 
   						0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
	HFONT	OldFont  = SelectObject (hDC,hFont);

	SaveDC (hDC);
	SethDCRotation (hDC,MidP,rot);
	if (Dir>0)
	    SetTextColor (hDC, 255);
	else
		SetTextColor (hDC,0);
	OldFont  = SelectObject (hDC,hFont);
	switch (abs(Dir))
	{
	case 1:
		TextOutWithShadow (hDC,MidP.x-3,MidP.y-radius+1,"N",1,1,RGB(255,255,255));
		break;
	case 2:
		TextOutWithShadow (hDC,MidP.x+radius-11,MidP.y-7,"E",1,1,RGB(255,255,255));
		break;
	case 3:
		TextOutWithShadow (hDC,MidP.x-radius+4,MidP.y-6,"W",1,1,RGB(255,255,255));
		break;
	case 4:
		TextOutWithShadow (hDC,MidP.x-3,MidP.y+radius-13,"S",1,1,RGB(255,255,255));
		break;
	}
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	RestoreDC (hDC,-1);
	return;
}

void DisplayPZIcons (HWND hWnd,HDC hDC)
{
	RECT	rect;
	char	FileName[MAX_PATH];
	HDIB32	hDib32;
	int		w=22;

	strcpy (FileName,"[%DL]icons\\zoomwin_tp.bmp");
	if ((hDib32 = LoadDIB32 (FileName,24)))
	{
		GetClientRect (hWnd,&rect);
		//rect.right--;
		//rect.bottom--;
		rect.left = rect.right -w;
		rect.top = rect.bottom -w;
		winzoomRect = rect;
		DisplayTransparentBitmapInRect  (hDC,hDib32,&rect,FALSE);
		DestroyDIB32(hDib32,FALSE);
	}
	if (ExistFile ("[%DL]menus\\findmenu.txt"))
		strcpy (FileName,"[%DL]icons\\find_tp.bmp");
	else
		strcpy (FileName,"[%DL]icons\\pan.bmp");
	if ((hDib32 = LoadDIB32 (FileName,24)))
	{
		GetClientRect (hWnd,&rect);
		//rect.left++;
		//rect.bottom--;
		rect.right = rect.left +w;
		rect.top = rect.bottom -w;
		findRect = rect;
		DisplayTransparentBitmapInRect(hDC, hDib32, &rect, FALSE);
		DestroyDIB32(hDib32,FALSE);
	}
	strcpy (FileName,"[%DL]icons\\cancelnew_tp.bmp");
	if ((hDib32 = LoadDIB32 (FileName,24)))
	{
		GetClientRect (hWnd,&rect);
		//rect.right--;
		//rect.top++;
		rect.left = rect.right -w;
		rect.bottom = rect.top +w;
		cancelRect = rect;
		DisplayTransparentBitmapInRect(hDC, hDib32, &rect, FALSE);
		DestroyDIB32(hDib32,FALSE);
	}
	strcpy (FileName,"[%DL]icons\\list_tp.bmp");
	if ((hDib32 = LoadDIB32 (FileName,24)))
	{
		GetClientRect (hWnd,&rect);
		//rect.left++;
		//rect.top++;
		rect.right = rect.left +w;
		rect.bottom = rect.top +w;
		listRect = rect;
		DisplayTransparentBitmapInRect(hDC, hDib32, &rect, FALSE);
		DestroyDIB32(hDib32,FALSE);
	}
	return;
}

void DisplayPZWinZoom (HDC hDC,POINT cenpt,int w)
{
	HFONT	hFont = CreateFont(10,0, 900, 0, FW_BOLD, 
    				0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
	HFONT	OldFont;
	double	rotdeg=300, rot, deginc=7;
	POINT	pt;
	int	i;
	char	text[]="Window";

	SaveDC (hDC);
	OldFont  = SelectObject (hDC,hFont);
	SetTextColor (hDC,0);
	for (i=0;i<strlen(text);i++)
	{
		rot = RADDEG * rotdeg;
		pt = newpt (cenpt,rot,w-3);
		SethDCRotation (hDC,cenpt,rot);
		TextOutWithShadow (hDC,cenpt.x+w,cenpt.y-3,&text[i],1,0,RGB(255,255,255));
		rotdeg += deginc;
	}
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	RestoreDC (hDC,-1);
	return;
}

void DisplayPanZoomRot (HWND hWnd,HDC hDC,LPRECT pRect)
{
	HFONT	hFont = CreateFont(10,0, 0, 0, FW_BOLD, 
    				0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
	HFONT	OldFont;
	//HBITMAP	hBM;
	double	az;
	BOOL	gotDC=FALSE;
	RECT	rect;
	HPEN	hPen, OldPen;
	POINT	Points[2];
	int		i;

	if (!hDC)
	{
		gotDC = TRUE;
		hDC = GetDC (hWnd);
	}
	if (pRect)
		rect = *pRect;
	else
		GetClientRect (hWnd,&rect);

	SaveDC (hDC);
	//SetDCBrushColor (hDC,RGB(0,255,0));
	FillRect (hDC,&rect,GetStockObject(WHITE_BRUSH));//LTGRAY_BRUSH));
	OldFont  = SelectObject (hDC,hFont);
/*			{
		HDIB32	hDib32 = LoadDIB32 ("..\\icons\\zoompan.bmp");
		HDIB32	hDib8 = QuantizeDib (hDib32,FIQ_NNQUANT);

		GM32SaveDIB (hDib8,"..\\icons\\zoompan8.bmp",0,0);
		DestroyDIB32(hDib8,FALSE);
		DestroyDIB32(hDib32,FALSE);
	}*/
	//hBM = LoadBitmap (hInst,"PANZOOMROT");

	DisplayBMFileInRect (hDC,"[%DL]icons\\zoompan8.bmp",rect,2);
//	DisplayBitmapInRect (hDC,rect,hBM,2,SRCCOPY);
//	DeleteObject (hBM);
    SetTextColor(hDC, RGB(0,0,128));
	DisplayZoomInOut (hWnd,hDC,0,0);
	DisplayZoomInOut (hWnd,hDC,1,0);
//	 TextOut (hDC,60,48,"Zoom In",7);
//	TextOut (hDC,57,88,"Zoom Out",8);
//			SetTextColor (hDC,RGB(255,255,255));
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	hFont = CreateFont(24,0, 0, 0, FW_BOLD, 
    					0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
	OldFont  = SelectObject (hDC,hFont);
	TextOutWithShadow (hDC,PZRMidPoint.x-35,PZRMidPoint.y-14,"-",1,1,RGB(255,255,255));
	TextOutWithShadow (hDC,PZRMidPoint.x+24,PZRMidPoint.y-13,"+",1,1,RGB(255,255,255));
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	i=-6;
	InflateRect (&rect,i,i);
	hPen = CreatePen(PS_SOLID,(int)9,RGB(196,196,224));
	OldPen = SelectObject (hDC,hPen);
	i = 4;
	Arc(hDC,rect.left+i,rect.top+i,rect.right-i,rect.bottom-i,
			rect.left+i,rect.top+i,rect.left+i,rect.top+i);
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);
	hPen = CreatePen(PS_SOLID,3,RGB(160,160,192));
	OldPen = SelectObject (hDC,hPen);
	i = 1;
	Arc(hDC,rect.left+i,rect.top+i,rect.right-i,rect.bottom-i,
			rect.left+i,rect.top+i,rect.left+i,rect.top+i);
	i = 7;
	Arc(hDC,rect.left+i,rect.top+i,rect.right-i,rect.bottom-i,
			rect.left+i,rect.top+i,rect.left+i,rect.top+i);
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);

	{
/*		RECT rect;
		int	w;
		POINT	bpt,ept,cenpt;

		GetClientRect (hWnd,&rect);
		cenpt = RectMid (&rect);
		w = RECTWIDTH(&rect)/2;
		bpt = newpt (cenpt,60*RADDEG,w);
		ept = newpt (cenpt,30*RADDEG,w);
		hPen = CreatePen(PS_SOLID,11,RGB(176,176,176));
		OldPen = SelectObject (hDC,hPen);
		i = -3;
		Arc(hDC,rect.left+i,rect.top+i,rect.right-i,rect.bottom-i,
				bpt.x,bpt.y,ept.x,ept.y);
		SelectObject (hDC,OldPen);
		DeleteObject (hPen);
		hPen = CreatePen(PS_SOLID,9,RGB(208,208,224));
		OldPen = SelectObject (hDC,hPen);
		Arc(hDC,rect.left+i,rect.top+i,rect.right-i,rect.bottom-i,
				bpt.x,bpt.y,ept.x,ept.y);
		SelectObject (hDC,OldPen);
		DeleteObject (hPen);*/
		//DisplayPZWinZoom (hDC,cenpt,w);
	}

	SethDCRotation (hDC,PZRMidPoint,Rotation);

	hPen = CreatePen(PS_SOLID,(int)3,RGB(0,0,0));
	OldPen = SelectObject (hDC,hPen);
	radius = (rect.right - rect.left)/2 + 2;
	Points[0] = Points[1] = PZRMidPoint;
	Points[0].x -= radius;
	i = 7;
	Points[1].x -= (radius - i);
	Polyline (hDC,Points,2);
	Points[0] = Points[1] = PZRMidPoint;
	Points[0].x += radius;
	Points[1].x += (radius - i);
	Polyline (hDC,Points,2);
	Points[0] = Points[1] = PZRMidPoint;
	Points[0].y -= radius;
	Points[1].y -= (radius - i);
	Polyline (hDC,Points,2);
	Points[0] = Points[1] = PZRMidPoint;
	Points[0].y += radius;
	Points[1].y += (radius - i);
	Polyline (hDC,Points,2);
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);
	hPen = CreatePen(PS_SOLID,(int)1,0);
	OldPen = SelectObject (hDC,hPen);
	for (i=0;i<12;i++)
	{
		if (!(i%3))
			continue;
		az = i*(PY/6);
		Points[0] = newpt (PZRMidPoint,az,radius-3);
		Points[1] = newpt (PZRMidPoint,az,radius-11);
		Polyline (hDC,Points,2);
	}
	SelectObject (hDC,OldPen);
	DeleteObject (hPen);
	DisplayCompassDir (hDC,-1,PZRMidPoint,Rotation);
	DisplayCompassDir (hDC,-2,PZRMidPoint,Rotation);
	DisplayCompassDir (hDC,-3,PZRMidPoint,Rotation);
	DisplayCompassDir (hDC,-4,PZRMidPoint,Rotation);
	RestoreDC (hDC,-1);
	{
		RECT rect;

		DestroySavedScreen (&hSavePZRScreenNoIcons,0);
		GetClientRect (hWnd,&rect);
		hSavePZRScreenNoIcons = SaveScreen2 ((HWND)-1,hDC,rect,0,0);
	}
	SetPZMaskBytes (hWnd,hDC);
	DestroySavedScreen (&hSavePZRScreen,0);
	DestroySavedScreen (&hSavePZRScreen2,0);
	GetClientRect (hWnd,&rect);
	hSavePZRScreen = SaveScreen2 ((HWND)-1,hDC,rect,0,0);
	
	SelectClipRgn (hDC,hRgnJoyStick);
	FillRect (hDC,&JoyStickRect,GetStockObject (BLACK_BRUSH));
	SelectClipRgn (hDC,0);

	hSavePZRScreen2 = SaveScreen2 ((HWND)-1,hDC,rect,0,0);
	DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
	if (gotDC)
		ReleaseDC (hWnd,hDC);
	return;
}

void DisplayPZRotImage (int Which)
{
	HDC	hDC, hDCPZ;
	RECT	MainRect,PZRectInMain,PZClientRect,MainClientRect;
	LPVIEWPORT	SaveVP = CurView;
	HBITMAP	hTempBM, hBufBM, hBMPZ, hBMPZOld;
	int	ix,iy, xoff, yoff;
	COLORREF	Color;
	POINT	pt;
	int BlendFactor = 0;
	int	w,h,ToolbarID;
	HRGN	hRgn=0;
static	int ii=0;
	BOOL	isMoving,showIcons=TRUE;// haveRgn=FALSE ;
//	HRGN hrgnx = CreateRectRgn(0,0,0,0);
//	int regionType = GetWindowRgn(hWndPZR, hrgn);
	
//	if (regionType != ERROR)
//		haveRgn = TRUE;
//	DeleteObject(hrgn); 

	if (!hWndPZR)
		return;

	ToolbarID = GetToolbarIDFromWnd (hWndPZR);
	GetWindowRect (hWndMain,&MainRect);
	GetClientRect (hWndMain,&MainClientRect);
	GetWindowRect (hWndPZR,&PZRectInMain);
	GetCursorPos (&pt);
	GetClientRect (hWndPZR,&PZClientRect);
	w = RECTWIDTH(&PZClientRect);
	h = RECTHEIGHT(&PZClientRect);
	if (w <= 0 || h <= 0)
		return;
	ScreenToClient (hWndPZR,&pt);
	isMoving = GetCapture () == hWndPZR;

	if (ToolbarTTWindow[ToolbarID])
		SendMessage(ToolbarTTWindow[ToolbarID], TTM_ACTIVATE, (WPARAM)!isMoving, (LPARAM)&toolItem[ToolbarID]);

	/*if (isMoving)
	{
	    if (g_hwndTrackingTT)
			SendMessage(g_hwndTrackingTT, TTM_ACTIVATE, (WPARAM)FALSE, (LPARAM)&g_toolItem);
		if (!haveRgn)
		{
extern	BOOL	InDebug;
			BOOL saveInDebug = InDebug;

			InDebug = FALSE;
			hRgn = CreateEllipticRgn(0,0,w,h);
			InDebug = saveInDebug;
			ii+=SetWindowRgn(hWndPZR,hRgn,FALSE);
			if (ii>100)
				ii=0;
			ignoreMM = TRUE;
		}
	}
	else if (haveRgn)
		SetWindowRgn(hWndPZR,0,FALSE);*/

	CurView = PZR_VP;
	hDCPZ = GetDC (hWndPZR);
	hDC = CreateCompatibleDC (hDCPZ);
	hBMPZ = CreateCompatibleBitmap (hDCPZ,w,h);
	hBMPZOld = SelectObject (hDC,hBMPZ);
	if (Which == 2)
		RestoreScreen2 (hDC, hSavePZRScreen2,0,FALSE);
	else if (!ToolbarFloating[ToolbarID] || PtInRect (&PZClientRect,pt))
		RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
	else
	{
		RestoreScreen2 (hDC, hSavePZRScreenNoIcons,0,FALSE);
		BlendFactor = AlphaBlendFactor;
		showIcons = FALSE;
	}
	//hDCMain = GetDC (hWndMain);
	{
		HDC	hDCBuf;
		HBITMAP	hBMBuf, hOldBMBuf=0;
		HWND	hWndDT = GetDesktopWindow ();

		ScreenRectToClientRect (hWndMain,&PZRectInMain);
		if (hDCPZMoveImage)
		{
			GetWindowRect (hWndPZR,&PZRectInMain);
			ScreenRectToClientRect (hWndDT,&PZRectInMain);
			xoff = PZRectInMain.left;
			yoff = PZRectInMain.top;
			hDCBuf = hDCPZMoveImage;
		}
		else if ((BufferedScreen && hDCScreenBuffer) && RectCompletelyInRect(&CurView->ScreenRect,&PZRectInMain))
		{
			xoff = PZRectInMain.left;
			yoff = PZRectInMain.top;
			hDCBuf = hDCScreenBuffer;
		}
		else
		{
			HDC hDCMain = GetDC (hWndMain);
			int	w=RECTWIDTH(&PZClientRect),h=RECTHEIGHT(&PZClientRect);

			BlendFactor = 0;
			xoff = 0;
			yoff = 0;
			hDCBuf = CreateCompatibleDC(hDCScreenBuffer);
			hBMBuf = CreateCompatibleBitmap (hDC,w,h);
			hOldBMBuf = SelectObject (hDCBuf,hBMBuf);
			ReleaseDC (hWndMain,hDCMain);
			FillRect (hDCBuf,&PZClientRect,GetStockObject (WHITE_BRUSH));
		}
		if (hDCBuf)
		{
			int	widthPZ=RECTWIDTH (&PZClientRect), heightPZ=RECTHEIGHT (&PZClientRect);
	//		BITMAP	bm;
		    float fAlphaFactor;    // used to do premultiply 
			BLENDFUNCTION bf;
    // create a DC for our bitmap -- the source DC for AlphaBlend  
			HDC	hdc = CreateCompatibleDC(hDCBuf);
			BITMAPINFO	bmi;
			RGBQUAD	*pvBits, *pBits;
			LPBYTE	pmaskBits;
			HBITMAP	hbitmap, hbmold;
			UINT	ir,ic;
			int		ubAlpha=255;
    // zero the memory for the bitmap info 
			ZeroMemory(&bmi, sizeof(BITMAPINFO));

			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = widthPZ;
			bmi.bmiHeader.biHeight = heightPZ;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = widthPZ * heightPZ * 4;
            fAlphaFactor = (float)ubAlpha / (float)0xff; 

			// create our DIB section and select the bitmap into the dc 
			hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
			if (hbitmap)
			{
				POINT midpoint = { widthPZ / 2, heightPZ / 2 }, p;
				double radius = widthPZ / 2;
				hbmold = SelectObject(hdc, hbitmap);
				BitBlt (hdc,0,0,widthPZ,heightPZ,hDCBuf,xoff,yoff,SRCCOPY);
				for (ir = 0, pBits = pvBits, pmaskBits = maskBits[0]; ir < heightPZ; ir++)
				{
					for (ic = 0; ic < widthPZ; ic++, pBits++, pmaskBits++)
					{
						p.x = ic;
						p.y = ir;
						if (idist(midpoint, p) <= radius)
						{

							pBits->rgbReserved = max(BlendFactor, *pmaskBits);
							fAlphaFactor = (float)pBits->rgbReserved / (float)0xff;

							pBits->rgbBlue *= fAlphaFactor;
							pBits->rgbRed *= fAlphaFactor;
							pBits->rgbGreen *= fAlphaFactor;
						}
					}
				}
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.AlphaFormat = AC_SRC_ALPHA;
				bf.SourceConstantAlpha =  0xFF;//AlphaBlendFactor;///
				AlphaBlend(hDC,0,0,widthPZ,heightPZ, 
							hdc,0,0,widthPZ,heightPZ,bf);
				SelectObject (hdc,hbmold);
				GSSiDeleteObject(&hbitmap);
				DeleteDC(hdc);
				if (hOldBMBuf)
				{
					SelectObject (hDCBuf,hOldBMBuf);
					DeleteDC (hDCBuf);
					GSSiDeleteObject (&hBMBuf);
				}
			}
			else if (hdc)
				DeleteDC (hdc);
		}
	}
	if (showIcons)
	{
		if (isMoving)
			ii=1;
		else
			DisplayPZIcons (hWndPZR,hDC);
	}
	if (hDC && hDCPZ && hBMPZOld)
	{
		BitBlt (hDCPZ,0,0,w,h,hDC,0,0,SRCCOPY);
		SelectObject (hDC,hBMPZOld);
		DeleteDC (hDC);
		GSSiDeleteObject (&hBMPZ);
		ReleaseDC (hWndPZR,hDCPZ);
	}
	CurView = SaveVP;
	return;
}

LONG FAR PASCAL PanZoomRotWndProc(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{
 HMENU      hMenu=0;            /* handle for the menu                 */
 HBITMAP    hBitmap=0;          /* handle for bitmaps                  */                              
 HDC        hDC;                /* handle for the display device       */
 PAINTSTRUCT ps;                /* holds PAINT information             */
 int        nRc=0;              /* return code                         */
 static		POINT		CursorPoint;
 RECT		rect;
 BOOL		rc;
 HPEN		hPen,OldPen;
 HBRUSH		hBrush, OldBrush;
 POINT		Points[8];
 int		radius, i,ii;
 char		str[128];
 char	PreDir[4],DegC[16],MinC[16],SecC[16],PostDir[4], BearingC[64];
 static		double		az, az2, dist=0;
 static		COLORREF	iColor;
			COLORREF	VHMoveColor=15953229, RED=RGB(255,0,0), DKRED=RGB(128,0,0), RED2=RGB(0,192,0);
 static		int	ZoomIO, JumpIO, RotDIR;
 static		double	PanAZ;
 static		int	zDelta = 5;
 static		int	TimerID=0,TimerInterval=1;
 static		int	InOut;
 static		POINT	LastMovePoint;
 static		BOOL	HaveTrackMouseEvent, InJoyStick, FirstPaint;
 static		double	ZoomScale;
 static		HANDLE	hSavedScreen=0;
 static		long	ScreenID;
 static		int		slidex,slidey, slidexinc, slideyinc;
 static		POINT	DownPoint, NewDownPoint, lastPoint;
 DPOINT		BasePointDown, BasePointUp;
 LPVIEWPORT	SaveVP = CurView;
 short		SaveConfig = CurrentConfig;
 DPOINT	ScreenPoints[5];
 DPOINT	DirPoints[2];
 DPOINT	IntPoint;
 double	IntDist[3],InAZ,OutAZ[3]; 
 short	WhichPoly[3];
 BOOL	OutReverse[3];
 int		n;
 POINT	ScreenPt;
 int	ToolbarID = GetToolbarIDFromWnd (hWnd);
 static BOOL AllowRotate = TRUE;
 static int	PANDIST = 53;


 if (Message != WM_CREATE)
	 CurView = PZR_VP;
 if (!CurrentConfig)
	 SetConfig (1);
 switch (Message)
   { 
    case WM_CREATE:
		{
			HANDLE	hCoords;
			double	Radius;
			DPOINT	WindowCenter;
			LPDPOINT	Points;
HRGN	hRgn;
			RECT	Rect;

			PZR_VP = CurView;
			AllowRotate = GetGlobalBVal2("[%ALLOWPZROTATE]", TRUE);
			if (!AllowRotate)
				PANDIST = 64;
/*			hCoords = CreateCirclePoly (WindowCenter,Radius,&nPnts,1);
			DPolyToPPoly (&nPnts, hCoords,0);
			Points = (HPDPOINT)GlobalLock (hCoords);
			GSSiGlobFree (&hCoords);*/
			GetClientRect (hWnd,&Rect);
			PZRMidPoint = RectMid (&Rect);
			JoyStickRect.left = PZRMidPoint.x-15;
			JoyStickRect.right = PZRMidPoint.x+15;
			JoyStickRect.top = PZRMidPoint.y-15;
			JoyStickRect.bottom = PZRMidPoint.y+15;

			hRgnJoyStick = CreateEllipticRgn(JoyStickRect.left,JoyStickRect.top,JoyStickRect.right,JoyStickRect.bottom);
			InJoyStick = FALSE;
//SetWindowRgn(hWnd,hRgn,FALSE);
			HaveTrackMouseEvent = FALSE;
			Rotation = CurView->Rotation;
			GetWindowRect (hWnd,&Rect);
			CurView->PanZoomControlPoint = RectMid (&Rect);
			CurView->UsePanZoomControl = TRUE;
			hWndPZR = hWnd;
			FirstPaint = TRUE;
			g_hwndTrackingTT = 0;//CreateTrackingToolTip(IDC_BUTTON1, hWnd, "");
			lastPoint.x = lastPoint.y = SHRT_MAX;
			*lastPrompt = 0;
     	}
         break;       /*  End of WM_CREATE                              */

	case WM_NOTIFY:
		 return OnWMNotify(hWnd,lParam);

    case WM_CLOSE:
		CurView->UsePanZoomControl = FALSE;
		DestroyWindow (hWnd);
		break;

	case WM_DESTROY:
		{
			int	ToolbarID = GetToolbarIDFromWnd (hWnd);

			DeleteObject (hRgnJoyStick);
    		DestroySavedScreen (&hSavedScreen,ScreenID);
			DestroySavedScreen (&hSavePZRScreen,0);
			DestroySavedScreen (&hSavePZRScreen2,0);
			DestroySavedScreen (&hSavePZRScreenNoIcons,0);
			KillTimer (hWnd,TimerID);
			
			if (ToolbarID > -1)
			{
				DestroyToolbar(ToolbarID);
				RemoveToolbar (ToolbarID,TRUE);
			}
			hWndPZR = 0;
		}
		break;
    case WM_MOVE:     /*  code for moving the window                    */
         break;
    
    case WM_SIZE:     /*  code for sizing client area                   */
         break;       /* End of WM_SIZE                                 */
	
	case WM_LBUTTONDBLCLK:
		break;

	case WM_MOUSEWHEEL:
	{
		int		fwKeys = LOWORD(wParam);    // key flags
		short	zDelta = (short) HIWORD(wParam);    // wheel rotation
		
		WheelZoom (zDelta,0,1);		
		break;
	}

	case WM_TIMER:
		switch (wParam)
		{
		case GF_WHEELZOOM:
			WheelZoom (zDelta*InOut,1,ZoomScale);
			break;
		case GF_SLIDE_SCREEN:
			slidex -= slidexinc;
			slidey -= slideyinc;
			MoveScreen (slidex,slidey,hSavedScreen,ScreenID);
			NewDownPoint.x = DownPoint.x + slidex;
			NewDownPoint.y = DownPoint.y + slidey;
			break;
		}
		break;

/*	case WM_RBUTTONDOWN:
    	LastMovePoint = POINTStoPOINT(MAKEPOINTS(lParam));
		ClientToScreen (hWnd,(LPPOINT)&LastMovePoint);
		SetCapture (hWnd);
		break;

	case WM_RBUTTONUP:
		{
			POINT pt;

			GetCursorPos (&pt);
			ReleaseCapture ();
			SeeIfToolbarShouldBeDocked (hWnd,&pt);
		}
		break;
*/
	case WM_MBUTTONUP:
		ShowWindow (hWnd,SW_HIDE);
		break;
	case WM_LBUTTONUP:
		hDC = GetDC (hWnd);
		KillTimer (hWnd,TimerID);
    	CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
    	DestroySavedScreen (&hSavedScreen,ScreenID);
		if (InJoyStick)
		{
			DPOINT BasePointDown = ScreenPtToBasePt (DownPoint);
			DPOINT BasePointUp = ScreenPtToBasePt (NewDownPoint); 
			double xmove = BasePointDown.x - BasePointUp.x;
			double ymove = BasePointDown.y - BasePointUp.y;

			CurView->MidPointW.x += xmove;
			CurView->MidPointW.y += ymove;
    		DestroySavedScreen (&hSavedScreen,ScreenID);
            PostMessage(CurView->hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);
			//ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);
			InJoyStick = FALSE;
			break;
		}

		if (ZoomIO)
		{
			ZoomIO = 0;
			WheelZoom (0,1,ZoomScale);
		}
		else if (PtInRect (&winzoomRect,CursorPoint))
			ProcessText ("$CMD(GF_WINDOW_ZOOM,Primary Viewport)");
		else if (PtInRect (&listRect,CursorPoint))
			ProcessText ("$CMD(IDM_ZOOM_LIST,Primary Viewport)");
		else if (PtInRect (&cancelRect,CursorPoint))
			PostMessage (hWnd,WM_CLOSE,0,0);
		else if (PtInRect (&findRect,CursorPoint))
		{
			if (ExistFile ("[%DL]menus\\findmenu.txt"))
				ProcessText ("$MENU([%DL]menus\\findmenu.txt)");
			else
				ProcessText ("$CMD(GF_SLIDE_SCREEN,Primary Viewport)");
		}
		else if (RotDIR)
		{
			switch (RotDIR)
			{
			case 1:
				CurView->Rotation = 0;
				break;
			case 2:
				CurView->Rotation = HALFPI;
				break;
			case 4:
				CurView->Rotation = PY;
				break;
			case 3:
				CurView->Rotation = PIHALF;
				break;
			}
			Rotation = CurView->Rotation;
			DisplayPanZoomRot (hWnd,0,0);
			RedisplayViewport(FALSE,FALSE);
		}
		else if (PanAZ)
		{

			DirPoints[0] = PointToDPoint (RectMid (&CurView->ScreenRect));
			DirPoints[1] = dnewpt (DirPoints[0],PanAZ,5000);

			RectToDPoints (&CurView->ScreenRect,ScreenPoints);
			ScreenPoints[4] = ScreenPoints[0];
			n = IntersectPolys2 (2,DirPoints,
								 5,ScreenPoints,
								 0,IntDist,&IntPoint,&InAZ,
								 OutAZ,OutReverse,WhichPoly,FALSE);
			ScreenPt = DPointToPoint (IntPoint);
			CurView->MidPointW = ScreenPtToBasePt (ScreenPt);
            PostMessage(CurView->hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);
			//ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);
		}
		else if (JumpIO == 2)
			PostMessage (hWndMain,WM_COMMAND,IDM_Z_IN,MAX_VIEWPORTS+1);
		else if (JumpIO == 1)
			PostMessage (hWndMain,WM_COMMAND,IDM_Z_OUT,MAX_VIEWPORTS+1);
		else if (AllowRotate && dist > 53)
		{
			CurView->Rotation += (az+HALFPI);
			CurView->Rotation = LTWOPI (CurView->Rotation);
			Rotation = CurView->Rotation;
			DisplayPanZoomRot (hWnd,0,0);
			RedisplayViewport(FALSE,FALSE);
		}
		else if (iColor == DKRED)
		{
			HBRUSH	hBrush = CreateSolidBrush (RED);
			HBRUSH	hOldBrush = SelectObject (hDC,hBrush);

			rc = ExtFloodFill (hDC,CursorPoint.x,CursorPoint.y,DKRED,FLOODFILLSURFACE);

			SelectObject (hDC,hOldBrush);
			GSSiDeleteObject (&hBrush);
			iColor = RED;
			if (CursorPoint.y < 50)
				PostMessage (hWndMain,WM_COMMAND,IDM_P_UP,0);
			else if (CursorPoint.y > 100)
				PostMessage (hWndMain,WM_COMMAND,IDM_P_DOWN,0);
			else if (CursorPoint.x > 100)
				PostMessage (hWndMain,WM_COMMAND,IDM_P_RIGHT,0);
			else if (CursorPoint.x < 50)
				PostMessage (hWndMain,WM_COMMAND,IDM_P_LEFT,0);
		}
		ReleaseDC (hWnd,hDC);
		lastPoint.x = INT_MAX;
		goto CursorMove;
		break;

	case WM_MOUSELEAVE:
		{
		int	ToolbarID = GetToolbarIDFromWnd (hWndPZR);
		hDC = GetDC (hWnd);
		HaveTrackMouseEvent = FALSE;
		KillTimer (hWnd,TimerID);
		ReleaseDC (hWnd,hDC);
		DisplayPZRotImage (1);
		SetSysMess ("");
	    SendMessage(ToolbarTTWindow[ToolbarID], TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&toolItem[ToolbarID]);
		lastPoint.x = lastPoint.y = SHRT_MAX;
		*lastPrompt = 0;
		}
		break;

     case GSSI_REINITDIALOG:
		{
			POINT	pt=ToolBarStartPoint;
			int	ToolbarID = GetToolbarIDFromWnd (hWnd);

			if (ToolbarID < 0)
				break;
			ClientToScreen (hWndMain,(LPPOINT)&pt);
			GetWindowRect (hWnd,&rect);
//			MoveWindow(hWnd,pt.x,pt.y,
//							rect.right-rect.left,rect.bottom-rect.top,TRUE);
			SetWindowPos (hWnd,0,pt.x,pt.y,
							rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
			GetWindowRect (hWnd,&ToolbarRect[ToolbarID]);
			CurView->PanZoomControlPoint = RectMid (&rect);
			break;
		}

	case WM_LBUTTONDOWN:
	    HaltMapDisplay (FALSE,TRUE);
		SetSysMess (0);
	case WM_MOUSEMOVE:
		ToolbarID = GetToolbarIDFromWnd (hWndPZR);
		if (!AllowRotate)
			RotDIR = 0;
		if (ignoreMM)
		{
			ignoreMM = FALSE;
			break;
		}
		{
			MovePromptMessage (hWnd,lParam);
			if (!wParam)
				PanAZ = RotDIR = JumpIO = ZoomIO = 0;
			if (!HaveTrackMouseEvent)
			{
				TRACKMOUSEEVENT EventTrack;

				EventTrack.dwFlags = TME_LEAVE;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				EventTrack.hwndTrack = hWnd;
				EventTrack.dwHoverTime = 0;
				TrackMouseEvent(&EventTrack);
				HaveTrackMouseEvent = TRUE;
		        //SendMessage(g_hwndTrackingTT, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&g_toolItem);
			}
CursorMove:
    		CursorPoint = POINTStoPOINT(MAKEPOINTS(lParam));
			if (Message != WM_LBUTTONDOWN && CursorPoint.x == lastPoint.x && CursorPoint.y == lastPoint.y)
				break;
			lastPoint = CursorPoint;
        
			if (GetLastPrompt (lastPrompt))
			{
				toolItem[ToolbarID].lpszText = lastPrompt;
				SendMessage(ToolbarTTWindow[ToolbarID], TTM_UPDATETIPTEXT, 0, (LPARAM)&toolItem[ToolbarID]);
			}
		//	SendMessage(g_hwndTrackingTT, TTM_SETTOOLINFO, 0, (LPARAM)&g_toolItem);
        // Position the tooltip. The coordinates are adjusted so that the tooltip does not overlap the mouse pointer.
        
			{
				POINT pt = CursorPoint; 
				ClientToScreen(hWnd, &pt);
				//SendMessage(g_hwndTrackingTT, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(pt.x + 10, pt.y - 20));
			}
			GetClientRect (hWnd,&rect);
			PZRMidPoint = RectMid (&rect);
			dist = idist (CursorPoint,PZRMidPoint);
			az = getaz (PZRMidPoint,CursorPoint);
			az2 = LTWOPI (az + Rotation);
//			sprintf (str,"%i,%i",CursorPoint.x,CursorPoint.y);
//			SetWindowText (hWndMain,str);
/*			if (Message != WM_LBUTTONDOWN && wParam == MK_RBUTTON)
			{
				POINT	LMP = LastMovePoint;
				GetWindowRect (hWnd,&rect);
				ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
				LastMovePoint = CursorPoint;
				MoveWindow(hWnd,rect.left+(CursorPoint.x-LMP.x),
								rect.top+(CursorPoint.y-LMP.y),
								rect.right-rect.left,rect.bottom-rect.top,TRUE);
				ToolbarID = GetToolbarIDFromWnd (hWnd);
				if (ToolbarID < 0)
					break;
				GetWindowRect (hWnd,&ToolbarRect[ToolbarID]);
				CurView->PanZoomControlPoint = RectMid (&rect);
				break;
			}*/
			hDC = GetDC (hWnd);
			if (!hDC)
				ii=1;
			if (Message != WM_LBUTTONDOWN && wParam == MK_LBUTTON && InJoyStick)
			{
				if (dist > 31)
				{
					CursorPoint = LastMovePoint;
					ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
					SetCursorPos(CursorPoint.x,CursorPoint.y);
				}
				else
				{
					LastMovePoint = CursorPoint;
					DisplayJoyStick (hDC,CursorPoint);
				}
				slidexinc = (LastMovePoint.x - PZRMidPoint.x)*JoyStickSpeed;
				slideyinc = (LastMovePoint.y - PZRMidPoint.y)*JoyStickSpeed;
				ReleaseDC (hWnd,hDC);
				break;
			}
			iColor = GetPixel (hDC,CursorPoint.x,CursorPoint.y);
			if (iColor != DKRED && iColor != RED)
			{
				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
				iColor = GetPixel (hDC,CursorPoint.x,CursorPoint.y);
			}
//			sprintf (str,"%f, %f, %i, %i, %i",dist,az2,iColor,CursorPoint.x,CursorPoint.y);
//			SetWindowText (hWndMain,str);
			InJoyStick = FALSE;
			if (PtInRect (&winzoomRect,CursorPoint))
				SetSysMess ("Click here to invoke window zoom");
			else if (PtInRect (&listRect,CursorPoint))
				SetSysMess ("Click here to select pre-defined zoom from list");
			else if (PtInRect (&cancelRect,CursorPoint))
				SetSysMess ("Click here to cancel Pan/Zoom control");
			else if (PtInRect (&findRect,CursorPoint))
			{
				if (ExistFile ("[%DL]menus\\findmenu.txt"))
					SetSysMess ("Click here to select Find option");
				else
					SetSysMess ("Click here to pan the map");
			}
			else if (dist < 15 && !ZoomIO)
			{
				if (wParam == MK_LBUTTON)
				{
					DisplayJoyStick (hDC,CursorPoint);
				}
				else
				{
					DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
					SetSysMess ("Click here to move joystick");
				}
				InJoyStick = TRUE;
			}
			else if (ZoomIO && wParam == MK_LBUTTON)
				ZoomScale = (106 - (double)min (105,CursorPoint.x))/5;
			else if ((dist < 34 && dist > 25) && (az < 0.1325 || az > 6.07))
			{
				JumpIO = 2;
				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
				SelectZoomInOut (hDC,JumpIO);
				SetSysMess ("Click here to zoom in by a factor of 2");
			}
			else if ((dist < 34 && dist > 25) && (az < 3.36 && az > 2.98))
			{
				JumpIO = 1;
				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
				SelectZoomInOut (hDC,JumpIO);
				SetSysMess ("Click here to zoom out by a factor of 2");
			}
			else if (dist < 31)
			{
				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
				DisplayZoomInOut (hWnd,hDC,CursorPoint.y<PZRMidPoint.y,RED);
				if (CursorPoint.y<PZRMidPoint.y)
					SetSysMess ("Click here to zoom in");
				else
					SetSysMess ("Click here to zoom out");
			}
			else if (AllowRotate)
			{
				if (az2 > 4.6 && az2 < 4.8 && dist > 53 && dist < 62)
				{
					DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
					RotDIR = 1;
					DisplayCompassDir(hDC, RotDIR, PZRMidPoint, Rotation);
					SetSysMess("Click here to rotate map to North");
				}
				else if ((az2 > 6.2 || az2 < 0.1) && dist > 53 && dist < 62)
				{
					DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
					RotDIR = 2;
					DisplayCompassDir(hDC, RotDIR, PZRMidPoint, Rotation);
					SetSysMess("Click here to rotate map to East");
				}
				else if (az2 > 1.48 && az2 < 1.79 && dist > 53 && dist < 62)
				{
					DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
					RotDIR = 4;
					DisplayCompassDir(hDC, RotDIR, PZRMidPoint, Rotation);
					SetSysMess("Click here to rotate map to South");
				}
				else if (az2 > 3 && az2 < 3.2 && dist > 53 && dist < 62)
				{
					DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
					RotDIR = 3;
					DisplayCompassDir(hDC, RotDIR, PZRMidPoint, Rotation);
					SetSysMess("Click here to rotate map to West");
				}
				else if (dist > 53 && dist < 65)
				{
					DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
					//DisplayPanZoomRot (hWnd,0,&rect);
					hPen = CreatePen(PS_SOLID, (int)1, RGB(160, 0, 0));
					OldPen = SelectObject(hDC, hPen);
					hBrush = CreateSolidBrush(RGB(160, 0, 0));
					OldBrush = SelectObject(hDC, hBrush);
					radius = (rect.right - rect.left) / 2 + 2;
					Points[0] = newpt(PZRMidPoint, az, radius - 12);
					Points[1] = newpt(PZRMidPoint, az, radius - 1);
					Points[2] = newpt(PZRMidPoint, az, radius - 6);
					Points[3] = newpt(Points[2], az - HALFPI, 3);
					Points[4] = newpt(Points[2], az + HALFPI, 3);
					Points[5] = Points[1];
					Points[6] = Points[3];
					Polyline(hDC, Points, 7);
					Polygon(hDC, &Points[3], 4);
					SelectObject(hDC, OldPen);
					DeleteObject(hPen);
					SelectObject(hDC, OldBrush);
					DeleteObject(hBrush);
					AZToBear(TWOPI - az, PreDir, DegC, MinC, SecC, PostDir);
					sprintf(BearingC, "%s %s %s %s %s", PreDir, DegC, MinC, SecC, PostDir);
					sprintf(str, "Click here to rotate map to %s", BearingC);
					SetSysMess(str);
				}
			}
			else if (iColor == VHMoveColor || iColor == RED)
			{
				*str = 0;
				if (iColor == VHMoveColor)
				{
					HBRUSH	hBrush = CreateSolidBrush (RED);
					HBRUSH	hOldBrush = SelectObject (hDC,hBrush);
					
					rc = ExtFloodFill (hDC,CursorPoint.x,CursorPoint.y,VHMoveColor,FLOODFILLSURFACE);
					SelectObject (hDC,hOldBrush);
					GSSiDeleteObject (&hBrush);
				}
				if (CursorPoint.y < 50)
					strcpy (str,"Click here to pan 1/2 screen up");
				else if (CursorPoint.y > 100)
					strcpy (str,"Click here to pan 1/2 screen down");
				else if (CursorPoint.x > 100)
					strcpy (str,"Click here to pan 1/2 screen right");
				else if (CursorPoint.x < 50)
					strcpy (str,"Click here to pan 1/2 screen left");
				SetSysMess (str);
			}
			else if (dist > 34 && dist < PANDIST && iColor != DKRED)
			{
				HBRUSH	hBrush = CreateSolidBrush (RED2);
				HBRUSH	OldBrush = SelectObject (hDC,hBrush);
				HPEN	OldPen = SelectObject (hDC,GetStockObject (NULL_PEN));

				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
				Points[0] = newpt (PZRMidPoint,az,36);
				Points[1] = newpt (Points[0],az-HALFPI,10);
				Points[2] = newpt (Points[0],az+HALFPI,10);
				Points[0] = newpt (PZRMidPoint,az,48);
				Polygon (hDC,Points,3);
				SelectObject (hDC,OldBrush);
				SelectObject (hDC,OldPen);
				GSSiDeleteObject (&hBrush);
				PanAZ = az;
				AZToBear (TWOPI-PanAZ,PreDir,DegC,MinC,SecC,PostDir); 
				sprintf (BearingC,"%s %s %s %s %s",PreDir,DegC,MinC,SecC,PostDir);
				sprintf (str,"Click here to pan map 1/2 screen %s",BearingC);
				SetSysMess (str);
			}
			else if (iColor != RED && iColor != DKRED)
				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
			ReleaseDC (hWnd,hDC);
			if (JumpIO || Message != WM_LBUTTONDOWN)
				break;

			hDC = GetDC (hWnd);
			DownPoint = CursorPoint;

			if (InJoyStick)
			{
				CursorPoint = RectMid (&JoyStickRect);
				DisplayJoyStick (hDC,CursorPoint);
				ClientToScreen (hWnd,(LPPOINT)&CursorPoint);
				SetCursorPos(CursorPoint.x,CursorPoint.y);
				if (BufferedScreen && hDCScreenBuffer)
					hSavedScreen = SaveScreen2 (CurView->hWnd,hDCScreenBuffer,CurView->ScreenRect,CurView,&ScreenID); 
				else
					hSavedScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,CurView->ScreenRect,CurView,&ScreenID); 
				KillTimer (hWnd,TimerID);
				TimerID = SetTimer(hWnd, GF_SLIDE_SCREEN,TimerInterval, (TIMERPROC) 0);
				slidex = slidey = slidexinc = slideyinc = 0;
			}
			else if (dist < 31)
			{
				//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
				ZoomIO = DisplayZoomInOut (hWnd,hDC,CursorPoint.y<PZRMidPoint.y,RED);
				if (ZoomIO == 2)
					InOut = 1;
				else
					InOut = -1;
				ZoomScale = (106 - (double)min(105,CursorPoint.x))/5;
				WheelZoom (zDelta*InOut,1,ZoomScale);
				KillTimer (hWnd,TimerID);
				TimerID = SetTimer(hWnd, GF_WHEELZOOM,TimerInterval, (TIMERPROC) 0);
			}
			else if (iColor == RED)
			{
				HBRUSH	hBrush = CreateSolidBrush (DKRED);
				HBRUSH	hOldBrush = SelectObject (hDC,hBrush);

				rc = ExtFloodFill (hDC,CursorPoint.x,CursorPoint.y,RED,FLOODFILLSURFACE);

				SelectObject (hDC,hOldBrush);
				GSSiDeleteObject (&hBrush);
				iColor = DKRED;
			}
			ReleaseDC (hWnd,hDC);
		}

		break;

    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
			memset(&ps, 0x00, sizeof(PAINTSTRUCT));
			hDC = BeginPaint(hWnd, &ps);
			if (hSavePZRScreen)
				DisplayPZRotImage(1);//RestoreScreen2 (hDC, hSavePZRScreen,0,FALSE);
			else
				DisplayPanZoomRot (hWnd,hDC,&ps.rcPaint);
		 if (FirstPaint)
		 {
			HDC	hDC, hDCMain = GetDC (CurView->hWnd);

			BufferedScreen = TRUE;
    		hDC = hDCMain;	
			DestroyVehicles (CurView->ID);
			ClearMeterPrompts (hDC);
			hDC = ScreenBufferDC (CurView->hWnd,hDC);
			ReleaseDC (CurView->hWnd,hDCMain); 
		 }
		 FirstPaint = FALSE;
         EndPaint(hWnd, &ps);
         break;       /*  End of WM_PAINT                               */

    case WM_COMMAND:
    {         
    	switch (wParam)
    	{
        }
        break;   	 
    			
    }
    	break;
    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing.                             */
		 SetConfig (SaveConfig);
		 CurView = SaveVP;
         return DefWindowProc(hWnd, Message, wParam, lParam);
   } 
   SetConfig (SaveConfig);
   CurView = SaveVP;
 return 0L;
}     /* End of WndProc                                         */

BOOL RegisterPanZoomRotClass(void)
{
    WNDCLASS  wc; 
    static	Called=FALSE;

    if (Called) return TRUE;
    Called = TRUE;
	wc.style = CS_OWNDC | CS_DBLCLKS | CS_SAVEBITS;
    wc.lpfnWndProc = (WNDPROC)PanZoomRotWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = "PanZoomRotWndClass";

    return (RegisterClass(&wc));
}

BOOL CreatePanZoomRotTool(HWND hWnd, POINT Center, BOOL adjustToWindow)
{
	RECT	Rect;
	int		Id;
	BOOL	st;

	if (!hWnd)
	{
		if (!hWndPZR)
			return FALSE;
		PostMessage (hWndPZR,WM_CLOSE,0,0);
		return TRUE;
	}
	if (hWndPZR)
		return FALSE;
	if (adjustToWindow)
	{
		GetClientRect(hWnd, &Rect);
		if (!PtInRect(&Rect, Center))
		{
			POINT	Points[4];
			UINT	i, Mini = 0;
			double	MinDist, d;

			RectToPoints(&Rect, Points);
			MinDist = idist(Center, Points[0]);
			for (i = 1; i < 4; i++)
			{
				d = idist(Center, Points[i]);
				if (d < MinDist)
				{
					MinDist = d;
					Mini = i;
				}
			}
			switch (Mini)
			{
			case 1:
				Center.x = Rect.left + 75;
				Center.y = Rect.top + 75;
				break;
			case 2:
				Center.x = Rect.right - 75;
				Center.y = Rect.top + 75;
				break;
			case 3:
				Center.x = Rect.right - 75;
				Center.y = Rect.bottom - 75;
				break;
			case 0:
				Center.x = Rect.left + 75;
				Center.y = Rect.bottom - 75;
				break;
			}

		}
	}
	RegisterPanZoomRotClass();
	if (!(hwndPanZoomRot = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED,
	    "PanZoomRotWndClass",
	    "PanZoomRot",
		WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN ,
	    Center.x-65, Center.y-65, 131, 131, 
	    hWnd,	/* parent */
	    0,	/* no menu */
	    hInst,
	    0)))
    {
		return FALSE;
    }
	st = SetLayeredWindowAttributes(hwndPanZoomRot, RGB(255, 255, 255), 0, LWA_COLORKEY);
	Id = LoadToolbar(hwndPanZoomRot, "", "ZOOM", 0, 1, "0 0", TRUE, FALSE, 0, 0, 0);
	DisplayAllToolbars (2);

	return TRUE;
}

BOOL SetToolbarHoverCmd(int toolbarId,LPSTR cmd)
{
	int ToolbarID = GetToolbarIDFromId(toolbarId);

	if (ToolbarID >= 0)
		strncpy(ToolbarHoverCmd[ToolbarID], cmd, MAX_TOOLBAR_HOVERCMD);
	return TRUE;
}

int GetToolbarIDFromWnd(HWND hWndDlg)
{
	int	i;

	for (i = 0; i<nToolbars; i++)
	{
		if (hWndDlg == ToolbarWindow[i])
			return i;
	}
	return -1;
}
int GetToolbarIDFromId(int Id)
{
	int	i;

	for (i = 0; i<nToolbars; i++)
	{
		if (Id == ToolbarId[i])
			return i;
	}
	return -1;
}
int GetToolbarIDFromWnd2(HWND hWndDlg)
{
	int	i;

	while (hWndDlg)
	{
		for (i=0;i<nToolbars;i++)
		{
			if (hWndDlg == ToolbarWindow[i])
				return i;
		}
		hWndDlg = GetParent (hWndDlg);
	}
	return -1;
}

void RemoveToolbar (int ToolbarID,BOOL DoAdjust)
{
	int	i;

	RemoveDockedToolbar (ToolbarID);
	if (ToolbarID < nToolbars-1)
	{
		for (i=ToolbarID;i<nToolbars;i++)
		{
			ToolbarId[i] = ToolbarId[i + 1];
			ToolbarWindow[i] = ToolbarWindow[i+1];
			ToolbarHandle[i] = ToolbarHandle[i+1];
			ToolbarImage[i] = ToolbarImage[i+1];
			strcpy (ToolbarPath[i],ToolbarPath[i+1]);
			toolItem[i] = toolItem[i+1];
			nToolbarControls[i] = nToolbarControls[i+1];
			ToolbarHeight[i] = ToolbarHeight[i+1];
			MaxButtonBitmapWidth[i] = MaxButtonBitmapWidth[i+1];
			MaxButtonBitmapHeight[i] = MaxButtonBitmapHeight[i+1];
			FirstButtonBitmapHeight[i] = FirstButtonBitmapHeight[i+1];
			ToolbarFloating[i] = ToolbarFloating[i+1];
			ToolbarFloatNumPerRow[i] = ToolbarFloatNumPerRow[i+1];
			ToolbarType[i] = ToolbarType[i+1];
			ToolbarTTWindow[i] = ToolbarTTWindow[i+1];
//			ToolbarHook[i] = ToolbarHook[i+1];
//			ToolbarMoveHook[i] = ToolbarMoveHook[i+1];
			DisplayMenuStatus[i] = DisplayMenuStatus[i+1];
			nToolbarConfigs[i] = nToolbarConfigs[i+1];
			CreateConfigs[i] = CreateConfigs[i+1];
			ToolbarCurrentConfig[i] = ToolbarCurrentConfig[i+1];
			memmove (ToolbarConfigs[i],ToolbarConfigs[i+1],sizeof(ToolbarConfigs[0]));
			memmove (ToolbarConfigNumPerRow[i],ToolbarConfigNumPerRow[i+1],sizeof(ToolbarConfigNumPerRow[0]));
			nToolbarRows[i] = nToolbarRows[i+1];
			ToolbarRect[i] = ToolbarRect[i+1];
			ToolbarDPoint[i] = ToolbarDPoint[i + 1];
			ToolbarVPID[i] = ToolbarVPID[i + 1];
			ToolbarReZoomScale[i] = ToolbarReZoomScale[i + 1];
			HaveTrackMouseEvent[i] = HaveTrackMouseEvent[i + 1];
			ToolbarPointerType[i] = ToolbarPointerType[i + 1];
			strcpy(ToolbarHoverCmd[i], ToolbarHoverCmd[i + 1]);
//static	POINTS	ToolbarConfigs[MAX_TOOLBARS][MAX_TOOLBAR_CONTROLS];
			ToolbarPos[i] = ToolbarPos[i+1];
		}
	}
	AdjustDockedToolbarsForRemovedToolbar (ToolbarID);
	nToolbars--;
	if (!nToolbars)
	{
		if (ToolbarMoveHook)
			UnhookWindowsHookEx(ToolbarMoveHook);
		ToolbarMoveHook = 0;
	}
	if (DoAdjust)
		AdjustToolbarPositions ();
	return;
}

HBITMAP GetToolBitmap (LPSTR BMPath)
{
    BITMAPINFOHEADER DibInfo;  
    HDIB32	hDib32=0; 
	HDIB32	hDib8;
	HBITMAP	hBM = 0;

	if (BMPath)
	{
		if (*BMPath == 1)
		{
			UINT idBmp;
			memmove (&idBmp,++BMPath,sizeof(UINT));
			hBM = LoadBitmap (hInst,MAKEINTRESOURCE(idBmp));
		}
		else
			hDib32 = LoadDIB32 (BMPath,FALSE);
		if (hDib32)    
		{
			int	nBits = FreeImage_GetBPP (hDib32);
			if (nBits > 0 && nBits != 24)
			{
				HDIB32	hDib24 = FreeImage_ConvertTo24Bits (hDib32);
				//hDib8 = QuantizeDib (hDib24,FIQ_NNQUANT);
				DestroyDIB32(hDib32,FALSE);
				hDib8 = hDib24;
				//GMDestroyDIB32(hDib24);
			}
			else
				hDib8 = hDib32;
			hBM = DIB32ToBitmap(hDib8,(HPALETTE)0);
			DestroyDIB32(hDib8,FALSE);
		}
	}
	return hBM;
}

HBITMAP PadBitmapToConsistentSize (HWND hWnd,HBITMAP hBM,int w, int h,BOOL Stretch)
{
	BITMAP	bm;
	HDC		hDCWnd = GetDC (hWnd);
	HDC		hDCMem1 = CreateCompatibleDC (hDCWnd);
	HDC		hDCMem2 = CreateCompatibleDC (hDCWnd);
	HBITMAP hBMNew = CreateCompatibleBitmap (hDCWnd,w,h);
	HBITMAP	hBMOld1 = SelectObject (hDCMem1,hBMNew);
	HBITMAP	hBMOld2 = SelectObject (hDCMem2,hBM);
	int		ix, iy;
	RECT	rect={0,0,w,h};

	GetObject(hBM, sizeof(bm), (LPSTR)&bm);
	ix = (w - bm.bmWidth) / 2;
	iy = (h - bm.bmHeight) / 2;
	FillRect (hDCMem1,&rect,GetStockObject (LTGRAY_BRUSH));
	if (Stretch)
		StretchBlt (hDCMem1,0,0,w,h,hDCMem2,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
	else
		BitBlt(hDCMem1,ix,iy,bm.bmWidth,bm.bmHeight,hDCMem2,0,0,SRCCOPY);
	SelectObject (hDCMem1,hBMOld1);
	SelectObject (hDCMem2,hBMOld2);
	DeleteDC (hDCMem1);
	DeleteDC (hDCMem2);
	GSSiDeleteObject (&hBM);
	return hBMNew;
}

void SetWindowSizeToBitmap(HWND hWnd, HBITMAP hBmp)
{
	BITMAP	bm;
	RECT	wrect, crect;
	int		border;
	POINT	midp;

	if (!GetObject(hBmp, sizeof(bm), (LPSTR)&bm))
		return;
	GetWindowRect(hWnd, &wrect);
	GetClientRect(hWnd, &crect);
	midp = RectMid(&wrect);
	border = RECTWIDTH(&wrect) - RECTWIDTH(&crect);
	wrect.left = midp.x - (bm.bmWidth + border) / 2;
	wrect.top = midp.y - (bm.bmHeight + border) / 2;
	wrect.bottom = wrect.top + bm.bmHeight + border;
	wrect.right = wrect.left + bm.bmWidth + border;
	MoveWindow(hWnd, wrect.left, wrect.top, RECTWIDTH(&wrect), RECTHEIGHT(&wrect),FALSE);
	return;
}

int SetButtonSizetoBitmap (int ToolbarID,HWND hWndBtn,HBITMAP *hBM,int ix,int irow,int RowHeight,int filepos)
{
	BITMAP	bm;
	RECT	Rect;
	int		w,h;
	static	int	firstRowH;
	static	BOOL	display=FALSE;

	if (!GetObject(*hBM, sizeof(bm), (LPSTR)&bm))
		return 0;
	if (filepos < 0)
	{
		if (!FirstToolbarPass)
		{
			*hBM = PadBitmapToConsistentSize (hWndBtn,*hBM,MaxButtonBitmapWidth[ToolbarID]+4,MaxButtonBitmapHeight[ToolbarID]/4+4,TRUE);
		}
	}
	else if (FirstToolbarPass)
	{
		MaxButtonBitmapWidth[ToolbarID] = max (MaxButtonBitmapWidth[ToolbarID],bm.bmWidth);
		MaxButtonBitmapHeight[ToolbarID] = max (MaxButtonBitmapHeight[ToolbarID],bm.bmHeight);
	}
	else
		*hBM = PadBitmapToConsistentSize (hWndBtn,*hBM,MaxButtonBitmapWidth[ToolbarID]+4,MaxButtonBitmapHeight[ToolbarID]+4,FALSE);
	GetObject(*hBM, sizeof(bm), (LPSTR)&bm);
	GetWindowRect (hWndBtn,&Rect);
	if (!irow)
	{
		FirstButtonBitmapHeight[ToolbarID] = bm.bmHeight;
		firstRowH = bm.bmHeight+4;
		MoveWindow(hWndBtn,ix+1,2,bm.bmWidth,bm.bmHeight,display);
	}
	else
		MoveWindow(hWndBtn,ix+1,firstRowH+2+(irow-1)*(bm.bmHeight+4),bm.bmWidth,bm.bmHeight,display);
	//hWDP = DeferWindowPos (hWDP,hWndBtn,0,ix+1,irow*(bm.bmHeight+4),bm.bmWidth,bm.bmHeight,SWP_NOZORDER);
	return ix + bm.bmWidth + 2;
}

int SetBitmapHeightToButton(HWND hWndBtn, HBITMAP *hBM, int iHeight)
{
	BITMAP	bm;
	RECT	Rect;
	HBITMAP	rtn = 0;
	int		rc = -1;

	if (GetObject(*hBM, sizeof(bm), (LPSTR)&bm))
	{
		HDC	hDC = GetDC(hWndBtn);
		HDC	hDC2 = CreateCompatibleDC(hDC);
		HDC	hDC3 = CreateCompatibleDC(hDC);
		double	factor = (double)iHeight / bm.bmHeight;
		int	Width = bm.bmWidth * factor;
		HBITMAP	hBM2 = CreateCompatibleBitmap(hDC, Width, iHeight);
		HBITMAP	hBMOld2 = SelectObject(hDC2, hBM2);
		HBITMAP	hBMOld3 = SelectObject(hDC3, *hBM);

		SetStretchBltMode(hDC2, HALFTONE);// COLORONCOLOR);

		rc = StretchBlt(hDC2, 0, 0, Width, iHeight,
			hDC3, 0, 0, bm.bmWidth, bm.bmHeight,
			SRCCOPY);
		SelectObject(hDC2, hBMOld2);
		SelectObject(hDC3, hBMOld3);
		ReleaseDC(hWndBtn, hDC);
		GSSiDeleteObject(hBM);
		DeleteDC(hDC2);
		DeleteDC(hDC3);
		*hBM = hBM2;
		rc = bm.bmWidth;
	}
	return rc;
}
int SetBitmapSizeToButton(HWND hWndBtn, HBITMAP *hBM)
{
	BITMAP	bm;
	RECT	Rect;
	HBITMAP	rtn = 0;
	int		rc = -1;

	if (GetObject(*hBM, sizeof(bm), (LPSTR)&bm))
	{
		HDC	hDC = GetDC(hWndBtn);
		HDC	hDC2 = CreateCompatibleDC(hDC);
		HDC	hDC3 = CreateCompatibleDC(hDC);
		int	Width, Height;
		RECT rect;

		GetClientRect(hWndBtn, &rect);
		Width = RECTWIDTH(&rect);
		Height = RECTHEIGHT(&rect);
		HBITMAP	hBM2 = CreateCompatibleBitmap(hDC, Width, Height);
		HBITMAP	hBMOld2 = SelectObject(hDC2, hBM2);
		HBITMAP	hBMOld3 = SelectObject(hDC3, *hBM);

		SetStretchBltMode(hDC2, COLORONCOLOR);

		rc = StretchBlt(hDC2, 0, 0, Width, Height,
			hDC3, 0, 0, bm.bmWidth, bm.bmHeight,
			SRCCOPY);
		SelectObject(hDC2, hBMOld2);
		SelectObject(hDC3, hBMOld3);
		ReleaseDC(hWndBtn, hDC);
		GSSiDeleteObject(hBM);
		DeleteDC(hDC2);
		DeleteDC(hDC3);
		*hBM = hBM2;
		rc = Width;
	}
	return rc;
}
BOOL AddToolToToolTip(HWND hwndCtrl, int ToolbarID)
	{ 
    TOOLINFO ti; 
 
        ti.cbSize = sizeof(TOOLINFO); 
        ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS; 
        ti.hwnd = ToolbarWindow[ToolbarID];
        ti.uId = (UINT) hwndCtrl; 
        ti.hinst = hInst; 
        ti.lpszText = LPSTR_TEXTCALLBACK; 
        SendMessage(ToolbarTTWindow[ToolbarID], TTM_ADDTOOL, 0, 
            (LPARAM) (LPTOOLINFO) &ti); 
    return TRUE; 
	} 
 


int AddButtonToToolbar2 (int ToolbarID,HWND hWndDlg,LPSTR BMPath,LPSTR ButtonText,int filepos,LPINT piButton)
{
	int		ButtonNumber = (*piButton)++;//nToolbarControls[ToolbarID];
	UINT	button;
	HBITMAP	hBM=0, hOldBM;
	LPTOOBAR_CONTROL_INFO	pTBInfo;

	if (filepos < 0)
		button = Buttons[ButtonNumber];
	else
		button = Buttons[ButtonNumber+5];

	if (FirstToolbarPass)
	{
		if (BMPath)
			hBM = GetToolBitmap(BMPath);
		else
		{
			int ln = strlen(ButtonText);
			SIZE	txSize;
			HDC		hDC = GetDC(hWndMain);
			HDC		hDCtemp = CreateCompatibleDC(hDC);
			HBITMAP hBMPtemp;
			RECT	rect = { 0 };
			HFONT	hFont,hOldFont;
			int		fontHeight = GetGlobalLVal2("[%TOOLBARFONTHEIGHT]", 38);

			hFont = CreateFont(fontHeight, 0, 0, 0, FW_BLACK,0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
			hOldFont = SelectObject(hDC, hFont);
			GetTextExtentPoint32(hDC, ButtonText, ln, &txSize);
			SelectObject(hDC, hOldFont);
			rect.right = txSize.cx + 6;
			rect.bottom = txSize.cy + 6;
			hBM = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
			ReleaseDC(hWndMain, hDC);
			hBMPtemp = SelectObject(hDCtemp, hBM);
			hOldFont = SelectObject(hDCtemp, hFont);
			FillRect(hDCtemp, &rect, GetStockObject(WHITE_BRUSH));
			SetBkMode(hDCtemp,TRANSPARENT);
			TextOut(hDCtemp, 3, 3, ButtonText, ln);
			SelectObject(hDCtemp, hOldFont);
			SelectObject(hDCtemp, hBMPtemp);
			DeleteDC(hDCtemp);
			DeleteObject(hFont);
		}
	}
	else
	{
		pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock (ToolbarHandle[ToolbarID]) + ButtonNumber;
		hBM = pTBInfo->hBM;
		GlobalUnlock (ToolbarHandle[ToolbarID]);
	}
	if (!hBM)
		return -1;
	if (nToolbarControls[ToolbarID] >= MAX_TOOLBAR_CONTROLS)
		return -1;
	nControlsInRow++;
	if (nControlsInRow > MaxControlsInRow)
	{
		nToolbarRow++;
		nControlsInRow = 1;
		ToolbarX = 1;
	}
	if (ToolbarHeight[ToolbarID] && FirstToolbarPass)
		SetBitmapHeightToButton (GetDlgItem(hWndDlg,button),&hBM,ToolbarHeight[ToolbarID]-3);
	ToolbarX = SetButtonSizetoBitmap (ToolbarID,GetDlgItem(hWndDlg,button),&hBM,ToolbarX,nToolbarRow,ToolbarHeight[ToolbarID],filepos) + 1;
	MaxToolbarX = max (MaxToolbarX,ToolbarX);
	hOldBM = (HBITMAP)SendDlgItemMessage (hWndDlg,button,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hBM);
	ShowWindow (GetDlgItem (hWndDlg,button),SW_SHOWNORMAL);
	EnableWindow (GetDlgItem (hWndDlg,button),TRUE);
	pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock (ToolbarHandle[ToolbarID]) + ButtonNumber;
	pTBInfo->CntlID = button;
	pTBInfo->hBM = hBM;
	pTBInfo->FileLoc = filepos;
	strncpy0 (pTBInfo->Label,ButtonText,63);
	if (FirstToolbarPass)
	{
		pTBInfo->wndProc = (WNDPROC)SetWindowLong(GetDlgItem(hWndDlg, button), GWL_WNDPROC, (LONG)ButtonSubclassProc);
		if (pTBInfo->wndProc == ButtonSubclassProc)
			ii = 1;
	}
	GlobalUnlock(ToolbarHandle[ToolbarID]);
	if (FirstToolbarPass)
		nToolbarControls[ToolbarID]++;
	return ToolbarX;
}

int AddButtonToToolbar (HWND hWndDlg,LPSTR BMPath,LPSTR ButtonText,int filepos,LPINT piButton)
{
	int	i, rtn;
	int	ToolbarID = GetToolbarIDFromWnd (hWndDlg);

	if (filepos < 0)
		for (i=0;i<MaxControlsInRow;i++)
			rtn = AddButtonToToolbar2 (ToolbarID,hWndDlg,BMPath,ButtonText,filepos,piButton);
	else
		rtn = AddButtonToToolbar2 (ToolbarID,hWndDlg,BMPath,ButtonText,filepos,piButton);
	return rtn;
}


BOOL ProcessToolbarCmd (HWND hWndDlg,UINT CntlID)
{
	LPTOOBAR_CONTROL_INFO	pTBInfo;
	int	i, ToolbarID;
	BOOL	rtn=FALSE;

	ToolbarID = GetToolbarIDFromWnd (hWndDlg);
	if (ToolbarID < 0)
		return FALSE;
	pTBInfo = GlobalLock (ToolbarHandle[ToolbarID]);
	for (i=0;i<nToolbarControls[ToolbarID];i++,pTBInfo++)
	{
		 if (pTBInfo->CntlID == CntlID)
		 {
			 currentToolbarWnd = hWndDlg;
			 RunGFCommandFromFileAtLoc (ToolbarPath[ToolbarID],pTBInfo->FileLoc,TRUE,0);
			 rtn = TRUE;
			 break;
		 }
	}
	GlobalUnlock (ToolbarHandle[ToolbarID]);
	return rtn;
}
BOOL DestroyCurrentToolbar(void)
{
	if (!currentToolbarWnd)
		return FALSE;
	DestroyWindow(currentToolbarWnd);
	return TRUE;
}
BOOL ReloadToolbar(LPSTR pOpt)
{
	int i;
	if (!*pOpt || !stricmp(pOpt, "CURRENT"))
	{
		if (!currentToolbarWnd)
			return FALSE;
		PostMessage(currentToolbarWnd, WM_EXITSIZEMOVE, 0, 0);
	}
	else if (!stricmp(pOpt, "ALL"))
	{
		for (i = 0; i < nToolbars; i++)
		{
			if (ToolbarWindow[i])
				PostMessage(ToolbarWindow[i], WM_EXITSIZEMOVE, 0, 0);
		}
	}
	else
		return FALSE;
	return TRUE;
}

BOOL RedisplayToolbar(LPSTR pOpt)
{
	int i;
	if (!*pOpt || !stricmp(pOpt, "CURRENT"))
	{
		if (!currentToolbarWnd)
			return FALSE;
		//PostMessage(currentToolbarWnd, WM_EXITSIZEMOVE, 0, 0);
	}
	else if (!stricmp(pOpt, "ALL"))
	{
		for (i = 0; i < nToolbars; i++)
		{
			if (ToolbarWindow[i])
				LoadGFFile(ToolbarWindow[i], ToolbarPath[i], 5, ToolbarFloating[i]);
		}
	}
	else
		return FALSE;
	return TRUE;
}

void DestroyAllToolbars(void)
{
	int	i;

	while (nToolbars)
	{
		if (ToolbarWindow[nToolbars-1])
			DestroyWindow (ToolbarWindow[nToolbars-1]);
		else
			nToolbars--;
	}
	if (ToolbarMoveHook)
		UnhookWindowsHookEx(ToolbarMoveHook);
	ToolbarMoveHook = 0;
	return;
}

BOOL DestroyToolbar (int ID)
{
	LPTOOBAR_CONTROL_INFO	pTBInfo;
	int	i;
	int	ToolbarID;
//	HBITMAP	hBMDefault = LoadBitmap (NULL,OBM_BTNCORNERS);

	if (ID > -1)
		ToolbarID = ID;
	else
		return FALSE;
	if (!ToolbarHandle[ToolbarID])
		return FALSE;
/*	if (ToolbarMoveHook[ToolbarID])
		UnhookWindowsHookEx(ToolbarMoveHook[ToolbarID]);
	ToolbarMoveHook[ToolbarID] = 0;*/
	DisplayMenuStatus[ToolbarID] = DMS_NORMAL;
	GSSiDeleteObject (&ToolbarImage[ToolbarID]);
//	ToolbarHook[ToolbarID] = 0;
	if (ToolbarTTWindow[ToolbarID])
		DestroyWindow (ToolbarTTWindow[ToolbarID]);
	ToolbarTTWindow[ToolbarID] = 0;
	pTBInfo = GlobalLock (ToolbarHandle[ToolbarID]);
	for (i=0;i<nToolbarControls[ToolbarID];i++)
	{
		//HBITMAP hBM = (HBITMAP)SendDlgItemMessage (hWndDlg,pTBInfo[i].CntlID,BM_SETIMAGE,IMAGE_BITMAP,hBMDefault);
		
		GSSiDeleteObject(&pTBInfo[i].hBM);
	}

	GSSiGlobUlFree (&ToolbarHandle[ToolbarID]);
	nToolbarControls[ToolbarID] = 0;

	return TRUE;
}

void AddToolbarConfig (int ToolbarID,int nInRow)
{
	int	i;

	for (i=0;i<nToolbarConfigs[ToolbarID];i++)
		if (ToolbarConfigNumPerRow[ToolbarID][i] == nInRow)
			return;
	ToolbarConfigNumPerRow[ToolbarID][nToolbarConfigs[ToolbarID]++] = max (1,nInRow);
	return;
}

BOOL RestoreToolbarImage (int ToolbarID)
{
	if (ToolbarImage[ToolbarID])
	{
		HDC	hDC = GetWindowDC (ToolbarWindow[ToolbarID]);
		RECT	rect;
		int	w,h;

		GetWindowRect (ToolbarWindow[ToolbarID],&rect);
		w = RECTWIDTH(&rect);
		h = RECTHEIGHT(&rect);
		rect.left = rect.top = 0;
		rect.right = w;
		rect.bottom = h;
		RestoreScreen (hDC,ToolbarImage[ToolbarID], rect);
		ReleaseDC (ToolbarWindow[ToolbarID],hDC);
		return TRUE;
	}
	else
		return FALSE;
}

void DrawToolbarPointer(int toolbarID)
{
    POINT toolbarPoint;
	HPEN  ArrowPen;
	LPVIEWPORT SaveView=CurView;
	RECT	rect;
	int LineWidth=4, TipWidth = 3;
	HANDLE hPointer;
	static int PLstyle = 3, BorderStyle=1;
	COLORREF PointerColor=RGB(0,0,255), BorderColor=RGB(0,0,255);

	SetViewport(ToolbarVPID[toolbarID]);
	SaveDC(CurView->hDC);
	SetDisplayMode(CurView->hDC, GF_MAPMODE);
	GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE, FALSE);
	SelectClipRgn(CurView->hDC, CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn);
	if (PointInWBounds(&ToolbarDPoint[toolbarID]))
	{
		POINT MapPoint = BasePtToWinPt(&ToolbarDPoint[toolbarID]);
		SetDisplayMode(CurView->hDC, GF_TEXTMODE);
		SetCurView(SaveView);
		GetWindowRect(ToolbarWindow[toolbarID],&rect);
		toolbarPoint = RectMid(&rect);
		ArrowPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
		//DrawPointerLine(CurView->hDC, toolbarPoint, MapPoint, ArrowPen, ArrowPen, 10, 0);
		hPointer = DrawTAGPointerLine(CurView->hDC, toolbarPoint, MapPoint, FALSE, LineWidth, TipWidth,
			PLstyle, PointerColor, BorderStyle, BorderColor);
		DeleteObject(ArrowPen);
	}
	RestoreDC(CurView->hDC, -1);
	

/*	hPointer = ShowPointerLine(CurView->hDC, TAGBox.rect, TAGBox.TAGPointScr, TAGBox.ConnectPoint, &HavePL, MoveMode, LineWidth, Elwh, Elwh, pRect);
	if (hPointer)
	{
		HRGN    NewRgn;
		LPPOINT	pPPoints = (LPPOINT)GlobalLock(hPointer);

		NewRgn = CreatePolygonRgn(pPPoints, 3, ALTERNATE);
		GSSiGlobUlFree(&hPointer);
		CombineRgn(NewRgn, hRgn, NewRgn, RGN_DIFF);
		SelectClipRgn(hDC, NewRgn);
		GSSiDeleteObject(&NewRgn);
	}
	*/

	return;
}

void RemoveToolbarPointer(int toolbarID)
{
	return;
}

BOOL CALLBACK TOOLBARMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	static	HBITMAP	hBM1,hBM2,hBM3;
	HDIB	hDIB;
	HPALETTE	hPal;
	int		ix=0, winc, hinc, nWidth, nHeight, fwSide, ii, nPerRow;
	int	w,h;
	RECT	rect, WindRect;
	LPRECT	lprc;
	BOOL	isDocked = FALSE;
	POINT	pt;
	static	int		iConfig;
	int		ID = GetToolbarIDFromWnd (hWndDlg);
//	int		HaveTrackMouseEvent;
	long	lRetVal;
	int		ToolbarID=-1;

	if (Message == WM_NOTIFY)
		ii=1;

	if (ID > -1)
	{
		ToolbarID = ID;

		if (Message == WM_COMMAND && HIWORD(wParam) == BN_SETFOCUS)
		{
			if (LOWORD (wParam) == IDC_BUTTON11)
			{
				if (DisplayMenuStatus[ToolbarID] == DMS_DIMMED)
					PostMessage (hWndDlg,GSSi_DimMenu,0,0);

			}
		}
	}
	else
		ToolbarID = ToolbarIDCur;
        //  return DefWindowProc(hWndDlg, Message, wParam, lParam);
switch(Message)
   {
	case WM_PAINT:

		if (ToolbarID >= 0 && ToolbarImage[ToolbarID])
		{
			PAINTSTRUCT	ps;

			BeginPaint (hWndDlg,&ps);
			RestoreToolbarImage (ToolbarID);
			EndPaint (hWndDlg,&ps);
			if ((DisplayMenuStatus[ToolbarID]==DMS_WANTDIMMED || DisplayMenuStatus[ToolbarID]==DMS_DIMMED) && ToolbarFloating[ToolbarID])
			{

				PostMessage (hWndDlg,GSSi_DimMenu,0,0);

				return 1;
			}
		}
		else
		{
			if (ToolbarID >= 0)
				DisplayMenuStatus[ToolbarID] = DMS_NOTDISPLAYED;
			return DefWindowProc(hWndDlg, Message, wParam, lParam);
		}
		break;
/*	case WM_SHOWWINDOW:
		//PostMessage (hWndDlg,GSSi_DimMenu,0,0);
		SetWindowPos (hWndDlg,HWND_NOTOPMOST,0,0,0,0,SWP_DRAWFRAME|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
		return TRUE;
		break;
*/
	case GSSi_DimMenu:
		DisplayDimmedMenu (ToolbarID);
		break;

	case WM_ERASEBKGND:
		if (!ToolbarFloating[ToolbarID])
			return 0;
		switch (DisplayMenuStatus[ToolbarID])
		{
		case DMS_WANTDIMMED:
			DisplayDimmedMenu (ToolbarID);
		case DMS_NORMAL:
		case DMS_DIMMED:
			return 1;
		case DMS_NOTDISPLAYED:
			return 1;
		}
		break;

	case WM_NCHITTEST: 
	{
	    lRetVal = DefWindowProc(hWndDlg, Message, wParam, lParam); 

FromNotify:
	    switch (lRetVal)
	    {   
	    	case HTCLIENT: 
			default:
				{
					SetInClientMain ();
					if (ID >= 0)
					{
						switch (DisplayMenuStatus[ID])
						{
						case DMS_DIMMED:
						case DMS_WANTDIMMED:
							if (ToolbarFloating[ID])
								RestoreToolbarImage (ID);
							DisplayMenuStatus[ID] = DMS_NORMAL;
							break;
						default:
						case DMS_NORMAL:
							break;
						case DMS_NOTDISPLAYED:
							break;
						}
					}
				}
				
		}
		if (lRetVal < LONG_MAX)
			SetWindowLong (hWndDlg,DWL_MSGRESULT,lRetVal);

	}
	break;

	case WM_MOUSEMOVE:
		/*if (!HaveTrackMouseEvent[ToolbarID])
		{
			TRACKMOUSEEVENT EventTrack;

			EventTrack.dwFlags = TME_LEAVE;
			EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
			EventTrack.hwndTrack = hWndDlg;
			EventTrack.dwHoverTime = 200;
			TrackMouseEvent(&EventTrack);
			HaveTrackMouseEvent[ToolbarID] = TRUE;
			DrawToolbarPointer(ToolbarID);
//			DrawBtnFocusRect(hWndDlg);
		} 

		break;

	case WM_TIMER:
		KillTimer(hWndDlg, 1);
		ProcessText(ToolbarHoverCmd[ToolbarID]);
		break;

	case WM_MOUSELEAVE:
//		DrawBtnFocusRect(hWndDlg);
		HaveTrackMouseEvent[ToolbarID] = FALSE;
		RemoveToolbarPointer(ToolbarID);
		if (*ToolbarHoverCmd[ToolbarID])
			ii = KillTimer(hWndDlg, 1);
		break;
*/
	case WM_SIZE:
		//if (!DestroyToolbar (hWndDlg))
		nWidth = LOWORD(lParam);
		nHeight = HIWORD(lParam); 
		GetWindowRect (hWndDlg,&rect);
		GetClientRect (hWndDlg,&rect);
		break;
	case WM_SIZING:
		fwSide = wParam;
		lprc = (LPRECT) lParam; 
		switch (fwSide)
		{
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				break;
			case WMSZ_LEFT:
			case WMSZ_RIGHT:
				if (nToolbarRows[ToolbarID] == 1)
				{
					*lprc = ToolbarRect[ToolbarID];
					break;
				}
				lprc->bottom += 10;
				break;
			case WMSZ_TOP:
			case WMSZ_TOPLEFT: 
			case WMSZ_TOPRIGHT:
				break;
		}
		return TRUE;
		break;
	case WM_EXITSIZEMOVE:
		{
			char	Pathname[MAX_PATH];

			if (ToolbarIsDocked(ToolbarID))
			{
				int height = ToolbarHeight[ToolbarID];
				nPerRow = nToolbarRows[ToolbarID];
				strcpy (Pathname,ToolbarPath[ToolbarID]);
				GetWindowRect (hWndDlg,&ToolbarRect[ToolbarID]);
				DestroyWindow (hWndDlg);
				LoadToolbar (hWndMain,Pathname,"DOCK",height,nPerRow,"",TRUE,FALSE,&ToolbarDPoint[ToolbarID],ToolbarReZoomScale[ToolbarID],ToolbarVPID[ToolbarID]);
			}
			else
			{
				POINT	pt;

				GetCursorPos (&pt);
				GetWindowRect (hWndDlg,&ToolbarRect[ToolbarID]);
				SeeIfToolbarShouldBeDocked (hWndDlg,&pt);
			}
		}
		break;
	case WM_DRAWITEM:
		{
			UINT	idCtl = (UINT) wParam;             // control identifier 
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; // item-drawing information 
			int		iState = 0;
			int		iBtn;
			HBITMAP	hBM, hBMPrev;

			switch (lpdis->itemAction)
			{  
				case ODA_SELECT:
					if (lpdis->itemState & ODS_SELECTED)
						iState = 1;
				case ODA_DRAWENTIRE:
					if (lpdis->itemState & ODS_DISABLED)
						iState = 2;
//					hBM = LoadBitmap (hInst,MAKEINTRESOURCE(PictBtnBM[iBtn][iState]));
					hBM = (HBITMAP)SendDlgItemMessage (hWndDlg,idCtl,BM_GETIMAGE,IMAGE_BITMAP,0);
					DisplayBitmapOnButton (lpdis->hDC,&lpdis->rcItem,hBM);
//					DeleteObject (hBM);
					return TRUE;
					break;
				case ODA_FOCUS:
				     HandleFocusState(lpdis);
					  break;
				default:
					ii=1;
					break;
			}  //itemAction
		}
		break;
      case WM_INITDIALOG:
         /* initialize working variables                                */  
        // DebugInfoWnd = hWndDlg;
		ToolbarWindow[ToolbarID] = hWndDlg;
		//g_hwndDlg = ToolbarWindow[ToolbarID];
		/* disable all controls											*/
//		if (!EnumChildWindows(hWndDlg, (WNDENUMPROC) EnumChildProcTT, 1)) 
//			return FALSE; 
 		FirstToolbarPass = TRUE;
		MaxButtonBitmapWidth[ToolbarID] = 0;
		MaxButtonBitmapHeight[ToolbarID] = 0;
		if (ToolbarHandle[ToolbarID])
		{
			LPTOOBAR_CONTROL_INFO	pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock (ToolbarHandle[ToolbarID]);
			int	i;

			for (i=0;i<nToolbarControls[ToolbarID];i++)
				GSSiDeleteObject(&pTBInfo[i].hBM);

			GlobalUnlock (ToolbarHandle[ToolbarID]);
		}
     case GSSI_REINITDIALOG:
		if (!nToolbarConfigs[ToolbarID])
		{
			CreateConfigs[ToolbarID] = TRUE;
			iConfig = 0;
			nToolbarConfigs[ToolbarID] = 1;
		}
		else
		{
			iConfig = ToolbarCurrentConfig[ToolbarID];
			CreateConfigs[ToolbarID] = FALSE;
		}
NextConfig:
		GetClientRect (hWndDlg,&rect);
		//ShowWindow (hWndDlg,SW_HIDE);
		MaxControlsInRow = ToolbarConfigNumPerRow[ToolbarID][iConfig];
		//if (ToolbarFloating[ToolbarID])
			MaxDesiredToolbarWidth = INT_MAX;
		//else
		//	MaxDesiredToolbarWidth = ToolbarWidthLeft;
TryAgain:
		ToolbarX = 1;
		MaxToolbarX = 0;
		nToolbarRow = 0;
		nControlsInRow = 0;
		rowY = 0;
		hWDP = BeginDeferWindowPos(1);

		if (!ToolbarHeight[ToolbarID])
			ToolbarHeight[ToolbarID] = rect.bottom - rect.top - 2;
		if (FirstToolbarPass)
			nToolbarControls[ToolbarID] = 0;
		LoadGFFile (hWndDlg,ToolbarPath[ToolbarID],3,ToolbarFloating[ToolbarID]);
		GetWindowRect (hWndDlg,&WindRect);
		winc = (WindRect.right - WindRect.left) - (rect.right - rect.left);
		hinc = (WindRect.bottom - WindRect.top) - (rect.bottom - rect.top);
		MaxControlsInRow = max (1,min (MaxControlsInRow,nToolbarControls[ToolbarID]));
		if (FirstToolbarPass)// || (MaxControlsInRow > 1 && (MaxToolbarX+winc+1 > MaxDesiredToolbarWidth)))
		{
			int	LastMaxControlsInRow = MaxControlsInRow, i;
			LPTOOBAR_CONTROL_INFO	pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock (ToolbarHandle[ToolbarID]);
			RECT	rect;

			if (!FirstToolbarPass)
				MaxControlsInRow --;
			pTBInfo += MaxControlsInRow;
			GetWindowRect (hWndDlg,&rect);
			pTBInfo->w = RECTWIDTH (&rect);
			pTBInfo->h = RECTHEIGHT (&rect);
			GlobalUnlock (ToolbarHandle[ToolbarID]);
			//nToolbarControls[ToolbarID] = 0;
			FirstToolbarPass = FALSE;
			goto TryAgain;
		}
//		MoveWindow(hWndDlg,ToolBarStartPoint.x,ToolBarStartPoint.y,MaxToolbarX+winc+1,(MaxButtonBitmapHeight+4)*++nToolbarRow+hinc+1,TRUE);
		pt = ToolBarStartPoint;
		ClientToScreen (hWndMain,(LPPOINT)&pt);
		if (ToolbarFloating[ToolbarID])
		{
			hWDP = DeferWindowPos (hWDP,hWndDlg,0,pt.x,pt.y,
				   MaxToolbarX+winc+1,
				   (MaxButtonBitmapHeight[ToolbarID]+8)*(nToolbarRow++ - 1) +2*(FirstButtonBitmapHeight[ToolbarID]+2)+hinc,SWP_NOZORDER);
		}
		else
		{
			int w = MaxToolbarX + winc + 1;
			int h = (MaxButtonBitmapHeight[ToolbarID] + 8)*nToolbarRow + FirstButtonBitmapHeight[ToolbarID] + 4 + hinc + winc;
			nToolbarRow++;
			hWDP = DeferWindowPos (hWDP,hWndDlg,0,pt.x,pt.y, w, h,SWP_NOZORDER);
		}

		if (CreateConfigs[ToolbarID])
		{
			EndDeferWindowPos(hWDP);
			GetWindowRect (hWndDlg,&ToolbarRect[ToolbarID]);
			ToolbarConfigs[ToolbarID][iConfig].cx = RECTWIDTH (&ToolbarRect[ToolbarID]);
			ToolbarConfigs[ToolbarID][iConfig].cy = RECTHEIGHT (&ToolbarRect[ToolbarID]);
			if (!iConfig++)
			{
				int	n, mid=nToolbarControls[ToolbarID]/2;

				AddToolbarConfig (ToolbarID,1);
				AddToolbarConfig (ToolbarID,nToolbarControls[ToolbarID]);
				for (n=2;n<=mid;n++)
					if (nToolbarControls[ToolbarID] % n == 0)
						AddToolbarConfig (ToolbarID,n);
			}
			if (iConfig == nToolbarConfigs[ToolbarID])
			{
				//HANDLE	saveToolbarHandle = ToolbarHandle[ToolbarID];
				HWND saveToolbarWindow = ToolbarWindow[ToolbarID];

				//ToolbarHandle[ToolbarID] = 0;
				ToolbarWindow[ToolbarID] = 0;
				DestroyWindow (hWndDlg);
				//ToolbarHandle[ToolbarID] = saveToolbarHandle;
				ToolbarWindow[ToolbarID] = saveToolbarWindow;
			}
			else
			{
				LPTOOBAR_CONTROL_INFO	pTBInfo = GlobalLock (ToolbarHandle[ToolbarID]);
				int i;

//				for (i=0;i<nToolbarControls[ToolbarID];i++)
//					 GSSiDeleteObject(&pTBInfo[i].hBM);
				GlobalUnlock (ToolbarHandle[ToolbarID]);
				goto NextConfig;
			}
		}
		else
		{
			EndDeferWindowPos(hWDP);
			if (DisplayToolbars)
			{
				GetWindowRect (hWndDlg,&ToolbarRect[ToolbarID]);
				ShowWindow (hWndDlg,SW_SHOW);
			}
			else
			{
				GetWindowRect (hWndDlg,&ToolbarRect[ToolbarID]);
				//MoveWindow (hWndDlg,5000,5000,RECTWIDTH(&ToolbarRect[ToolbarID]),RECTHEIGHT(&ToolbarRect[ToolbarID]),FALSE);
				SetWindowPos (hWndDlg,0,5000,5000,RECTWIDTH(&ToolbarRect[ToolbarID]),RECTHEIGHT(&ToolbarRect[ToolbarID]),SWP_NOZORDER|SWP_NOOWNERZORDER);
			}
		}
        return 1; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
		 DestroyWindow (hWndDlg);
         break; /* End of WM_CLOSE                                      */
	case WM_DESTROY:
	{
		int  ToolbarID2 = GetToolbarIDFromWnd(hWndDlg);
		if (ToolbarID2 > -1)
		{
			if (ToolbarHandle[ToolbarID2])
			{
				DestroyToolbar(ToolbarID2);
				RemoveToolbar(ToolbarID2, TRUE);
			}
		}
		AdjustToolbarPositions();
	}
		 break;
	case WM_NOTIFY:
		 OnWMNotify(hWndDlg,lParam);
		 lRetVal = LONG_MAX; 
		 goto FromNotify;
		 break;

	case WM_COMMAND:
		{
			int ID = LOWORD(wParam);
			int wNotifyCode = HIWORD(wParam); 

			if (!wNotifyCode && !ProcessToolbarCmd (hWndDlg,ID))
			 switch(ID)
			   {
           
				  case IDOK:
					  DebugWait = FALSE;
					  break;

			   }
		}
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

int LoadToolbar (HWND hWnd,LPSTR Pathname,LPSTR TypeIn,int Height,int nPerRow,LPSTR Pos,BOOL CheckForDocked,BOOL Float,LPDPOINT pCenterPoint,double  scale,int vpID) 
{
	static	BOOL	First=TRUE;
	char	Type[32];
	int	i;
	BOOL rc=FALSE;
	LPTOOBAR_CONTROL_INFO	pTBInfo;
	BOOL	err;
	RECT	MainRect, rect;
	POINT	SaveToolBarStartPoint;
	int		ToolbarID;

	strcpy (Type,TypeIn);
	if (!GetGlobalBVal2 ("[%ALLOWTOOLBARDOCKING]",TRUE))
	{
		Float = TRUE;
		if (!stricmp (Type,"DOCK"))
			strcpy (Type,"FLOAT");
	}
	GetWindowRect (hWndMain,&MainRect);

	if (!stricmp (Type,"VIS"))
	{
		GetWindowRect (hWnd,&rect);
		Height = RECTHEIGHT (&rect);
	}
	else if (Height <= 0)
		Height = 32;
	if (nToolbars >= MAX_TOOLBARS)
		return FALSE;
	if ((stricmp (Type,"VIS") && stricmp (Type,"ZOOM")) && !ExistFile (Pathname))
		return FALSE;
	ToolBarStartPoint = atopt16 (Pos,&err);
	if (err)
	{
		if (!stricmp (Type,"FLOAT"))
		{
			ToolBarStartPoint = RectMid (&MainRect);
			ToolBarStartPoint.x -= RECTWIDTH (&MainRect) / 4;
			ToolBarStartPoint.y -= RECTHEIGHT (&MainRect) / 4;
		}
		else
			ToolBarStartPoint.x = ToolBarStartPoint.y = 0;
	}
	SaveToolBarStartPoint = ToolBarStartPoint;
	ToolbarHeight[nToolbars] = Height;
	ToolbarHandle[nToolbars] = GSSiGlobAlloc (1570,GHND,MAX_TOOLBAR_BUTTONS*sizeof(TOOBAR_CONTROL_INFO));
	strcpy (ToolbarPath[nToolbars],Pathname);
	ToolbarId[nToolbars] = nextToolbarId++;
	ToolbarID = nToolbars++;
	ToolbarIDCur = ToolbarID;
	DisplayMenuStatus[ToolbarID] = DMS_NOTDISPLAYED;
	ToolbarFloatNumPerRow[ToolbarID] = nPerRow;
	ToolbarConfigNumPerRow[ToolbarID][0] = max (1,nPerRow);
	nToolbarConfigs[ToolbarID] = 0;
	ToolbarType[ToolbarID] = TBT_STANDARDMENU;
	ToolbarDPoint[ToolbarID].x = ToolbarDPoint[ToolbarID].y = 0;
	GSSiDeleteObject (&ToolbarImage[ToolbarID]);
	if (!stricmp (Type,"ZOOM"))
	{
		ToolbarType[ToolbarID] = TBT_ZOOMPAN;
		nToolbarConfigs[ToolbarID] = 1;
		GetWindowRect (hWnd,&ToolbarRect[ToolbarID]);
		ToolbarFloating[ToolbarID] = Float;
		ToolbarWindow[ToolbarID] = hWnd;
		ToolbarConfigs[ToolbarID][0].cx = RECTWIDTH (&ToolbarRect[ToolbarID]);
		ToolbarConfigs[ToolbarID][0].cy = RECTHEIGHT (&ToolbarRect[ToolbarID]);
	}
	else if (!stricmp (Type,"VIS"))
	{
		RECT	rect;

		ToolbarType[ToolbarID] = TBT_VISMENU;
		nToolbarConfigs[ToolbarID] = 1;
		GetWindowRect (hWnd,&rect);
		ToolbarFloating[ToolbarID] = Float;
		ToolbarConfigs[ToolbarID][0].cx = RECTWIDTH (&rect);
		ToolbarConfigs[ToolbarID][0].cy = RECTHEIGHT (&rect);
	}
	else
	{
		int	saveToolbarID = ToolbarID;
		BOOL	saveDisplayToolbars = DisplayToolbars;

		DisplayToolbars = FALSE;
		if (pCenterPoint)
		{
			ToolbarDPoint[ToolbarID] = *pCenterPoint;
			ToolbarReZoomScale[ToolbarID] = scale;
			ToolbarVPID[ToolbarID] = vpID;
		}
		ToolbarType[ToolbarID] = TBT_STANDARDMENU_DOCKED;
		ToolbarFloating[ToolbarID] = FALSE;
		ToolbarIDCur = ToolbarID;
		CreateDialog(hInst, "TOOLBAR_DOCKED", hWnd, (DLGPROC)TOOLBARMsgProc);
		DisplayToolbars = saveDisplayToolbars;
		ToolbarID = saveToolbarID;
		if (!ToolbarHandle[ToolbarID])
			ToolbarHandle[ToolbarID] = GSSiGlobAlloc (1570,GHND,MAX_TOOLBAR_BUTTONS*sizeof(TOOBAR_CONTROL_INFO));
		strcpy (ToolbarPath[ToolbarID],Pathname);
		//nToolbars++;
	}
	ToolBarStartPoint = SaveToolBarStartPoint;
	if (!stricmp (Type,"FLOAT"))
	{
		POINT	mp;

		ToolbarCurrentConfig[ToolbarID] = 0;
		ToolbarFloating[ToolbarID] = TRUE;
		DisplayMenuStatus[ToolbarID] = DMS_NOTDISPLAYED;
		ToolbarIDCur = ToolbarID;
		ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_FLOAT", hWnd, (DLGPROC)TOOLBARMsgProc);
		GetWindowRect (ToolbarWindow[ToolbarID],&rect);
		mp = RectMid (&rect);
		SetCursorPos (mp.x,mp.y);
		DoCreateDialogTooltip(ToolbarID); 
	}
	else if (!stricmp (Type,"DOCK"))
	{
		ToolbarCurrentConfig[ToolbarID] = 1;
		ToolbarFloating[ToolbarID] = FALSE;
		ToolbarIDCur = ToolbarID;
		ToolbarWindow[ToolbarID] = CreateDialog(hInst, "TOOLBAR_DOCKED", hWnd, (DLGPROC)TOOLBARMsgProc);
		DoCreateDialogTooltip(ToolbarID); 
	}
	else if (!stricmp (Type,"DOCK"))
		ToolbarWindow[ToolbarID] = hWnd;
	else
	{
		ToolbarWindow[ToolbarID] = hWnd;
		DoCreateDialogTooltip(ToolbarID); 
		g_hwndTrackingTT = ToolbarTTWindow[ToolbarID];
	}
	pTBInfo = GlobalLock (ToolbarHandle[ToolbarID]);
//	for (i=0;i<nToolbarControls[ToolbarID];i++,pTBInfo++)
//		AddToolToToolTip (GetDlgItem(ToolbarWindow[ToolbarID],pTBInfo->CntlID));
	GlobalUnlock (ToolbarHandle[ToolbarID]);
	if (CheckForDocked && !ToolbarFloating[ToolbarID])
		SeeIfToolbarShouldBeDocked (ToolbarWindow[ToolbarID],&ToolBarStartPoint);
	ConfigChangesMade = TRUE;
	return ToolbarId[ToolbarID];
}

HWND CreateToolbarWnd (HWND hWnd) 
{
	HWND rc;
	
	rc = CreateDialog(hInst, "TOOLBAR_FLOAT", hWnd, (DLGPROC)TOOLBARMsgProc); 
	return rc;
}

BOOL CreateToolbarWndx (HWND hWnd) 
{ 
 
   hWndToolbar = CreateToolbarEx (hWnd, 
      WS_CHILD | WS_BORDER | WS_VISIBLE |TBSTYLE_TRANSPARENT|TBSTYLE_TOOLTIPS , 
      IDR_TOOLBAR1, 
      5,                  // number of tools inside the bitmap 
      hInst, 
      IDR_TOOLBAR1,     // bitmap resource ID (can't use MAKEINTRESOURCE) 
      tbButtons, 
      TB_ENTRIES,0,0,0,0,sizeof(TBBUTTON)) ; 
 
   return (hWndToolbar ? TRUE : FALSE) ; 
 
}  // ToolbarInitializeApplication 
 
void ToolbarEnableButton (HWND hWndTB, int iButtonNum, BOOL bEnable) 
{ 
   SendMessage (hWndTB, TB_ENABLEBUTTON, iButtonNum, (LONG)bEnable) ; 
}  // ToolbarEnableButton 
 
void ToolbarDepressButton (HWND hWndTB, int iButtonNum, BOOL bDepress) 
{ 
   if (iButtonNum >= IDM_CFG1 && iButtonNum <= IDM_CFG2) 
      { 
      // these buttons are push button and will not stay down after 
      // each hit 
      SendMessage (hWndTB, TB_PRESSBUTTON, iButtonNum, (LONG)bDepress) ; 
      } 
   else 
      { 
      // for the four view buttons, have to use CHECKBUTTON so they 
      // will stay down after selected. 
      SendMessage (hWndTB, TB_CHECKBUTTON, iButtonNum, (LONG)bDepress) ; 
      } 
}  // ToolbarDepressButton 
 
void OnToolbarHit (WPARAM wParam, LPARAM lParam) 
{ 
 
   WORD  ToolbarHit ; 
 
   if (HIWORD(wParam) == TBN_ENDDRAG) 
      { 
      //StatusLineReady (hWndStatus) ; 
      } 
   else if (HIWORD(wParam) == TBN_BEGINDRAG) 
      { 
      ToolbarHit = LOWORD (lParam) ; 
 
      if (ToolbarHit >= IDM_CFG1 && 
          ToolbarHit <= IDM_CFG2) 
         { 
         ToolbarHit -= IDM_CFG1 ; 
 
         switch (iPerfmonView) 
            { 
            case IDM_CFG1: 
               ToolbarHit += IDM_CFG1 ; 
               break ; 
 
            case IDM_CFG2: 
               ToolbarHit += IDM_CFG2 ; 
               break ; 
 
            } 
         } 
/*      else if (ToolbarHit == IDM_TOOLBARBOOKMARK) 
         { 
         ToolbarHit = IDM_OPTIONSBOOKMARK ; 
         } 
      else if (ToolbarHit == IDM_TOOLBARREFRESH) 
         { 
         switch (iPerfmonView) 
            { 
            case IDM_VIEWALERT: 
               ToolbarHit = IDM_OPTIONSREFRESHNOWALERT ; 
               break ; 
 
            case IDM_VIEWLOG: 
               ToolbarHit = IDM_OPTIONSREFRESHNOWLOG ; 
               break ; 
 
            case IDM_VIEWREPORT: 
               ToolbarHit = IDM_OPTIONSREFRESHNOWREPORT ; 
               break ; 
 
            case IDM_VIEWCHART: 
            default: 
               ToolbarHit = IDM_OPTIONSREFRESHNOWCHART ; 
               break ; 
            } 
         } 
      else if (ToolbarHit == IDM_TOOLBAROPTIONS) 
         { 
         switch (iPerfmonView) 
            { 
            case IDM_VIEWALERT: 
               ToolbarHit = IDM_OPTIONSALERT ; 
               break ; 
 
            case IDM_VIEWLOG: 
               ToolbarHit = IDM_OPTIONSLOG ; 
               break ; 
 
            case IDM_VIEWREPORT: 
               ToolbarHit = IDM_OPTIONSREPORT ; 
               break ; 
 
            case IDM_VIEWCHART: 
            default: 
               ToolbarHit = IDM_OPTIONSCHART ; 
               break ; 
            } 
         } 
 
      StatusLine (hWndStatus, ToolbarHit) ; */
      } 
}  // OnToolBarHit 

void DrawBtnFocusRect (HWND BtnWnd)
{
	HDC	hDC = GetDC (BtnWnd);
	RECT	Rect;
	static	int		iStyle=1;

	GetClientRect (BtnWnd,&Rect);
	//InflateRect (&Rect,-2,-2);
//	DrawFocusRect(hDC, &Rect);
	FrameRect(hDC, &Rect,GetStockObject (BLACK_BRUSH));
	Draw3DFrame(BtnWnd, iStyle);
	if (iStyle == 1)
		iStyle = 2;
	else
		iStyle = 1;
	ReleaseDC (BtnWnd,hDC);
	return;
}

	LRESULT CALLBACK GetTBMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
	{ 
    MSG *lpmsg; 
	HWND	g_hwndDlg, g_hwndTT;
	int		ToolbarID;
	HHOOK	g_hhk;
	static	BOOL	HaveTrackMouseEvent=FALSE;
 
	lpmsg = (MSG *) lParam; 
	g_hwndDlg = GetParent (lpmsg->hwnd);
	ToolbarID = GetToolbarIDFromWnd (g_hwndDlg);
	if (ToolbarID < 0)
        return (CallNextHookEx(0, nCode, wParam, lParam)); 
	g_hwndDlg = ToolbarWindow[ToolbarID];
	g_hwndTT  = ToolbarTTWindow[ToolbarID];
//	g_hhk = ToolbarHook;//[ToolbarID];
    if (nCode < 0) 
        return (CallNextHookEx(0, nCode, wParam, lParam)); 
 
    switch (lpmsg->message) { 

        case WM_RBUTTONDOWN:
			ii=1;
		case WM_MOUSEMOVE:
		/*	if (!HaveTrackMouseEvent)
			{
				TRACKMOUSEEVENT EventTrack;

				EventTrack.dwFlags = TME_LEAVE;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				EventTrack.hwndTrack = lpmsg->hwnd;
				EventTrack.dwHoverTime = 0;
				TrackMouseEvent(&EventTrack);
				HaveTrackMouseEvent = TRUE;
	            DrawBtnFocusRect(lpmsg->hwnd);
			}*/
			ii = 1;
        case WM_LBUTTONDOWN: 
        case WM_LBUTTONUP: 
        case WM_RBUTTONUP: 
Next:
            if (g_hwndTT != NULL) { 
                MSG msg; 
 
                msg.lParam = lpmsg->lParam; 
                msg.wParam = lpmsg->wParam; 
                msg.message = lpmsg->message; 
                msg.hwnd = lpmsg->hwnd; 
 //               SendMessage(g_hwndTT, TTM_RELAYEVENT, 0, 
//                    (LPARAM) (LPMSG) &msg); 
            } 
            break; 
		case WM_MOUSELEAVE:
            DrawBtnFocusRect(lpmsg->hwnd);
 			HaveTrackMouseEvent = FALSE;
			break;
		case WM_CHAR:
			{
				switch (lpmsg->wParam)
				{
				case 'E':
					GMEdit (hWndMain,ToolbarPath[ToolbarID]);
					break;
				}
			}
			break;
       default: 
            break; 
    } 
    return (CallNextHookEx(0, nCode, wParam, lParam)); 
} 
LRESULT CALLBACK GetTBMoveMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
    MSG *lpmsg; 
	HWND	g_hwndDlg;
	int		ToolbarID;
	HHOOK	g_hhk;
	static	DWORD nMM=0;
	static	int	HaveCapture=0;
	static	BOOL	HaveTrackMouseEvent=FALSE;
    lpmsg = (MSG *) lParam; 
	if (lpmsg->message == WM_RBUTTONDOWN)
		ii=1;
	g_hwndDlg = lpmsg->hwnd;
	if ((ToolbarID = GetToolbarIDFromWnd (g_hwndDlg))<0)
	{
		g_hwndDlg = GetParent (g_hwndDlg);
		ToolbarID = GetToolbarIDFromWnd (g_hwndDlg);
		if ((ToolbarID = GetToolbarIDFromWnd (g_hwndDlg))<0)
		{
			g_hwndDlg = GetParent (g_hwndDlg);
			ToolbarID = GetToolbarIDFromWnd (g_hwndDlg);
		}
	}
	if (ToolbarID < 0)
        return (CallNextHookEx(0, nCode, wParam, lParam)); 
	g_hwndDlg = ToolbarWindow[ToolbarID];
	g_hhk = ToolbarMoveHook;//[ToolbarID];
    if (nCode < 0) 
        return (CallNextHookEx(g_hhk, nCode, wParam, lParam)); 
	if (nCode ==  HC_ACTION)

    switch (lpmsg->message) { 

        case WM_RBUTTONDOWN: 
			{
			   	POINT MousePoint = POINTStoPOINT(MAKEPOINTS(lpmsg->lParam));
				RECT	rect;

				if (GetDebug() && GetKeyState(VK_CONTROL) & 0x1000)
				{
					GMEdit(hWndMain, ToolbarPath[ToolbarID]);
					return 0;
				}

				GetClientRect (lpmsg->hwnd,&rect);

			//ToolbarBeginMove[ToolbarID] = POINTStoPOINT(MAKEPOINTS(lpmsg->lParam));
			//ClientToScreen (lpmsg->hwnd,&ToolbarBeginMove[ToolbarID]);
				if (PtInRect (&rect,MousePoint))
				{
					SetCapture (lpmsg->hwnd);
					HaveCapture = 1;
					PZStartMove (ToolbarID);
//					SetWindowPos (ToolbarWindow[ToolbarID],HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_DRAWFRAME|SWP_SHOWWINDOW);//|SWP_NOSIZE|SWP_NOMOVE);
					PostMessage (g_hwndDlg,WM_MOUSEMOVE,0,lpmsg->lParam);
				}
				//return 0;
			}
			break;
		case WM_MOUSEMOVE:
			
			if (GetCapture () == lpmsg->hwnd && lpmsg->wParam == MK_RBUTTON)
			{
				POINT movePoint = POINTStoPOINT(MAKEPOINTS(lpmsg->lParam));
				RECT	rect;
				
				ClientToScreen (lpmsg->hwnd,&movePoint);
				DisplayMenuStatus[ToolbarID] = DMS_WANTDIMMED;
				switch (HaveCapture)
				{
				case 0:
					goto Exit;
				case 1:
					ToolbarBeginMove[ToolbarID] = movePoint;
					SetWindowPos (ToolbarWindow[ToolbarID],HWND_NOTOPMOST,0,0,0,0,SWP_DRAWFRAME|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
					HaveCapture++;
					return 0;
				}
//				if (nMM++ % 2)
//					break;
				GetWindowRect (ToolbarWindow[ToolbarID],&rect);
				{
					int		iwidth  = RECTWIDTH (&rect);
					int		iheight = RECTHEIGHT (&rect);
					int		xmove   = movePoint.x - ToolbarBeginMove[ToolbarID].x;
					int		ymove   = movePoint.y - ToolbarBeginMove[ToolbarID].y;
					HWND	hPar = GetAncestor (ToolbarWindow[ToolbarID],GA_PARENT);
					POINT	newPt;

					newPt.x = rect.left + xmove;
					newPt.y = rect.top + ymove;
					ScreenToClient (hPar,&newPt);
					/*{
					 	char	str[128];
						sprintf (str,"%i %i %i %i",xmove,ymove,newPt.x,newPt.y);
						SetWindowText (hWndMain,str);
					}*/
					//MoveWindow(ToolbarWindow[ToolbarID], newPt.x, newPt.y, iwidth, iheight, TRUE);
					SetWindowPos (ToolbarWindow[ToolbarID],0, newPt.x, newPt.y, iwidth, iheight,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_DRAWFRAME|SWP_SHOWWINDOW);
					//Sleep (500);
				}
 				ToolbarBeginMove[ToolbarID] = movePoint;
				return 0;
			}
			break;
		/*	if (!HaveTrackMouseEvent)
			{
				TRACKMOUSEEVENT EventTrack;

				EventTrack.dwFlags = TME_LEAVE;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				EventTrack.hwndTrack = lpmsg->hwnd;
				EventTrack.dwHoverTime = 0;
				TrackMouseEvent(&EventTrack);
				HaveTrackMouseEvent = TRUE;
	            DrawBtnFocusRect(lpmsg->hwnd);
			}*/
        case WM_RBUTTONUP: 
			{
				POINT	pt = POINTStoPOINT(MAKEPOINTS(lpmsg->lParam));;
				
				if (GetCapture ()== lpmsg->hwnd)
				{
					ReleaseCapture ();
					HaveCapture = 0;
					ClientToScreen (lpmsg->hwnd,&pt);
					DisplayMenuStatus[ToolbarID] = DMS_NORMAL;
					GetWindowRect (ToolbarWindow[ToolbarID],&ToolbarRect[ToolbarID]);
					SeeIfToolbarShouldBeDocked (ToolbarWindow[ToolbarID],&pt);
					PZEndMove (ToolbarID);
					return 0;
				}
			}
            break; 
		case WM_MOUSELEAVE:
            DrawBtnFocusRect(lpmsg->hwnd);
 			HaveTrackMouseEvent = FALSE;
			break;
       default: 
            break; 
    } 
Exit:
    return (CallNextHookEx(g_hhk, nCode, wParam, lParam)); 
} 
 	// DoCreateDialogTooltip - creates a tooltip control for a dialog box, 
	//     enumerates the child control windows, and installs a hook 
	//     procedure to monitor the message stream for mouse messages posted 
	//     to the control windows. 
	// Returns TRUE if successful, or FALSE otherwise. 
	// 
	// Global variables 
	// g_hinst - handle to the application instance. 
	// g_hwndTT - handle to the tooltip control. 
	// g_hwndDlg - handle to the dialog box. 
	// g_hhk - handle to the hook procedure. 
 
	HWND DoCreateDialogTooltip(int ToolbarID) 
	{ 
		HWND	g_hwndDlg;

    // Ensure that the common control DLL is loaded, and create 
    // a tooltip control. 
//    InitCommonControls(); 
    ToolbarTTWindow[ToolbarID] = CreateWindowEx(0, TOOLTIPS_CLASS, (LPSTR) NULL, 
												WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
												ToolbarWindow[ToolbarID], (HMENU) NULL, hInst, NULL); 
 
 
    if (ToolbarTTWindow[ToolbarID] == NULL) 
        return NULL; 
 
    // Enumerate the child windows to register them with the tooltip
    // control. 
	g_hwndDlg = ToolbarWindow[ToolbarID];
/*    if (!ToolbarFloating[ToolbarID] || ToolbarType[ToolbarID] == TBT_ZOOMPAN)
		ToolbarMoveHook[ToolbarID] = SetWindowsHookEx(WH_GETMESSAGE, GetTBMoveMsgProc,(HINSTANCE) NULL, GetCurrentThreadId()); 
	else
		ToolbarMoveHook[ToolbarID] = NULL;*/
    if (!ToolbarFloating[ToolbarID] || ToolbarType[ToolbarID] == TBT_ZOOMPAN && !ToolbarMoveHook)
		ToolbarMoveHook = SetWindowsHookEx(WH_GETMESSAGE, GetTBMoveMsgProc,(HINSTANCE) NULL, GetCurrentThreadId()); 
	if (ToolbarType[ToolbarID] == TBT_ZOOMPAN || ToolbarType[ToolbarID] ==TBT_VISMENU)
	{
		TOOLINFO ti; 

		memset (&ti,0,sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO); 
		ti.hwnd = ToolbarWindow[ToolbarID]; 
		ti.uId = 1;
		ti.hinst = hInst;
		ti.uFlags = TTF_SUBCLASS;// | TTF_TRACK;
		GetClientRect (ToolbarWindow[ToolbarID],&ti.rect);
		ti.lpszText = LPSTR_TEXTCALLBACK; 
		toolItem[ToolbarID] = ti;
		ii=SendMessage(ToolbarTTWindow[ToolbarID], TTM_ADDTOOL, 0, 
			(LPARAM) (LPTOOLINFO) &ti); 
	} 
	else if (!EnumChildWindows(g_hwndDlg, (WNDENUMPROC) EnumChildProcTT, ToolbarID)) 
        return NULL; 
 
    // Install a hook procedure to monitor the message stream for mouse 
    // messages intended for the controls in the dialog box. 
    /*ToolbarHook[ToolbarID] = SetWindowsHookEx(WH_GETMESSAGE, GetTBMsgProc, 
        (HINSTANCE) NULL, GetCurrentThreadId()); 
 
    if (ToolbarHook[ToolbarID] == (HHOOK) NULL) 
		return FALSE;*/

/*	if (!ToolbarHook)
	{
		ToolbarHook = SetWindowsHookEx(WH_GETMESSAGE, GetTBMsgProc, 
			(HINSTANCE) NULL, GetCurrentThreadId()); 
	 
		if (ToolbarHook == (HHOOK) NULL) 
			return FALSE; 
 
	}*/
 
    SendMessage(ToolbarTTWindow[ToolbarID], TTM_ACTIVATE, TRUE, 0);
    return ToolbarTTWindow[ToolbarID]; 
	} 
 
	// EmumChildProc - registers control windows with a tooltip control by
	//     using the TTM_ADDTOOL message to pass the address of a 
	//     TOOLINFO structure. 
	// Returns TRUE if successful, or FALSE otherwise. 
	// hwndCtrl - handle of a control window. 
	// lParam - application-defined value (not used). 
	BOOL CALLBACK EnumChildProcTT(HWND hwndCtrl, LPARAM lParam) 
	{ 
    TOOLINFO ti; 
    char szClass[64];
	int	ToolbarID = lParam;

	if (ToolbarID < 0)
		return FALSE;
 
    // Skip static controls. 
    GetClassName(hwndCtrl, szClass, sizeof(szClass)); 
    if (IsWindowVisible (hwndCtrl) && lstrcmpi(szClass, "STATIC")) { 
        ti.cbSize = sizeof(TOOLINFO); 
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS ; 
        ti.hwnd = ToolbarWindow[ToolbarID]; 
        ti.uId = (UINT) hwndCtrl; 
        ti.hinst = hInst; 
        ti.lpszText = LPSTR_TEXTCALLBACK; 
        SendMessage(ToolbarTTWindow[ToolbarID], TTM_ADDTOOL, 0, 
            (LPARAM) (LPTOOLINFO) &ti); 
    } 
    return TRUE; 
	} 
 
	// GetMsgProc - monitors the message stream for mouse messages intended 
	//     for a control window in the dialog box. 
	// Returns a message-dependent value. 
	// nCode - hook code. 
	// wParam - message flag (not used). 
	// lParam - address of an MSG structure. 
 
	// OnWMNotify - provides the tooltip control with the appropriate text 
	//     to display for a control window. This function is called by 
	//     the dialog box procedure in response to a WM_NOTIFY message. 
	// lParam - second message parameter of the WM_NOTIFY message. 
	BOOL OnWMNotify(HWND hWndDlg,LPARAM lParam) 
	{ 
    LPTOOLTIPTEXT lpttt; 
	LPNMHDR lpnmhdr = (LPNMHDR)lParam;
    int idCtrl, i; 
	int	ToolbarID = GetToolbarIDFromWnd2 (hWndDlg);
	static	char text[256];
 
	if (ToolbarID < 0)
		return FALSE;
	if (lpnmhdr->code == NM_HOVER)
		return FALSE;
	*text = 0;
    if (lpnmhdr->code == TTN_NEEDTEXT)
	{
		lpttt = (LPTOOLTIPTEXT) lParam; 
//			SetWindowPos (ToolbarTTWindow[ToolbarID],ToolbarWindow[nToolbars-1],0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);

		if (ToolbarType[ToolbarID] == TBT_ZOOMPAN)
		{
			GetLastPrompt (text);
			lpttt->lpszText = text; 
			return TRUE;
		}
		else if (ToolbarType[ToolbarID] == TBT_VISMENU)
		{
			GetLastPrompt (text);
			lpttt->lpszText = text; 
			return TRUE;
		}
		else if (ToolbarHandle[ToolbarID])
		{
			LPTOOBAR_CONTROL_INFO	pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock (ToolbarHandle[ToolbarID]);
			idCtrl = GetDlgCtrlID((HWND) ((LPNMHDR) lParam)->idFrom); 
			for (i=0;i<nToolbarControls[ToolbarID];i++,pTBInfo++)
				if (pTBInfo->CntlID == idCtrl)
					strcpy (text,pTBInfo->Label);
			GlobalUnlock (ToolbarHandle[ToolbarID]);
			lpttt->lpszText = text; 
			if (*ToolbarHoverCmd[ToolbarID])
				ii = SetTimer(hWndDlg, 1, 5000, 0);
			return TRUE;
		}
	} 
	return FALSE; 
} 
 
int DisplayBitmapOnButton (HDC hDCBtn,LPRECT pRectBtn,HBITMAP hBM)
{
	BOOL	rc=FALSE;
	BITMAP	bm;

	if (GetObject(hBM, sizeof(bm), (LPSTR)&bm))
	{
	    HDC	hDC2 = CreateCompatibleDC(hDCBtn); 
		HBITMAP	hBMOld = SelectObject(hDC2,hBM);

		SetStretchBltMode (hDC2, COLORONCOLOR);

	    rc = StretchBlt(hDCBtn,0,0,pRectBtn->right,pRectBtn->bottom,
					    hDC2,0,0,bm.bmWidth,bm.bmHeight,
						SRCCOPY);
		SelectObject(hDC2, hBMOld);
		DeleteDC(hDC2);
		rc = TRUE;
	}
	return rc;
}

BOOL GetFileNameFromLink (LPSTR FileName)
{
	HFILE	Fid;
	BOOL	rtn = FALSE;

	strlwr (FileName);
	if (strstr (FileName,".glk"))
	{
		Fid = GSSiOpenFile (FileName,0,OF_READ);
		fgetstring (FileName,MAX_PATH,Fid);
		GSSiClose (Fid);
		rtn = TRUE;
	}
	return rtn;
}

BOOL CALLBACK PICTVIEWERMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	HDIB	hDIB;
	HPALETTE	hPal;
	RECT	rect, WindRect;
	LPRECT	lprc;
	int		nFiles, nWidth, nHeight,fwSide, n, ii, TotLen, CurLoc;
	static	int	CurFile;
	LPSTR	pBS;
	char	txt[128],str[256];
	HWND	hWndPict;
	HDC		hDC;
	BOOL	rtn;
	HFILE	Fid;
	LPSTR	pDot;
	char	ToFile[MAX_PATH], LinkFile[MAX_PATH];
static	BOOL	LinkToFile=TRUE;
static	char	FileName[MAX_PATH];
static	HDIB32	hDib32=0;
static	int	imagelft, imagergt, imagebot, imagetop;
static	int	nxw, nxh, nxlft, nxbot, prvlft, prvbot,DisplayHeight,ImageWidth,ImageHeight;

 switch(Message)
   {
/*	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MAXIMIZED:
	        PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCURFILE, 0L);
			break;
		}
		break;
	case WM_SIZING:
		fwSide = wParam;
		lprc = (LPRECT) lParam; 
		ToolbarID = GetToolbarIDFromWnd (hWndDlg);
		switch (fwSide)
		{
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				break;
			case WMSZ_LEFT:
			case WMSZ_RIGHT:
				if (nToolbarRows[ToolbarID] == 1)
				{
					*lprc = ToolbarRect[ToolbarID];
					break;
				}
				lprc->bottom += 10;
				break;
			case WMSZ_TOP:
			case WMSZ_TOPLEFT: 
			case WMSZ_TOPRIGHT:
				break;
		}
		return TRUE;
		break;*/
//	case WM_SIZING:
	case WM_SIZE:
		switch (wParam)
		{
		default:
			return FALSE;
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
			break;
		}
	case WM_EXITSIZEMOVE:
		GetClientRect (hWndDlg,&WindRect);
		hWndPict = GetDlgItem (hWndDlg,IDC_IMAGE);
		hDC = GetDC (hWndPict);
		GetClientRect (hWndPict,&rect);
		FillRect (hDC,&rect,GetStockObject(DC_BRUSH));
		SetWindowPos(GetDlgItem (hWndDlg,IDC_IMAGE),HWND_TOP,imagelft,imagetop,
									   WindRect.right-WindRect.left-imagergt,
									   WindRect.bottom-WindRect.top-imagebot,SWP_NOZORDER|SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem (hWndDlg,IDC_NEXTPICT),HWND_TOP,WindRect.right-nxlft,WindRect.bottom-nxbot,
									   nxw,nxh,SWP_NOZORDER|SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem (hWndDlg,IDC_PREVPICT),HWND_TOP,WindRect.left+prvlft,WindRect.bottom-prvbot,
									   nxw,nxh,SWP_NOZORDER|SWP_SHOWWINDOW);
		GetClientRect (hWndPict,&rect);
		FillRect (hDC,&rect,GetStockObject(DC_BRUSH));
		if (hDib32)
		{
			DisplayHeight = DisplayBMInRect32 (hDC,hDib32,rect,TRUE);
		}
		ReleaseDC (hWndPict,hDC);
        PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCURFILE, 0L);
		break;
	case WM_DROPFILES:
    {
    	HANDLE	hFile = (HANDLE)wParam;   
    	HANDLE	hFileMem=GSSiGlobAlloc (1525,GMEM_MOVEABLE,256);
    	LPSTR	pFile = GlobalLock (hFileMem);
    	int	n=0, LocAdded;  

		while (DragQueryFile (hFile,n,pFile,256)) 
    	{
    	 	//GetLongPathName (pFile,256);
    	 	if (FileType (pFile) == 1)
    	 	{
				LPSTR	pBS=strrchr (pFile,'\\');

				TotLen = GSSiLength (pFile);
				CurLoc = 0;
				if (pBS)
				{
					sprintf (ToFile,"%s%s",PictViewerDir,pBS);

					if (LinkToFile)
					{
						strcpy (LinkFile,ToFile);	
						pDot = strrchr (LinkFile,'.');
						strcpy (pDot,".glk");
						Fid = GSSiOpenFile (LinkFile,0,OF_CREATE);
						fputstring (pFile,Fid);
						GSSiClose (Fid);
					}
					else
					{
						TotLen = GSSiLength (str);
						CurLoc = 0;
						rtn = copyfile (ToFile, pFile,FALSE,0,0,hWndDlg,IDC_IMAGENAME,TotLen,&CurLoc);
					}
				}
			}
    	 	n++;
    	 } 
    	 GSSiGlobUlFree (&hFileMem);  
    	 DragFinish (hFile);
         PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
	}
		break;
    case WM_INITDIALOG:
         /* initialize working variables                                */  
        // DebugInfoWnd = hWndDlg;
		switch (FileType(PictViewerDir))
		{
		case 0:
			return 0;
		case 1://file
			if ((pBS = strrchr(PictViewerDir, '\\')))
			{
				*pBS++ = 0;
				strcpy(FileName, pBS);
			}
			break;
		case 2://directory
			*FileName = 0;
			break;
		}
		pBS = strrchr (PictViewerDir,'\\');
		SetWindowText (hWndDlg,pBS+1);
        cwCenter(hWndDlg, 0);
		GetClientRect (hWndDlg,&WindRect);
		ClientRectToScreenRect (hWndDlg,&WindRect);
		GetWindowRect (GetDlgItem (hWndDlg,IDC_IMAGE),&rect);
		imagetop = rect.top - WindRect.top;
		imagebot = (WindRect.bottom - WindRect.top) - (rect.bottom - rect.top);
		imagergt = (WindRect.right - WindRect.left) - (rect.right - rect.left);
		imagelft = rect.left - WindRect.left;
		GetWindowRect (GetDlgItem (hWndDlg,IDC_NEXTPICT),&rect);
		nxw = rect.right - rect.left;
		nxh = rect.bottom - rect.top;
		nxlft = WindRect.right - rect.left;
		nxbot = WindRect.bottom - rect.top;
		GetWindowRect (GetDlgItem (hWndDlg,IDC_PREVPICT),&rect);
		prvlft = rect.left - WindRect.left;
		prvbot = WindRect.bottom - rect.top;
    case GSSI_REINITDIALOG:
	{
		char	ListFile[256], test[256]; 
	
		SendDlgItemMessage (hWndDlg,IDC_PICTLIST,LB_RESETCONTENT,0,0);
		GSSiGetTempFileName (0,"gt",0,ListFile); 
		Fid = GSSiOpenFile (ListFile,0,OF_CREATE);   
		SearchFilesInDir (PictViewerDir, "", Fid,&n,"*.*",-1,TRUE,TRUE);   
		GSSiClose (Fid);
		Fid = GSSiOpenFile (ListFile,0,OF_READ);  
		while (fgetstring (str,250,Fid))
           	SendDlgItemMessage (hWndDlg,IDC_PICTLIST,LB_ADDSTRING,0,(LPARAM)str);
		GSSiClose (Fid);
		GSSiRemove (ListFile);
		CurFile = 0;
        PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCURFILE, 0L);
	}
        break; /* End of WM_INITDIALOG                                 */

	case WM_DRAWITEM:
		{
			UINT	idCtl = (UINT) wParam;             // control identifier 
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; // item-drawing information 
			int		iState = 0;
			int		iBtn;
			HBITMAP	hBM, hBMPrev;

			if (idCtl == IDC_PREVPICT)
				iBtn = 0;
			else if (idCtl == IDC_NEXTPICT)
				iBtn = 1;
			else
				break;

			switch (lpdis->itemAction)
			{  
				case ODA_SELECT:
					if (lpdis->itemState & ODS_SELECTED)
						iState = 1;
				case ODA_DRAWENTIRE:
					if (lpdis->itemState & ODS_DISABLED)
						iState = 2;
					hBM = LoadBitmap (hInst,MAKEINTRESOURCE(PictBtnBM[iBtn][iState]));
					DisplayBitmapOnButton (lpdis->hDC,&lpdis->rcItem,hBM);
					DeleteObject (hBM);
					return TRUE;
					break;
				case ODA_FOCUS:
				 //    HandleFocusState(lpdis);
					  break;
				default:
					ii=1;
					break;
			}  //itemAction
		}
		break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
		 DestroyWindow (hWndDlg);
         break; /* End of WM_CLOSE                                      */
	case WM_DESTROY:
		 hDib32 = GMDestroyDIB32 (hDib32);
		 ImageWnd = 0;
		 break;
	case WM_NOTIFY:
		 return OnWMNotify(hWndDlg,lParam);
    case WM_COMMAND:
		{
	         switch(LOWORD(wParam))
			 {
			 case IDM_EXIT:
				 PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
				 break;
			 case IDM_PRINT:
             	 PrintImage (hWndDlg,FileName,1);
				 break;
			 case IDM_VIEWFOLDER:
				 sprintf (txt,"$WEB(%s)",PictViewerDir);
				 ProcessText (txt);
			     PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
				 break;
			 case IDM_MAKEINFOBOX:
				 {
					 if (!*FileName)
						 break;
					 GetClientRect (GetDlgItem (hWndDlg,IDC_IMAGE),&rect);
				 	 SetViewport(*pCommandViewport);
					 sprintf (str,"%s(%f,%f)",FileName,(float)((DisplayHeight*((double)ImageWidth/ImageHeight))*CurView->BaseUnitsPerPixel),
														(float)(DisplayHeight*CurView->BaseUnitsPerPixel)),
					 SetGlobalValue ("PICTIMAGE",str);
					 _splitpath (FileName,0,0,str,0);
					 SetGlobalValue ("PICTIMAGETEXT",str); 
					 sprintf (str,"$INFOBOX(CREATE,[%%DL]infobox\\pictimage.inb,ITEM,[%%PICKED_ITEM])");
					 ProcessText("[PICTIMAGETEXT]=$GETMLCVAL(Enter Description (Optional),PICTIMAGETEXT);");
					 ProcessText (str);
				 }
				 break;
			 case IDC_GETFILES:
				{
    				 HANDLE	hFileMem=GSSiGlobAlloc (0,GHND,USHRT_MAX);
    				 LPSTR	ImageFiles = GlobalLock (hFileMem);
					 LPSTR	pName = ImageFiles;
					 if (GetMultFiles(hWndDlg,ImageFiles,USHRT_MAX,IDS_FILTERALLIMAGE,IDS_FILEIMAGE)) 
					 {
						 pName = strchr (pName,0)+1;
						 while (*pName)
						 {
							sprintf (str,"%s\\%s",ImageFiles,pName);
							sprintf (ToFile,"%s\\%s",PictViewerDir,pName);
							if (LinkToFile)
							{
								strcpy (LinkFile,ToFile);	
								pDot = strrchr (LinkFile,'.');
								strcpy (pDot,".glk");
								Fid = GSSiOpenFile (LinkFile,0,OF_CREATE);
								fputstring (str,Fid);
								GSSiClose (Fid);
							}
							else
							{
								TotLen = GSSiLength (str);
								CurLoc = 0;
								rtn = copyfile (ToFile, str,FALSE,0,0,hWndDlg,IDC_IMAGENAME,TotLen,&CurLoc);
							}
							//SendDlgItemMessage (hWndDlg,IDC_PICTLIST,LB_ADDSTRING,0,(LPARAM)str);
							pName = strchr (pName,0)+1;
						 }
					 }
					 GSSiGlobUlFree (&hFileMem);
			         PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
				}
				break;
				case IDC_PREVPICT:
					CurFile--;
			        PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCURFILE, 0L);
				break;
				case IDC_NEXTPICT:
					CurFile++;
			        PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCURFILE, 0L);
				break;
				case IDC_IMAGE:
					ii=LOWORD(wParam);
					break;
				case IDC_SHOWCURFILE:
				{

					EnableWindow (GetDlgItem(hWndDlg,IDC_PREVPICT),CurFile);
		           	nFiles = SendDlgItemMessage (hWndDlg,IDC_PICTLIST,LB_GETCOUNT,0,0);
					if (CurFile >= nFiles-1)
					{
						CurFile = max (0,nFiles-1);
						EnableWindow (GetDlgItem(hWndDlg,IDC_NEXTPICT),FALSE);
					}
					else
						EnableWindow (GetDlgItem(hWndDlg,IDC_NEXTPICT),TRUE);
                    if (SendDlgItemMessage(hWndDlg,IDC_PICTLIST,LB_GETTEXT,CurFile,(DWORD)FileName) != LB_ERR) 
					{
						LPSTR	pBS = strrchr (FileName,'\\');

						SetDlgItemText (hWndDlg,IDC_IMAGENAME,pBS+1);
						hDib32 = GMDestroyDIB32 (hDib32);
						GetFileNameFromLink (FileName);
						if ((hDib32 = BMPHandleFromEXT (FileName)))
						{
							hWndPict = GetDlgItem (hWndDlg,IDC_IMAGE);
							hDC = GetDC (hWndPict);
							GetClientRect (hWndPict,&rect);
							rect.right++;
							rect.bottom++;
							FillRect (hDC,&rect,GetStockObject(DC_BRUSH));
							rect.right--;
							rect.bottom--;
							GetDIBDimensionsFromHandle (hDib32,&ImageHeight,&ImageWidth);
							DisplayHeight = DisplayBMInRect32 (hDC,hDib32,rect,TRUE);
							ReleaseDC (hWndPict,hDC);
						}
					}
				}
				break;
			 }

		}
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL ViewImage (HWND hWnd,LPSTR ImagePath)
{
	
	strcpy (PictViewerDir,ImagePath);
	if (!ImageWnd)
		ImageWnd = CreateDialog(hInst, "PICTVIEWER", hWnd, (DLGPROC)PICTVIEWERMsgProc);
	else
        PostMessage(ImageWnd, GSSI_REINITDIALOG, 0, 0L);

	return TRUE;
}

BOOL SetToolbarConfig (int ToolbarID,int ConfigNum)
{
	RECT	rect;

	if (ToolbarID < 0)
		return FALSE;
	if (!GetWindowRect (ToolbarWindow[ToolbarID],&rect))
		return FALSE;
	ToolbarRect[ToolbarID] = rect;
	ToolbarConfigs[ToolbarID][ConfigNum].cx = RECTWIDTH (&rect);
	ToolbarConfigs[ToolbarID][ConfigNum].cy = RECTHEIGHT (&rect);
	return TRUE;
}

BOOL AdjustToolbarWindowRect (HWND hWnd)
{
	int ToolbarID = GetToolbarIDFromWnd (hWnd);
	RECT	rect;
	RECT	mainRect;
	int	w,h;
	BOOL	adjust=FALSE;

	if (ToolbarID < 0)
		return FALSE;
	GetWindowRect (hWnd,&rect);
	w = RECTWIDTH(&rect);
	h = RECTHEIGHT(&rect);
	GetClientRect (hWndMain,&mainRect);
	if (w > RECTWIDTH(&mainRect))
	{
		w = RECTWIDTH(&mainRect);
		adjust = TRUE;
	}
	if (h > RECTHEIGHT(&mainRect))
	{
		h = RECTHEIGHT(&mainRect);
		adjust = TRUE;
	}
	if (adjust)
	{
		//MoveWindow (hWnd,rect.left,rect.top,w,h,TRUE);
		SetWindowPos (hWnd,0,rect.left,rect.top,w,h,SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
		return TRUE;
	}
	return FALSE;
}
void DrawAlphaBlend (HWND hWnd, HDC hdcwnd)
{
    HDC hdc;               // handle of the DC we will create  
    BLENDFUNCTION bf;      // structure for alpha blending 
    HBITMAP hbitmap;       // bitmap handle 
    BITMAPINFO bmi;        // bitmap header 
    VOID *pvBits;          // pointer to DIB section 
    ULONG   ulWindowWidth, ulWindowHeight;      // window width/height 
    ULONG   ulBitmapWidth, ulBitmapHeight;      // bitmap width/height 
    RECT    rt;            // used for getting window dimensions 
    UINT32   x,y;          // stepping variables 
    UCHAR ubAlpha;         // used for doing transparent gradient 
    UCHAR ubRed;        
    UCHAR ubGreen;
    UCHAR ubBlue;
    float fAlphaFactor;    // used to do premultiply 
            
    // get window dimensions 
    GetClientRect(hWnd, &rt);
    
    // calculate window width/height 
    ulWindowWidth = rt.right - rt.left;  
    ulWindowHeight = rt.bottom - rt.top;  

    // make sure we have at least some window size 
    if ((!ulWindowWidth) || (!ulWindowHeight))
        return;

    // divide the window into 3 horizontal areas 
    ulWindowHeight = ulWindowHeight / 3;

    // create a DC for our bitmap -- the source DC for AlphaBlend  
    hdc = CreateCompatibleDC(hdcwnd);
    
    // zero the memory for the bitmap info 
    ZeroMemory(&bmi, sizeof(BITMAPINFO));

    // setup bitmap info  
    // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth - (ulWindowWidth/5)*2;
    bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight - (ulWindowHeight/5)*2;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

    // create our DIB section and select the bitmap into the dc 
    hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
    SelectObject(hdc, hbitmap);

    // in top window area, constant alpha = 50%, but no source alpha 
    // the color format for each pixel is 0xaarrggbb  
    // set all pixels to blue and set source alpha to zero 
    for (y = 0; y < ulBitmapHeight; y++)
        for (x = 0; x < ulBitmapWidth; x++)
            ((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x000000ff; 

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 0x7f;  // half of 0xff = 50% transparency 
    bf.AlphaFormat = 0;             // ignore source alpha channel 

    if (!AlphaBlend(hdcwnd, ulWindowWidth/5, ulWindowHeight/5, 
                    ulBitmapWidth, ulBitmapHeight, 
                    hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf))
        return;                     // alpha blend failed 
    
    // in middle window area, constant alpha = 100% (disabled), source  
    // alpha is 0 in middle of bitmap and opaque in rest of bitmap  
    for (y = 0; y < ulBitmapHeight; y++)
        for (x = 0; x < ulBitmapWidth; x++)
            if ((x > (int)(ulBitmapWidth/5)) && (x < (ulBitmapWidth-ulBitmapWidth/5)) &&
                (y > (int)(ulBitmapHeight/5)) && (y < (ulBitmapHeight-ulBitmapHeight/5)))
                //in middle of bitmap: source alpha = 0 (transparent). 
                // This means multiply each color component by 0x00. 
                // Thus, after AlphaBlend, we have a, 0x00 * r,  
                // 0x00 * g,and 0x00 * b (which is 0x00000000) 
                // for now, set all pixels to red 
                ((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x00ff0000; 
            else
                // in the rest of bitmap, source alpha = 0xff (opaque)  
                // and set all pixels to blue  
                ((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0xff0000ff; 
    
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha  
    bf.SourceConstantAlpha = 0xff;  // opaque (disable constant alpha) 
   
    if (!AlphaBlend(hdcwnd, ulWindowWidth/5, ulWindowHeight/5+ulWindowHeight, ulBitmapWidth, ulBitmapHeight, hdc, 0, 0, ulBitmapWidth, ulBitmapHeight, bf))
        return;

    // bottom window area, use constant alpha = 75% and a changing 
    // source alpha. Create a gradient effect using source alpha, and  
    // then fade it even more with constant alpha 
    ubRed = 0x00;
    ubGreen = 0x00;
    ubBlue = 0xff;
    
    for (y = 0; y < ulBitmapHeight; y++)
        for (x = 0; x < ulBitmapWidth; x++)
        {
            // for a simple gradient, base the alpha value on the x  
            // value of the pixel  
            ubAlpha = (UCHAR)((float)x / (float)ulBitmapWidth * 255);
            //calculate the factor by which we multiply each component 
            fAlphaFactor = (float)ubAlpha / (float)0xff; 
            // multiply each pixel by fAlphaFactor, so each component  
            // is less than or equal to the alpha value. 
            ((UINT32 *)pvBits)[x + y * ulBitmapWidth] 
                = (ubAlpha << 24) |                       //0xaa000000 
                 ((UCHAR)(ubRed * fAlphaFactor) << 16) |  //0x00rr0000 
                 ((UCHAR)(ubGreen * fAlphaFactor) << 8) | //0x0000gg00 
                 ((UCHAR)(ubBlue   * fAlphaFactor));      //0x000000bb 
        }

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.AlphaFormat = AC_SRC_ALPHA;   // use source alpha  
    bf.SourceConstantAlpha = 0xbf;   // use constant alpha, with  
                                     // 75% opaqueness 

    AlphaBlend(hdcwnd, ulWindowWidth/5, 
               ulWindowHeight/5+2*ulWindowHeight, ulBitmapWidth, 
               ulBitmapHeight, hdc, 0, 0, ulBitmapWidth, 
               ulBitmapHeight, bf);

    // do cleanup 
    DeleteObject(hbitmap);
    DeleteDC(hdc);
    
}

void SaveToolbarsInConfig (HFILE Fid)
{
 	short	Version=1, id=OB_TOOLBARS; 
    int		i;
	long	loc, loc2,Length=0;
	RECT	rect;
	char	path[MAX_PATH];
    
 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1); 
	loc = GSSillseek (Fid,0,1);
	BigWrite (Fid,(HPSTR)&Length,4,-1);
	BigWrite (Fid,(HPSTR)&nToolbars,4,-1);Length+=4;
	
	BigWrite (Fid,(HPSTR)&ToolbarWidthTop,4,-1);Length+=4;
	BigWrite (Fid,(HPSTR)&ToolbarWidthBottom,4,-1);Length+=4;
	BigWrite (Fid,(HPSTR)&ToolbarWidthLeft,4,-1);Length+=4;
	BigWrite (Fid,(HPSTR)&ToolbarWidthRight,4,-1);Length+=4;

	for (i=0;i<nToolbars;i++)
	{
		BigWrite (Fid,(HPSTR)&ToolbarType[i],4,-1);Length+=4;
		BigWrite (Fid,(HPSTR)&ToolbarFloating[i],4,-1);Length+=4;
		BigWrite (Fid,(HPSTR)&ToolbarConfigNumPerRow[i][0],4,-1);Length+=4;
		BigWrite (Fid,(HPSTR)&ToolbarHeight[i],4,-1);Length+=4;
		rect = ToolbarRect[i];
		ScreenRectToClientRect (hWndMain,&rect);
		BigWrite (Fid,(HPSTR)&rect,sizeof(RECT),-1);Length+=sizeof(RECT);
		strcpy (path,ToolbarPath[i]);
		SubstituteDL (path,FALSE);
		BigWrite (Fid,(HPSTR)path,MAX_PATH,-1);Length+=MAX_PATH;
	}
	loc2 = GSSillseek (Fid,0,1);
	GSSillseek (Fid,loc,0);
	BigWrite (Fid,(HPSTR)&Length,4,-1);
	GSSillseek (Fid,loc2,0);
	return;
}

void LoadToolbarsInConfig (HFILE Fid)
{
	int		nToolbar, iType, iFloat,i, npr, h;
	RECT	rect, windowRect;
	char	path[MAX_PATH];
	char	cmd[1024];
	POINT	pt;

	SetViewport (0);
	DisplayToolbars = FALSE;
	GSSilread (Fid,&nToolbar,4);
	GetWindowRect(hWndMain, &windowRect);

	GSSilread (Fid,&ToolbarWidthTop,4);
	GSSilread (Fid,&ToolbarWidthBottom,4);
	GSSilread (Fid,&ToolbarWidthLeft,4);
	GSSilread (Fid,&ToolbarWidthRight,4);

	for (i=0;i<nToolbar;i++)
	{
		GSSilread (Fid,&iType,4);
		GSSilread (Fid,&iFloat,4);
		GSSilread (Fid,&npr,4);
		GSSilread (Fid,&h,4);
		GSSilread (Fid,&rect,sizeof(RECT));
		GSSilread (Fid,path,MAX_PATH);

		ClientRectToScreenRect (hWndMain,&rect);

		switch (iType)
		{
		case TBT_STANDARDMENU_DOCKED:
			pt = RectMid (&rect);
			pt.x = rect.left;
			pt.y = rect.top;
			sprintf(cmd, "$TOOLBAR(LOAD,DOCK,%s,%i,%i,%i %i)", path, h, npr, pt.x, pt.y);
			ProcessText (cmd);
			break;
		case TBT_ZOOMPAN:
			{
				POINT pt=RectMid (&rect);

				sprintf (cmd,"$DIALOG(LOAD,PANZOOMROTATE,%i %i,F)",pt.x,pt.y);
				ProcessText (cmd);
			}
			break;
		case TBT_VISMENU:
			break;
			sprintf (cmd,"$VIS(CONTROL,,%i %i %i %i,0.80)",rect.left,rect.top,rect.right,rect.bottom);
			ProcessText (cmd);
			break;
		}
	}
	DisplayToolbars = TRUE;
	AdjustToolbarPositions ();
	return;

}
LRESULT CALLBACK TabSubclassProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern WNDPROC	g_OrigTabProc;

 	switch (message)
	{
	case WM_RBUTTONDOWN:
		{
			HWND	hWndPar = GetParent (hwnd);
			DisplayTabbedMenu (hWndPar);

		}
		break;

	case WM_MOUSEMOVE:
		{
			TCHITTESTINFO info;
			int	iTab, ToolbarID;
			static	int	lastTab=-1;

			info.pt = POINTStoPOINT(MAKEPOINTS(lParam));
			iTab = TabCtrl_HitTest(hwnd,&info);
			if (iTab != lastTab)
			{
				lastTab = iTab;
				ToolbarID = GetToolbarIDFromWnd2 (hwnd);
				if (iTab >= 0 && ToolbarID >= 0)
				{
					toolItem[ToolbarID].lpszText = lastPrompt;
					SendMessage(ToolbarTTWindow[ToolbarID], TTM_UPDATETIPTEXT, 0, (LPARAM)&toolItem[ToolbarID]);
				}
			}

		}
		break;

	case WM_NOTIFY:
		{
			OnWMNotify(hwnd,lParam);
		}
		break;

	}
	return CallWindowProc (g_OrigTabProc, hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT message,WPARAM wParam, LPARAM lParam)
{
	HWND	hWndPar = GetParent(hwnd);
	int		ToolbarID = GetToolbarIDFromWnd2(hwnd);
	int		ButtonNumber = 0;
	LPTOOBAR_CONTROL_INFO pTBInfo;
	WNDPROC	g_OrigTabProc;

	if (ToolbarID < 0)
		return DefWindowProc(hwnd, message, wParam, lParam);
	if (!ToolbarHandle[ToolbarID])
		return DefWindowProc(hwnd, message, wParam, lParam);

	pTBInfo = (LPTOOBAR_CONTROL_INFO)GlobalLock(ToolbarHandle[ToolbarID]);
	for (ButtonNumber = 0; ButtonNumber < nToolbarControls[ToolbarID]; ButtonNumber++,pTBInfo++)
	{
		if (GetDlgItem(ToolbarWindow[ToolbarID], pTBInfo->CntlID) == hwnd)
			goto foundControl;
	}
	GlobalUnlock(ToolbarHandle[ToolbarID]);
	return DefWindowProc(hwnd, message, wParam, lParam);

foundControl:
	g_OrigTabProc = pTBInfo->wndProc;
	GlobalUnlock(ToolbarHandle[ToolbarID]);

	switch (message)
	{
		case WM_MOUSEMOVE:

		break;

		case WM_MOUSEHOVER:
			ProcessText(ToolbarHoverCmd[ToolbarID]);
		//break;

		case WM_MOUSELEAVE:
		//		DrawBtnFocusRect(hWndDlg);
			HaveTrackMouseEvent[ToolbarID] = FALSE;
			RemoveToolbarPointer(ToolbarID);
		break;
		
		case WM_LBUTTONDOWN:
			if (!HaveTrackMouseEvent[ToolbarID])
			{
				TRACKMOUSEEVENT EventTrack;

				EventTrack.dwFlags = TME_LEAVE | TME_HOVER;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				EventTrack.hwndTrack = hwnd;
				EventTrack.dwHoverTime = 500;
				TrackMouseEvent(&EventTrack);
				HaveTrackMouseEvent[ToolbarID] = TRUE;
				DrawToolbarPointer(ToolbarID);
				//			DrawBtnFocusRect(hWndDlg);
			}
			break;

	}
	return CallWindowProc(g_OrigTabProc, hwnd, message, wParam, lParam);
}


LRESULT CALLBACK WndProcTempImage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hTempWindata;
	LPTEMPWINDDATA pTempWindata;

	switch (message)
	{
	case WM_CREATE:
	{
	  return 0;
	}
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC		hDC;
		memset(&ps, 0x00, sizeof(PAINTSTRUCT));
		hDC = BeginPaint(hWnd, &ps);
		hTempWindata = (HANDLE)GetWindowLong(hWnd, GWL_USERDATA);
		pTempWindata = GlobalLock(hTempWindata);
		DisplayBMInRect32(hDC, pTempWindata->hDib32, ps.rcPaint, TRUE);
		GlobalUnlock(hTempWindata);
		EndPaint(hWnd, &ps);
	}
		break;

	case WM_MOUSEMOVE:
		hTempWindata = (HANDLE)GetWindowLong(hWnd, GWL_USERDATA);
		if (hTempWindata)
		{
			pTempWindata = GlobalLock(hTempWindata);
			if (!pTempWindata->haveTracking)
			{
				TRACKMOUSEEVENT EventTrack;

				pTempWindata->haveTracking = TRUE;
				EventTrack.dwFlags = TME_LEAVE;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				EventTrack.hwndTrack = hWnd;
				EventTrack.dwHoverTime = 0;
				TrackMouseEvent(&EventTrack);
			}
			GlobalUnlock(hTempWindata);
		}
		break;

		case WM_LBUTTONUP:
		case WM_MOUSELEAVE:
			DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		hTempWindata = (HANDLE)GetWindowLong(hWnd, GWL_USERDATA);
		pTempWindata = GlobalLock(hTempWindata);
		GMDestroyDIB32(pTempWindata->hDib32);
		GSSiGlobUlFree(&hTempWindata);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


ATOM TempImageRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	static ATOM c;
	static BOOL first = TRUE;

	if (!first)
		return c;
	first = FALSE;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcTempImage;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "TempImageWindow";
	wcex.hIconSm = 0;

	c = RegisterClassEx(&wcex);
	return c;
}

HWND InitTempImageInstance(HWND hWndPar, LPSTR imageFile,int centerOpt)
{
	HWND hWnd;
	RECT	rect;
	LPRECT pRect = &rect;
	HDIB32 hDib32 = BMPHandleFromEXT(imageFile);
	BITMAPINFOHEADER DibInfo;
	int	width, height;
	HANDLE hTempWindata;
	LPTEMPWINDDATA pTempWindata;
	POINT centerPoint = { 0 }, rectMidpt;
	RECT	wrect;
	int		xmove, ymove;

	if (!hDib32)
		return NULL;
	GetDIBDimensionsFromHandle(hDib32, &height, &width);
	TempImageRegisterClass(hInst);

	rect.left = rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	GetWindowRect(hWndPar, &wrect);
	switch (centerOpt)
	{
	case 0://on cursor
		GetCursorPos(&centerPoint);
		break;
	case 1://in parent window
		centerPoint = RectMid(&wrect);
		break;
	}
	rectMidpt = RectMid(&rect);
	xmove = centerPoint.x - rectMidpt.x;
	ymove = centerPoint.y - rectMidpt.y;

	rect.left += xmove;
	rect.right += xmove;
	rect.top += ymove;
	rect.bottom += ymove;
	xmove = ymove = 0;
	if (rect.top < wrect.top)
		ymove = wrect.top - rect.top;
	else if (rect.bottom > wrect.bottom)
		ymove = wrect.bottom - rect.bottom;
	if (rect.left < wrect.left)
		xmove = wrect.left - rect.left;
	else if (rect.right > wrect.right)
		xmove = wrect.right - rect.right;
	rect.left += xmove;
	rect.right += xmove;
	rect.top += ymove;
	rect.bottom += ymove;
	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
						  "TempImageWindow", "External DB",
						  WS_VISIBLE | WS_POPUP,
						  pRect->left, pRect->top, RECTWIDTH(pRect), RECTHEIGHT(pRect), 
						  hWndPar, NULL, hInst, hDib32);

	if (!hWnd)
	{
		return FALSE;
	}
	hTempWindata = GSSiGlobAlloc(1782,GHND, sizeof(TEMPWINDDATA));
	pTempWindata = GlobalLock(hTempWindata);
	pTempWindata->hDib32 = hDib32;
	GlobalUnlock(hTempWindata);
	SetWindowLong(hWnd, GWL_USERDATA, (LONG)hTempWindata);
	InvalidateRect(hWnd, 0,TRUE);

	//MoveWindow (hWnd,0,0,500,500,TRUE);
	return hWnd;
}

BOOL GetToolbarBounds(HWND hWnd,LPMNMXCORD pBounds)
{
	MNMXCORD bounds;
	int i;
	BOOL rtn = FALSE;
	RECT rect;
	int	dst;
	DPOINT p;

	GetClientRect(hWnd, &rect);
	dst = max(RECTWIDTH(&rect), RECTHEIGHT(&rect));
	DBoundsInit(&bounds);
	for (i = 0; i < nToolbars; i++)
	{
		if (ToolbarDPoint[i].x)
		{
			AddDPointToMinMax(&ToolbarDPoint[i],&bounds);
			p = ToolbarDPoint[i];
			p.x += dst * ToolbarReZoomScale[i];
			AddDPointToMinMax(&ToolbarDPoint[i], &bounds);
			p = ToolbarDPoint[i];
			p.x -= dst * ToolbarReZoomScale[i];
			AddDPointToMinMax(&ToolbarDPoint[i], &bounds);
			p = ToolbarDPoint[i];
			p.y += dst * ToolbarReZoomScale[i];
			AddDPointToMinMax(&ToolbarDPoint[i], &bounds);
			p = ToolbarDPoint[i];
			p.y -= dst * ToolbarReZoomScale[i];
			AddDPointToMinMax(&ToolbarDPoint[i], &bounds);
			rtn = TRUE;
		}
	}

	*pBounds = bounds;
	return rtn;
}

int RedisplayButtonToCMDMenu(HWND hWndDlg, LPSTR BMPath, LPSTR ButtonText, int iButton)
{
	LPTOOBAR_CONTROL_INFO	pTBInfo;
	int	i, ToolbarID;
	HBITMAP	hBM, hOldBM;

	ToolbarID = GetToolbarIDFromWnd(hWndDlg);
	if (ToolbarID < 0)
		return FALSE;
	hBM = GetToolBitmap(BMPath);
	pTBInfo = GlobalLock(ToolbarHandle[ToolbarID]);
	pTBInfo += iButton;
	hOldBM = (HBITMAP)SendDlgItemMessage(hWndDlg, pTBInfo->CntlID, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBM);
	pTBInfo->hBM = hBM;
	GlobalUnlock(ToolbarHandle[ToolbarID]);

	GSSiDeleteObject(&hOldBM);

	return 0;
}

