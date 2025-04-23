#include "graphint.h"
#include <shlobj.h>
#include <commdlg.h>
#include <stdio.h>
#include <direct.h>
#include <winnt.h> 
#include <Wownt32.h>
#include "minilzo.h"
#include <vfw.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <memory.h>
#include <stdlib.h>
#include "FreeImage.h"
#include <dibutil.h>
#include "dibapi.h"
#include "gm32lib.h"
#include "gmextern.h"

static	LPBYTE	g_load_address;
static	int		MemDIBSize;
static	char	CurImageName[MAX_PATH];
static  int		numImagesAllocated = 0;
static  char	curOutImagePath[MAX_PATH];
static  DWORD	curFlag;

/*#define MAX_ALLOCATED_IMAGES	1024
static	HDIB32	allocatedImages[MAX_ALLOCATED_IMAGES];
static	int		allocatedImagesFrom[MAX_ALLOCATED_IMAGES];
static	int		allocatedImagesID[MAX_ALLOCATED_IMAGES];
static	int		numAllocatedImages = 0;*/

HDIB32 BitmapToDIB_32(HBITMAP hBitmap, HPALETTE hPal);
extern int ii;
extern int curProgID;

#if USEFREEIMAGE
// freeim16.cpp : Defines the entry point for the DLL application.
//

void imageAllocated(HDIB32 dib,int from)
{
	return;
/*	if (from == -1)
	{
		numImagesAllocated--;
		for (int i = 0; i < numAllocatedImages;i++)
		{
			if (dib == allocatedImages[i])
			{
				allocatedImages[i] = 0;
				allocatedImagesID[i] = 0;
				break;
			}
		}
	}
	else if (from == -2)
	{
		numImagesAllocated--;
	}
	else
	{
		if (numAllocatedImages == 55)
			ii = 1;
		allocatedImages[numAllocatedImages] = dib;
		allocatedImagesID[numAllocatedImages++] = from;
		numImagesAllocated++;
	}*/
}
FIBITMAP* GSSiFreeImage_ConvertTo4Bits(HDIB32 hDIB)
{
	FIBITMAP* dib = FreeImage_ConvertTo4Bits(hDIB);
	imageAllocated(dib, 1);
	return dib;
}
FIBITMAP* GSSiFreeImage_ConvertTo8Bits(HDIB32 hDIB)
{
	FIBITMAP* dib = FreeImage_ConvertTo8Bits(hDIB);
	imageAllocated(dib, 1);
	return dib;
}
FIBITMAP* GSSiFreeImage_ColorQuantize(HDIB32 hDib, DWORD Flag)
{
	FIBITMAP* dib = FreeImage_ColorQuantize(hDib,Flag);
	imageAllocated(dib, 1);
	return dib;
}
FIBITMAP* GSSiFreeImage_ConvertTo24Bits(HDIB32 hDIB)
{
	FIBITMAP* dib = FreeImage_ConvertTo24Bits(hDIB);
	imageAllocated(dib, 2);
	return dib;
}
FIBITMAP* GSSiFreeImage_ConvertTo32Bits(HDIB32 hDIB)
{
	FIBITMAP* dib = FreeImage_ConvertTo32Bits(hDIB);
	imageAllocated(dib, 3);
	return dib;
}
FIBITMAP* GSSiFreeImage_ConvertTo16Bits565(HDIB32 hDIB)
{
	FIBITMAP* dib = FreeImage_ConvertTo16Bits565(hDIB);
	imageAllocated(dib, 4);
	return dib;
}

FIBITMAP* GSSiFreeImage_ConvertToGreyscale(HDIB32 hDib)
{
	FIBITMAP* dib = FreeImage_ConvertToGreyscale(hDib);
	imageAllocated(dib, 5);
	return dib;
}
FIBITMAP* GSSiFreeImage_Allocate(int width, int height, int bpp, unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0))
{
	FIBITMAP* dib = FreeImage_Allocate(width, height, bpp, red_mask, green_mask, blue_mask);
	imageAllocated(dib, 6);
	return dib;
}
FIBITMAP* GSSiFreeImage_Load(FREE_IMAGE_FORMAT fif, const char* filename, int flags FI_DEFAULT(0))
{
	FIBITMAP* dib = FreeImage_Load(fif, filename, flags);
	imageAllocated(dib, 7);
	return dib;
}
DWORD GMFIGetVersionAndCopyright (LPSTR Version,LPSTR Copyright)
{
	strcpy (Version,FreeImage_GetVersion ());
	strcpy (Copyright,FreeImage_GetCopyrightMessage ());
	return 0;
}

BOOL GMFIBMPHandleToEXT (LPSTR lpszPathName,HANDLE hBMP,DWORD Flag);

/** FreeImage error handler @param fif Format /
 Plugin responsible for the error @param message Error message */
 void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
 {
	 char mes[1024];
	 if (InDisplayOrthos)
	 {
		 sprintf(mes, "File:%s Frame:%ld Error:%s", CurrentOrthoFile, CurrentOrthoFrame, message);
		 AppendFile("[%DL]abends\\FreeImageErrors.txt", mes);
	 }
	 if (InDisplayConfigs)
	 {
		 sprintf(mes, "File:%s Error:%s", CurrentConfigFile, message);
		 AppendFile("[%DL]abends\\FreeImageErrors.txt", mes);
		 MessageBox(0, mes, "FreeImage Error", MB_ICONEXCLAMATION);
	 }
	 else
	 {
		 //	 MessageBox (0,message,"FreeImage Error",MB_ICONEXCLAMATION);
		 sprintf(mes, "%s Format\n Message:%s\n%s\n%ld", FreeImage_GetFormatFromFIF(fif), message, curOutImagePath, curFlag);
		 MessageBox(0, mes, "FreeImage Error", MB_ICONEXCLAMATION);
	 }
 }
// In your main program …


// ----------------------------------------------------------
BOOL GenericWriter(FIBITMAP* dib, const char* lpszPathName, int flag) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	BOOL bSuccess = FALSE;

	strcpy(curOutImagePath, lpszPathName);
	curFlag = flag;
//	FreeImage_SetOutputMessage(FreeImageErrorHandler);
	if(dib) {
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
		if(fif != FIF_UNKNOWN ) {
			// check that the plugin has sufficient writing and export capabilities ...
			WORD bpp = FreeImage_GetBPP(dib);
			if (fif == FIF_JPEG && (bpp != 8 && bpp != 24))
			{
				FIBITMAP* dib2 = GSSiFreeImage_ConvertTo24Bits(dib);

				bSuccess = FreeImage_Save(fif, dib2, lpszPathName, flag);

				GSSiFreeImage_Unload(dib2);
			}
			else if (fif == FIF_JPEG)
			{
				if (flag == 1000)
					flag = JPEG_QUALITYBAD;
				else if (flag == 1001)
					flag = JPEG_QUALITYAVERAGE;
				else if (flag == 1002)
					flag = JPEG_QUALITYNORMAL;
				else if (flag == 1003)
					flag = JPEG_QUALITYGOOD;
				else if (flag == 1004)
					flag = JPEG_QUALITYSUPERB;
				bSuccess = FreeImage_Save(fif, dib, lpszPathName, flag);
			}
			else if(FreeImage_FIFSupportsWriting(fif) && FreeImage_FIFSupportsExportBPP(fif, bpp)) 
			{
				bSuccess = FreeImage_Save(fif, dib, lpszPathName, flag);
			}
		}
	}
	return (bSuccess == TRUE) ? TRUE : FALSE;
}

