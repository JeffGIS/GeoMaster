#include "graphint.h"  

#include "gmextern.h"   
#include "dibapi.h"

static short	MaxORTHOBUFS;
static BOOL	WantPalleteOrthos=FALSE,LoadBMPShowMess;
static HANDLE	hGSPal=0;
static	short	FilterColorFrom=0, FilterColorTo=0;  
static	char	LoadBMPInFile[128], LoadBMPOutFile[128], LoadBMPExt[4]; 
static	DWORD	RastOpts[15]={SRCCOPY,SRCAND,SRCPAINT,SRCINVERT,SRCERASE,NOTSRCCOPY,NOTSRCERASE,MERGECOPY,
	           			  MERGEPAINT,PATCOPY,PATPAINT,PATINVERT,DSTINVERT,BLACKNESS,WHITENESS};
static	char	CmpImageInFile[128]="", CmpImageOutFile[128];
static	long	CmpImageQuality;


DWORD GetROpt (short iopt)
{
	return RastOpts[iopt];
}

BOOL OrthoInBuffer(LPSTR Name, long frame)
{	int i, MinOrthoID;
	long	MinUse;
	LPORTHO	MinOrtho, CurOrtho;
	
	if (!hOrthos)
		OpenOrthos();
    
    MinUse = LONG_MAX;
    CurOrtho = (LPORTHO)GlobalLock(hOrthos);
	for (OrthoID=0;OrthoID<UsedOrthoBufs;OrthoID++,CurOrtho++)
	{
		if (!_fstrcmp (CurOrtho->Name,Name) && CurOrtho->Frame == frame && CurOrtho->DeleteBM)
		{
			GlobalUnlock(hOrthos);
			return (TRUE);      
		}
		if (!*CurOrtho->Name) CurOrtho->LastUsed = LONG_MIN;
		if (CurOrtho->LastUsed<MinUse)
		{
			MinUse = CurOrtho->LastUsed;	
			MinOrtho = CurOrtho; 
			MinOrthoID = OrthoID;
		}
	}
	if (UsedOrthoBufs == MaxORTHOBUFS)
	{
		CurOrtho = MinOrtho;
		OrthoID = MinOrthoID; 
	}
	else
		UsedOrthoBufs++;
	if (*CurOrtho->Name)
	{
		if (CurOrtho->DeleteBM == 1)
			hDibFree (&CurOrtho->hDib);
		else if (CurOrtho->DeleteBM == 2) 
		{
			if (hDibIs32Bit (CurOrtho->hDib))
				DestroyDIB32(CurOrtho->hDib,FALSE);
			else
				DestroyDIB((HANDLE)CurOrtho->hDib);
		}
		CurOrtho->hDib = 0;  
	} 
	GlobalUnlock(hOrthos);
	return (FALSE);

}

void OpenOrthos ()
{	int i;
	char	txt[128];
    LPORTHO	CurOrtho;
    
	if (hOrthos) return; 
	MaxORTHOBUFS = GetGlobalLVal ("[%ORTHO_BUFFERS]"); 
	MaxORTHOBUFS = min (max (MaxORTHOBUFS,1),MAXORTHOBUFS);
	hOrthos = (HANDLE) GSSiGlobAlloc (1108,GHND,MaxORTHOBUFS*(long)sizeof(ORTHO));
	UsedOrthoBufs = 0;
	CurOrtho = (LPORTHO) GlobalLock (hOrthos);
	for (i=0;i<MaxORTHOBUFS;i++,CurOrtho++)
	{   
		CurOrtho->Frame = -1;
	} 
	GlobalUnlock(hOrthos); 
	return;
}

void CloseOrthos (void)
{   int	i;
    LPORTHO	CurOrtho;
    
	if (!hOrthos) return;
	CurOrtho = (LPORTHO)GlobalLock(hOrthos);
	for (i=0;i<UsedOrthoBufs;i++,CurOrtho++)
	{
		if (*CurOrtho->Name)
		{
			if (CurOrtho->DeleteBM == 1)
				hDibFree (&CurOrtho->hDib);
			else if (CurOrtho->DeleteBM == 2) 
			{
				if (hDibIs32Bit (CurOrtho->hDib))
					DestroyDIB32(CurOrtho->hDib,FALSE);
				else
					DestroyDIB((HANDLE)CurOrtho->hDib);
			}
		} 
	}
	GSSiGlobUlFree (&hOrthos);
	CurOrtho = NULL;
	return;
}
void DisplayOrthoPhoto ()
{	RECT Rect;
	DPOINT	WPoint;
	POINT	Point;
	double	ScaleDiff;  
	BOOL	DIBColorsArePalleteEntries=FALSE; 
	LPORTHO	CurOrtho;  
	static	long	debugframe=41203436;   
	short	ii;
    
    if (!hOrthos)
    	return;
    CurOrtho = (LPORTHO)GlobalLock (hOrthos) + OrthoID;
	WPoint.x = CurOrtho->Bounds.xmn;
	WPoint.y = CurOrtho->Bounds.ymn;
	Point = BasePtToWinPt (&WPoint);
	Rect.left = Point.x;
	Rect.bottom = Point.y;
	WPoint.x = CurOrtho->Bounds.xmx;
	WPoint.y = CurOrtho->Bounds.ymx;
	Point = BasePtToWinPt (&WPoint);
	Rect.right = Point.x;
	Rect.top = Point.y;
/*	ScaleDiff = fabs (CurOrtho->Res - CurView->BaseUnitsPerPixel);*/
	ScaleDiff = CurOrtho->Res - CurView->BaseUnitsPerPixel; 
	ShowOrthoRes (CurOrtho->Res);  
	if (CurOrtho->Frame == debugframe)
		ii=1;
/*	if (ScaleDiff < -1.0e-10)
		FillRect (CurView->hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
	else
*/	{
		if (!CurOrtho->hDib)
		{
			if (LoadBitMap (CurOrtho->Name,CurOrtho->Frame, &CurOrtho->hDib, &CurOrtho->DeleteBM,
						&CurOrtho->BitCount,&CurOrtho->Width, &CurOrtho->Height))
						
//			SaveDIB (CurOrtho->hDib,"c:\\test.bmp");
		    	CurOrtho->DIBColorsArePalleteEntries = ConvertDIBToPallete (&CurOrtho->hDib,CurOrtho->BitCount);
		}			
		if (CurOrtho->hDib)
		{
//            long nc = NumDIBColors (CurOrtho->hDib);  
			if (!CurOrtho->Res)
				CurOrtho->Res = 1;  
			if (hDibIs32Bit (CurOrtho->hDib))
				DisplayBMInVP32 (CurView->hDC, CurOrtho->hDib, CurOrtho->DIBColorsArePalleteEntries,CurOrtho);
			else
				DisplayBMInVP (CurView->hDC, (HANDLE)CurOrtho->hDib, CurOrtho->DIBColorsArePalleteEntries,CurOrtho);
		}
	}
//	RealizePalette (CurView->hDC);
	GlobalUnlock (hOrthos);
	if (SingleStepOrthos)
	{   
		short	key=WaitForKeystroke (TRUE);
		
		switch (key)
		{
			case 'G':
			case 'g':
			SingleStepOrthos = FALSE;
			break;
			
			default:
			break;
		}
	}
	return;
} 

void SetMemDCBackground (HDC hDC,COLORREF WindowColor)
{   
	UINT	x,y;
	
	SaveDC (hDC);
	SetPixel (hDC,x,y,WindowColor);
	RestoreDC (hDC,-1);
	return;
}

void AdjustDIBColorPallet (HDIB hDib)    
{    
	LPBITMAPINFO pBi=(LPBITMAPINFO)GlobalLock(hDib);
	UINT	i, Mid=GetGlobalLVal ("[%MIDCOLOR]"), NewColor;   
	double	Move=GetGlobalDVal ("[%MOVECOLOR]");
	double	Brighten = GetGlobalDVal ("[%BRIGHTEN]");
	
	 
	if (pBi->bmiHeader.biClrUsed == 256 && pBi->bmiHeader.biBitCount == 8)
	{ 
		if (Move)
		for (i=0;i<256;i++)
		{
			if (i < Mid)
			{
				NewColor = pBi->bmiColors[i].rgbBlue -
						   (((double)i / Mid) * Move) * pBi->bmiColors[i].rgbBlue;
			}
			else
			{
				NewColor = pBi->bmiColors[i].rgbBlue +
						   (((double)(255-i)/ (255 - Mid)) * Move) * pBi->bmiColors[i].rgbBlue;
			}
			pBi->bmiColors[i].rgbBlue = 
			pBi->bmiColors[i].rgbGreen =
			pBi->bmiColors[i].rgbRed = NewColor; 
		}
		
		if (Brighten)
		for (i=0;i<256;i++)
			pBi->bmiColors[i].rgbBlue = 
			pBi->bmiColors[i].rgbGreen =
			pBi->bmiColors[i].rgbRed = IDNINT (pBi->bmiColors[i].rgbBlue + 
									   (255 - pBi->bmiColors[i].rgbBlue) * Brighten);
	}
	GlobalUnlock (hDib);
}    

BOOL DisplayBMInVP (HDC hDC, HANDLE hDib, BOOL DIBColorsArePalleteEntries,LPORTHO CurOrtho)
{   BITMAPFILEHEADER bmfHead;
	LPBITMAPINFOHEADER	pDibInfo;
	LPSTR pImage;
	int		i, bmx, bmy, vpx, vpy;
	int	VPWidth, VPHeight, BMHeight, BMWidth, OutWidth, InWidth, OutHeight;
	int		vpyp, bmyp;
	int	vpheight, vpwidth;
	int  bmwidth, bmheight;
	HDC hdcMem;
	HBITMAP	hbmPrev, hNewBM;
	double	BMTopLeftWx, BMTopLeftWy, VPOrigBMx, VPOrigBMy, BMTopLeftVPx, BMTopLeftVPy;
	double	BMBotLeftWx, BMBotLeftWy, BMTopRightWx, BMTopRightWy;
	double	BMBotLeftVPx,BMBotLeftVPy,BMTopRightVPx,BMTopRightVPy;
	double	VPBotLeftBMx, VPBotLeftBMy, VPTopRightBMx, VPTopRightBMy;
    double	VPTopLeftBMx, VPTopLeftBMy;  
    UINT	ColorType=DIB_RGB_COLORS;
	HPALETTE hGSPal;
	RECT	bm, vp;   
	short	rop=0;
	double	ZFactor=1;

	AdjustDIBColorPallet (hDib);    
	pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (hDib);
	if (!pDibInfo)
		return FALSE;
    if (DIBColorsArePalleteEntries)
    {
    	ColorType = DIB_PAL_COLORS; 
    }
   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
    
    VPWidth = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left +1;
    VPHeight = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top+1;
	TRANS2 (0,CurOrtho->Height-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBase);
	TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBase);
	TRANS2 (BMBotLeftWx-CurOrtho->Res/2,BMBotLeftWy-CurOrtho->Res/2,
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToWin);
	TRANS2 (CurOrtho->Width-1,CurOrtho->Height-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+CurOrtho->Res/2,BMTopRightWy+CurOrtho->Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToWin);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-CurOrtho->Res/2,BMTopLeftWy+CurOrtho->Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToWin);
	if (BMBotLeftVPx < CurView->DrawRect.left)
	{
		bm.left = IDNINT (VPOrigBMx);
		vp.left = CurView->DrawRect.left;
	}
	else
	{
		bm.left = 0;
		vp.left = IDNINT (BMBotLeftVPx);
	}
	if (BMTopRightVPy < CurView->DrawRect.top)
	{
		bm.top = IDNINT (VPTopRightBMy);
		vp.top = CurView->DrawRect.top;
	}
	else
	{
		bm.top = CurOrtho->Height-1;
		vp.top = IDNINT (BMTopRightVPy);
	}

	if (BMTopRightVPx > CurView->DrawRect.right)
	{
		bm.right = IDNINT (VPTopRightBMx);
		vp.right = CurView->DrawRect.right;
	}
	else
	{
		bm.right = CurOrtho->Width-1;
		vp.right = IDNINT (BMTopRightVPx);
	}
	if (BMBotLeftVPy > CurView->DrawRect.bottom)
	{
		bm.bottom = IDNINT (VPOrigBMy);
		vp.bottom = CurView->DrawRect.bottom;
	}
	else
	{
		bm.bottom = 0;
		vp.bottom = IDNINT (BMBotLeftVPy);
	}
	vpx = vp.left;
	vpy = vp.top;
	bmx = bm.left;
	bmy = bm.top;
	vpwidth = vp.right - vp.left + 1;
	vpheight = vp.bottom - vp.top + 1;
	bmheight = bm.top - bm.bottom;// + 1;
	bmwidth = bm.right - bm.left;// + 1;
   	SetStretchBltMode(hDC, StretchMode);   
   	rop = GetGlobalLVal2 ("[%RASTEROPT]",0);  
   	OutWidth = vpwidth; 
   	OutHeight = vpheight;
   	InWidth = bmwidth;  
//   	if (GetGlobalBVal2 ("[%PR]",FALSE) && OutWidth < InWidth)
   	if (DoShrinkOrtho && Printing && OutWidth < InWidth)
   	{   
	    BITMAP  bmp; 
	    HDC		hDCMain = GetDC (hWndMain);
	    HDC		hdcMem = CreateCompatibleDC(hDC);
	    HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,(int)OutWidth,(int)OutHeight), hbmPrev;     
	    HDIB	hDIB;
        
        ReleaseDC (hWndMain,hDCMain);
//    	GetObject(hNewBM, sizeof(BITMAP), &bmp);
        hbmPrev = SelectObject(hdcMem, hNewBM);
	    
	   	SetStretchBltMode(hdcMem, StretchMode);   
	   	i=StretchDIBits (hdcMem,0,0,OutWidth,OutHeight,
	    				   bmx,bmy-bmheight,//9-15-2005+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				      (LPBITMAPINFO)pDibInfo,
	   				      ColorType,RastOpts[rop]);
/*	   	i=StretchDIBits (hDC,vpx,vpy,vpwidth,vpheight,
	    				   0,0,OutWidth,OutHeight,
	   				       pImage,
	   				      (LPBITMAPINFO)pDibInfo,
	   				      ColorType,RastOpts[rop]);*/
	    i=BitBlt(hDC, vpx,vpy,vpwidth,vpheight, hdcMem, 0, 0,SRCCOPY);
//	    hDIB = BitmapToDIB (hNewBM,NULL);  
		if (hbmPrev)
        	SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem);    
//	    SaveDIB (hDIB,"c:\\test.bmp");
        DeleteObject (hNewBM);        
    }
	else 	
	   	i=StretchDIBits32 (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight,//9-15-2005+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				       pDibInfo,
	   				       ColorType,RastOpts[rop],&ZFactor);
    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
    nOrthoBlocks++;
 	GlobalUnlock (hDib);
	return (TRUE);
}

BOOL DisplayBMInVP32 (HDC hDC, HDIB32 hDib, BOOL DIBColorsArePalleteEntries,LPORTHO CurOrtho)
{   BITMAPFILEHEADER bmfHead;
	LPBITMAPINFOHEADER	pDibInfo;
	LPSTR pImage;
	int		i, bmx, bmy, vpx, vpy;
	int	VPWidth, VPHeight, BMHeight, BMWidth, OutWidth, InWidth, OutHeight;
	int		vpyp, bmyp;
	int	vpheight, vpwidth;
	int  bmwidth, bmheight;
	HDC hdcMem;
	HBITMAP	hbmPrev, hNewBM;
	double	BMTopLeftWx, BMTopLeftWy, VPOrigBMx, VPOrigBMy, BMTopLeftVPx, BMTopLeftVPy;
	double	BMBotLeftWx, BMBotLeftWy, BMTopRightWx, BMTopRightWy;
	double	BMBotLeftVPx,BMBotLeftVPy,BMTopRightVPx,BMTopRightVPy;
	double	VPBotLeftBMx, VPBotLeftBMy, VPTopRightBMx, VPTopRightBMy;
    double	VPTopLeftBMx, VPTopLeftBMy;  
    UINT	ColorType=DIB_RGB_COLORS;
	HPALETTE hGSPal;
	RECT	bm, vp;   
	short	rop=0;
	double	ZFactor=1; 
	BOOL	AllowShrink=FALSE;

	//AdjustDIBColorPallet (hDib);    
	pDibInfo = (LPBITMAPINFOHEADER) GetDibHeader (hDib);
	if (!pDibInfo)
		return FALSE;
    if (DIBColorsArePalleteEntries)
    {
    	ColorType = DIB_PAL_COLORS; 
    }
   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
    
    VPWidth = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left +1;
    VPHeight = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top+1;
	TRANS2 (0,CurOrtho->Height-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBase);
	TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBase);
	TRANS2 (BMBotLeftWx-CurOrtho->Res/2,BMBotLeftWy-CurOrtho->Res/2,
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToWin);
	TRANS2 (CurOrtho->Width-1,CurOrtho->Height-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+CurOrtho->Res/2,BMTopRightWy+CurOrtho->Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToWin);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-CurOrtho->Res/2,BMTopLeftWy+CurOrtho->Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToWin);
	if (BMBotLeftVPx < CurView->DrawRect.left)
	{
		bm.left = IDNINT (VPOrigBMx);
		vp.left = CurView->DrawRect.left;
	}
	else
	{
		bm.left = 0;
		vp.left = IDNINT (BMBotLeftVPx);
	}
	if (BMTopRightVPy < CurView->DrawRect.top)
	{
		bm.top = IDNINT (VPTopRightBMy);
		vp.top = CurView->DrawRect.top;
	}
	else
	{
		bm.top = CurOrtho->Height-1;
		vp.top = IDNINT (BMTopRightVPy);
	}

	if (BMTopRightVPx > CurView->DrawRect.right)
	{
		bm.right = IDNINT (VPTopRightBMx);
		vp.right = CurView->DrawRect.right;
	}
	else
	{
		bm.right = CurOrtho->Width-1;
		vp.right = IDNINT (BMTopRightVPx);
	}
	if (BMBotLeftVPy > CurView->DrawRect.bottom)
	{
		bm.bottom = IDNINT (VPOrigBMy);
		vp.bottom = CurView->DrawRect.bottom;
	}
	else
	{
		bm.bottom = 0;
		vp.bottom = IDNINT (BMBotLeftVPy);
	}
	vpx = vp.left;
	vpy = vp.top;
	bmx = bm.left;
	bmy = bm.top;
	vpwidth = vp.right - vp.left + 1;
	vpheight = vp.bottom - vp.top + 1;
	bmheight = bm.top - bm.bottom;// + 1;
	bmwidth = bm.right - bm.left;// + 1;
   	SetStretchBltMode(hDC, StretchMode);   
   	rop = GetGlobalLVal2 ("[%RASTEROPT]",0);  
   	OutWidth = vpwidth; 
   	OutHeight = vpheight;
   	InWidth = bmwidth;  
//   	if (GetGlobalBVal2 ("[%PR]",FALSE) && OutWidth < InWidth)
   	if (AllowShrink && DoShrinkOrtho && Printing && OutWidth < InWidth)
   	{   
	    BITMAP  bmp; 
	    HDC		hDCMain = GetDC (hWndMain);
	    HDC		hdcMem = CreateCompatibleDC(hDC);
	    HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,(int)OutWidth,(int)OutHeight), hbmPrev;     
	    HDIB	hDIB;
        
        ReleaseDC (hWndMain,hDCMain);
