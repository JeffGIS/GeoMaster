#include "graphint.h"  

#include "gmextern.h"   
#include "dibapi.h"

static short	MaxORTHOBUFS;
static	char	LoadBMPInFile[128], LoadBMPOutFile[128], LoadBMPExt[4]; 
static	DWORD	RastOpts[15]={SRCCOPY,SRCAND,SRCPAINT,SRCINVERT,SRCERASE,NOTSRCCOPY,NOTSRCERASE,MERGECOPY,
	           			  MERGEPAINT,PATCOPY,PATPAINT,PATINVERT,DSTINVERT,BLACKNESS,WHITENESS};


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
		else if (CurOrtho->DeleteBM == 2 && CurOrtho->hDib)
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
	hOrthos = (HANDLE) GSSiGlobAlloc(GAIDNO 1108,GHND,MaxORTHOBUFS*(long)sizeof(ORTHO));
	UsedOrthoBufs = 0;
	CurOrtho = (LPORTHO) GlobalLock (hOrthos);
	for (i=0;i<MaxORTHOBUFS;i++,CurOrtho++)
	{   
		CurOrtho->Frame = -1;
	} 
	GlobalUnlock(hOrthos); 
	return;
}

void CloseOrthos (BOOL Clear)
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
			else if (CurOrtho->DeleteBM == 2 && CurOrtho->hDib)
			{
				if (hDibIs32Bit (CurOrtho->hDib))
					DestroyDIB32(CurOrtho->hDib,FALSE);
				else
					DestroyDIB((HANDLE)CurOrtho->hDib);
			}
		} 
	}
	GlobalUnlock(hOrthos);
	if (Clear)
		GSSiGlobFree (&hOrthos);
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
    static BOOL	Fast=FALSE;

    if (!hOrthos)
    	return;
	InDisplayOrthos = TRUE;
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
				DisplayBMInVP (CurView->hDC, (HANDLE)CurOrtho->hDib, CurOrtho->DIBColorsArePalleteEntries,CurOrtho,Fast);
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
	InDisplayOrthos = FALSE;
	return;
} 

void SetMemDCBackground (HDC hDC,COLORREF WindowColor)
{   
	UINT	x=0,y=0;
	
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

BOOL DisplayBMInVP (HDC hDC, HANDLE hDib, BOOL DIBColorsArePalleteEntries,LPORTHO CurOrtho,BOOL Fast)
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
	extern short StretchMode;

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
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToVP);
	TRANS2 (CurOrtho->Width-1,CurOrtho->Height-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+CurOrtho->Res/2,BMTopRightWy+CurOrtho->Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToVP);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-CurOrtho->Res/2,BMTopLeftWy+CurOrtho->Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToVP);
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
		curProgID = 10020;
		HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,(int)OutWidth,(int)OutHeight), hbmPrev;
		curProgID = -1;
		HDIB	hDIB;
        
        ReleaseDC (hWndMain,hDCMain);
//    	GetObject(hNewBM, sizeof(BITMAP), &bmp);
        hbmPrev = SelectObject(hdcMem, hNewBM);
	    
	   	i=StretchDIBits32 (hdcMem,0,0,OutWidth,OutHeight,
	    				   bmx,bmy-bmheight,//9-15-2005+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				       pDibInfo,
	   				       ColorType,RastOpts[rop],&ZFactor,Fast);
/*	   	SetStretchBltMode(hdcMem, StretchMode);   
	   	i=StretchDIBits (hdcMem,0,0,OutWidth,OutHeight,
	    				   bmx,bmy-bmheight,//9-15-2005+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				      (LPBITMAPINFO)pDibInfo,
	   				      ColorType,RastOpts[rop]);*/
/*	   	i=StretchDIBits (hDC,vpx,vpy,vpwidth,vpheight,
	    				   0,0,OutWidth,OutHeight,
	   				       pImage,
	   				      (LPBITMAPINFO)pDibInfo,
	   				      ColorType,RastOpts[rop]);*/
	    i=BitBlt(hDC, vpx,vpy,vpwidth,vpheight, hdcMem, 0, 0,SRCCOPY);
//	    hDIB = BitmapToDIB (hNewBM,0,NULL);  
		if (hbmPrev)
        	SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem);    
//	    SaveDIB (hDIB,"c:\\temp\\test.bmp");
        DeleteObject (hNewBM);        
    }
	else 	
	   	i=StretchDIBits32 (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight,//9-15-2005+1,
	   				       bmwidth,bmheight,
	   				       pImage,
	   				       pDibInfo,
	   				       ColorType,RastOpts[rop],&ZFactor,Fast);
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
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToVP);
	TRANS2 (CurOrtho->Width-1,CurOrtho->Height-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+CurOrtho->Res/2,BMTopRightWy+CurOrtho->Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToVP);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-CurOrtho->Res/2,BMTopLeftWy+CurOrtho->Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToVP);
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
		curProgID = 10021;
		HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,(int)OutWidth,(int)OutHeight), hbmPrev;
		curProgID = -1;
		if (!hNewBM)
			goto DoNotShrink;
		{
			BITMAP  bmp; 
			HDC		hDCMain = GetDC (hWndMain);
			HDC		hdcMem = CreateCompatibleDC(hDC);
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
    }
	else 
DoNotShrink:		
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

/*BOOL DisplayTranBMFileInVP (HDC hDC,LPSTR BMFile,LPSTR TranFile)
{
	int		i, bmx=0, bmy=0, vpx=0, vpy=0,  OutWidth, InWidth, OutHeight, vpyp, bmyp;
	long	vpheight, vpwidth, bmwidth, bmheight;   
	long	BMHeight, BMWidth, VPWidth, VPHeight;
    UINT	ColorType=DIB_RGB_COLORS;
	RECT	bm, vp;   
	short	rop=0,ii;   
	double	Res=1;
	HDIB32 hDib=LoadDIB32 (BMFile); 
	MNMXCORD	BitmapBounds,WBounds;
	XFORM	xForm;
	HANDLE	hTran;
    
    if (!hDib)
    	return FALSE; 
	if (!GetImageBounds (BMFile,hDib,&BitmapBounds,&WBounds))
		return FALSE;
	hTran = LoadTranFile (TranFile,1,102,0,0);
	if (!hTran)
		return FALSE;
	xForm = SetXFORMFromTRANS (hTran);
	CloseTRANS2 (&hTran);
	SaveDC (hDC);
	SetGraphicsMode(hDC, GM_ADVANCED);
	SetWindowOrgEx  ( hDC, CurView->wOrigX, CurView->wOrigY,0 );
	//SetWindowOrgEx  ( hDC, 0,0,0 );
	SetViewportOrgEx( hDC, CurView->DrawRect.left, CurView->DrawRect.top,0 );
	if (CurView->wExtX &&  CurView->wExtY)
        SetWindowExtEx  ( hDC, CurView->wExtX, CurView->wExtY,0 ); 
	if (CurView->vExtX && CurView->vExtY) 
	{
        if (!SetViewportExtEx( hDC, CurView->vExtX, CurView->vExtY,0 ))
        	ii=1;
	}
	if (!SetMapMode    ( hDC, MM_ISOTROPIC ))
		ii=1;
	SetWorldTransform(hDC, &xForm);
	SetCurImage (BMFile);
	BMWidth = IDNINT(BitmapBounds.xmx+1);
	BMHeight = IDNINT(BitmapBounds.ymx+1);
    VPWidth = BMWidth;
    VPHeight = BMHeight;
   	SetStretchBltMode(hDC, StretchMode);   
   	rop = GetGlobalLVal2 ("[%RASTEROPT]",0);  
   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,
						   VPWidth,VPHeight,
	    				   bmx,bmy,
	   				       BMWidth,BMHeight,
	   				       hDib, ColorType,SRCCOPY,1);
	DestroyDIB32(hDib,FALSE);
	RestoreDC (hDC,-1);
	return (TRUE);
}*/