WORD GM32SaveDCBitMap (HDC hDC16,LPSTR OutFile,long Format,DWORD Flag,DWORD BackgroundColor,
					   HANDLE hOverViewBitmap16,
					   DWORD OverViewTileCol, DWORD OverViewTileRow,
					   DWORD OverViewTileWidth, DWORD OverViewTileHeight,
					   DWORD OverViewBitmapHeight)
{
	HBITMAP hBitmap;
	HBITMAP hOverViewBitmap= hOverViewBitmap16;
	FIBITMAP *dib;
	BOOL	rtn=FALSE;
	HDC	hDC= hDC16;
	DWORD	nrow,ncol,x,y;
	HDC	hdcMem;
	HBITMAP	hbmPrev,hTempBM;

	curProgID = 10042;
	hTempBM = CreateCompatibleBitmap(hDC,10,10);
	curProgID = -1;
	//    SetMapMode    (hDC, MM_ANISOTROPIC );
//    SetMapMode    (hDC, MM_TEXT );
//    SetWindowOrgEx  ( hDC, 0, 0,0 );
//    SetViewportOrgEx( hDC, 0, 0,0 );    
    hdcMem = CreateCompatibleDC(hDC);
    hbmPrev = SelectObject(hdcMem, hOverViewBitmap);

 /*   i=StretchBlt (hDC, NewRect.left,NewRect.top,
                NewRect.right-NewRect.left+1,
                NewRect.bottom-NewRect.top+1,
                hdcMem, 0, 0,
                Rect.right-Rect.left+1,
                Rect.bottom-Rect.top+1,
                SRCCOPY);*/

    
	hBitmap = SelectObject (hDC,hTempBM);
//	sprintf (mess,"After select %ld %ld",(long)hOverViewBitmap,(long)hBitmap);
//	MessageBox (0,mess,NULL,MB_OK);
	if (hBitmap)
	{
		dib = BitmapToDIB_32(hBitmap,NULL);

//		if (hOverViewBitmap)
		{
			DWORD	OldMode=0;
			LPBYTE	pImage;
			LPBITMAPINFOHEADER	pDibInfo;
			int	destX = OverViewTileCol * OverViewTileWidth;
			int destY = OverViewTileRow * OverViewTileHeight;
			POINT	pt;
			HDC	hDC=hdcMem;

			//destY = OverViewBitmapHeight - destY;
			pDibInfo = FreeImage_GetInfoHeader(dib);
			pImage = FreeImage_GetBits(dib);//(LPBYTE)pDibInfo + pDibInfo->biSize + pDibInfo->biClrUsed * sizeof(RGBQUAD);
			OldMode = SetStretchBltMode(hDC,HALFTONE); 
			SetBrushOrgEx (hDC,0,0,&pt);
			rtn=StretchDIBits (hDC,destX,destY,
								   OverViewTileWidth, OverViewTileHeight,
								   0,0,
								   pDibInfo->biWidth,
								   pDibInfo->biHeight,
								   pImage,
								   (LPBITMAPINFO)pDibInfo,
								   DIB_RGB_COLORS,
								   SRCCOPY);
			/*sprintf (mess,"%ld,%ld,%ld,%ld,%ld,%ld,%ld",rtn,destX,destY,
								   OverViewTileWidth, OverViewTileHeight,
								   pDibInfo->biWidth,
								   pDibInfo->biHeight);
			MessageBox (0,mess,NULL,MB_OK);*/
			if (OldMode)
				SetStretchBltMode(hDC,OldMode);
		}

		nrow = FreeImage_GetHeight (dib);
		ncol = FreeImage_GetWidth (dib);
		for (y=0;y<nrow;y++)
			for (x=0;x<ncol;x++)
			{
				COLORREF	pixelcolor;

				FreeImage_GetPixelColor(dib,x,y,(RGBQUAD *)&pixelcolor);
				if (pixelcolor != BackgroundColor)
				{
					//char mess[256];

					//sprintf (mess,"(%i,%i)%ld : %ld",x,y,pixelcolor,BackgroundColor);
					//MessageBox (0,mess,"SaveDCBM",MB_OK);
					goto DoSave;
				}
			}
			goto NoSave;
DoSave:
		rtn = GMFIBMPHandleToEXT (OutFile,dib,Flag);
NoSave:
		GSSiFreeImage_Unload(dib);
		SelectObject (hDC,hBitmap);
		DeleteObject (hTempBM);
	    SelectObject(hdcMem, hbmPrev);
	    DeleteDC(hdcMem);

	}
	return (WORD)rtn;
}

WORD GM32SaveBitmap (HANDLE hBitmap16,LPSTR OutFile,long Format,DWORD Flag)
{
	HDIB32 dib,dib24;
	BOOL	rtn=FALSE;
	HBITMAP hBitmap= hBitmap16;
	int	l;

	rtn=0;

	if (hBitmap)
	{
		dib = BitmapToDIB_32(hBitmap,NULL);
		l = FreeImage_GetDIBSize (dib);
		dib24 = GSSiFreeImage_ConvertTo24Bits(dib);
		rtn = GMFIBMPHandleToEXT (OutFile,dib24,Flag);
		GSSiFreeImage_Unload(dib);
	}
	return (WORD)rtn;
}

DWORD GM32GetDIBPalette (HDIB32 hDIB,LPLONG pPalletSize,LPRGBQUAD pPalletIn)
{
	FIBITMAP *dib = (FIBITMAP *) hDIB;
	RGBQUAD	*pPallet=(RGBQUAD	*)pPalletIn;
	RGBQUAD	*pColors;
	DWORD	rtn=0;
	UINT	i;

	*(long *)pPalletSize = 0;
	if (dib)
	{
		LPBITMAPINFOHEADER pDibInfo   = FreeImage_GetInfoHeader(dib);
	    if (pDibInfo->biClrUsed)
		{  
			pColors = FreeImage_GetPalette (dib);
			*(long *)pPalletSize = pDibInfo->biClrUsed;
	   	    for (i=0;i<pDibInfo->biClrUsed;i++) 
				pPallet[i] = pColors[i];
		    rtn = 1;
		}
	}
	return rtn;
}

DWORD GM32SetDIBPalette(HDIB32 hDIB,DWORD PalletSize,LPRGBQUAD pPalletIn)
{
	FIBITMAP *dib = (FIBITMAP *) hDIB;
	RGBQUAD	*pPallet=(RGBQUAD	*)pPalletIn;
	RGBQUAD	*pColors;
	DWORD	rtn=0;
	UINT	i;

	if (dib)
	{
		LPBITMAPINFOHEADER pDibInfo = FreeImage_GetInfoHeader(dib);
	    if (pDibInfo->biClrUsed)
		{  
			pColors = FreeImage_GetPalette (dib);
	   	    for (i=0;i<min(PalletSize,pDibInfo->biClrUsed);i++) 
				pColors[i] = pPallet[i];
		    rtn = 1;
		}
	}
	return rtn;
}