//    	GetObject(hNewBM, sizeof(BITMAP), &bmp);
        hbmPrev = SelectObject(hdcMem, hNewBM);
	    
	   	SetStretchBltMode(hdcMem, StretchMode);   
	   	i=StretchDIBits (hdcMem,0,0,OutWidth,OutHeight,
	    				   bmx,bmy-bmheight,//+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				      (LPBITMAPINFO)pDibInfo,
	   				      ColorType,RastOpts[rop]);
/*	   	i=StretchDIBits (hDC,vpx,vpy,vpwidth,vpheight,
	    				   0,0,OutWidth,OutHeight,
	   				       pImage,
	   				      (LPBITMAPINFO)pDibInfo,
	   				      ColorType,RastOpts[rop]);*/
	    i=BitBlt(hDC, vpx,vpy,vpwidth,vpheight, hdcMem, 0, 0,SRCCOPY);
//	    hDIB = BitmapToDIB (hNewBM,NULL);  
		if (hbmPrev)
        	SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem);    
//	    SaveDIB (hDIB,"c:\\test.bmp");
        DeleteObject (hNewBM);        
    }
	else 	
/*	   	i=StretchDIBits32 (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				       pDibInfo,
	   				       ColorType,RastOpts[rop],&ZFactor);*/
	   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight,//+1,
	   				       bmwidth,bmheight,
	   				       hDib, ColorType,RastOpts[rop],1);
    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
    nOrthoBlocks++;
// 	GlobalUnlock (hDib);
	return (TRUE);
}


BOOL DisplayBMFileInVP32 (HDC hDC,LPSTR BMFile,double RotationAZ)
{
//    BITMAPFILEHEADER bmfHead;
//	BITMAPINFOHEADER	DibInfo;
//	LPBITMAPINFOHEADER	pDibInfo=&DibInfo;
//	LPSTR pImage;
	int		i, bmx, bmy, vpx, vpy,  OutWidth, InWidth, OutHeight, vpyp, bmyp;
	long	vpheight, vpwidth, bmwidth, bmheight;   
	long	BMHeight, BMWidth, VPWidth, VPHeight;
	HDC hdcMem;
	HBITMAP	hbmPrev, hNewBM;
	double	BMTopLeftWx, BMTopLeftWy, VPOrigBMx, VPOrigBMy, BMTopLeftVPx, BMTopLeftVPy;
	double	BMBotLeftWx, BMBotLeftWy, BMTopRightWx, BMTopRightWy;
	double	BMBotLeftVPx,BMBotLeftVPy,BMTopRightVPx,BMTopRightVPy;
	double	VPBotLeftBMx, VPBotLeftBMy, VPTopRightBMx, VPTopRightBMy;
    double	VPTopLeftBMx, VPTopLeftBMy;  
    UINT	ColorType=DIB_RGB_COLORS;
	HPALETTE hGSPal;
	RECT	bm, vp;   
	short	rop=0;   
	double	Res=1;
	HDIB32 hDib=LoadDIB32 (BMFile); 
	MNMXCORD	BitmapBounds,WBounds;
    
    if (!hDib)
    	return FALSE; 
    if (RotationAZ)
    {   
   		HDIB32	hDibRotated = GMRotateImageClassic (hDib,RotationAZ*DEGRAD);
    		
		DestroyDIB32 (hDib,FALSE); 
		hDib = hDibRotated;
    }
//	AdjustDIBColorPallet (hDib);    
	if (!GetImageBounds (BMFile,hDib,&BitmapBounds,&WBounds))
		return FALSE;

	BMWidth = IDNINT(BitmapBounds.xmx+1);
	BMHeight = IDNINT(BitmapBounds.ymx+1);
	{       
		double BMX[4],BMY[4],BASEX[4],BASEY[4]; 
		float	RSQMIN;
		
	    CloseTRANS2 (&hTranBMToBase);
	    CloseTRANS2 (&hTranBaseToBM); 
	    BMX[0]=BMX[1]=BitmapBounds.xmn;
	    BMX[2]=BMX[3]=BitmapBounds.xmx;
	    BMY[0]=BMY[3]=BitmapBounds.ymn;
	    BMY[1]=BMY[2]=BitmapBounds.ymx;
	    BASEX[0]=BASEX[1]=WBounds.xmn;
	    BASEX[2]=BASEX[3]=WBounds.xmx;
	    BASEY[0]=BASEY[3]=WBounds.ymn;
	    BASEY[1]=BASEY[2]=WBounds.ymx;
	    hTranBMToBase = STRAN2 (BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
    }

/*    if (DIBColorsArePalleteEntries)
    {
    	ColorType = DIB_PAL_COLORS; 
    } */
//   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
    
    VPWidth = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left ;
    VPHeight = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top;
	TRANS2 (0,BMHeight-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBase);
	TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBase);
	TRANS2 (BMBotLeftWx-Res/2,BMBotLeftWy-Res/2,
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToWin);
	TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+Res/2,BMTopRightWy+Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToWin);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-Res/2,BMTopLeftWy+Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToWin);
	if (BMBotLeftVPx < CurView->DrawRect.left)
	{
		bm.left = IDNINT (VPOrigBMx);
		vp.left = CurView->DrawRect.left;
	}
	else
	{
		bm.left = 0;
		vp.left = IDNINT (BMBotLeftVPx);
	}
	if (BMTopRightVPy < CurView->DrawRect.top)
	{
		bm.top = IDNINT (VPTopRightBMy);
		vp.top = CurView->DrawRect.top;
	}
	else
	{
		bm.top = BMHeight-1;
		vp.top = IDNINT (BMTopRightVPy);
	}

	if (BMTopRightVPx > CurView->DrawRect.right)
	{
		bm.right = IDNINT (VPTopRightBMx);
		vp.right = CurView->DrawRect.right;
	}
	else
	{
		bm.right = BMWidth-1;
		vp.right = IDNINT (BMTopRightVPx);
	}
	if (BMBotLeftVPy > CurView->DrawRect.bottom)
	{
		bm.bottom = IDNINT (VPOrigBMy);
		vp.bottom = CurView->DrawRect.bottom;
	}
	else
	{
		bm.bottom = 0;
		vp.bottom = IDNINT (BMBotLeftVPy);
	}
	vpx = vp.left;
	vpy = vp.top;
	bmx = bm.left;
	bmy = bm.top;
	vpwidth = vp.right - vp.left ;
	vpheight = vp.bottom - vp.top ;
	bmheight = bm.top - bm.bottom;// + 1;
	bmwidth = bm.right - bm.left;// + 1;
   	SetStretchBltMode(hDC, StretchMode);   
   	rop = GetGlobalLVal2 ("[%RASTEROPT]",0);  
   	OutWidth = vpwidth; 
   	OutHeight = vpheight;
   	InWidth = bmwidth;  
//   	if (GetGlobalBVal2 ("[%PR]",FALSE) && OutWidth < InWidth)
/*   	if (DoShrinkOrtho && Printing && OutWidth < InWidth)
   	{   
	    BITMAP  bmp; 
	    HDC		hDCMain = GetDC (hWndMain);
	    HDC		hdcMem = CreateCompatibleDC(hDC);
	    HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,OutWidth,OutHeight), hbmPrev;     
	    HDIB	hDIB;
        
        ReleaseDC (hWndMain,hDCMain);
//    	GetObject(hNewBM, sizeof(BITMAP), &bmp);
        hbmPrev = SelectObject(hdcMem, hNewBM);
	    
	   	SetStretchBltMode(hdcMem, StretchMode);   
	   	i=StretchDIBitsFromHandle (hdcMem,0,0,OutWidth,OutHeight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       hDib,1);
//	   				      ColorType,RastOpts[rop]);
//	    i=BitBlt(hDC, vpx,vpy,vpwidth,vpheight, hdcMem, 0, 0,SRCCOPY);
//	    hDIB = BitmapToDIB (hNewBM,NULL);  
		if (hbmPrev)
        	SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem);    
//	    SaveDIB (hDIB,"c:\\test.bmp");
        DeleteObject (hNewBM);        
    }
	else */	
	   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       hDib, ColorType,SRCCOPY,1);
//	   				      ColorType,RastOpts[rop]);
    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
    nOrthoBlocks++;
// 	GlobalUnlock (hDib);
	return (TRUE);
}

BOOL DisplaySIDInVP32 (LPVIEWPORT pVP,LPSTR File)
{   BITMAPFILEHEADER bmfHead;
	BITMAPINFOHEADER	DibInfo;
	LPBITMAPINFOHEADER	pDibInfo=&DibInfo;
//	LPSTR pImage;
	int		i, bmx, bmy, vpx, vpy,  OutWidth, InWidth, OutHeight, vpyp, bmyp;
	long	vpheight, vpwidth, bmwidth, bmheight;   
	long	BMHeight, BMWidth;//, VPWidth, VPHeight;
	HDC hdcMem;
	HBITMAP	hbmPrev, hNewBM;
	double	BMTopLeftWx, BMTopLeftWy, VPOrigBMx, VPOrigBMy, BMTopLeftVPx, BMTopLeftVPy;
	double	BMBotLeftWx, BMBotLeftWy, BMTopRightWx, BMTopRightWy;
	double	BMBotLeftVPx,BMBotLeftVPy,BMTopRightVPx,BMTopRightVPy;
	double	VPBotLeftBMx, VPBotLeftBMy, VPTopRightBMx, VPTopRightBMy;
    double	VPTopLeftBMx, VPTopLeftBMy;  
    UINT	ColorType=DIB_RGB_COLORS;
	HPALETTE hGSPal;
	RECT	bm, vp;   
	short	rop=0;   
	double	Res=1;  
	HDC	hDC = pVP->hDC;
	HDIB32 hDib=0;
	DWORD Width,Height,TWidth,THeight;
	DWORD ColorSpace,NumBands,DataType,IsLocked;
	double MinMag,MaxMag,ULX,ULY,XRes,YRes,XRot,YRot,Mag=1, SidMag;  
	double BMX[4],BMY[4],BASEX[4],BASEY[4], TileWidth,TileHeight; 
	float	RSQMIN;
	double x,y, ximage, yimage,BaseDistPerPixel;
	HDIB32 hImage;
	DWORD ImageHandle=0,st,NumMetaRecords;
	long	Factor=1;  
	short	irec,ii;  
	double	conversion, xFactor, xmx, ymn;
    char	GIFile[128], SaveMAFile[128];  
    LPSTR	pBS;   
    BOOL	rtn=FALSE;  
    short	itilerow,ntilerows,itilecol,ntilecols;
    HANDLE	hTranBMToBaseT=0,hTranBaseToBMT=0;    
    long	iw,ih,MaxSidWidth=1024,MaxSidHeight=1024;
    
	SaveDC (pVP->hDC);    
	_fstrcpy (SaveMAFile,MaskAreaFile);
    _fstrcpy (GIFile,File);
    _fstrlwr (GIFile);
    if ((pBS = _fstrrchr (GIFile,'\\')))
    {   
    	int		RegionType;
    	RECT	Rect; 
    	
    	_fstrcpy (pBS,"\\global.ini");
    	LoadGlobalInit (GIFile,FALSE); 
		LayerUnits = PRJ_UNITS[3];  
	    GSSiDeleteObject(&pVP->hRgn);
        pVP->hRgn = CreateVPRgn(FALSE,FALSE); 
        RegionType = GetRgnBox (pVP->hRgn,&Rect);
        SelectClipRgn (pVP->hDC,pVP->hRgn);
        GSSiDeleteObject(&pVP->hRgn); 
        if (RegionType == NULLREGION)
		{
			rtn = TRUE;
        	goto Exit;
        }
	}
    if (LayerUnits == 1 && PRJ_UNITS[1] == 2)
    	conversion = FTM;
    else if (LayerUnits == 2 && PRJ_UNITS[1] == 1)
    	conversion = MFT;
    else
    	conversion = 1;

	if (File)
		ImageHandle=MrSidOpen (File);
	else
		ImageHandle = MrSIDImageHandle;
	st = MrSidGetInfo (ImageHandle,&Width,&Height,&ColorSpace,&NumBands,&DataType,&MinMag,&MaxMag,
					   &IsLocked,&ULX,&ULY,&XRes,&YRes,&XRot,&YRot,&NumMetaRecords); 
	ULX *= conversion;
	ULY *= conversion;
	XRes *= conversion;
	YRes *= conversion;
/*	for (irec=0;irec<NumMetaRecords;irec++)
	{   
		char	Tag[128]; 
		short	datatype,numDims;
		LPDWORD	pDims; 
		LPVOID	pData;
		DWORD st = GMMrSidGetMetadataRecord(ImageHandle,
		                               irec,
		                               Tag,
		                               &datatype,
		                               &numDims,
		                               &pDims,
		                               &pData);
		if (st)
			ii=1;
	}*/
	BaseDistPerPixel = (pVP->NewBounds.xmx - pVP->NewBounds.xmn)/
	    				   		 ((long)pVP->DrawRect.right - (long)pVP->DrawRect.left);
	xFactor = XRes;   
	if (xFactor < BaseDistPerPixel)
	{
		while (xFactor < BaseDistPerPixel) 
		{
			Factor *=2;	
			xFactor *= 2;
		} 
		Factor /= 2;
		xFactor /= 2; 
	}
	Mag = Factor;
	{       
	    CloseTRANS2 (&hTranBMToBase);
	    CloseTRANS2 (&hTranBaseToBM); 
	    BMX[0]=0;
	    BMX[1]=0;                                                                           
	    BMX[2]=Width-1;
	    BMX[3]=Width-1;
	    BMY[0]=Height-1;
	    BMY[1]=0;
	    BMY[2]=0;
	    BMY[3]=Height-1;
	    BASEX[0]=ULX;
	    BASEX[1]=ULX;
	    BASEX[2]=ULX + XRes * (Width-1);
	    BASEX[3]=BASEX[2];
	    BASEY[0]=ULY + YRes * (Height-1);
	    BASEY[1]=ULY;
	    BASEY[2]=BASEY[1];
	    BASEY[3]=BASEY[0];
	    hTranBMToBase = STRAN2 (BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
    }
    ntilerows = ntilecols = 1; 
    iw = pVP->DrawRect.right - pVP->DrawRect.left;
    while (iw > MaxSidWidth) 
    {
    	iw -= MaxSidWidth;
    	ntilecols ++;
    }
    ih = pVP->DrawRect.bottom - pVP->DrawRect.top;
    while (ih > MaxSidHeight) 
    {
    	ih -= MaxSidHeight;
    	ntilerows ++;
    }
    TileWidth = (pVP->WBounds.xmx - pVP->WBounds.xmn) / ntilecols;
    TileHeight = (pVP->WBounds.ymx - pVP->WBounds.ymn) / ntilerows;
    for (itilerow=0;itilerow<ntilerows;itilerow++)
    {
    	for (itilecol=0;itilecol<ntilecols;itilecol++)
    	{   
    		MNMXCORD	TileBounds;
    		
    		TileBounds.xmn = pVP->WBounds.xmn + itilecol * TileWidth;
    		TileBounds.xmx = TileBounds.xmn + TileWidth;
    		TileBounds.ymn = pVP->WBounds.ymn + itilerow * TileHeight;
    		TileBounds.ymx = TileBounds.ymn + TileHeight;
			x = max (ULX,TileBounds.xmn);
			y = min (ULY,TileBounds.ymx);	  
			ymn = max (TileBounds.ymn,ULY + (Height-1) * YRes);  
			xmx = min (TileBounds.xmx,ULX + (Width-1) * XRes); 
			if (x >= xmx || y <= ymn)
				goto NextTile;
			TRANS2 (x,y,&ximage,&yimage,hTranBaseToBM);  
			ximage /= Mag;
			yimage /= Mag;
			TWidth = ((xmx - x)/XRes)/Mag +1;
			THeight = ((y - ymn)/-YRes)/Mag +1;
			SidMag=1/Mag;
			hDib = MrSidGetImage (ImageHandle,&ximage,&yimage,&TWidth,&THeight,&SidMag); 
		    if (!hDib)
		    	goto NextTile;
		//	AdjustDIBColorPallet (hDib);    
		    if (!GetBitmapInfoFromHandle (&DibInfo,hDib))  
		    {
		        DestroyDIB32 (hDib,FALSE); 
				goto NextTile; 
			}
		//    st = DisplayBMInRect32 (hDC,hDib, pVP->DrawRect,TRUE); 
		//    return TRUE;
			BMWidth = DibInfo.biWidth;   
			BMHeight = DibInfo.biHeight;  
			Res = Mag;
			{       
			    CloseTRANS2 (&hTranBMToBaseT);
			    CloseTRANS2 (&hTranBaseToBMT); 
			    BMX[0]=0;
			    BMX[1]=0;
			    BMX[2]=BMWidth-1;
			    BMX[3]=BMX[2];
			    BMY[0]=0;
			    BMY[1]=BMHeight-1;
			    BMY[2]=BMY[1];
			    BMY[3]=0;
			    BASEX[0]=x;//-(XRes)*Mag;
			    BASEX[1]=BASEX[0];
		//	    BASEX[2]=x+(BMWidth+1)*(XRes)*Mag;
			    BASEX[2]=x+(BMWidth)*(XRes)*Mag;
			    BASEX[3]=BASEX[2];
			    BASEY[1]=y;//+(YRes)*Mag;
		//	    BASEY[0]=y+(BMHeight+1)*(YRes)*Mag;
			    BASEY[0]=y+(BMHeight)*(YRes)*Mag;
			    BASEY[2]=BASEY[1];
			    BASEY[3]=BASEY[0];
			    hTranBMToBaseT = STRAN2 (BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
			    hTranBaseToBMT = STRAN2 (BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
		    }
		
		/*    if (DIBColorsArePalleteEntries)
		    {
		    	ColorType = DIB_PAL_COLORS; 
		    } */
		//   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
		    
		//    VPWidth = (long)pVP->DrawRect.right - (long)pVP->DrawRect.left ;
		//    VPHeight = (long)pVP->DrawRect.bottom - (long)pVP->DrawRect.top;
			TRANS2 (0,BMHeight-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBaseT);
			TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBaseT);
			TRANS2 (BMBotLeftWx,BMBotLeftWy,
					&BMBotLeftVPx,&BMBotLeftVPy,pVP->hTranBaseToWin);
			TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
					&BMTopRightWy,hTranBMToBaseT);
			TRANS2 (BMTopRightWx,BMTopRightWy,
					&BMTopRightVPx,&BMTopRightVPy,pVP->hTranBaseToWin);
			TRANS2 (pVP->NewBounds.xmn,pVP->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
				    hTranBaseToBMT);
			TRANS2 (pVP->NewBounds.xmx,pVP->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
				    hTranBaseToBMT);
			TRANS2 (BMTopLeftWx,BMTopLeftWy,
					&BMTopLeftVPx,&BMTopLeftVPy,pVP->hTranBaseToWin);
			if (BMBotLeftVPx < pVP->DrawRect.left)
			{
				bm.left = IDNINT (VPOrigBMx);
				vp.left = pVP->DrawRect.left;
			}
			else
			{
				bm.left = 0;
				vp.left = IDNINT (BMBotLeftVPx);
			}
			if (BMTopRightVPy < pVP->DrawRect.top)
			{
				bm.top = IDNINT (VPTopRightBMy);
				vp.top = pVP->DrawRect.top;
			}
			else
			{
				bm.top = BMHeight;
				vp.top = IDNINT (BMTopRightVPy);
			}
		
			if (BMTopRightVPx > pVP->DrawRect.right)
			{
				bm.right = IDNINT (VPTopRightBMx);
				vp.right = pVP->DrawRect.right;
			}
			else
			{
				bm.right = BMWidth;
				vp.right = IDNINT (BMTopRightVPx);
			}
			if (BMBotLeftVPy > pVP->DrawRect.bottom)
			{
				bm.bottom = IDNINT (VPOrigBMy);
				vp.bottom = pVP->DrawRect.bottom;
			}
			else
			{
				bm.bottom = 0;
				vp.bottom = IDNINT (BMBotLeftVPy);
			}
			vpx = vp.left;
			vpy = vp.top;
			bmx = bm.left;
			bmy = bm.top;
			vpwidth = vp.right - vp.left + 1;
			vpheight = vp.bottom - vp.top + 1;
			bmheight = bm.top - bm.bottom;// + 1;
			bmwidth = bm.right - bm.left;// + 1;
//		   	SetStretchBltMode(hDC, StretchMode);   
		   	rop = GetGlobalLVal2 ("[%RASTEROPT]",0);  
		   	OutWidth = vpwidth; 
		   	OutHeight = vpheight;
		   	InWidth = bmwidth;  
		//   	if (GetGlobalBVal2 ("[%PR]",FALSE) && OutWidth < InWidth) 
			{
				RECT	BMRect;
	            
	            BMRect.left=bmx;
	            BMRect.right = bmx + bmwidth;
	            BMRect.top = bmy - bmheight;
	            BMRect.bottom = bmy;
//			   	if (Printing || (DoShrinkOrtho && OutWidth < InWidth))
			   	{   
				    BITMAP  bmp; 
				    HDC		hDCMain = GetDC (hWndMain);
				    HDC		hdcMem = CreateCompatibleDC(hDC);
				    HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,OutWidth,OutHeight), hbmPrev;     
				    HDIB	hDIB;
				        
			        ReleaseDC (hWndMain,hDCMain);
			//    	GetObject(hNewBM, sizeof(BITMAP), &bmp);
			        hbmPrev = SelectObject(hdcMem, hNewBM);
					    
	/*			   	SetStretchBltMode(hdcMem, StretchMode);   
				   	i=StretchDIBitsFromHandle (hdcMem,0,0,OutWidth,OutHeight,
				    				   bmx,bmy-bmheight+1,
				   				       bmwidth,bmheight,
				   				       hDib,1);*/
				   	i=StretchDIBitsFromHandle (hdcMem,0,0,OutWidth,OutHeight,
				   						BMRect.left,BMRect.top,
				    				   //bmxz,bmyz-bmheightz+1, 
				    				   BMRect.right - BMRect.left,BMRect.bottom -BMRect.top,
				   				       //bmwidthz,bmheightz,
				   				       hDib,ColorType,RastOpts[rop]+(DWORD)0x01000000, 1);
			//	   				      ColorType,RastOpts[rop]);
				    i=BitBlt(hDC, vpx,vpy,(int)vpwidth,(int)vpheight, hdcMem, 0, 0,SRCCOPY);
			//	    hDIB = BitmapToDIB (hNewBM,NULL);  
					if (hbmPrev)
			        	SelectObject(hdcMem, hbmPrev);
				    DeleteDC(hdcMem);    
			//	    SaveDIB (hDIB,"c:\\test.bmp");
			        DeleteObject (hNewBM);        
			    }
/*				else 	  
				{
					   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,
										   vpwidth,vpheight,
					    				   bmxz,bmyz-bmheightz+1,
					   				       bmwidthz,bmheightz,
					   				       hDib,ColorType,RastOpts[rop]+(DWORD)0x01000000, 1);   //
				//	   				      ColorType,RastOpts[rop]); 
				}*/  
			}
		    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
		    nOrthoBlocks++;
		// 	GlobalUnlock (hDib);
		    DestroyDIB32 (hDib,FALSE); 
		    rtn = TRUE;  
NextTile:
		    CloseTRANS2 (&hTranBMToBaseT);
		    CloseTRANS2 (&hTranBaseToBMT); 
		    if (ntilerows > 1 && !CheckForContinue (FALSE))
		    	goto Exit;
		}
	}
Exit:
	if (File && ImageHandle)
		st = MrSidClose (ImageHandle);
    CloseTRANS2 (&hTranBMToBase);
    CloseTRANS2 (&hTranBaseToBM); 
	RestoreDC (pVP->hDC,-1);
	_fstrcpy (MaskAreaFile,SaveMAFile);
    return rtn;
}

