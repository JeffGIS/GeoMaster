#include "graphint.h" 

#include "gmextern.h"

#include "cddemo.h"                   

enum FREE_IMAGE_FORMAT {
	FIF_UNKNOWN = -1,
	FIF_BMP		= 0,
	FIF_ICO		= 1,
	FIF_JPEG	= 2,
	FIF_JNG		= 3,
	FIF_KOALA	= 4,
	FIF_LBM		= 5,
	FIF_IFF = FIF_LBM,
	FIF_MNG		= 6,
	FIF_PBM		= 7,
	FIF_PBMRAW	= 8,
	FIF_PCD		= 9,
	FIF_PCX		= 10,
	FIF_PGM		= 11,
	FIF_PGMRAW	= 12,
	FIF_PNG		= 13,
	FIF_PPM		= 14,
	FIF_PPMRAW	= 15,
	FIF_RAS		= 16,
	FIF_TARGA	= 17,
	FIF_TIFF	= 18,
	FIF_WBMP	= 19,
	FIF_PSD		= 20,
	FIF_CUT		= 21,
	FIF_XBM		= 22,
	FIF_XPM		= 23,
	FIF_DDS		= 24
};

static	RECT	PrinterRect={0,0,0,0};
static	char DocName[128], LeafName[34];   
static  HANDLE	hSaveView[MAX_VIEWPORTS],SaveMaskArea[MAX_VIEWPORTS];  
static	BOOL	SaveDisableZoomMacro[MAX_VIEWPORTS];
static  BOOL		SaveHaveBounds[MAX_VIEWPORTS];
static  MNMXCORD	SaveWBounds[MAX_VIEWPORTS]; 
static  UINT		SaveMaskNumPoints[MAX_VIEWPORTS];
static  UINT		SaveMaskNumParts[MAX_VIEWPORTS];
static	char		Prefix[10], UDI[66];   
static	char		str[1030], ReportName[144]; 
static	BOOL	UseSecondStatus=FALSE;   
static	char	VirtualPrinterPathname[144]="";
static	double	PhysicalPrinterWidth, PhysicalPrinterHeight;
static	HANDLE	hPDChunkLocal=0;
static	BOOL	HaveSavedVP=FALSE;	
static	char	VirtFmt[8]=".tif", VirtFmt2[8]="*.tif";    
static	long	StatusMacro=0;   
static	double	VirtualPrinterResolution;    
static	BOOL	SaveDH;
static short	VPRes[3]={200,300,600}; 

extern BOOL PrintWasCanceled;
extern char	CurVirtPrinter[128];
extern char	CustomHeight[16],CustomWidth[16];  
extern char	VirtPrinterImageFile[256];

static	short	CurrentStatusWindowVP;
static	FARPROC lpfnAbortProc=0, lpfnPrintDlgProc=0, lpfnProcessStatusDlgProc;

BOOL CheckPrintAbort (HDC hPr);

BOOL VirtualPlotToImage (LPSTR ImageFile,LPSTR VirtualPlotIndex,DWORD Flag)
{   
	MNMXCORD	IndexBounds;
	HANDLE	handle;
	HDIB32	hDIB,hDIBTile;
    LPFILEINDEX lpIndex;  
    LPFILELISTENTRY lpEntry; 
    DWORD	Width,Height;
    BOOL	rtn=FALSE;
	LPVIEWPORT	SaveVP = CurView;   
	char	TileName[256],VPDir[256],Mess[256];     
	LPSTR	lpBS;   
	long	TotNum=0, CurLoc=0;
	
	if (!(handle = OpenMapIndex (VirtualPlotIndex,&IndexBounds)))
		goto Exit;
	lpIndex = (LPFILEINDEX)GlobalLock (handle);  
NextFile:  
        GetNextIndexEntry(lpIndex); 
        TotNum++;
        if (lpIndex->FileInIndex >=(long)lpIndex->NumFiles)
        {   
            if (!(lpIndex=GetNextIndexHeader(&handle,FALSE)))
                goto EndFile;
        }
       	goto NextFile;  
EndFile:   
    CloseMapIndex (VirtualPlotIndex,handle,FALSE,TRUE); 
	handle = OpenMapIndex (VirtualPlotIndex,&IndexBounds);
	lpIndex = (LPFILEINDEX)GlobalLock (handle);  
	Width = IndexBounds.xmx;
	Height = IndexBounds.ymx;
	WaitCursor (1);
	hDIB = AllocateDIB (Width,Height,24); 
	WaitCursor (-1);
	if (!hDIB)     
	{   
		GlobalUnlock (handle);
    	CloseMapIndex (VirtualPlotIndex,handle,FALSE,TRUE); 
		goto Exit;
	}
	_fstrcpy (VPDir,VirtualPlotIndex);
	lpBS = _fstrrchr (VPDir,'\\');
	*lpBS = 0;  
	sprintf (Mess,"Creating %s",ImageFile);   
	CreateStatusWindow (hWndMain,1,Mess);
Next:
        GetNextIndexEntry(lpIndex); 
        sprintf (TileName,"%s%s",VPDir,lpIndex->CurrentEntry->Name);
        hDIBTile = BMPHandleFromEXT (TileName);
        PasteDIB (hDIB,hDIBTile,(DWORD)lpIndex->CurrentEntry->Bounds.xmn,Height - (DWORD)lpIndex->CurrentEntry->Bounds.ymx,256);   
        GMDestroyDIB32 (hDIBTile);
        sprintf (Mess,"Adding tile %ld",++CurLoc);
		StatusWindowUpdate (NULL,Mess, TotNum, CurLoc);
        if (lpIndex->FileInIndex >=(long)lpIndex->NumFiles)
        {   
            if (!(lpIndex=GetNextIndexHeader(&handle,FALSE)))
                goto End;
        }
        if (ContinueProcessing) 
        	goto Next;            
        else
        	GlobalUnlock (handle);

End:        
    CloseMapIndex (VirtualPlotIndex,handle,FALSE,TRUE); 
    if (ContinueProcessing)
    {
		WaitCursor (1);
		StatusWindowUpdate (NULL,"Writing file to disk", TotNum, CurLoc);
		rtn = BMPToEXT32 (hDIB,ImageFile,Flag);
		WaitCursor (-1);
	}
	ContinueProcessing = TRUE;    
	DestroyStatusWindow (0);
	GMDestroyDIB32 (hDIB);
Exit:
	CurView = SaveVP;	
    return rtn;
} 

BOOL CreateVirtualPlotIndex (LPSTR VirtPlotDir,LPSTR ImageFile)
{ 
	char	Name[144], TempName[144]; 
	HFILE	Fid,Fid2;  
	long	TotFiles=0;
	BOOL	rtn=TRUE;   
    
    sprintf (Name,"%s\\index",VirtPlotDir);
	GSSiGetTempFileName(0,"gm1",0,TempName);
	Fid =	GSSiOpenFile (TempName,NULL,OF_CREATE);
	SearchFilesInDir (VirtPlotDir,VirtFmt , Fid,&TotFiles,VirtFmt2,1);     
	GSSiClose (Fid);
	CreateMapIndex (VirtPlotDir,TempName,4,FALSE,"","",FALSE,1,NULL,NULL,NULL,FALSE);
	GSSiRemove (TempName); 
	if (*ImageFile)
	{
		rtn = VirtualPlotToImage (ImageFile,Name,0); 
		DeleteDirAndContents (VirtPlotDir);
	}
	return rtn;
}
 
void SetVirtPrinterBackground (HDC hDC,COLORREF WindowColor)
{
	UINT	x,y; 
	RECT	Rect;
	
	Rect.left = Rect.top = 0;
	Rect.right = VirtualPageWidth-1;
	Rect.bottom = VirtualPageHeight-1;
	SaveDC (hDC);  
  	SelectClipRgn (hDC,NULL);
		FillRectPoly (hDC,&Rect,WindowColor); 
	RestoreDC (hDC,-1);  
	return;
}

void SaveViewports (void)
{  
	short	iview;
	LPVIEWPORT	lpSaveView; 

	HaveSavedVP = TRUE;
	for (iview=0;iview<*pNumViewports;iview++)
	{
		SetCurView (pViewports[iview]);
        GSSiDeleteObject(&CurView->hRgn);
	    CloseTRANS2 (&CurView->hTranWinToBase);
	    CloseTRANS2 (&CurView->hTranBaseToWin); 
	    CloseTRANS2 (&CurView->hFileTransIn);
		GSSiGlobFree (&CurView->ToolbarHandle);
	    GSSiGlobFree (&CurView->hTAGList);  
	    GSSiGlobFree (&CurView->hProfileElev);
	    GSSiGlobFree (&CurView->hProfileRoute);
		if (CurView->Type == 7)
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
		if (!CurView->WindowIsZoomed)
			CurView->HaveBounds = FALSE;
		SaveHaveBounds[iview]=CurView->HaveBounds;
		SaveWBounds[iview]=CurView->WBounds;
		SaveMaskNumPoints[iview] = CurView->NumMaskPoints; 
		SaveMaskNumParts[iview] = CurView->NumMaskAreaParts;  
		SaveDisableZoomMacro[iview] = CurView->DisableZoomMacro;
		CurView->DisableZoomMacro = TRUE;
		GSSiGlobFree (&CurView->hMaskAccelerator[0]);
		GSSiGlobFree (&CurView->hMaskAccelerator[1]);
		GSSiGlobFree (&CurView->hMaskAccelerator[2]);
		if (CurView->hMaskArea)
		{
			HPSTR	pMask=GlobalLock (CurView->hMaskArea), pNewMask;
			long	size = sizeof(MNMXCORD) + (long)CurView->NumMaskPoints * (long)sizeof(DPOINT) + CurView->NumMaskAreaParts * 2;
			   			
			SaveMaskArea[iview] = GSSiGlobAlloc ( 490,GMEM_MOVEABLE,size);
			pNewMask = GlobalLock (SaveMaskArea[iview]);
			hmemmove (pNewMask,pMask,size);
			GlobalUnlock (SaveMaskArea[iview]);
			GlobalUnlock (CurView->hMaskArea);
		}
		else
			SaveMaskArea[iview]=0;
	    hSaveView[iview] = GSSiGlobAlloc ( 491,GHND,sizeof(VIEWPORT));
	    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[iview]);
		*lpSaveView = *CurView;
		GlobalUnlock (hSaveView[iview]);
	} 
	return;
}

void RestoreViewports (void)
{   
	short	iview;
	LPVIEWPORT	lpSaveView; 
	
	if (!HaveSavedVP)
		return;
	HaveSavedVP = FALSE;
	for (iview=0;iview<*pNumViewports;iview++)
	{
		SetCurView (pViewports[iview]);
        GSSiDeleteObject(&CurView->hRgn);
	    CloseTRANS2 (&CurView->hTranWinToBase);
	    CloseTRANS2 (&CurView->hTranBaseToWin); 
	    CloseTRANS2 (&CurView->hFileTransIn);
		GSSiGlobFree (&CurView->ToolbarHandle);
	    GSSiGlobFree (&CurView->hTAGList);
	    GSSiGlobFree (&CurView->hProfileElev);
	    GSSiGlobFree (&CurView->hProfileRoute);
		if (CurView->Type == 7)
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
	    lpSaveView = (LPVIEWPORT)GlobalLock (hSaveView[iview]);  
		GSSiGlobFree (&CurView->hMaskAccelerator[0]);
		GSSiGlobFree (&CurView->hMaskAccelerator[1]);
		GSSiGlobFree (&CurView->hMaskAccelerator[2]);
	    if (lpSaveView)
	    {  
			ClearMaskArea ();
			lpSaveView->pVisList1 = CurView->pVisList1;
			lpSaveView->pVisListManual = CurView->pVisListManual;
	   		*CurView = *lpSaveView;
			CurView->NumMaskPoints = SaveMaskNumPoints[iview];
			CurView->NumMaskAreaParts = SaveMaskNumParts[iview];
			CurView->hMaskArea = SaveMaskArea[iview];    
			CurView->DisableZoomMacro = SaveDisableZoomMacro[iview];
			GSSiGlobUlFree (&hSaveView[iview]); 
			if (CurView->CurrentFunction == GF_TOOLBAR)
				LoadVPToolBar (); 
		}
					
		SetBounds(CurView->hWnd,NULL); /*Null prevents redisplay of view bounds*/
	} 
	return;
}
	
BOOL PrintReport2 (HDC hPr,LPSTR ReportName,long Refno,LPSTR Prefix,LPSTR UDI)
{    
	char	txt[64];
	LPVIEWPORT	SaveView;        
	HANDLE	hView;   
	BOOL	SaveUMRP = UseMultReportPages;
	
	UseMultReportPages=TRUE;
	StartPage (hPr); 
	DisplayCycle++;
	SaveView = CurView;
	hView = GSSiGlobAlloc ( 492,GHND,sizeof(VIEWPORT));
	SetCurView ( (LPVIEWPORT)GlobalLock (hView));
	PrintReportID=0;
	PrintReportPage=1;
	CurView->ReportRefno = Refno;
	SetIntRefno (Refno); 
	if (Prefix)
	{
		_fstrcpy (CurView->Prefix,Prefix);
		_fstrcpy (CurView->UDI,UDI); 
		SetUDIValue (Prefix,UDI);  
	}
	CurView->Active = TRUE;
	CurView->ShrinkToFit=FALSE; 
	CurView->hReport = LoadReport (ReportName);
    if (CurView->hReport) 
	{   
		double Factor, Pixelsperinch; 
		char	cID[64]="", cPage[32]="";
		
		if (UDI && *UDI)
			_fstrcpy (cID,UDI);
		else
			ltoa (Refno,cID,10);			
		PrintMessage2 ("Print Report",ReportName,cID);
		Pixelsperinch = GetDeviceCaps(hPr, LOGPIXELSY);  
		Factor = Pixelsperinch/72;
		//Factor = 1;
		DisplayReport2 (hPr, CurView->hReport,MainRect,Factor,FALSE,FALSE);
	}
	UnloadReport (&CurView->hReport); 
	GlobalUnlock (hView);
	GlobalFree (hView);
	SetCurView ( SaveView);
	UseMultReportPages=SaveUMRP;
	return TRUE;
		   
}