int ConvertBitmapColorToTransparent(LPSTR BitmapPath,COLORREF FromColor)
{
	HDIB32	hDIB, hDIB32;
	DWORD	nrow, ncol, row, col, begrow, begcol;
	int		height, width, n = 0;
	RGBQUAD	FromColorQ = RGBQUADFromCOLORREF(FromColor);
	RGBQUAD	ToColorQ = { 0 };


	hDIB = BMPHandleFromEXT(BitmapPath);
	if (!hDIB)
		return -1;
	hDIB32 = GSSiFreeImage_ConvertTo32Bits(hDIB);
	GSSiFreeImage_Unload(hDIB);
	GetDIBDimensionsFromHandle(hDIB32, &height, &width);
	nrow = height;
	ncol = width;
	begrow = 0;
	begcol = 0;
	for (row = begrow; row < nrow; row++)
	{
		for (col = begcol; col < ncol; col++)
		{
			RGBQUAD	c;

			FreeImage_GetPixelColor(hDIB32, col, row, &c);
			if (COLORREFFromRGBQUAD(c) == FromColor)
			{
				n++;
				FreeImage_SetPixelColor(hDIB32, col, row, &ToColorQ);
			}
		}
	}
	SaveDIB32(hDIB32, BitmapPath, 0, -1);
	GSSiFreeImage_Unload(hDIB32);
	return n;
}
int ConvertBitmapColorsInRect(LPSTR BitmapPath, LPMNMXCORD pBounds, COLORREF FromColor, COLORREF ToColor, BOOL CountOnly)
{
	HDIB32	hDIB, hDIB24;
	DWORD	nrow, ncol, row, col, begrow, begcol;
	int		height, width, n = 0;
	RGBQUAD	FromColorQ = RGBQUADFromCOLORREF(FromColor);
	RGBQUAD	ToColorQ = RGBQUADFromCOLORREF(ToColor);
	RECT	Rect;


	hDIB = BMPHandleFromEXT(BitmapPath);
	if (!hDIB)
		return -1;
	BoundsToRect(pBounds, &Rect);
	hDIB24 = GSSiFreeImage_ConvertTo24Bits(hDIB);
	GSSiFreeImage_Unload(hDIB);
	GetDIBDimensionsFromHandle(hDIB24, &height, &width);
	nrow = min(height, Rect.bottom);
	ncol = min(width, Rect.right);
	begrow = Rect.top;
	begcol = Rect.left;
	//CreateStatusWindow(hWndMain,1,0);
	for (row = begrow; row < nrow; row++)
	{
		for (col = begcol; col < ncol; col++)
		{
			RGBQUAD	c;

			FreeImage_GetPixelColor(hDIB24, col, row, &c);
			if (COLORREFFromRGBQUAD(c) == FromColor)
			{
				n++;
				FreeImage_SetPixelColor(hDIB24, col, row, &ToColorQ);
			}
		}
		//StatusWindowUpdate (0,0,nrow-begrow,row-begrow);
	}
	if (!CountOnly)
		SaveDIB32(hDIB24, BitmapPath, 0, -1);
	GSSiFreeImage_Unload(hDIB24);
	return n;
}

BOOL AddImageToImage(HDIB32 hDib32Out, HDIB32 hDib32In, COLORREF FromColor, COLORREF ToColor)
{
	DWORD	nrow, ncol, row, col, begrow, begcol;
	int		height, width, n = 0;
	RGBQUAD	FromColorQ = RGBQUADFromCOLORREF(FromColor);
	RGBQUAD	ToColorQ = RGBQUADFromCOLORREF(ToColor);

	GetDIBDimensionsFromHandle(hDib32In, &height, &width);
	for (row = 0; row < height; row++)
	{
		for (col = 0; col < width; col++)
		{
			RGBQUAD	c;

			if (FreeImage_GetPixelColor(hDib32In, col, row, &c))
			{
				if (c.rgbBlue == FromColorQ.rgbBlue && c.rgbGreen == FromColorQ.rgbGreen && c.rgbRed == FromColorQ.rgbRed)
				{
					if (FreeImage_SetPixelColor(hDib32Out, col, row, &ToColorQ))
						n++;
				}
			}
		}
	}
	return n;
}


int ConvertBitmapColorsInRange(LPSTR BitmapPath, LPSTR ToPath, COLORREF FromColor, COLORREF ToColor, double colordist, LPMNMXCORD pBounds)
{
	HDIB32	hDIB, hDIB24;
	DWORD	nrow, ncol, row, col, begrow, begcol;
	int		height, width, n = 0;
	RGBQUAD	FromColorQ = RGBQUADFromCOLORREF(FromColor);
	RGBQUAD	ToColorQ = RGBQUADFromCOLORREF(ToColor);
	RECT	Rect;

	hDIB = BMPHandleFromEXT(BitmapPath);
	if (!hDIB)
		return -1;
	hDIB24 = GSSiFreeImage_ConvertTo24Bits(hDIB);
	GSSiFreeImage_Unload(hDIB);
	GetDIBDimensionsFromHandle(hDIB24, &height, &width);
	if (pBounds)
	{
		BoundsToRect(pBounds, &Rect);
		nrow = min(height, Rect.bottom);
		ncol = min(width, Rect.right);
		begrow = Rect.top;
		begcol = Rect.left;
	}
	else
	{
		nrow = height;
		ncol = width;
		begrow = 0;
		begcol = 0;
	}
	//CreateStatusWindow(hWndMain,1,0);
	for (row = begrow; row<nrow; row++)
	{
		for (col = begcol; col<ncol; col++)
		{
			RGBQUAD	c;

			FreeImage_GetPixelColor(hDIB24, col, row, &c);
			if (c.rgbBlue != ToColorQ.rgbBlue || c.rgbGreen != ToColorQ.rgbGreen || c.rgbRed != ToColorQ.rgbRed)
			{
				if (RGBQUADDist(c, FromColorQ) <= colordist)
				{
					n++;
					FreeImage_SetPixelColor(hDIB24, col, row, &ToColorQ);
				}
			}
		}
		//StatusWindowUpdate (0,0,nrow-begrow,row-begrow);
	}
	if (!SaveDIB32(hDIB24, ToPath,-1, 0))
		n = -1;
	GSSiFreeImage_Unload(hDIB24);
	return n;
}

DWORD GM32QuantizeDIB (HDIB32 hDIB,DWORD Flag)
{
	FIBITMAP *dib = (FIBITMAP *) hDIB;
	DWORD	rtn=0;
	
	if(hDIB)
	{
		rtn = (DWORD)FreeImage_ColorQuantize(dib,Flag);
	}
	return rtn;
}
DWORD GM32QuantizeDIBEx (HDIB32 hDIB,DWORD Flag,DWORD PalSize, DWORD ResPalSize, RGBQUAD *Pallet)
{
	FIBITMAP *dib = (FIBITMAP *) hDIB;
	DWORD	rtn=0;
	
	if(hDIB)
	{
		rtn = (DWORD)FreeImage_ColorQuantizeEx (dib,Flag,PalSize,ResPalSize,Pallet);
	}
	return rtn;
}

WORD GM32SaveDIB (HDIB32 hDIB,LPSTR OutFileIN,long Format,DWORD Flag)
{
	FIBITMAP *dib = (FIBITMAP *) hDIB;
	BOOL	rtn=FALSE;
	char	OutFile[MAX_PATH];
	int		l;

	if (hDIB)
	{
		strcpy(OutFile, OutFileIN);
		ExpandText(OutFile);
		rtn = GMFIBMPHandleToEXT(OutFile, (HANDLE)hDIB, Flag); 
//		rtn = GenericWriter(dib,OutFile,Flag);

	}
	return (WORD)rtn;
}