BOOL OrthoFilterFunction (HWND hWnd, WORD Message, WORD wParam, LONG lParam, short Function)
{
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
		  FARPROC lpfnORTHOFILTERMsgProc;
		  
	      lpfnORTHOFILTERMsgProc = MakeProcInstance((FARPROC)ORTHOFILTERMsgProc, hInst);
	      CreateDialog(hInst, (LPSTR)"ORTHOFILTER", hWnd, lpfnORTHOFILTERMsgProc);
	    }
   		break;

    case WM_MOUSEMOVE:
		
    	if (!hWndOrthoFilter)
    		break;
		MousePoint = MAKEPOINT(lParam);
		Color = GetPixel (CurView->hDC,MousePoint.x,MousePoint.y); 
        CurColorIndex = GetRValue(Color);
		sprintf (str,"%ld %ld %ld",(long)GetRValue(Color),(long)GetGValue(Color),(long)GetBValue(Color)); 
		SetDlgItemText (hWndOrthoFilter,IDC_STATIC,str);  
		break;
		
	case WM_KEYDOWN:
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
#define	NUMENTRIES	256  
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
		GlobalFree (handle);  
	}
		break;               
		
    case WM_RBUTTONUP:
    {
#define	NUMENTRIES	256  
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
		GlobalFree (handle);  
	}
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}  

BOOL FAR PASCAL ORTHOFILTERMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	 
	static	LPVIEWPORT	OrthVP; 
	short	ii;
	
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         hWndOrthoFilter = hWndDlg; 
         OrthVP = CurView;
		 WantPalleteOrthos=TRUE;
         break; 
    
    case WM_DESTROY:
		 WantPalleteOrthos=FALSE;
         hWndOrthoFilter = 0; 
    	 return 0;
    	 
    case WM_CLOSE:
	     DestroyWindow(hWndDlg); 
	     break;
	      
    case WM_COMMAND: 
    	 SetCurView ( OrthVP);
         switch(wParam)
         {  
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
            	
				hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
            	
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

BOOL ConvertDIBToPallete (LPHDIB32 lphDib,short BitCount)
{   
	LPBITMAPINFOHEADER	pDibInfo;
	short	i;
	LPSHORT	pColors;
	
	if (!WantPalleteOrthos || BitCount != 8)
		return FALSE; 
	if (!hGSPal)
	{
		hGSPal = CreateGrayScalePallette ();  
		SelectPalette (CurView->hDC,hGSPal,FALSE); 
		RealizePalette (CurView->hDC);
	} 
	pDibInfo = (LPBITMAPINFOHEADER)GetDibHeader (*lphDib);
   	pColors =(LPSHORT)((LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER));     
    for (i=0;i<256;i++)
    	*pColors++=i;
    if (!hDibIs32Bit (*lphDib))
    	GlobalUnlock ((HANDLE)*lphDib);
	return TRUE;
}

HANDLE CreateGrayScalePallette (void)
{   
#define	NUMENTRIES	256  
	LPLOGPALETTE	plgpl;
	HPALETTE hpal;
	PALETTEENTRY ape[NUMENTRIES]; 
	UINT	i;
	BYTE	red, green, blue; 
	HANDLE	handle;

	handle = GSSiGlobAlloc (1111,GMEM_MOVEABLE,sizeof(LOGPALETTE) + NUMENTRIES * sizeof(PALETTEENTRY));
    
    plgpl = (LPLOGPALETTE)GlobalLock (handle);
	plgpl->palNumEntries = NUMENTRIES;
	plgpl->palVersion = 0x300;

	for (i = 0, red = 0, green = 0, blue = 0; i < NUMENTRIES;
	        i++, red++, green++, blue++) {
	    ape[i].peRed =
	        plgpl->palPalEntry[i].peRed = red;
	    ape[i].peGreen =
	        plgpl->palPalEntry[i].peGreen = green;
	    ape[i].peBlue =
	        plgpl->palPalEntry[i].peBlue = blue;
	    ape[i].peFlags =
	        plgpl->palPalEntry[i].peFlags = PC_RESERVED;
	}
	hpal = CreatePalette(plgpl); 
	GlobalUnlock (handle);
	GlobalFree (handle); 
	return hpal;

}  

int     WINAPI DlgDirList32 (HWND hWndDlg, LPSTR cType, int LBCNTL, int CurrentDirCNTL, UINT Flags)
{
//         DlgDirList 32 (hWndDlg,str,LMD_NEWFILES,LMD_DIR,DDL_DIRECTORY|DDL_DRIVES/*|DDL_EXCLUSIVE*/);     
	char	str[512], VolLabel[32];
	DWORD	hDir, Type; 
	LPSTR	pBS;
	short	i;
	
	SendDlgItemMessage (hWndDlg,LBCNTL,LB_RESETCONTENT,0,0);
	for (i=0;i<26;i++)
	{   
		short DriveType = GetDriveType (i);
		if (DriveType > 0)
		{   
//			if (GetVolumeLabel(i,VolLabel))
			{
				char	DriveID=(char)('a'+i);
				sprintf (str,"[-%c-]",DriveID);	
				SendDlgItemMessage (hWndDlg,LBCNTL,LB_ADDSTRING,0,(LPARAM)str);  
			}
		}
	}
/*	if (!GetDlgItemText (hWndDlg,CurrentDirCNTL,str,255))
		return 0; */
	GetDlgItemText (hWndDlg,CurrentDirCNTL,str,255);
   	if (*LastChr (str) == '\\')
   		*LastChr (str) = 0;
	if ((pBS = _fstrchr (cType,'\\')))
	{
		*pBS++ = 0;
		sprintf (_fstrchr(str,0),"\\%s",cType);
		SetDlgItemText (hWndDlg,CurrentDirCNTL,str);   
		cType = pBS;
	} 
	else if (*LastChr (str) == ':')
		_fstrcat (str,"\\");
/*	else
		_fstrcpy (str,cType);
		l =_fstrlen (str);
		if (l > 4 && !_fstricmp (&str[l-4],".gmc"))
		{   
			LPSTR	pBS;
			
			*&str[l-4] = 0; 
			pBS = _fstrrchr (str,'\\');
			if (pBS)
				*pBS = 0;
		}*/
	SetDlgItemText (hWndDlg,CurrentDirCNTL,str);
   	if (*LastChr (str) == '\\')
   		*LastChr (str) = 0;
	_fstrcat (str,"\\*.*");
	hDir = SearchDirectory32 (str,0,&Type);
	while (hDir)
	{   
		if (Type && *str != '.') 
		{
			char	str2[256];
			
			sprintf (str2,"[%s]",str);
			SendDlgItemMessage (hWndDlg,LBCNTL,LB_ADDSTRING,0,(LPARAM)str2); 
		}
		hDir = SearchDirectory32 (str,hDir,&Type);
	}
	GetDlgItemText (hWndDlg,CurrentDirCNTL,str,255); 
	if (*LastChr(str) != '\\')
		_fstrcat (str,"\\"); 
	_fstrcat (str,cType);
	hDir = SearchDirectory32 (str,0,&Type);
	while (hDir)
	{   
		if (!Type)
			SendDlgItemMessage (hWndDlg,LBCNTL,LB_ADDSTRING,0,(LPARAM)str);
		hDir = SearchDirectory32 (str,hDir,&Type);
	}
	return 0;
} 


BOOL FAR PASCAL LOADMDMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{   
    char    str[256], FromDir[256], ToDir[256], ErrMess[128];
    BOOL    isDir;
    short     nchar, item;
    long    Num;
    HANDLE  Handle, hlpFI=0;
    LPSHORT   lpItems;
    LPFILEINDEX lpFI;
    OFSTRUCT    OFStruct;
    char    Index[256];
    static  char    ctype[64];
    static  short     itype;
    HANDLE  hCheckBox;
    HCURSOR hcurSave;  
    short	ii, i, TileSize; 
    UINT	TileSizeControls[4]={IDC_TSIZE0,IDC_TSIZE1,IDC_TSIZE2,IDC_TSIZE3};
	static		HANDLE	hSaveBM=0;    
	static	BOOL	AutoSelect=TRUE, AutoFileList;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:   
    	 AutoFileList = FALSE;
         hSaveBM = EnterBlockingWindow (hWndDlg);
		 SendDlgItemMessage (hWndDlg,IDC_AUTOSELECT,BM_SETCHECK,AutoSelect,0L);
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");   
       	 GetGlobalCVal ("[%MAKEORTHOUNITS]",str,"METERS");
         if (*str == 'M' || *str == 'm')
	     	SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)(1),NULL);
	     else 
	     	SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)(0),NULL);
    	 GetGlobalCVal ("[%MAKEORTHOLEVELS]",str,NULL);
    	 SetDlgItemText (hWndDlg,IDC_ORTHLEVELS,str);
    	 GetGlobalCVal ("[%MAKEORTHOQUALITY]",str,NULL);
    	 SetDlgItemText (hWndDlg,IDC_IMAGE_QUALITY,str); 
    	 GetGlobalCVal ("[%MAKEORTHODESTDIR]",str,NULL);
    	 SetDlgItemText(hWndDlg,IDC_DESTDIR,str);
    	 TileSize = GetGlobalLVal2 ("[%MAKEORTHOTILESIZE]",4);
		 SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_SETCHECK,TRUE,0L);
		 if (GetGlobalBVal2 ("[%USEFREEIMAGE]",FALSE)) 
			 SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_SETCHECK,TRUE,0L);
		 for (i=0;i<4;i++)
         	SendDlgItemMessage (hWndDlg,TileSizeControls[i],BM_SETCHECK,FALSE,0L);
         SendDlgItemMessage (hWndDlg,TileSizeControls[TileSize-1],BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_TXT,BM_SETCHECK,TRUE,0L);
         if (*AutoExportName)   
		 {   
		 	 short	l=_fstrlen (AutoExportName);
		 	 
		 	 if (!_fstricmp (&AutoExportName[l-12],"filelist.txt"))
		 	 { 
		 	 	AutoFileList = TRUE;   
		 	 	itype = 1;
		 	 }
		 	 else
		 	 {
			 	 if (_fstricmp (&AutoExportName[l-3],"txt") &&
			 	 	 _fstricmp (&AutoExportName[l-3],"tfw") &&
			 	 	 _fstricmp (&AutoExportName[l-3],"bpw"))
			 	 {
				 	SendDlgItemMessage (hWndDlg,IDC_BITMAPCOORD,BM_SETCHECK,TRUE,0L);
				 	SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_SETCHECK,TRUE,0L);
				 }
		         PostMessage(hWndDlg, WM_COMMAND, LMD_TYPE_ORTHO, 0L);   
	         }
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
	         break;
	     }
	     else 
	     {
	         itype = 1;
	         _fstrcpy (ctype,"*.plt");
	         SendDlgItemMessage (hWndDlg,LMD_TYPE_MAP,BM_SETCHECK,TRUE,0L); 
	     }
	     _fstrcpy (str,"[%DL]");
	     ExpandText (str); 
	     if (*LastChr (str) == '\\')
	     	*LastChr (str) = 0; 
	     _fstrlwr (str);
	     SetDlgItemText (hWndDlg,LMD_DIR,str);
SetType: 
         _fstrcpy(str,ctype);

         DlgDirList32 (hWndDlg,str,LMD_NEWFILES,LMD_DIR,DDL_DIRECTORY|DDL_DRIVES/*|DDL_EXCLUSIVE*/);
SelectFiles:
         if (AutoSelect && (itype ==1 || itype == 2 || itype == 4|| itype == 5))
         {
         	item=0;
         	while (SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_GETTEXT,item,(LPARAM)str) != LB_ERR)
         	{   
         		_fstrlwr (str);
             	if (_fstrstr (str,&ctype[1]))
             	{
             		SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_SETSEL,TRUE,item);
             	} 
             	item++;
            }
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
		 switch(LOWORD (wParam))
#else
         switch(wParam)
#endif
           {
            case LMD_NEWFILES: /* List Box                                 */
              {
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(lParam))
#endif
                    {
                    // case LBN_DBLCLK:
                     case LBN_SELCHANGE:
                          isDir = DlgDirSelect(hWndDlg,str,LMD_NEWFILES);
                          if (isDir)
                             {nchar = _fstrlen (str);
                              if (!nchar) break;
                              if (str[nchar-1] == '\\')
                                  _fstrcat (str,ctype);
                              if (str[nchar-1] == ':') 
                              {
                              	  _fstrcat (str,"\\");
                           	      SetDlgItemText (hWndDlg,LMD_DIR,str);
                                  _fstrcpy (str,ctype); 
                              }
                              ii=DlgDirList32 (hWndDlg,str,
                                  LMD_NEWFILES,LMD_DIR,DDL_DIRECTORY|DDL_DRIVES);
                              goto SelectFiles;
                              }
                          else
                          	ii=0;
                          break;
                     }

              }
              break;
            
   	       
			case LMD_TYPE_DGN:
               	_fstrcpy (ctype,"*.dgn");
                goto SetType;
             break;   
			case LMD_TYPE_DGNCOM:
               	_fstrcpy (ctype,"*.com");
                goto SetType;
             break;   
			case LMD_TYPE_SHP:
               	_fstrcpy (ctype,"*.shp");
                goto SetType;
             break;   
			case LMD_TYPE_ORA:
               	_fstrcpy (ctype,"*.ora");
                goto SetType;
             break;   
			case LMD_TYPE_VIRTUALPLOT:
               	_fstrcpy (ctype,"*.tif");
               	itype = 4;
                goto SetType;
             break;  
            case LMD_TYPE_GEOTIFF:  
                 itype = 5;
               	_fstrcpy (ctype,"*.tif");   
               	goto S100;
			case LMD_TYPE_SID:
                 itype = 5;  
               	_fstrcpy (ctype,"*.sid"); 
               	 SetDlgItemText (hWndDlg,IDC_ORTHLEVELS,"");
       S100:
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHSTATUS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTITLE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEAVI),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_EXCMP),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEFREEIMAGE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELST),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTYPE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TXT),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TFW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BPW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BITMAPCOORD),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_GEOTIFF),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_HIDE);  
                goto SetType;
             break;   
			case IDC_TXT:
               	_fstrcpy (ctype,"*.txt");
                goto SetType;
             break;   
			case IDC_TFW:
               	_fstrcpy (ctype,"*.tfw");
                goto SetType;
              break;   
			case IDC_BPW:
               	_fstrcpy (ctype,"*.bpw");
                goto SetType;
              break;   
			case IDC_GEOTIFF:
               	_fstrcpy (ctype,"*.tif");
                goto SetType;
              break;   
            case IDC_BITMAPCOORD:
               	_fstrcpy (ctype,"*.*");
                goto SetType;
              break;
            case IDC_AUTOSELECT:
				AutoSelect = SendDlgItemMessage (hWndDlg,IDC_AUTOSELECT,BM_GETCHECK,0,0);
              	goto SetType; 
            case IDCANCEL:
                 ContinueProcessing=FALSE;   
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 break;
                 
            case IDC_EXIT:
                 GSSiEndDialog(hWndDlg, ContinueProcessing,hSaveBM); 
                 ContinueProcessing = TRUE;
                 break;
                 
            case LMD_TYPE_MAP:
                 itype = 1;
                 _fstrcpy (ctype,"*.plt"); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHSTATUS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTITLE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEAVI),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_EXCMP),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEFREEIMAGE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELST),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTYPE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TXT),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TFW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BPW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BITMAPCOORD),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_GEOTIFF),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_HIDE);
                 
                 goto SetType;
                 break;    
            