BOOL DisplayTranBMFileInVP (HDC hDC,LPSTR BMFile,LPSTR TranFile)
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
	HDIB32 hDib=LoadDIB32 (BMFile,TRUE, 0);
	MNMXCORD	BitmapBounds,WBounds;
	LPSTR	pDot;
	HANDLE	hTran;
	XFORM	xForm;
    
    if (!hDib)
    	return FALSE; 
//	AdjustDIBColorPallet (hDib);    
	if (!GetImageBounds (BMFile,hDib,&BitmapBounds,&WBounds))
		return FALSE;
	hTran = LoadTranFile (TranFile,1,102,0,0);
	if (!hTran)
		return FALSE;
	xForm = SetXFORMFromTRANS (hTran);
	CloseTRANS2 (&hTran);
	SetCurImage (BMFile);
	BMWidth = IDNINT(BitmapBounds.xmx+1);
	BMHeight = IDNINT(BitmapBounds.ymx+1);
    CloseTRANS2 (&hTranBMToBase);
    CloseTRANS2 (&hTranBaseToBM); 
	hTranBMToBase = LoadTranFile (TranFile,1,2,0,0);
	hTranBaseToBM = LoadTranFile (TranFile,2,2,0,0);
/*	{       
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
    }*/

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
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToVP);
	TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+Res/2,BMTopRightWy+Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToVP);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-Res/2,BMTopLeftWy+Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToVP);
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
	SaveDC (hDC);
	SetGraphicsMode(hDC, GM_ADVANCED);
	SetWorldTransform(hDC, &xForm);
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
	char LastImageTrnFile[MAX_PATH];
	if (GetGlobalCVal("[%LASTIMAGETRNFILE]", LastImageTrnFile, 0))
	{
		char txt[256];
		GSSiRemove(LastImageTrnFile);
		sprintf(txt, "%ld %d %ld %ld\r\n%ld %ld %ld %ld\r\n%ld %ld %ld %ld\r\n%ld %ld %ld %ld",
			bmx, bmy, vpx, vpy,
			bmx, bmy - bmheight + 1 , vpx, vpy + vpheight,
			bmx + bmwidth, bmy, vpx + vpwidth, vpy,
			bmx + bmwidth, bmy - bmheight + 1, vpx + vpwidth, vpy + vpheight);
		AppendFile(LastImageTrnFile, txt);
	}
	   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       hDib, ColorType,SRCCOPY,1);
//	   				      ColorType,RastOpts[rop]);
	DestroyDIB32(hDib,FALSE);
	RestoreDC (hDC,-1);
	return (TRUE);
}

BOOL DisplayBMFileInVP32(HDC hDC, LPSTR BMFile, double RotationAZ, BOOL fitToVP, int fixedTransparent)
{
//    BITMAPFILEHEADER bmfHead;
	BITMAPINFOHEADER	DibInfo;
	LPBITMAPINFOHEADER	pDibInfo=(LPBITMAPINFOHEADER)&DibInfo;
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
	HDIB32 hDib=LoadDIB32 (BMFile,TRUE, 0);
	HDIB32 hDib32bit;
	MNMXCORD	BitmapBounds,WBounds;
	char	TranFile[MAX_PATH];
	LPSTR	pDot;
	

	strcpy (TranFile,BMFile);
	if ((pDot = strrchr (TranFile,'.')))
	{
		strcpy (pDot,".trn");
		if (ExistFile (TranFile))
			return DisplayTranBMFileInVP (hDC,BMFile,TranFile);
	}
    if (!hDib)
    	return FALSE; 
	if (fixedTransparent < 0)
	{
		hDib32bit = FreeImage_ConvertToGreyscale(hDib);
		DestroyDIB32(hDib, FALSE);
		hDib = hDib32bit;
	}
	if (fixedTransparent)
	{
		hDib32bit = FreeImage_ConvertTo32Bits(hDib);
		DestroyDIB32(hDib, FALSE);
		hDib = hDib32bit;
	}
    if (RotationAZ)
    {   
   		HDIB32	hDibRotated = GMRotateImageClassic (hDib,RotationAZ*DEGRAD);
    		
		DestroyDIB32 (hDib,FALSE); 
		hDib = hDibRotated;
    }
//	AdjustDIBColorPallet (hDib);    
	if (!GetImageBounds (BMFile,hDib,&BitmapBounds,&WBounds))
		return FALSE;
	if (fitToVP)
		WBounds = CurView->WBounds;
	CurView->OrthoRes = (WBounds.xmx - WBounds.xmn)/(BitmapBounds.xmx - BitmapBounds.xmn);
	SetCurImage (BMFile);
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
	    hTranBMToBase = STRAN2 (1640,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (1641,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
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
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToVP);
	TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+Res/2,BMTopRightWy+Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToVP);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-Res/2,BMTopLeftWy+Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToVP);
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
	vpwidth = vp.right - vp.left +1;
	vpheight = vp.bottom - vp.top +1;
	bmheight = bm.top - bm.bottom; + 1;
	bmwidth = bm.right - bm.left; + 1;
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
	GetBitmapInfoFromHandle (&DibInfo,hDib);
	SetDisplayMode(hDC, GF_TEXTMODE);

	if (pDibInfo->biBitCount == 32)
	{
		    float fAlphaFactor;    // used to do premultiply 
			BLENDFUNCTION bf;
    // create a DC for our bitmap -- the source DC for AlphaBlend  
			HDC	hdc = CreateCompatibleDC(hDC);
			BITMAPINFO	bmi;
			RGBQUAD	*pvBits, *pBits;
			LPBYTE	pmaskBits;
			HBITMAP	hbitmap, hbmold;
			UINT	ir,ic;
			int		ubAlpha=255;
			LPBYTE pImageBits;
    // zero the memory for the bitmap info 
			ZeroMemory(&bmi, sizeof(BITMAPINFO));

			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = pDibInfo->biWidth;
			bmi.bmiHeader.biHeight = pDibInfo->biHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = pDibInfo->biWidth * pDibInfo->biHeight * 4;
            fAlphaFactor = (float)ubAlpha / (float)0xff; 

			// create our DIB section and select the bitmap into the dc 
			hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
			pImageBits = FreeImage_GetBits(hDib);
			memcpy(pvBits, pImageBits, pDibInfo->biSizeImage);
			//BitBlt (hdc,0,0,pDibInfo->biWidth,pDibInfo->biHeight,hDC,0,0,SRCCOPY);
			for (ir = 0, pBits = pvBits; ir < pDibInfo->biHeight; ir++)
			{
				if (ir == pDibInfo->biHeight / 2)
					ii = 1;
				for (ic = 0; ic < pDibInfo->biWidth; ic++, pBits++)
				{
					///pBits->rgbReserved = max (BlendFactor,*pmaskBits);
					//if (pBits->rgbReserved)
					//	fAlphaFactor = 0;
					//else
					//	fAlphaFactor = 1;
					//fAlphaFactor = (float)pBits->rgbReserved / (float)0xff; 
					if (pBits->rgbReserved == 0)// && pBits->rgbReserved < 255)
						pBits->rgbReserved = 255;
					fAlphaFactor = (float)pBits->rgbReserved / (float)0xff;

					pBits->rgbBlue *= fAlphaFactor;
					pBits->rgbRed *= fAlphaFactor;
					pBits->rgbGreen *= fAlphaFactor;
				}
			}
			hbmold = SelectObject(hdc, hbitmap);
			if (fixedTransparent)
			{
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.AlphaFormat = 0;
				bf.SourceConstantAlpha = abs (fixedTransparent);
			}
			else
			{
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.AlphaFormat = AC_SRC_ALPHA;
				bf.SourceConstantAlpha = 0xFF;//AlphaBlendFactor;///
			}
			BOOL st = AlphaBlend(hDC,0,0,vpwidth,vpheight, 
						hdc,0,0,pDibInfo->biWidth,pDibInfo->biHeight,bf);
			SelectObject (hdc,hbmold);
		    DeleteObject(hbitmap);
            DeleteDC(hdc);
		}
		else
	   		i=StretchDIBitsFromHandle (hDC,vpx,vpy,
						   vpwidth,vpheight,
	    				   bmx,bmy-bmheight+1,
	   				       bmwidth,bmheight,
	   				       hDib, ColorType,SRCCOPY,1);
