#include "graphint.h" 
#include "dibapi.h"

#include "gmextern.h"
#include <winspool.h>

#include "cddemo.h"                   


static short	VPRes[3]={200,300,600}; 
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
static	BOOL	HaveSavedVP=FALSE;	
static	char	VirtFmt[8]=".tif", VirtFmt2[8]="*.tif";    
static	long	StatusMacro=0;   
static	BOOL	SaveDH;
static	char	MetaFile[MAX_PATH];
static	char	VirtPlotDir[MAX_PATH];
static	char	PrinterSetup[256];
static	char	PrinterName[256];


extern BOOL PrintWasCanceled;
extern char	CurVirtPrinter[128];
extern char	CustomHeight[16],CustomWidth[16];  
extern char	VirtPrinterImageFile[256];


static	short	CurrentStatusWindowVP;
static	FARPROC lpfnAbortProc=0, lpfnPrintDlgProc=0, lpfnProcessStatusDlgProc;

BOOL CheckPrintAbort (HDC hPr);
void SetVirtPrinterBackground (HDC hDC,COLORREF WindowColor);

LPDEVMODE GetLandscapeDevMode(HWND hWnd, char *pDevice,int nCopies)
   {

   HANDLE      hPrinter;
   LPDEVMODE   pDevMode;
   DWORD       dwNeeded, dwRet;

   /* Start by opening the printer */ 
   if (!OpenPrinter(pDevice, &hPrinter, NULL))
       return NULL;

   /*
    * Step 1:
    * Allocate a buffer of the correct size.
    */ 
   dwNeeded = DocumentProperties(hWnd,
       hPrinter,       /* Handle to our printer. */ 
       pDevice,        /* Name of the printer. */ 
       NULL,           /* Asking for size, so */ 
       NULL,           /* these are not used. */ 
       0);             /* Zero returns buffer size. */ 
   pDevMode = (LPDEVMODE)malloc(dwNeeded);

   /*
    * Step 2:
    * Get the default DevMode for the printer and
    * modify it for your needs.
    */ 
   dwRet = DocumentProperties(hWnd,
       hPrinter,
       pDevice,
       pDevMode,       /* The address of the buffer to fill. */ 
       NULL,           /* Not using the input buffer. */ 
       DM_OUT_BUFFER); /* Have the output buffer filled. */ 
   if (dwRet != IDOK)
   {
       /* If failure, cleanup and return failure. */ 
       free(pDevMode);
       ClosePrinter(hPrinter);
       return NULL;
   }

   pDevMode->dmCopies = nCopies;
   /*
        * Make changes to the DevMode which are supported.
    */ 
   if (pDevMode->dmFields & DM_ORIENTATION)
   {
       /* If the printer supports paper orientation, set it.*/ 
       pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
   }

   if (pDevMode->dmFields & DM_DUPLEX)
    {
       /* If it supports duplex printing, use it. */ 
       pDevMode->dmDuplex = DMDUP_HORIZONTAL;
   }

   /*
    * Step 3:
    * Merge the new settings with the old.
    * This gives the driver an opportunity to update any private
    * portions of the DevMode structure.
    */ 
    dwRet = DocumentProperties(hWnd,
       hPrinter,
       pDevice,
       pDevMode,       /* Reuse our buffer for output. */ 
       pDevMode,       /* Pass the driver our changes. */ 
       DM_IN_BUFFER |  /* Commands to Merge our changes and */ 
       DM_OUT_BUFFER); /* write the result. */ 

   /* Finished with the printer */ 
   ClosePrinter(hPrinter);

   if (dwRet != IDOK)
   {
       /* If failure, cleanup and return failure. */ 
       free(pDevMode);
       return NULL;
   }

   /* Return the modified DevMode structure. */ 
   return pDevMode;

   }
	

int GSSiStartDoc(HDC hPr,HDC PrinterDC,BOOL IsVirtPrinter,CONST DOCINFO *pDI,LPPRINTDLG lpPDChunk)
{
	int	rtn,ii;
	HDC	hDC=hPr;
	short	nCopies = lpPDChunk->nCopies;
	LPDEVMODE pDevMode;
	char	DeviceName[CCHDEVICENAME];

	if (PrinterDC)
		hDC = PrinterDC;
	if (lpPDChunk->hDevMode)
	{
		HDC rtnDC;

		IgnoreLock = TRUE;
		pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
		strcpy (DeviceName,pDevMode->dmDeviceName);
		pDevMode->dmCopies = lpPDChunk->nCopies;
		rtnDC = ResetDC (hPr,pDevMode);
		GlobalUnlock (lpPDChunk->hDevMode); 
		IgnoreLock = FALSE;
	}
//	ii=Escape(hDC, SETCOPYCOUNT, sizeof(short),(LPCSTR) &lpPDChunk->nCopies, &nCopies);
//    pDevMode = GetLandscapeDevMode(hWndMain, DeviceName,lpPDChunk->nCopies);
//	ResetDC (hPr,pDevMode);
	if (IsVirtPrinter)
		rtn = 1;
	else
		rtn = StartDoc (hDC,pDI);
//	ii=Escape(hDC, SETCOPYCOUNT, sizeof(short),(LPCSTR) &lpPDChunk->nCopies, &nCopies);
	if (rtn < 0)
		ii=GetLastError ();
 
	return rtn;
}

int GSSiEndDoc(HDC hPr,HDC PrinterDC,BOOL IsVirtPrinter)
{
	int	rtn=0;
	int ierr=0;
	HDC	hDC=hPr;

	if (PrinterDC)
	{
		hDC = PrinterDC;
		rtn = EndDoc (hDC);
		IsVirtPrinter = TRUE;
	}
	else if (!IsVirtPrinter)
		rtn = EndDoc (hPr);
	GSSiDeleteDC(hPr,IsVirtPrinter); 
	if (rtn <= 0)
		ierr = GetLastError ();
	return rtn;
}

int GSSiStartPage (HDC hPr,HDC PrinterDC,BOOL IsVirtPrinter)
{
	int	rtn=1;
	int ierr=0;
	HDC	hDC=hPr;

	if (PrinterDC)
	{
		RECT	Rect;
		int	PageWidth = GetDeviceCaps(PrinterDC, HORZRES);
		int	PageHeight = GetDeviceCaps(PrinterDC, VERTRES);

		Rect.left = Rect.top= 0;
		Rect.right = PageWidth;
		Rect.bottom = PageHeight;
		hDC = PrinterDC;
	    SetDisplayMode (hPr, GF_SCREENMODE);
		SelectClipRgn (hPr,0);
		FillRect (hPr,&Rect,GetStockObject (WHITE_BRUSH));
		rtn = StartPage (hDC);
	}
	else if (IsVirtPrinter)
	{
   		SetVirtPrinterBackground (hPr,WindowColor);
	}
	else
		rtn = StartPage (hPr);
	
	if (rtn <= 0)
		ierr = GetLastError ();
	return rtn;
}