/*            case LMD_TYPE_USGSDOQ:
            	 itype = 3;
                 _fstrcpy (ctype,"*.*");
            	 goto ShowOrthoControls;*/

            case LMD_TYPE_ORTHO:
                 itype = 2;
                 _fstrcpy (ctype,"*.txt");   
				 SendDlgItemMessage (hWndDlg,IDC_TXT,BM_SETCHECK,TRUE,0L);
       ShowOrthoControls:
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHSTATUS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTITLE),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEAVI),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_EXCMP),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEFREEIMAGE),SW_SHOW);
				 for (i=0;i<4;i++)
                 	ShowWindow (GetDlgItem(hWndDlg,TileSizeControls[i]),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TSLABEL),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELST),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY_LABEL),SW_SHOW); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR_LABEL),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTYPE),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TXT),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TFW),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BPW),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BITMAPCOORD),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_GEOTIFF),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_SHOW);
                  
                 goto SetType;
                 break;

            case ID_UP: 
                {
                 char   Dir[128];
                 LPSTR  LastDir;
                 short    len;
                 
                 GetDlgItemText (hWndDlg,LMD_DIR,Dir,128); 
                 LastDir=_fstrrchr(Dir,'\\');
                 if (LastDir) 
                 {
                 	*(++LastDir) = '\0'; 
                 	len = _fstrlen(Dir);
                 	if (len > 3)
                 		*(--LastDir) = '\0';
                 } 
                 else
                 	*Dir = 0;
                 SetDlgItemText (hWndDlg,LMD_DIR,Dir);
                 goto SetType;
                } 
                 
            case IDC_FINDFILES: 
            {
            	char	TempName[144], CurDir[144];
            	HFILE	Fid; 
            	LONG	TotFiles=0; 
            	short	lCurDir;
            	
            	lCurDir = GetDlgItemText (hWndDlg,LMD_DIR,CurDir,144);   
				GSSiGetTempFileName(0,"gm",0,TempName);
				Fid =	GSSiOpenFile (TempName,NULL,OF_CREATE);
				SearchFilesInDir (CurDir,ctype,Fid,&TotFiles,ctype,1); 
				GSSillseek (Fid,0,0);
				SendDlgItemMessage(hWndDlg, LMD_NEWFILES, LB_RESETCONTENT, 0, 0);
				while (fgetstring (str,144,Fid))
				{   
					LPSTR	pFile=str;
					if (!_fstrnicmp (str,CurDir,lCurDir))
						pFile = &str[lCurDir+1];
					SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_ADDSTRING,0,(LPARAM)pFile);    					
				}
				GSSiClose (Fid); 
				goto SelectFiles;
			}	
            	 break;
            	      
                 
            case IDOK: 
            {
                char saveWT[144], cOrthRes[4], cOrthResList[32], Mess[144];
                short  SizeOpt; 
                BOOL	CurHeaderWritten;  
                HFILE   FidIndex, FidFileList;
                short     Version=2, NumLevs=1, CurLev=0;
                long    Signature=80251, FirstHeaderOffset, LastHeaderOffset, CurOffset=0;
                long	TotLen=0, CurLoc=1, FileListLen;
                LPSTR	pOrthRes, lpComma;
                MNMXCORD	FileBounds; 
                BOOL	UsesTimes, UseCPT; 
                char	SaveTFWUnits[32];
				BOOL SaveBMPCache = AllowBMPCaching;
				char	TempCoordFile[128]=""; 
				HFILE	FidAutoFileList = HFILE_ERROR;  
				BOOL	UseExCmp=FALSE;

                ContinueProcessing = TRUE;  
                AddBMPToCache (NULL,1);
                Processing = TRUE;
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
				UseCPT = SendDlgItemMessage (hWndDlg,IDC_USECPT,BM_GETCHECK,0,0);  
				UseExCmp = SendDlgItemMessage (hWndDlg,IDC_EXCMP,BM_GETCHECK,0,0);  
				 UsesTimes = SendDlgItemMessage (hWndDlg,IDC_USETIME,BM_GETCHECK,0,0);  
				 if (itype == 1)
				 	*cOrthResList = 0;
				 else
	                 GetDlgItemText (hWndDlg,IDC_ORTHLEVELS,cOrthResList,32);
                 Strip(cOrthResList,' ');
                 pOrthRes = cOrthResList; 
                 lpComma = _fstrchr (pOrthRes,',');
                 while (lpComma)
                 {
                 	NumLevs++;
                 	lpComma++;
                 	lpComma = _fstrchr (lpComma,',');
                 }
                 do
                 {
	                 lpComma = _fstrchr (pOrthRes,',');
	                 if (lpComma)
	                 	*lpComma++=0;
	                 else
	                 	lpComma = _fstrchr(pOrthRes,0);
	                 _fstrcpy (cOrthRes,pOrthRes);
	                 pOrthRes = lpComma;
	                 
	                 
/*	                 if (SendDlgItemMessage (hWndDlg,IDC_BCHECK,BM_GETCHECK,0,0))
	                 {  
	                 	char	IndexPath[128], str[128]; 
					    LPFILEINDEX lpIndex;
	                 	HANDLE	handle; 
	                 	double	maxdist;
	                 	
	                 	IgnoreBounds=TRUE;
						SendDlgItemMessage(hWndDlg, LMD_NEWFILES, LB_RESETCONTENT, 0, 0);
	                 	_fullpath (IndexPath,"index",128);
						handle = OpenMapIndex (IndexPath);
						if (!handle)
							return TRUE;
						lpIndex = GlobalLock (handle); 
			NextDumpFile:
					    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles-1)
					    {   
					        if (!(lpIndex=GetNextIndexHeader(&handle)))
					        	return TRUE;
					        goto NextDumpFile;
					    }
					    GetNextIndexEntry(lpIndex);
    					maxdist = max (lpIndex->CurrentEntry->Bounds.xmx - lpIndex->CurrentEntry->Bounds.xmn, 
    								   lpIndex->CurrentEntry->Bounds.ymx - lpIndex->CurrentEntry->Bounds.ymn);
    					sprintf (str,"%f %s",maxdist,lpIndex->CurrentEntry->Name);
						SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_ADDSTRING,0,(LPARAM)str);    					
   	                 	goto NextDumpFile;
	                 }*/
	                 if (*AutoExportName)   
	                 {  
	                 	char	FullName[128], Drive[6], Dir[128]; 
	                 	LPSTR	lc;
	                 	Num=1;
	                 	*FromDir = 0; 
	                 	if (*AutoMapIndexName)
	                 		_fstrcpy (ToDir,AutoMapIndexName);
	                 	else
	                 	{
		                 	_fullpath (FullName,AutoExportName,sizeof(FullName));
		                 	_splitpath (FullName,Drive,Dir,NULL,NULL);
		                 	sprintf (ToDir,"%s%s",Drive,Dir);  
		                 	lc = LastChr (ToDir);
		                 	if (*lc == '\\')
		                 		*lc = 0;
		                }
	                 	if (!GetDlgItemText (hWndDlg,IDC_DESTDIR,FullName,128))
							SetDlgItemText(hWndDlg,IDC_DESTDIR,ToDir);
		                if (AutoFileList)
		                {
		                 	FidAutoFileList = GSSiOpenFile (AutoExportName,NULL,OF_READ);   
		                 	Num = NumRowsInTxtFile (FidAutoFileList);
		                }
	                 }
	                 else
	                 {
		                 Num = SendDlgItemMessage(hWndDlg, LMD_NEWFILES, LB_GETSELCOUNT, 0, 0);
		                 GetDlgItemText(hWndDlg,LMD_DIR,FromDir,255);
		             }
	                 if (!Num)
	                 	goto Exit2; 
	                 GetGlobalCVal ("[%TFWUNITS]",SaveTFWUnits,NULL); 
	  			     if (SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,NULL,NULL)) 
	  			     {
        				PRJ_UNITS[3] = 2;
	                	SetGlobalValue ("%TFWUNITS","METERS"); 
	                 }
	  			     else   
	  			     {
	                	SetGlobalValue ("%TFWUNITS","FEET");
						PRJ_UNITS[3] = 1; 
					 }
	                 DisableHalt = TRUE; 
	                 Processing = TRUE; 
	                 DoPaint = FALSE;
	                 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	                 Handle = GSSiGlobAlloc (1112,GHND,Num*2);
	                 lpItems =(LPSHORT) GlobalLock (Handle);
	                 SendDlgItemMessage((HWND) hWndDlg, LMD_NEWFILES, LB_GETSELITEMS,(short) Num, (LPARAM) lpItems);
	                 if (!GetDlgItemText(hWndDlg,IDC_DESTDIR,ToDir,255))
	                 	_fstrcpy(ToDir,FromDir); 
	                 else 
	                 {
	                 	ExpandText (ToDir); 
	                 	makedirectories (ToDir,TRUE,FALSE);
	                 }
	                 if (GetDlgItemText(hWndDlg,IDC_IMAGE_QUALITY,str,sizeof(str)))  
	                 	UserDefinedImageQuality = atoi(str) * 100; 
	                 else
	                 	UserDefinedImageQuality = 7200; 
	                 if (*FromDir && *LastChr (FromDir) != '\\')
	                 	_fstrcat (FromDir,"\\"); 
	                 sprintf (Index,"%s\\index%s",ToDir,cOrthRes);
	                 
	                 FidIndex = GSSiOpenFile (Index,(LPOFSTRUCT) &OFStruct,OF_CREATE); 
				     if (FileErrMess (FidIndex,Index,&OFStruct,OF_CREATE))  
				     	goto Exit;
	                 hlpFI = GlobalAlloc(GHND,sizeof(FILEINDEX)+sizeof(FILEINDEXENTRY)); 
	                 lpFI = (LPFILEINDEX) GlobalLock(hlpFI);    
	                 lpFI->CurrentEntry=(FILEINDEXENTRY *) &lpFI->FirstIndex;
	                 lpFI->Type = min (5,itype + 3);   
	                 lpFI->UsesTimes = UsesTimes;     
	                 
				     DBoundsInit (&FileBounds);
				     BigWrite (FidIndex,(char *)&FileBounds,sizeof(MNMXCORD),-1);  
				     FirstHeaderOffset = LastHeaderOffset = GSSillseek (FidIndex,0,1);    

					 DBoundsInit (&lpFI->Bounds);   
					 lpFI->NumFiles=0;
					 lpFI->Length=0; 
					 BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1); 
					 CurHeaderWritten = TRUE;
	                 GetWindowText(hWndDlg,saveWT,144);
	
	                 SizeOpt = 0; 
	                 if (SendDlgItemMessage (hWndDlg,IDC_TSIZE1,BM_GETCHECK,0,0))
	                    SizeOpt = 1;
	                 if (SendDlgItemMessage (hWndDlg,IDC_TSIZE2,BM_GETCHECK,0,0))
	                    SizeOpt = 2;
	                 if (SendDlgItemMessage (hWndDlg,IDC_TSIZE3,BM_GETCHECK,0,0))
	                    SizeOpt = 3;
	                 
	                 for (item=0;item<Num;item++,lpItems++)
	                 {  
	                 	if (!ContinueProcessing)
	                 		break;  
	                 	if (AutoFileList)
	                 		fgetstring (str,128,FidAutoFileList);                               
	                 	else if (*AutoExportName)
	                 		_fstrcpy (str,AutoExportName);
	                 	else
	                 	{   
		                    _fstrcpy (str,FromDir); 
		                    SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_GETTEXT,
		                        *lpItems, (LPARAM) _fstrchr(str,0));  
		                } 
		                ExpandText (str);
	                    _fstrlwr (str);  
	                    GetLongPathName (str,256);
	                    if (_fstrstr (str,"filelist.txt"))
	                    {   
	                    	HANDLE	hDLT=0;
	                    	char	SkipList[130], SkipDir[130];
	                    	
                    	    GetGlobalCVal ("[%SKIPLIST]",SkipList,NULL);
                    	    GetGlobalCVal ("[%SKIPDIR]",SkipDir,NULL); 
	                    	FidFileList = GSSiOpenFile (str,&OFStruct,OF_READ);
	                    	if (FidFileList == HFILE_ERROR)
	                    		goto Exit;     
	                    	if (!TotLen)
	                    	{
	                    		FileListLen = GSSillseek (FidFileList,0,2);
	                    		TotLen = FileListLen * NumLevs;
	                    		GSSillseek (FidFileList,0,0);
	                    	} 
	                    	fgetstring (str,128,FidFileList);
							if (!ProcessDelimTextHeader(str,NULL,FidFileList,&hDLT))
								goto Exit;
	                    	
	                    	while (ContinueProcessing && fgetstring (str,128,FidFileList))
	                    	{   
	                    		char	leaf[34]="[FILENAME]";
							    GetDelimTextData(str,hDLT); 
							    _fstrcpy (str,"[FULLNAME]");
							    ExpandText (str);  
							    ExpandText (leaf);
							    if (*SkipList)
							    {
							    	HANDLE hDLTSkip=0;
							    	char	skipfile[130];
							    	HFILE	FidSkip=GSSiOpenFile (SkipList,&OFStruct,OF_READ);
							    	if (FidSkip != HFILE_ERROR)
							    	{   
							    		fgetstring (skipfile,128,FidSkip);
										ProcessDelimTextHeader(skipfile,NULL,FidSkip,&hDLTSkip);
										while (fgetstring (skipfile,128,FidSkip))
										{
											GetDelimTextData(skipfile,hDLTSkip); 
											_fstrcpy (skipfile,"[SKIPNAME]");
											ExpandText (skipfile);
											if (!_fstricmp (skipfile,leaf))
											{  
									    		GSSiClose (FidSkip); 
									    		GSSiGlobFree (&hDLTSkip);  
									    		if (!CurLev)
									    		{
										    		_fstrcpy (skipfile,"[%SKIPDIR]\\[SKIPNAME].bpw");
										    		ExpandText (skipfile);
										    		copyfile (skipfile,str,FALSE,0,0,0,0,0,0);
										    		_fstrcpy (skipfile,"[%SKIPDIR]\\[SKIPNAME].bmp");
										    		ExpandText (skipfile); 
										    		REPLAC (str,".BPW",".BMP",sizeof(str));
										    		copyfile (skipfile,str,FALSE,0,0,0,0,0,0); 
										    	}
												goto NextFile;
											}	
										}
							    		GSSiGlobFree (&hDLTSkip); 
							    		GSSiClose (FidSkip);
							    	}
							    }
	                    		sprintf (Mess,"File %s: Level %s",str,cOrthRes);
		                    	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,Mess);
			                    if (!AddMapToDir (hWndDlg,str,ToDir,lpFI,itype,  
			                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0), 
			                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_GETCHECK,0,0),
			                                  GetDlgItem(hWndDlg,IDC_ORTHSTATUS),FidIndex,SizeOpt,cOrthRes,UsesTimes,
			                                  &LastHeaderOffset,&CurHeaderWritten,&FileBounds,UseCPT,UseExCmp))
			                        goto Exit;  
			                    CurLoc = FileListLen * CurLev + GSSillseek (FidFileList,0,1);
			                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS_BAR), TotLen, CurLoc,0);
		                    	CurOffset = GSSillseek (FidIndex,0,1);
			                    if (!CurHeaderWritten && (itype > 1 || lpFI->NumFiles > 255))
			                    {   
			                    	lpFI->NextHeaderOffset = CurOffset;
			                    	GSSillseek (FidIndex,LastHeaderOffset,0);
									BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
									GSSillseek (FidIndex,CurOffset,0);
									DBoundsInit (&lpFI->Bounds);   
									lpFI->NumFiles=0;
									lpFI->Length=0; 
			                    	LastHeaderOffset = CurOffset;
									BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
									CurHeaderWritten = TRUE;
			                    }
			                    else
									CurHeaderWritten = FALSE;
	               NextFile:;
			                    
			                }
			                GSSiClose (FidFileList);  
   							GSSiGlobFree (&hDLT);
	                    }
	                    else
	                    {
                    		sprintf (Mess,"File %s: Level %s",str,cOrthRes);
	                    	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,Mess);
		                    SetWindowText(hWndDlg,Mess);
		                    if ((BOOL)SendDlgItemMessage (hWndDlg,IDC_BITMAPCOORD,BM_GETCHECK,0,0))
		                    {   
		                    	HFILE	FidOut;
		                    	
								GSSiGetTempFileName (0,"gm",0,TempCoordFile); 
						    	FidOut = GSSiOpenFile (TempCoordFile,&OFStruct,OF_CREATE);  
						    	fputstring ("METERS",FidOut);
						    	fputstring ("0.0",FidOut);
						    	fputstring ("0.0",FidOut);
						    	fputstring ("1.0",FidOut);
							    fputstring (str,FidOut);
								GSSiClose (FidOut);
								_fstrcpy (str,TempCoordFile);                    	
		                    } 
		                    if (!AddMapToDir (hWndDlg,str,ToDir,lpFI,itype,   
		                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0),
		                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_GETCHECK,0,0),
		                                  GetDlgItem(hWndDlg,IDC_ORTHSTATUS),FidIndex,SizeOpt,cOrthRes,UsesTimes,
		                                  &LastHeaderOffset,&CurHeaderWritten,&FileBounds,UseCPT,UseExCmp))
		                        goto Exit;
		                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS_BAR), Num*NumLevs, CurLoc++,0);
	                    	CurOffset = GSSillseek (FidIndex,0,1);
		                    if (!CurHeaderWritten && (itype > 1 || lpFI->NumFiles > 255))
		                    {   
		                    	lpFI->NextHeaderOffset = CurOffset;
		                    	GSSillseek (FidIndex,LastHeaderOffset,0);
								BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
								GSSillseek (FidIndex,CurOffset,0);
								DBoundsInit (&lpFI->Bounds);   
								lpFI->NumFiles=0;
								lpFI->Length=0; 
		                    	LastHeaderOffset = CurOffset;
								BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
								CurHeaderWritten = TRUE;
		                    }
		                    else
								CurHeaderWritten = FALSE;
		                } 
	                 }
	            Exit:
	            	 if (*TempCoordFile)
	            	 	GSSiRemove (TempCoordFile);
	            	 *TempCoordFile = 0;
	            	 GSSiGlobUlFree (&Handle);
	                 GSSiChangeLength (FidIndex,CurOffset);
	                 ii=GSSillseek (FidIndex,0,2);
	                 BigWrite(FidIndex,(HPSTR)&Signature,4,-1);
	                 BigWrite(FidIndex,(HPSTR)&Version,2,-1);
	                 GSSillseek (FidIndex,0,0);
				     BigWrite (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD),-1); 
				     if (!CurHeaderWritten)
				     { 
					     GSSillseek (FidIndex,LastHeaderOffset,0);
					     BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
					 }
				     GSSillseek (FidIndex,FirstHeaderOffset,0);
				     BigRead (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH);
				     lpFI->EndOffset = CurOffset;
				     GSSillseek (FidIndex,FirstHeaderOffset,0);
				     BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
	                 GSSiClose (FidIndex);  
	                 GSSiGlobUlFree (&hlpFI);
	                 /*CloseMapIndex (Index,hlpFI,TRUE);*/
	                 if (SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0))
	                 {
	                    AVIOutClose(&hAVIFile);
	                 }
	                 DisableHalt = FALSE; 
	                 DoPaint = TRUE; 
	                 SetWindowText(hWndDlg,saveWT);  
	                 CurLev++;
	             } while (*pOrthRes && ContinueProcessing); 
	             if (itype == 1 && ContinueProcessing)
	             {  
	             	long	lmem=sizeof(VIEWPORT)+128*MAX_VIEWPORT_FILES; 
	             	HANDLE	hVP=GSSiGlobAlloc (1113,GHND,lmem);
	             	LPVIEWPORT	pSaveVP = (LPVIEWPORT)GlobalLock (hVP);  
	             	HANDLE		hSymDesc;
	             	short		NumParent=0, NumSyms=0;
	             	HFILE		FidSyms;
				    LPSYMDESC	pSymDesc;
				    LPSYMBOL    pSymbol;  
				    LPSTR		lpTAB;   
				    VISLIST		SaveVis;
				    char		VisName[128];
	             	
				    SetCurView (pViewports[*pCommandViewport-1]);
	             	_fmemmove (pSaveVP,CurView,(size_t)lmem);  
	             	CurView->NumFiles = 1;
					CurView->FileType[0]=4;
	             	_fstrcpy (CurView->lpFiles[0],ToDir);
	             	_fstrcat (CurView->lpFiles[0],"\\symlist"); 
	             	GSSiRemove (CurView->lpFiles[0]);  
	             	_fstrcpy (VisName,CurView->lpFiles[0]);
	             	_fstrcpy (CurView->lpFiles[0],ToDir);
	             	_fstrcat (CurView->lpFiles[0],"\\index"); 
	             	if (!CurVis)
	             		SelectVisList (FALSE);
					SaveVis = *CurVis; 
					IgnoreBounds = TRUE;
					InitVis ();   
				 	GetVisList (hWndDlg,SYM_VIS_LB, PAR_VIS_LB,0,1,HFILE_ERROR); 
				 	*CurVis = SaveVis; 
				 	IgnoreBounds = FALSE; 
	             	FidSyms = GSSiOpenFile (VisName,&OFStruct,OF_CREATE);
				 	hSymDesc = GSSiGlobAlloc (1114,GMEM_MOVEABLE,UINT_MAX);
				    pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
				 	while (SendDlgItemMessage (hWndDlg,SYM_VIS_LB,LB_GETTEXT,NumSyms++,(LPARAM)str)!=LB_ERR) 
				 	{   
				 		pSymDesc->Handle = GSSiGlobAlloc (1115,GHND,sizeof(SYMBOL));
				        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle); 
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymDesc->Number = atoi (lpTAB);
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymbol->Parent = atoi (lpTAB);
			            pSymbol->Type = 1;
				        lpTAB = _fstrchr (str,'\t');
				        *lpTAB = 0;
			            _fstrcpy (pSymbol->Name,str);
				        GlobalUnlock (pSymDesc++->Handle); 
				    } 
				    NumSyms--;
				 	while (SendDlgItemMessage (hWndDlg,PAR_VIS_LB,LB_GETTEXT,NumParent++,(LPARAM)str)!=LB_ERR) 
				 	{   
				 		NumSyms++;
				 		pSymDesc->Handle = GSSiGlobAlloc (1116,GHND,sizeof(SYMBOL));
				        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle); 
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymDesc->Number = atoi (lpTAB);
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymbol->Parent = atoi (lpTAB);
			            pSymbol->Type = 0;
				        lpTAB = _fstrchr (str,'\t');
				        *lpTAB = 0;
			            _fstrcpy (pSymbol->Name,str);
				        GlobalUnlock (pSymDesc++->Handle); 
				    }  
				    NumParent--;
					GlobalUnlock (hSymDesc); 
					WriteSymList (FidSyms, NumParent, NumSyms,hSymDesc);
	                DestroySymList (&NumSyms,&hSymDesc);
	             	_fmemmove (CurView,pSaveVP,(size_t)lmem);
				    GlobalUnlock (hVP);
				    GlobalFree (hVP); 
				    GSSiClose (FidSyms);
	             }
	      Exit2:
                 GSSiSetCursor (hcurSave);
                 Processing = FALSE;
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);  
                 if (FidAutoFileList != HFILE_ERROR)
                 	GSSiClose (FidAutoFileList);
                 if (*AutoExportName)
                 	GSSiEndDialog(hWndDlg, ContinueProcessing,hSaveBM); 
                 else if (ContinueProcessing)
                  	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,"Index created"); 
                 else
                  	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,"Index creation cancelled");
                 ContinueProcessing = TRUE;  
                 SetGlobalValue ("%TFWUNITS",SaveTFWUnits);
				 AllowBMPCaching = SaveBMPCache;
                 break;      
            }

           }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL CreateMapIndex (LPSTR Dir,LPSTR FileListName,short itype,BOOL UsesTimes,LPSTR ImageQuality,LPSTR cOrthResList,BOOL Compress,short  SizeOpt,
					 HWND hWndDlg,UINT MESSCntl, UINT STATUSCntl,BOOL UseCPT)
{
	char Mess[144], ToDir[128],str[260],File[132];
    HANDLE  Handle, hlpFI=0;
	BOOL	CurHeaderWritten;  
	HFILE   FidIndex, FidFileList, Fid;
    LPFILEINDEX lpFI;
    OFSTRUCT    OFStruct;
    char    Index[256], cOrthRes[4];
	short     Version=2, NumLevs=1, CurLev=0;
	long    Signature=80251, FirstHeaderOffset, LastHeaderOffset, CurOffset=0;
	long	TotLen=0, CurLoc=0, FileListLen;
	LPSTR	pOrthRes, lpComma;
	MNMXCORD	FileBounds; 
	BOOL	UseExCmp=FALSE;
	                
	Fid = GSSiOpenFile (FileListName,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;  
	TotLen = GSSillseek (Fid,0,2);
	if (itype == 1)
		*cOrthResList = 0;
	Strip(cOrthResList,' ');
	pOrthRes = cOrthResList; 
	lpComma = _fstrchr (pOrthRes,',');
	while (lpComma)
	{
		NumLevs++;
		lpComma++;
		lpComma = _fstrchr (lpComma,',');
	}
	do
	{
		lpComma = _fstrchr (pOrthRes,',');
		if (lpComma)
			*lpComma++=0;
		else
			lpComma = _fstrchr(pOrthRes,0);
		_fstrcpy (cOrthRes,pOrthRes);
		pOrthRes = lpComma;
		DisableHalt = TRUE; 
		Processing = TRUE; 
		DoPaint = FALSE;
		_fstrcpy (ToDir,Dir);
		ExpandText (ToDir); 
	 	makedirectories (ToDir,TRUE,FALSE);
		if (*ImageQuality)  
		 	UserDefinedImageQuality = atoi(ImageQuality) * 100; 
		else
			UserDefinedImageQuality = 7200; 
		sprintf (Index,"%s\\index%s",ToDir,cOrthRes);
			                 
		FidIndex = GSSiOpenFile (Index,(LPOFSTRUCT) &OFStruct,OF_CREATE); 
			
		hlpFI = GlobalAlloc(GHND,sizeof(FILEINDEX)+sizeof(FILEINDEXENTRY)); 
		lpFI = (LPFILEINDEX) GlobalLock(hlpFI);    
		lpFI->CurrentEntry=(FILEINDEXENTRY *) &lpFI->FirstIndex;
		lpFI->Type = min (5,itype + 3);   
		lpFI->UsesTimes = UsesTimes;     
				                 
		DBoundsInit (&FileBounds);
		BigWrite (FidIndex,(char *)&FileBounds,sizeof(MNMXCORD),-1);  
		FirstHeaderOffset = LastHeaderOffset = GSSillseek (FidIndex,0,1);    
			
		DBoundsInit (&lpFI->Bounds);   
		lpFI->NumFiles=0;
		lpFI->Length=0; 
		BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1); 
		CurHeaderWritten = TRUE;
		GSSillseek (Fid,0,0);
		while (fgetstring (str,256,Fid))
		{   
			CurLoc += GSSillseek (Fid,0,1);                                
		    _fstrlwr (str); 
		    if (_fstrstr (str,"filelist.txt"))
		    {   
		    	HANDLE	hDLT=0;
		    	char	SkipList[130], SkipDir[130];
			                    	
			    GetGlobalCVal ("[%SKIPLIST]",SkipList,NULL);
			    GetGlobalCVal ("[%SKIPDIR]",SkipDir,NULL); 
		    	FidFileList = GSSiOpenFile (str,&OFStruct,OF_READ);
		    	if (FidFileList == HFILE_ERROR)
		    		goto Exit;     
		    	if (!TotLen)
		    	{
		    		FileListLen = GSSillseek (FidFileList,0,2);
		    		TotLen = FileListLen * NumLevs;
		    		GSSillseek (FidFileList,0,0);
		    	} 
		    	fgetstring (str,128,FidFileList);
				if (!ProcessDelimTextHeader(str,NULL,FidFileList,&hDLT))
					goto Exit;
			                    	
		    	while (fgetstring (str,128,FidFileList))
		    	{   
		    		char	leaf[34]="[FILENAME]";
				    GetDelimTextData(str,hDLT); 
				    _fstrcpy (str,"[FULLNAME]");
				    ExpandText (str);  
				    ExpandText (leaf);
				    if (*SkipList)
				    {
				    	HANDLE hDLTSkip=0;
				    	char	skipfile[130];
				    	HFILE	FidSkip=GSSiOpenFile (SkipList,&OFStruct,OF_READ);
				    	if (FidSkip != HFILE_ERROR)
				    	{   
				    		fgetstring (skipfile,128,FidSkip);
							ProcessDelimTextHeader(skipfile,NULL,FidSkip,&hDLTSkip);
							while (fgetstring (skipfile,128,FidSkip))
							{
								GetDelimTextData(skipfile,hDLTSkip); 
								_fstrcpy (skipfile,"[SKIPNAME]");
								ExpandText (skipfile);
								if (!_fstricmp (skipfile,leaf))
								{  
						    		GSSiClose (FidSkip); 
						    		GSSiGlobFree (&hDLTSkip);  
						    		if (!CurLev)
						    		{
							    		_fstrcpy (skipfile,"[%SKIPDIR]\\[SKIPNAME].bpw");
							    		ExpandText (skipfile);
							    		copyfile (skipfile,str,FALSE,0,0,0,0,0,0);
							    		_fstrcpy (skipfile,"[%SKIPDIR]\\[SKIPNAME].bmp");
							    		ExpandText (skipfile); 
							    		REPLAC (str,".BPW",".BMP",sizeof(str));
							    		copyfile (skipfile,str,FALSE,0,0,0,0,0,0); 
							    	}
									goto NextFile;
								}	
							}
				    		GSSiGlobFree (&hDLTSkip); 
				    		GSSiClose (FidSkip);
				    	}
				    }
		    		sprintf (Mess,"File %s: Level %s",str,cOrthRes);
		        	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,Mess);
		            if (!AddMapToDir (hWndDlg,str,ToDir,lpFI,itype,  
		                          (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0),
                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_GETCHECK,0,0),
		                          GetDlgItem(hWndDlg,IDC_ORTHSTATUS),FidIndex,SizeOpt,cOrthRes,UsesTimes,
		                          &LastHeaderOffset,&CurHeaderWritten,&FileBounds,UseCPT,UseExCmp))
		                goto Exit;  
		            CurLoc = FileListLen * CurLev + GSSillseek (FidFileList,0,1);
		            PctBox (GetDlgItem(hWndDlg,IDC_STATUS_BAR), TotLen, CurLoc,0);
		        	CurOffset = GSSillseek (FidIndex,0,1);
		            if (!CurHeaderWritten && (itype > 1 || lpFI->NumFiles > 255))
		            {   
		            	lpFI->NextHeaderOffset = CurOffset;
		            	GSSillseek (FidIndex,LastHeaderOffset,0);
						BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
						GSSillseek (FidIndex,CurOffset,0);
						DBoundsInit (&lpFI->Bounds);   
						lpFI->NumFiles=0;
						lpFI->Length=0; 
		            	LastHeaderOffset = CurOffset;
						BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
						CurHeaderWritten = TRUE;
		            }
		            else
						CurHeaderWritten = FALSE;
		NextFile:;
					                    
		        }
		        GSSiClose (FidFileList);  
				GSSiGlobFree (&hDLT);
		    }
		    else
		    {
				sprintf (Mess,"File %s: Level %s",str,cOrthRes);  
				if (hWndDlg)
				{
			    	SetDlgItemText(hWndDlg,MESSCntl,Mess);
			        SetWindowText(hWndDlg,Mess);
			    } 
		        if (!AddMapToDir (hWndDlg,str,ToDir,lpFI,itype,   
		                      (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0),
                              (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_GETCHECK,0,0),
		                      GetDlgItem(hWndDlg,IDC_ORTHSTATUS),FidIndex,SizeOpt,cOrthRes,UsesTimes,
		                      &LastHeaderOffset,&CurHeaderWritten,&FileBounds,UseCPT,UseExCmp))
		            goto Exit;
		        if (hWndDlg)
		        	PctBox (GetDlgItem(hWndDlg,STATUSCntl), NumLevs*TotLen, CurLoc,0);
		    	CurOffset = GSSillseek (FidIndex,0,1);
		        if (!CurHeaderWritten && (itype > 1 || lpFI->NumFiles > 255))
		        {   
		        	lpFI->NextHeaderOffset = CurOffset;
		        	GSSillseek (FidIndex,LastHeaderOffset,0);
					BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
					GSSillseek (FidIndex,CurOffset,0);
					DBoundsInit (&lpFI->Bounds);   
					lpFI->NumFiles=0;
					lpFI->Length=0; 
		        	LastHeaderOffset = CurOffset;
					BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
					CurHeaderWritten = TRUE;
		        }
		        else
					CurHeaderWritten = FALSE;
		    } 
		 }
	Exit:
		 GSSiChangeLength (FidIndex,CurOffset);
		 GSSillseek (FidIndex,0,2);
		 BigWrite(FidIndex,(char *)&Signature,4,-1);
		 BigWrite(FidIndex,(char *)&Version,2,-1);
		 GSSillseek (FidIndex,0,0);
		 BigWrite (FidIndex,(char *)&FileBounds,sizeof(MNMXCORD),-1); 
		 if (!CurHeaderWritten)
		 { 
		     GSSillseek (FidIndex,LastHeaderOffset,0);
		     BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
		 }
		 GSSillseek (FidIndex,FirstHeaderOffset,0);
		 BigRead (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH);
		 lpFI->EndOffset = CurOffset;
		 GSSillseek (FidIndex,FirstHeaderOffset,0);
		 BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
		 GSSiClose (FidIndex);  
		 GSSiGlobUlFree (&hlpFI);
		 if (Compress)
		    AVIOutClose(&hAVIFile);
		 DisableHalt = FALSE; 
		 DoPaint = TRUE; 
		 CurLev++;
	} while (*pOrthRes); 
	GSSiClose (Fid);
	if (itype == 1 && hWndDlg)
	{  
		long	lmem=sizeof(VIEWPORT)+128*MAX_VIEWPORT_FILES; 
		HANDLE	hVP=GSSiGlobAlloc (1117,GHND,lmem);
		LPVIEWPORT	pSaveVP = (LPVIEWPORT)GlobalLock (hVP);  
		HANDLE		hSymDesc;
		short		NumParent=0, NumSyms=0;
		HFILE		FidSyms;
		LPSYMDESC	pSymDesc;
		LPSYMBOL    pSymbol;  
		LPSTR		lpTAB;   
		VISLIST		SaveVis;
		char		VisName[128];
			             	
		SetCurView (pViewports[*pCommandViewport-1]);
		_fmemmove (pSaveVP,CurView,(size_t)lmem);  
		CurView->NumFiles = 1;
		CurView->FileType[0]=4;
		_fstrcpy (CurView->lpFiles[0],ToDir);
		_fstrcat (CurView->lpFiles[0],"\\symlist"); 
		GSSiRemove (CurView->lpFiles[0]);  
		_fstrcpy (VisName,CurView->lpFiles[0]);
		_fstrcpy (CurView->lpFiles[0],ToDir);
		_fstrcat (CurView->lpFiles[0],"\\index");
		SaveVis = *CurVis; 
		IgnoreBounds = TRUE;
		InitVis ();   
		GetVisList (hWndDlg,SYM_VIS_LB, PAR_VIS_LB,0,1,HFILE_ERROR); 
		*CurVis = SaveVis;
		IgnoreBounds = FALSE; 
		FidSyms = GSSiOpenFile (VisName,&OFStruct,OF_CREATE);
		hSymDesc = GSSiGlobAlloc (1118,GMEM_MOVEABLE,UINT_MAX);
		pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
		while (SendDlgItemMessage (hWndDlg,SYM_VIS_LB,LB_GETTEXT,NumSyms++,(LPARAM)str)!=LB_ERR) 
		{   
			pSymDesc->Handle = GSSiGlobAlloc (1119,GHND,sizeof(SYMBOL));
		    pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle); 
		    lpTAB = _fstrrchr (str,'\t');
		    *lpTAB++ = 0;
		    pSymDesc->Number = atoi (lpTAB);
		    lpTAB = _fstrrchr (str,'\t');
		    *lpTAB++ = 0;
		    pSymbol->Parent = atoi (lpTAB);
		    pSymbol->Type = 1;
		    lpTAB = _fstrrchr (str,'\t');
		    *lpTAB = 0;
		    _fstrcpy (pSymbol->Name,str);
		    GlobalUnlock (pSymDesc++->Handle); 
		} 
		NumSyms--;
		while (SendDlgItemMessage (hWndDlg,PAR_VIS_LB,LB_GETTEXT,NumParent++,(LPARAM)str)!=LB_ERR) 
		{   
			NumSyms++;
			pSymDesc->Handle = GSSiGlobAlloc (1120,GHND,sizeof(SYMBOL));
		    pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle); 
		    lpTAB = _fstrrchr (str,'\t');
		    *lpTAB++ = 0;
		    pSymDesc->Number = atoi (lpTAB);
		    lpTAB = _fstrrchr (str,'\t');
		    *lpTAB++ = 0;
		    pSymbol->Parent = atoi (lpTAB);
		    pSymbol->Type = 0;
		    lpTAB = _fstrrchr (str,'\t');
		    *lpTAB = 0;
		    _fstrcpy (pSymbol->Name,str);
		    GlobalUnlock (pSymDesc++->Handle); 
		}  
		NumParent--;
		GlobalUnlock (hSymDesc); 
		WriteSymList (FidSyms, NumParent, NumSyms,hSymDesc);
		DestroySymList (&NumSyms,&hSymDesc);
		_fmemmove (CurView,pSaveVP,(size_t)lmem);
		GlobalUnlock (hVP);
		GlobalFree (hVP); 
		GSSiClose (FidSyms);
	}
	Processing = FALSE;
	ContinueProcessing = TRUE;  
                 
	return TRUE;
}