BOOL CreateStatusWindow (HWND hWnd,short nStatusBarsIN,LPSTR Title)
{   
	short	nStatusBars = abs (nStatusBarsIN);
	BOOL	AllowBlock=TRUE;
	
	if (nStatusBarsIN < 0)
		AllowBlock = FALSE;     
	StatusMacro = CurrentMacro;
    if (!hSaveStatWindowBM && AllowBlock)  
    {
		ClearFullWindowBitmap ();
    	hSaveStatWindowBM = EnterBlockingWindow (hWnd); 
    	SaveStatWindowBMWindow = hWnd;
    }
    if (!PrintMsgWnd)
    {
	    gbUserAbort = FALSE;
	    lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
	    ghPrintingDlg = CreateDialog(ghInst, "PRINTING", hWnd,
	                                     lpfnPrintDlgProc);
	//    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
		if (Title)
			SetWindowText (PrintMsgWnd,Title); 
		UseSecondStatus = FALSE;
	}
	else 
	{
		UseSecondStatus = TRUE;
		nStatusBars = 2;
	}
    if (nStatusBars > 1 && PrintMsgWnd)
    	ShowWindow (GetDlgItem(PrintMsgWnd,IDC_STATUS),SW_SHOW);  
    else
    	ShowWindow (GetDlgItem(PrintMsgWnd,IDC_STATUS),SW_HIDE); 
    SaveDH = DisableHalt; 
    DisableHalt = AllowBlock;  
	CheckForContinue(TRUE);
	return TRUE;
} 

BOOL StatusWindowUpdate (LPSTR Title, LPSTR Mess, DWORD Tot, DWORD Done)
{   
	char	MessText[256];
	
	if (!PrintMsgWnd)
		return TRUE;  
	if (UseSecondStatus)
	{ 
		StatusWindowUpdate2 (Mess,Tot,Done);
	}
	else
	{
		if (Title)
			SetWindowText (PrintMsgWnd,Title);
		if (Mess)
		{
			_fstrcpy (MessText,Mess);
			ExpandText (MessText);
			SetDlgItemText (PrintMsgWnd,IDC_PRINT_RECORD,MessText);
		}
	    PctBox (GetDlgItem(PrintMsgWnd,PRINT_VIEW), Tot, Done,0); 
	    if (gbUserAbort)
			return FALSE;
    }
	return TRUE;
}

void StatusWindowUpdate2 (LPSTR Mess, DWORD Tot, DWORD Done)
{ 
	char	MessText[256];
	if (!PrintMsgWnd)
		return;
	if (Mess)
	{
		_fstrcpy (MessText,Mess);
		ExpandText (MessText);
		SetDlgItemText (PrintMsgWnd,PRINT_FILE,MessText);
	}
    PctBox (GetDlgItem(PrintMsgWnd,IDC_STATUS), Tot, Done,0);

	return;
}

BOOL DestroyStatusWindow (long Macro)
{   
	if (Macro && Macro != StatusMacro)
		return FALSE;
	if (UseSecondStatus)
	{
		UseSecondStatus = FALSE;
		return TRUE;
	}
    DisableHalt = SaveDH;  
    if (!gbUserAbort)
    {  
       if (ghPrintingDlg)
       		DestroyWindow(ghPrintingDlg);
       ghPrintingDlg = NULL;
    } 
    else
       MessageBox(GetFocus(), "Operation cancelled", " ", MB_OK);
//    FreeProcInstance(lpfnAbortProc);
    if (lpfnPrintDlgProc)
    	FreeProcInstance(lpfnPrintDlgProc);
    lpfnPrintDlgProc = 0;
	LeaveBlockingWindow(hSaveStatWindowBM);
	hSaveStatWindowBM = 0;
	return TRUE;
} 

BOOL CreateProcessStatusWindow (HWND hWnd,LPSTR Title)
{   
    if (ProcessStatusWnd)
    	return FALSE;  
    StatusMacro = CurrentMacro;
	lpfnProcessStatusDlgProc = MakeProcInstance(ProcessStatusDlgProc, ghInst);
	CreateDialog(ghInst, "PROCESSSTATUS", hWnd,lpfnProcessStatusDlgProc);
	if (Title)
		SetDlgItemText (ProcessStatusWnd,IDC_STATUSTITLE,Title);
	CurrentStatusWindowVP = CurView->ID;
	CheckForContinue(TRUE);
	return TRUE;
} 

BOOL ProcessStatusWindowUpdate (LPSTR Line1, LPSTR Line2)
{   
	
	if (!ProcessStatusWnd)
		return ContinueProcessing;  
	if (Line1)
		SetDlgItemText (ProcessStatusWnd,IDC_STATUS1,Line1);
	if (Line2)
		SetDlgItemText (ProcessStatusWnd,IDC_STATUS2,Line2);
	return ContinueProcessing;
}

BOOL DestroyProcessStatusWindow (void)
{   
	if (!ProcessStatusWnd || CurView->ID != CurrentStatusWindowVP)
		return ContinueProcessing;  
	DestroyWindow (ProcessStatusWnd);
    if (lpfnPrintDlgProc)
    	FreeProcInstance(lpfnPrintDlgProc);
    lpfnPrintDlgProc = 0;
    SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC,GF_TEXTMODE);
    SelectClipRgn (CurView->hDC,0);
	FillVPBackground ();
    RestoreDC (CurView->hDC,-1);
	return TRUE;
} 

BOOL FAR PASCAL ProcessStatusDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hDlg,message, wParam, lParam)))
 	return (BRtn);
   switch (message)
      {
   case WM_INITDIALOG: 
   		ProcessStatusWnd = hDlg;
   		ContinueProcessing=TRUE; 
		cwCenter(hDlg, 0);
   		break;
   case WM_DESTROY:
		LeaveBlockingWindow(hSaveStatWindowBM);
		hSaveStatWindowBM = 0;
        ProcessStatusWnd = 0;  
        break;
   case WM_COMMAND:
      switch (wParam)
         {
      case IDCANCEL:
         ContinueProcessing=FALSE;
         DestroyWindow(ProcessStatusWnd); 
         HaltMapDisplay (FALSE);
         return TRUE;

      default:
         return FALSE;
         }
      break;

   default:
      return FALSE;
      }
   return TRUE;
}

void AdjustMainRectVirtualPrinter (LPRECT Rect,short VirtualPage,LPSHORT pIrow,LPSHORT pIcol)
{   
	
	*pIrow = VirtualPage / VirtualPagesPerRow;
	*pIcol = VirtualPage % VirtualPagesPerRow;

	Rect->top -= (*pIrow * VirtualPageHeight);
	Rect->bottom = Rect->top + VirtualPlotHeight;
	Rect->left -= (*pIcol * VirtualPageWidth);
	Rect->right = Rect->left + VirtualPlotWidth; 
	*pIrow = NumVirtualRows - *pIrow - 1;
	return;
}
 
void GSSiDeleteDC(HDC hDC,BOOL IsVirtPrinter)
{
	if (IsVirtPrinter && hbmpVirtPrinterOld)
	{
		HBITMAP hBitMap = SelectObject (hDC,hbmpVirtPrinterOld);
		
		DeleteObject (hBitMap);
		hbmpVirtPrinterOld = 0;
	}
	DeleteDC (hDC);
	return;
}

BOOL PrintMap (HWND hWnd, long Page, long TotPage)
	{
	HDC hPr;
	RECT	Rect;
	/*******************************************************************
	*                                                                  *
	*                             PRINTDLG VARIABLES                   *
	*                                                                  *
	*******************************************************************/
	HDC	hDC;
	long xPage, yPage;
	WORD wSize;
	BOOL bError;
	HBRUSH	BkBrush;
	BOOL		rtn=TRUE;
	LPVIEWPORT	lpSaveView, LastVP; 
	int		iview;  
	MSG		msg; 
	HWND		DTW;
	int		SaveShadow = ShadowInc;
	RECT		BandRect,LastBandRect, SaveRect; 
	int		Band, bRc,ii; 
	LPSTR	lpStr=str;
	BOOL		PrintPrompt=TRUE, DoPrint, LastPage=FALSE, First;
	short	ForceOrient=DMORIENT_PORTRAIT;  
	long		iseg=0;
	LPDEVMODE pDevMode;  
	double	SaveSFLFF=SmallFontLargeFontFactor,SaveWidthFactor;
	HANDLE	hSaveBM; 
	long		Refno;
	COLORREF	SaveColor=WindowColor;  
	BOOL		UseBands = GetGlobalBVal2 ("[%USEBANDS]",TRUE); 
	BOOL		DoRedisplay = FALSE;
	char	Warning[128]="";  
	short	ivp; 
	BOOL IsVirtPrinter=FALSE;
	HANDLE hVirtPrinter=0;
	BOOL SaveIPV = InPlotView; 
	short	VirtualPageRow,VirtualPageCol;  
	char	VirtPlotDir[256],VirtPlotName[128];    
	DWORD	SaveFlags;   
	DWORD	Err;    
	HBITMAP	hOverViewBitmap=0; 
	long	OverViewWidth, OverViewHeight;
	char	OutFile[128]; 
	HWND	hWnd2, ghWnd; 
    
    NumVirtualPages = 1;
	GSSiTrace ("Begin Printmap",0);    
	
	if (!hWnd)
		PrintPrompt = FALSE; 
	else
		ghWnd = hWnd;
	hWnd = NULL;
	HaveReports = FALSE; 
	Printing = TRUE;
	DisplayTAGs2 (0,4,0); //scans for reports
	Printing = FALSE;
	AddReportToPrintList (NULL,NULL,NULL,NULL);
	DeleteAllVPRegions ();
	if (Page >= TotPage)
		LastPage = TRUE;
 	SaveWidthFactor = WidthFactor;
	if (!OpenConfig(NULL,NULL)) return(FALSE);

     wSize = sizeof(PRINTDLG);
     if (!hPDChunk)
     {
     	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
        	return(MemError());
	    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
	    PrintPrompt = TRUE;  
     }
     else
     	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     
/*	 {
		RECT	MRect;
		HDC		hDC;
	
		GetClientRect(hWndMain, &MRect); 
		hDC = GetDC (hWndMain);   
		hSaveBM = SaveScreen2 (hDC,MRect,NULL,0);	
		ReleaseDC (hWndMain,hDC);
	 } */   
	 WindowColor = RGB(255,255,255);   
     DoPaint = FALSE;   
     HaltPaint = TRUE;  
     hSaveBM = EnterBlockingWindow (hWnd);
	 EnableWindow (hWndMain,FALSE);
	 lpPDChunk->hwndOwner = ghWnd;	
	 lpPDChunk->hInstance = ghInst;
	 if (GetGlobalLVal2("[%PRINTDIALOGOPT]",0))
		lpPDChunk->Flags = lpPDChunk->Flags|PD_PRINTSETUP|PD_RETURNDC;
	 else if (!PrintPrompt && Page == 1)
	 {
		PrintPrompt = TRUE;   
		DoPrint = TRUE;
		goto S10;
	 }
	 else if (Page > 1)
	 {
	 	PrintPrompt = FALSE;   
	 	DoPrint = TRUE;
	 	goto S10;
	 }
	 lpPDChunk->hDC=0;  
	 lpPDChunk->lpfnSetupHook = (FARHOOK)PrintSetupHook;   
	 lpPDChunk->lpSetupTemplateName = "PRNSETUPDLGGM";
	 SetCurView (pViewportsD[0]); 
	 if (CurView->WidthType == 2 || CurView->DesiredWidth > CurView->DesiredHeight)
	 	ForceOrient = DMORIENT_LANDSCAPE;
	 if (ForceOrient && !ShowVirtualPrintAreas)
	 { 
	 	SaveFlags = lpPDChunk->Flags;
	    
	    lpPDChunk->Flags = PD_RETURNDEFAULT;
	    GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter);
	    lpPDChunk->Flags = SaveFlags; 
	    if (lpPDChunk->hDevMode)
	    { 	
	    	IgnoreLock = TRUE;   
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			pDevMode->dmOrientation = ForceOrient; 
			pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
		GSSiGlobFree (&hVirtPrinter);
	 }
	 if (!(DoPrint = GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter)))
	 {
	    if (!PrintWasCanceled)
	    {
		    IgnoreLock = TRUE;
		    GSSiGlobFree(&lpPDChunk->hDevMode);
		    GSSiGlobFree(&lpPDChunk->hDevNames);
		    IgnoreLock = FALSE;
	        GSSiGlobUlFree (&hPDChunk);
			lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize);
		    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
		    lpPDChunk->Flags = SaveFlags; 
		    if (lpPDChunk->hDevMode)
		    { 	
		    	IgnoreLock = TRUE;
				pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
				pDevMode->dmOrientation = ForceOrient; 
				pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
				GlobalUnlock (lpPDChunk->hDevMode); 
				IgnoreLock = FALSE;
			}
			GSSiGlobFree (&hVirtPrinter); 
	    	DoPrint = GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter);
	    }
	 }
	 if (lpPDChunk->hDevMode)
	 { 	
    	IgnoreLock = TRUE;
		pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
		GlobalUnlock (lpPDChunk->hDevMode); 
    	IgnoreLock = FALSE;
	 }
     hPr = lpPDChunk->hDC;
     if (TotPage < 0)
     {  
     	if (hPr && TotPage == -2) 
     	{
	       int dpi = GSSiGetDeviceCaps(hPr, LOGPIXELSX);
	       
		       xPage = (USHORT)GetDeviceCaps(hPr, HORZSIZE);
		       yPage = (USHORT)GetDeviceCaps(hPr, VERTSIZE);
		       xPage = (USHORT)GetDeviceCaps(hPr, HORZRES);
	       xPage = GSSiGetDeviceCaps(hPr, HORZRES);
	       yPage = GSSiGetDeviceCaps(hPr, VERTRES);  
	       if (*CustomWidth)
	       {
	       		xPage = atof (CustomWidth) * dpi;
	       		yPage = atof (CustomHeight) * dpi;
	       } 
	       if (IsVirtPrinter)
	       {
	       		dpi = VirtualPrintDPI;
	       		xPage = VirtualPlotWidth;
	       		yPage = VirtualPlotHeight; 
	       }   
	       PrinterResolution=dpi;
	       xPage--;
	       yPage--;
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage;
	       Rect.right = xPage;
	       PrinterRect = Rect;
	       Printing = TRUE;
	       SetMainRect (0,hPr,&Rect); 
	       yPage = Rect.bottom;
	       IgnoreLock = TRUE;
	       {    
	       		double fwidth =(double)((long)(100*(double)xPage/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)yPage/(double)dpi))/100;     
	       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
	       		
	       		if (IsVirtPrinter)
	       			_fstrcpy (PrinterName,CurVirtPrinter);
	       		if (xPage < 0 || xPage > SHRT_MAX || yPage < 0 || yPage > SHRT_MAX)
	       			_fstrcpy (Warning,"\r\nWARNING: This printer configuration exceeds the maximum image size and will not print");
		        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f%s",PrinterName,dpi,fwidth,fheight,Warning);  
		        if (*Warning)
		        	MessageBox (GetFocus(),str,"Printer Parameters",MB_ICONEXCLAMATION); 
		        PrinterWidth = fwidth;
		        PrinterHeight = fheight;
			    PrinterFormatPixelWidth = Rect.right - Rect.left;
		        GlobalUnlock (lpPDChunk->hDevNames);
		   }
	       IgnoreLock = FALSE;
		} 
		Printing = FALSE;
        GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  
        lpPDChunk->hDC = 0;
        GlobalUnlock(hPDChunk);
		HaltPaint = FALSE;
    	DoPaint = TRUE;   
	 	EnableWindow (hWndMain,TRUE);
		LeaveBlockingWindow(hSaveBM); 
		WindowColor = SaveColor;
		InVirtualPrint = FALSE;
    	return DoPrint;
     } 