//	   				      ColorType,RastOpts[rop]);
    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
    nOrthoBlocks++;
	DestroyDIB32(hDib, FALSE);
	return (TRUE);
}

void ConvertMrSidXY (LPDOUBLE x,LPDOUBLE y)
{
	if (*MrSidProjection)
	{
		DPOINT Point;

		Point.x = *x;
		Point.y = *y;
		ConvertCoord (&Point,1,0);
		*x = Point.x;
		*y = Point.y;
	}
	return;
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
	HDIB32 hImage,ImageHandle=0;
	DWORD st,NumMetaRecords;
	long	Factor=1;  
	short	irec,ii;  
	double	conversion, xFactor, xmx, ymn;
    char	GIFile[MAX_PATH], SaveMAFile[MAX_PATH];  
    LPSTR	pBS;   
    BOOL	rtn=FALSE;  
    short	itilerow,ntilerows,itilecol,ntilecols;
    HANDLE	hTranBMToBaseT=0,hTranBaseToBMT=0;    
    long	iw,ih,MaxSidWidth=1024,MaxSidHeight=1024;
	DWORD	FastCode=(DWORD)0x01000000;
    BOOL	Fast=TRUE;
	BOOL	OrthosAreGray=GetGlobalBVal2 ("[%ORTHOSAREGRAY]",FALSE);

	if (pVP->ConvertToGray)
		OrthosAreGray =  TRUE;
	if (Fast)
		FastCode = 0;
	if (File && !*File)
		return FALSE;
	SaveDC (pVP->hDC);    
	_fstrcpy (SaveMAFile,MaskAreaFile);
	if (File)
	{
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
			if (RegionType == NULLREGION && !PrintingToMF)
			{
				rtn = TRUE;
        		goto Exit;
			}
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
	NumMetaRecords=0;
	for (irec=0;irec<NumMetaRecords;irec++)
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
	}
	BaseDistPerPixel = (pVP->NewBounds.xmx - pVP->NewBounds.xmn)/
	    				   		 ((long)pVP->DrawRect.right - (long)pVP->DrawRect.left);
	xFactor = XRes;   
	if (xFactor < BaseDistPerPixel)
	{
		while (xFactor < BaseDistPerPixel && 1/(double)Factor >= MinMag) 
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
	    hTranBMToBase = STRAN2 (1642,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (1643,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
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
    		MNMXCORD	TileBounds,MrSidTileBounds;
    		
    		TileBounds.xmn = pVP->WBounds.xmn + itilecol * TileWidth;
    		TileBounds.xmx = TileBounds.xmn + TileWidth;
    		TileBounds.ymn = pVP->WBounds.ymn + itilerow * TileHeight;
    		TileBounds.ymx = TileBounds.ymn + TileHeight;
			if (*MrSidProjection)
				ConvertRectCoord (&MrSidTileBounds,&TileBounds,1,0);
			else
				MrSidTileBounds = TileBounds;
			x = max (ULX,MrSidTileBounds.xmn);
			y = min (ULY,MrSidTileBounds.ymx);	  
			ymn = max (MrSidTileBounds.ymn,ULY + (Height-1) * YRes);  
			xmx = min (MrSidTileBounds.xmx,ULX + (Width-1) * XRes); 
			if (x >= xmx || y <= ymn)
				goto NextTile;
			TRANS2 (x,y,&ximage,&yimage,hTranBaseToBM);  
			ximage /= Mag;
			yimage /= Mag;
			TWidth = ((xmx - x)/XRes)/Mag +1/Mag;
			THeight = ((y - ymn)/-YRes)/Mag +1/Mag;
			SidMag=1/Mag;
    		CurView->OrthoRes = XRes;

			hDib = MrSidGetImage (ImageHandle,&ximage,&yimage,&TWidth,&THeight,&SidMag,OrthosAreGray,CurView->HalfTone); 
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
				if (*MrSidProjection)
				{
					BASEY[0]=y;//+(YRes)*Mag;
			//	    BASEY[0]=y+(BMHeight+1)*(YRes)*Mag;
					BASEY[1]=y+(BMHeight)*(YRes)*Mag;
				}
				else
				{
					BASEY[1]=y;//+(YRes)*Mag;
			//	    BASEY[0]=y+(BMHeight+1)*(YRes)*Mag;
					BASEY[0]=y+(BMHeight)*(YRes)*Mag;
				}
			    BASEY[2]=BASEY[1];
			    BASEY[3]=BASEY[0];
			    hTranBMToBaseT = STRAN2 (1644,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
			    hTranBaseToBMT = STRAN2 (1645,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
		    }
		
		/*    if (DIBColorsArePalleteEntries)
		    {
		    	ColorType = DIB_PAL_COLORS; 
		    } */
		//   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
		    
		//    VPWidth = (long)pVP->DrawRect.right - (long)pVP->DrawRect.left ;
		//    VPHeight = (long)pVP->DrawRect.bottom - (long)pVP->DrawRect.top;
			if (*MrSidProjection)
			{
				XFORM	xForm;
				double	bmx[4], bmy[4], vpx[4], vpy[4];
				double	wx[4], wy[4];
				HANDLE	hTran;

				wx[0] = TileBounds.xmn;
				wy[0] = TileBounds.ymn;
				wx[1] = TileBounds.xmn;
				wy[1] = TileBounds.ymx;
				wx[2] = TileBounds.xmx;
				wy[2] = TileBounds.ymx;
				wx[3] = TileBounds.xmx;
				wy[3] = TileBounds.ymn;
				for (i=0;i<4;i++)
				{
					TRANS2 (wx[i],wy[i],&vpx[i],&vpy[i],pVP->hTranBaseToVP);
					ConvertMrSidXY (&wx[i],&wy[i]);
					TRANS2 (wx[i],wy[i],&bmx[i],&bmy[i],hTranBaseToBMT);
				}
				hTran = STRAN2 (1646,bmx,bmy,vpx,vpy,4,&RSQMIN,1,NULL);
				
				xForm = SetXFORMFromTRANS (hTran);
				SaveDC (pVP->hDC);
			    SetDisplayMode (pVP->hDC, GF_TEXTMODE);  
				SetGraphicsMode(pVP->hDC, GM_ADVANCED);
				SetWorldTransform(pVP->hDC, &xForm);
				SetMapMode (pVP->hDC, MM_ISOTROPIC );
			    CloseTRANS2 (&hTran);
				i=StretchDIBitsFromHandle (pVP->hDC,0,0,BMWidth,BMHeight,
													0,0,BMWidth,BMHeight,
//					   	i=StretchDIBitsFromHandle (pVP->hDC,vpx[0],vpy[1],vpx[2]-vpx[0],fabs(vpy[1]-vpy[0]),
//															bmx[0],bmy[0],bmx[2]-bmx[0],fabs(bmy[1]-bmy[0]),
					    		   //bmxz,bmyz-bmheightz+1,
								   //BMRect.right - BMRect.left,BMRect.bottom -BMRect.top,
					   			   //bmwidthz,bmheightz,
					   			   hDib,ColorType,RastOpts[rop]+FastCode, 1);   //
				goto NextTile;
			}
			TRANS2 (0,BMHeight-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBaseT);
			TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBaseT);
			TRANS2 (BMBotLeftWx,BMBotLeftWy,
					&BMBotLeftVPx,&BMBotLeftVPy,pVP->hTranBaseToVP);
			TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
					&BMTopRightWy,hTranBMToBaseT);
			TRANS2 (BMTopRightWx,BMTopRightWy,
					&BMTopRightVPx,&BMTopRightVPy,pVP->hTranBaseToVP);
			TRANS2 (pVP->NewBounds.xmn,pVP->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
				    hTranBaseToBMT);
			TRANS2 (pVP->NewBounds.xmx,pVP->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
				    hTranBaseToBMT);
			TRANS2 (BMTopLeftWx,BMTopLeftWy,
					&BMTopLeftVPx,&BMTopLeftVPy,pVP->hTranBaseToVP);
			if (BMBotLeftVPx < pVP->DrawRect.left)
			{
				bm.left = IDNINT (VPOrigBMx);
				vp.left = pVP->DrawRect.left;
			}
			else
			{
				bm.left = 0;
				vp.left = IDNINT (BMBotLeftVPx - 0.25);
			}
			if (BMTopRightVPy < pVP->DrawRect.top)
			{
				bm.top = IDNINT (VPTopRightBMy);
				vp.top = pVP->DrawRect.top;
			}
			else
			{
				bm.top = BMHeight;
				vp.top = IDNINT (BMTopRightVPy - 0.25);
			}
		
			if (BMTopRightVPx > pVP->DrawRect.right)
			{
				bm.right = IDNINT (VPTopRightBMx);
				vp.right = pVP->DrawRect.right;
			}
			else
			{
				bm.right = BMWidth;
				vp.right = IDNINT (BMTopRightVPx  + 0.25);
			}
			if (BMBotLeftVPy > pVP->DrawRect.bottom)
			{
				bm.bottom = IDNINT (VPOrigBMy);
				vp.bottom = pVP->DrawRect.bottom;
			}
			else
			{
				bm.bottom = 0;
				vp.bottom = IDNINT (BMBotLeftVPy + 0.25);
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
//			SelectClipRgn (hDC,0);
			   	if (Printing && DoShrinkOrtho && OutWidth < InWidth)
			   	{   
					curProgID = 10022;
					HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,OutWidth,OutHeight), hbmPrev;
					curProgID = -1;

					if (!hNewBM)
						goto DoNotShrink;
					{
						BITMAP  bmp; 
						HDC		hDCMain = GetDC (hWndMain);
						HDC		hdcMem = CreateCompatibleDC(hDC);
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
				   						   hDib,ColorType,RastOpts[rop]+FastCode, 1);
				//	   				      ColorType,RastOpts[rop]);
						i=BitBlt(hDC, vpx,vpy,(int)vpwidth,(int)vpheight, hdcMem, 0, 0,SRCCOPY);
				//	    hDIB = BitmapToDIB (hNewBM,NULL);  
						if (hbmPrev)
			        		SelectObject(hdcMem, hbmPrev);
						DeleteDC(hdcMem);    
				//	    SaveDIB (hDIB,"c:\\test.bmp");
						DeleteObject (hNewBM);  
					}
			    }
				else 	  
				{
DoNotShrink:
					   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,vpwidth,vpheight,
										   BMRect.left,BMRect.top,
					    				   //bmxz,bmyz-bmheightz+1,
										   BMRect.right - BMRect.left,BMRect.bottom -BMRect.top,
					   				       //bmwidthz,bmheightz,
					   				       hDib,ColorType,RastOpts[rop]+FastCode, 1);   //
//				   				      ColorType,RastOpts[rop]); 
				}  
			}
		    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
		    nOrthoBlocks++;
		// 	GlobalUnlock (hDib);
		    DestroyDIB32 (hDib,FALSE); 
		    rtn = TRUE;  