int GSSiEndPage (HDC hPr,HDC PrinterDC,BOOL IsVirtPrinter,HDC mfDC)
{
	int	rtn=-1;
	int ierr=0;

	if (PrinterDC)
	{
		int	PageWidth = GetDeviceCaps(PrinterDC, HORZRES);
		int	PageHeight = GetDeviceCaps(PrinterDC, VERTRES);
		HBITMAP	hBmp = SelectObject (hPr,hbmpVirtPrinterOld);
		BITMAP	bm;

	    if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm))
		   return -1;
		//GM32SaveBitmap(hBmp, "c:\\temp\\print.bmp", 0, 0);

		if (StartPage(PrinterDC) > 0)
		{  
			int		nCopies = lpPDChunk->nCopies;
			short	ii=Escape(PrinterDC, SETCOPYCOUNT, sizeof(int),(LPCSTR) &nCopies, &nCopies);
			int		iBegScanLine = 0, nScanLines = bm.bmHeight,iTop = bm.bmHeight;
			HDIB	hDIB = BitmapToDIB2 (hBmp, 0,&iBegScanLine,&nScanLines);
			
			if (hDIB)
			{
				StartPage (PrinterDC); 
				SetWindowOrgEx  (PrinterDC, 0, 0,0 );
				SetViewportOrgEx(PrinterDC, 0, 0,0 );    
				SetMapMode    (PrinterDC, MM_TEXT );
			}
			while (hDIB)
			{
				LPBITMAPINFOHEADER	pDibInfo = (LPBITMAPINFOHEADER)GlobalLock (hDIB);
				LPSTR	pImage = FindDIBBits ((LPSTR)pDibInfo);
				
				iTop -= nScanLines;
				ii = StretchDIBits (PrinterDC, 0,iTop,PageWidth,nScanLines, 0,0,PageWidth,nScanLines,pImage,
				  (LPBITMAPINFO)pDibInfo,
				  (UINT)DIB_RGB_COLORS,
				  (DWORD) SRCCOPY); 
				GlobalUnlock (hDIB);
				DestroyDIB (hDIB);
				//ii=BitBlt(PrinterDC, 0,0,PageWidth,PageHeight, hPr, 0,0,SRCCOPY);
				iBegScanLine += nScanLines;
				if (iBegScanLine < bm.bmHeight)
					hDIB = BitmapToDIB2 (hBmp, 0,&iBegScanLine,&nScanLines);
				else
				{
					hDIB = 0;
					rtn = EndPage (PrinterDC);
				}
			}
		}
		SelectObject (hPr,hBmp);
	}
    else if (mfDC)
    {
	   DWORD	ierr;
	   HANDLE	hMF = CloseEnhMetaFile(hPr);
	   if (!hMF)
	   {
		   char	Mess[256];
		   GetSystemErrMessage (GetLastError(),Mess);
		   MessageBox (0,Mess,"Error Closing Enhanced Metafile",MB_ICONEXCLAMATION);
	   }
	   else
	   {
		    int i,ii;
			BOOL	st;
			RECT	WindowRect;
			RECT	rect;
			int		VirtualPageRow,VirtualPageCol;  
			char	OutFile[MAX_PATH];
			HDC		hDC, hDCMain = GetDC (hWndMain);
			HBITMAP	hBM, hBMOld;
			

			hDC = CreateCompatibleDC(hDCMain);
			hBM = CreateCompatibleBitmap(hDCMain,VirtualPageWidth,VirtualPageHeight); 
			ReleaseDC (hWndMain,hDCMain);
			 
			for (VirtualPageRow = 0;VirtualPageRow < NumVirtualRows; VirtualPageRow++)
			{
				for (VirtualPageCol = 0;VirtualPageCol < VirtualPagesPerRow; VirtualPageCol++)
				{
					HDIB32	hDib32;
					ENHMETAHEADER emfh;
					XFORM	xForm;
					int b=1;
					int	wox=0,woy=0,vox=0,voy=0,wex=VirtualPlotWidth,wey=VirtualPlotHeight,vex=VirtualPlotWidth,vey=VirtualPlotHeight;
					RECT	rect=MainRect;
					double	f1=1,f2=1,f3=1;
					GetEnhMetaFileHeader(hMF,sizeof(ENHMETAHEADER),&emfh);

					sprintf (OutFile,"%s\\%3.3i%3.3i%s",VirtPlotDir,VirtualPageRow,VirtualPageCol,VirtFmt);
					hBMOld = SelectObject(hDC,hBM);
					
					SetDisplayMode (hDC,GF_TEXTMODE);
					SelectClipRgn (hDC,0);

					while (b)
					{
					SetDisplayMode (hDC,GF_TEXTMODE);
					FillRectPoly (hDC,&MainRect,RGB(255,255,255));

			SetGraphicsMode(hDC, GM_ADVANCED);

			SetWindowOrgEx  ( hDC, wox*f1,woy*f1,0 );
			SetWindowExtEx(   hDC, wex*f1,wey*f1,0 );  

			SetViewportOrgEx (hDC, vox*f2,voy*f2,0);
			SetViewportExtEx( hDC, vex*f2,vey*f2,0 );  
			xForm =  SetXFORMFromTRANS (0);

			SetWorldTransform(hDC, &xForm); 
    		if (!SetMapMode    ( hDC, MM_ISOTROPIC ))
				ii=1;
			
			rect.left = MainRect.left * f3;
			rect.right = MainRect.right * f3;
			rect.top = MainRect.top * f3;
			rect.bottom = MainRect.bottom * f3;

					st=PlayEnhMetaFile(hDC,hMF,&rect);
					hBM = SelectObject (hDC,hBMOld);
					hDib32 = BitmapToDIB32 (hBM);
					{
						char str[MAX_PATH];
						sprintf (str,"$TEXTTOCLIPBOARD(%s)",OutFile);
						ProcessText (str);
					}
			   		SaveDIB32 (hDib32,OutFile,0,-1);//FIF_TIFF,TIFF_ADOBE_DEFLATE); 
					FreeImage_Unload(hDib32);
					hBMOld = SelectObject(hDC,hBM);
					}
		
				}
			}

			
			/*if (ii)SaveDCBitMap (hPr,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE,WindowColor, 
				   						  hOverViewBitmap,
						   			      NumVirtualRows-VirtualPageRow-1,VirtualPageCol,
						   			      OverViewWidth/VirtualPagesPerRow,OverViewHeight/NumVirtualRows,OverViewHeight);



			HDC	hDC = GetDC (hWndMain);
			SetDisplayMode (hDC,GF_TEXTMODE);
			SelectClipRgn (hDC,0);
			GetClientRect (hWndMain,&WindowRect);
			f1 = (double)RECTWIDTH (&WindowRect) / (double)RECTWIDTH (&MainRect);
			f2 = (double)RECTHEIGHT (&WindowRect) / (double)RECTHEIGHT (&MainRect);
			f = min (f1,f2);
			SetViewportExtEx( hDC, 1024*f/2, 1024*f/2,0 );  
			SetViewportOrgEx  ( hDC, 100, 0,0 );


			rect.left = MainRect.left * f;
			rect.right = MainRect.right * f;
			rect.top = MainRect.top * f;
			rect.bottom = MainRect.bottom * f;
			FillRectPoly (hDC,&MainRect,RGB(255,255,200));
			st=PlayEnhMetaFile(hDC,hMF,&rect);
			ReleaseDC (hWndMain,hDC);*/

			DeleteEnhMetaFile (hMF);
	   }
    }
	else if (IsVirtPrinter)
	{
		HBITMAP hBitMap = SelectObject (hPr,hbmpVirtPrinterOld);

		if (hBitMap)
		{
			HDIB32 hDib = BitmapToDIB32 (hBitmap);
			int	Flag=0;

			if (hDib)
			{
				char OutFile[256];

				strcpy (OutFile,VirtPrinterImageFile);
				ExpandText (OutFile);
				rtn = GMFIBMPHandleToEXT (OutFile,hDib,Flag);
				FreeImage_Unload(hDib);
				rtn = 0;
			}
			SelectObject (hPr,hBitMap);
		}

	}
	else
		rtn = EndPage (hPr);
	if (rtn <= 0)
		ierr = GetLastError ();
	return rtn;
}

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
	CreateStatusWind (hWndMain,1,Mess);
Next:
        GetNextIndexEntry(lpIndex); 
        sprintf (TileName,"%s%s",VPDir,lpIndex->CurrentEntry->Name);
        hDIBTile = BMPHandleFromEXT (TileName);
        PasteDIB (hDIB,hDIBTile,(DWORD)lpIndex->CurrentEntry->Bounds.xmn,Height - (DWORD)lpIndex->CurrentEntry->Bounds.ymx,256);   
        GMDestroyDIB32 (hDIBTile);
        sprintf (Mess,"Adding tile %ld",++CurLoc);
		StatusWindowUpdate (0,Mess, TotNum, CurLoc);
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
		StatusWindowUpdate (0,"Writing file to disk", TotNum, CurLoc);
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
	GSSiGetTempFileName(0,"gma",0,TempName);
	Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
	SearchFilesInDir (VirtPlotDir,VirtFmt , Fid,&TotFiles,VirtFmt2,1,TRUE,TRUE);     
	GSSiClose (Fid);
	CreateMapIndex (VirtPlotDir,TempName,4,FALSE,"","",FALSE,1,0,0,0,FALSE);
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
	Rect.right = VirtualPlotWidth;//VirtualPageWidth-1;
	Rect.bottom = VirtualPlotHeight;//VirtualPageHeight-1;
	SaveDC (hDC);  
  	SelectClipRgn (hDC,0);
		FillRectPoly (hDC,&Rect,WindowColor); 
	RestoreDC (hDC,-1);  
	return;
}

void SaveViewports (int from)
{  
	short	iview;
	LPVIEWPORT	lpSaveView; 
	LPVIEWPORT	saveVP=CurView;

	HaveSavedVP = TRUE;
	for (iview=0;iview<*pNumViewports;iview++)
	{
		SetCurView (pViewports[iview]);
        GSSiDeleteObject(&CurView->hRgn);
	    CloseTRANS2 (&CurView->hTranVPToBase);
	    CloseTRANS2 (&CurView->hTranBaseToVP); 
	    CloseTRANS2 (&CurView->hTranVPToScreen);
	    CloseTRANS2 (&CurView->hTranScreenToVP); 
	    CloseTRANS2 (&CurView->hFileTransIn);
		GSSiGlobFree (&CurView->ToolbarHandle);
	    GSSiGlobFree (&CurView->hTAGList);  
//	    GSSiGlobFree (&CurView->hProfileElev);
//	    GSSiGlobFree (&CurView->hProfileRoute);
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
		if (from)
			CurView->LastWidth = 0;
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
	CurView = saveVP;
	return;
}

void RestoreViewports (void)
{   
	short	iview;
	LPVIEWPORT	lpSaveView; 
	LPVIEWPORT	saveVP=CurView;
	
	if (!HaveSavedVP)
		return;
	HaveSavedVP = FALSE;
	for (iview=0;iview<*pNumViewports;iview++)
	{
		SetCurView (pViewports[iview]);
        GSSiDeleteObject(&CurView->hRgn);
	    CloseTRANS2 (&CurView->hTranVPToBase);
	    CloseTRANS2 (&CurView->hTranBaseToVP); 
	    CloseTRANS2 (&CurView->hTranVPToScreen);
	    CloseTRANS2 (&CurView->hTranScreenToVP); 
	    CloseTRANS2 (&CurView->hFileTransIn);
		GSSiGlobFree (&CurView->ToolbarHandle);
	    GSSiGlobFree (&CurView->hTAGList);
	   // GSSiGlobFree (&CurView->hProfileElev);
	   // GSSiGlobFree (&CurView->hProfileRoute);
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
			lpSaveView->pPickList1 = CurView->pPickList1;
			lpSaveView->pPickListManual = CurView->pPickListManual;
	   		*CurView = *lpSaveView;
			CurView->NumMaskPoints = SaveMaskNumPoints[iview];
			CurView->NumMaskAreaParts = SaveMaskNumParts[iview];
			CurView->hMaskArea = SaveMaskArea[iview];    
			CurView->DisableZoomMacro = SaveDisableZoomMacro[iview];
			GSSiGlobUlFree (&hSaveView[iview]); 
			if (CurView->CurrentFunction == GF_TOOLBAR)
				LoadVPToolBar (); 
		}
		CurView->DisplayCycle = 0;			
		SetBounds(CurView->hWnd,0); /*Null prevents redisplay of view bounds*/
	} 
	CurView = saveVP;
	return;
}
	