BOOL FAR PASCAL LOADDOQSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    short     TabStops[2]={1200,1300}, i;
    char    Ext[8], ExtID[34], Name[128],str[260], VolLabel[16], CDDrive[6];
    LPSTR    lpstr=str;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    long        TotLen, CurLoc, County, NumCounties;
    LPLONG      pCounty, pCnty;  
	LPSTR	pCDDrive, pEnd;
    static      short     FileType = 0;  
	static		char		CDName[10]="";
	static		short		FileNum=0;
	static		char		CDSeries[16]="";
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         
         FileNum++; 
         SetDlgItemText (hWndDlg,IDC_CD,CDName);
         SetDlgItemInt	(hWndDlg,IDC_FILENUM,FileNum,FALSE);  
         SetDlgItemText (hWndDlg,IDC_CDSERIES,CDSeries);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDC_LOCATE_SOURCE: 
            {    
                BOOL    HaveMID=FALSE;
                LPTAGDEF    lpTAGDef; 
                short     nFields,ii; 
                LPSTR   lpSpace, lpDelim; 
				UINT	PrevErrMode;
				
				PrevErrMode = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
                SendDlgItemMessage (hWndDlg,IDC_QUAD_LIST,LB_RESETCONTENT,0,0);
				for (i=3;i<26;i++)
				{   
					ii=GetDriveType (i);
					if (GetDriveType (i) == DRIVE_REMOTE)
					{                 
						if (GetVolumeLabel(i,VolLabel))
						{
		                    sprintf (str,"%s (%c:)",VolLabel,(char)('A'+i));
		                    SendDlgItemMessage (hWndDlg,IDC_QUAD_LIST,LB_ADDSTRING,0,(LPARAM)str); 
		                } 
		            }
				} 
				SetErrorMode (PrevErrMode);
            }
                 break;
                 
            case IDC_LOCATE_DESTMAP: 
                 *str=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
                 break;
            
            case IDC_AUTOMARK:
            {
				HCURSOR	hcurSave;
			    GWDHEADER GWDHead; 
			    LPGWDHEADER lpGWDHead;
			    HANDLE  hDB;   
			    long	Offset;
				LPQUADDATA	pQuadData;  
				BOOL	rtn=FALSE;
				char	Name[]="[%DATA_LOC]quads.gmd"; 
				char	Quad[16];
				short	pos=BT_FIRST; 
			
				hDB = OpenGWDatabase (Name,BT_READ); 
				if (!hDB)
					break;
			    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
			    pQuadData = (LPQUADDATA)&lpGWDHead->GWDData;
				while (!BT_FIND (lpGWDHead->BTHandle[0],Quad,pos,BT_ANY,(LPSTR)&Offset))
			    {   
			    	pos = BT_NEXT;
			        FillGWDData (lpGWDHead,Offset);
			        if (pQuadData->ZoneCode == GetGlobalLVal ("[%UTMZONE]")) 
            	 		MarkQuadData ("usgsauto.gmd",pQuadData->QuadID,pQuadData->VolLabel,1); 
			    }
				GlobalUnlock (hDB);
			    CloseGWDatabase (hDB); 
            	 
			    GSSiSetCursor (hcurSave); 
			}
                break;
                 
            case IDC_MARK: 
            {    
            	 OFSTRUCT	OFStruct;
            	 HFILE		OutFid;
            	 char		Quad[14]="O4509353.SEH";
				 HIGHLIGHTDATA       HighlightData;  
				 short		pos;
				 long		Refno;
				 BOOL		Err;
            	 
                 if (!GetDlgItemText (hWndDlg,IDC_CDSERIES,CDSeries,sizeof(CDSeries)))
                 	break;
                 GetDlgItemText (hWndDlg,IDC_CD,CDName,8);
                 FileNum = GetDlgItemInt (hWndDlg,IDC_FILENUM,&Err,FALSE);
			     if (!hHighlight)
			     {
			        MessageBox( GetFocus(),"No Highlight List","Error", MB_OK);
			        break;
			     }  


			     pos = BT_FIRST;
			     while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
            	 {    
            	 	pos = BT_NEXT;  
            	 	_fstrcpy (Quad,HighlightData.PD.UDI); 
            	 	if (!_fstricmp (HighlightData.PD.Prefix,"USGSQUAD"))
	            		MarkQuadData (CDSeries,Quad,CDName,FileNum); 
            	 }
            	 GSSiClose (OutFid);      
            	 EndDialog(hWndDlg, TRUE);
            }
            	 break;     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
                    ContinueProcessing=FALSE;
                 else 
                 {
                 	ContinueProcessing = TRUE;
                    EndDialog(hWndDlg, FALSE);
                 }
                 break;
            
            case IDOK: 
            {
                 MNMXCORD MinMaxCoord;  
                 long   lineno=0, TotLen, CurLoc, MidLine;  
                 long   NewRefno; 
                 short    st, SymNum, iUDI=0, AreaSym, LineSym, Pass;
                 BOOL   Done; 
                 HANDLE hMIDstr;
                 char   Prefix[10], UDI[34], AreaSymName[10], LineSymName[10]; 
                 LPSTR  lpComma;  
                 OFSTRUCT   OFStruct;
                 long       filecode,filelength,ii=100, NumOverLimit;  
                 char   mess[256];  
                 short        nPoly, nPoints,lastnpoints, Symbol, MaxSides;
                 long   nVertex;
                 DPOINT	LinkPoint, FirstIslandPoint;
                 HANDLE hPoly;
                 HPDPOINT pPoints;
                 HANDLE     hPoints, hnPoints;
                 short        NumPoints, Nump;  
                 short    NumSyms=0, n;  
                 HANDLE hSymDesc=0; 
                 BOOL   Store,WantFirstPoint;   
                 char   LSym[20], VolLabel[8], SaveAlt[64];
                 LPSTR  lpBeg, lpEnd, lpSpace;
                 long nItems;
                 HANDLE hItems, hTran;  
                 LPSHORT   lpItems; 
                 short	item;        
                 double	XBASE[4], YBASE[4], XORTH[4], YORTH[4];
                 float	RSQMIN;
                                          
                                        
                 _fstrcpy (SaveAlt,"[%ALT_PROJECTION]");
                 ExpandText (SaveAlt);
                 if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,PltName,128))
                 {  
                    
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 PltType = 2; 
                 _fstrcpy (Prefix,"USGSQUAD");
                 AreaSym = GetDictSymbolNumber (Prefix);
                 if (AreaSym)
                    AddToSymList (AreaSym,&NumSyms,&hSymDesc);
                 else
                 {
                    sprintf (mess,"Unable to load area symbol: %s",Prefix);
                    MessageBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                 }
                 if ((nItems=SendDlgItemMessage(hWndDlg,IDC_QUAD_LIST,LB_GETSELCOUNT,0,0)))
                 {   
                    
                     hItems = GSSiGlobAlloc (1121,GMEM_MOVEABLE,nItems*2);
                     lpItems = (LPSHORT) GlobalLock (hItems); 
                     SendDlgItemMessage(hWndDlg,IDC_QUAD_LIST,LB_GETSELITEMS,(WPARAM)nItems,(LPARAM)lpItems); 
                 }
                 else
                 {
                    MessageBox( GetFocus(), "No CDs Selected",0, MB_OK);
                    break;
                 } 
                 ContinueProcessing = TRUE;
                 ExpandText (PltName);
                 PltType = 2;
                 Done = FALSE;
                 MidLine=0; 
                 str[0]=0;  
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
                    Pass = 0;
                 else 
                 {
                 	OpenMap (0,CurView->hDC); 
                 	if (!AnyDataInMap())
                 	{
                 		Pass = 0;
						MinMaxCoord = CurView->FileMNMX;
                 		goto EndFile;
                 	}
                    Pass = 1;                
					CloseMap(FALSE);
                 }
                 
                 SaveFPT = FileProjectionType;      
				 FileProjectionType=0; 
				 Processing = TRUE;
                 DisableHalt = TRUE;  
NextPass:        
				 NumOverLimit = MaxSides = 0;
                 if (Pass)
                 {
                    SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
					EditBounds = CurView->FileMNMX;
				 }                    
                 else
                 {       
                    SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Scanning for min/max coordinates");
                    MinMaxCoord.xmn = DBL_MAX;
                    MinMaxCoord.xmx = -DBL_MAX; 
                    MinMaxCoord.ymn = DBL_MAX;
                    MinMaxCoord.ymx = -DBL_MAX; 
                 }       
                                    
                 NewRefno=0;  
                 item = -1;
NextLine:       item++;
				if (item >= nItems)
					goto EndFile;
				lpItems = (LPSHORT)GlobalLock (hItems);
				lpItems += item;
				SendDlgItemMessage(hWndDlg,IDC_QUAD_LIST,LB_GETTEXT,*lpItems,(LPARAM)str); 
                Store = TRUE;
                pCDDrive = _fstrrchr (str,'('); 
                *pCDDrive = 0;
                _fstrncpy (VolLabel,str,6);
                pCDDrive++;
                pEnd = _fstrchr (pCDDrive,':');
                *pEnd = 0;     
                _fstrcpy (CDDrive,pCDDrive);
				{
				 DPOINT BasePoint;
				 char   NearRec[256];
				#if WIN32
				    WIN32_FIND_DATA FindFileData;
				    LPWIN32_FIND_DATA lpFindFileData = &FindFileData; 
				    HANDLE st;
				#else
				    struct _find_t FileInfo;
				    short st;
				#endif
				 char   str[514], Name[32], name[128], Date[8];  
				 TRN        tr;
				 short    i, rtn, nr, ElvUnits;  
				 long	FltHeight;
				 HFILE  FidOrtho,FidDump,FidCvt; 
				 LPSTR  lpStr;  
				 OFSTRUCT   OFStruct;
				 long   nLines, nSamples;    
				 DPOINT NWPoint, SWPoint, SEPoint, NEPoint; 
				 DPOINT NWOrtho, SWOrtho, SEOrtho, NEOrtho;
				 double DistGround, DistOrtho, GroundPerOrtho;
				 static HANDLE hRect=0,hNames=0,hCoord=0,hRes=0;
				 double	Res; 
				 DPOINT	Coord[5];
				 static short nRect=0;
				 HANDLE hTranOrthoToBase,hTranBaseToOrtho;
				 float  RSQMIN;  
				 char	ProjectionName[16]; 
 				 QUADDATA	QuadData; 
 				 short	BMWidth, BMHeight, HeightInc, WidthInc;   
 				 char	mess[128];
				 char vl[8];

				
				        sprintf(str,"%s:\\data\\*.??h",CDDrive);
				#if WIN32
				        st = FindFirstFile(str,lpFindFileData);
				#else
				        st = _dos_findfirst (str,_A_NORMAL,&FileInfo);
				#endif  
						if (st)
						{   
							
							_fstrncpy (vl,VolLabel,6);
							vl[6]=0;      
							Truncate (vl);
				        	sprintf(str,"%s:\\%s\\data\\*.??h",CDDrive,vl);
					        st = _dos_findfirst (str,_A_NORMAL,&FileInfo);
						}
						else
							*vl=0;
				        while (!st)
				        {   
				#if WIN32
				            strcpy(name,lpFindFileData->cFileName);
				#else
				            _fstrcpy(name,FileInfo.name);
				#endif      
							if (Pass)
								sprintf (mess,"Loading data: Quad %s",name); 
							else
								sprintf (mess,"Scanning for min/max coordinates: Quad %s",name); 
							SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,mess);
							_fmemset (&QuadData,0,sizeof(QUADDATA)); 
							_fstrncpy (QuadData.VolLabel,VolLabel,6); 
							if (*vl)
				            	sprintf (Name,"%s:\\%s\\data\\%s",pCDDrive,vl,name); 
				            else
				            	sprintf (Name,"%s:\\data\\%s",pCDDrive,name);
				            _fstrcpy (UDI,name);
				            _fstrncpy (QuadData.QuadID,name,12);
				            FidOrtho=GSSiOpenFile(Name,&OFStruct,OF_READ); 
				            lpStr = str;

//Read record 1
				            fgetstring (lpStr,510,FidOrtho); str[400]=0; 
				            if (_fstrstr (str,"DLG")) // cant handle optional DLG format yet
				            {
					            GSSiClose (FidOrtho);
				            	goto NextFile;
				            }
				            
				            nLines = atol (&str[144]); 
				            nSamples = atol (&str[150]); 
				            PRJ_UNITS[3] = atoi (&str[204]);
				            QuadData.nRows = nLines;
				            QuadData.nCols = nSamples; 
				            QuadData.Datum = ldread (&str[167],2);   
				            QuadData.ProjectionCode = ldread (&str[195],3);
				            QuadData.ZoneCode = ldread (&str[198],6); 
				            lpStr = &str[207-24];
				            for (i=0;i<4;i++) 
				            {
					            XBASE[i] = dread (lpStr+=24,24);
					            YBASE[i] = dread (lpStr+=24,24);
				            }
				            
				            lpStr = str; 
				            GSSillseek (FidOrtho,nSamples,0); 
//Read record 2				       
				            fgetstring (lpStr,510,FidOrtho); str[400]=0;   
				            tr.a = dread (lpStr,24);
				            tr.b = dread (lpStr+=24,24);
				            tr.c = dread (lpStr+=24,24);
				            tr.d = dread (lpStr+=24,24);
				            tr.e = dread (lpStr+=24,24);
				            tr.f = dread (lpStr+=24,24);
				            tr.xc = dread (lpStr+=24,24);
				            tr.yc = dread (lpStr+=24,24);
				            
				            GSSillseek (FidOrtho,2*nSamples,0);
				            lpStr = str; 
//Read record 3				            
				            fgetstring (lpStr,510,FidOrtho); str[400]=0;   
				            lpStr = &str[192]; 
				            YORTH[0] = SWOrtho.y = dread (lpStr,6);
				            XORTH[0] = SWOrtho.x = dread (lpStr+=6,6);
				            YORTH[1] = NWOrtho.y = dread (lpStr+=6,6);
				            XORTH[1] = NWOrtho.x = dread (lpStr+=6,6);
				            YORTH[2] = NEOrtho.y = dread (lpStr+=6,6);
				            XORTH[2] = NEOrtho.x = dread (lpStr+=6,6);
				            YORTH[3] = SEOrtho.y = dread (lpStr+=6,6);
				            XORTH[3] = SEOrtho.x = dread (lpStr+=6,6); 
							hTran = STRAN2 (XORTH,YORTH,XBASE,YBASE,4,(LPFLOAT)&RSQMIN,1,NULL);
				            QuadData.ClipRect.left = SWOrtho.x = NWOrtho.x = min (SWOrtho.x,NWOrtho.x);
				            QuadData.ClipRect.right = SEOrtho.x = NEOrtho.x = max (SEOrtho.x,NEOrtho.x);
				            QuadData.ClipRect.top = NWOrtho.y = NEOrtho.y = min (NWOrtho.y,NEOrtho.y);
				            QuadData.ClipRect.bottom = SWOrtho.y = SEOrtho.y = max (SWOrtho.y,SEOrtho.y);
				            BMHeight = QuadData.ClipRect.bottom - QuadData.ClipRect.top + 1;
				            BMWidth = QuadData.ClipRect.right - QuadData.ClipRect.left + 1;
				            WidthInc = BMWidth%16;
				            if (!WidthInc)
				            	WidthInc = 16;
				            HeightInc = BMHeight%16;
				            if (!HeightInc)
				            	HeightInc = 16;
				            WidthInc = (WidthInc+1)/2;
				            HeightInc = (HeightInc+1)/2;
				            NWOrtho.x -= WidthInc;
				            NWOrtho.y -= HeightInc;
				            NEOrtho.x += WidthInc;
				            NEOrtho.y -= HeightInc;
				            SEOrtho.x += WidthInc;
				            SEOrtho.y += HeightInc;
				            SWOrtho.x -= WidthInc;
				            SWOrtho.y += HeightInc;
				            QuadData.ClipRect.left -= WidthInc;
				            QuadData.ClipRect.right += WidthInc;
				            QuadData.ClipRect.top -= HeightInc;
				            QuadData.ClipRect.bottom += HeightInc;
				            /*QuadData.NEClipPoint = OrthoToGround(NEOrtho,&tr);
				            QuadData.SWClipPoint = OrthoToGround(SWOrtho,&tr);
				            QuadData.SEClipPoint = OrthoToGround(SEOrtho,&tr);
				            QuadData.NWClipPoint = OrthoToGround(NWOrtho,&tr);*/  
				            QuadData.NEClipPoint = OrthoToGround2(NEOrtho,hTran);
				            QuadData.SWClipPoint = OrthoToGround2(SWOrtho,hTran);
				            QuadData.SEClipPoint = OrthoToGround2(SEOrtho,hTran);
				            QuadData.NWClipPoint = OrthoToGround2(NWOrtho,hTran);
				            CloseTRANS2 (&hTran);
				            DistOrtho = ldistp (SWOrtho,NEOrtho);
				            DistGround = ldistp (QuadData.SWClipPoint,QuadData.NEClipPoint);
				            QuadData.Res = DistGround / DistOrtho; 
//Read record 4				
				            GSSillseek (FidOrtho,3*nSamples,0);
				            lpStr = str; 
				            fgetstring (lpStr,510,FidOrtho); str[400]=0;   
				            GSSiClose (FidOrtho);
				            ElvUnits = ldread (str,3);
				            lpStr = &str[214]; 
				            _fstrncpy (QuadData.Date,lpStr,8);
                            FltHeight = ldread (&str[230],10);
//                            if (ElvUnits == 2)
//                            	FltHeight *= MFT;
                            QuadData.FltHeight = FltHeight; 
                            FidCvt = GSSiOpenFile ("usgsdoq.cvt",&OFStruct,OF_CREATE); 
                            itoa (QuadData.ProjectionCode,str,10);
                            fputstring (str,FidCvt);
                            ltoa (QuadData.ZoneCode,str,10);
                            fputstring (str,FidCvt);
                            fputstring ("8",FidCvt);
                            fputstring ("1.0",FidCvt);
                            for (i=0;i<14;i++)
                            	fputstring ("0.0",FidCvt);
                            GSSiClose (FidCvt); 
					        ConvertCoordClose (); 
                            _fstrcpy (str,"[%ALT_PROJECTION]=USGSDOQ");
				            ExpandText (str);
				            ConvertCoord(&QuadData.SWClipPoint,3,1);
				            ConvertCoord(&QuadData.NEClipPoint,3,1);
				            ConvertCoord(&QuadData.SEClipPoint,3,1);
				            ConvertCoord(&QuadData.NWClipPoint,3,1);
				            DistOrtho = ldistp (SWOrtho,NEOrtho);
				            DistGround = ldistp (QuadData.SWClipPoint,QuadData.NEClipPoint);
				            QuadData.Res = DistGround / DistOrtho; 
				            if (Pass)
				            {
					            if (!WriteQuadFile (&QuadData))
					            	goto NextFile;   
					        }
                NumPoints =5;  
                hPoly = GSSiGlobAlloc (1122,GMEM_MOVEABLE,sizeof(DPOINT)*(long)NumPoints*2);
                pPoints = (HPDPOINT)GlobalLock (hPoly); 
	            *pPoints++ = QuadData.SWClipPoint;
	            *pPoints++ = QuadData.NWClipPoint;
	            *pPoints++ = QuadData.NEClipPoint;
	            *pPoints++ = QuadData.SEClipPoint;
	            *pPoints = QuadData.SWClipPoint;
                GlobalUnlock (hPoly);
            	pPoints = (HPDPOINT) GlobalLock (hPoly); 
                Store = TRUE;
                for (i=0;i<NumPoints;i++,pPoints++)
                {
                    Store = PointInFileBounds (pPoints,&MinMaxCoord,Pass);
                }
                GlobalUnlock (hPoly);
                if (Pass)
                {
                     if (Store)
                     {     
                 	 	NewRefno = GetNewRefno (PltName,NULL,NULL,NULL,NULL);
                        AddPolyToMap (1,
                                      &NumPoints, 
                                      &hPoly,0,NewRefno,NULL,-1,AreaSym,0,Prefix,UDI,
                                        -1,0,0,0,0,0,0,FALSE);
                     } 
                 }
                 GlobalFree (hPoly); 
NextFile:                 
				#if WIN32
				            if(!FindNextFile(st,lpFindFileData)) break;
				#else
				            st = _dos_findnext (&FileInfo);
				#endif
		        }
			}  
			GlobalUnlock (hItems);
        goto NextLine;         
                    
        EndFile: 
	     CloseMap (TRUE);
	     CloseRefIndex(TRUE);           
	     if (!Pass && ContinueProcessing)
	     {
	         short NumPens=10;
	         PENDESC PenDesc[10];
	                     
	         for (i=0;i<NumPens;i++)
	         {
	            PenDesc[i].PenNum = i+1;
	            PenDesc[i].Width = (float)1.0; 
	            PenDesc[i].Style = 1;
	            PenDesc[i].Color = RGB(0,0,0);
	         }
	         CreateNewMap (PltName,&MinMaxCoord,NumSyms,hSymDesc,
	                                            NumPens,(LPPENDESC)&PenDesc,0,0,TRUE);  
	        Pass = 1;
	        lineno = 0;
	        goto NextPass;
	     } 
	//                 else if (!SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0))
	//                     AddSymToMap (PltName,NumSyms,hSymDesc,0,NULL);  
	
	     DestroySymList (&NumSyms,&hSymDesc);
	                 
	     DisableHalt = FALSE;
	     FileProjectionType = SaveFPT;   
	     Processing = FALSE;
	     ContinueProcessing = TRUE;  
         ConvertCoordClose ();  
         SetGlobalValue ("%ALT_PROJECTION",SaveAlt);
         ExpandText (str);
	     
	     EndDialog(hWndDlg, TRUE); 
	     break;
	                 
		}   

        case IDC_CATALOG: 
        {
             char   LSym[20], VolLabel[10], SaveAlt[64];
             LPSTR  lpDot;
             long nItems;
             HANDLE hItems;  
             LPSHORT   lpItems; 
             short	item;        
			 BOOL	HaveWorld;
                                        
             if ((nItems=SendDlgItemMessage(hWndDlg,IDC_QUAD_LIST,LB_GETSELCOUNT,0,0)))
             {   
                    
                 hItems = GSSiGlobAlloc (1123,GMEM_MOVEABLE,nItems*2);
                 lpItems = (LPSHORT) GlobalLock (hItems); 
                 SendDlgItemMessage(hWndDlg,IDC_QUAD_LIST,LB_GETSELITEMS,(WPARAM)nItems,(LPARAM)lpItems); 
             }
             else
             {
                MessageBox( GetFocus(), "No CDs Selected",0, MB_OK);
                break;
             } 
             ContinueProcessing = TRUE;
            item = -1;
NextCatLine:item++;
			if (item >= nItems)
				goto EndCatFile;
			lpItems = (LPSHORT)GlobalLock (hItems);
			lpItems += item;
			SendDlgItemMessage(hWndDlg,IDC_QUAD_LIST,LB_GETTEXT,*lpItems,(LPARAM)str); 
            pCDDrive = _fstrrchr (str,'('); 
            *pCDDrive = 0;
            _fstrncpy (VolLabel,str,10);  
            Truncate (VolLabel);
            pCDDrive++;
            pEnd = _fstrchr (pCDDrive,':');
            *pEnd = 0;     
            _fstrcpy (CDDrive,pCDDrive);
				{
				 DPOINT BasePoint;
				 char   NearRec[256];
				#if WIN32
				    WIN32_FIND_DATA FindFileData;
				    LPWIN32_FIND_DATA lpFindFileData = &FindFileData; 
				    HANDLE st; 
				#else
				    struct _find_t FileInfo;
				    short st;
				#endif
				 char   str[512], Name[128], name[128], Date[8];  

				
				        sprintf(str,"%s:\\*.*",CDDrive);
				        st = _dos_findfirst (str,_A_SUBDIR,&FileInfo); 
				        _fstrcpy(VolLabel,FileInfo.name);
				        sprintf(str,"%s:\\%s\\*.bmp",CDDrive,VolLabel);
				#if WIN32
				        st = FindFirstFile(str,lpFindFileData);
				#else
				        st = _dos_findfirst (str,_A_NORMAL,&FileInfo);
				#endif  
				        while (!st)
				        {   
				#if WIN32
				            strcpy(name,lpFindFileData->cFileName);
				#else
				            _fstrcpy(name,FileInfo.name);
				#endif  
						    if ((lpDot = _fstrrchr (name,'.')))
						    	*lpDot = 0;
						    sprintf(Name,"%s:\\%s\\%s.bpw",CDDrive,VolLabel,name);
		                    HaveWorld = ExistFile (Name);
							OutputCatalogData (name, VolLabel, HaveWorld);
				   
				#if WIN32
				            if(!FindNextFile(st,lpFindFileData)) break;
				#else
				            st = _dos_findnext (&FileInfo);
				#endif
		        }
			}  
			GlobalUnlock (hItems);
        goto NextCatLine;         
                    
EndCatFile: 
	     DisableHalt = FALSE;
	     Processing = FALSE;
	     ContinueProcessing = TRUE;  
	     
	     EndDialog(hWndDlg, TRUE); 
	     break;
	                 
		}   


	}
	break;

    default:
        return FALSE;
   }
 return TRUE;    
} 