HDIB32 BitmapToDIB_32(HBITMAP hBitmap, HPALETTE hPal)
{
   BITMAP bm;                   // bitmap structure
   HDIB32	dib = NULL;
   BITMAPINFOHEADER bi;         // bitmap header
   BITMAPINFOHEADER FAR *lpbi;  // pointer to BITMAPINFOHEADER
   DWORD dwLen;                 // size of memory block
   HDC hDC;                     // handle to DC
   WORD biBits;                 // bits per pixel
   int	ctype;
   

   /* check if bitmap handle is valid */

   if (!hBitmap)
      return NULL;

   /* fill in BITMAP structure, return NULL if it didn't work */
   if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm))
      return NULL;

   /* if no palette is specified, use default palette */
   if (hPal == NULL)
      hPal = GetStockObject(DEFAULT_PALETTE);

   /* calculate bits per pixel */
   biBits = bm.bmPlanes * bm.bmBitsPixel;

   /* make sure bits per pixel is valid */
   if (biBits <= 1)
      biBits = 1;
   else if (biBits <= 4)
      biBits = 4;
   else if (biBits <= 8)
      biBits = 8;
   else if (biBits != 32)
      biBits = 24;

   /* initialize BITMAPINFOHEADER */
   bi.biSize = sizeof(BITMAPINFOHEADER);
   bi.biWidth = bm.bmWidth;
   bi.biHeight = bm.bmHeight;
   bi.biPlanes = 1;
   bi.biBitCount = biBits;
   bi.biCompression = BI_RGB;
   bi.biSizeImage = 0;
   bi.biXPelsPerMeter = 0;
   bi.biYPelsPerMeter = 0;
   bi.biClrUsed = 0;
   bi.biClrImportant = 0;

   /* calculate size of memory block required to store BITMAPINFO */
   dwLen = bi.biSize + PaletteSize((LPSTR)&bi);

   /* get a DC */
   hDC = GetDC(NULL);

   /* select and realize our palette */
   hPal = SelectPalette(hDC, hPal, FALSE);
   RealizePalette(hDC);


	dib = GSSiFreeImage_Allocate (bi.biWidth,bi.biHeight,bi.biBitCount,0,0,0);
   /* alloc memory block to store our bitmap */
   //hDIB = GlobalAlloc(GHND, dwLen);

   /* if we couldn't get memory block */
   if (!dib)
   {
      /* clean up and return NULL */
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }

   lpbi   = FreeImage_GetInfoHeader(dib);
   /* lock memory and get pointer to it */
   


   /*  call GetDIBits with a NON-NULL lpBits param, and actualy get the
    *  bits this time
    */
   if (bi.biBitCount == 1)
   {
	   ctype = DIB_PAL_COLORS;
	   bi.biClrUsed = 2;
   }
   else if (bi.biBitCount == 8)
   {
	   ctype = DIB_PAL_COLORS;
	   bi.biClrUsed = 256;
   }
   else
	   ctype = DIB_RGB_COLORS;
   /* use our bitmap info. to fill BITMAPINFOHEADER */
   *lpbi = bi;
   if (bi.biBitCount == 1)
   {
	   RGBQUAD colors[2] = { RGB(0,0,0),RGB(255,255,255) };
	   RGBQUAD* pColors = (RGBQUAD * )(lpbi + 1);
	   pColors[0].rgbRed = 0;
	   pColors[0].rgbGreen = 0;
	   pColors[0].rgbBlue = 0;
	   pColors[0].rgbReserved = 0;
	   pColors[1].rgbRed = 255;
	   pColors[1].rgbGreen = 255;
	   pColors[1].rgbBlue = 255;
	   pColors[1].rgbReserved = 255;
   }
   if (GetDIBits(hDC, hBitmap, 0, (WORD)bi.biHeight, (LPSTR)lpbi + (WORD)lpbi
         ->biSize + PaletteSize((LPSTR)lpbi), (LPBITMAPINFO)lpbi,
         ctype) == 0)
   {
      /* clean up and return NULL */
	  
	  GSSiFreeImage_Unload(dib);
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }
   bi = *lpbi;

   /* clean up */
   SelectPalette(hDC, hPal, TRUE);
   RealizePalette(hDC);
   ReleaseDC(NULL, hDC);

   /* return handle to the DIB */
   return dib;
}

DWORD GM32BitmapToDIB (DWORD hBitmap16)
{
	HDIB32 dib=NULL;
	BOOL	rtn=FALSE;
	HBITMAP hBitmap= (HBITMAP)hBitmap16;
	rtn=0;

	if (hBitmap)
	{
		dib = BitmapToDIB_32(hBitmap,NULL);
	}
	return (DWORD)dib;
}


long StretchDIBits32 (HDC hDC,long destX,long destY,long destW,long destH,long xoff,long yoff,long bmWidth,long bmHeight,
					  LPBYTE pImage, LPBITMAPINFOHEADER pDibInfo,long ColorType,long RastOpts,LPDOUBLE pFactor,BOOL Fast)
{
	DWORD	rtn, OldMode=0;
	double	Factor = *pFactor;
	POINT	pt;

//	MessageBox (0,"Made it in",NULL,MB_ICONEXCLAMATION);
	if (destW < 0 || bmWidth < 0 || destH < 0 || bmHeight < 0)
		return 0;
	if (RastOpts == SRCCOPY && !Fast)
	{
		OldMode = SetStretchBltMode(hDC,HALFTONE); 
		SetBrushOrgEx (hDC,0,0,&pt);
	}
	rtn=StretchDIBits (hDC,destX,destY,
	                   destW, destH,
	                   xoff,yoff,
	                   (int)(bmWidth*Factor),
	                   (int)(bmHeight*Factor),
	                   pImage,
	                  (LPBITMAPINFO)pDibInfo,
	                  (UINT)ColorType,
	                  (DWORD) RastOpts);
	if (OldMode)
		SetStretchBltMode(hDC,OldMode);
	return rtn;
}



BOOL GMSetDIBMonoColors (HANDLE hBMP,LPLONG Colors)
{
	FIBITMAP *dib = (FIBITMAP *) hBMP;
	LPLONG	pColors;
	DWORD	rtn=0;

	if (dib)
	{
		LPBITMAPINFOHEADER pDibInfo   = FreeImage_GetInfoHeader(dib);
	    if (pDibInfo->biClrUsed == 2)
		{   
	   	   
		   pColors = (LPLONG)((LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER));
		   pColors[0] = Colors[0];
		   pColors[1] = Colors[1];
		   rtn = 1;
		}
	}
	return rtn;
}

BOOL GMFIBMPHandleToEXT (LPSTR lpszPathName,HANDLE hBMP,DWORD Flag)
{
	FIBITMAP *dib = (FIBITMAP *) hBMP;
	BOOL	rtn=FALSE;

	if (!*lpszPathName)
		return FALSE;
	makedirectories (lpszPathName,FALSE,FALSE);
	if (dib)
		rtn = GenericWriter(dib,lpszPathName,Flag);
	return rtn;
}

BOOL GMFIBMPToEXT (LPSTR lpszPathName,LPSTR FromMem,DWORD * pSize,DWORD Flag)
{
	FIBITMAP *dib = NULL;
	BOOL	rtn=FALSE;
	BITMAPINFOHEADER * bihIn = (BITMAPINFOHEADER *) FromMem;

	dib = FreeImage_Allocate (bihIn->biWidth,bihIn->biHeight,bihIn->biBitCount,0,0,0);
	if (dib)
	{
		DWORD	size = (DWORD)FreeImage_GetDIBSize(dib);
		BITMAPINFOHEADER *bih   = FreeImage_GetInfoHeader(dib);
		
		size = min (size,*pSize);
		memcpy(bih,FromMem,size);
		rtn = GenericWriter(dib,lpszPathName,Flag);
		GSSiFreeImage_Unload(dib);
	}

	return rtn;
}