S10: 
     InPrintProcess= TRUE;
	 SmallFontLargeFontFactor=1;
     if (DoPrint)
     
     {	DOCINFO	DI;
     
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        if (PrintPrompt)
        {
	        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
	        hWnd2 = ghWnd;
	        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", hWnd2,
	                                         lpfnPrintDlgProc);
	        GetGlobalCVal ("[%PRINTNAME]",LeafName,NULL);
	        if (!*LeafName)
				GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);
		    DI.cbSize = sizeof(DOCINFO);
		    DI.lpszDocName= LeafName;
		    DI.lpszOutput = NULL;
	        ShowWindow (GetDlgItem(ghPrintingDlg,IDC_STATUS),FALSE);
		    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
	        SetAbortProc(hPr,lpfnAbortProc);
		    if (IsVirtPrinter)
		    {   
		    	LPSTR pDot;
		    	short	n=1,st=1;
		    	
		    	DoPrint = TRUE; 
	   			//GSSiGetTempFileName (0,"gmp",0,VirtPlotDir); 
	   			//if ((pDot = _fstrrchr (VirtPlotDir,'.')))
	   			//	*pDot = 0;
	   			GetTempDir (VirtPlotDir);
	   			_fstrcat (VirtPlotDir,"\\GeoMaster Virtual Plots");
	   			GSSiMakeDir (VirtPlotDir,&Err); 
	   			GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);  
	   			while (st)
	   			{
		   			GetTempDir (VirtPlotDir);
		   			sprintf (VirtPlotName,"%s_%4.4i",LeafName,n++);
	   				sprintf (_fstrchr (VirtPlotDir,0),"\\GeoMaster Virtual Plots\\%s",VirtPlotName);   
	   				if (n > 1000)
	   				{
	   					MessageBox (0,VirtPlotDir,"Unable to create virtural plot directory",MB_ICONEXCLAMATION);
	   					break;
	   				}
	   				st = FileType (VirtPlotDir);
	   				if (!st)
	   					GSSiMakeDir (VirtPlotDir,&Err); 
	   			} 
		    }
		    else if (StartDoc(hPr,&DI) > 0)
	    	{   
	       		short ii=Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
	    		DoPrint = TRUE;
	    	}
	    	else
	    		DoPrint = FALSE;
		}
		else
			DoPrint = TRUE;
		if (DoPrint)
	    {  
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);
	       int planes = GetDeviceCaps(hPr, PLANES);
	       int nc = GetDeviceCaps(hPr, NUMCOLORS);
	       
	       if (IsVirtPrinter)
	       {    
	       		HBITMAP	hOldBM;
	       		
	       		dpi = VirtualPrintDPI;
	       		xPage = VirtualPlotWidth;
	       		yPage = VirtualPlotHeight;
				VirtualPrinterFactor=2;  
				UseBands = FALSE;
				InVirtualPrint = TRUE;
	       		if (VirtualPlotWidth > VirtualPlotHeight)
	       			OverViewWidth = 1000;
	       		else
	       			OverViewWidth = (1000 * VirtualPlotWidth) / VirtualPlotHeight;
	       		OverViewHeight = (OverViewWidth * VirtualPlotHeight)/VirtualPlotWidth; 
	       		if (!*VirtPrinterImageFile)      
	       			hOverViewBitmap = CreateCompatibleBitmap(hPr,(int)OverViewWidth,(int)OverViewHeight); 
	       } 
	       else
	       {
		       xPage = (USHORT)GetDeviceCaps(hPr, LOGPIXELSY);
		       xPage = (USHORT)GetDeviceCaps(hPr, HORZSIZE);
		       xPage = (USHORT)GetDeviceCaps(hPr, VERTSIZE);
		       xPage = (USHORT)GetDeviceCaps(hPr, HORZRES);
		       yPage = (USHORT)GetDeviceCaps(hPr, VERTRES); 
		       if (*CustomWidth)
		       {
		       		xPage = atof (CustomWidth) * dpi;
		       		yPage = atof (CustomHeight) * dpi;
		       } 
		       NumVirtualPages=1; 
		       VirtualPrinterFactor=2;
		   } 
		   PrinterResolution = dpi;
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1; 
	       PrinterRect = Rect;
	       IgnoreLock = TRUE;  
           Printing = TRUE;
		   if (xPage < 0 || xPage > SHRT_MAX || yPage < 0 || yPage > SHRT_MAX)
		   {
	       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset; 
	       		double fwidth =(double)((long)(100*(double)(xPage)/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)(yPage)/(double)dpi))/100;     
	       		 
				_fstrcpy (Warning,"\r\nWARNING: This printer configuration exceeds the maximum image size and will not print");
			    sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f%s",PrinterName,dpi,fwidth,fheight,Warning);  
				MessageBox (GetFocus(),str,"Printer Parameters",MB_ICONEXCLAMATION);   
				GlobalUnlock (lpPDChunk->hDevNames);
		        IgnoreLock = FALSE;
	            GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  
	            lpPDChunk->hDC = 0;
				DestroyWindow(ghPrintingDlg);
				ghPrintingDlg = NULL; 
	            rtn = FALSE;
				goto Exit2;
		   }
	       IgnoreLock = FALSE;
	       SaveRect = Rect;  
	       {
		       SetMainRect (0,hPr,&Rect);
		       yPage = Rect.bottom;
		       IgnoreLock = TRUE;
//		       if (!ivp)
		       {
		       		double fwidth =(double)((long)(100*(double)(xPage)/(double)dpi))/100;
		       		double fheight=(double)((long)(100*(double)(yPage)/(double)dpi))/100;     
		       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
		       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
	//	       		char	Tech[128];
		       		
	//	       		if (!Escape(hPr, GETTECHNOLOGY, NULL, NULL, Tech))
	//	       			*Tech = 0; 
					DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
					if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
						DoShrinkOrtho = FALSE;
		       		if (IsVirtPrinter)
		       		{
			        	sprintf (str,"Virtual Printer:%s\r\nOutput to:%s",CurVirtPrinter,VirtPrinterImageFile); 
			        }
		       		else
			        	sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
			        GlobalUnlock (lpPDChunk->hDevNames); 
			        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
			        if (!*VirtualPrinterPathname)
			        {
				        PrinterWidth = fwidth;
				        PrinterHeight = fheight;
				    }
		       }
		       IgnoreLock = FALSE;  
	           
	           SaveViewports ();
		       if (!IsVirtPrinter)
			   		StartPage (hPr); 
			   DisplayCycle++; 
			   if (UseBands)
			   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect);
			   else
	           		bRc = 1;
	           Band=0;
	       	   ivp=0;
	           while (!gbUserAbort && ivp < NumVirtualPages && bRc > 0 && (!UseBands || !IsRectEmpty(&BandRect)))
	           {   
			       Rect = SaveRect; 
			       if (IsVirtPrinter)
			       {
			       		AdjustMainRectVirtualPrinter (&Rect,ivp,&VirtualPageRow,&VirtualPageCol);
			       		BandRect.left = BandRect.top = 0;
			       		BandRect.bottom = VirtualPageHeight; 
			       		BandRect.right = VirtualPageWidth; 
			       }
			       SetMainRect (0,hPr,&Rect);
			       if (Printing && GetGlobalBVal2 ("[%SHOWPRINTABLEAREA]",FALSE))
			       	FillRect (hPr,&Rect,GetStockObject (LTGRAY_BRUSH));
				   if (!SetupViewports (NULL,hPr,0,Rect,Band))
				   {
				   		gbUserAbort=TRUE;
						DestroyWindow(ghPrintingDlg);
						ghPrintingDlg = NULL; 
						goto Exit;
				   }
	               Band++;
	               ivp++; 
	               LastBandRect = BandRect;
		   	       SetDisplayMode (hPr, GF_TEXTMODE);
				   if (IsVirtPrinter)
				   		SetVirtPrinterBackground (hPr,WindowColor);
				   NumViewportsToDisplay = *pNumViewports;
				   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
				   {
						SetCurView (pViewports[DisplayViewID]);
						if (CurViewActive ())
							CurView->Display = TRUE;
						ResetViewport (TRUE,TRUE);
						if (SaveHaveBounds[DisplayViewID])
						{
							CurView->HaveBounds = TRUE;
							CurView->WBounds = SaveWBounds[DisplayViewID];
							CurView->NewBounds = CurView->WBounds;
						}
				   }
				   DisplayViewID = 0; 
				   LastVP = 0;
				   while (!gbUserAbort && DisplayViewID<NumViewportsToDisplay)
				   {
				   		SetCurView (pViewportsD[DisplayViewID]);
				   		if (CurView != LastVP && CurView->NumFiles) 
				   		{
				   			if (CurView->ID == *pCommandViewport &&
						     	CurView->OrthoRes && CurView->WindowZoomedToOrtho)
						    {
						        CurView->WBounds = CurView->NewBounds;
						        SetNewBoundsToOrtho();
						    }
					   			
				   			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);  
	//						SetBounds2 (CurView->hWnd,CurView->hDC);
				   		}
				   		if (CurView != LastVP)
				   			ii=1;  
				   		LastVP = CurView;
				   		if (CurView->Type == 7 || (CurView->pTheme && CurView->pTheme->ID == GF_BOUNDS_DISPLAY_THEME))
				   			DisplayViewID++;
				   		else 
						{  
						   char	Line1[128], Line3[128], str[128]; 
						   
						   DisplayViewport (hWnd,hPr,FALSE);	   
						   sprintf (Line1,"Page %ld of %ld", Page,TotPage);
						   sprintf (Line3,"Band %i of %i - Viewport %i",Band,NumVirtualPages,CurView->ID); 
						   _fstrcpy (str,PltName);
						   ExpandText (str);
						   PrintMessage2 (Line1,str,Line3);
					       if (OpenMap (hWnd, hPr))
					       {   
		   	       	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
				
						       do
						       {
						   			sprintf (Line3,"Band %i of %i - Viewport %i - Segment %ld",Band,NumVirtualPages,CurView->ID,iseg++);
									PrintMessage2 (NULL,NULL,Line3);
						       }
						       while (DisplaySeg (hPr,FALSE) && CheckPrintAbort (hPr) && CurView); 
						       if (gbUserAbort)
						       		HaltMapDisplay (FALSE);
						   }
					     }
					     
				   }
		 NextBand: 
		       	   EndDisplayProcessing (TRUE);
			   	   ApplyVPShadows ();	
			   	   if (GetGlobalCVal ("[%PRINTPROMPT]",str,NULL))
				   		DisplayPromptText (hPr,str);
	   			   DisplayCycle++; 
		 		   if (UseBands)  
		 		   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect); 
		 		   else if (!IsVirtPrinter)
				   		bRc = 0;
				   else
				   {    
				   		short	ii=1;
				   		
				   		sprintf (OutFile,"%s\\%3.3i%3.3i%s",VirtPlotDir,VirtualPageRow,VirtualPageCol,VirtFmt);
				   		if (ii)SaveDCBitMap (hPr,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE,WindowColor, 
				   						  hOverViewBitmap,
						   			      NumVirtualRows-VirtualPageRow-1,VirtualPageCol,
						   			      OverViewWidth/VirtualPagesPerRow,OverViewHeight/NumVirtualRows,OverViewHeight);

				   		//SaveDCBitMap (hPr,OutFile,FIF_BMP,0);
				   }
			   }
		       if (!IsVirtPrinter)
			   		EndPage (hPr);
			   if (!gbUserAbort && GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
			   {    
			   		_fstrcpy (ReportName,"[%DL]macros\\summary.txt");
			   		PrintReport2 (hPr,ReportName,NULL,NULL,0);
	           }
	           First = TRUE;
			   while (!gbUserAbort && GetNextPrintReport (First,ReportName,&Refno,Prefix,UDI))
			   {    
			   		LPSHORT	ID;
			   		int		i, NumIDs;
			   		LPLONG	CNum; 
			   		char	txt[64];
					LPVIEWPORT	SaveView;        
					HANDLE	hView;  
			   		
			   		First = FALSE;  
				    if (GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 1)
			   		{
				   		PrintReport2 (hPr,ReportName,Refno,Prefix,UDI);      
					    EndPage (hPr);
				   	}
			   }
			   InVirtualPrint = FALSE;
			   RestoreViewports ();
		   }   