NextTile:
		    CloseTRANS2 (&hTranBMToBaseT);
		    CloseTRANS2 (&hTranBaseToBMT); 
		    if (ntilerows > 1 && !CheckForContinue (FALSE,0))
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

BOOL DisplaySIDInVP32new (LPVIEWPORT pVP,LPSTR File)
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
	HDIB32 hImage,ImageHandle=0;
	DWORD st,NumMetaRecords;
	long	Factor=1;  
	short	irec,ii;  
	double	conversion, xFactor, xmx, ymn;
    char	GIFile[MAX_PATH], SaveMAFile[MAX_PATH];  
    LPSTR	pBS;   
    BOOL	rtn=FALSE;  
    short	itilerow,ntilerows,itilecol,ntilecols;
    HANDLE	hTranBMToBaseT=0,hTranBaseToBMT=0;    
    long	iw,ih,MaxSidWidth=1024,MaxSidHeight=1024;
	DWORD	FastCode=(DWORD)0x01000000;
    BOOL	Fast=TRUE;
	BOOL	OrthosAreGray=GetGlobalBVal2 ("[%ORTHOSAREGRAY]",FALSE);
	MNMXCORD	VPBounds;
	char	Projection[64]="";
    
	if (pVP->ConvertToGray)
		OrthosAreGray =  TRUE;
	if (Fast)
		FastCode = 0;
	if (File && !*File)
		return FALSE;
	SaveDC (pVP->hDC);    
	_fstrcpy (SaveMAFile,MaskAreaFile);
	SetGlobalValue("%LAYER_PROJECTION",Projection);
	if (File)
	{
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
			if (RegionType == NULLREGION && !PrintingToMF)
			{
				rtn = TRUE;
        		goto Exit;
			}
		}
	}
	
    GetGlobalCVal ("[%LAYER_PROJECTION]",Projection,0);
	if (*Projection)
	{
		char	SaveAltProj[MAX_PATH];

		GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,0);
		SetGlobalValue("%ALT_PROJECTION",Projection);
		ConvertCoordClose ();
		ConvertRectCoord (&VPBounds,&CurView->WBounds,3,0);
		SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
	}
	else
		VPBounds = CurView->WBounds;
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
	NumMetaRecords=0;
	for (irec=0;irec<NumMetaRecords;irec++)
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
	}
	BaseDistPerPixel = (VPBounds.xmx - VPBounds.xmn)/
	    				   		 ((long)pVP->DrawRect.right - (long)pVP->DrawRect.left);
	xFactor = XRes;   
	if (xFactor < BaseDistPerPixel)
	{
		while (xFactor < BaseDistPerPixel && 1/(double)Factor >= MinMag) 
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
	    hTranBMToBase = STRAN2 (1647,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (1648,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
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
    TileWidth = (VPBounds.xmx - VPBounds.xmn) / ntilecols;
    TileHeight = (VPBounds.ymx - VPBounds.ymn) / ntilerows;
    for (itilerow=0;itilerow<ntilerows;itilerow++)
    {
    	for (itilecol=0;itilecol<ntilecols;itilecol++)
    	{   
    		MNMXCORD	TileBounds;
    		
    		TileBounds.xmn = VPBounds.xmn + itilecol * TileWidth;
    		TileBounds.xmx = TileBounds.xmn + TileWidth;
    		TileBounds.ymn = VPBounds.ymn + itilerow * TileHeight;
    		TileBounds.ymx = TileBounds.ymn + TileHeight;
			if (*MrSidProjection)
			{
				char	SaveAltProj[MAX_PATH];
				MNMXCORD	TempBounds=TileBounds;

				ConvertCoordClose ();
				LoadProjection(0,MrSidProjection); 
				ConvertRectCoord (&TileBounds,&TempBounds,1,0);
			}
			x = max (ULX,TileBounds.xmn);
			y = min (ULY,TileBounds.ymx);	  
			ymn = max (TileBounds.ymn,ULY + (Height-1) * YRes);  
			xmx = min (TileBounds.xmx,ULX + (Width-1) * XRes); 
			if (x >= xmx || y <= ymn)
				goto NextTile;
			TRANS2 (x,y,&ximage,&yimage,hTranBaseToBM);  
			ximage /= Mag;
			yimage /= Mag;
			TWidth = ((xmx - x)/XRes)/Mag +1/Mag;
			THeight = ((y - ymn)/-YRes)/Mag +1/Mag;
			SidMag=1/Mag;
    		CurView->OrthoRes = XRes;

			hDib = MrSidGetImage (ImageHandle,&ximage,&yimage,&TWidth,&THeight,&SidMag,OrthosAreGray,CurView->HalfTone); 
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
			    hTranBMToBaseT = STRAN2 (1649,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
			    hTranBaseToBMT = STRAN2 (1650,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
		    }
		
		/*    if (DIBColorsArePalleteEntries)
		    {
		    	ColorType = DIB_PAL_COLORS; 
		    } */
		//   	pImage =(LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
		    
		//    VPWidth = (long)pVP->DrawRect.right - (long)pVP->DrawRect.left ;
		//    VPHeight = (long)pVP->DrawRect.bottom - (long)pVP->DrawRect.top;
			if (*MrSidProjection)
			{
				XFORM	xForm;
				double	bmx[4], bmy[4], vpx[4], vpy[4];
				double	wx[4], wy[4];
				HANDLE	hTran;

				vpx[0] = pVP->DrawRect.left;
				vpx[1] = pVP->DrawRect.left;
				vpx[2] = pVP->DrawRect.right;
				vpx[3] = pVP->DrawRect.right;
				vpy[0] = pVP->DrawRect.bottom;
				vpy[1] = pVP->DrawRect.top;
				vpy[2] = pVP->DrawRect.top;
				vpy[3] = pVP->DrawRect.bottom;
				wx[0] = pVP->WBounds.xmn;
				wy[0] = pVP->WBounds.ymn;
				wx[1] = pVP->WBounds.xmn;
				wy[1] = pVP->WBounds.ymx;
				wx[2] = pVP->WBounds.xmx;
				wy[2] = pVP->WBounds.ymx;
				wx[3] = pVP->WBounds.xmx;
				wy[3] = pVP->WBounds.ymn;
				for (i=0;i<4;i++)
				{
					ConvertMrSidXY (&wx[i],&wy[i]);
					TRANS2 (wx[i],wy[i],&bmx[i],&bmy[i],hTranBaseToBMT);
				}
				hTran = STRAN2 (1651,bmx,bmy,vpx,vpy,4,&RSQMIN,1,NULL);
				
				xForm = SetXFORMFromTRANS (hTran);
				SaveDC (pVP->hDC);
				SetGraphicsMode(pVP->hDC, GM_ADVANCED);
				SetWorldTransform(pVP->hDC, &xForm);
			    CloseTRANS2 (&hTran);
			}
			TRANS2 (0,BMHeight-1,&BMTopLeftWx,&BMTopLeftWy,hTranBMToBaseT);
			ConvertMrSidXY (&BMTopLeftWx,&BMTopLeftWy);
			TRANS2 (0,0,&BMBotLeftWx,&BMBotLeftWy,hTranBMToBaseT);
			ConvertMrSidXY (&BMBotLeftWx,&BMBotLeftWy);
			TRANS2 (BMBotLeftWx,BMBotLeftWy,
					&BMBotLeftVPx,&BMBotLeftVPy,pVP->hTranBaseToVP);
			TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
					&BMTopRightWy,hTranBMToBaseT);
			ConvertMrSidXY (&BMTopRightWx,&BMTopRightWy);
			TRANS2 (BMTopRightWx,BMTopRightWy,
					&BMTopRightVPx,&BMTopRightVPy,pVP->hTranBaseToVP);
			TRANS2 (TileBounds.xmn,TileBounds.ymn,&VPOrigBMx,&VPOrigBMy,
				    hTranBaseToBMT);
			TRANS2 (TileBounds.xmx,TileBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
				    hTranBaseToBMT);
			TRANS2 (BMTopLeftWx,BMTopLeftWy,
					&BMTopLeftVPx,&BMTopLeftVPy,pVP->hTranBaseToVP);
			if (BMBotLeftVPx < pVP->DrawRect.left)
			{
				bm.left = IDNINT (VPOrigBMx);
				vp.left = pVP->DrawRect.left;
			}
			else
			{
				bm.left = 0;
				vp.left = IDNINT (BMBotLeftVPx - 0.25);
			}
			if (BMTopRightVPy < pVP->DrawRect.top)
			{
				bm.top = IDNINT (VPTopRightBMy);
				vp.top = pVP->DrawRect.top;
			}
			else
			{
				bm.top = BMHeight;
				vp.top = IDNINT (BMTopRightVPy - 0.25);
			}
		
			if (BMTopRightVPx > pVP->DrawRect.right)
			{
				bm.right = IDNINT (VPTopRightBMx);
				vp.right = pVP->DrawRect.right;
			}
			else
			{
				bm.right = BMWidth;
				vp.right = IDNINT (BMTopRightVPx  + 0.25);
			}
			if (BMBotLeftVPy > pVP->DrawRect.bottom)
			{
				bm.bottom = IDNINT (VPOrigBMy);
				vp.bottom = pVP->DrawRect.bottom;
			}
			else
			{
				bm.bottom = 0;
				vp.bottom = IDNINT (BMBotLeftVPy + 0.25);
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
//			SelectClipRgn (hDC,0);
			   	if (Printing && DoShrinkOrtho && OutWidth < InWidth)
			   	{   
					curProgID = 10023;
					HBITMAP	hNewBM = CreateCompatibleBitmap(hDC,OutWidth,OutHeight), hbmPrev;
					curProgID = -1;
					if (!hNewBM)
						goto DoNotShrink;
					{
						BITMAP  bmp; 
						HDC		hDCMain = GetDC (hWndMain);
						HDC		hdcMem = CreateCompatibleDC(hDC);
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
				   						   hDib,ColorType,RastOpts[rop]+FastCode, 1);
				//	   				      ColorType,RastOpts[rop]);
						i=BitBlt(hDC, vpx,vpy,(int)vpwidth,(int)vpheight, hdcMem, 0, 0,SRCCOPY);
				//	    hDIB = BitmapToDIB (hNewBM,NULL);  
						if (hbmPrev)
			        		SelectObject(hdcMem, hbmPrev);
						DeleteDC(hdcMem);    
				//	    SaveDIB (hDIB,"c:\\test.bmp");
						DeleteObject (hNewBM);  
					}
			    }
				else 	  
				{
DoNotShrink:
					   	i=StretchDIBitsFromHandle (hDC,vpx,vpy,vpwidth,vpheight,
										   BMRect.left,BMRect.top,
					    				   //bmxz,bmyz-bmheightz+1,
										   BMRect.right - BMRect.left,BMRect.bottom -BMRect.top,
					   				       //bmwidthz,bmheightz,
					   				       hDib,ColorType,RastOpts[rop]+FastCode, 1);   //
				//	   				      ColorType,RastOpts[rop]); 
				}  
			}
		    nOrthoBytes += (long) bmwidth * (long) bmheight * 3;
		    nOrthoBlocks++;
		// 	GlobalUnlock (hDib);
		    DestroyDIB32 (hDib,FALSE); 
		    rtn = TRUE;  
NextTile:
		    CloseTRANS2 (&hTranBMToBaseT);
		    CloseTRANS2 (&hTranBaseToBMT); 
		    if (ntilerows > 1 && !CheckForContinue (FALSE,0))
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

 

HANDLE CreateGrayScalePallette (void)
{   
#define	NUMENTRIES	256  
	LPLOGPALETTE	plgpl;
	HPALETTE hpal;
	PALETTEENTRY ape[NUMENTRIES]; 
	UINT	i;
	BYTE	red, green, blue; 
	HANDLE	handle;

	handle = GSSiGlobAlloc(GAIDNO 1111,GMEM_MOVEABLE,sizeof(LOGPALETTE) + NUMENTRIES * sizeof(PALETTEENTRY));
    
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
	GSSiGlobUlFree (&handle); 
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
		char	DriveID[4]={(char)('A'+i),':','\\',0};	
		short	DriveType = GetDriveType (DriveID);
		if (DriveType > DRIVE_NO_ROOT_DIR)
		{   
//			if (GetVolumeLabel(i,VolLabel))
			{
				sprintf (str," [ %s ]",DriveID);	
				SendDlgItemMessage (hWndDlg,LBCNTL,LB_ADDSTRING,0,(LPARAM)str);  
			}
		}
	}
	if (!GetDlgItemText (hWndDlg,CurrentDirCNTL,str,255))
		return 0; 
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
	hDir = SearchDirectory32 (str,0,&Type,0);
	while (hDir)
	{   
		if (Type && *str != '.') 
		{
			char	str2[256];
			
			sprintf (str2," [%s]",str);
			SendDlgItemMessage (hWndDlg,LBCNTL,LB_ADDSTRING,0,(LPARAM)str2); 
		}
		hDir = SearchDirectory32 (str,hDir,&Type,0);
	}
	GetDlgItemText (hWndDlg,CurrentDirCNTL,str,255); 
	if (*LastChr(str) != '\\')
		_fstrcat (str,"\\"); 
	_fstrcat (str,cType);
	hDir = SearchDirectory32 (str,0,&Type,0);
	while (hDir)
	{   
		if (!Type)
			SendDlgItemMessage (hWndDlg,LBCNTL,LB_ADDSTRING,0,(LPARAM)str);
		hDir = SearchDirectory32 (str,hDir,&Type,0);
	}
	return 0;
} 




BOOL CreateMapIndex (LPSTR Dir,LPSTR FileListName,short itype,BOOL UsesTimes,LPSTR ImageQuality,LPSTR cOrthResList,BOOL Compress,short  SizeOpt,
					 HWND hWndDlg,UINT MESSCntl, UINT STATUSCntl,BOOL UseCPT)
{
	char Mess[144], ToDir[128],str[260],File[132];
    HANDLE  Handle, hlpFI=0;
	BOOL	CurHeaderWritten;  
	HFILE   FidIndex, FidFileList, Fid;
    LPFILEINDEX lpFI;
    OFSTRUCTGM    OFStruct;
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
		setDoPaint( FALSE);
		_fstrcpy (ToDir,Dir);
		ExpandText (ToDir); 
	 	makedirectories (ToDir,TRUE,FALSE);
		if (*ImageQuality)  
		 	UserDefinedImageQuality = atoi(ImageQuality) * 100; 
		else
			UserDefinedImageQuality = 7200; 
		sprintf (Index,"%s\\index%s",ToDir,cOrthRes);
			                 
		FidIndex = GSSiOpenFile (Index,(LPOFSTRUCTGM) &OFStruct,OF_CREATE); 
			
		hlpFI = GSSiGlobAlloc(GAIDNO 1744,GHND,sizeof(FILEINDEX)+sizeof(FILEINDEXENTRY)); 
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
				if (!ProcessDelimTextHeader(str, NULL, FidFileList, &hDLT, 0, 0))
					goto Exit;
			                    	
		    	while (fgetstring (str,128,FidFileList))
		    	{   
		    		char	leaf[34]="[FILENAME]";
				    GetDelimTextData(str,hDLT,128); 
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
							ProcessDelimTextHeader(skipfile, NULL, FidSkip, &hDLTSkip, 0, 0);
							while (fgetstring (skipfile,128,FidSkip))
							{
								GetDelimTextData(skipfile,hDLTSkip,128); 
								_fstrcpy (skipfile,"[SKIPNAME]");
								ExpandText (skipfile);
								if (!_fstricmp (skipfile,leaf))
								{  
						    		GSSiClose2 (&FidSkip); 
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
				    		GSSiClose2 (&FidSkip);
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
		        GSSiClose2 (&FidFileList);  
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
		 GSSiClose2 (&FidIndex);  
		 GSSiGlobUlFree (&hlpFI);
		 if (Compress)
		    AVIOutClose(&hAVIFile);
		 DisableHalt = FALSE; 
		 setDoPaint( TRUE); 
		 CurLev++;
	} while (*pOrthRes); 
	GSSiClose2 (&Fid);
	if (itype == 1 && hWndDlg)
	{  
		long	lmem=sizeof(VIEWPORT)+128*MAX_VIEWPORT_FILES; 
		HANDLE	hVP=GSSiGlobAlloc(GAIDNO 1117,GHND,lmem);
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
		hSymDesc = GSSiGlobAlloc(GAIDNO 1118,GMEM_MOVEABLE,USHRT_MAX);
		pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
		while (SendDlgItemMessage (hWndDlg,SYM_VIS_LB,LB_GETTEXT,NumSyms++,(LPARAM)str)!=LB_ERR) 
		{   
			pSymDesc->Handle = GSSiGlobAlloc(GAIDNO 1119,GHND,sizeof(SYMBOL));
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
			pSymDesc->Handle = GSSiGlobAlloc(GAIDNO 1120,GHND,sizeof(SYMBOL));
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
		GSSiGlobUlFree (&hVP); 
		GSSiClose2 (&FidSyms);
	}
	Processing = FALSE;
	SetContinueProcessing ( TRUE);  
                 
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
		
		hAreaPts = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,USHRT_MAX);
		AreaPoints = (LPDPOINT)GlobalLock (hAreaPts);
		if (FidArea != HFILE_ERROR)
		{   
			while (fgetstring (str,128,FidArea))
				if (sscanf (str,"%lf %lf",&AreaPoints[nAreaPts].x,&AreaPoints[nAreaPts].y) == 2)
					nAreaPts++;  
			GSSiClose2 (&FidArea);

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
		GSSiClose2 (&Fid);
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
	BOOL	rtn=FALSE;
	FARPROC lpfnCmpImageMsgProc; 
	HFILE	Fid; 
	long	len;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD	CRFlags=0;
	//char	modulePath[MAX_PATH]="C:\\Gssi\\prog\\cmpframe\\Release\\cmpframe.exe";
	char	modulePath[MAX_PATH]="[%DL]cmpframe.exe";
	char	cmd[MAX_PATH*2+128];
	
	ExpandText (modulePath);
	if (!*CmpImageInFile)
	{
		GSSiGetTempFileName (0,"gma",0,CmpImageInFile);
		GSSiGetTempFileName (0,"gmb",0,CmpImageOutFile);
	}  
	CmpImageQuality = Quality;
	Fid = GSSiOpenFile (CmpImageInFile,NULL,OF_CREATE);
	len = lpbiIn->biSize + lpbiIn->biClrUsed * sizeof(RGBQUAD) + lpbiIn->biSizeImage; 
	BigWrite (Fid,(HPSTR)&len,4,-1);
	BigWrite (Fid,(HPSTR)lpbiIn,len,-1); 
	GSSiClose2 (&Fid);
	CloseAllRequestedFiles (FALSE);

    sprintf (cmd,"CMPFrame %s;%s;%ld",CmpImageInFile,CmpImageOutFile,CmpImageQuality);
   	ExpandText (cmd);
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) ); 
	CRFlags = 0;//DETACHED_PROCESS;
	if (CreateProcess(modulePath,cmd, 
						NULL,             // Process handle not inheritable. 
						NULL,             // Thread handle not inheritable. 
						FALSE,            // Set handle inheritance to FALSE. 
						CRFlags,		  // creation flags. 
						NULL,             // Use parent's environment block. 
						NULL,             // Use parent's starting directory. 
						&si,              // Pointer to STARTUPINFO structure.
						&pi ))             // Pointer to PROCESS_INFORMATION structure.
	{
			BOOL TimedOut;
			int	 MaxWait = 10000;
			DWORD	ProcessID = GetProcessId(pi.hProcess);

			//Wait (1000);

			WaitForInputIdle (pi.hProcess,INFINITE);
			//hWnd = FindWindowByProcessID (ProcessID,"");
			//ltoa ((long)hWnd,OutLoc,10);
			//if (atob (Arg[5]))
			{
				while (WaitForProcessToEnd (pi.dwProcessId,&MaxWait));
			}
			rtn = TRUE;
	}


	if (rtn)
	{
		Fid = GSSiOpenFile (CmpImageOutFile,NULL,OF_READ);
		if (Fid == HFILE_ERROR)
			rtn = FALSE;
		else
		{
			BigRead (Fid,(HPSTR)&len,4);
			BigRead (Fid,(HPSTR)lpbiOut,len); 
			GSSiClose2 (&Fid);
		}
    }
	return rtn;
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
	static	char	SaveAltProj[MAX_PATH];
	DPOINT	DPoint;
    char    str[132], str2[132], str3[132],TempFile[MAX_PATH]="", ext[6]=".bmp"; 
    LPSTR   lpTXT, pDot;
    HFILE	Fid, FidOut;
	OFSTRUCTGM	OFStruct;
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
	    	GSSiClose2 (&Fid);
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
		GSSiClose2 (&FidOut);
		GSSiClose2 (&Fid); 
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
		st = GetGeoTiffData (hDib32,&ScaleX,&ScaleY,&BitmapPoint, &WorldPoint,FALSE);
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
		GSSiClose2 (&FidOut);
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
		GSSiClose2 (&FidOut);
		GSSiClose2 (&Fid); 
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
	    GSSiClose2 (&Fid);
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
        *phTran = STRAN2 (1652,BMPX,BMPY,WX,WY,ncor,(LPFLOAT)&RSQMIN,1,NULL); 
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
    GSSiClose2 (&Fid); 
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
	hPnts = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,nPnts*sizeof(DPOINT));
	lpPoints = lpPointsIn = (HPDPOINT)GlobalLock (hPnts); 
	BigRead (Fid,(HPSTR)lpPoints,nPnts*sizeof(DPOINT));
	GSSiClose2 (&Fid);
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
	
			Handle = GSSiGlobAlloc(GAIDNO 770,GMEM_MOVEABLE,(nPnts+1) * sizeof(POINT)); 
			lpPntNew = lpNewPoints = (HPPOINT)GlobalLock (Handle);  
			*lpPntNew = BasePtToWinPt (lpPoints++);  
			if (abs(lpPntNew->x) == INT_MAX ||
				abs(lpPntNew->y) == INT_MAX)
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
		Handle = GSSiGlobAlloc(GAIDNO 771,GMEM_MOVEABLE,(long)(nPnts+1) * sizeof(POINT)); 
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
	HDIB32 hDib=LoadDIB32 (BMFile,TRUE, 0);
    
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
	    hTranBMToBase = STRAN2 (1653,BMX,BMY,BASEX,BASEY,4,&RSQMIN,1,NULL);
	    hTranBaseToBM = STRAN2 (1654,BASEX,BASEY,BMX,BMY,4,&RSQMIN,1,NULL);
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
			&BMBotLeftVPx,&BMBotLeftVPy,CurView->hTranBaseToVP);
	TRANS2 (BMWidth-1,BMHeight-1,&BMTopRightWx,
			&BMTopRightWy,hTranBMToBase);
	TRANS2 (BMTopRightWx+Res/2,BMTopRightWy+Res/2,
			&BMTopRightVPx,&BMTopRightVPy,CurView->hTranBaseToVP);
	TRANS2 (CurView->NewBounds.xmn,CurView->NewBounds.ymn,&VPOrigBMx,&VPOrigBMy,
		    hTranBaseToBM);
	TRANS2 (CurView->NewBounds.xmx,CurView->NewBounds.ymx,&VPTopRightBMx,&VPTopRightBMy,
		    hTranBaseToBM);
	TRANS2 (BMTopLeftWx-Res/2,BMTopLeftWy+Res/2,
			&BMTopLeftVPx,&BMTopLeftVPy,CurView->hTranBaseToVP);
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
			if (!CheckForContinue(FALSE,0))
				return FALSE;
		}
	}
	return TRUE;
}
FILEINDEXENTRY CopyIndexEntry(LPFILEINDEXENTRY pIndexEntry)
{
	FILEINDEXENTRY indexEntryOut = { 0 };
	if (pIndexEntry)
	{
		indexEntryOut.BMBitCount = pIndexEntry->BMBitCount;
		indexEntryOut.BMHeight = pIndexEntry->BMHeight;
		indexEntryOut.BMWidth = pIndexEntry->BMWidth;
		indexEntryOut.Bounds = pIndexEntry->Bounds;
		indexEntryOut.Len = pIndexEntry->Len;
		strcpy(indexEntryOut.Name, pIndexEntry->Name);
	}
	return indexEntryOut;
}
BOOL ConvertOrthoToJP2 (LPSTR Name,LPSTR NewName,int fmt)
{
	BOOL rtn = FALSE;

    OFSTRUCTGM    OFStruct; 
    short     Version;
    HFILE   FidIndex, FidIndexOut, FidJP2Out;
    long    Signature, EndOffset;
    HANDLE  Handle;
    LPFILEINDEX lpIndex;
    FILEINDEX FirstIndex;   
    MNMXCORD    TestBounds;
	MNMXCORD	FileBounds;
    char        File[MAX_PATH], drive[8], dir[MAX_PATH], leaf[40], IndexRes[16]; 
	char		GCIFile[MAX_PATH], JP2File[MAX_PATH];
    short	SaveUnits;
	LPSTR	pBS;
	int		totLen, indexLoc, curLoc, curNumFiles;
	int	JP2CompressionFactor = GetGlobalLVal2 ("[%JP2Factor]",16);

     _fstrcpy (File,Name);
    ExpandText (File);
    _splitpath (File,drive,dir,leaf,0); 
    if (!_fstrnicmp (leaf,"index",5))
    	_fstrcpy (IndexRes,&leaf[5]);
    else
    	IndexRes[0]=0;
    FidIndex = GSSiOpenFile (Name,(LPOFSTRUCTGM) &OFStruct,OF_READ);
    if (FidIndex == HFILE_ERROR)
       return rtn; 
	FidIndexOut = GSSiOpenFile (NewName,0,OF_CREATE);
	strcpy (JP2File,NewName);
	pBS = strrchr (JP2File,'\\');
	REPLAC (pBS,"index","orthos",16);
	strcat (JP2File,".gco");
	FidJP2Out = GSSiOpenFile (JP2File,0,OF_CREATE);
	sprintf (GCIFile,"%s%sorthos%s.gci",drive,dir,IndexRes);
    EndOffset = GSSillseek(FidIndex,(LONG)-(6),2);
    BigRead (FidIndex,(HPSTR)&Signature,4);
    BigRead (FidIndex,(HPSTR)&Version,2);
    if (Signature != 80251 || Version != 2)
    {    
        GSSiClose2 (&FidIndex); 
        return rtn;
    }
	Version = 3;
    GSSillseek(FidIndex,0,0);
    BigRead (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD));
	BigWrite (FidIndexOut,(HPSTR)&FileBounds,sizeof(MNMXCORD),-1);
    BigRead (FidIndex, (HPSTR)&FirstIndex,STOREDINDEXLENGTH);
	indexLoc = GSSillseek (FidIndexOut,0,1);
    BigWrite (FidIndexOut, (HPSTR)&FirstIndex,STOREDINDEXLENGTH,-1);
    FirstIndex.EndOffset = EndOffset; 
    FirstIndex.FirstFoundFile=TRUE;
    _fstrcpy(FirstIndex.FileName,OFStruct.szPathName);
    Handle = GSSiGlobAlloc(GAIDNO  51,GMEM_MOVEABLE,sizeof(FILEINDEX)+FirstIndex.Length);
    lpIndex = (LPFILEINDEX)GlobalLock(Handle);
    *lpIndex = FirstIndex;
    lpIndex->FirstIndexFileOffset = GSSillseek (FidIndex,0,1);
    lpIndex->FileInIndex=0; 
    lpIndex->CurrentEntryOffset=-1; 
    if (lpIndex->NumFiles > USHRT_MAX)
    	lpIndex->NumFiles = USHRT_MAX - 1;
    BigRead (FidIndex,&lpIndex->FirstIndex,(size_t)lpIndex->Length);
    lpIndex->NextHeaderOffset = GSSillseek (FidIndex,0,1);
    GSSiClose2 (&FidIndex);