BOOL PrintReport2 (HDC hPr,HDC PrinterDC,BOOL IsVirtPrinter,HDC mfDC,LPSTR ReportName,long Refno,LPSTR Prefix,LPSTR UDI,LPSTR InitCmd)
{    
	char	txt[64];
	LPVIEWPORT	SaveView;        
	HANDLE	hView;   
	BOOL	SaveUMRP = UseMultReportPages;
	COLORREF saveWindowColor = WindowColor;

	WindowColor = RGB (255,255,255);
	
	UseMultReportPages=TRUE;
	GSSiStartPage (hPr,PrinterDC,IsVirtPrinter); 
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
	if (InitCmd)
		ProcessText (InitCmd);
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
		if (IsVirtPrinter)
			Factor = (double)VirtualPrintDPI / 72;
		else
			Factor = Pixelsperinch/72;
		//Factor = 1;
		DisplayReport2 (hPr, CurView->hReport,MainRect,Factor,FALSE,FALSE);
	}
	UnloadReport (&CurView->hReport); 
	GSSiGlobUlFree (&hView);
	SetCurView ( SaveView);
	UseMultReportPages=SaveUMRP;
    GSSiEndPage (hPr,PrinterDC,IsVirtPrinter,mfDC);
	WindowColor = saveWindowColor;
	return TRUE;
		   
}

BOOL CreateStatusWind (HWND hWnd,int nStatusBarsIN,LPSTR Title)
{   
	short	nStatusBars = abs (nStatusBarsIN);
	BOOL	AllowBlock=TRUE;
	
	if (nStatusBarsIN < 0)
		AllowBlock = FALSE;     
	StatusMacro = CurrentMacro;
    if (!hSaveStatWindowBM && AllowBlock)  
    {
		ClearFullWindowBitmap (0);
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
		if (BackgroundTask)
			ShowWindow (ghPrintingDlg,SW_HIDE);
	}
	else 
	{
		UseSecondStatus = TRUE;
		nStatusBars = 2;
	}
    if (nStatusBars > 1 && PrintMsgWnd && !BackgroundTask)
    	ShowWindow (GetDlgItem(PrintMsgWnd,IDC_STATUS),SW_SHOW);  
    else
    	ShowWindow (GetDlgItem(PrintMsgWnd,IDC_STATUS),SW_HIDE); 
    SaveDH = DisableHalt; 
    DisableHalt = AllowBlock;  
	CheckForContinue(TRUE, 0);
	return TRUE;
} 

void StatusExtraInfoUpdate(LPSTR mess)
{
	if (PrintMsgWnd)
		SetDlgItemText(PrintMsgWnd, IDC_PRINTERINFO, mess);
	else SetWindowText(hWndMain, mess);
	return;
}