Exit:	   VirtualPrinterFactor=1;		   
		   if (gbUserAbort) 
		   {
		   		if (!IsVirtPrinter)
		   			AbortDoc (hPr);
	            GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  
	            lpPDChunk->hDC = 0;
	            rtn = FALSE;
	       } 
		   else
		   {
	           if (LastPage)
	           {
		           if (!IsVirtPrinter)
		           		EndDoc (hPr);
		           else
		           {    
		           		char	PPCmd[256]; 
		           		HFILE	FidPS;    
		           		double	ScaleX,ScaleY;
		           		DPOINT	BitmapPoint, WorldPoint;  
		           		HDIB32	hDib;
		           		
		           		BitmapPoint.x = 0;
		           		BitmapPoint.y = 0;
		           		WorldPoint.x = 0;
		           		WorldPoint.y = VirtualPlotHeight-1;
		           		ScaleX = ScaleY = VirtualPlotWidth/(double)OverViewWidth;
		           		CreateVirtualPlotIndex (VirtPlotDir,VirtPrinterImageFile); 
		           		if (!*VirtPrinterImageFile)
		           		{
			           		sprintf (PPCmd,"%s\\printsetup.bin",VirtPlotDir); 
			           		FidPS = GSSiOpenFile (PPCmd,NULL,OF_CREATE);
			           		SavePrintSetupData (FidPS,PrinterResolution);                  
			           		GSSiClose (FidPS);
					   		sprintf (OutFile,"%s\\Overview%s",VirtPlotDir,VirtFmt);   
	//				   		SaveBitmap (hOverViewBitmap,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE); 
					   		sprintf (OutFile,"%s\\Overview%s",VirtPlotDir,".tif");  
					   		hDib = BitmapToDIB32 (hOverViewBitmap);
					   		SetGeoTiffData (hDib,&ScaleX,&ScaleY,&BitmapPoint,&WorldPoint); 
					   		SaveDIB32 (hDib,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE); 
					   		GMDestroyDIB32 (hDib);
					   		GSSiDeleteObject (&hOverViewBitmap); 
			           		sprintf (PPCmd,"$EXECUTE([%DL]gmloader.exe printpreview.gmc(@[PLOTPREVIEWNAME]=%s))",VirtPlotName);
			           		ProcessText (PPCmd);
			           	}
		           }
		           GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  
		           lpPDChunk->hDC = 0;
		       }
			   
	       }
		   
			RestoreViewports ();
	    }
	    else
	       bError = TRUE;
	    if (!gbUserAbort && LastPage)
	    {
	       DestroyWindow(ghPrintingDlg);
	       ghPrintingDlg = NULL;
	    }
	    if (bError)
	       MessageBox(ghWnd, "Error while printing", szAppName, MB_OK);
	    else
	    {
	       if (gbUserAbort) 
	       {
	            MessageBox(ghWnd, "Printing Aborted", "", MB_OK);
		        if (PrintMsgWnd)
		        {
			         DestroyWindow(PrintMsgWnd); 
			         ghPrintingDlg = NULL; 
			         DoRedisplay=TRUE;
			    }
			}
	    }
	    if (LastPage)
	    {
		    if (lpfnAbortProc)
		    	FreeProcInstance(lpfnAbortProc);
		    if (lpfnPrintDlgProc)
		    	FreeProcInstance(lpfnPrintDlgProc);
		    lpfnAbortProc = 0;     
		    lpfnPrintDlgProc = 0;
	    }
	}
    else
    {
        ProcessCDError(CommDlgExtendedError());  
        rtn = FALSE;
    }  
Exit2:
    GlobalUnlock(hPDChunk);
    Printing = FALSE;  
    if (LastPage)
    {
		HaltPaint = FALSE;
    	DoPaint = TRUE;   
    }
    ShadowInc = SaveShadow;
 	WidthFactor = SaveWidthFactor;
	EnableWindow (hWndMain,TRUE);
	SetFocus (hWndMain);
	GSSiTrace ("End Printmap",0);    
	hDC = GetDC (hWndMain);
    SetMainRect (hWndMain,hDC,NULL);
	LeaveBlockingWindow(hSaveBM);
    ReleaseDC (hWndMain,hDC);  
    SmallFontLargeFontFactor = SaveSFLFF;    
    WindowColor = SaveColor;
    if (hWndAbortWaitMessage)
    	DestroyWindow (hWndAbortWaitMessage); 
    if (DoRedisplay)
		RedisplayWindow (); 
	InVirtualPrint = FALSE;  
	InPrintProcess = FALSE;
    return (rtn);

}   

BOOL CheckPrintAbort (HDC hPr)
{   
	MSG	msg;
	
	if (gbUserAbort || !QueryAbort(hPr,0))
		return FALSE;
    while (PeekMessage(&msg, NULL, NULL, NULL, TRUE)) 
    {
        if (!IsDialogMessage(ghPrintingDlg, &msg))
        {
#if ENABLETRACE
SetLastMessage(-1*(long)msg.message);
#endif
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		if (!QueryAbort(hPr,0))
			return FALSE; 
	}
	return TRUE;
}

BOOL PrintScrollReport (HWND hWnd)
{	HDC hPr;
    RECT	Rect;
   short xPage, yPage;
   WORD wSize;
   BOOL bError;
//   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn;
   LPVIEWPORT	lpSaveView; 
   int		iview, i, n, nrow, ncol, irow, icol, height, width, margin=4;  
   LPSTR	lpchr;
   MSG		msg; 
   HWND		DTW;
   RECT		BandRect, SubRect; 
   HDIB		hDIB; 
   char		drive[6], dir[128], leaf[16], ext[6], File[128], SavePrintName[34]; 
   char		ReportName[128]; 
   double	SaveDTSF = DeviceToScreenFactor;
   RECT		SaveMainRect = MainRect, SaveRect;
   short	SaveShadow = ShadowInc;    
   short 	ForceOrient = DMORIENT_LANDSCAPE;
   LPDEVMODE pDevMode;  
   HWND		ghWnd;


   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,NULL); 
    
   ghWnd = hWnd;
   hWnd = NULL;
   
     wSize = sizeof(PRINTDLG);
     if (!hPDChunk)
     {
     	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
        	return(MemError());
	    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
     }
     else
     	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     lpPDChunk->hwndOwner = ghWnd;
     	
//     DoPaint = FALSE; 
//	 EnableWindow (hWndMain,FALSE);
	 if (ForceOrient && !ShowVirtualPrintAreas)
	 { 
	 	DWORD	SaveFlags = lpPDChunk->Flags;
	    
	    lpPDChunk->Flags = PD_RETURNDEFAULT;
	    GSSiPrintDlg(lpPDChunk,NULL,NULL);
	    lpPDChunk->Flags = SaveFlags; 
	    if (lpPDChunk->hDevMode)
	    { 	
	    	IgnoreLock = TRUE;
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			pDevMode->dmOrientation = ForceOrient; 
			pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
	 }

     if (GSSiPrintDlg(lpPDChunk,NULL,NULL) != 0)
     
     {	DOCINFO	DI;
     
     	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    GetGlobalCVal ("[%PRINTNAME]",LeafName,NULL); 
	    if (!*LeafName)
			GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    if (StartDoc(hPr,&DI) > 0)
	    {   
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);  
	       
	       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
	       xPage = GetDeviceCaps(hPr, HORZRES);
	       yPage = GetDeviceCaps(hPr, VERTRES);
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1;
	       SaveRect = Rect;
	       SetMainRect (0,hPr,&Rect);
	       yPage = Rect.bottom;
	       IgnoreLock = TRUE;
	       {
	       		double fwidth =(double)((long)(100*(double)Rect.right/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)Rect.bottom/(double)dpi))/100;     
	       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
				DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
				if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
					DoShrinkOrtho = FALSE;
		        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
		        GlobalUnlock (lpPDChunk->hDevNames); 
		        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
		        PrinterWidth = fwidth;
		        PrinterHeight = fheight;
	       }
	       IgnoreLock = FALSE;  
	       Rect = SaveRect;
	       SetMainRect (0,hPr,&Rect);
		   		
		   GetCurVal (ReportName,sizeof(ReportName),IDS_FILERPT);
	       PrintReport2 (hPr,ReportName,CurView->ReportRefno,CurView->Prefix,CurView->UDI);
           EndDoc (hPr);
           DeleteDC(lpPDChunk->hDC);
	    }
	    else
	       bError = TRUE;
//	    EnableWindow (hWndMain,TRUE);
	    if (!gbUserAbort)
	    {
	       DestroyWindow(ghPrintingDlg);
	       ghPrintingDlg = NULL;
	    }
	    if (bError)
	       MessageBox(ghWnd, "Error while printing", szAppName, MB_OK);
	    else
	    {
	       if (gbUserAbort) 
	       {
	            MessageBox(ghWnd, "Printing Aborted", "", MB_OK);
		        if (PrintMsgWnd)
		        {
			         DestroyWindow(PrintMsgWnd); 
			         ghPrintingDlg = NULL; 
			    }
			}
	    }
	    FreeProcInstance(lpfnAbortProc);
	    FreeProcInstance(lpfnPrintDlgProc);
	    lpfnAbortProc = 0;  
	    lpfnPrintDlgProc = 0;
	}
    else
     {
        ProcessCDError(CommDlgExtendedError());
     }
    GlobalUnlock(hPDChunk);
    Printing = FALSE; 
    {
		HaltPaint = FALSE;
    	DoPaint = TRUE;   
    }
//	EnableWindow (hWndMain,TRUE);  
    DeviceToScreenFactor = SaveDTSF;
   	MainRect = SaveMainRect;
	ShadowInc = SaveShadow;
   	SetGlobalValue ("%PRINTNAME",SavePrintName); 

    return (rtn);

}     

BOOL PrintTextFile (HWND hWnd,LPSTR File,int nTabs,LPSHORT TabsIn)
{	HDC hPr;
    RECT	Rect;
   short xPage, yPage,x=10, y=10;
   WORD wSize;
   BOOL bError;
//   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn;
   LPVIEWPORT	lpSaveView; 
   int		iview, i, n, nrow, ncol, irow, icol, height, width, margin=4, page=1;  
   LPSTR	lpchr;
   MSG		msg; 
   HWND		DTW;
   RECT		BandRect, SubRect; 
   HDIB		hDIB; 
   char		drive[6], dir[128], leaf[16], ext[6], SavePrintName[34],str[514]; 
   char		ReportName[128]; 
   double	SaveDTSF = DeviceToScreenFactor;
   RECT		SaveMainRect = MainRect, SaveRect;
   short	SaveShadow = ShadowInc;    
   short 	ForceOrient = DMORIENT_PORTRAIT;
   LPDEVMODE pDevMode;   
   HFILE	Fid; 
   DWORD	TextExt;  
   short	Tabs[32], Margin=120;   
   double	Factor; 
   HFONT	hFont, hFontBold, OldFont;
   char		Header[514]; 
   short	FontSize = 8, LineInc, BottomOfPage; 
   HWND		ghWnd;

   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,NULL); 
    
   ghWnd = hWnd;
   hWnd = NULL;
   
     wSize = sizeof(PRINTDLG);
     if (!hPDChunk)
     {
     	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
        	return(MemError());
	    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
     }
     else
     	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     lpPDChunk->hwndOwner = ghWnd;
     	
//     DoPaint = FALSE; 
//	 EnableWindow (hWndMain,FALSE);
	 if (ForceOrient && !ShowVirtualPrintAreas)
	 { 
	 	DWORD	SaveFlags = lpPDChunk->Flags;
	    
	    lpPDChunk->Flags = PD_RETURNDEFAULT;
	    GSSiPrintDlg(lpPDChunk,NULL,NULL);
	    lpPDChunk->Flags = SaveFlags; 
	    if (lpPDChunk->hDevMode)
	    { 	
	    	IgnoreLock = TRUE;
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			pDevMode->dmOrientation = ForceOrient; 
			pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
	 }

     if (GSSiPrintDlg(lpPDChunk,NULL,NULL) != 0)
     
     {	DOCINFO	DI;
     
     	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    GetGlobalCVal ("[%PRINTNAME]",LeafName,NULL); 
	    if (!*LeafName)
			GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    if (StartDoc(hPr,&DI) > 0)
	    {   
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);  
	       
	       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
	       xPage = GetDeviceCaps(hPr, HORZRES);
	       yPage = GetDeviceCaps(hPr, VERTRES);
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1;
	       SaveRect = Rect;
	       SetMainRect (0,hPr,&Rect);
	       yPage = Rect.bottom;
	       IgnoreLock = TRUE;
	       {
	       		double fwidth =(double)((long)(100*(double)Rect.right/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)Rect.bottom/(double)dpi))/100;     
	       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
				DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
				if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
					DoShrinkOrtho = FALSE;
		        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
		        GlobalUnlock (lpPDChunk->hDevNames); 
		        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
		        PrinterWidth = fwidth;
		        PrinterHeight = fheight;
	       }
	       IgnoreLock = FALSE;  
	       Rect = SaveRect;
	       SetMainRect (0,hPr,&Rect); 
	       Factor = (double)(Rect.right - Rect.left-Margin)/(double)TabsIn[nTabs-1];   
	       BottomOfPage = Rect.bottom - Margin;
		   for (i=0;i<nTabs;i++)
		   		Tabs[i] = TabsIn[i] * Factor;
		   Tabs[nTabs-1] *= 2;
		   hFontBold = CreateFont((int)IDNINT(FontSize*Factor*1.4), 0, 0, 0, FW_BLACK,0, 0, 0, 0, 0, 0, 0, 0,"Arial Black");   
		   hFont = CreateFont((int)IDNINT(FontSize*Factor), 0, 0, 0, FW_THIN,0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
		   OldFont = SelectObject (hPr,hFontBold);
		   Fid = GSSiOpenFile (File,NULL,OF_READ);  
		   fgetstring (Header,512,Fid);
		   TextExt = GetTextExtent (hPr,Header,_fstrlen(Header)); 
		   LineInc = HIWORD(TextExt); 
NextPage:
		   SelectObject (hPr,hFontBold);
		   StartPage (hPr); 
		   sprintf (str,"Page %i",page++);
		   TextOut (hPr,0,Rect.bottom-LineInc,str,_fstrlen(str));
		   y = Margin*2;
		   TabbedTextOut (hPr,Margin,y,Header,_fstrlen(Header),nTabs,Tabs,Margin);	
		   y += LineInc*2;
		   SelectObject (hPr,hFont);
		   while (fgetstring (str,512,Fid))
		   {
				TabbedTextOut (hPr,Margin,y,str,_fstrlen(str),nTabs,Tabs,Margin);	
				TextExt = GetTextExtent (hPr,str,_fstrlen(str)); 
				y+=HIWORD(TextExt); 
				if (y > BottomOfPage) 
				{   
					EndPage (hPr);
					goto NextPage;
				}
		   }
		   GSSiClose (Fid);
		   SelectObject (hPr,OldFont);
		   DeleteObject (hFont);
		   DeleteObject (hFontBold);
		   		
           EndDoc (hPr);
           DeleteDC(lpPDChunk->hDC);
	    }
	    else
	       bError = TRUE;
//	    EnableWindow (hWndMain,TRUE);
	    if (!gbUserAbort)
	    {
	       DestroyWindow(ghPrintingDlg);
	       ghPrintingDlg = NULL;
	    }
	    if (bError)
	       MessageBox(ghWnd, "Error while printing", szAppName, MB_OK);
	    else
	    {
	       if (gbUserAbort) 
	       {
	            MessageBox(ghWnd, "Printing Aborted", "", MB_OK);
		        if (PrintMsgWnd)
		        {
			         DestroyWindow(PrintMsgWnd); 
			         ghPrintingDlg = NULL; 
			    }
			}
	    }
	    FreeProcInstance(lpfnAbortProc);
	    FreeProcInstance(lpfnPrintDlgProc);     
	    lpfnAbortProc = 0;
	    lpfnPrintDlgProc = 0;
	}
    else
     {
        ProcessCDError(CommDlgExtendedError());
     }
    GlobalUnlock(hPDChunk);
    Printing = FALSE; 
    {
		HaltPaint = FALSE;
    	DoPaint = TRUE;   
    }