Next:
	totLen = 0;
	curNumFiles = lpIndex->NumFiles;
	while (GetNextIndexEntry (lpIndex))
	{
		LPSTR pFrame;
		if (lpIndex->CurrentEntry->Len > 0 && (pFrame = strchr (lpIndex->CurrentEntry->Name,'@')))
		{
			int  frame = atoi (pFrame+1);
			short DeleteBM,BitCount, width, height;
			HANDLE hDib;

			if (AVIFrameToDIB (GCIFile,frame,&hDib,&DeleteBM,lpIndex->CurrentEntry->BMBitCount,lpIndex->CurrentEntry->BMWidth,lpIndex->CurrentEntry->BMHeight))
			{
		   		HDIB32 hDib32 = BMPToDIB32 (hDib);  
				//HDIB32 hDib32 = FreeImage_ConvertTo32Bits(hDib24);
				//DestroyDIB32(hDib24, FALSE);

				int	imageLen, imageOffset = GSSillseek (FidJP2Out,0,1);
				HANDLE hmemDIB;
				switch (fmt)
				{
				case 1:
					hmemDIB = WriteDIBToMem(hDib32, FIF_JP2, JP2CompressionFactor, &imageLen);
					break;
				case 2:
					hmemDIB = WriteDIBToMem(hDib32, FIF_TIFF, TIFF_ADOBE_DEFLATE, &imageLen);
					break;
				}

				LPSTR pMem;
				FILEINDEXENTRY indexEntryOut = CopyIndexEntry(lpIndex->CurrentEntry);
				LPSTR pAt = strchr (indexEntryOut.Name,'@');

				itoa (imageOffset,pAt+1,10);
				indexEntryOut.Len = lpIndex->CurrentEntry->Len - strlen (lpIndex->CurrentEntry->Name) + strlen (indexEntryOut.Name);
				totLen += indexEntryOut.Len;
				GSSiGlobFree (&hDib);
				GMDestroyDIB32 (hDib32);
				pMem = GlobalLock (hmemDIB);
				BigWrite (FidIndexOut,&indexEntryOut,indexEntryOut.Len,-1);
				BigWrite (FidJP2Out,&imageLen,sizeof(int),-1);
				BigWrite (FidJP2Out,pMem,imageLen,-1);
				GSSiGlobUlFree (&hmemDIB);
			}
		}
	}
	curLoc = GSSillseek (FidIndexOut,0,1);
	GSSillseek (FidIndexOut,indexLoc,0);
    BigRead (FidIndexOut, (HPSTR)&FirstIndex,STOREDINDEXLENGTH);
	FirstIndex.Length = totLen;
	FirstIndex.NextHeaderOffset = curLoc;
	GSSillseek (FidIndexOut,indexLoc,0);
    BigWrite (FidIndexOut, (HPSTR)&FirstIndex,STOREDINDEXLENGTH,-1);
	GSSillseek (FidIndexOut,curLoc,0);
	indexLoc = curLoc;
	rtn = TRUE;
	lpIndex=GetNextIndexHeader(&Handle,FALSE);
    if (lpIndex == NULL)
		goto Exit;
    lpIndex->CurrentEntry=0;
	lpIndex->NumFiles -= curNumFiles;
    BigWrite (FidIndexOut, (HPSTR)lpIndex,STOREDINDEXLENGTH,-1);
	lpIndex->NumFiles += curNumFiles;
	goto Next;