BOOL GMFIBMPFromEXT (LPSTR lpszPathName,LPSTR ToMem,DWORD * pSize)
{
	FIBITMAP *dib = NULL;
	BOOL	rtn=TRUE;
	int id = 1;

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if(fif == FIF_UNKNOWN)
	{
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		// ok, let's load the file
		FIBITMAP *dib = GSSiFreeImage_Load(fif, lpszPathName, BMP_DEFAULT);
		// unless a bad file format, we are done !


			if (dib != NULL)
			{
				DWORD	size = (DWORD)FreeImage_GetDIBSize(dib);
				BITMAPINFOHEADER *bih   = FreeImage_GetInfoHeader(dib);
				if (size <= *pSize)
					memcpy(ToMem,bih,size);
				else
					rtn = FALSE;
				*pSize = size;
//				FreeImage_SaveBMP(dib, ToFile);

				GSSiFreeImage_Unload(dib);

			}

	}
	return rtn;
}

BOOL GMFIBMPFileFromEXT (LPSTR lpszPathName,LPSTR ToFile,double factor)
{
	FIBITMAP *dib = NULL;
	DWORD	rtn=0;
	int id = 1;

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
//	fif = FreeImage_GetFileType(lpszPathName, 0);
	if(fif == FIF_UNKNOWN)
	{
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		// ok, let's load the file
		FIBITMAP *dib = GSSiFreeImage_Load(fif, lpszPathName, BMP_DEFAULT);
		rtn = fif;
		// unless a bad file format, we are done !


			if (dib != NULL)
			{
				if (factor != 1.0)
				{
					int w, h;
					LPBITMAPINFOHEADER pDibInfo = FreeImage_GetInfoHeader(dib);
					FIBITMAP *dib2 = FreeImage_Rescale(dib,pDibInfo->biWidth*factor,pDibInfo->biHeight*factor, FILTER_CATMULLROM);
					GSSiFreeImage_Unload(dib);
					dib = dib2;
				}
				rtn = FreeImage_Save(FIF_BMP, dib,ToFile,BMP_DEFAULT);
//				FreeImage_SaveBMP(dib, ToFile);

				GSSiFreeImage_Unload(dib);
				if (rtn)
					rtn = 3;
				else
					rtn = 4;

			}

	}
	return rtn;
}

HDIB32 GM32AllocateDIB (DWORD Width,DWORD Height,DWORD BitsPerPixel)
{
	FIBITMAP *dib = FreeImage_Allocate (Width,Height,BitsPerPixel,0,0,0);

	return dib;
}

DWORD GM32PasteDIB (HDIB32 ToDIB,HDIB32 FromDIB,DWORD Left,DWORD Top,DWORD Alpha)
{
	DWORD rtn = FreeImage_Paste ((FIBITMAP *)ToDIB,(FIBITMAP *)FromDIB,Left,Top,Alpha);

	return rtn;
}

HDIB32 GMFIBMPHandleFromEXT (LPSTR PathName, BOOL InfoOnly)
{
	FIBITMAP *dib = NULL;
	HDIB32	rtn=0;
	int id = 1;
	char	lpszPathName[MAX_PATH];
	UINT flag = BMP_DEFAULT;

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	if (!*PathName)
		return 0;
	if (InfoOnly)
		flag = FIF_LOAD_NOPIXELS;
	strcpy (lpszPathName,PathName);
	ExpandText (lpszPathName);
	ConvertToNewLocation (lpszPathName,FALSE);
	if (ExistFile (lpszPathName))
	{
		// check the file signature and deduce its format
		// (the second argument is currently not used by FreeImage)
		fif = FreeImage_GetFileType(lpszPathName, 0);
		if (strstr (lpszPathName,".sbm"))
			fif = FIF_BMP;
		else if(fif == FIF_UNKNOWN)
		{
			// no signature ?
			// try to guess the file format from the file extension
			fif = FreeImage_GetFIFFromFilename(lpszPathName);
		}
		// check that the plugin has reading capabilities ...
		if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
		{
			// ok, let's load the file
			FIBITMAP* dib;
			if (fif == FIF_JPEG && !InfoOnly)
			{
				if (GetGlobalBVal2("[%USEJPEGROTATION]", TRUE))
					flag = JPEG_EXIFROTATE | JPEG_ACCURATE;
				else
					flag = JPEG_ACCURATE;
			}
			dib = GSSiFreeImage_Load(fif, lpszPathName, flag);
			rtn = (HDIB32)dib;
			// unless a bad file format, we are done !
			LPBITMAPINFOHEADER	pDibInfo = FreeImage_GetInfoHeader((FIBITMAP*)dib);
			id = 1;
		}
	}
	return rtn;
}
BOOL GMFIInfoFromEXT(LPSTR PathName, LPBITMAPINFOHEADER	pDibInfo)
{
	FIBITMAP *dib = NULL;
	BOOL	rtn = FALSE;
	int id = 1;
	char	lpszPathName[MAX_PATH];

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	strcpy(lpszPathName, PathName);
	ExpandText(lpszPathName);
	ConvertToNewLocation(lpszPathName, FALSE);
	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);
	if (strstr(lpszPathName, ".sbm"))
		fif = FIF_BMP;
	else if (fif == FIF_UNKNOWN)
	{
		// no signature ?
		// try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(lpszPathName);
	}
	// check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		dib = GSSiFreeImage_Load(fif, lpszPathName, FIF_LOAD_NOPIXELS);
		if (dib)
		{
			*pDibInfo = *FreeImage_GetInfoHeader((FIBITMAP *)dib);
			GSSiFreeImage_Unload(dib);
			rtn = TRUE;
		}
	}
	return rtn;
}

HANDLE GMFreeImageRotateClassic (HANDLE hDIBIn,double Rotate)
{
	FIBITMAP *dibIn = (FIBITMAP *) hDIBIn;
	HANDLE	rtn=0;
	
	if(hDIBIn)
	{
			FIBITMAP *dib = FreeImage_Rotate (dibIn,Rotate,NULL);
			rtn = (HANDLE)dib;
	}
	return rtn;
}

DWORD GMFICopy (DWORD hDIBIn,DWORD left,DWORD right, DWORD top, DWORD bottom)
{
	FIBITMAP *dibIn = (FIBITMAP *) hDIBIn;
	DWORD	rtn=0;

	if (hDIBIn)
	{
		FIBITMAP *dibOut = FreeImage_Copy (dibIn,left,top,right,bottom);
		
		rtn = (DWORD)dibOut;
	}
	return rtn;

}

void GSSiFreeImage_Unload(HDIB32 hdib)
{
	if (hdib)
	{
		imageAllocated(hdib, -1);

		FreeImage_Unload(hdib);
	}
	else
		imageAllocated(hdib, -2);
}
//DWORD GMFIBMPUnload (DWORD dibdw)
HDIB32 GMDestroyDIB32 (HDIB32 hDib)
{
	if (hDib)
		GSSiFreeImage_Unload(hDib);
/*	FIBITMAP *dib = (FIBITMAP*)hDib;

	if (dib != NULL)
		FreeImage_Unload(dib);*/
	return NULL;
}

/*DLL_API FIMETADATA *DLL_CALLCONV FreeImage_FindFirstMetadata(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, FITAG **tag);
DLL_API BOOL DLL_CALLCONV FreeImage_FindNextMetadata(FIMETADATA *mdhandle, FITAG **tag);
DLL_API void DLL_CALLCONV FreeImage_FindCloseMetadata(FIMETADATA *mdhandle);
DLL_API BOOL DLL_CALLCONV FreeImage_SetMetadata(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const char *key, FITAG *tag);
DLL_API BOOL DLL_CALLCONV FreeImage_GetMetadata(FREE_IMAGE_MDMODEL model, FIBITMAP *dib, const char *key, FITAG **tag);
33550
33922*/