//	EnableWindow (hWndMain,TRUE);  
    DeviceToScreenFactor = SaveDTSF;
   	MainRect = SaveMainRect;
	ShadowInc = SaveShadow;
   	SetGlobalValue ("%PRINTNAME",SavePrintName); 

    return (rtn);

}     

BOOL PrintImage (HWND hWnd, LPSTR Name,int option)
{	HDC hPr;
    RECT	Rect;
   LPPRINTDLG lpPDChunk;
   short xPage, yPage;
   WORD wSize;
   BOOL bError;
//   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn;
   LPVIEWPORT	lpSaveView; 
   int		iview, i, n, nrow, ncol, irow, icol, height, width, margin=4;  
   LPSTR	lpchr;
   MSG		msg; 
   HWND		DTW;
   int		SaveShadow;
   RECT		BandRect, SubRect; 
   HDIB		hDIB; 
   char		drive[6], dir[128], leaf[16], ext[6], File[128]; 
   HWND		ghWnd;
       
   ghWnd = hWnd;
   hWnd = NULL;
   
     _fstrupr (Name);
     wSize = sizeof(PRINTDLG);
     if (!hPDChunk)
     {
     	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
        	return(MemError());
	    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
     }
     else
     	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     lpPDChunk->hwndOwner = ghWnd;
     	
//     DoPaint = FALSE; 
	 EnableWindow (hWndMain,FALSE);
     if (PrintDlg(lpPDChunk) != 0)
     
     {	DOCINFO	DI;
     
     	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
		GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    if (StartDoc(hPr,&DI) > 0)
	    {   
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX); 
	       
	       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
	       xPage = GetDeviceCaps(hPr, HORZRES);
	       yPage = GetDeviceCaps(hPr, VERTRES);
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1;
	       SetMainRect (0,hPr,&Rect); 
	       {
	       		double fwidth=(double)xPage/(double)dpi, fheight=(double)yPage/(double)dpi;
	       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;
	       		char	str[256]; 
	       		
		        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
		        GlobalUnlock (lpPDChunk->hDevNames); 
		        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str);
	       }
           Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect);
           while (!IsRectEmpty(&BandRect))
           {    
           
	       		switch (option)
		        {
		       		case 1: 
		     PrintSingle:
		               if (_fstrstr(Name,".BMP"))
						   DisplayBMFileInRect (hPr,Name,Rect,TRUE);
					   else if (_fstrstr(Name,".PCX"))
						   DisplayPCXFileInRect (hPr,Name,Rect,TRUE);
		       		   goto EndPrint;
		       		case 2:
		       			nrow=ncol=2;
		       		break;
		        }  
		         GSSisplitpath (Name,drive,dir,leaf,ext);
		         i=_fstrlen(leaf);
		         lpchr = &leaf[i-1];
		         while (i && isdigit(*lpchr--))
		         	i--; 
		         n = atoi(&leaf[i]);
		         if (!n)
		         	goto PrintSingle;
		         leaf[i]=0; 
		         n=0;
		        height = (Rect.bottom-Rect.top)/nrow;
		        width  = (Rect.right-Rect.left)/ncol;
				for (irow=0;irow<nrow;irow++)
				{   
					for (icol=0;icol<ncol;icol++)
					{   
						n++; 
						SubRect.top = Rect.top+irow*height+margin;
						SubRect.bottom = Rect.top+(irow+1)*height-margin;
						SubRect.left = Rect.left+icol*width+margin;
						SubRect.right = Rect.left+(icol+1)*width-margin;
						
			         	sprintf (File,"%s%s%s%i%s",drive,dir,leaf,n,ext);
						if (ExistFile(File))
						{	
			               if (_fstrstr(Name,".BMP"))
							   DisplayBMFileInRect (hPr,File,SubRect,TRUE);
						   else if (_fstrstr(Name,".PCX"))
							   DisplayPCXFileInRect (hPr,File,SubRect,TRUE);
						}
					}
				}
		EndPrint:				   	
			    Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect); 
		   }
           EndDoc (hPr);
           DeleteDC(lpPDChunk->hDC);
	    }
	    else
	       bError = TRUE;
	    EnableWindow (hWndMain,TRUE);
	    if (!gbUserAbort)
	    {
	       DestroyWindow(ghPrintingDlg);
	       ghPrintingDlg = NULL;
	    }
	    if (bError)
	       MessageBox(ghWnd, "Error while printing", szAppName, MB_OK);
	    else
	    {
	       if (gbUserAbort) 
	       {
	            MessageBox(ghWnd, "Printing Aborted", "", MB_OK);
		        if (PrintMsgWnd)
		        {
			         DestroyWindow(PrintMsgWnd); 
			         ghPrintingDlg = NULL; 
			    }
			}
	    }
	    FreeProcInstance(lpfnAbortProc);
	    FreeProcInstance(lpfnPrintDlgProc); 
	    lpfnAbortProc = 0;   
	    lpfnPrintDlgProc = 0;
	}
    else
     {
        ProcessCDError(CommDlgExtendedError());
     }
    GlobalUnlock(hPDChunk);
    Printing = FALSE; 
    {
		HaltPaint = FALSE;
    	DoPaint = TRUE;   
    }
	EnableWindow (hWndMain,TRUE);
    return (rtn);

} 