BOOL StatusWindowUpdate (LPSTR Title, LPSTR Mess, DWORD Tot, DWORD Done)
{   
	char	MessText[256];
	
	if (!PrintMsgWnd)
		return FALSE;  
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

BOOL StatusWindowUpdate2 (LPSTR Mess, DWORD Tot, DWORD Done)
{ 
	char	MessText[256];
	if (!PrintMsgWnd)
		return FALSE;
	if (Mess)
	{
		_fstrcpy (MessText,Mess);
		ExpandText (MessText);
		SetDlgItemText (PrintMsgWnd,PRINT_FILE,MessText);
	}
    if (Tot)
		PctBox (GetDlgItem(PrintMsgWnd,IDC_STATUS), Tot, Done,0);
	if (gbUserAbort)
		return FALSE;

	return TRUE;
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
	StatusMacro = 0;
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
	CheckForContinue(TRUE, 0);
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

BOOL FAR PASCAL ProcessStatusDlgProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
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
      switch (LOWORD(wParam))
         {
      case IDCANCEL:
         ContinueProcessing=FALSE;
         DestroyWindow(ProcessStatusWnd); 
         HaltMapDisplay (FALSE,FALSE);
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

BOOL FAR PASCAL TemplateDlgProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
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
      switch (LOWORD(wParam))
         {
      case IDCANCEL:
         ContinueProcessing=FALSE;
         DestroyWindow(ProcessStatusWnd); 
         HaltMapDisplay (FALSE,FALSE);
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

void strncpy0s (LPSTR to,LPSTR from,int l,char fill)
{
	int	i;

	for (i=0;i<l;i++,to++,from++)
	{
		if (*from)
			*to = *from;
		else
			*to = fill;
	}
	*to = 0;
	return;
}

BOOL FAR PASCAL DeconstructMsgProc (HWND hWndDlg, int message, WPARAM wParam, LPARAM lParam)
{
#define MAXITEM	1024
 int	i;
 static	HANDLE hMem=0;
 LPSTR pMem, pMemStart;
 static int	Curloc=0, CurCharLen = 1;
 static UINT ItemType[MAXITEM];
 static	int	ItemLen[MAXITEM], Offset[MAXITEM],NumItem;
 int ResetCurloc,lc,Err;
 char	str[USHRT_MAX];
 char	File[MAX_PATH]="c:\\curvedecode";
 static	BOOL	Skip,HaveInit=FALSE;
   switch (message)
      {
   case WM_INITDIALOG: 
	   {
		   char Ext[6]=".bin";  
		   if (!GetFileName2 (hWndDlg,File,Ext,0))
			   break;
	   }
	   {
 		 HFILE Fid=GSSiOpenFile (File,0,OF_READ);
		 int	len = GSSifilelength (Fid);
		 
		 SetDlgItemText (hWndDlg,IDC_OPENPATH,File);
		 SetDlgItemInt (hWndDlg,IDC_INFO,len,TRUE);
		 NumItem = 0;
		 Curloc  = 0;
		 CurCharLen = 0;
		 hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,len+1024);
		 pMem = (LPSTR)GlobalLock(hMem);

		 BigRead (Fid,pMem,len);
		 strcpy(pMem+len,"END-OF-FILE|END-OF-FILE|END-OF-FILE|END-OF-FILE|END-OF-FILE|END-OF-FILE|");
		 GlobalUnlock (hMem);
		 GSSiClose (Fid);
		 SetDlgItemText (hWndDlg,IDC_CL,"0");
		 cwCenter(hWndDlg, 0);
		 HaveInit = TRUE;
    case GSSI_REINITDIALOG:
		 pMemStart = GlobalLock (hMem);
	     SendDlgItemMessage (hWndDlg,IDC_DECONSTRUCT_LIST,LB_RESETCONTENT,0,0);
		 for (i=0;i<NumItem;i++)
		 {
			 pMem = pMemStart + Offset[i];
			 switch (ItemType[i])
			 {
			 case IDC_I2:
				 sprintf (str,"%i\t%i\tI2\t%i",i,Offset[i],*(LPSHORT)pMem);
				 break;
			 case IDC_I4:
				 sprintf (str,"%i\t%i\tI4\t%i",i,Offset[i],*(LPLONG)pMem);
				 break;
			 case IDC_R4:
				 sprintf (str,"%i\t%i\tR4\t%f",i,Offset[i],*(LPFLOAT)pMem);
				 break;
			 case IDC_R8:
				 sprintf (str,"%i\t%i\tR8\t%f",i,Offset[i],*(LPDOUBLE)pMem);
				 break;
			 case IDC_CHAR:
				 {
					 char	ctmp[USHRT_MAX];

					 strncpy0s (ctmp,pMem,ItemLen[i],'~');
					 sprintf (str,"%i\t%i\tC%i\t%s",i,Offset[i],ItemLen[i],ctmp);
				 }
				 break;
			 }
			 SendDlgItemMessage (hWndDlg,IDC_DECONSTRUCT_LIST,LB_ADDSTRING,0,(LPARAM)str);
			 pMem += ItemLen[i];
		 }
		 SendDlgItemMessage (hWndDlg,IDC_DECONSTRUCT_LIST,LB_SETTOPINDEX,NumItem-1,(LPARAM)0);
		 GlobalUnlock (hMem);
		 pMem = GlobalLock (hMem);
		 pMem += Curloc;
		 strncpy0s (str,pMem,CurCharLen,'~');
		 SetDlgItemText (hWndDlg,IDC_CD,str);
		 *str = 0;
		 for (i=0;i<CurCharLen;i++)
			sprintf (strchr (str,0),"%3i|",*(LPBYTE)(pMem+i));
		 SetDlgItemText (hWndDlg,IDC_CNUM,str);

		 pMem += CurCharLen;
		 itoa (*(LPSHORT)pMem,str,10);
		 SetDlgItemText (hWndDlg,IDC_I2D,str);
		 itoa (*(LPLONG)pMem,str,10);
		 SetDlgItemText (hWndDlg,IDC_I4D,str);
		 ftoa (str,*(LPFLOAT)pMem);
		 SetDlgItemText (hWndDlg,IDC_R4D,str);
		 ftoa (str,*(LPDOUBLE)pMem);
		 SetDlgItemText (hWndDlg,IDC_R8D,str);
		 if (message == GSSI_REINITDIALOG && lParam)
			 Curloc = lParam;
		 Skip = TRUE;
		 SetDlgItemInt (hWndDlg,IDC_CURLOC,Curloc,TRUE);
		 GlobalUnlock (hMem);
	   }
   		break;

   case WM_DESTROY:
		GSSiGlobFree (&hMem);
		HaveInit = FALSE;
        break;
   case WM_COMMAND:
      switch (LOWORD(wParam))
         {
			case IDC_I2:
			case IDC_I4:
			case IDC_R4:
			case IDC_R8:
				lc = GetDlgItemInt (hWndDlg,IDC_CL,&Err,TRUE);
				if (lc)
				{
					Offset[NumItem] = Curloc;
	                Curloc += lc;
					ItemLen[NumItem] = lc;
					ItemType[NumItem++] = IDC_CHAR;
					CurCharLen = 0;
					SetDlgItemText (hWndDlg,IDC_CL,"0");
				}
			default:
			  break;
	  }
      switch (LOWORD(wParam))
         {
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;  
			
            case IDC_DUMP: 
				{
					int i=0;
					HFILE	Fid=GSSiOpenFile ("c:\\temp.txt",0,OF_CREATE);
					while (SendDlgItemMessage(hWndDlg,IDC_DECONSTRUCT_LIST,LB_GETTEXT,i++,(LPARAM)str) != LB_ERR)
					{
						fputstring (str,Fid);
					}
					GSSiClose (Fid);
				}
            break;  
			
            case IDC_CURLOC:
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
						if (Skip)
						{
							Skip = FALSE;
							break;
						}
                        Curloc = GetDlgItemInt (hWndDlg,IDC_CURLOC,&Err,TRUE);
						PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
                        break;
                 }
                 break;
			case IDC_I2:
				Offset[NumItem] = Curloc;
                Curloc += 2;
				ItemLen[NumItem] = 2;
				ItemType[NumItem++] = LOWORD(wParam);
				CurCharLen = 0;
				SetDlgItemText (hWndDlg,IDC_CL,"0");
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				break;

			case IDC_I4:
				Offset[NumItem] = Curloc;
                Curloc += 4;
				ItemLen[NumItem] = 4;
				ItemType[NumItem++] = LOWORD(wParam);
				CurCharLen = 0;
				SetDlgItemText (hWndDlg,IDC_CL,"0");
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				break;

			case IDC_R4:
				Offset[NumItem] = Curloc;
                Curloc += 4;
				ItemLen[NumItem] = 4;
				ItemType[NumItem++] = LOWORD(wParam);
				CurCharLen = 0;
				SetDlgItemText (hWndDlg,IDC_CL,"0");
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				break;

			case IDC_R8:
 				Offset[NumItem] = Curloc;
                Curloc += 8;
				ItemLen[NumItem] = 8;
				ItemType[NumItem++] = LOWORD(wParam);
				CurCharLen = 0;
				SetDlgItemText (hWndDlg,IDC_CL,"0");
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				break;

			case IDC_CHAR:
				Offset[NumItem] = Curloc;
				lc = GetDlgItemInt (hWndDlg,IDC_CL,&Err,TRUE);
                Curloc += lc;
				ItemLen[NumItem] = lc;
				ItemType[NumItem++] = LOWORD(wParam);
				CurCharLen = 0;
				SetDlgItemText (hWndDlg,IDC_CL,"0");
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				break;

			case IDC_CL:
				if (!HaveInit)
					return FALSE;
				lc = GetDlgItemInt (hWndDlg,IDC_CL,&Err,TRUE);
				if (lc < 0)
					break;
				ResetCurloc = Curloc;
				CurCharLen = lc;
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, ResetCurloc); 
				break;

			case IDC_BACK:
				if (!NumItem)
					break;
				Curloc -= ItemLen[--NumItem];
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0);
				break;

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
		
		if (hBitMap != hbmpVirtPrinterOld)
			GSSiDeleteObject (&hBitMap);
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
	BOOL		DoRedisplay = TRUE;
	char	Warning[128]="";  
	short	ivp; 
	BOOL IsVirtPrinter=FALSE;
	HANDLE hVirtPrinter=0;
	BOOL SaveIPV = InPlotView; 
	char	VirtPlotName[128];    
	DWORD	SaveFlags;   
	DWORD	Err;    
	HBITMAP	hOverViewBitmap=0; 
	long	OverViewWidth, OverViewHeight;
	char	OutFile[MAX_PATH]; 
	HWND	hWnd2, ghWnd; 
	HENHMETAFILE	hMF=0;
	short	i;
	BOOL	SetBuf=FALSE;
	HDC		mfDC = 0, PrinterDC=0;
	HDC		*pPrinterDC = &PrinterDC;
	char	InitCmd[256];
    
	NumVirtualPages = 1;
	GSSiTrace ("Begin Printmap",0);    
	
	if (Page < TotPage)
		DoRedisplay = FALSE;
	if (!hWnd)
		PrintPrompt = FALSE; 
	else
		ghWnd = hWnd;
	hWnd = NULL;
	HaveReports = FALSE; 
	Printing = TRUE;
	DisplayTAGs2 (0,4,0); //scans for reports
	Printing = FALSE;
	AddReportToPrintList (0,0,0,0,0);
	DeleteAllVPRegions ();
	if (Page >= TotPage)
		LastPage = TRUE;
 	SaveWidthFactor = WidthFactor;
	if (!OpenConfig(0,0)) return(FALSE);

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
		hSaveBM = SaveScreen2 (hDC,MRect,0,0);	
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
	 {
		lpPDChunk->Flags = lpPDChunk->Flags|PD_RETURNDC|PD_ENABLESETUPTEMPLATE|PD_ENABLESETUPHOOK|PD_PRINTSETUP|PD_USEDEVMODECOPIESANDCOLLATE;
//		if (App==2)
//			lpPDChunk->Flags = lpPDChunk->Flags|PD_ENABLESETUPTEMPLATE|PD_ENABLESETUPHOOK|PD_PRINTSETUP;
	 }
	 if (!PrintPrompt && Page == 1)
	 {
		PrintPrompt = TRUE;   
		DoPrint = TRUE;
		goto S10;
	 }
	 else if (Page > 1)
	 {
	 	PrintPrompt = FALSE;   
	 	DoPrint = TRUE;
		hPr = lpPDChunk->hDC;
	 	goto S10;
	 }
	 lpPDChunk->hDC=0;  
	 lpPDChunk->lpfnSetupHook = (LPOFNHOOKPROC)PrintSetupHook;   
	 lpPDChunk->lpSetupTemplateName = "PRNSETUPDLGGM";
	 SetCurView (pViewportsD[0]); 
	 if (CurView->WidthType == 2 || CurView->DesiredWidth > CurView->DesiredHeight)
	 	ForceOrient = DMORIENT_LANDSCAPE;
	 if (ForceOrient && !ShowVirtualPrintAreas)
	 { 
	 	SaveFlags = lpPDChunk->Flags;
	    
	    lpPDChunk->Flags = PD_RETURNDEFAULT;
	    GSSiPrintDlg(lpPDChunk,0,&hVirtPrinter,0);
	    lpPDChunk->Flags = SaveFlags; 
	    if (lpPDChunk->hDevMode)
	    { 	
	    	IgnoreLock = TRUE;   
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			pDevMode->dmCopies = lpPDChunk->nCopies;
			pDevMode->dmOrientation = ForceOrient; 
			pDevMode->dmFields = pDevMode->dmFields | DM_ORIENTATION;
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
		GSSiGlobFree (&hVirtPrinter);
	 }
	 if (TotPage == -2 || Page < TotPage)
		 pPrinterDC = 0;
	 if (!(DoPrint = GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter,pPrinterDC)))
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
			if (PrintPrompt)
				lpPDChunk->Flags = lpPDChunk->Flags^PD_RETURNDEFAULT;
			else
				lpPDChunk->Flags = lpPDChunk->Flags|PD_RETURNDEFAULT;
			GSSiGlobFree (&hVirtPrinter); 
    		DoPrint = GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter,pPrinterDC);
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
     	if (hPr && TotPage == -2) //-2 == print setup call
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
	       SetMainRect (0,hPr,&Rect,3); 
	       yPage = Rect.bottom;
	       IgnoreLock = TRUE;
		   if (lpPDChunk->hDevNames)
	       {    
	       		double fwidth =(double)((long)(100*(double)xPage/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)yPage/(double)dpi))/100;     
	       		LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       		LPSTR	PrinterName=(LPSTR)pdn+pdn->wDeviceOffset;  
	       		
	       		if (IsVirtPrinter)
	       			_fstrcpy (PrinterName,CurVirtPrinter);
	       	//	if (xPage < 0 || xPage > SHRT_MAX || yPage < 0 || yPage > SHRT_MAX)
	       		if (xPage < 0  || yPage < 0 )
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
	 if (*pPrinterDC)
		hDCPrinter = *pPrinterDC;
	 else
		hDCPrinter = hPr;
	 SmallFontLargeFontFactor=1;
     if (DoPrint)
     
     {	DOCINFO	DI;
     
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        GetGlobalCVal ("[%PRINTNAME]",LeafName,0);
        if (!*LeafName)
			GSSisplitpath (CfgName,0,0,LeafName,0);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        if (PrintPrompt)
        {
	        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
	        hWnd2 = ghWnd;
	        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", hWnd2,
	                                         lpfnPrintDlgProc);
	        ShowWindow (GetDlgItem(ghPrintingDlg,IDC_STATUS),FALSE);
		    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
	        SetAbortProc(hPr,lpfnAbortProc);
		    if (IsVirtPrinter)
		    {   
		    	LPSTR pDot;
		    	short	n=1,st=1;
				RECT	Rect;
		    	
				Rect.left = Rect.top = 0;
				Rect.right = VirtualPlotWidth;
				Rect.bottom = VirtualPlotHeight;
		    	DoPrint = TRUE; 
	   			//GSSiGetTempFileName (0,"gmp",0,VirtPlotDir); 
	   			//if ((pDot = _fstrrchr (VirtPlotDir,'.')))
	   			//	*pDot = 0;
	   			GetTempDir (VirtPlotDir);
	   			_fstrcat (VirtPlotDir,"\\GeoMaster Virtual Plots");
	   			GSSiMakeDir (VirtPlotDir,&Err); 
	   			GSSisplitpath (CfgName,0,0,LeafName,0);  
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
				if (NumVirtualRows > 1 || NumVirtualPages > 1)
				{
					sprintf (MetaFile,"%s\\preview.emf",VirtPlotDir);
					mfDC = lpPDChunk->hDC = CreateEnhMetaFile(hPr,MetaFile,&Rect,0);
					PrintingToMF = TRUE;
					DeleteDC (hPr);
					hPr = mfDC;
				}
		    }
			else if (PrinterDC) // useing bitmap
			{
				DoPrint = TRUE;
				UseBands = FALSE;
			}
	    	else
	    		DoPrint = TRUE;
		}
		else
			DoPrint = TRUE;
		if (DoPrint)
	    {  
		   HDC	SaveDC = hPr;

		   _getcwd (CurDir,MAX_PATH); 
  		   SaveDrive = _getdrive();

		   if (Page == 1)
		   		GSSiStartDoc(hPr,PrinterDC,IsVirtPrinter,&DI,lpPDChunk);
		   if (PrinterDC)
			  hPr = PrinterDC;
		   {
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);
	       int planes = GetDeviceCaps(hPr, PLANES);
	       int nc = GetDeviceCaps(hPr, NUMCOLORS);
	       
		   _chdir (CurDir);
		   _chdrive (SaveDrive);

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
	     //  		if (!*VirtPrinterImageFile)      
	     //  			hOverViewBitmap = CreateCompatibleBitmap(hPr,(int)OverViewWidth,(int)OverViewHeight); 
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
		   hPr = SaveDC;
		   PrinterResolution = dpi;
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1; 
	       PrinterRect = Rect;
           Printing = TRUE;
		   if (xPage < 0|| yPage < 0 )
		   {
	       		double fwidth =(double)((long)(100*(double)(xPage)/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)(yPage)/(double)dpi))/100; 

			    if (lpPDChunk->hDevNames)
				{
					IgnoreLock = TRUE;  
					{
	       				LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       				LPSTR	pName=(LPSTR)pdn+pdn->wDeviceOffset; 

						strcpy (PrinterName,pName);
						GlobalUnlock (lpPDChunk->hDevNames);
					}
			        IgnoreLock = FALSE;
				}
				else
					*PrinterName = 0;
	       		 
				_fstrcpy (Warning,"\r\nWARNING: This printer configuration exceeds the maximum image size and will not print");
			    sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f%s",PrinterName,dpi,fwidth,fheight,Warning);  
				MessageBox (GetFocus(),str,"Printer Parameters",MB_ICONEXCLAMATION);   
		        IgnoreLock = FALSE;
	            GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  
	            lpPDChunk->hDC = 0;
				DestroyWindow(ghPrintingDlg);
				ghPrintingDlg = NULL; 
	            rtn = FALSE;
				goto Exit2;
		   }
	       if (BufferedScreen)
		   {
			   ProcessText ("[%BUFFERSCREEN]=F");
			   SetBuf = TRUE;
		   }
	       SaveViewports (0);
	       SaveRect = Rect;  
	       {
			   if (PrinterDC)
					SetMainRect (0,PrinterDC,&Rect,3);
			   else
					SetMainRect (0,hPr,&Rect,3);
		       yPage = Rect.bottom;
//		       if (!ivp)
		       {
		       		double fwidth =(double)((long)(100*(double)(xPage)/(double)dpi))/100;
		       		double fheight=(double)((long)(100*(double)(yPage)/(double)dpi))/100; 

					if (lpPDChunk->hDevNames)
					{
						IgnoreLock = TRUE;
						{
		       				LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
		       				LPSTR	pName=(LPSTR)pdn+pdn->wDeviceOffset;  

							strcpy (PrinterName,pName);
							GlobalUnlock (lpPDChunk->hDevNames); 
						}
						IgnoreLock = FALSE;  
					}
	//	       		char	Tech[128];
		       		
	//	       		if (!Escape(hPr, GETTECHNOLOGY, 0, 0, Tech))
	//	       			*Tech = 0; 
					DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
					if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
						DoShrinkOrtho = FALSE;
					if (strstr (PrinterName,"PDF"))
						ForceBorder = TRUE;

		       		if (IsVirtPrinter)
		       		{
						DoShrinkOrtho = TRUE;
			        	sprintf (str,"Virtual Printer:%s\r\nOutput to:%s",CurVirtPrinter,VirtPrinterImageFile); 
			        	sprintf (PrinterSetup,"Virtual Printer:%s (Output to:%s)",CurVirtPrinter,VirtPrinterImageFile); 
			        }
		       		else
					{
						char	mode[10]="Vector";

						if (PrinterDC)
							strcpy (mode,"Bitmap");
			        	sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f  mode:%s",PrinterName,dpi,fwidth,fheight,mode);
			        	sprintf (PrinterSetup,"Printer:%s (dpi:%i  width:%.2f  height:%.2f)",PrinterName,dpi,fwidth,fheight);
					}
					SetGlobalValue ("%PRINTER",PrinterSetup); 

			        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
			        //not sure what this is for if (!*VirtualPrinterPathname)
			        {
				        PrinterWidth = fwidth;
				        PrinterHeight = fheight;
				    }
		       }
		   	   GSSiStartPage (hPr,PrinterDC,IsVirtPrinter); 
			   DisplayCycle++; 
			   if (UseBands)
			   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)0, &BandRect);
			   else
	           		bRc = 1;
	           Band=0;
	       	   ivp=0;
	           while (!gbUserAbort && ivp < 1/*NumVirtualPages*/ && bRc > 0 && (!UseBands || !IsRectEmpty(&BandRect)))
	           {   
			       Rect = SaveRect; 
			   /*    if (IsVirtPrinter)
			       {
			       		AdjustMainRectVirtualPrinter (&Rect,ivp,&VirtualPageRow,&VirtualPageCol);
			       		BandRect.left = BandRect.top = 0;
			       		BandRect.bottom = VirtualPageHeight; 
			       		BandRect.right = VirtualPageWidth; 
			       }*/
				   if (PrinterDC)
						SetMainRect (0,PrinterDC,&Rect,3);
				   else
						SetMainRect (0,hPr,&Rect,3);
			       if (Printing && GetGlobalBVal2 ("[%SHOWPRINTABLEAREA]",FALSE))
			       		FillRect (hPr,&Rect,GetStockObject (LTGRAY_BRUSH));
				   if (!SetupViewports (0,hPr,0,Rect,Band))
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
						   char	Line1[128], Line3[128], str[MAX_PATH]; 
						   LPSTR pPar;

						   DisplayViewport (hWnd,hPr,FALSE);	   
						   sprintf (Line1,"Page %ld of %ld", Page,TotPage);
						   sprintf (Line3,"Band %i of %i - Viewport %i",Band,NumVirtualPages,CurView->ID); 
						   _fstrcpy (str,PltName);
						   ExpandText (str);
						   if ((pPar = strrchr(str, '(')))
							   *pPar = 0;
						   if (FileType (str))
						   {
							   if (pPar)
								   *pPar = '(';
							   PrintMessage2 (Line1,str,Line3);
							   if (OpenMap (hWnd, hPr))
							   {   
		   	       	   			   SetDisplayMode (CurView->hDC, GF_MAPMODE);
					
								   do
								   {
						   				sprintf (Line3,"Band %i of %i - Viewport %i - Segment %ld",Band,NumVirtualPages,CurView->ID,iseg++);
										PrintMessage2 (0,0,Line3);
								   }
								   while (DisplaySeg (&hPr,FALSE) && CheckPrintAbort (hPr) && CurView); 
								   if (gbUserAbort)
						       			HaltMapDisplay (FALSE,FALSE);
							   }
						   }
					     }
					     
				   }
		 NextBand: 
		       	   EndDisplayProcessing (TRUE);
			   	   ApplyVPShadows ();	
			   	   if (GetGlobalCVal ("[%PRINTPROMPT]",str,0))
				   		DisplayPromptText (hPr,str);
	   			   DisplayCycle++; 
		 		   if (UseBands)  
		 		   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)0, &BandRect); 
		 		   else if (!IsVirtPrinter)
				   		bRc = 0;
				 /*  else
				   {    
				   		short	ii=0;
				   		
				   		sprintf (OutFile,"%s\\%3.3i%3.3i%s",VirtPlotDir,VirtualPageRow,VirtualPageCol,VirtFmt);
				   		if (ii)SaveDCBitMap (hPr,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE,WindowColor, 
				   						  hOverViewBitmap,
						   			      NumVirtualRows-VirtualPageRow-1,VirtualPageCol,
						   			      OverViewWidth/VirtualPagesPerRow,OverViewHeight/NumVirtualRows,OverViewHeight);

				   		//SaveDCBitMap (hPr,OutFile,FIF_BMP,0);
				   }*/
			   }
		   	   if (GSSiEndPage (hPr,PrinterDC,IsVirtPrinter,mfDC) < 0)
				   MessageBox (0,"Insufficient memory for print - use smaller dpi",0,MB_ICONEXCLAMATION);
			   if (!gbUserAbort && GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
			   {   
					HIGHLIGHTDATA	HighlightData;

		            First = TRUE;
				    ClearHighlightList (FALSE); 
					OpenHighlightList(0,0);
					memset (&HighlightData,0,sizeof(HIGHLIGHTDATA));
					while (GetNextPrintReport (First,ReportName,&Refno,Prefix,UDI,InitCmd))
					{
				   		First = FALSE;  
						SetUDIValue(Prefix, UDI);
						strncpy0(HighlightData.PD.Prefix, Prefix, 32);
						strncpy0 (HighlightData.PD.UDI,UDI,64);
						HighlightData.PD.Refno = Refno;
						BT_PUT (hHighlight,(LPSTR)&Refno,(LPSTR)&HighlightData); 
					}
					CloseHighlightList ();
				    GetGlobalCVal ("[%SUMMARYREPORT]",ReportName,"[%DL]macros\\summary.txt");
			   		PrintReport2 (hPr,PrinterDC,IsVirtPrinter,mfDC,ReportName,0,0,0,0);
				    ClearHighlightList (FALSE); 
	           }
	           First = TRUE;
			   while (!gbUserAbort && GetNextPrintReport (First,ReportName,&Refno,Prefix,UDI,InitCmd))
			   {    
			   		LPSHORT	ID;
			   		int		i, NumIDs;
			   		LPLONG	CNum; 
			   		char	txt[64];
					LPVIEWPORT	SaveView;        
					HANDLE	hView;  
			   		
			   		First = FALSE;  
					SetUDIValue(Prefix, UDI);
					if (GetGlobalLVal2("[%PRINTWPOPT]", FALSE) == 1)
			   		{
				   		PrintReport2 (hPr,PrinterDC,IsVirtPrinter,mfDC,ReportName,Refno,Prefix,UDI,InitCmd);      
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
				if (PrinterDC)
					GSSiDeleteDC(hPr,TRUE);  
				else
					GSSiDeleteDC(hPr,IsVirtPrinter);  
	            lpPDChunk->hDC = 0;
	            rtn = FALSE;
	       } 
		   else
		   {
	           if (LastPage)
	           {
		           GSSiEndDoc (hPr,PrinterDC,IsVirtPrinter); 
				   /*else
	   		            GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  

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
			           		FidPS = GSSiOpenFile (PPCmd,0,OF_CREATE);
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
		           }*/
		           lpPDChunk->hDC = 0;
		       }
			   
	       }
		   
			RestoreViewports ();
	    }
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
	if (SetBuf)
		ProcessText ("[%BUFFERSCREEN]=T");
	EnableWindow (hWndMain,TRUE);
	SetFocus (hWndMain);
	GSSiTrace ("End Printmap",0);    
	hDC = GetDC (hWndMain);
    SetMainRect (hWndMain,hDC,0,3);
	LeaveBlockingWindow(hSaveBM);
    ReleaseDC (hWndMain,hDC);  
    SmallFontLargeFontFactor = SaveSFLFF;    
    WindowColor = SaveColor;
    if (hWndAbortWaitMessage)
    	DestroyWindow (hWndAbortWaitMessage); 
    if (SetBuf || DoRedisplay)
		RedisplayWindow (); 
	InVirtualPrint = FALSE;  
	InPrintProcess = FALSE;
	ForceBorder = FALSE;
	PrintingToMF = FALSE;

    return (rtn);

}   