BOOL TransformImage (LPSTR TIFFile,LPSTR TranFile,LPSTR BMPFile,LPSTR BPWFile,LPSTR AreaFileOrTAG)
{   
	HANDLE	hDib, hDibNew, hTran, hTranBack;
	char	TranFileBack[128], str[256];
	MNMXCORD	Bounds;
	double	Res; 
	HFILE	Fid; 
	BOOL	rtn;  
	HANDLE	hAreaPts=0;
	long	nAreaPts=0;      
	LPDPOINT	AreaPoints;
	
	_fstrcpy (TranFileBack,"|OPP|");
	_fstrcat (TranFileBack,TranFile);
	hDib = LoadDIB (TIFFile); 
	if (!hDib)
		return FALSE;
	hTran = LoadTranFileWithDandT (TranFile);   
	hTranBack = LoadTranFileWithDandT (TranFileBack);
	if (!hTran)
	{
		DestroyDIB(hDib); 
		return FALSE;
	}
	if (AreaFileOrTAG && *AreaFileOrTAG)
	{   
		HFILE FidArea = GSSiOpenFile (AreaFileOrTAG,NULL,OF_READ);
		
		hAreaPts = GSSiGlobAlloc (0,GMEM_MOVEABLE,UINT_MAX);
		AreaPoints = (LPDPOINT)GlobalLock (hAreaPts);
		if (FidArea != HFILE_ERROR)
		{   
			while (fgetstring (str,128,FidArea))
				if (sscanf (str,"%Flf %Flf",&AreaPoints[nAreaPts].x,&AreaPoints[nAreaPts].y) == 2)
					nAreaPts++;  
			GSSiClose (FidArea);

		}
		GlobalUnlock (hAreaPts);
		if (nAreaPts == 2)
			nAreaPts = ConvertRectToArea (&hAreaPts);
	}
	hDibNew = TransformBitmap (hDib,hTran,hTranBack,&Bounds,&Res,BMPFile,nAreaPts,hAreaPts,TRUE); 
	if (hDibNew)
	{
		SaveDIB (hDibNew,BMPFile); 
		Fid = GSSiOpenFile (BPWFile,NULL,OF_CREATE);
		fputstring ("METERS",Fid);
		sprintf (str,"%f",Bounds.xmn);
		fputstring (str,Fid);
		sprintf (str,"%f",Bounds.ymn);
		fputstring (str,Fid);
		sprintf (str,"%f",1.0/Res);
		fputstring (str,Fid);
		fputstring (BMPFile,Fid);
		GSSiClose (Fid);
		GSSiGlobFree (&hDibNew); 
		rtn = TRUE;
	}
	else
		rtn = FALSE;
	DestroyDIB (hDib); 
	CloseTRANS2 (&hTran);  	
	CloseTRANS2 (&hTranBack); 
	GSSiGlobFree (&hAreaPts); 	
	return rtn;
}