BOOL GetPrintMergeRec (HFILE FidPM,LPSTR str)
{   
	BOOL	rtn=TRUE;  
	static	long	CurRec=0;
	 
	if (!str)
	{
		CurRec = 0;
		return TRUE;
	}
	if (hPMRecList) 
	{ 
		HPSTR pRec = GlobalLock (hPMRecList), bRec=pRec; 
		pRec += CurRec;  
		if (*pRec)
		{
			_fstrcpy (str,pRec);
			pRec = _fstrchr (pRec,0);
			pRec++;
			CurRec = pRec - bRec;
		}
		else
			rtn = FALSE;  
		GlobalUnlock (hPMRecList);
	}
	else
	{
NextRec:
		if (!fgetstring (str,1024,FidPM))
			return FALSE;
		CurRec++;
		if (CurRec < FirstMergeRecord)
			goto NextRec;
		if (CurRec > LastMergeRecord)
			return FALSE; 
	}			
	return rtn;
}
BOOL PrintMerge (HWND hWnd)
{	HDC hPr;
    RECT	Rect;
   /*******************************************************************
   *                                                                  *
   *                             PRINTDLG VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   short xPage, yPage;
   int		SaveShadow = ShadowInc;
   WORD wSize;
   BOOL bError;
//   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn=FALSE;
   LPVIEWPORT	lpSaveView, LastVP; 
   HANDLE	hDLT;
   RECT		BandRect, SaveRect;
   double	SaveWidthFactor; 
   long		Page=0, TotPage, iseg;
   int	SaveShadowInc;
   int		iview, record, bRc, Band, IsFirstRecord; 
   HFILE	FidPM; 
   BOOL		PrintPrompt=TRUE, DoPrint, First, FirstRecord=TRUE;
   short	ForceOrient=DMORIENT_PORTRAIT;  
   double	SaveSFLFF=SmallFontLargeFontFactor;
   LPDEVMODE pDevMode;
   DEVMODE	SaveDevMode;  
   OFSTRUCT	OFStruct;
   char		str2[32], SavePrintName[34], PrintDriver[64],PrintType[64],PrintPort[64];
   long		Refno;
   COLORREF	SaveColor = WindowColor;
   BOOL		UseBands = GetGlobalBVal2 ("[%USEBANDS]",TRUE);  
   short	ii;
   HWND		ghWnd;
       
   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,NULL); 
   ghWnd = hWnd;
	if (!OpenConfig(NULL,NULL)) return(FALSE);
    SaveShadowInc = ShadowInc;   
 	SaveWidthFactor = WidthFactor;

	if (!_fstricmp (PMDataFile,"Virtual Printer"))
	{
	 	LPSTR pFile=GlobalLock (hVirtualPrintFile);
	 	_fstrcpy (PMDataFile,pFile);
	 	GlobalUnlock (hVirtualPrintFile);
	}
    else if (!ExistFile (PMMacroFile))
    {
		MessageBox( GetFocus(), PMMacroFile,"Unable to open macro file", MB_OK);
     	return (FALSE);
    } 
	FidPM = GSSiOpenFile (PMDataFile,&OFStruct,OF_READ);
    if (FidPM == HFILE_ERROR)
    {
		MessageBox( GetFocus(), PMDataFile,"Unable to open data file", MB_OK);
     	return (FALSE);
    } 
	fgetstring (str,1024,FidPM);
	    		  
	while (str[0]=='#' || *LastChr (str) == ';') 
	{   
		ExpandText (str);             
	  	if (!fgetstring (str,1024,FidPM))
	  	{
	  		GSSiClose (FidPM);
	  		return FALSE;
	  	}
    }

	TotPage = 0;
	GetPrintMergeRec (0,NULL);
	while (GetPrintMergeRec (FidPM,str))
		TotPage++;
	GSSillseek (FidPM,0,0);
    wSize = sizeof(PRINTDLG);
    if (!hPDChunk)
    {
    	if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
       	return(MemError());
		InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
    }
    else
    	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
     	
    EnableWindow (hWndMain,FALSE);  
   	WindowColor = RGB(255,255,255);   
    DoPaint = FALSE;  
    HaltPaint = TRUE;
	lpPDChunk->hwndOwner = ghWnd;	
	lpPDChunk->hDC=0;
	if (GetGlobalLVal2("[%PRINTDIALOGOPT]",0))
		lpPDChunk->Flags = lpPDChunk->Flags|PD_PRINTSETUP|PD_RETURNDC;
	SetCurView (pViewportsD[0]); 
	if (CurView->WidthType == 2 || CurView->DesiredWidth > CurView->DesiredHeight)
		ForceOrient = DMORIENT_LANDSCAPE;
	 if (ForceOrient && !ShowVirtualPrintAreas)
	{ 
		DWORD	SaveFlags = lpPDChunk->Flags;
			    
		lpPDChunk->Flags = PD_RETURNDEFAULT;
		PrintDlg(lpPDChunk);
		lpPDChunk->Flags = SaveFlags; 
		if (lpPDChunk->hDevMode)
		{ 	
	    	IgnoreLock = TRUE;
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			pDevMode->dmOrientation = ForceOrient; 
			pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
	}
	DoPrint = PrintDlg(lpPDChunk); 
	if (lpPDChunk->hDevMode)
	{ 	
    	IgnoreLock = TRUE;
		pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
		SaveDevMode = *pDevMode; 
		GlobalUnlock (lpPDChunk->hDevMode); 
	}
    if (DoPrint)
    {	DOCINFO	DI;
     
    	hPr = lpPDChunk->hDC;  
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        PrintMerging = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    
        SaveViewports ();
		  
		fgetstring (str,1024,FidPM);
	    		  
		while (str[0]=='#' || *LastChr (str) == ';') 
		{                
		  	ExpandText (str);
		  	if (!fgetstring (str,1024,FidPM))
		  		goto Exit;
        }
        
        rtn = TRUE;
		GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName = LeafName;
	    DI.lpszOutput = NULL;  
	    
		ProcessDelimTextHeader(str,NULL,FidPM,&hDLT); 
		GetPrintMergeRec (0,NULL);
		record = 0; 
		{
			LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
       		LPSTR	lpPrintDriver=(LPSTR)pdn+pdn->wDriverOffset;  
       		LPSTR	lpPrintType=(LPSTR)pdn+pdn->wDeviceOffset;  
       		LPSTR	lpPrintPort=(LPSTR)pdn+pdn->wOutputOffset;
       		
       		_fstrcpy (PrintDriver,lpPrintDriver);
       		_fstrcpy (PrintType,lpPrintType);
       		_fstrcpy (PrintPort,lpPrintPort);  
	        GlobalUnlock (lpPDChunk->hDevNames); 
        }
		while (!gbUserAbort && GetPrintMergeRec (FidPM,str))
  	    {   
  	       if (FirstRecord)
  	       	FirstRecord = FALSE;
  	       else
  	       {  
        	DeleteDC(lpPDChunk->hDC);  
//	        	lpPDChunk->hDC = CreateDC(PrintDriver, PrintType, PrintPort, &SaveDevMode);
			AutoPrintOK = TRUE;
  	       	PrintDlg(lpPDChunk);   
	    	hPr = lpPDChunk->hDC;   
	       }
//		   SaveDC (hPr);
  	       IsFirstRecord = record;
           GetDelimTextData(str,hDLT);
           record++; 
		   AddReportToPrintList (NULL,NULL,NULL,NULL);
  	       GetGlobalCVal ("[%PRINTNAME]",DocName,NULL); 
  	       if (!*DocName)
		   {
		   		GSSisplitpath (CfgName,NULL,NULL,LeafName,NULL);
           		sprintf (DocName,"%s.%5.5i",LeafName,record); 
           }
		   StatusWindowUpdate2 (NULL, TotPage, Page++);
	       DI.lpszDocName = DocName;
	
		    if (StartDoc(hPr,&DI) > 0)
		    {   
		       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);
		       
		       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &lpPDChunk->nCopies, &lpPDChunk->nCopies);
		       xPage = GetDeviceCaps(hPr, HORZRES);
		       yPage = GetDeviceCaps(hPr, VERTRES);
		       Rect.left = 0;
		       Rect.top = 0;
		       Rect.bottom = yPage-1;
		       Rect.right = xPage-1;

		       SaveRect = Rect;
		       SetMainRect (0,hPr,&Rect);
		       yPage = Rect.bottom;
		       IgnoreLock = TRUE;
		       {
		       		double fwidth =(double)((long)(100*(double)Rect.right/(double)dpi))/100;
		       		double fheight=(double)((long)(100*(double)Rect.bottom/(double)dpi))/100;     
		       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
		       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
					DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
					if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
						DoShrinkOrtho = FALSE;
			        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
			        GlobalUnlock (lpPDChunk->hDevNames); 
			        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
			        PrinterWidth = fwidth;
			        PrinterHeight = fheight; 
			        PrinterFormatPixelWidth = Rect.right - Rect.left;
		       }
		       IgnoreLock = FALSE;  
		       Rect = SaveRect;
		       SetMainRect (0,hPr,&Rect);

//		   	   StartPage (hPr); 
		   	   DisplayCycle++;
			   if (UseBands)
			   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect);
			   else
	           		bRc = 1;
	           Band=0;
	           while (!gbUserAbort && bRc > 0 && (!UseBands || !IsRectEmpty(&BandRect)))
	           {
		
				   if (!SetupViewports (NULL,hPr,0,Rect,IsFirstRecord))
				   {
				   		gbUserAbort=TRUE;
						DestroyWindow(ghPrintingDlg);
						ghPrintingDlg = NULL; 
						goto Exit;
				   }
	               Band++;
		   	       SetDisplayMode (CurView->hDC, GF_TEXTMODE);
				   FillRectPoly (hPr,&Rect,WindowColor);
				   NumViewportsToDisplay = *pNumViewports;
				   
				   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
				   {
						SetCurView (pViewports[DisplayViewID]);
						if (CurViewActive ())
							CurView->Display = TRUE;
						ResetViewport (TRUE,TRUE);
						if (SaveHaveBounds[DisplayViewID])
						{
							CurView->HaveBounds = TRUE;
							CurView->WBounds = SaveWBounds[DisplayViewID];
							CurView->NewBounds = CurView->WBounds;
						}
        				SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
				   }
				   
				   Display = FALSE;
		           if (*PMMacroFile)
			     	  ProcessMacroFile (PMMacroFile,str2,0,0);  
			       else 
			       {
			          MNMXCORD	Bounds;
			         
					  SetViewport (*pCommandViewport);
			          Bounds = GetGlobalBoundsVal ("[PrintMergeBounds]",NULL);
					  ZoomToRect(Bounds,FALSE);  
                   }
				   Display = TRUE; 
				   SetupViewports (NULL,hPr,0,Rect,1);
				   for (DisplayViewID=0;DisplayViewID<NumViewportsToDisplay;DisplayViewID++)
				   {
						SetCurView (pViewports[DisplayViewID]);
						if (CurViewActive ())
							CurView->Display = TRUE;
						ResetViewport (TRUE,TRUE); 
				   } 

				   LastVP = 0;
				   DisplayViewID = 0;
			       while (DisplayViewID<NumViewportsToDisplay)
				   {
				   		SetCurView (pViewportsD[DisplayViewID]);
				   		if (CurView != LastVP && CurView->NumFiles) 
				   		{
				   			if (CurView->ID == *pCommandViewport &&
						     	CurView->OrthoRes && CurView->WindowZoomedToOrtho)
						    {
						        CurView->WBounds = CurView->NewBounds;
						        SetNewBoundsToOrtho();
						    }
					   			
				   			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
				   		}  
				   		LastVP = CurView;
				   		if (CurView->Type == 7 || (CurView->pTheme && CurView->pTheme->ID == GF_BOUNDS_DISPLAY_THEME))
				   			DisplayViewID++;
						else if (DisplayViewport (hWnd,hPr,FALSE)) 
						{  
						   char	Line1[128], Line3[128], str[128]; 
							   
						   sprintf (Line1,"Page %ld of %ld", Page,TotPage);
						   sprintf (Line3,"Band %i - Viewport %i",Band,CurView->ID); 
						   _fstrcpy (str,PltName);
						   ExpandText (str);      
						   iseg = 0;
						   PrintMessage2 (Line1,str,Line3);
					       if (OpenMap (hWnd, hPr))
					       {   
		   	       	   	       SetDisplayMode (CurView->hDC, GF_MAPMODE);
				
						       do
						       {
						   			sprintf (Line3,"Band %i - Viewport %i - Segment %ld",Band,CurView->ID,iseg++);
									PrintMessage2 (NULL,NULL,Line3);
						       }
						       while (DisplaySeg (hPr,FALSE) && CheckPrintAbort (hPr) && CurView); 
						       if (gbUserAbort)
						       		HaltMapDisplay (FALSE);
						   }
					     }
					     
				   }
				   if (!gbUserAbort)
				   {
						//ApplyVPBounds ();
		       			EndDisplayProcessing (TRUE);
				   		ApplyVPShadows ();	
				   }
				   else
				   		ii=1;
				   if (GetGlobalCVal ("[%PRINTPROMPT]",str,NULL))
					   DisplayPromptText (hPr,str);
		
		 NextBand: 
		 		   if (UseBands)  
		 		   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)NULL, &BandRect); 
		 		   else
				   		bRc = 0;
			   }
	   		   EndPage (hPr); 
//			   if (!gbUserAbort && GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
			   {    
			   		if (ExistFile ("[%DL]macros\\summary.txt"))
			   		{
				   		_fstrcpy (ReportName,"[%DL]macros\\summary.txt");  
				   		PrintReport2 (hPr,ReportName,NULL,NULL,0); 
				   	}
	           }
	           First = TRUE;
			   while (!gbUserAbort && GetNextPrintReport (First,ReportName,&Refno,Prefix,UDI))
			   {    
			   		LPSHORT	ID;
			   		int		i, NumIDs;
			   		LPLONG	CNum; 
			   		char	txt[64];
					LPVIEWPORT	SaveView;        
					HANDLE	hView;  
			   		
			   		First = FALSE;
				    if (GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 1)
				    {    
				   		EndPage (hPr);
				   		PrintReport2 (hPr,ReportName,Refno,Prefix,UDI); 
				   	}
			   }   

           	   EndDoc (hPr);
		    }
		    else 
		    {
		       bError = TRUE;		   
		       goto Exit;
		    }
	      NextRec:
	      	;
//			ResetDC (hPr,&SaveDevMode);
//	      	RestoreDC (hPr,-1);
		}
		if (!gbUserAbort && GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
		{    
			if (ExistFile ("[%DL]macros\\jobsum.txt"))
			{
		        _fstrcpy (DocName,"Summary");
			    if (StartDoc(hPr,&DI) > 0) 
			    {
					_fstrcpy (ReportName,"[%DL]macros\\jobsum.txt");  
			   		PrintReport2 (hPr,ReportName,NULL,NULL,0); 
	           	    EndDoc (hPr);
	           	}
		   	}
		}
Exit:
	   GSSiClose (FidPM);
	   GSSiGlobFree (&hDLT);
	   DeleteDC(lpPDChunk->hDC);
	   RestoreViewports ();
	   if (!gbUserAbort)
	   {
	      DestroyWindow(ghPrintingDlg);
	      ghPrintingDlg = NULL;
	   }
	   if (bError)
	      MessageBox(ghWnd, "Error while printing", szAppName, MB_OK);
	   else
	   {
	      if (gbUserAbort)
	      {
	            MessageBox(ghWnd, "Printing Aborted", "", MB_OK);
		        if (PrintMsgWnd)
		        {
			         DestroyWindow(PrintMsgWnd); 
			         ghPrintingDlg = NULL; 
			    }
			}
	   }
	   FreeProcInstance(lpfnAbortProc);
	   FreeProcInstance(lpfnPrintDlgProc);  
	   lpfnAbortProc = 0;
	   lpfnPrintDlgProc = 0;
	}
	else
	 {
	 //Process error.  Be sure to free any memory that may be associated with
	 //hDevMode or hDevNames.
	   /* if (lpPDChunk->hDevMode)
	       GlobalFree(lpPDChunk->hDevMode);
	    if (lpPDChunk->hDevNames)
	       GlobalFree(lpPDChunk->hDevNames);*/
	    ProcessCDError(CommDlgExtendedError());
	 }

    GlobalUnlock(hPDChunk);
    Printing = FALSE;  
    PrintMerging = FALSE;
	HaltPaint = FALSE;
   	DoPaint = TRUE;   
    ShadowInc = SaveShadow;
 	WidthFactor = SaveWidthFactor;
	EnableWindow (hWndMain,TRUE);
	SetFocus (hWndMain);
	GSSiTrace ("End PrintMerge",0);
	{
		HDC	hDC = GetDC (hWndMain);
	    SetMainRect (hWndMain,hDC,NULL);
	    ReleaseDC (hWndMain,hDC); 
	} 
    SmallFontLargeFontFactor = SaveSFLFF;
    WindowColor = SaveColor;
    GetGlobalCVal ("[%PRINTNAME]",SavePrintName,NULL); 
    AutoPrintOK = FALSE;
	return (rtn);

}

void SetBitmapFormatAndSize (HWND hWndDlg) 
{

	if (SendDlgItemMessage(hWndDlg,IDC_FORMATBMP,BM_GETCHECK,0,0))
		BitmapFormatOpt = 0; 
	if (SendDlgItemMessage(hWndDlg,IDC_FORMATTIF,BM_GETCHECK,0,0))
		BitmapFormatOpt = 1; 
	if (SendDlgItemMessage(hWndDlg,IDC_FORMATGEOTIF,BM_GETCHECK,0,0))
		BitmapFormatOpt = 2; 
	if (SendDlgItemMessage(hWndDlg,IDC_FORMATJPEG,BM_GETCHECK,0,0))
		BitmapFormatOpt = 3;   
	if (SendDlgItemMessage(hWndDlg,IDC_SIZESMALL,BM_GETCHECK,0,0))
		BitmapSizeOpt = 1; 
	if (SendDlgItemMessage(hWndDlg,IDC_SIZEMEDIUM,BM_GETCHECK,0,0))
		BitmapSizeOpt = 2; 
	if (SendDlgItemMessage(hWndDlg,IDC_SIZELARGE,BM_GETCHECK,0,0))
		BitmapSizeOpt = 3;  
	return;
} 

UINT GetImageFilterAndExtension (LPSTR Ext)
{    
	UINT	Filt;
	
	switch (BitmapFormatOpt)
	{
		case 0:
		_fstrcpy (Ext,".bmp");   
		Filt = IDS_FILTERBMP;
		break;
		
		case 1:
		case 2:
		Filt = IDS_FILTERTIF;
		_fstrcpy (Ext,".tif");
		break;
	
		case 3: 
		Filt = IDS_FILTERJPG;
		_fstrcpy (Ext,".jpg");
		break;
	}
	return Filt;
} 

  