BOOL CheckPrintAbort (HDC hPr)
{   
	MSG	msg;
	
	if (gbUserAbort)// || !QueryAbort(hPr,0))
		return FALSE;
    while (GSSiPeekMessage(&msg, 0, 0, 0, TRUE)) 
    {
        if (!IsDialogMessage(ghPrintingDlg, &msg))
        {
#if ENABLETRACE
SetLastMessage(-1*(long)msg.message);
#endif
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		//if (!QueryAbort(hPr,0))
		//	return FALSE; 
	}
	return TRUE;
}

BOOL PrintScrollReport (HWND hWnd,BOOL useCurrentPrintSetup)
{	HDC hPr;
    RECT	Rect;
   short xPage, yPage;
   WORD wSize;
   BOOL bError;
//   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn=TRUE;
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
   static	BOOL IsVirtPrinter=FALSE;
   static	HANDLE hVirtPrinter=0;
   static HANDLE	hPDChunk=NULL;
	LPPRINTDLG	lpPDChunk;

	HDC		mfDC=0, PrinterDC=0;
	HDC		*pPrinterDC = &PrinterDC;
	static BOOL havePrintSetup = FALSE;


   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,0); 
    
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
	 if (!havePrintSetup && ForceOrient && !ShowVirtualPrintAreas)
	 { 
	 	DWORD	SaveFlags = lpPDChunk->Flags;
	    
	    lpPDChunk->Flags = PD_RETURNDEFAULT;
	    GSSiPrintDlg(lpPDChunk,0,0,0);
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
	 lpPDChunk->Flags = lpPDChunk->Flags|PD_RETURNDC;
	 if (GetGlobalLVal2("[%PRINTDIALOGOPT]",0))
		lpPDChunk->Flags = lpPDChunk->Flags|PD_ENABLESETUPTEMPLATE|PD_ENABLESETUPHOOK|PD_PRINTSETUP;
     if ((useCurrentPrintSetup && havePrintSetup) || GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter,pPrinterDC) != 0)
     
     {	DOCINFO	DI;
     
     	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    GetGlobalCVal ("[%PRINTNAME]",LeafName,0); 
	    if (!*LeafName)
			GSSisplitpath (CfgName,0,0,LeafName,0);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
		if (GSSiStartDoc(hPr,PrinterDC,IsVirtPrinter,&DI,lpPDChunk)>0)
	    {   
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);  
		   int	nCopies = lpPDChunk->nCopies;
	       
		   if (PrinterDC)
		   {
				Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &nCopies, &nCopies);
		   }
		   if (IsVirtPrinter)
		   {
			   xPage = VirtualPageWidth;
			   yPage = VirtualPageHeight;
		   }
		   else
		   {
			   xPage = GetDeviceCaps(hPr, HORZRES);
			   yPage = GetDeviceCaps(hPr, VERTRES);
		   }
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1;
	       SaveRect = Rect;
	       SetMainRect (0,hPr,&Rect,3);
	       yPage = Rect.bottom;
	       {
	       		double fwidth =(double)((long)(100*(double)Rect.right/(double)dpi))/100;
	       		double fheight=(double)((long)(100*(double)Rect.bottom/(double)dpi))/100; 
				if (lpPDChunk->hDevNames)
				{
					IgnoreLock = TRUE;
					{
	       				LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       				LPSTR	pName=(LPSTR)pdn+pdn->wDeviceOffset;  

						strcpy (PrinterName,pName);
						GlobalUnlock (lpPDChunk->hDevNames); 
					}
					IgnoreLock = FALSE;  
				}
				else
					*PrinterName = 0;

				DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
				if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
					DoShrinkOrtho = FALSE;
		        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
		        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
		        PrinterWidth = fwidth;
		        PrinterHeight = fheight;
	       }
	       Rect = SaveRect;
	       SetMainRect (0,hPr,&Rect,3);
		   		
		   GetCurVal (ReportName,sizeof(ReportName),IDS_FILERPT);

	       PrintReport2 (hPr,PrinterDC,IsVirtPrinter,0,ReportName,CurView->ReportRefno,CurView->Prefix,CurView->UDI,0);
		   if (!useCurrentPrintSetup)
			   GSSiEndDoc (hPr,PrinterDC,IsVirtPrinter);
		   else
			   havePrintSetup = TRUE;
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

BOOL PrintTextFile (HWND hWnd,LPSTR File,int nTabs,LPINT TabsIn)
{	HDC hPr;
    RECT	Rect;
   short xPage, yPage,x=10, y=10;
   WORD wSize;
   BOOL bError;
//   FARPROC lpfnAbortProc, lpfnPrintDlgProc;
   HBRUSH	BkBrush;
   BOOL		rtn=TRUE;
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
   SIZE		txSize;  
   int		Tabs[32], Margin=120;   
   double	Factor; 
   HFONT	hFont, hFontBold, OldFont;
   char		Header[514]; 
   short	FontSize = 8, LineInc, BottomOfPage; 
   HWND		ghWnd;

   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,0); 
    
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
	    GSSiPrintDlg(lpPDChunk,0,0,0);
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

     if (GSSiPrintDlg(lpPDChunk,0,0,0) != 0)
     
     {	DOCINFO	DI;
     
     	hPr = lpPDChunk->hDC;
        gbUserAbort = FALSE;
        bError = FALSE;
        Printing = TRUE;
        lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
        ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
	    GetGlobalCVal ("[%PRINTNAME]",LeafName,0); 
	    if (!*LeafName)
			GSSisplitpath (CfgName,0,0,LeafName,0);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    if (StartDoc(hPr,&DI) > 0)
	    {   
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX); 
		   int	nCopies=lpPDChunk->nCopies;
	       
	       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &nCopies, &nCopies);
	       xPage = GetDeviceCaps(hPr, HORZRES);
	       yPage = GetDeviceCaps(hPr, VERTRES);
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1;
	       SaveRect = Rect;
	       SetMainRect (0,hPr,&Rect,3);
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
	       SetMainRect (0,hPr,&Rect,3); 
	       Factor = (double)(Rect.right - Rect.left-Margin)/(double)TabsIn[nTabs-1];   
	       BottomOfPage = Rect.bottom - Margin;
		   for (i=0;i<nTabs;i++)
		   		Tabs[i] = TabsIn[i] * Factor;
		   Tabs[nTabs-1] *= 2;
		   hFontBold = CreateFont((int)IDNINT(FontSize*Factor*1.4), 0, 0, 0, FW_BLACK,0, 0, 0, 0, 0, 0, 0, 0,"Arial Black");   
		   hFont = CreateFont((int)IDNINT(FontSize*Factor), 0, 0, 0, FW_THIN,0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
		   OldFont = SelectObject (hPr,hFontBold);
		   Fid = GSSiOpenFile (File,0,OF_READ);  
		   fgetstring (Header,512,Fid);
		   GetTextExtentPoint32 (hPr,Header,_fstrlen(Header),&txSize); 
		   LineInc = txSize.cy; 
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
				GetTextExtentPoint32 (hPr,str,_fstrlen(str),&txSize); 
				y+=txSize.cy; 
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
   BOOL		rtn=TRUE;
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
		GSSisplitpath (CfgName,0,0,LeafName,0);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName= LeafName;
	    DI.lpszOutput = NULL;
        
	    lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
        SetAbortProc(hPr,lpfnAbortProc);
	    if (StartDoc(hPr,&DI) > 0)
	    {   
	       int dpi = GetDeviceCaps(hPr, LOGPIXELSX); 
		   int	nCopies = lpPDChunk->nCopies;
	       
	       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &nCopies, &nCopies);
	       xPage = GetDeviceCaps(hPr, HORZRES);
	       yPage = GetDeviceCaps(hPr, VERTRES);
	       Rect.left = 0;
	       Rect.top = 0;
	       Rect.bottom = yPage-1;
	       Rect.right = xPage-1;
	       SetMainRect (0,hPr,&Rect,3); 
		   IgnoreLock = TRUE;
	       {
	       		double fwidth=(double)xPage/(double)dpi, fheight=(double)yPage/(double)dpi;
		       	char	str[256]; 

				if (lpPDChunk->hDevNames)
				{
	       			LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
	       			LPSTR	pName=(LPSTR)pdn+pdn->wDeviceOffset;

					strcpy (PrinterName,pName);
					GlobalUnlock (lpPDChunk->hDevNames); 
				}
				else
					*PrinterName = 0;
	       		
		        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
		        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str);
	       }
  		   IgnoreLock = FALSE;
           Escape(hPr, NEXTBAND, 0, (LPSTR)0, &BandRect);
           while (!IsRectEmpty(&BandRect))
           {    
           
	       		switch (option)
		        {
		       		case 1: 
		     PrintSingle:
		               //if (_fstrstr(Name,".BMP"))
						   DisplayBMFileInRect (hPr,Name,Rect,TRUE);
					   //else if (_fstrstr(Name,".PCX"))
					   //	   DisplayPCXFileInRect (hPr,Name,Rect,TRUE);
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
			    Escape(hPr, NEXTBAND, 0, (LPSTR)0, &BandRect); 
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
   OFSTRUCTGM	OFStruct;
   char		str2[32], SavePrintName[34], PrintDriver[64],PrintType[64],PrintPort[64];
   long		Refno;
   COLORREF	SaveColor = WindowColor;
   BOOL		UseBands = GetGlobalBVal2 ("[%USEBANDS]",TRUE);  
   short	ii;
   HWND		ghWnd;
	BOOL IsVirtPrinter=FALSE;
	HANDLE hVirtPrinter=0;
	HDC		mfDC=0, PrinterDC=0, SaveDC;
	HDC		*pPrinterDC = &PrinterDC;
	BOOL	UseSeparateDocuments=GetGlobalBVal2 ("[%SEPARATEDOCUMENTS]",FALSE);
	char	InitCmd[256];
       
   GetGlobalCVal ("[%PRINTNAME]",SavePrintName,0); 
   ghWnd = hWnd;
	if (!OpenConfig(0,0)) return(FALSE);
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
	GetPrintMergeRec (0,0);
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
	lpPDChunk->lpfnSetupHook = (LPOFNHOOKPROC)PrintSetupHook;   
	lpPDChunk->lpSetupTemplateName = "PRNSETUPDLGGM";
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
//	DoPrint = PrintDlg(lpPDChunk); 
	DoPrint = GSSiPrintDlg(lpPDChunk,&IsVirtPrinter,&hVirtPrinter,pPrinterDC);
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
	    
        SaveViewports (1);
		  
		fgetstring (str,1024,FidPM);
	    		  
		while (str[0]=='#' || *LastChr (str) == ';') 
		{                
		  	ExpandText (str);
		  	if (!fgetstring (str,1024,FidPM))
		  		goto Exit;
        }
        
        rtn = TRUE;
		GSSisplitpath (CfgName,0,0,LeafName,0);
	    DI.cbSize = sizeof(DOCINFO);
	    DI.lpszDocName = LeafName;
	    DI.lpszOutput = NULL;  
	    
		ProcessDelimTextHeader(str, 0, FidPM, &hDLT, 0, 0);
		GetPrintMergeRec (0,0);
		record = 0; 
 	    IgnoreLock = TRUE;
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
		IgnoreLock = FALSE;
		while (!gbUserAbort && GetPrintMergeRec (FidPM,str))
  	    {   
  	       if (FirstRecord)
			   ii=1;
  	       else if (UseSeparateDocuments)
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
		   AddReportToPrintList (0,0,0,0,0);
  	       GetGlobalCVal ("[%PRINTNAME]",DocName,0); 
  	       if (!*DocName)
		   {
		   		GSSisplitpath (CfgName,0,0,LeafName,0);
           		sprintf (DocName,"%s.%5.5i",LeafName,record); 
           }
		   StatusWindowUpdate2 (0, TotPage, Page++);
	       DI.lpszDocName = DocName;
	
		   SaveDC = hPr;
		   
		    if ((!FirstRecord && !UseSeparateDocuments) || GSSiStartDoc(hPr,PrinterDC,IsVirtPrinter,&DI,lpPDChunk) > 0)
		    {   
		       int dpi = GetDeviceCaps(hPr, LOGPIXELSX);
			   int	nCopies = lpPDChunk->nCopies;
		       
			   if (PrinterDC)
				  hPr = PrinterDC;
		       Escape(hPr, SETCOPYCOUNT, sizeof(int),(LPCSTR) &nCopies, &nCopies);
		       xPage = GetDeviceCaps(hPr, HORZRES);
		       yPage = GetDeviceCaps(hPr, VERTRES);
		       Rect.left = 0;
		       Rect.top = 0;
		       Rect.bottom = yPage-1;
		       Rect.right = xPage-1;

		       SaveRect = Rect;
		       SetMainRect (0,hPr,&Rect,3);
		       yPage = Rect.bottom;
		       IgnoreLock = TRUE;
		       {
		       		double fwidth =(double)((long)(100*(double)Rect.right/(double)dpi))/100;
		       		double fheight=(double)((long)(100*(double)Rect.bottom/(double)dpi))/100;   
					if (lpPDChunk->hDevNames)
					{
		       			LPDEVNAMES pdn=(LPDEVNAMES)GlobalLock (lpPDChunk->hDevNames);
		       			LPSTR	pName=(LPSTR)pdn+pdn->wDeviceOffset;

						strcpy (PrinterName,pName);
						GlobalUnlock (lpPDChunk->hDevNames); 
					}
					else
						*PrinterName = 0;
					/*DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
					if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
						DoShrinkOrtho = FALSE;
			        sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f",PrinterName,dpi,fwidth,fheight);
			        GlobalUnlock (lpPDChunk->hDevNames); 
			        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
			        PrinterWidth = fwidth;
			        PrinterHeight = fheight; 
			        PrinterFormatPixelWidth = Rect.right - Rect.left;*/
					DoShrinkOrtho = GetGlobalBVal2 ("[%SHRINKORTHOS]",FALSE);
					if (!_fstricmp (PrinterName,"Acrobat PDFWriter"))
						DoShrinkOrtho = FALSE;
					if (strstr (PrinterName,"PDF"))
						ForceBorder = TRUE;

		       		if (IsVirtPrinter)
		       		{
			        	sprintf (str,"Virtual Printer:%s\r\nOutput to:%s",CurVirtPrinter,VirtPrinterImageFile); 
			        	sprintf (PrinterSetup,"Virtual Printer:%s (Output to:%s)",CurVirtPrinter,VirtPrinterImageFile); 
			        }
		       		else
					{
						char	mode[10]="Vector";

						if (PrinterDC)
							strcpy (mode,"Bitmap");
			        	sprintf (str,"Printer:%s\r\ndpi:%i  width:%.2f  height:%.2f  mode:%s",PrinterName,dpi,fwidth,fheight,mode);
			        	sprintf (PrinterSetup,"Printer:%s (dpi:%i  width:%.2f  height:%.2f)",PrinterName,dpi,fwidth,fheight);
					}
					SetGlobalValue ("%PRINTER",PrinterSetup); 

			        SetDlgItemText (ghPrintingDlg,IDC_PRINTERINFO,str); 
			        //not sure what this is for if (!*VirtualPrinterPathname)
			        {
				        PrinterWidth = fwidth;
				        PrinterHeight = fheight;
				    }
		       }
		       IgnoreLock = FALSE;  
			   hPr = SaveDC;
		       Rect = SaveRect;
			   if (PrinterDC)
			   {
				    UseBands = FALSE;
					SetMainRect (0,PrinterDC,&Rect,3);
			   }
			   else
					SetMainRect (0,hPr,&Rect,3);
//		       SetMainRect (0,hPr,&Rect);

		   	   GSSiStartPage (hPr,PrinterDC,IsVirtPrinter); 
//		   	   StartPage (hPr); 
		   	   DisplayCycle++;
			   if (UseBands)
			   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)0, &BandRect);
			   else
	           		bRc = 1;
	           Band=0;
	           while (!gbUserAbort && bRc > 0 && (!UseBands || !IsRectEmpty(&BandRect)))
	           {
		
				   if (!SetupViewports (0,hPr,0,Rect,IsFirstRecord))
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
			          Bounds = GetGlobalBoundsVal ("[PrintMergeBounds]",0);
					  ZoomToRect(Bounds,FALSE);  
                   }
				   Display = TRUE; 
				   SetupViewports (0,hPr,0,Rect,1);
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
						   char	Line1[128], Line3[128], str[MAX_PATH]; 
							   
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
									PrintMessage2 (0,0,Line3);
						       }
						       while (DisplaySeg (&hPr,FALSE) && CheckPrintAbort (hPr) && CurView); 
						       if (gbUserAbort)
								   HaltMapDisplay(FALSE, FALSE);
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
				   if (GetGlobalCVal ("[%PRINTPROMPT]",str,0))
					   DisplayPromptText (hPr,str);
		
		 NextBand: 
		 		   if (UseBands)  
		 		   		bRc = Escape(hPr, NEXTBAND, 0, (LPSTR)0, &BandRect); 
		 		   else
				   		bRc = 0;
			   }
		   	   GSSiEndPage (hPr,PrinterDC,IsVirtPrinter,mfDC);
//			   if (!gbUserAbort && GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
			   {    
			   		if (ExistFile ("[%DL]macros\\summary.txt"))
			   		{
				   		_fstrcpy (ReportName,"[%DL]macros\\summary.txt");  
				   		PrintReport2 (hPr,0,0,0,ReportName,0,0,0,0); 
				   	}
	           }
	           First = TRUE;
			   while (!gbUserAbort && GetNextPrintReport (First,ReportName,&Refno,Prefix,UDI,InitCmd))
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
				   		PrintReport2 (hPr,0,0,0,ReportName,Refno,Prefix,UDI,InitCmd); 
				   	}
			   }   

		       if (UseSeparateDocuments)
				   GSSiEndDoc (hPr,PrinterDC,IsVirtPrinter); 
           	   //EndDoc (hPr);
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
 	       	FirstRecord = FALSE;
		}
		if (!UseSeparateDocuments)
			GSSiEndDoc (hPr,PrinterDC,IsVirtPrinter); 
		if (!gbUserAbort && GetGlobalLVal2 ("[%PRINTWPOPT]",FALSE) == 2)
		{    
			if (ExistFile ("[%DL]macros\\jobsum.txt"))
			{
		        _fstrcpy (DocName,"Summary");
			    if (StartDoc(hPr,&DI) > 0) 
			    {
					_fstrcpy (ReportName,"[%DL]macros\\jobsum.txt");  
			   		PrintReport2 (hPr,0,0,0,ReportName,0,0,0,0); 
	           	    EndDoc (hPr);
	           	}
		   	}
		}