BOOL CompressFrameEX (LPBITMAPINFOHEADER	lpbiIn, LPBITMAPINFOHEADER lpbiOut,long Quality)
{
	BOOL	rtn;
	FARPROC lpfnCmpImageMsgProc; 
	HFILE	Fid; 
	long	len;
	
	if (!*CmpImageInFile)
	{
		GSSiGetTempFileName (0,"gm1",0,CmpImageInFile);
		GSSiGetTempFileName (0,"gm2",0,CmpImageOutFile);
	}  
	CmpImageQuality = Quality;
	Fid = GSSiOpenFile (CmpImageInFile,NULL,OF_CREATE);
	len = lpbiIn->biSize + lpbiIn->biClrUsed * sizeof(RGBQUAD) + lpbiIn->biSizeImage; 
	BigWrite (Fid,(HPSTR)&len,4,-1);
	BigWrite (Fid,(HPSTR)lpbiIn,len,-1); 
	GSSiClose (Fid);
	lpfnCmpImageMsgProc = MakeProcInstance((FARPROC)CmpImageMsgProc, hInst);
	rtn = DialogBox(hInst, (LPSTR)"LOADBMP", hWndMain, lpfnCmpImageMsgProc);
	FreeProcInstance(lpfnCmpImageMsgProc);
	if (rtn)
	{
		Fid = GSSiOpenFile (CmpImageOutFile,NULL,OF_READ);
		BigRead (Fid,(HPSTR)&len,4);
		BigRead (Fid,(HPSTR)lpbiOut,len); 
		GSSiClose (Fid);
    }
	return rtn;
}

BOOL FAR PASCAL CmpImageMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[512];
	HANDLE hI; 
 switch(Message)
   {
    case WM_INITDIALOG:  
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_LOADFREEIMAGEBMP, 0L);
         return 0;  
         break; 
    
    case WM_PAINT:
//         ShowWindow (hWndDlg,SW_HIDE);
         return 0;
    	 break;
    	 
    case WM_CLOSE:
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; 

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDC_LOADFREEIMAGEBMP:
            	sprintf (str,"%s %s;%s;%ld;%ld","[%DL]cmpframe.exe",CmpImageInFile,CmpImageOutFile,CmpImageQuality,(long)hWndDlg);
               	ExpandText (str);
				hI = WinExec (str,SW_SHOWNORMAL);
				if (hI <32)
				{
					DisplayShellExError (hI,str);
					PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
				} 
            	break;
            case IDOK:
                EndDialog(hWndDlg, (int)lParam);
            	break;
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL GetOriginalImageBounds (LPSTR IndexPath,LPSTR ImageName,LPMNMXCORD	lpBounds)
#if ENABLETRACE
{GSSiEnterProg (604);
#endif
{
	LPFILEINDEX lpIndex;
	HANDLE	handle; 
	LPSTR	lpDot;
	BOOL	SaveIgnoreBounds=IgnoreBounds; 
		                 	
	DBoundsInit (lpBounds);
	IgnoreBounds=TRUE;
	handle = OpenMapIndex (IndexPath,NULL);
	if (!handle)
	{
		IgnoreBounds = SaveIgnoreBounds;
{
#if ENABLETRACE
GSSiExitProg (604);
#endif
		return FALSE;   
}
	}
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
	if (lpIndex->FileInIndex >= lpIndex->NumFiles-1)
	{   
	    if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
		{
			IgnoreBounds = SaveIgnoreBounds;
{
#if ENABLETRACE
GSSiExitProg (604);
#endif
	    	return TRUE;
}
	    }
	    goto NextFile;
	}
	GetNextIndexEntry(lpIndex); 
	lpDot = _fstrrchr (lpIndex->CurrentEntry->Name,'.');
	if (lpDot)
		*lpDot = 0;
	if (!_fstricmp (lpIndex->CurrentEntry->Name,ImageName))
		AddMinMaxD (lpBounds,&lpIndex->CurrentEntry->Bounds);
	if (lpDot)
		*lpDot = '.';
	goto NextFile;
#if ENABLETRACE
}
#endif
}

BOOL GetBMCoord (LPSTR lpFile,MNMXCORD *Bounds, double *Resolution,LPRECT32 ClipRect,LPHANDLE phTran,BOOL UseCPT)
{
	static	char	SaveAltProj[32];  
	DPOINT	DPoint;
    char    str[132], str2[132], str3[132],TempFile[144]="", ext[6]=".bmp"; 
    LPSTR   lpTXT, pDot;
    HFILE	Fid, FidOut;
    OFSTRUCT	OFStruct;
    double  conversion;  
    int		ncor,n,i;
    float	rv, rv1, rv2, RSQMIN;
    double	WX[32], WY[32], BMPX[32], BMPY[32], Angle, xinc, yinc;
    
//    _fstrcpy(File,lpFile); 
	*phTran = 0;                
    _fstrlwr(lpFile); 
    if ((lpTXT = _fstrstr(lpFile,".tfw")))
    { 
		GSSiGetTempFileName (0,"gm",0,TempFile); 
	    Fid = GSSiOpenFile (lpFile,&OFStruct,OF_READ); 
	    if (FileErrMess (Fid,lpFile,&OFStruct,OF_READ))
	    	return FALSE; 
    	FidOut = GSSiOpenFile (TempFile,&OFStruct,OF_CREATE);  
	    if (FileErrMess (FidOut,TempFile,&OFStruct,OF_CREATE)) 
	    {
	    	GSSiClose (Fid);
	    	return FALSE;
	    } 
    	if (GetGlobalCVal ("[%TFWUNITS]",str,NULL))
	    	fputstring (str,FidOut);
	    else if (PRJ_UNITS[1] == 1) 
	    	fputstring ("FEET",FidOut);
	    else
	    	fputstring ("METERS",FidOut);
		fgetstring (str2,128,Fid);    	
		fgetstring (str2,128,Fid);    	
		fgetstring (str2,128,Fid);    	
		fgetstring (str2,128,Fid);    	
		fgetstring (str3,128,Fid);
		fputstring (str3,FidOut);    	
		fgetstring (str3,128,Fid);
		fputstring (str3,FidOut);    	
		fputstring (str2,FidOut);
		GSSiClose (FidOut);
		GSSiClose (Fid); 
		_fstrcpy (ext,".tif");   	
	    Fid = GSSiOpenFile (TempFile,&OFStruct,OF_READ);  
    }
    else if ((lpTXT = _fstrstr(lpFile,".tif")))
    { 
	    BITMAPINFOHEADER DibInfo; 
	    double	ScaleX, ScaleY;
	    DPOINT	BitmapPoint,WorldPoint;  
	    HDIB32	hDib32;
	    BOOL	st;
        
        hDib32 = BMPHandleFromEXT (lpFile);
        if (!hDib32)
        	goto ErrOut;

		GetBitmapInfoFromHandle (&DibInfo,hDib32);
		st = GetGeoTiffData (hDib32,&ScaleX,&ScaleY,&BitmapPoint, &WorldPoint);
		GMDestroyDIB32 (hDib32); 
		if (!st)
			goto ErrOut;
		GSSiGetTempFileName (0,"gm",0,TempFile); 
    	FidOut = GSSiOpenFile (TempFile,&OFStruct,OF_CREATE);
    	 
    	if (GetGlobalCVal ("[%TFWUNITS]",str,NULL))
	    	fputstring (str,FidOut);
	    else if (PRJ_UNITS[1] == 1) 
	    	fputstring ("FEET",FidOut);
	    else
	    	fputstring ("METERS",FidOut);
		ftoa (str2,WorldPoint.x);    	
		fputstring (str2,FidOut);
		ftoa (str2,WorldPoint.y-ScaleY*(DibInfo.biHeight-1));    	
		fputstring (str2,FidOut);
		ftoa (str2,ScaleX);    	
		fputstring (str2,FidOut);
		_fstrcpy (ext,".tif");   	
		GSSiClose (FidOut);
	    Fid = GSSiOpenFile (TempFile,&OFStruct,OF_READ);  
    } 
    else if ((lpTXT = _fstrstr(lpFile,".bpw")))
    { 
		GSSiGetTempFileName (0,"gm",0,TempFile); 
	    Fid = GSSiOpenFile (lpFile,&OFStruct,OF_READ);  
    	FidOut = GSSiOpenFile (TempFile,&OFStruct,OF_CREATE); 
    	if (GetGlobalCVal ("[%TFWUNITS]",str,NULL))
	    	fputstring (str,FidOut);
	    else if (PRJ_UNITS[1] == 1) 
	    	fputstring ("FEET",FidOut);
	    else
	    	fputstring ("METERS",FidOut);
		fgetstring (str2,128,Fid);    	
		fgetstring (str2,128,Fid);    	
		fgetstring (str2,128,Fid);    	
		fgetstring (str2,128,Fid);    	
		fgetstring (str3,128,Fid);
		fputstring (str3,FidOut);    	
		fgetstring (str3,128,Fid);
		fputstring (str3,FidOut);    	
		fputstring (str2,FidOut);
		GSSiClose (FidOut);
		GSSiClose (Fid); 
		_fstrcpy (ext,".bmp");   	
	    Fid = GSSiOpenFile (TempFile,&OFStruct,OF_READ);  
    } 
    else if (_fstrstr(lpFile,".tmp"))   
	    Fid = GSSiOpenFile (lpFile,&OFStruct,OF_READ);  
    else
    {
	    lpTXT = _fstrstr(lpFile,".txt");
	    if (!lpTXT)
	    {
	     
ErrOut:     MessageBox( GetFocus(),lpFile, "Invalid ortho coordinate file", MB_OK);
	        return (FALSE);
	    } 
	    Fid = GSSiOpenFile (lpFile,&OFStruct,OF_READ);  
	}
    if (Fid == HFILE_ERROR)
    {
        MessageBox( GetFocus(),lpFile, "Unable to open ortho coordinate file", MB_OK);
        return (FALSE);
    } 
    ClipRect->left = -1;
    
	_fstrcpy (SaveAltProj,"[%ALT_PROJECTION]");  
	ExpandText (SaveAltProj);
    SetGlobalValue("%ALT_PROJECTION","baseproj.cvt");
    fgetstring (str,128,Fid); 
    _fstrupr (str);
    ConvertCoordClose ();
	if (_fstrstr (str,".CVT"))
	{
        SetGlobalValue("%ALT_PROJECTION",str);
	    fgetstring (str,128,Fid);
	}
	ConvertCoordInit();
    if (!_fstricmp(str,"CLIP")) 
    	fgetstring (str2,128,Fid);
    else
    	_fstrcpy (str2,str);
    if (!_fstricmp(str2,"FEET")) 
		PRJ_UNITS[3] = 1;
    else if (!_fstricmp(str2,"METERS"))
        PRJ_UNITS[3] = 2;
    else
    {
        MessageBox( GetFocus(), str2,"Invalid Units Parameter", MB_OK);
	    GSSiClose (Fid);
        return (FALSE);
    } 
     
    if (PRJ_UNITS[3] == 1 && PRJ_UNITS[1] == 2)
    	conversion = FTM;
    else if (PRJ_UNITS[3] == 2 && PRJ_UNITS[1] == 1)
    	conversion = MFT;
    else
    	conversion = 1;
    if (!_fstricmp(str,"CLIP")) 
    {
    	fgetstring (str,128,Fid);
	    n=sscanf (str,"%f %f",&rv1, &rv2); 
	    ClipRect->left = max (0,IDNINT (rv1));//conversion needed since vales not pixels bu
	    										  //pixels * FTM
	    ClipRect->bottom = max (0,IDNINT (rv2));   
	    ClipRect->left = IDNINT(rv1);
	    ClipRect->bottom = IDNINT (rv2);   
	    
    	fgetstring (str,128,Fid);
	    n=sscanf (str,"%f %f",&rv1, &rv2);
	    ClipRect->right = IDNINT (rv1);
	    ClipRect->top = IDNINT (rv2);
    	fgetstring (str,128,Fid);
	    sscanf (str,"%i",&ncor);  
	    for (i=0;i<ncor;i++)
	    {
	    	fgetstring (str,128,Fid);
	    	n = sscanf (str,"%lf %lf %lf %lf",&BMPX[i],&BMPY[i],&WX[i],&WY[i]);   
	    	BMPX[i];
	    	BMPY[i];
	    	DPoint.x = WX[i];
	    	DPoint.y = WY[i]; 
	    	ConvertCoord(&DPoint,3,1); 
	    	WX[i] = DPoint.x;
	    	WY[i] = DPoint.y;
	    }
        *phTran = STRAN2 (BMPX,BMPY,WX,WY,ncor,(LPFLOAT)&RSQMIN,1,NULL); 
        Bounds->xmn = ClipRect->left;
        Bounds->ymn = ClipRect->bottom; 
        Bounds->xmx = ClipRect->right;
        Bounds->ymx = ClipRect->top; 
        DPoint = MinMaxMidPointD (Bounds); 
        xinc = DPoint.x - Bounds->xmn;
        yinc = DPoint.y - Bounds->ymn;
        TRANS2 (DPoint.x,DPoint.y,&DPoint.x,&DPoint.y,*phTran); 
        Angle = TranAngle (*phTran,Bounds)/RADDEG;
        *Resolution = TranScale (*phTran,Bounds);
        TranBounds (*phTran,Bounds);
    }
    else
    {
	    fgetstring (str,128,Fid);
	    DPoint.x = atof (str);
	    fgetstring (str,128,Fid);
	    DPoint.y = atof (str);   
	    ConvertCoord(&DPoint,3,1); 
	    Bounds->xmn = DPoint.x;
	    Bounds->ymn = DPoint.y;
	    fgetstring (str,128,Fid);
	    *Resolution = atof (str)*conversion; 
	}
    if (fgetstring (str,128,Fid))
    {   
        Truncate (str); 
        ExpandText (str);
        if (str[0])
        {   
            _fstrcpy (lpFile,str);
            goto Exit;
        }
    }
    *lpTXT = '\0'; 
    _fstrcat(lpFile,ext);
Exit:
    SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
    GSSiClose (Fid); 
    if (*TempFile)
	    GSSiRemove (TempFile);  
    ConvertCoordClose ();
    return (TRUE);
}