BOOL GMFIGetGeoTiffData (HANDLE hBMP,DWORD ShowTag,DWORD pScaleX, DWORD pScaleY, DWORD pBitmapPoint, DWORD pWorldPoint)
{
	FIBITMAP *dib = (FIBITMAP *) hBMP;
	BOOL	rtn=FALSE;
	char	Tag[4096];
	FREE_IMAGE_MDMODEL	model = FIMD_GEOTIFF;
	BOOL	HaveScale=FALSE, HavePoint=FALSE;
	LPDOUBLE	pDouble;

	if (dib)
	{
		FITAG *tag = NULL;
		FIMETADATA *mdhandle = NULL;
		
		if (FreeImage_GetMetadataCount (model,dib))
		{
		mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);

		if(mdhandle) {

			do {
				// convert the tag value to a string
				const char *value = FreeImage_TagToString(model, tag,NULL);

				// note that most tags do not have a description, 
				// especially when the metadata specifications are not available
				if (ShowTag)
				{
					if(FreeImage_GetTagDescription(tag))
					{
						sprintf (Tag,"%s:%s(%s)",FreeImage_GetTagKey(tag),value,FreeImage_GetTagDescription(tag));
					}
					else
					{
						sprintf (Tag,"%s:%s(%s)",FreeImage_GetTagKey(tag),value,"No desc");
					}
					MessageBox (0,Tag,NULL,MB_OK);
				}
				//sprintf (Tag,"%ld %ld %ld",FreeImage_GetTagID(tag),FreeImage_GetTagType(tag),FreeImage_GetTagCount(tag));
				//MessageBox (0,Tag,NULL,MB_OK);
				switch (FreeImage_GetTagID(tag))
				{
				case 33550:
					if (FreeImage_GetTagType(tag)==FIDT_DOUBLE && FreeImage_GetTagCount(tag)==3)
					{
						pDouble = (LPDOUBLE)FreeImage_GetTagValue(tag);
						*(LPDOUBLE)pScaleX = *pDouble++;
						*(LPDOUBLE)pScaleY = *pDouble;
						HaveScale=TRUE;
					}
					break;

				case 33922:
					if (FreeImage_GetTagType(tag)==FIDT_DOUBLE && FreeImage_GetTagCount(tag)==6)
					{
						LPDPOINT pPoint=(LPDPOINT)pBitmapPoint;

						pDouble = (LPDOUBLE)FreeImage_GetTagValue(tag);
						pPoint->x = *pDouble++;
						pPoint->y = *pDouble++;
						pDouble++;
						pPoint = (LPDPOINT)pWorldPoint;
						pPoint->x = *pDouble++;
						pPoint->y = *pDouble++;
						HavePoint=TRUE;
					}
					break;
				}

			} while(FreeImage_FindNextMetadata(mdhandle, &tag));
		}
		FreeImage_FindCloseMetadata(mdhandle);
		if (ShowTag)
			MessageBox (0,"End",NULL,MB_OK);
		if (HaveScale && HavePoint)
			rtn = TRUE;
	}
	}

return rtn;
}

BOOL GMFISetGeoTiffData (DWORD hBMP,DWORD pScaleX, DWORD pScaleY, DWORD pBitmapPoint, DWORD pWorldPoint)
{
	FIBITMAP *dib = (FIBITMAP *) hBMP;
	BOOL	rtn=FALSE;
	FREE_IMAGE_MDMODEL	model = FIMD_GEOTIFF;
	double	Scale[3],Points[6];

	Scale[0]=*(LPDOUBLE)pScaleX;
	Scale[1]=*(LPDOUBLE)pScaleY;
	Scale[2]=0;

	Points[0] = ((LPDPOINT)pBitmapPoint)->x;
	Points[1] = ((LPDPOINT)pBitmapPoint)->y;
	Points[2] = 0;
	Points[3] = ((LPDPOINT)pWorldPoint)->x;
	Points[4] = ((LPDPOINT)pWorldPoint)->y;
	Points[5] = 0;

	if (dib)
	{
		FITAG *tag;
		
		tag = FreeImage_CreateTag ();
		FreeImage_SetTagKey (tag,"GeoPixelScale");
		FreeImage_SetTagType (tag,FIDT_DOUBLE);
		FreeImage_SetTagCount (tag,3);
		FreeImage_SetTagLength (tag,3*sizeof(double));
		FreeImage_SetTagValue (tag,&Scale);
		FreeImage_SetMetadata (model,dib,FreeImage_GetTagKey(tag),tag);
		FreeImage_DeleteTag (tag);
		
		tag = FreeImage_CreateTag ();
		FreeImage_SetTagKey (tag,"GeoTiePoints");
		FreeImage_SetTagType (tag,FIDT_DOUBLE);
		FreeImage_SetTagCount (tag,6);
		FreeImage_SetTagLength (tag,6*sizeof(double));
		FreeImage_SetTagValue (tag,&Points);
		FreeImage_SetMetadata (model,dib,FreeImage_GetTagKey(tag),tag);
		FreeImage_DeleteTag (tag);
		
		rtn = TRUE;
	}

return rtn;
}