Exit:
	   GSSiClose (FidPM);
	   GSSiGlobFree (&hDLT);
       GSSiDeleteDC(lpPDChunk->hDC,IsVirtPrinter);  
       lpPDChunk->hDC = 0;
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
	    SetMainRect (hWndMain,hDC,0,3);
	    ReleaseDC (hWndMain,hDC); 
	} 
    SmallFontLargeFontFactor = SaveSFLFF;
    WindowColor = SaveColor;
    GetGlobalCVal ("[%PRINTNAME]",SavePrintName,0); 
 	RedisplayWindow (); 
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

  



BOOL ClipMap (HWND hWnd, LPSTR Name,short SizeOpt,short FormatOpt) 
{   
	HDIB	hDib;
	DWORD	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
    HDC SaveDC = CurView->hDC;  
    BOOL	SaveMemMap = MemMap; 
    HDC		hPr; 
    RECT	Rect;
	LPVIEWPORT	lpSaveView, LastVP; 
	int		startW,	endW = 4096;
	int		startH;
	double	WtoHFactor;  
	char	str[256];    
	RECT	ShapeRect; 
	long	rtn; 
	HCURSOR	hcurSave;  
	HBITMAP	hbmpOld; 
	char	MapName[256];
	HBITMAP	hTempBM;
	HDIB32	hDib32;
	DPOINT BitmapPoint[2], WorldPoint[2];
	double	ScaleX, ScaleY; 
	BOOL	SaveBS = BufferedScreen;
	RECT	CRect;
	RECT	SaveConfigDisplayRect = ConfigDisplayRect;
	double	SaveDeviceToScreenFactor = DeviceToScreenFactorMemMap;
	BOOL	SaveSFS;
	double	SavePW=PrinterWidth, SavePH=PrinterHeight;
	double	SavePrinterMarginLeft=PrinterMarginLeft,SavePrinterMarginRight=PrinterMarginRight,SavePrinterMarginTop=PrinterMarginTop,SavePrinterMarginBottom=PrinterMarginBottom;
	double	sw = GetGlobalDVal2 ("[%SCREENWIDTH]",(double)GetDeviceCaps(CurView->hDC, HORZSIZE) * MFT/100);
    
    HaltMapDisplay (FALSE,TRUE);
   	BufferedScreen = FALSE; 
	ScreenBufferDC ((HWND)1,0); 
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
    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT)); 
	startW = min (endW,pow (128000000 / (WtoHFactor * 3),0.5)-1); 