BOOL FAR PASCAL EXPORTIMAGEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{   
	static	HANDLE	hSaveBM;
	char	str[256], Ext[16]; 

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
        
		hSaveBM = EnterBlockingWindow (hWndDlg); 
		GetCurVal (str,256,IDS_FILEEXPORTIMAGE);
		SetDlgItemText (hWndDlg,IDC_IMAGEFILE,str);  
		if (App == 1)
	        EnableWindow (GetDlgItem(hWndDlg,IDC_FORMATGEOTIF),FALSE);
		switch (BitmapFormatOpt)
		{
			case 0:
				SendDlgItemMessage (hWndDlg,IDC_FORMATBMP,BM_SETCHECK,TRUE,0L);
			break;
			case 1:
				SendDlgItemMessage (hWndDlg,IDC_FORMATTIF,BM_SETCHECK,TRUE,0L);
			break;
			case 2:  
				SendDlgItemMessage (hWndDlg,IDC_FORMATGEOTIF,BM_SETCHECK,TRUE,0L);
			break;
			case 3:
				SendDlgItemMessage (hWndDlg,IDC_FORMATJPEG,BM_SETCHECK,TRUE,0L);
			break;
		}		
		
		switch (BitmapSizeOpt)
		{
			case 1:
				SendDlgItemMessage (hWndDlg,IDC_SIZESMALL,BM_SETCHECK,TRUE,0L);
			break;
			case 2:  
				SendDlgItemMessage (hWndDlg,IDC_SIZEMEDIUM,BM_SETCHECK,TRUE,0L);
			break;
			case 3:
				SendDlgItemMessage (hWndDlg,IDC_SIZELARGE,BM_SETCHECK,TRUE,0L);
			break;
		}		
		
		 break;                              
    case WM_COMMAND:
         switch(wParam)
         {  
         	case IDC_FINDFILE:
         	{
         		UINT	Filt;
         		
				Filt = GetImageFilterAndExtension (Ext); 
         		if (GetSaveName2 (hWndDlg,str,Filt,Ext,IDS_FILEEXPORTIMAGE))
         			SetDlgItemText (hWndDlg,IDC_IMAGEFILE,str);
         	}
			break;
			
			case IDC_FORMATBMP:
			case IDC_FORMATTIF:
			case IDC_FORMATGEOTIF:
			case IDC_FORMATJPEG:
			case IDC_SIZESMALL:
			case IDC_SIZEMEDIUM:
			case IDC_SIZELARGE:
			
			SetBitmapFormatAndSize (hWndDlg);
			if (GetDlgItemText (hWndDlg,IDC_IMAGEFILE,str,256))
			{
				LPSTR pDot=_fstrrchr (str,'.');
				
				if (!pDot)
					pDot = _fstrchr (str,0);
				GetImageFilterAndExtension (Ext);
				_fstrcpy (pDot,Ext);
				SetDlgItemText (hWndDlg,IDC_IMAGEFILE,str);
			}
			break;

            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDOK: 
            {   
            	SetBitmapFormatAndSize (hWndDlg); 
                if (GetDlgItemText (hWndDlg,IDC_IMAGEFILE,str,256))
                {
					LPSTR pDot=_fstrrchr (str,'.');
					
					if (!pDot)
						pDot = _fstrchr (str,0);
					GetImageFilterAndExtension (Ext);
					_fstrcpy (pDot,Ext);
	                SetCurVal (str,IDS_FILEEXPORTIMAGE);
	                GSSiEndDialog(hWndDlg,TRUE,hSaveBM);  
	            }
            }
            break;    
            
         }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL ClipMap (HWND hWnd, LPSTR Name,short SizeOpt,short FormatOpt) 
{   
	HDIB	hDib;
	DWORD	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
    HDC SaveDC = CurView->hDC;  
    BOOL	SaveMemMap = MemMap; 
    HDC		hPr; 
    RECT	Rect;
	LPVIEWPORT	lpSaveView, LastVP; 
	short	startW,	endW = 4096*2;
	short	startH;
	double	WtoHFactor;  
	char	str[256];    
	RECT	ShapeRect; 
	long	rtn; 
	HCURSOR	hcurSave;  
	DWORD	hbmpOld; 
	char	MapName[256];
	HBITMAP	hTempBM;
	HDIB32	hDib32;
	DPOINT BitmapPoint[2], WorldPoint[2];
	double	ScaleX, ScaleY; 
	BOOL	SaveBS = BufferedScreen;
    
   	BufferedScreen = FALSE; 
	ScreenBufferDC (1,0); 
    if (SizeOpt == 1)
    	endW = 1024; 
    if (SizeOpt == 2)
    	endW = 2048; 
    
    if (!GetFormatRect (&ShapeRect))
    {
    	if (IsRectEmpty (&ConfigDisplayRect))
    		GetClientRect (hWnd,&ShapeRect);
    	else
    		ShapeRect = ConfigDisplayRect;
    } 
    WtoHFactor =(double)(ShapeRect.bottom - ShapeRect.top) / (ShapeRect.right - ShapeRect.left);
    OldCursor = GSSiSetCursor (LoadCursor (NULL,IDC_WAIT)); 
	startW = min (endW,pow (128000000 / (WtoHFactor * 3),0.5)-1); 
ReTry:
	MemMapWidth = startW;  
	MemMapHeight = MemMapWidth * WtoHFactor;
	hdcMemMap = LargeMemDC (CurView->hDC,&MemMapWidth,&MemMapHeight,&hbmpOld);    
/*		goto temp;
	hdcMemMap = CreateCompatibleDC(CurView->hDC); 
	hMemBitmap = 0;
	do
	{   
//		sprintf (str,"%i",MemMapWidth);
//		SetWindowText (hWnd,str);
		GSSiDeleteObject (&hMemBitmap);
		MemMapWidth++;
		MemMapHeight = MemMapWidth * WtoHFactor;
		hMemBitmap = CreateCompatibleBitmap (CurView->hDC,MemMapWidth,MemMapHeight);
	}
	while (hMemBitmap && MemMapWidth < endW + 1 && ((double)MemMapWidth * (double)MemMapWidth * WtoHFactor * 3) < 16000000);
	
	MemMapWidth--;    
	MemMapHeight = MemMapWidth * WtoHFactor;
	GSSiDeleteObject (&hMemBitmap);
	hMemBitmap = CreateCompatibleBitmap (CurView->hDC,MemMapWidth,MemMapHeight);
    if (!hMemBitmap)
    {   
    	if (startW > 1024)
    	{   
    		startW -= 128;
    		goto ReTry;
    	}
    	GSSiMessageBox ("Unable to create memory bitmap",NULL,MB_ICONEXCLAMATION);
    	return FALSE;
    }
	hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
temp: */
	MemMap = TRUE;
	*MemMapName = 0;
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	DoPaint = TRUE;	
    SaveViewports ();
	CreateStatusWindow (hWndMain,-1,"Creating Export File");
	PaintMap (CurView->hWnd,hdcMemMap,TRUE,NULL); 
	if (FormatOpt == 2)
	{   
		POINT	BPoint[2];
		
		SetViewport (*pCommandViewport);
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
   	}
	RestoreViewports ();
	DestroyStatusWindow(0); 
	hTempBM = CreateCompatibleBitmap (hdcMemMap,10,10);    
	if (Name)
	{
		_fstrcpy (MapName,Name);
		ExpandText (MapName);
		hMemBitmap = SelectObject (hdcMemMap,hTempBM);
   		hDib32 = BitmapToDIB32 (hMemBitmap);  
   		SelectObject (hdcMemMap,hMemBitmap); 
		if (FormatOpt == 2)
  			SetGeoTiffData (hDib32,&ScaleX,&ScaleY,&BitmapPoint[0],&WorldPoint[0]);
   		rtn = SaveDIB32 (hDib32,MapName,-1,0);  
		GMDestroyDIB32 (hDib32);

		if (!rtn)
		{
			sprintf (str,"Error saving bitmap. Code %ld",rtn);
			MessageBox (0,str,NULL,MB_ICONEXCLAMATION);
		}
	
	}
	else
    {
		if (!OpenClipboard (hWnd))
		{  
			MessageBeep (MB_ICONEXCLAMATION);
			MessageBox( GetFocus(), "ERROR: Cannot access the clipboard",NULL, MB_OK|MB_ICONEXCLAMATION);
		}                   
		else
		{
			EmptyClipboard();
			hMemBitmap = SelectObject (hdcMemMap,hTempBM);
			SetClipboardData(CF_BITMAP, hMemBitmap);
			CloseClipboard();  
			SelectObject(hdcMemMap, hMemBitmap);   
		}
	}


	rtn = DeleteLargeDC (hdcMemMap,hbmpOld);   
/*	SelectObject(hdcMemMap,hbmpOld); 
	GSSiDeleteObject (&hMemBitmap);
	DeleteDC (hdcMemMap);*/      
	GSSiDeleteObject (&hTempBM);
	hdcMemMap = 0;
	MemMap = SaveMemMap;
	MemMapWidth = SaveMMW;
	MemMapHeight = SaveMMH; 
	GSSiSetCursor (hcurSave);
	BufferedScreen = SaveBS; 
  
	return TRUE;
}
			
BOOL SetupVirtualPrinter (HWND hWnd,LPSTR VPName)
{
    BOOL nRc;
   	FARPROC lpfnVIRTUALPRINTERMsgProc; 
    
    _fstrcpy (VirtualPrinterPathname,VPName);
    lpfnVIRTUALPRINTERMsgProc = MakeProcInstance((FARPROC)VIRTUALPRINTERMsgProc, hInst);
    nRc = DialogBox(hInst, (LPSTR)"VIRTUALPRINTER", hWnd, lpfnVIRTUALPRINTERMsgProc);
    FreeProcInstance(lpfnVIRTUALPRINTERMsgProc);
	return nRc;
} 

BOOL ComputePrinterWandH (HWND hWndDlg)
{	
	HWND	hPr;
    LPPRINTDLG lpPDChunk; 
	
/*           	KeepMemLength = TRUE;  
	        IgnoreLock = TRUE;
			if (Length)
			{ 
				lpPDChunk->hDevMode = GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
				pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
				GSSilread (*Fid,pDevMode,Length);
				GlobalUnlock (lpPDChunk->hDevMode);
			}
			GSSilread (*Fid,&Length,2); 
			if (Length)
			{ 
				lpPDChunk->hDevNames = GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
				pDevNames = (LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
				GSSilread (*Fid,pDevNames,Length);
				GlobalUnlock (lpPDChunk->hDevNames);
			}
   	        KeepMemLength = KeepMemLengthSave;
	        IgnoreLock = FALSE;   
			AutoPrintOK = TRUE;
  	       	PrintDlg(lpPDChunk);   
	    	hPr = lpPDChunk->hDC; */  
	return TRUE;
}

BOOL FAR PASCAL VIRTUALPRINTERMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[256];
    LPPRINTDLG lpPDChunk,lpPDChunkLocal; 

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         if (!_fstricmp (VirtualPrinterPathname,"Create New Virtual Printer"))
         	*VirtualPrinterPathname=0;
         else
         {
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
    
    case WM_DESTROY:  
    	 if (hPDChunkLocal)
    	 {  
    	 	lpPDChunkLocal = (LPPRINTDLG)GlobalLock (hPDChunkLocal);
		    IgnoreLock = TRUE;
		    GSSiGlobFree(&lpPDChunkLocal->hDevMode);
		    GSSiGlobFree(&lpPDChunkLocal->hDevNames);
		    IgnoreLock = FALSE;
			GSSiGlobUlFree (&hPDChunkLocal);
		 }
    	 break;  
    	 
    case WM_COMMAND:
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
            case IDOK: 
            	ComputePrinterWandH (hWndDlg);  
                EndDialog(hWndDlg, TRUE);

            break;
            case IDC_SELECTPRINTER:
			{
			    long	wSize; 
			    size_t	Length;
			    LPDEVMODE	pDevMode, pDevModeLocal;
			    LPDEVNAMES	pDevNames, pDevNamesLocal; 
			    
			    wSize = sizeof(PRINTDLG);
				lpPDChunkLocal = (LPPRINTDLG)AllocAndLockMem(&hPDChunkLocal, (WORD)wSize);
				if (hPDChunk)
				{   
					BOOL	KeepMemLengthSave = KeepMemLength;
					
					lpPDChunk = (LPPRINTDLG)GlobalLock (hPDChunk);
					*lpPDChunkLocal = *lpPDChunk;
		           	KeepMemLength = TRUE;  
			        IgnoreLock = TRUE;  
			        Length = GlobalSize (lpPDChunk->hDevMode);
					if (Length)
					{ 
						lpPDChunkLocal->hDevMode = GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
						pDevModeLocal = (LPDEVMODE)GlobalLock (lpPDChunkLocal->hDevMode);
						pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode); 
						_fmemmove (pDevModeLocal,pDevMode,Length);
						GlobalUnlock (lpPDChunkLocal->hDevMode); 
						GlobalUnlock (lpPDChunk->hDevMode);
					}
					else
						lpPDChunk->hDevMode = 0;
			        Length = GlobalSize (lpPDChunk->hDevNames);
					if (Length)
					{ 
						lpPDChunkLocal->hDevNames = GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
						pDevNamesLocal = (LPDEVNAMES)GlobalLock (lpPDChunkLocal->hDevNames);
						pDevNames = (LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames); 
						_fmemmove (pDevNamesLocal,pDevNames,Length);
						GlobalUnlock (lpPDChunkLocal->hDevNames); 
						GlobalUnlock (lpPDChunk->hDevNames);
					}
					else
						lpPDChunk->hDevNames = 0;
		   	        KeepMemLength = KeepMemLengthSave;
			        IgnoreLock = FALSE;   
					GlobalUnlock (hPDChunk);
				}
				else
				    InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunkLocal); 
				lpPDChunkLocal->hwndOwner = hWndDlg;	
				lpPDChunkLocal->hDC=0;
				lpPDChunkLocal->Flags = PD_HIDEPRINTTOFILE|PD_NOPAGENUMS|PD_RETURNDC;
				if (PrintDlg(lpPDChunkLocal))
		     	{
				   HWND	hPr=lpPDChunkLocal->hDC;
			       int dpi = GSSiGetDeviceCaps(hPr, LOGPIXELSX); 
			       RECT	Rect;
				   long xPage, yPage;
			       
			       xPage = GSSiGetDeviceCaps(hPr, HORZRES);
			       yPage = GSSiGetDeviceCaps(hPr, VERTRES);  
			       xPage--;
			       yPage--;
			       Rect.left = 0;
			       Rect.top = 0;
			       Rect.bottom = yPage;
			       Rect.right = xPage;
			       yPage = Rect.bottom;
			       IgnoreLock = TRUE;
			       if (dpi)
			       {    
			       		double fwidth =(double)((long)(100*(double)xPage/(double)dpi))/100;
			       		double fheight=(double)((long)(100*(double)yPage/(double)dpi))/100;     
			       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunkLocal->hDevNames);
			       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
			       		
				        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);  
				        SetDlgItemText (hWndDlg,IDC_PHYSICALPRINTER,str);
				        PhysicalPrinterWidth = fwidth;
				        PhysicalPrinterHeight = fheight;
				        GlobalUnlock (lpPDChunkLocal->hDevNames);
				   }
			       IgnoreLock = FALSE;
				   DeleteDC(lpPDChunkLocal->hDC);
				} 
				GlobalUnlock (hPDChunkLocal);
			}
            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 
     