HRGN SetOrthoMask (LPSTR MaskAreaFileName)
{	
	HFILE	Fid;
	HANDLE	hPnts=0, Handle;    
	HPDPOINT lpPoints, lpPointsIn;
	HPPOINT	lpNewPoints, lpPntNew;
	DWORD	i, NewPnts, LastPartPoint; 
	short	Attempts=0, iPoly, nPoly=1;  
	BOOL	ReduceNumPoints=TRUE, FirstPartPoint=TRUE, LinkPoint=FALSE;
	POINT	LastPoint, NextPoint, OutPoint;
	double	SkipDist=1;  
	LPWORD	pPartLen; 
	RECT	Rect, NewRect, BoundsRect;
	BOOL	LastOut;
	HBRUSH	hOldBrush=0, hBrush=0;
	HPEN	hOldPen=0, hPen=0, hReturnPen=0; 
	short	ii; 
	RECT	AllPointsRect;   
	BOOL	AreaIsNull = FALSE, AddPoint=FALSE;
	HRGN	hRgn=0;
	
	if (!Display)
		return 0;
	Fid = GSSiOpenFile (MaskAreaFileName,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;  
	BigRead (Fid,(HPSTR)&nPnts,4); 
	hPnts = GSSiGlobAlloc (0,GMEM_MOVEABLE,nPnts*sizeof(DPOINT));
	lpPoints = lpPointsIn = (HPDPOINT)GlobalLock (hPnts); 
	BigRead (Fid,(HPSTR)lpPoints,nPnts*sizeof(DPOINT));
	GSSiClose (Fid);
	if (!SameDPoint (&lpPoints[0],&lpPoints[nPnts-1]))
    	AddPoint = TRUE;
	if (nPnts > MaxDisplayPoints)   
	{
		if (CurrentItemMinMax.xmn/CurView->FileFactor >= CurView->Bounds.xmn &&
			CurrentItemMinMax.ymn/CurView->FileFactor >= CurView->Bounds.ymn &&
			CurrentItemMinMax.xmx/CurView->FileFactor <= CurView->Bounds.xmx &&
			CurrentItemMinMax.ymx/CurView->FileFactor <= CurView->Bounds.ymx)
			Attempts = 1;
		else
			Attempts = 0; 
		SkipDist = 2;
		ReduceNumPoints = TRUE; 
	}
	if (!ReduceNumPoints) 
	{
		if (DoGraphics)
		{
			POINT	FirstPoint,LastPoint;
			long	np=1;   
	
			Handle = GSSiGlobAlloc ( 770,GMEM_MOVEABLE,(nPnts+1) * sizeof(POINT)); 
			lpPntNew = lpNewPoints = (HPPOINT)GlobalLock (Handle);  
			*lpPntNew = BasePtToWinPt (lpPoints++);  
			if (abs(lpPntNew->x) == SHRT_MAX ||
				abs(lpPntNew->y) == SHRT_MAX)
			{   
				GSSiGlobUlFree (&Handle);
				ReduceNumPoints = TRUE; 
				goto DoReduce;
			}
			FirstPoint = LastPoint = *lpPntNew++;
			for (i=1;i<nPnts;i++,lpPoints++)
			{
				*lpPntNew = BasePtToWinPt (lpPoints);   
				if (lpPntNew->x != LastPoint.x || lpPntNew->y != LastPoint.y)
				{   
					LastPoint = *lpPntNew;
					lpPntNew++;
					np++;
				}
			}
			if (np > 1) 
			{   
				if (AddPoint) 
				{
					*lpPntNew++ = FirstPoint;
					np++;
				}
				hRgn = CreatePolygonRgn (lpNewPoints,(short)np,ALTERNATE); 
			}
			GSSiGlobUlFree (&Handle);
		}
	}
	else                                 
	{
DoReduce:   
		BoundsRect = CurView->Rect;
		Handle = GSSiGlobAlloc ( 771,GMEM_MOVEABLE,(long)(nPnts+1) * sizeof(POINT)); 
		lpNewPoints = (HPPOINT)GlobalLock (Handle);  
TryAgain: 
		iPoly = 1;
		LastOut = FALSE;
		LastPartPoint = nPnts-1;
		if (ReduceNumPoints)
			NewPnts = 0;
		else
			NewPnts = nPnts;
		RectInit (&AllPointsRect);  
		lpPoints = lpPointsIn;
		for (i=0,lpPntNew = lpNewPoints;i<nPnts;i++,lpPoints++)
		{
			*lpPntNew = BasePtToWinPt (lpPoints);
			AddPointToRect (*lpPntNew,&AllPointsRect);   
			if (ReduceNumPoints)
			{
				if (LinkPoint) 
				{
					LastPoint = *lpPntNew++;  
					NewPnts++;
					FirstPartPoint = TRUE; 
					LinkPoint = FALSE;
					LastOut = FALSE;
				}
				else if (FirstPartPoint) 
				{
					LastPoint = *lpPntNew++;  
					NewPnts++;
					FirstPartPoint = FALSE; 
					LastOut = FALSE;
				}
				else if (i == LastPartPoint)
				{   
					if (LastOut)
					{
						NewPnts++;
						NextPoint = *lpPntNew;
						*lpPntNew++ = OutPoint;
						*lpPntNew = NextPoint;
					}
					LastPoint = *lpPntNew++;  
					NewPnts++;
					LastOut = FALSE;
					if (iPoly < nPoly)
					{
						pPartLen = (LPWORD)GlobalLock (hPolyPartLen);
						pPartLen += iPoly;
						LastPartPoint = i + *pPartLen;
						if (iPoly > 1)
						{
							LastPartPoint++; 
							LinkPoint = TRUE;
						}
						iPoly++; 
						GlobalUnlock (hPolyPartLen);
					}
				}
				else if (Attempts)
				{   
				
					if (idist (*lpPntNew,LastPoint) > SkipDist)
					{
						NewPnts++;
						LastPoint = *lpPntNew++; 
					}
				} 
				else
				{ 
					Rect.left = min (lpPntNew->x,LastPoint.x);
					Rect.right = max (lpPntNew->x,LastPoint.x);
					Rect.top = min (lpPntNew->y,LastPoint.y);
					Rect.bottom = max (lpPntNew->y,LastPoint.y); 
					if (IntersectRect (&NewRect,&Rect,&BoundsRect))
					{   
						if (LastOut)
						{
							NewPnts++;
							NextPoint = *lpPntNew;
							*lpPntNew++ = OutPoint;
							*lpPntNew = NextPoint;
						}
						NewPnts++;
						LastPoint = *lpPntNew++;
						LastOut = FALSE;
					}
					else
					{
						OutPoint = *lpPntNew;
						LastOut = TRUE;      
					}
				}
			}
			else
				lpPntNew++;	
		} 
		if (NewPnts > MaxDisplayPoints && Attempts < 8)
		{
			SkipDist*=2; 
			Attempts++;    
			goto TryAgain;                            //lpNewPoints[5]
		}
//		if (DoGraphics && IntersectRect (&NewRect,&AllPointsRect,&BoundsRect))
		{
			hRgn = CreatePolygonRgn (lpNewPoints,(short)NewPnts,ALTERNATE); 
		} 
		GSSiGlobUlFree (&Handle);
	} 
	GSSiGlobUlFree (&hPnts);
	return hRgn;
}
                                        // lpNewPoints[2]

BOOL DisplayBMInVP32Ext (HDC hDC,LPSTR BMFile,short vpxin,short vpyin,short vpwin,short vphin)
{   BITMAPFILEHEADER bmfHead;
	BITMAPINFOHEADER	DibInfo;
	LPBITMAPINFOHEADER	pDibInfo=&DibInfo;
//	LPSTR pImage;
	int		i, bmx, bmy, vpx, vpy,  OutWidth, InWidth, OutHeight, vpyp, bmyp;
	long	vpheight, vpwidth, bmwidth, bmheight;   
	long	BMHeight, BMWidth, VPWidth, VPHeight;
	HDC hdcMem;
	HBITMAP	hbmPrev, hNewBM;
	double	BMTopLeftWx, BMTopLeftWy, VPOrigBMx, VPOrigBMy, BMTopLeftVPx, BMTopLeftVPy;
	double	BMBotLeftWx, BMBotLeftWy, BMTopRightWx, BMTopRightWy;
	double	BMBotLeftVPx,BMBotLeftVPy,BMTopRightVPx,BMTopRightVPy;
	double	VPBotLeftBMx, VPBotLeftBMy, VPTopRightBMx, VPTopRightBMy;
    double	VPTopLeftBMx, VPTopLeftBMy;  
    UINT	ColorType=DIB_RGB_COLORS;
	HPALETTE hGSPal;
	RECT	bm, vp;   
	short	rop=0;   
	double	Res=1;
	HDIB32 hDib=LoadDIB32 (BMFile);
    
    if (!hDib)
    	return FALSE;
//	AdjustDIBColorPallet (hDib);    
    if (!GetBitmapInfoFromHandle (&DibInfo,hDib))
		return FALSE;  
	if (CurView->WBounds.xmx <= CurView->WBounds.xmn)
	{
		CurView->WBounds.xmn = CurView->DrawRect.left;
		CurView->WBounds.xmx = CurView->DrawRect.right;
		CurView->WBounds.ymn = CurView->DrawRect.top;
		CurView->WBounds.ymx = CurView->DrawRect.bottom;
	}
	BMWidth = DibInfo.biWidth;
	BMHeight = DibInfo.biHeight;
	{       
		double BMX[4],BMY[4],BASEX[4],BASEY[4]; 
		float	RSQMIN;
		
	    CloseTRANS2 (&hTranBMToBase);
	    CloseTRANS2 (&hTranBaseToBM); 
	    BMX[0]=-1;
	    BMX[1]=-1;
	    BMX[2]=BMWidth;
	    BMX[3]=BMX[2];
	    BMY[0]=-1;
	    BMY[1]=BMHeight;
	    BMY[2]=BMY[1];
	    BMY[3]=-1;
	    BASEX[0]=0-Res;
	    BASEX[1]=BASEX[0];
	    BASEX[2]=BMWidth+Res;
	    BASEX[3]=BASEX[2];
	    BASEY[0]=0-Res;
	    BASEY[1]=BMHeight+Res;
	    BASEY[2]=BASEY[1];
	    BASEY[3]=BASEY[0];
	    hTranBMToBase = STRAN2 (BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
    }

/*    if (DIBColorsArePalleteEntries)
    {
    	ColorType = DIB_PAL_COLORS; 
    } */
//   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
    
    VPWidth = CurView->DrawRect.right - CurView->DrawRect.left +1;
    VPHeight = CurView->DrawRect.bottom - CurView->DrawRect.top+1;
	TRANS2 (0,BMHeight-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBase);
	TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBase);
	TRANS2 (BMBotLeftWx-Res/2,BMBotLeftWy-Res/2,
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToWin);
	TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+Res/2,BMTopRightWy+Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToWin);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-Res/2,BMTopLeftWy+Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToWin);
	if (BMBotLeftVPx < CurView->DrawRect.left)
	{
		bm.left = IDNINT (VPOrigBMx);
		vp.left = CurView->DrawRect.left;
	}
	else
	{
		bm.left = 0;
		vp.left = IDNINT (BMBotLeftVPx);
	}
	if (BMTopRightVPy < CurView->DrawRect.top)
	{
		bm.top = IDNINT (VPTopRightBMy);
		vp.top = CurView->DrawRect.top;
	}
	else
	{
		bm.top = BMHeight-1;
		vp.top = IDNINT (BMTopRightVPy);
	}

	if (BMTopRightVPx > CurView->DrawRect.right)
	{
		bm.right = IDNINT (VPTopRightBMx);
		vp.right = CurView->DrawRect.right;
	}
	else
	{
		bm.right = BMWidth-1;
		vp.right = IDNINT (BMTopRightVPx);
	}
	if (BMBotLeftVPy > CurView->DrawRect.bottom)
	{
		bm.bottom = IDNINT (VPOrigBMy);
		vp.bottom = CurView->DrawRect.bottom;
	}
	else
	{
		bm.bottom = 0;
		vp.bottom = IDNINT (BMBotLeftVPy);
	}
	vpx = vp.left;
	vpy = vp.top;
	bmx = bm.left;
	bmy = bm.top;
	vpwidth = vp.right - vp.left + 1;
	vpheight = vp.bottom - vp.top + 1; 
	vpx = vpxin;
	vpy = vpyin;
	vpwidth = vpwin;
	vpheight = vphin;
	bmheight = bm.top - bm.bottom;// + 1;
	bmwidth = bm.right - bm.left;// + 1;
   	SetStretchBltMode(hDC, StretchMode);   
   	rop = GetGlobalLVal2 ("[%RASTEROPT]",0);  
   	OutWidth = vpwidth; 
   	OutHeight = vpheight;
   	InWidth = bmwidth;  
//   	if (GetGlobalBVal2 ("[%PR]",FALSE) && OutWidth < InWidth)
/*   	if (DoShrinkOrtho && Printing && OutWidth < InWidth)
   	{   
	    BITMAP  bmp; 
	    HDC		hDCMain = GetDC (hWndMain);
	    HDC		hdcMem = CreateCompatibleDC(hDC);
	    HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,OutWidth,OutHeight), hbmPrev;     
	    HDIB	hDIB;
        
        ReleaseDC (hWndMain,hDCMain);
//    	GetObject(hNewBM, sizeof(BITMAP), &bmp);
        hbmPrev = SelectObject(hdcMem, hNewBM);
	    
	   	SetStretchBltMode(hdcMem, StretchMode);   
	   	i=StretchDIBitsFromHandle (hdcMem,0,0,OutWidth,OutHeight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       hDib,1);
//	   				      ColorType,RastOpts[rop]);
//	    i=BitBlt(hDC, vpx,vpy,vpwidth,vpheight, hdcMem, 0, 0,SRCCOPY);
//	    hDIB = BitmapToDIB (hNewBM,NULL);  
		if (hbmPrev)
        	SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem);    
//	    SaveDIB (hDIB,"c:\\test.bmp");
        DeleteObject (hNewBM);        
    }
	else */	
	   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       hDib, ColorType,RastOpts[rop],1);
//	   				      ColorType,RastOpts[rop]);
    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
    nOrthoBlocks++;
// 	GlobalUnlock (hDib);
    DestroyDIB32 (hDib,FALSE); 
	return (TRUE);
} 

BOOL DisplayVirtualPlot (LPSTR VPName)
{
    short	i,j,n=1,tilex,tiley,tilew,tileh; 
    char	Name[128],RLEName[128];

	SetViewport(*pCommandViewport);
    
    tilew = min ((CurView->DrawRect.right - CurView->DrawRect.left)/4,(CurView->DrawRect.bottom - CurView->DrawRect.top)/4);
    tileh = tilew;
    
	for (i=0;i<4;i++)
	{
		tilex = CurView->DrawRect.left;
		tiley = CurView->DrawRect.top + i*tileh;
		for (j=0;j<4;j++)
		{   
			sprintf (Name,"c:\\printest\\band%4.4i.bmp",n);  
			sprintf (RLEName,"c:\\printes2\\band%4.4i.tif",n);  
			SetWindowText (hWndMain,Name); 
/*			{
				HDIB32 hDib=LoadDIB32 (Name); 
				BMPToEXT32 (hDib,RLEName,TIFF_DEFLATE);
			    DestroyDIB32 (hDib,FALSE); 
            }*/
			DisplayBMInVP32Ext (CurView->hDC,Name,tilex,tiley,tilew,tileh);
			tilex += tilew; 
			n++;  
			if (!CheckForContinue(FALSE))
				return FALSE;
		}
	}
	return TRUE;
}