Exit:
    BigWrite (FidIndexOut,(HPSTR)&Signature,4,-1);
    BigWrite (FidIndexOut,(HPSTR)&Version,2,-1);
	GSSiClose2 (&FidIndexOut);
	GSSiClose2 (&FidJP2Out);
	return rtn;
}

BOOL ConvertToJP2(LPSTR fromDir,int nparts)
{
	char cmd[128] = "[%JP2Factor]=16";
	char levels[256];
	char from[MAX_PATH], to[MAX_PATH];
	BOOL rtn = TRUE;
	int year;
	int maxLevels = 256;
	char saveC = fromDir[4];
	ExpandText(cmd);
	fromDir[4] = 0;
	year = atoi(fromDir);
	fromDir[4] = saveC;
	LPSTR pNextLev=0;
	LPSTR pLevel = 0;

	for (int part = 0; part < nparts; part++)
	{
		int lev = 1;
		sprintf(from, "[%%DL]orthos\\%s\\%i_%i\\global.ini", fromDir, year, part + 1);
		if (ExistFile(from))
		{
			LoadGlobalInit(from, FALSE);
			strcpy(levels, "[%ORTHOLEVS]");
			ExpandText(levels);
			pLevel = levels;
			pNextLev = strchr(levels, ',');
			lev = atoi(pLevel);
			sprintf(to, "[%%DL]orthos\\jp2\\Orth%s\\%i_%i\\global.ini", fromDir, year, part + 1);
			GSSiCopyFile(from, to, TRUE);
		}
		while (rtn && lev > 0)
		{
			sprintf(from, "[%%DL]orthos\\%s\\%i_%i\\index%i", fromDir,year, part+1, lev);
			sprintf(to, "[%%DL]orthos\\jp2\\%s\\%i_%i\\index%i",fromDir,year, part+1, lev);
			ExpandText(from);
			ExpandText(to);
			rtn = ConvertOrthoToJP2(from, to, 1);
			if (pNextLev)
			{
				pNextLev++;
				pLevel = pNextLev;
				pNextLev = strchr(pLevel, ',');
				lev = atoi(pLevel);
			}
			else
				lev = 0;
		}
	}
	return rtn;
}