BOOL GetImageCoord (HANDLE hBMP, LPDPOINT pWorldPoint,LPSTR DateTaken)
{
	FIBITMAP *dib = (FIBITMAP *) hBMP;
	BOOL	rtn=FALSE;
	char	Tag[4096];
	FREE_IMAGE_MDMODEL	model = FIMD_EXIF_GPS;
	BOOL	HaveScale=FALSE, HavePoint=FALSE,ShowTag=FALSE;
	LPDOUBLE	pDouble, pScaleX, pScaleY;
	int		ii;

	if (DateTaken)
		*DateTaken = 0;
	if (dib)
	{
		FITAG *tag = NULL;
		FIMETADATA *mdhandle = NULL;
		int	nTag[10],i;
		
		for (model=0;model<10;model++)
		{
			if (FreeImage_GetMetadataCount (model,dib))
			{
				mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);

				if(mdhandle) {

					do {
						// convert the tag value to a string
						const char *value = FreeImage_TagToString(model, tag,NULL);

						// note that most tags do not have a description, 
						// especially when the metadata specifications are not available
						if (ShowTag)
						{
							if(FreeImage_GetTagDescription(tag))
							{
								sprintf (Tag,"%i:%i:%i:%i:%s:%s(%s)",model,FreeImage_GetTagID(tag),FreeImage_GetTagType(tag),FreeImage_GetTagCount(tag),FreeImage_GetTagKey(tag),value,FreeImage_GetTagDescription(tag));
							}
							else
							{
								sprintf (Tag,"%s:%s(%s)",FreeImage_GetTagKey(tag),value,"No desc");
							}
							MessageBox (0,Tag,NULL,MB_OK);
						}
						//sprintf (Tag,"%ld %ld %ld",FreeImage_GetTagID(tag),FreeImage_GetTagType(tag),FreeImage_GetTagCount(tag));
						//MessageBox (0,Tag,NULL,MB_OK);
						if (model == FIMD_EXIF_EXIF)
						{
							int	TagID = FreeImage_GetTagID(tag);

								switch (TagID)
							{
								case 36867:
								if (FreeImage_GetTagType(tag)==FIDT_ASCII)
								{
									LPSTR pDate = (LPSTR)FreeImage_GetTagValue(tag);

									if (DateTaken)
										strcpy (DateTaken,pDate);
									
								}
								break;
							}
						}

						else if (model == FIMD_EXIF_GPS)
							switch (FreeImage_GetTagID(tag))
						{
						case 0:		//GPS Version
							if (FreeImage_GetTagType(tag)==FIDT_UNDEFINED && FreeImage_GetTagCount(tag)==4)
							{
								LPSTR pRef = (LPSTR)FreeImage_GetTagValue(tag);
								
								ii=1;
							}
							break;
						case 1:		//GPS Latitude ref
							if (FreeImage_GetTagType(tag)==FIDT_ASCII)
							{
								LPSTR pRef = (LPSTR)FreeImage_GetTagValue(tag);
								
								if (*pRef == 'S')
									pWorldPoint->y *= -1;
							}
							break;
						case 2:		//GPS Latitude
							if (FreeImage_GetTagType(tag)==FIDT_RATIONAL && FreeImage_GetTagCount(tag)==3)
							{
								LPINT pFract = (LPINT)FreeImage_GetTagValue(tag);
								int Deg1 = *pFract++;
								int Deg2 = *pFract++;
								int Min1 = *pFract++;
								int Min2 = *pFract++;
								int Sec1 = *pFract++;
								int Sec2 = *pFract;
								
								HavePoint = TRUE;
								pWorldPoint->y = (double)Deg1/Deg2 + ((double)Min1/Min2)/60 + ((double)Sec1/Sec2)/3600;
							}
							break;
						case 3:		//GPS Longitude ref
							if (FreeImage_GetTagType(tag)==FIDT_ASCII)
							{
								LPSTR pRef = (LPSTR)FreeImage_GetTagValue(tag);
								
								if (*pRef == 'W')
									pWorldPoint->x *= -1;
							}
							break;
						case 4:		//GPS Longitude
							if (FreeImage_GetTagType(tag)==FIDT_RATIONAL && FreeImage_GetTagCount(tag)==3)
							{
								LPINT pFract = (LPINT)FreeImage_GetTagValue(tag);
								int Deg1 = *pFract++;
								int Deg2 = *pFract++;
								int Min1 = *pFract++;
								int Min2 = *pFract++;
								int Sec1 = *pFract++;
								int Sec2 = *pFract;

								pWorldPoint->x = (double)Deg1/Deg2 + ((double)Min1/Min2)/60 + ((double)Sec1/Sec2)/3600;
							}
							break;
						case 5:		//GPS altitude reference
							if (FreeImage_GetTagType(tag)==FIDT_UNDEFINED && FreeImage_GetTagCount(tag)==1)
							{
								LPSTR pByte = (LPSTR)FreeImage_GetTagValue(tag);
								HaveScale=TRUE;
							}
							if (FreeImage_GetTagType(tag)==FIDT_BYTE && FreeImage_GetTagCount(tag)==1)
							{
								LPBYTE pByte = (LPBYTE)FreeImage_GetTagValue(tag);
								HaveScale=TRUE;
							}
							break;
						case 6:		//GPS Altitude
							if (FreeImage_GetTagType(tag)==FIDT_RATIONAL && FreeImage_GetTagCount(tag)==1)
							{
								LPINT pFract = (LPINT)FreeImage_GetTagValue(tag);
								int Fract1 = *pFract++;
								int Fract2 = *pFract;
								HaveScale=TRUE;
							}
							break;
						case 12:	//GPS Speed Units
							if (FreeImage_GetTagType(tag)==FIDT_ASCII && FreeImage_GetTagCount(tag)==1)
							{
								LPSTR pRef = (LPSTR)FreeImage_GetTagValue(tag);
								
							}
							break;
						case 13:	//GPS Speed
							if (FreeImage_GetTagType(tag)==FIDT_RATIONAL && FreeImage_GetTagCount(tag)==1)
							{
								LPINT pFract = (LPINT)FreeImage_GetTagValue(tag);
								int Fract1 = *pFract++;
								int Fract2 = *pFract;
								HaveScale=TRUE;
							}
							break;
						case 14:	//GPS Direction of movement reference
							if (FreeImage_GetTagType(tag)==FIDT_ASCII && FreeImage_GetTagCount(tag)==1)
							{
								LPSTR pRef = (LPSTR)FreeImage_GetTagValue(tag);
								
							}
							break;
						case 15:	//GPS Direction of movement	
							if (FreeImage_GetTagType(tag)==FIDT_RATIONAL && FreeImage_GetTagCount(tag)==1)
							{
								LPINT pFract = (LPINT)FreeImage_GetTagValue(tag);
								int Fract1 = *pFract++;
								int Fract2 = *pFract;
								HaveScale=TRUE;
							}
							break;
						}

					} while(FreeImage_FindNextMetadata(mdhandle, &tag));
				}
				FreeImage_FindCloseMetadata(mdhandle);
				if (ShowTag)
					MessageBox (0,"End",NULL,MB_OK);
			}
		}
		if (HavePoint)
		{
			if (!ConvertCoord (pWorldPoint,2,1))
				rtn = TRUE;
		}
	}

	return rtn;
}


//BOOL GMFIBMPGetBitmapInfo (LPBITMAPINFOHEADER pDibInfoD, HANDLE hDib)
BOOL GetBitmapInfoFromHandle (LPBITMAPINFOHEADER pDibInfoD,HDIB32 hDib)
{
	LPBITMAPINFOHEADER	pDibInfo;

	if (!hDib)
		return FALSE;

	pDibInfo = FreeImage_GetInfoHeader((FIBITMAP *)hDib);

	*pDibInfoD = *pDibInfo;
	if (pDibInfoD->biSizeImage == 0)
	{
		pDibInfoD->biSizeImage = ((((pDibInfoD->biWidth * (DWORD)pDibInfoD->biBitCount) + 31) & ~31) >> 3)
			* pDibInfoD->biHeight;
	}
	return TRUE;
}
int SetCurImage (LPSTR Name)
{
	strcpy (CurImageName,Name);
	return 0;
}

DWORD GM32StretchDIBitsFromHandle (HDC hDC16,long destX,long destY,long destW,long destH,long xoff,
						 long yoff,long sourcew, long sourceh, HANDLE Handle,DWORD ColorType,DWORD RasterOpt,LPDOUBLE pFactorD)
{
	HDC	hDC= hDC16;
	DWORD	rtn, OldMode=0;
	LPBYTE	pImage;
	LPBITMAPINFOHEADER	pDibInfo;
	double	Factor = *(double*)pFactorD;
	POINT	pt;
	BOOL	UseHalf=TRUE;
	char	str[512];
	int		ii;
	extern short StretchMode;

	if (destW < 0 || sourcew < 0 || destH < 0 || sourceh < 0)
		return 0;
	pDibInfo = FreeImage_GetInfoHeader((FIBITMAP *)Handle);
	pImage = FreeImage_GetBits((FIBITMAP *)Handle);
//	pImage = (LPBYTE)pDibInfo + pDibInfo->biSize + pDibInfo->biClrUsed * sizeof(RGBQUAD);
//	MessageBox (0,"Made it in",NULL,MB_ICONEXCLAMATION);
/*	if (UseHalf && RasterOpt == SRCCOPY)
	{
		OldMode = SetStretchBltMode(hDC,HALFTONE); 
		SetBrushOrgEx (hDC,0,0,&pt);
	}
	else
		OldMode = SetStretchBltMode(hDC,COLORONCOLOR);*/ //01/05/2013 for waypoint basemap
	SetStretchBltMode(hDC,StretchMode);
	if (StretchMode == HALFTONE)
		SetBrushOrgEx(hDC, 0, 0, &pt);
	if (RasterOpt >= (DWORD)0x01000000)
		RasterOpt -= (DWORD)0x01000000;
/*	sprintf (str,"%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld",
		pDibInfo->biWidth,pDibInfo->biHeight,pDibInfo->biBitCount,destX,destY,destW,destH,xoff,yoff,IDNINT(sourcew*Factor),IDNINT(sourceh*Factor),ColorType,pDibInfo->biSize,pDibInfo->biClrUsed);
	AppendFile2 ("c:\\bmptrace.txt",str);
	AppendFile2 ("c:\\bmptrace.txt",CurImageName);*/
	rtn=StretchDIBits (hDC,destX,destY,
	                   destW, destH,
	                   xoff,yoff,
	                   IDNINT(sourcew*Factor),
	                   IDNINT(sourceh*Factor),
	                   pImage,
	                  (LPBITMAPINFO)pDibInfo,
	                  (UINT)ColorType,
	                  (DWORD) RasterOpt);
	SetHBirdStretchDIBits (destX,destY,
	                   destW, destH,
	                   xoff,yoff,
	                   IDNINT(sourcew*Factor),
	                   IDNINT(sourceh*Factor),
					   pDibInfo->biWidth,pDibInfo->biHeight);
	if (OldMode)
		SetStretchBltMode(hDC,OldMode);
//	AppendFile2 ("c:\\bmptrace.txt","Done");
	return rtn;
}


 _stdcall