void SavePrintSetupData (HFILE Fid,short dpi)
{
 	short	Length=0, Version=3, id=OB_SAVEPRINTSETUP; 
    RECT	Rect;  
    LPDEVMODE	pDevMode;
    LPDEVNAMES	pDevNames; 
    HANDLE	hDevMode, hDevNames;
    
    if (!hPDChunk)
    	return;	
	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk); 
	hDevMode = lpPDChunk->hDevMode; 
	hDevNames = lpPDChunk->hDevNames;
 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1); 
 	Length = sizeof(PRINTDLG) * sizeof(short) * 3; 
	if (hDevMode)
		Length += GlobalSize (hDevMode);
	if (hDevNames)
		Length += GlobalSize (hDevNames);
	Length += 32;
	BigWrite (Fid,(HPSTR)&Length,2,-1); 
 	Length = sizeof(PRINTDLG);
	BigWrite (Fid,(HPSTR)&Length,2,-1); 
	lpPDChunk->hDevMode = 0; 
	lpPDChunk->hDevNames = 0;
	BigWrite (Fid,(HPSTR)lpPDChunk,Length,-1); 
    IgnoreLock = TRUE;
	if (hDevMode)
	{
		Length = GlobalSize (hDevMode);
		pDevMode = (LPDEVMODE)GlobalLock (hDevMode);
		BigWrite (Fid,(HPSTR)&Length,2,-1); 
		BigWrite (Fid,(HPSTR)pDevMode,Length,-1); 
		GlobalUnlock (hDevMode);
	}
	else
	{
		Length = 0;
		BigWrite (Fid,(HPSTR)&Length,2,-1); 
	}
	if (hDevNames)
	{
		Length = GlobalSize (hDevNames);
		pDevNames = (LPDEVNAMES)GlobalLock (hDevNames);
		BigWrite (Fid,(HPSTR)&Length,2,-1); 
		BigWrite (Fid,(HPSTR)pDevNames,Length,-1); 
		GlobalUnlock (hDevNames);
	}
	else
	{
		Length = 0;
		BigWrite (Fid,(HPSTR)&Length,2,-1); 
	}
	BigWrite (Fid,CustomHeight,16,1);
	BigWrite (Fid,CustomWidth,16,1);
	BigWrite (Fid,(HPSTR)&dpi,2,1);
	BigWrite (Fid,(HPSTR)&PrinterWidth,8,1);
	BigWrite (Fid,(HPSTR)&PrinterHeight,8,1);
    IgnoreLock = FALSE;
	lpPDChunk->hDevMode = hDevMode; 
	lpPDChunk->hDevNames = hDevNames;
	GlobalUnlock (hPDChunk);
	return;
}

BOOL ReadPrintSetupData (HFILE Fid)
{
    short ObjectID,Version;   
	UINT	Length;
    LPDEVMODE	pDevMode;
    LPDEVNAMES	pDevNames; 
    BOOL        KeepMemLengthSave = KeepMemLength;
	
	if (Fid == HFILE_ERROR)
		return FALSE;	    
	GSSilread (Fid,&ObjectID,2);
	if (ObjectID != OB_SAVEPRINTSETUP)
		return FALSE;
	GSSilread (Fid,&Version,2);
	GSSilread (Fid,&Version,2);
	GSSilread (Fid,&Length,2);
	GSSilread (Fid,&Length,2);
	if (hPDChunk)
	{   
		lpPDChunk = (LPPRINTDLG)GlobalLock (hPDChunk);
	    IgnoreLock = TRUE;
	    GSSiGlobFree(&lpPDChunk->hDevMode);
	    GSSiGlobFree(&lpPDChunk->hDevNames);
	    IgnoreLock = FALSE;
        GSSiGlobUlFree (&hPDChunk);
    }
	if (!hPDChunk)
		hPDChunk= GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
	lpPDChunk = (LPPRINTDLG) GlobalLock (hPDChunk);
	GSSilread (Fid,lpPDChunk,Length);
	GSSilread (Fid,&Length,2); 
   	KeepMemLength = TRUE;  
    IgnoreLock = TRUE;
	if (Length)
	{ 
		lpPDChunk->hDevMode = GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
		pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
		GSSilread (Fid,pDevMode,Length);
		GlobalUnlock (lpPDChunk->hDevMode);
	}
	GSSilread (Fid,&Length,2); 
	if (Length)
	{ 
		lpPDChunk->hDevNames = GSSiGlobAlloc (0,GMEM_MOVEABLE,Length);
		pDevNames = (LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
		GSSilread (Fid,pDevNames,Length);
		GlobalUnlock (lpPDChunk->hDevNames);
	} 
	if (Version > 1)
	{
		GSSilread (Fid,CustomHeight,16);
		GSSilread (Fid,CustomWidth,16);
	}
	if (Version > 2)
	{
		GSSilread  (Fid,(HPSTR)&PrinterResolution,2);  
		VirtualPrinterResolution = PrinterResolution;
		GSSilread  (Fid,(HPSTR)&PrinterWidth,8);
		GSSilread  (Fid,(HPSTR)&PrinterHeight,8);  
	}
    KeepMemLength = KeepMemLengthSave;
    IgnoreLock = FALSE; 
    lpPDChunk->hDC = 0;   
   	lpPDChunk->hwndOwner = ghWnd;	
	lpPDChunk->hInstance = ghInst;  
	lpPDChunk->lpfnPrintHook = (FARHOOK)NULL;
    lpPDChunk->lpfnSetupHook = (FARHOOK)NULL;
    lpPDChunk->lpPrintTemplateName = (LPSTR)NULL;
    lpPDChunk->lpSetupTemplateName = (LPSTR)NULL;

	GlobalUnlock (hPDChunk);
	return TRUE;
}

BOOL FAR PASCAL VIRTUAL_PRINTER_CREATEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
 short  BRtn,Error,Choice;
 double	Width,Height; 
 HFILE	Fid;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_ADDSTRING,0,(LPARAM)"200 dpi"); 
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_ADDSTRING,0,(LPARAM)"300 dpi"); 
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_ADDSTRING,0,(LPARAM)"600 dpi"); 
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_SETCURSEL,(WPARAM)1,(LPARAM)NULL); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;  
            
            case IDOK: 
            {
            	 char	VirtualPrinterList[256];
            	 
                 Choice=(short)SendDlgItemMessage(hWndDlg,IDC_VIRTPRINTRES,CB_GETCURSEL,0,0);
                 VirtualPrintDPI = VPRes[Choice];
                 GetDlgItemText (hWndDlg,IDC_VIRTPRINTWIDTH,str,32);
            	 Width = atof (str);
                 GetDlgItemText (hWndDlg,IDC_VIRTPRINTHEIGHT,str,32);
            	 Height= atof (str);
        		 GetGlobalCVal ("[%VIRTUALPRINTERLIST]",VirtualPrinterList,"[%DL]virtualprinters.txt");   
        		 Fid = GSSiOpenFile (VirtualPrinterList,NULL,OF_READWRITE);
            	 if (Fid == HFILE_ERROR)
            	 	Fid = GSSiOpenFile (VirtualPrinterList,NULL,OF_CREATE);
            	 GSSillseek (Fid,0,2);
            	 sprintf (CurVirtPrinter,"%.2f inches wide by %.2f inches high (%i dpi)",Width,Height,VirtualPrintDPI);
            	 fputstring (CurVirtPrinter,Fid);  
            	 GSSiClose (Fid);
                 EndDialog(hWndDlg, TRUE); 
            }
         		 break;
            
         }
         break;   

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL ZoomToVirtualPrint (void) 
{   
	MNMXCORD	PlotBounds, VPBounds;
	int	NumRows, NumCols, irow, icol;  
	long	CurrentPrinterWidth=(long)PrinterRect.right - (long)PrinterRect.left;
	long	CurrentPrinterHeight=(long)PrinterRect.bottom - (long)PrinterRect.top;
	double	ResFactor=1;
	
	if (!GetGlobalBVal2 ("[%SHOWPRINTAREAS]",FALSE) || CurrentPrinterWidth <=0)
		return FALSE;
	
	if (VirtualPrinterResolution && PrinterResolution)
		ResFactor = PrinterResolution / VirtualPrinterResolution;
	SetViewport (*pCommandViewport);
	GetMapBounds (CurView->lpFiles[0],&PlotBounds);  
	NumRows = 1+((PlotBounds.ymx - PlotBounds.ymn-1)*ResFactor)/((long)PrinterRect.bottom - (long)PrinterRect.top); 
	NumCols = 1+((PlotBounds.xmx - PlotBounds.xmn-1)*ResFactor)/((long)PrinterRect.right - (long)PrinterRect.left); 
	VPBounds.xmn = VPBounds.ymn = 0;
	VPBounds.xmx = (NumCols) * CurrentPrinterWidth/ResFactor;
	VPBounds.ymx = (NumRows) * CurrentPrinterHeight/ResFactor;
	ZoomToRect(VPBounds,FALSE);
	return TRUE;
}

void DisplayVirtualPrintAreas (void)
{   
	MNMXCORD	PlotBounds, PageBounds;
	int	NumRows, NumCols, irow, icol;  
	long	CurrentPrinterWidth=(long)PrinterRect.right - (long)PrinterRect.left;
	long	CurrentPrinterHeight=(long)PrinterRect.bottom - (long)PrinterRect.top;  
	char	str[128]; 
	int		Page=1;
	HFILE	Fid; 
	LPSTR	pFile; 
	double	ResFactor=1;
	
	if (Printing || !ShowVirtualPrintAreas)
		return;
	if (VirtualPrinterResolution && PrinterResolution)
		ResFactor = PrinterResolution / VirtualPrinterResolution;
	SetViewport (*pCommandViewport);
    SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC,GF_TEXTMODE);
    SelectClipRgn (CurView->hDC,0);
	
	if (!hVirtualPrintFile)
	{
		hVirtualPrintFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);
		pFile = GlobalLock (hVirtualPrintFile);
		GSSiGetTempFileName(0,"gm1",0,pFile); 
	}
	else
		pFile = GlobalLock (hVirtualPrintFile); 
	_fstrcpy (PMDataFile,pFile);  
	SetGlobalValue ("%FILEPMDATA","Virtual Printer");
	*PMMacroFile = 0;
	SetGlobalValue ("%FILEPMMACRO",""); 
	Fid =	GSSiOpenFile (pFile,NULL,OF_CREATE); 
	GlobalUnlock (hVirtualPrintFile);
	fputstring ("[%PRINTNAME]=@[PrintMergeID];",Fid);
	fputstring ("PrintMergeID,PrintMergeDesc,PrintMergeBounds",Fid);
	GetMapBounds (CurView->lpFiles[0],&PlotBounds);  
	NumRows = 1+((PlotBounds.ymx - PlotBounds.ymn-1)*ResFactor)/((long)PrinterRect.bottom - (long)PrinterRect.top); 
	NumCols = 1+((PlotBounds.xmx - PlotBounds.xmn-1)*ResFactor)/((long)PrinterRect.right - (long)PrinterRect.left); 
	for (irow=0;irow<NumRows;irow++)
	{
		for (icol=0;icol<NumCols;icol++)
		{   
			DPOINT	DPoint;
			POINT	WinPt;
			RECT	Rect;
			
			DPoint.x = PageBounds.xmn = icol * CurrentPrinterWidth / ResFactor;
			DPoint.y = PageBounds.ymn = irow * CurrentPrinterHeight / ResFactor;
			WinPt = BasePtToWinPt (&DPoint);
			Rect.left = WinPt.x;
			Rect.bottom = WinPt.y;  
			DPoint.x = PageBounds.xmx = (icol+1) * CurrentPrinterWidth / ResFactor;
			DPoint.y = PageBounds.ymx =(irow+1) * CurrentPrinterHeight / ResFactor;
			WinPt = BasePtToWinPt (&DPoint);
			Rect.right = WinPt.x;
			Rect.top = WinPt.y; 
			FrameRect (CurView->hDC,&Rect,GetStockObject(BLACK_BRUSH));
			OffsetRect (&Rect,1,1); 
			FrameRect (CurView->hDC,&Rect,GetStockObject(WHITE_BRUSH));
			OffsetRect (&Rect,-2,-2); 
			FrameRect (CurView->hDC,&Rect,GetStockObject(WHITE_BRUSH));
			OffsetRect (&Rect,-1,-1); 
			FrameRect (CurView->hDC,&Rect,GetStockObject(BLACK_BRUSH));  
			sprintf (str,"Page %i,Row %i Col %i,%f %f %f %f",Page++,irow+1,icol+1,PageBounds.xmn,PageBounds.ymn,PageBounds.xmx,PageBounds.ymx);
			fputstring (str,Fid);
		}
	}
	GSSiClose (Fid);                                                        
    RestoreDC (CurView->hDC,-1);
	return;
}

BOOL SplitImage (LPSTR InFile,LPSTR OutDir,LPSTR OutType,short nrows, short ncols)
{   
	HDIB32	hFullImage, hTile; 
	MNMXCORD BitmapBounds, WBounds;
	short	width, height, irow, icol;
	long	left, right, top, bottom, FullRight, FullTop; 
	DPOINT	BitmapPoint, WorldPoint;
	double	ScaleX, ScaleY, WorldWidth, WorldHeight;  
	char	OutFile[256];
	
	hFullImage = BMPHandleFromEXT (InFile);
	if (!hFullImage)
		return FALSE;

	makedirectories (OutDir,TRUE,FALSE); 
	
	GetImageBounds (InFile, hFullImage,&BitmapBounds,&WBounds); 
	FullRight = IDNINT(BitmapBounds.xmx);
	FullTop = IDNINT(BitmapBounds.ymx);
	width = BitmapBounds.xmx / ncols;	
	height =  BitmapBounds.ymx / nrows;
	BitmapPoint.x = BitmapPoint.y = 0;
	top = 0;
	bottom = top + height; 
	WorldWidth = WBounds.xmx - WBounds.xmn;   
	WorldHeight = WBounds.ymx - WBounds.ymn;
	ScaleX = WorldWidth/(BitmapBounds.xmx);
	ScaleY = WorldHeight/(BitmapBounds.ymx);
	for (irow = 0;irow < nrows; irow++)
	{
		left = 0;
		right = left + width;
		WorldPoint.y = WBounds.ymx - top * ScaleY; 
		for (icol = 0; icol < ncols; icol++)
		{
			WorldPoint.x = WBounds.xmn + left * ScaleX;  
			hTile = CopyBMP32 (hFullImage,left,min(FullRight,right),min(FullTop,top),bottom);
	   		SetGeoTiffData (hTile,&ScaleX,&ScaleY,&BitmapPoint,&WorldPoint);  
	   		sprintf (OutFile,"%s\\tile%2.2i%2.2i.tif",OutDir,irow,icol);
	   		SaveDIB32 (hTile,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE); 
			GMDestroyDIB32 (hTile); 
	   		left += width;
	   		right += width;
		} 
		top += height;
		bottom += height;
	}
	GMDestroyDIB32 (hFullImage); 
	return TRUE;
}	