ReTry:
	MemMapWidth = startW;  
	MemMapHeight = MemMapWidth * WtoHFactor;
//	hdcMemMap = LargeMemDC (CurView->hDC,&MemMapWidth,&MemMapHeight,&hbmpOld);    
//		goto temp;
	hdcMemMap = CreateCompatibleDC(CurView->hDC); 
	hMemBitmap = 0;
	MemMapWidth+=64;
	do
	{   
//		sprintf (str,"%i",MemMapWidth);
//		SetWindowText (hWnd,str);
		GSSiDeleteObject (&hMemBitmap);
		MemMapWidth-=64;
		MemMapHeight = MemMapWidth * WtoHFactor;
		hMemBitmap = CreateCompatibleBitmap (CurView->hDC,MemMapWidth,MemMapHeight);
	}
	while (!hMemBitmap && MemMapWidth > 0);
	
    if (!hMemBitmap)
    {   
    	GSSiMessageBox ("Unable to create memory bitmap",0,MB_ICONEXCLAMATION,0);
    	return FALSE;
    }
	hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
temp: 
	MemMap = TRUE;
	*MemMapName = 0;
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	DoPaint = TRUE;	
    SaveViewports (0);
	GetClientRect (hWndMain,&CRect);
	DeviceToScreenFactorMemMap = (double)MemMapWidth / (CRect.right - CRect.left);
	SaveSFS = pViewportsD[0]->ShowFullScreen;
	pViewportsD[0]->ShowFullScreen = 0;
	PrinterMarginLeft = 0;
	PrinterMarginRight = 0;
	PrinterMarginTop = 0;
	PrinterMarginBottom = 0;
	if (!PrinterHeight)
	{
		DeviceToScreenFactorMemMap = 1;
		PrinterWidth = sw;
		PrinterHeight = (sw * (CRect.bottom - CRect.top))/(CRect.right - CRect.left);
	}
	CreateStatusWind (hWndMain,-1,"Creating Export File");
	Printing = FALSE;
	PaintMap (CurView->hWnd,hdcMemMap,TRUE,0,0); 
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
	PrinterWidth = SavePW;
	PrinterHeight = SavePH;
	PrinterMarginLeft = SavePrinterMarginLeft;
	PrinterMarginRight = SavePrinterMarginRight;
	PrinterMarginTop = SavePrinterMarginTop;
	PrinterMarginBottom = SavePrinterMarginBottom;
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
			MessageBox (0,str,0,MB_ICONEXCLAMATION);
		}
	
	}
	else
    {
		if (!OpenClipboard (hWnd))
		{  
			MessageBeep (MB_ICONEXCLAMATION);
			MessageBox( GetFocus(), "ERROR: Cannot access the clipboard",0, MB_OK|MB_ICONEXCLAMATION);
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


//	rtn = DeleteLargeDC (hdcMemMap,hbmpOld);   
	SelectObject(hdcMemMap,hbmpOld); 
	GSSiDeleteObject (&hMemBitmap);
	DeleteDC (hdcMemMap);      
	GSSiDeleteObject (&hTempBM);
	hdcMemMap = 0;
	MemMap = SaveMemMap;
	MemMapWidth = SaveMMW;
	MemMapHeight = SaveMMH; 
	GSSiSetCursor (hcurSave);
	BufferedScreen = SaveBS; 
	ConfigDisplayRect = SaveConfigDisplayRect;
	pViewportsD[0]->ShowFullScreen = SaveSFS;
	DeviceToScreenFactorMemMap = SaveDeviceToScreenFactor;
	//RedisplayViewports (FALSE);
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


     
void SavePrintSetupData (HFILE Fid,short dpi)
{
 	short	Length=0, Version=5, id=OB_SAVEPRINTSETUP; 
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
	BigWrite (Fid,(HPSTR)&PrinterMarginLeft,8,1);
	BigWrite (Fid,(HPSTR)&PrinterMarginRight,8,1);
	BigWrite (Fid,(HPSTR)&PrinterMarginTop,8,1);
	BigWrite (Fid,(HPSTR)&PrinterMarginBottom,8,1);
    IgnoreLock = FALSE;
	lpPDChunk->hDevMode = hDevMode; 
	lpPDChunk->hDevNames = hDevNames;
	GlobalUnlock (hPDChunk);
	return;
}

void FreePrintDlg (void)
{
	LPPRINTDLG	lpPDChunk;

	if (hPDChunk)
	{   
		lpPDChunk = (LPPRINTDLG)GlobalLock (hPDChunk);
	    IgnoreLock = TRUE;
	    GSSiGlobFree(&lpPDChunk->hDevMode);
	    GSSiGlobFree(&lpPDChunk->hDevNames);
	    IgnoreLock = FALSE;
        GSSiGlobUlFree (&hPDChunk);
    }
	return;
}

BOOL ReadPrintSetupData (HFILE Fid)
{
    short ObjectID,Version;   
	USHORT	Length;
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
	FreePrintDlg ();
	if (!hPDChunk)
		hPDChunk= GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(PRINTDLG));
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
	if (Version > 4)
	{
		GSSilread (Fid,(HPSTR)&PrinterMarginLeft,8);
		GSSilread (Fid,(HPSTR)&PrinterMarginRight,8);
		GSSilread (Fid,(HPSTR)&PrinterMarginTop,8);
		GSSilread (Fid,(HPSTR)&PrinterMarginBottom,8);
	}
    KeepMemLength = KeepMemLengthSave;
    IgnoreLock = FALSE; 
    lpPDChunk->hDC = 0;   
   	lpPDChunk->hwndOwner = ghWnd;	
	lpPDChunk->hInstance = ghInst;  
	lpPDChunk->lpfnPrintHook = 0;
    lpPDChunk->lpfnSetupHook = 0;
    lpPDChunk->lpPrintTemplateName = 0;
    lpPDChunk->lpSetupTemplateName = 0;
	GlobalUnlock (hPDChunk);
    if (Version < 4)
		FreePrintDlg ();
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
		GSSiGetTempFileName(0,"gmb",0,pFile); 
	}
	else
		pFile = GlobalLock (hVirtualPrintFile); 
	_fstrcpy (PMDataFile,pFile);  
	SetGlobalValue ("%FILEPMDATA","Virtual Printer");
	*PMMacroFile = 0;
	SetGlobalValue ("%FILEPMMACRO",""); 
	Fid =	GSSiOpenFile (pFile,0,OF_CREATE); 
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

BOOL SplitImage (LPSTR InFile,LPSTR OutDir,LPSTR OutType,int nrows, int ncols)
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

BOOL FAR PASCAL VIRTUAL_PRINTER_CREATEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
 short  BRtn,Error,Choice;
 double	Width,Height; 
 HFILE	Fid;
 char	VirtualPrinterList[MAX_PATH];
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_ADDSTRING,0,(LPARAM)"200 dpi"); 
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_ADDSTRING,0,(LPARAM)"300 dpi"); 
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_ADDSTRING,0,(LPARAM)"600 dpi"); 
		SendDlgItemMessage (hWndDlg,IDC_VIRTPRINTRES,CB_SETCURSEL,(WPARAM)1,(LPARAM)0); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;  
            
            case IDOK: 
            {
                 Choice=(short)SendDlgItemMessage(hWndDlg,IDC_VIRTPRINTRES,CB_GETCURSEL,0,0);
                 VirtualPrintDPI = VPRes[Choice];
                 GetDlgItemText (hWndDlg,IDC_VIRTPRINTWIDTH,str,32);
            	 Width = atof (str);
                 GetDlgItemText (hWndDlg,IDC_VIRTPRINTHEIGHT,str,32);
            	 Height= atof (str);
        		 GetGlobalCVal ("[%VIRTUALPRINTERLIST]",VirtualPrinterList,"[%DL]virtualprinters.txt");   
        		 Fid = GSSiOpenFile (VirtualPrinterList,0,OF_READWRITE);
            	 if (Fid == HFILE_ERROR)
            	 	Fid = GSSiOpenFile (VirtualPrinterList,0,OF_CREATE);
				 if (Fid == HFILE_ERROR)
				 {
					 MessageBox (hWndDlg,"Unable to create virtual printer list",VirtualPrinterList,MB_ICONEXCLAMATION);
					 break;
				 }
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