_ReadProc(void *buffer, unsigned int size, unsigned int count, fi_handle handle) {
	BYTE *tmp = (BYTE *)buffer;
	unsigned int	c;

	for (c = 0; c < count; c++) {
		memcpy(tmp, g_load_address, size);

		g_load_address = (BYTE *)g_load_address + size;

		tmp += size;
	}

	return count;
}

int _stdcall
_WriteProc(void *buffer, unsigned int size, unsigned int count, fi_handle handle) {
	// there's not much use for saving the bitmap into memory now, is there?

	return size;
}

int _stdcall
_SeekProc(fi_handle handle, long offset, int origin) {
	assert(origin != SEEK_END);

	if (origin == SEEK_SET) {
		g_load_address = (BYTE *)handle + offset;
	} else {
		g_load_address = (BYTE *)g_load_address + offset;
	}

	return 0;
}

long _stdcall
_TellProc(fi_handle handle) {
	assert((int)handle > (int)g_load_address);

	return ((int)g_load_address - (int)handle);
}

// ----------------------------------------------------------

HDIB32 LoadDIBFromMem (LPBYTE pMem,int MemLen,int Format,int flags)
{
	FIBITMAP *dib;
	FIMEMORY *hmem = FreeImage_OpenMemory(pMem,MemLen);
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
	int	Flag=0;
	FIBITMAP *check;
//	long file_size = FreeImage_GetDIBSize(check);
	
	if (Format == FIF_TIFF)
		Flag = TIFF_ADOBE_DEFLATE;
	check  = FreeImage_LoadFromMemory(Format, hmem, Flag);
	FreeImage_CloseMemory(hmem);
	imageAllocated(check, 8);

	return check;
}

HANDLE WriteDIBToMem (HDIB32 hDib,int Format, int flags,LPINT pSize)
{
	FIBITMAP *dib = hDib;
	FIMEMORY *hmem = NULL; 
	BOOL	rtn;
	HANDLE	hMem;
	LPBYTE	pMem,mem_buffer;
	DWORD	size_in_bytes;

	hmem = FreeImage_OpenMemory(0,0);
	rtn=FreeImage_SaveToMemory(Format, dib, hmem, flags);
	FreeImage_AcquireMemory(hmem, &mem_buffer, &size_in_bytes);
	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,size_in_bytes);
	pMem = GlobalLock (hMem);
	memmove (pMem,mem_buffer,size_in_bytes);
	*pSize = size_in_bytes;
	FreeImage_CloseMemory(hmem);
	GlobalUnlock (hMem);
	return hMem;
}
/*	BOOL	rtn;

	// load a regular file
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(lpszPathName);
	FIBITMAP *dib = FreeImage_Load(fif, lpszPathName, 0);
	
	// open a memory handle
	hmem = FreeImage_OpenMemory();

	// save the file to memory
	rtn=FreeImage_SaveToMemory(fif, dib, hmem, 0);

	// at this point, hmem contains the entire PNG data in memory. 
	// the amount of space used by the memory is equal to file_size
	long file_size = FreeImage_TellMemory(hmem);
	//printf("File size : %ld\n", file_size);


	// its easy load an image from memory as well

	// seek to the start of the memory stream
	FreeImage_SeekMemory(hmem, 0L, SEEK_SET);
	
	// get the file type
	FREE_IMAGE_FORMAT mem_fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
	
	// load an image from the memory handle 
	FIBITMAP *check = FreeImage_LoadFromMemory(mem_fif, hmem, 0);

	// save as a regular file
	FreeImage_Save(FIF_PNG, check, "dump.png", PNG_DEFAULT);

	// make sure to free the data since FreeImage_SaveToMemory 
	// will cause it to be malloc'd
	FreeImage_CloseMemory(hmem);

	FreeImage_Unload(check);
	FreeImage_Unload(dib);*/

#else


DWORD GMFIBMPHandleFromEXT (LPSTR PathName)
{
	OFSTRUCTGM	OFStruct;
	HFILE Fid = OpenFileGM (PathName,&OFStruct,OF_READ);
	HANDLE	handle;

	if (Fid == HFILE_ERROR)
		return FALSE;
	handle = ReadDIBFile(Fid);
	return (DWORD)handle;
}

DWORD GMFIBMPGetBitmapInfo (LPBITMAPINFOHEADER pDibInfoD, DWORD hDib)
{
	LPBITMAPINFOHEADER	pDibInfo = GlobalLock ((HANDLE)hDib);

	*pDibInfoD = *pDibInfo;
	GlobalUnlock ((HANDLE)hDib);
	return TRUE;

}

DWORD GM32StretchDIBitsFromHandle (HDC hDC16,DWORD destX,DWORD destY,DWORD destW,DWORD destH,DWORD xoff,
						 DWORD yoff,DWORD sourcew, DWORD sourceh, HANDLE Handle,DWORD ColorType,DWORD RasterOpt,DWORD pFactorD)
{
	HDC	hDC= hDC16;
	DWORD	rtn, OldMode=0;
	LPBYTE	pImage;
	LPBITMAPINFOHEADER	pDibInfo;
	double	Factor = *(double*)pFactorD;
	POINT	pt;

	pDibInfo = FreeImage_GetInfoHeader((FIBITMAP *)Handle);
	pImage = (LPBYTE)pDibInfo + pDibInfo->biSize + pDibInfo->biClrUsed * sizeof(RGBQUAD);
//	MessageBox (0,"Made it in",NULL,MB_ICONEXCLAMATION);
	if (RasterOpt == SRCCOPY)
	{
		OldMode = SetStretchBltMode(hDC,HALFTONE); 
		SetBrushOrgEx (hDC,0,0,&pt);
	}
	rtn=StretchDIBits (hDC,destX,destY,
	                   destW, destH,
	                   xoff,yoff,
	                   IDNINT(sourcew*Factor),
	                   IDNINT(sourceh*Factor),
	                   pImage,
	                  (LPBITMAPINFO)pDibInfo,
	                  (UINT)ColorType,
	                  (DWORD) RasterOpt);
	if (OldMode)
		SetStretchBltMode(hDC,OldMode);
	return rtn;
}


BOOL GMFIBMPFileFromEXT (LPSTR lpszPathName,LPSTR ToFile,double factor)
{
	return FALSE;
}
BOOL GMFIBMPFromEXT (LPSTR lpszPathName,LPSTR ToMem,DWORD * pSize)
{
	return FALSE;
}
BOOL GMFIBMPToEXT (LPSTR lpszPathName,LPSTR FromMem,DWORD * pSize,DWORD Flag)
{
	return FALSE;
}
DWORD GMFIBMPUnload (DWORD dibdw)
{
	GlobalFree ((HGLOBAL)dibdw);
	return TRUE;
}
#endif