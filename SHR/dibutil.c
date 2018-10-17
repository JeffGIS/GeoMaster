//**********************************************************************
//
//  dibutil.c
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  CreateDIB()         - Creates new DIB
//  FindDIBBits()       - Sets pointer to the DIB bits
//  DIBWidth()          - Gets the width of the DIB
//  DIBHeight()         - Gets the height of the DIB
//  PaletteSize()       - Calculates the buffer size required by a palette
//  DIBNumColors()      - Calculates number of colors in the DIB's color table
//  CreateDIBPalette()  - Creates a palette from a DIB
//  DIBToBitmap()       - Creates a bitmap from a DIB
//  BitmapToDIB()       - Creates a DIB from a bitmap
//  PalEntriesOnDevice()- Gets the number of palette entries of a device
//  GetSystemPalette()  - Returns a handle to the current system palette
//  AllocRoomForDIB()   - Allocates memory for a DIB
//  ChangeDIBFormat()   - Changes a DIB's BPP and/or compression format
//  ChangeBitmapFormat()- Changes a bitmap to a DIB with specified BPP and
//                        compression format
//
// Development Team: Mark Bader
//                   Patrick Schreiber
//                   Garrett McAuliffe
//                   Eric Flo
//                   Tony Claflin
//
// Written by Microsoft Product Support Services, Developer Support.
// COPYRIGHT:
//
//   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
//
//   You have a royalty-free right to use, modify, reproduce and
//   distribute the Sample Files (and/or any modified version) in
//   any way you find useful, provided that you agree that
//   Microsoft has no warranty obligations or liability for any
//   Sample Application Files which are modified.
//
//**********************************************************************

/* header files */    
#include "shr.h" 
#include "bigmemln.h"
#include "bigmem.h"
#include "dibapi.h"
#include "dibutil.h"

#include "gmextern.h"      
#define MAXBMPSIZETOCACHE	1024L * 256L
static	BOOL	FirstBMPCache=TRUE; 
static	char		BMPNames[MAXBMPCACHE][MAX_PATH];
static	HANDLE		BMPHandles[MAXBMPCACHE];
static	ULONG		BMPLastUse[MAXBMPCACHE], BMPNextUse=0;
static	BOOL	FirstBMPCache32=TRUE; 
static	char		BMPNames32[MAXBMPCACHE][MAX_PATH];
static	HDIB32		BMPHandles32[MAXBMPCACHE];
static	ULONG		BMPLastUse32[MAXBMPCACHE], BMPNextUse32=0;
static	int			imageFileRotation=0;
static	int			maxBMP32Cache = MAXBMPCACHE;

void SetImageFileRotation (int r)
{
	imageFileRotation = r;
	return;
}

BOOL SetIconColors(LPSTR BitmapPath, COLORREF *colors, int ncolors)
{
	int height, width;
	RGBQUAD white;
	HDIB32 hDIB = BMPHandleFromEXT(BitmapPath);
	if (!hDIB)
		return FALSE;
	ncolors++;
	white.rgbRed = 255;
	white.rgbGreen = 255;
	white.rgbBlue = 255;
	white.rgbReserved = 0;
	RGBQUAD *rgbcolors = malloc(ncolors * sizeof(RGBQUAD)+4);
	double *dist = malloc(ncolors * sizeof(double)+4);

	rgbcolors[0] = white;
	for (int i = 1; i < ncolors;i++)
		rgbcolors[i] = RGBQUADFromCOLORREF(colors[i-1]);
	GetDIBDimensionsFromHandle(hDIB, &height, &width);
	for (int row = 0; row < height; row++)
	{
		ii = 1;
		for (int col = 0; col < width; col++)
		{
			RGBQUAD	c;

			FreeImage_GetPixelColor(hDIB, col, row, &c);
			for (int i = 0; i < ncolors; i++)
			{
				dist[i] = RGBQUADDist(c, rgbcolors[i]);
			}
			int mini = 0;
			double mindist = dist[0];
			for (int i = 1; i < ncolors;i++)
			{
				if (dist[i] < mindist)
				{
					mindist = dist[i];
					mini = i;
				}
			}
			FreeImage_SetPixelColor(hDIB, col, row, &rgbcolors[mini]);

		}
	}
	free(rgbcolors);
	free(dist);
	SaveDIB32(hDIB, BitmapPath, -1, 0);
	DestroyDIB32(hDIB, TRUE);
}

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask, hBM1, hBM2;
    BITMAP bm;

    // Create monochrome (1 bit) mask bitmap.  

    GetObject(hbmColour, sizeof(BITMAP), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

    // Get some HDCs that are compatible with the display driver

    hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);

    hBM1 = SelectObject (hdcMem, hbmColour);
    hBM2 = SelectObject (hdcMem2, hbmMask);

    // Set the background colour of the colour image to the colour
    // you want to be transparent.
    SetBkColor(hdcMem, crTransparent);

    // Copy the bits from the colour image to the B+W mask... everything
    // with the background colour ends up white while everythig else ends up
    // black...Just what we wanted.

    BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    // Take our new mask and use it to turn the transparent colour in our
    // original colour image to black so the transparency effect will
    // work right.
    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

    // Clean up.
	SelectObject (hdcMem,hBM1);
	SelectObject (hdcMem2,hBM2);
    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);

    return hbmMask;
}


/*************************************************************************
 *
 * CreateDIB()
 *
 * Parameters:
 *
 * DWORD dwWidth    - Width for new bitmap, in pixels
 * DWORD dwHeight   - Height for new bitmap 
 * WORD  wBitCount  - Bit Count for new DIB (1, 4, 8, or 24)
 *
 * Return Value:
 *
 * HDIB             - Handle to new DIB
 *
 * Description:
 *
 * This function allocates memory for and initializes a new DIB by
 * filling in the BITMAPINFOHEADER, allocating memory for the color
 * table, and allocating memory for the bitmap bits.  As with all
 * HDIBs, the header, colortable and bits are all in one contiguous
 * memory block.  This function is similar to the CreateBitmap() 
 * Windows API.
 *
 * The colortable and bitmap bits are left uninitialized (zeroed) in the
 * returned HDIB.
 *
 *
 * History:   Date      Author              Reason
 *            3/20/92   Mark Bader          Created
 *
 ************************************************************************/

HDIB FAR CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBitCount)
{
   BITMAPINFOHEADER bi;         // bitmap header
   LPBITMAPINFOHEADER lpbi;     // pointer to BITMAPINFOHEADER
   DWORD dwLen;                 // size of memory block
   HDIB hDIB;
   DWORD dwBytesPerLine;        // Number of bytes per scanline


   // Make sure bits per pixel is valid
   if (wBitCount <= 1)
      wBitCount = 1;
   else if (wBitCount <= 4)
      wBitCount = 4;
   else if (wBitCount <= 8)
      wBitCount = 8;
   else if (wBitCount <= 24)
      wBitCount = 24;
   else
      wBitCount = 4;  // set default value to 4 if parameter is bogus

   // initialize BITMAPINFOHEADER
   bi.biSize = sizeof(BITMAPINFOHEADER);
   bi.biWidth = dwWidth;         // fill in width from parameter
   bi.biHeight = dwHeight;       // fill in height from parameter
   bi.biPlanes = 1;              // must be 1
   bi.biBitCount = wBitCount;    // from parameter
   bi.biCompression = BI_RGB;    
   bi.biSizeImage = 0;           // 0's here mean "default"
   bi.biXPelsPerMeter = 0;
   bi.biYPelsPerMeter = 0;
   bi.biClrUsed = 0;
   bi.biClrImportant = 0;

   // calculate size of memory block required to store the DIB.  This
   // block should be big enough to hold the BITMAPINFOHEADER, the color
   // table, and the bits

   dwBytesPerLine = WIDTHBYTES(wBitCount * dwWidth);
   dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + (dwBytesPerLine * dwHeight);

   // alloc memory block to store our bitmap
   hDIB = GSSiGlobAlloc(1761,GHND, dwLen);

   // major bummer if we couldn't get memory block
   if (!hDIB)
   {
      return NULL;
   }

   // lock memory and get pointer to it
   lpbi = (VOID FAR *)GlobalLock(hDIB);

   // use our bitmap info structure to fill in first part of
   // our DIB with the BITMAPINFOHEADER
   *lpbi = bi;

   // Since we don't know what the colortable and bits should contain,
   // just leave these blank.  Unlock the DIB and return the HDIB.

   GlobalUnlock(hDIB);

   /* return handle to the DIB */
   return hDIB;
}



/*************************************************************************
 *
 * FindDIBBits()
 *
 * Parameter:
 *
 * LPSTR lpDIB      - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * LPSTR            - pointer to the DIB bits
 *
 * Description:
 *
 * This function calculates the address of the DIB's bits and returns a
 * pointer to the DIB bits.
 *
 * History:   Date      Author              Reason
 *            6/01/91   Garrett McAuliffe   Created
 *            9/15/91   Patrick Schreiber   Added header and comments
 *
 ************************************************************************/


LPSTR FAR FindDIBBits(LPSTR lpDIB)
{
   return (lpDIB + *(LPDWORD)lpDIB + PaletteSize(lpDIB));
}


/*************************************************************************
 *
 * DIBWidth()
 *
 * Parameter:
 *
 * LPSTR lpDIB      - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - width of the DIB
 *
 * Description:
 *
 * This function gets the width of the DIB from the BITMAPINFOHEADER
 * width field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * width field if it is an OS/2-style DIB.
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *
 ************************************************************************/


DWORD FAR DIBWidth(LPSTR lpDIB)
{
   LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
   LPBITMAPCOREHEADER lpbmc;  // pointer to an OS/2-style DIB

   /* point to the header (whether Win 3.0 and OS/2) */

   lpbmi = (LPBITMAPINFOHEADER)lpDIB;
   lpbmc = (LPBITMAPCOREHEADER)lpDIB;

   /* return the DIB width if it is a Win 3.0 DIB */
   if (lpbmi->biSize == sizeof(BITMAPINFOHEADER))
      return lpbmi->biWidth;
   else  /* it is an OS/2 DIB, so return its width */
      return (DWORD)lpbmc->bcWidth;
}


/*************************************************************************
 *
 * DIBHeight()
 *
 * Parameter:
 *
 * LPSTR lpDIB      - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - height of the DIB
 *
 * Description:
 *
 * This function gets the height of the DIB from the BITMAPINFOHEADER
 * height field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * height field if it is an OS/2-style DIB.
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *
 ************************************************************************/


DWORD FAR DIBHeight(LPSTR lpDIB)
{
   LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
   LPBITMAPCOREHEADER lpbmc;  // pointer to an OS/2-style DIB

   /* point to the header (whether OS/2 or Win 3.0 */

   lpbmi = (LPBITMAPINFOHEADER)lpDIB;
   lpbmc = (LPBITMAPCOREHEADER)lpDIB;

   /* return the DIB height if it is a Win 3.0 DIB */
   if (lpbmi->biSize == sizeof(BITMAPINFOHEADER))
      return lpbmi->biHeight;
   else  /* it is an OS/2 DIB, so return its height */
      return (DWORD)lpbmc->bcHeight;
}


/*************************************************************************
 *
 * PaletteSize()
 *
 * Parameter:
 *
 * LPSTR lpDIB      - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - size of the color palette of the DIB
 *
 * Description:
 *
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD (for a
 * Windows 3.0-style DIB) or by the size of an RGBTRIPLE (for an OS/2-
 * style DIB).
 *
 * History:   Date      Author             Reason
 *            6/01/91   Garrett McAuliffe  Created
 *            9/15/91   Patrick Schreiber  Added header and comments
 *
 ************************************************************************/


WORD FAR PaletteSize(LPSTR lpDIB)
{
   /* calculate the size required by the palette */
   if (IS_WIN30_DIB (lpDIB))
      return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
   else
      return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE));
}


/*************************************************************************
 *
 * DIBNumColors()
 *
 * Parameter:
 *
 * LPSTR lpDIB      - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - number of colors in the color table
 *
 * Description:
 *
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or OS/2-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *
 ************************************************************************/


WORD FAR DIBNumColors(LPSTR lpDIB)
{
   WORD wBitCount;  // DIB bit count

   /*  If this is a Windows-style DIB, the number of colors in the
    *  color table can be less than the number of bits per pixel
    *  allows for (i.e. lpbi->biClrUsed can be set to some value).
    *  If this is the case, return the appropriate value.
    */

   if (IS_WIN30_DIB(lpDIB))
   {
      DWORD dwClrUsed;

      dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
      if (dwClrUsed)
     return (WORD)dwClrUsed;
   }

   /*  Calculate the number of colors in the color table based on
    *  the number of bits per pixel for the DIB.
    */
   if (IS_WIN30_DIB(lpDIB))
      wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
   else
      wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

   /* return number of colors based on bits per pixel */
   switch (wBitCount)
      {
   case 1:
      return 2;

   case 4:
      return 16;

   case 8:
      return 256;

   default:
      return 0;
      }
}


/*************************************************************************
 *
 * CreateDIBPalette()
 *
 * Parameter:
 *
 * HDIB hDIB        - specifies the DIB
 *
 * Return Value:
 *
 * HPALETTE         - specifies the palette
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *
 ************************************************************************/


HPALETTE FAR CreateDIBPalette(HDIB hDIB)
{
   LPLOGPALETTE lpPal;      // pointer to a logical palette
   HANDLE hLogPal=NULL;          // handle to a logical palette
   HPALETTE hPal = NULL;    // handle to a palette
   int i, wNumColors;       // loop index, number of colors in color table
   LPSTR lpbi;              // pointer to packed-DIB
   LPBITMAPINFO lpbmi;      // pointer to BITMAPINFO structure (Win3.0)
   LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (OS/2)
   BOOL bWinStyleDIB;       // flag which signifies whether this is a Win3.0 DIB

   /* if handle to DIB is invalid, return NULL */

   if (!hDIB)
      return NULL;

   /* lock DIB memory block and get a pointer to it */
   lpbi = GlobalLock(hDIB);

   /* get pointer to BITMAPINFO (Win 3.0) */
   lpbmi = (LPBITMAPINFO)lpbi;

   /* get pointer to BITMAPCOREINFO (OS/2 1.x) */
   lpbmc = (LPBITMAPCOREINFO)lpbi;

   /* get the number of colors in the DIB */
   wNumColors = DIBNumColors(lpbi);

   /* is this a Win 3.0 DIB? */
   bWinStyleDIB = IS_WIN30_DIB(lpbi);
   if (wNumColors)
   {
      /* allocate memory block for logical palette */
      hLogPal = GSSiGlobAlloc(1762,GHND, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) *
                wNumColors);

      /* if not enough memory, clean up and return NULL */
      if (!hLogPal)
      {
     GlobalUnlock(hDIB);
     return NULL;
      }

      /* lock memory block and get pointer to it */
      lpPal = (LPLOGPALETTE)GlobalLock(hLogPal);

      /* set version and number of palette entries */
      lpPal->palVersion = PALVERSION;
      lpPal->palNumEntries = wNumColors;

      /*  store RGB triples (if Win 3.0 DIB) or RGB quads (if OS/2 DIB)
       *  into palette
       */
      for (i = 0; i < wNumColors; i++)
      {
     if (bWinStyleDIB)
     {
        lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
        lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
        lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
        lpPal->palPalEntry[i].peFlags = 0;
     }
     else
     {
        lpPal->palPalEntry[i].peRed = lpbmc->bmciColors[i].rgbtRed;
        lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
        lpPal->palPalEntry[i].peBlue = lpbmc->bmciColors[i].rgbtBlue;
        lpPal->palPalEntry[i].peFlags = 0;
     }
      }

      /* create the palette and get handle to it */
      hPal = CreatePalette(lpPal);

      /* if error getting handle to palette, clean up and return NULL */
      if (!hPal)
      {
     GlobalUnlock(hLogPal);
     GSSiGlobFree (&hLogPal);
     return NULL;
      }
   }

   /* clean up */ 
   if (hLogPal)
   {
	   GlobalUnlock(hLogPal);
	   GSSiGlobFree (&hLogPal);  
   }
   GlobalUnlock(hDIB);

   /* return handle to DIB's palette */
   return hPal;
}


/*************************************************************************
 *
 * DIBToBitmap()
 *
 * Parameters:
 *
 * HDIB hDIB        - specifies the DIB to convert
 *
 * HPALETTE hPal    - specifies the palette to use with the bitmap
 *
 * Return Value:
 *
 * HBITMAP          - identifies the device-dependent bitmap
 *
 * Description:
 *
 * This function creates a bitmap from a DIB using the specified palette.
 * If no palette is specified, default is used.
 *
 * NOTE:
 *
 * The bitmap returned from this funciton is always a bitmap compatible
 * with the screen (e.g. same bits/pixel and color planes) rather than
 * a bitmap with the same attributes as the DIB.  This behavior is by
 * design, and occurs because this function calls CreateDIBitmap to
 * do its work, and CreateDIBitmap always creates a bitmap compatible
 * with the hDC parameter passed in (because it in turn calls
 * CreateCompatibleBitmap).
 *
 * So for instance, if your DIB is a monochrome DIB and you call this
 * function, you will not get back a monochrome HBITMAP -- you will
 * get an HBITMAP compatible with the screen DC, but with only 2
 * colors used in the bitmap.
 *
 * If your application requires a monochrome HBITMAP returned for a
 * monochrome DIB, use the function SetDIBits().
 *
 * Also, the DIBpassed in to the function is not destroyed on exit. This
 * must be done later, once it is no longer needed.
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *            3/27/92   Mark Bader           Added comments about resulting
 *                                           bitmap format
 *
 ************************************************************************/


HBITMAP FAR DIBToBitmap(HDIB hDIB, HPALETTE hPal)
{
   LPSTR lpDIBHdr, lpDIBBits;  // pointer to DIB header, pointer to DIB bits
   HBITMAP hBitmap;            // handle to device-dependent bitmap
   HDC hDC;                    // handle to DC
   HPALETTE hOldPal = NULL;    // handle to a palette

   /* if invalid handle, return NULL */

   if (!hDIB)
      return NULL;

   /* lock memory block and get a pointer to it */
   lpDIBHdr = GlobalLock(hDIB);

   /* get a pointer to the DIB bits */
   lpDIBBits = FindDIBBits(lpDIBHdr);

   /* get a DC */
   hDC = GetDC(NULL);
   if (!hDC)
   {
      /* clean up and return NULL */
      GlobalUnlock(hDIB);
      return NULL;
   }

   /* select and realize palette */
   if (hPal)
      hOldPal = SelectPalette(hDC, hPal, FALSE);
   RealizePalette(hDC);

   /* create bitmap from DIB info. and bits */
   hBitmap = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpDIBHdr, CBM_INIT,
                lpDIBBits, (LPBITMAPINFO)lpDIBHdr, DIB_RGB_COLORS);

   /* restore previous palette */
   if (hOldPal)
      SelectPalette(hDC, hOldPal, FALSE);

   /* clean up */
   ReleaseDC(NULL, hDC);
   GlobalUnlock(hDIB);

   /* return handle to the bitmap */
   return hBitmap;
}

HBITMAP FAR DIB32ToBitmap(HDIB32 hDIB32, HPALETTE hPal)
{
   LPBITMAPINFOHEADER lpDIBHdr;
   LPSTR	lpDIBBits;		// pointer to DIB header, pointer to DIB bits
   HBITMAP hBitmap;            // handle to device-dependent bitmap
   HDC hDC;                    // handle to DC
   HPALETTE hOldPal = NULL;    // handle to a palette
   UINT colorType = DIB_RGB_COLORS;

   /* if invalid handle, return NULL */

   if (!hDIB32)
      return NULL;

   /* lock memory block and get a pointer to it */
   lpDIBHdr = FreeImage_GetInfoHeader(hDIB32);

   /* get a pointer to the DIB bits */
   lpDIBBits = FreeImage_GetBits(hDIB32);

   /* get a DC */
   hDC = GetDC (hWndMain);//GetDC(NULL);
   if (!hDC)
   {
      /* clean up and return NULL */
      return NULL;
   }

   /* select and realize palette */
   if (hPal)
      hOldPal = SelectPalette(hDC, hPal, FALSE);
   RealizePalette(hDC);

   if (lpDIBHdr->biBitCount == 8)
	   colorType = DIB_PAL_COLORS;
   /* create bitmap from DIB info. and bits */
   hBitmap = CreateDIBitmap(hDC, lpDIBHdr, CBM_INIT,
                lpDIBBits, (LPBITMAPINFO)lpDIBHdr, colorType);

   /* restore previous palette */
   if (hOldPal)
      SelectPalette(hDC, hOldPal, FALSE);

   /* clean up */
   ReleaseDC(NULL, hDC);

   /* return handle to the bitmap */
   return hBitmap;
}


/*************************************************************************
 *
 * BitmapToDIB()
 *
 * Parameters:
 *
 * HBITMAP hBitmap  - specifies the bitmap to convert
 *
 * HPALETTE hPal    - specifies the palette to use with the bitmap
 *
 * Return Value:
 *
 * HDIB             - identifies the device-dependent bitmap
 *
 * Description:
 *
 * This function creates a DIB from a bitmap using the specified palette.
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *            12/10/91  Patrick Schreiber    Added bits per pixel validation
 *                                           and check GetObject return value
 *
 ************************************************************************/


HDIB FAR BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal,LPINT plbitmap)
{
   BITMAP bm;                   // bitmap structure
   BITMAPINFOHEADER bi;         // bitmap header
   BITMAPINFOHEADER FAR *lpbi;  // pointer to BITMAPINFOHEADER
   DWORD dwLen;                 // size of memory block
   HANDLE hDIB, h;              // handle to DIB, temp handle
   HDC hDC;                     // handle to DC
   WORD biBits;                 // bits per pixel
   

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
   else /* if greater than 8-bit, force to 24-bit */
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

   /* alloc memory block to store our bitmap */
   hDIB = GSSiGlobAlloc(1763,GHND, dwLen);

   /* if we couldn't get memory block */
   if (!hDIB)
   {
      /* clean up and return NULL */
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }

   /* lock memory and get pointer to it */
   lpbi = (VOID FAR *)GlobalLock(hDIB);
   
   /* use our bitmap info. to fill BITMAPINFOHEADER */
   *lpbi = bi;

   /*  call GetDIBits with a NULL lpBits param, so it will calculate the
    *  biSizeImage field for us
    */
   GetDIBits(hDC, hBitmap, 0, (WORD)bi.biHeight, NULL, (LPBITMAPINFO)lpbi,
         DIB_RGB_COLORS);

   /* get the info. returned by GetDIBits and unlock memory block */
   bi = *lpbi;
   GlobalUnlock(hDIB);

   /* if the driver did not fill in the biSizeImage field, make one up */
   if (bi.biSizeImage == 0)
      bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

   /* realloc the buffer big enough to hold all the bits */
   dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + bi.biSizeImage;
   if (h = GSSiGlobalReAlloc(0, hDIB, dwLen, GMEM_MOVEABLE))
   {
	   hDIB = h;
	   if (plbitmap)
			*plbitmap = dwLen;
   }
   else
   {
      /* clean up and return NULL */
      GSSiGlobFree (&hDIB);
      hDIB = NULL;
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }

   /* lock memory block and get pointer to it */
   lpbi = (VOID FAR *)GlobalLock(hDIB);

   /*  call GetDIBits with a NON-NULL lpBits param, and actualy get the
    *  bits this time
    */
   if (GetDIBits(hDC, hBitmap, 0, (WORD)bi.biHeight, (LPSTR)lpbi + (WORD)lpbi
         ->biSize + PaletteSize((LPSTR)lpbi), (LPBITMAPINFO)lpbi,
         DIB_RGB_COLORS) == 0)
   {
      /* clean up and return NULL */
      GSSiGlobUlFree (&hDIB);
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }
   bi = *lpbi;

   /* clean up */
   GlobalUnlock(hDIB);
   SelectPalette(hDC, hPal, TRUE);
   RealizePalette(hDC);
   ReleaseDC(NULL, hDC);

   /* return handle to the DIB */
   return hDIB;
}

HDIB FAR BitmapToDIB2(HBITMAP hBitmap, HPALETTE hPal,LPINT piScanLine,LPINT pnScanLine)
{
   BITMAP bm;                   // bitmap structure
   BITMAPINFOHEADER bi;         // bitmap header
   BITMAPINFOHEADER FAR *lpbi;  // pointer to BITMAPINFOHEADER
   DWORD dwLen;                 // size of memory block
   HANDLE hDIB, h;              // handle to DIB, temp handle
   HDC hDC;                     // handle to DC
   WORD biBits;                 // bits per pixel
   DWORD	iScanLineBeg, iScanLineEnd;
   int	n=0, nLines;
   LPSTR	pImage;
   

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
   else /* if greater than 8-bit, force to 24-bit */
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

   /* alloc memory block to store our bitmap */
   hDIB = GSSiGlobAlloc(1764,GHND, dwLen);

   /* if we couldn't get memory block */
   if (!hDIB)
   {
      /* clean up and return NULL */
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }

   /* lock memory and get pointer to it */
   lpbi = (VOID FAR *)GlobalLock(hDIB);
   
   /* use our bitmap info. to fill BITMAPINFOHEADER */
   *lpbi = bi;

   /*  call GetDIBits with a NULL lpBits param, so it will calculate the
    *  biSizeImage field for us
    */
   GetDIBits(hDC, hBitmap, 0, (WORD)bi.biHeight, NULL, (LPBITMAPINFO)lpbi,
         DIB_RGB_COLORS);

   if (!*pnScanLine)
	   *pnScanLine = bm.bmHeight;
  /* get the info. returned by GetDIBits and unlock memory block */
   bi = *lpbi;
   GlobalUnlock(hDIB);

Getmem:
   bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * (*pnScanLine);
   bi.biHeight = *pnScanLine;
   /* realloc the buffer big enough to hold all the bits */
   dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + bi.biSizeImage;
   if (h = GSSiGlobalReAlloc (0,hDIB, dwLen, GMEM_MOVEABLE))
      hDIB = h;
   else
   {
	   if (n++ < 8)
	   {
		  *pnScanLine /= 2;
		  goto Getmem;
	   }
      GSSiGlobFree (&hDIB);
      hDIB = NULL;
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }
   /* lock memory block and get pointer to it */
   lpbi = (VOID FAR *)GlobalLock(hDIB);
   lpbi->biSizeImage = bi.biSizeImage;
   /*  call GetDIBits with a NON-NULL lpBits param, and actualy get the
    *  bits this time
    */
   iScanLineBeg = *piScanLine;
   iScanLineEnd = iScanLineBeg + *pnScanLine - 1;
   pImage = FindDIBBits ((LPSTR)lpbi);
   if ((nLines = GetDIBits(hDC, hBitmap, iScanLineBeg, *pnScanLine, pImage, (LPBITMAPINFO)lpbi,
         DIB_RGB_COLORS)) == 0)
   {
      /* clean up and return NULL */
      GSSiGlobUlFree (&hDIB);
      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
   }
   lpbi->biHeight = bi.biHeight;
   bi = *lpbi;

   /* clean up */
   GlobalUnlock(hDIB);
   SelectPalette(hDC, hPal, TRUE);
   RealizePalette(hDC);
   ReleaseDC(NULL, hDC);

   /* return handle to the DIB */
   return hDIB;
}


/*************************************************************************
 *
 * PalEntriesOnDevice()
 *
 * Parameter:
 *
 * HDC hDC          - device context
 *
 * Return Value:
 *
 * int              - number of palette entries on device
 *
 * Description:
 *
 * This function gets the number of palette entries on the specified device
 *
 * History:   Date      Author               Reason
 *            6/01/91   Garrett McAuliffe    Created
 *            9/15/91   Patrick Schreiber    Added header and comments
 *
 ************************************************************************/


int FAR PalEntriesOnDevice(HDC hDC)
{
   int nColors;  // number of colors

   /*  Find out the number of palette entries on this
    *  device.
    */

   nColors = GetDeviceCaps(hDC, SIZEPALETTE);

   /*  For non-palette devices, we'll use the # of system
    *  colors for our palette size.
    */
   if (!nColors)
      nColors = GetDeviceCaps(hDC, NUMCOLORS);
   assert(nColors);
   return nColors;
}


/*************************************************************************
 *
 * GetSystemPalette()
 *
 * Parameters:
 *
 * None
 *
 * Return Value:
 *
 * HPALETTE         - handle to a copy of the current system palette
 *
 * Description:
 *
 * This function returns a handle to a palette which represents the system
 * palette.  The system RGB values are copied into our logical palette using
 * the GetSystemPaletteEntries function.  
 *
 * History:   
 *            
 *    Date      Author               Reason        
 *    6/01/91   Garrett McAuliffe    Created        
 *    9/15/91   Patrick Schreiber    Added header and comments
 *    12/20/91  Mark Bader           Added GetSystemPaletteEntries call
 *
 ************************************************************************/


HPALETTE FAR GetSystemPalette(void)
{
   HDC hDC;                // handle to a DC
   static HPALETTE hPal = NULL;   // handle to a palette
   HANDLE hLogPal;         // handle to a logical palette
   LPLOGPALETTE lpLogPal;  // pointer to a logical palette
   int nColors;            // number of colors

   /* Find out how many palette entries we want. */

   hDC = GetDC(NULL);
   if (!hDC)
      return NULL;
   nColors = PalEntriesOnDevice(hDC);   // Number of palette entries
   if (nColors <= 0)
   	return NULL;
   /* Allocate room for the palette and lock it. */
   hLogPal = GSSiGlobAlloc(1765,GHND, sizeof(LOGPALETTE) + nColors * sizeof(
             PALETTEENTRY));

   /* if we didn't get a logical palette, return NULL */
   if (!hLogPal)
      return NULL;

   /* get a pointer to the logical palette */
   lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal);

   /* set some important fields */
   lpLogPal->palVersion = PALVERSION;
   lpLogPal->palNumEntries = nColors;

   /* Copy the current system palette into our logical palette */

   GetSystemPaletteEntries(hDC, 0, nColors, 
                           (LPPALETTEENTRY)(lpLogPal->palPalEntry));

   /*  Go ahead and create the palette.  Once it's created,
    *  we no longer need the LOGPALETTE, so free it.
    */

   hPal = CreatePalette(lpLogPal);

   /* clean up */
   GlobalUnlock(hLogPal);
   GSSiGlobFree (&hLogPal);
   ReleaseDC(NULL, hDC);

   return hPal;
}



/*************************************************************************
 *
 * ChangeDIBFormat()
 *
 * Parameter:
 *
 * HDIB             - handle to packed-DIB in memory
 *
 * WORD             - desired bits per pixel
 *
 * DWORD            - desired compression format
 *
 * Return Value:
 *
 * HDIB             - handle to the new DIB if successful, else NULL
 *
 * Description:
 *
 * This function will convert the bits per pixel and/or the compression
 * format of the specified DIB. Note: If the conversion was unsuccessful,
 * we return NULL. The original DIB is left alone. Don't use code like the
 * following:
 *
 *    hMyDIB = ChangeDIBFormat(hMyDIB, 8, BI_RLE4);
 *
 * The conversion will fail, but hMyDIB will now be NULL and the original
 * DIB will now hang around in memory. We could have returned the old
 * DIB, but we wanted to allow the programmer to check whether this
 * conversion succeeded or failed.
 *
 * History:   
 *            
 *   Date      Author             Reason         
 *   6/01/91   Garrett McAuliffe  Created         
 *   12/10/91  Patrick Schreiber  Modified from converting RGB to RLE8        
 *                                  to converting RGB/RLE to RGB/RLE.         
 *                                  Added wBitCount and dwCompression         
 *                                  parameters. Also added header and         
 *                                  comments.         
 *
 ************************************************************************/


/*************************************************************************
 *
 * ChangeBitmapFormat()
 *
 * Parameter:
 *
 * HBITMAP          - handle to a bitmap
 *
 * WORD             - desired bits per pixel
 *
 * DWORD            - desired compression format
 *
 * HPALETTE         - handle to palette
 *
 * Return Value:
 *
 * HDIB             - handle to the new DIB if successful, else NULL
 *
 * Description:
 *
 * This function will convert a bitmap to the specified bits per pixel
 * and compression format. The bitmap and it's palette will remain
 * after calling this function.
 *
 * History:   
 *            
 *   Date      Author             Reason         
 *   6/01/91   Garrett McAuliffe  Created         
 *   12/10/91  Patrick Schreiber  Modified from converting RGB to RLE8         
 *                                 to converting RGB/RLE to RGB/RLE.         
 *                                 Added wBitCount and dwCompression         
 *                                 parameters. Also added header and         
 *                                 comments.         
 *   12/11/91  Patrick Schreiber  Destroy old DIB if conversion was         
 *                                 successful.         
 *   12/16/91  Patrick Schreiber  Modified from converting DIB to new         
 *                                 DIB to bitmap to new DIB. Added palette
 *                                 parameter.
 *
 ************************************************************************/

void ComputeBMLoc (RECT Rect,LPBITMAPINFO pDibInfo,short MaintainAspect)
#if ENABLETRACE
{GSSiEnterProg (397);
#endif
{    float  pw, ph, fac;

     if (!pDibInfo)
{
#if ENABLETRACE
GSSiExitProg (397);
#endif
     	return;
}    
	 if (abs(MaintainAspect) == 2)   
	 {
	     rect_width = Rect.right - Rect.left;
	     rect_height = Rect.bottom - Rect.top;
	     ph =  rect_height / (float)pDibInfo->bmiHeader.biHeight;
	     pw =  rect_width / (float)pDibInfo->bmiHeader.biWidth;
	     ph = min (ph,pw);
     	 fac = 1;
	     
	     if (ph > 1) 
	     	fac = (long)ph;
	     else if (ph > 0) 
	     {
	     	while (ph*fac < 1) 
	     		fac *= 2;
	     	fac = 1/fac;
	     }	
         destH = pDibInfo->bmiHeader.biHeight * fac;
         destW = pDibInfo->bmiHeader.biWidth * fac;
         destX = (int)(Rect.left + (rect_width - destW) / 2); 
         if (MaintainAspect < 0) 
         	destY = Rect.top;
         else
         	destY =  Rect.top + (rect_height - destH) / 2;   
	 }
	 else
	 {
	 
	     rect_width = Rect.right - Rect.left + 1;
	     rect_height = Rect.bottom - Rect.top + 1;
	     ph = (float)pDibInfo->bmiHeader.biHeight / rect_height;
	     pw = (float)pDibInfo->bmiHeader.biWidth / rect_width;
	     if (ph > pw)
	        {destY = (int)Rect.top;
	         destH = (int)rect_height;
	         destW = (int)(IDNINT(destH * ((double)pDibInfo->bmiHeader.biWidth /
	                          (double)pDibInfo->bmiHeader.biHeight)));
	         destX = (int)(Rect.left + (rect_width - destW) / 2);}
	     else
	        {destX = (int)Rect.left;
	         destW = (int)rect_width;
	         destH = (int)(IDNINT(destW * ((double)pDibInfo->bmiHeader.biHeight /
	                          (double)pDibInfo->bmiHeader.biWidth)));
	         destY = (int)(Rect.top + (rect_height - destH) / 2);}   
     }
     bmX = bmY = 0;
{
#if ENABLETRACE
GSSiExitProg (397);
#endif
     return;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetDIBDimensionsFromHandle(HDIB32	hDib32,LPINT height,LPINT width)
#if ENABLETRACE
{GSSiEnterProg (398);
#endif
{
    BITMAPINFOHEADER    DibInfo; 
	BOOL	rtn;
    
	rtn = GetBitmapInfoFromHandle (&DibInfo,hDib32);
	*height = DibInfo.biHeight;
	*width = DibInfo.biWidth;
{
#if ENABLETRACE
GSSiExitProg (398);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GetDIBDimensions(LPSTR File,LPINT height,LPINT width)
#if ENABLETRACE
{GSSiEnterProg (398);
#endif
{
	HDIB32	hDib32;
    BITMAPINFOHEADER    DibInfo; 
	BOOL	rtn=FALSE;
    
	if ((hDib32 = LoadDIB32(File,FALSE)))
	{
		rtn = TRUE;
	    GetBitmapInfoFromHandle (&DibInfo,hDib32);
		*height = DibInfo.biHeight;
		*width = DibInfo.biWidth;
		DestroyDIB32(hDib32,FALSE);
	}
{
#if ENABLETRACE
GSSiExitProg (398);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

HANDLE GetBMPFromCache (LPSTR Name)
{   
	USHORT	i;  
	
	if (!AllowBMPCaching)
		return 0;
	if (FirstBMPCache)
	{   
		FirstBMPCache = FALSE;
		for (i=0;i<MAXBMPCACHE;i++)
			*BMPNames[i] = 0;  
		return 0;
	}
	
	for (i=0;i<MAXBMPCACHE;i++)  
	{
		if (!_fstricmp (Name,BMPNames[i])) 
		{   
			BMPLastUse[i] = BMPNextUse++;
			return BMPHandles[i];
		}
	}
	return 0;
} 

BOOL hDIBInCache (HANDLE hBMP)
{   
	USHORT	i;  
	
	if (FirstBMPCache)
		return FALSE;
	
	for (i=0;i<MAXBMPCACHE;i++)  
	{
		if (*BMPNames[i] && BMPHandles[i] == hBMP)
			return TRUE;
	}
	return FALSE;
} 

void AddBMPToCache (LPSTR Name,HANDLE hBMP)
{   
	ULONG	MinUse=ULONG_MAX;
	USHORT	Mini, i;
	LPBITMAPINFOHEADER  pDibInfo; 
	long	BMPSize;
	
	if (!Name)
	{
		if (!FirstBMPCache)
			for (i=0;i<MAXBMPCACHE;i++)  
			{
				if (*BMPNames[i]) 
					GSSiGlobFree (&BMPHandles[i]);  
				*BMPNames[i] = 0;
			}
		FirstBMPCache = TRUE; 
		if (hBMP)  
		{
			AllowCache = FALSE;
	        AllowBMPCaching = FALSE; 
	    }
		return;
	}
	
	if (!hBMP)
		return;  
	
	if (!AllowBMPCaching)
		return;
		
	if (FirstBMPCache)
	{   
		FirstBMPCache = FALSE;
		for (i=0;i<MAXBMPCACHE;i++)
			*BMPNames[i] = 0;  
	}
	pDibInfo = (LPBITMAPINFOHEADER)GlobalLock (hBMP);
	BMPSize = pDibInfo->biHeight * pDibInfo->biWidth * pDibInfo->biBitCount/8;
    GlobalUnlock (hBMP);
	if (BMPSize > MAXBMPSIZETOCACHE)
		return;
	for (i=0;i<MAXBMPCACHE;i++)  
	{
		if (!*BMPNames[i]) 
		{   
			BMPLastUse[i] = BMPNextUse++;
			BMPHandles[i] = hBMP;
			_fstrcpy (BMPNames[i],Name);
			return;
		}
		if (BMPLastUse[i] < MinUse)
		{
			MinUse = BMPLastUse[i];
			Mini = i;              
		}
	}
	GSSiGlobFree (&BMPHandles[Mini]);
	BMPLastUse[Mini] = BMPNextUse++;
	BMPHandles[Mini] = hBMP;
	_fstrcpy (BMPNames[Mini],Name);
	return;
}
	

HDIB32 GetBMPFromCache32 (LPSTR Name)
{   
	USHORT	i;  
	
	if (!AllowBMPCaching)
		return 0;
	if (FirstBMPCache32)
	{   
		FirstBMPCache32 = FALSE;
		for (i=0;i<MAXBMPCACHE;i++)
			*BMPNames32[i] = 0;  
		return 0;
	}
	
	for (i=0;i<MAXBMPCACHE;i++)  
	{
		if (!_fstricmp (Name,BMPNames32[i])) 
		{   
			BMPLastUse32[i] = BMPNextUse32++;
			return BMPHandles32[i];
		}
	}
	return 0;
} 

BOOL DestroyDIB32(HDIB32 hDib,BOOL Force)
{ 
	if (!hDib)
		return TRUE;
   if (!Force && hDIBInCache32 (hDib))
		return TRUE;//allows caching
   GMDestroyDIB32 (hDib); 
   hDib = 0;
   return TRUE;
}

BOOL hDIBInCache32 (HDIB32 hBMP)
{   
	USHORT	i;  
	
	if (FirstBMPCache32)
		return FALSE;
	
	for (i=0;i<MAXBMPCACHE;i++)  
	{
		if (*BMPNames32[i] && BMPHandles32[i] == hBMP)
			return TRUE;
	}
	return FALSE;
} 

BOOL RemoveBMPFromCache32 (LPSTR Name)
{   
	UINT	i;
	
	if (FirstBMPCache32)  
		return FALSE;
	for (i=0;i<MAXBMPCACHE;i++)  
	{
		if (!_fstricmp (Name,BMPNames32[i]))
		{ 
			DestroyDIB32 (BMPHandles32[i],TRUE); 
			BMPHandles32[i] = 0; 
			*BMPNames32[i] = 0;
			return TRUE;
		}
	}
    return FALSE;
}

void AddBMPToCache32 (LPSTR Name,HDIB32 hBMP)
{   
	ULONG	MinUse=ULONG_MAX;
	USHORT	Mini, i;
	
	if (!Name)
	{
		if (!FirstBMPCache32)
			for (i=0;i<MAXBMPCACHE;i++)  
			{
				if (*BMPNames32[i]) 
					DestroyDIB32 (BMPHandles32[i],TRUE); 
				BMPHandles32[i] = 0; 
				*BMPNames32[i] = 0;
			}
		FirstBMPCache32 = TRUE; 
		if ((int)hBMP < 0)  
		{
			AllowCache = FALSE;
	        AllowBMPCaching = FALSE; 
	    }
		if ((int)hBMP > 0)
			maxBMP32Cache = (int)hBMP;
		return;
	}
	
	if (!hBMP)
		return;  
	
	if (!AllowBMPCaching)
		return;
		
	if (FirstBMPCache32)
	{   
		FirstBMPCache32 = FALSE;
		for (i=0;i<MAXBMPCACHE;i++)
			*BMPNames32[i] = 0;  
	}
	for (i=0;i<maxBMP32Cache;i++)  
	{
		if (!*BMPNames32[i]) 
		{   
			BMPLastUse32[i] = BMPNextUse++;
			BMPHandles32[i] = hBMP;
			_fstrcpy (BMPNames32[i],Name);
			return;
		}
		if (BMPLastUse32[i] < MinUse)
		{
			MinUse = BMPLastUse32[i];
			Mini = i;              
		}
	}
	DestroyDIB32  (BMPHandles32[Mini],TRUE); 
	BMPLastUse32[Mini] = BMPNextUse32++;
	BMPHandles32[Mini] = hBMP;
	_fstrcpy (BMPNames32[Mini],Name);
	return;
}


LPBITMAPINFO GetDibHeader (HDIB32 hDib)
{   
    static BITMAPINFOHEADER DibInfo;  
	LPBITMAPINFO	pDibInfo;

	if (hDibIs32Bit (hDib))
	{ 
	    GetBitmapInfoFromHandle (&DibInfo,hDib);
	    pDibInfo = (LPBITMAPINFO)&DibInfo;
	}
	else
		pDibInfo = (LPBITMAPINFO)GlobalLock ((HANDLE)hDib);
	return pDibInfo;
}

void hDibFree (LPHDIB32 phDib)
{   
	if (!*phDib)
		return;
	if (hDibIs32Bit (*phDib)) 
		GMDestroyDIB32 (*phDib);
	else
	{
		HANDLE	handle = (HANDLE)*phDib;

		GSSiGlobFree (&handle);
	}
	*phDib = 0;
	return;
}
	
BOOL LoadBitMap (LPSTR ImageFile, long frame, LPHDIB32 phDib, LPSHORT pDeleteBM,
                 LPSHORT BitCount,LPSHORT width, LPSHORT height)
#if ENABLETRACE
{GSSiEnterProg (399);
#endif
{   UINT nRead;
    HFILE  Fid;
	OFSTRUCTGM    fStruct;
	LPOFSTRUCTGM  pStruct = &fStruct;
    BITMAPFILEHEADER bmfHead;
    LPBITMAPINFO    pDibInfo;
    LPSTR pImage;
    long	nBytes; 
    short	ShouldDelete;    
    BOOL	rtn=TRUE;
    char	NewName[MAX_PATH];

    if (!ImageFile[0])
{
#if ENABLETRACE
GSSiExitProg (399);
#endif
    	return (FALSE); 
}
    _fstrcpy (NewName,ImageFile);
    //ConvertNameToCDName (NewName);
    if (frame >= 0)
    {   
        HANDLE hDib;
                
        rtn = AVIFrameToDIB (NewName,frame,&hDib,pDeleteBM,*BitCount,*width,*height);
        *phDib = hDib;
    }
    else// if (_fstrstr (NewName,".pcx") || _fstrstr (NewName,".tif") || _fstrstr (NewName,".bmp") || _fstrstr (NewName,".jpg"))
    {
    	if (!(*phDib = LoadDIB32 (NewName,TRUE))) 
			return FALSE;
    	*pDeleteBM = 2;  
    	if (!*width)
    	{
	        pDibInfo = GetDibHeader (*phDib);
	        *width = pDibInfo->bmiHeader.biWidth;
	        *height = pDibInfo->bmiHeader.biHeight;
	        if (!hDibIs32Bit (*phDib))
	        	GlobalUnlock ((HANDLE)*phDib);
	    }
    }
/*    else 
    {
        Fid = GSSiOpenFile (NewName,pStruct,OF_READ); 
        if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (399);
#endif
        	return FALSE;
}
        nRead = BigRead (Fid,(HPSTR)&bmfHead,sizeof(BITMAPFILEHEADER));
        nBytes = bmfHead.bfSize-sizeof(BITMAPFILEHEADER);
        *phDib = GSSiGlobAlloc (1401,GMEM_MOVEABLE,nBytes);
        pDibInfo = GetDibHeader (*phDib);
    
        BigRead (Fid,pDibInfo,nBytes);   
        if (! pDibInfo->bmiHeader.biSizeImage)
        	pDibInfo->bmiHeader.biSizeImage= bmfHead.bfSize-bmfHead.bfOffBits;
        if (! pDibInfo->bmiHeader.biClrUsed)
            {
            if (pDibInfo->bmiHeader.biBitCount==4) pDibInfo->bmiHeader.biClrUsed=16;
            if (pDibInfo->bmiHeader.biBitCount==8) pDibInfo->bmiHeader.biClrUsed=256;
            }
        GSSiClose (Fid); 
        if (!hDibIs32Bit (*phDib))
        	GlobalUnlock ((HANDLE)*phDib);
    } */   
{
#if ENABLETRACE
GSSiExitProg (399);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL ReadBitMapHeader (HFILE Fid, LPHANDLE phDibInfo, LPLONG ImageOffset)
#if ENABLETRACE
{GSSiEnterProg (400);
#endif
{   UINT nRead;
    BITMAPFILEHEADER bmfHead;
    LPBITMAPINFO    pDibInfo;
	BOOL rtn = FALSE;
    if (!Fid || Fid==HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (400);
#endif
    	return (FALSE);
}

    nRead = BigRead (Fid,(HPSTR)&bmfHead,sizeof(BITMAPFILEHEADER));
	if (bmfHead.bfType == 0x4d42)
	{
		*phDibInfo = GSSiGlobAlloc(1402, GMEM_MOVEABLE,
			bmfHead.bfOffBits - sizeof(BITMAPFILEHEADER));
		pDibInfo = (LPBITMAPINFO)GlobalLock(*phDibInfo);

		nRead = BigRead(Fid, (HPSTR)pDibInfo, sizeof(BITMAPINFOHEADER));
		if (!pDibInfo->bmiHeader.biSizeImage)
		{
			long	RowLen = (long)pDibInfo->bmiHeader.biBitCount * (long)pDibInfo->bmiHeader.biWidth;
			if (RowLen % 8)
				RowLen = RowLen / 8 + 1;
			else
				RowLen = RowLen / 8;
			if (RowLen % 4)
				RowLen += 4 - RowLen % 4;
			pDibInfo->bmiHeader.biSizeImage = (long)pDibInfo->bmiHeader.biHeight * RowLen;
		}
		/*    if (pDibInfo->bmiHeader.biBitCount > 8)
			{
			pDibInfo->bmiHeader.biClrUsed=0;
			pDibInfo->bmiHeader.biClrImportant=0;
			} */
		if (!pDibInfo->bmiHeader.biClrUsed)
		{
			if (pDibInfo->bmiHeader.biBitCount == 4) pDibInfo->bmiHeader.biClrUsed = 16;
			if (pDibInfo->bmiHeader.biBitCount == 8) pDibInfo->bmiHeader.biClrUsed = 256;
		}
		nRead = BigRead(Fid, (HPSTR)pDibInfo->bmiColors, (UINT)pDibInfo->bmiHeader.biClrUsed * 4);
		*ImageOffset = GSSillseek(Fid, 0, 1);
		GlobalUnlock(*phDibInfo);
		rtn = TRUE;
	}
{
#if ENABLETRACE
GSSiExitProg (400);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL FixBitMapHeader (LPSTR FileName)
#if ENABLETRACE
{GSSiEnterProg (400);
#endif
{   UINT nRead;
    BITMAPFILEHEADER bmfHead;
    BITMAPINFO    DibInfo;
	long	loc;
	HFILE Fid=GSSiOpenFile (FileName,0,OF_READWRITE);

    if (!Fid || Fid==HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (400);
#endif
    	return (FALSE);
}

    nRead = BigRead (Fid,(HPSTR)&bmfHead,sizeof(BITMAPFILEHEADER));

	loc = GSSillseek (Fid,0,1);
    nRead = BigRead (Fid,(HPSTR)&DibInfo,sizeof(BITMAPINFOHEADER));
	DibInfo.bmiHeader.biHeight = abs (DibInfo.bmiHeader.biHeight);
	GSSillseek (Fid,loc,0);
	BigWrite (Fid,(HPSTR)&DibInfo,sizeof(BITMAPINFOHEADER),-1);
	GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (400);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}
int DisplayBMFileInRect (HDC hDC,LPSTR ImageFile, RECT InRect, short MaintainAspect)
#if ENABLETRACE
{GSSiEnterProg (392);
#endif
{ 
    int     rtn;
    HDIB32  hDibInfo;
    char	Name[MAX_PATH];  
    RGBQUAD	*pColors;
	RECT	Rect=InRect;
    
    _fstrcpy (Name,ImageFile);
    ExpandText (Name);
    if (!Name[0])
{
#if ENABLETRACE
GSSiExitProg (392);
#endif
    	return FALSE;  
}
    _fstrlwr (Name);
	SetCurImage (Name);
    hDibInfo=LoadDIB32(Name,FALSE);
	if (imageFileRotation)
		hDibInfo = GMRotateImageClassic (hDibInfo,imageFileRotation);

    if (!hDibInfo)
    {   
    	GSSiMessageBox (0,"Error loading bitmap",Name,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (392);
#endif
    	return FALSE;
}
    }
    rtn = DisplayBMInRect32 (hDC,hDibInfo, Rect, MaintainAspect); 
    DestroyDIB32 (hDibInfo,FALSE); 
{
#if ENABLETRACE
GSSiExitProg (392);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

int DisplayBMInRect (HDC hDC,LPBITMAPINFOHEADER pDibInfo,LPSTR pImage, RECT Rect, short MaintainAspect,LPRECT pOutRect)
#if ENABLETRACE
{GSSiEnterProg (393);
#endif
{   short   i;
    UINT	ColorOpt;   
    double	Factor=1;
    
    if (MaintainAspect)
        ComputeBMLoc (Rect,(LPBITMAPINFO) pDibInfo,MaintainAspect);
    else
    { 
        destX = (int)Rect.left;
        destY = (int)Rect.top;
        destW = (int)(Rect.right - Rect.left + 1);
        destH = (int)(Rect.bottom - Rect.top + 1);
    }
    SetStretchBltMode(hDC, StretchMode); 
/*    if (pDibInfo->biClrUsed)
    	ColorOpt = DIB_PAL_COLORS;
    else*/
    	ColorOpt = DIB_RGB_COLORS;
	if (pOutRect)
	{
		pOutRect->left = destX;
		pOutRect->right = destX + destW - 1;
		pOutRect->top = destY;
		pOutRect->bottom = destY + destH - 1;
	}
    i=StretchDIBits32 (hDC,destX,destY,
                       destW, destH,
                       0,0,
                       (int) pDibInfo->biWidth,
                       (int) pDibInfo->biHeight,
                       pImage,
                      (LPBITMAPINFOHEADER)pDibInfo,
                      (UINT)ColorOpt,
                      (DWORD) SRCCOPY,&Factor,FALSE);
/*  GlobalUnlock (hImage);
    GlobalUnlock (hDibInfo);
    GlobalFree (hImage);
    GlobalFree (hDibInfo);*/

{
#if ENABLETRACE
GSSiExitProg (393);
#endif
    return (destH);
}
#if ENABLETRACE
}
#endif
}

int  DisplayBMInRect32 (HDC hDC,HDIB32 hDib, RECT Rect, short MaintainAspect)
#if ENABLETRACE
{GSSiEnterProg (393);
#endif
{   short   i;
    UINT	ColorOpt;
    BITMAPINFOHEADER DibInfo;  
    double	Factor=1; 
    short	ii;
	BOOL deleteImage = FALSE;
    
	destH = 0;
    if (!hDib)
		goto Exit;
	GetBitmapInfoFromHandle (&DibInfo,hDib);
	/*if (DibInfo.biBitCount != 32)
	{
		hDib = FreeImage_ConvertTo32Bits(hDib);
		GetBitmapInfoFromHandle(&DibInfo, hDib);

		deleteImage = TRUE;
	}*/
	if (MaintainAspect == 2)
	{
		POINT	Center = RectMid (&Rect);
		int		w = (Rect.right - Rect.left)/2;
		int		h = (Rect.bottom - Rect.top)/2;

		w = DibInfo.biWidth/2;
		h = DibInfo.biHeight/2;
		
		Rect.left = Center.x - w;
		Rect.right = Center.x + w;
		Rect.top = Center.y - h;
		Rect.bottom = Center.y + h;
		MaintainAspect = 1;
	}
    if (MaintainAspect)
        ComputeBMLoc (Rect,(LPBITMAPINFO) &DibInfo,MaintainAspect);
    else
    { 
        destX = (int)Rect.left;
        destY = (int)Rect.top;
        destW = (int)(Rect.right - Rect.left + 1);
        destH = (int)(Rect.bottom - Rect.top + 1);
    }
    SetStretchBltMode(hDC, StretchMode); 
/*    if (pDibInfo->biClrUsed)
    	ColorOpt = DIB_PAL_COLORS;
    else*/
    	ColorOpt = DIB_RGB_COLORS;  
    i=StretchDIBitsFromHandle (hDC,destX,destY,
                       			destW, destH,
                       0,0, 
                       DibInfo.biWidth,DibInfo.biHeight,
                       hDib,ColorOpt,SRCCOPY,
					   Factor);
/*  GlobalUnlock (hImage);
    GlobalUnlock (hDibInfo);
    GlobalFree (hImage);
    GlobalFree (hDibInfo);*/
	if (deleteImage)
		GMDestroyDIB32(hDib);

Exit:
{
#if ENABLETRACE
GSSiExitProg (393);
#endif
    return (destH);
}
#if ENABLETRACE
}
#endif
}

int  DisplayBMInRect32_2 (HDC hDC,HDIB32 hDib, RECT Rect, short MaintainAspect,DWORD RasterOpt)
#if ENABLETRACE
{GSSiEnterProg (393);
#endif
{   short   i;
    UINT	ColorOpt;
    BITMAPINFOHEADER DibInfo;  
    double	Factor=1; 
    short	ii;
    
    GetBitmapInfoFromHandle (&DibInfo,hDib);
	if (MaintainAspect == 2)
	{
		POINT	Center = RectMid (&Rect);
		int		w = (Rect.right - Rect.left)/2;
		int		h = (Rect.bottom - Rect.top)/2;

		w = DibInfo.biWidth/2;
		h = DibInfo.biHeight/2;
		
		Rect.left = Center.x - w;
		Rect.right = Center.x + w;
		Rect.top = Center.y - h;
		Rect.bottom = Center.y + h;
		MaintainAspect = 1;
	}
    if (MaintainAspect)
        ComputeBMLoc (Rect,(LPBITMAPINFO) &DibInfo,MaintainAspect);
    else
    { 
        destX = (int)Rect.left;
        destY = (int)Rect.top;
        destW = (int)(Rect.right - Rect.left + 1);
        destH = (int)(Rect.bottom - Rect.top + 1);
    }
    SetStretchBltMode(hDC, StretchMode); 
/*    if (pDibInfo->biClrUsed)
    	ColorOpt = DIB_PAL_COLORS;
    else*/
    	ColorOpt = DIB_RGB_COLORS;  
    i=StretchDIBitsFromHandle (hDC,destX,destY,
                       			destW, destH,
                       0,0, 
                       DibInfo.biWidth,DibInfo.biHeight,
                       hDib,ColorOpt,RasterOpt,
                       Factor);
/*  GlobalUnlock (hImage);
    GlobalUnlock (hDibInfo);
    GlobalFree (hImage);
    GlobalFree (hDibInfo);*/

{
#if ENABLETRACE
GSSiExitProg (393);
#endif
    return (destH);
}
#if ENABLETRACE
}
#endif
}

BOOL  DisplayBMInRect2 (HDC hDC, HANDLE hBM, RECT Rect, double Factor,double VPct, double HPct,LPRECT pOutRect)
#if ENABLETRACE
{GSSiEnterProg (394);
#endif
{   short   i=0;
	LPBITMAPINFOHEADER pDibInfo;
	LPSTR pImage;  
	short	xoff=IDNINT(HPct),yoff=IDNINT(VPct);    
	BOOL	Use32=TRUE;
    
    if (!hBM)
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return FALSE;
}
    pDibInfo = (LPBITMAPINFOHEADER)GlobalLock (hBM);
    if (!pDibInfo)
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return FALSE;
}
    pImage = FindDIBBits ((LPSTR)pDibInfo);
    if (!Factor)
		i=DisplayBMInRect (hDC,pDibInfo,pImage, Rect, TRUE,pOutRect);
    else if (Factor < 0)
		i=DisplayBMInRect (hDC,pDibInfo,pImage, Rect, FALSE,pOutRect);
	else
	{
        destX = (short)Rect.left;
        destY = (short)Rect.top;
        destW = (short)(Rect.right - Rect.left + 1);
        destH = (short)(Rect.bottom - Rect.top + 1); 
        if (!Use32)
        {
		    SetStretchBltMode(hDC, StretchMode); 
		    i=StretchDIBits (hDC,destX,destY,
		                       destW, destH,
		                       xoff,yoff,
		                       (short) IDNINT(min(destW*Factor,(pDibInfo->biWidth-xoff)*Factor)),
		                       (short) IDNINT(min(destH*Factor,(pDibInfo->biHeight-yoff)*Factor)),
		                       pImage,
		                      (LPBITMAPINFO)pDibInfo,
		                      (UINT)DIB_RGB_COLORS,
		                      (DWORD) SRCCOPY); 
	    } 
	    else                 
		    i=StretchDIBits32 (hDC,destX,destY,
		                       destW, destH,
		                       xoff,yoff,
		                       IDNINT(min(destW*Factor,(pDibInfo->biWidth-xoff)*Factor)),
		                       IDNINT(min(destH*Factor,(pDibInfo->biHeight-yoff)*Factor)),
		                      (LPBYTE) pImage,
		                      (LPBITMAPINFOHEADER)pDibInfo,
		                      (UINT)DIB_RGB_COLORS,
		                      (DWORD) SRCCOPY, 
		                      &Factor,FALSE);
	                      
    }  
    GlobalUnlock (hBM); 
    if (i)
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return TRUE;
}
    else
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return FALSE;
}
#if ENABLETRACE
}
#endif
}

double DistToGray (short R, short G, short B, short Gray)
#if ENABLETRACE
{GSSiEnterProg (1434);
#endif
{   
	double dist = pow (
						pow ((double)(R-Gray),2) + 
						pow ((double)(G-Gray),2) +
						pow ((double)(B-Gray),2)
					,1e0/2e0);
    
{
#if ENABLETRACE
GSSiExitProg (1434);
#endif
    	return dist;
}
#if ENABLETRACE
}
#endif
}

COLORREF RGBTRIPLEToCOLORREF (RGBTRIPLE Trip)
{
	COLORREF c;

	c = RGB(Trip.rgbtRed,Trip.rgbtGreen,Trip.rgbtBlue);
	return c;
}

COLORREF RGBQUADToCOLORREF (RGBQUAD Quad)
{
	COLORREF c;

	c = RGB(Quad.rgbRed,Quad.rgbGreen,Quad.rgbBlue);
	return c;
}

RGBQUAD RGBTripleToRGBQuad (RGBTRIPLE Trip)
{
	RGBQUAD	qcolor;

	qcolor.rgbBlue = Trip.rgbtBlue;
	qcolor.rgbGreen = Trip.rgbtGreen;
	qcolor.rgbRed = Trip.rgbtRed;
	qcolor.rgbReserved = 0;
	return qcolor;
}

RGBTRIPLE RGBQuadToRGBTriple (RGBQUAD qcolor)
{
	RGBTRIPLE Trip;

	Trip.rgbtBlue = qcolor.rgbBlue;
	Trip.rgbtGreen = qcolor.rgbGreen;
	Trip.rgbtRed = qcolor.rgbRed;
	return Trip;
}

RGBTRIPLE ConvertToGrayTriple (RGBTRIPLE color3)
#if ENABLETRACE
{GSSiEnterProg (1428);
#endif
{
	COLORREF	color;
	
	switch (ConvertToGrayTechnique)
	{
		case 1:
		{
//			short luminance=0.3*color3.rgbtRed+0.59*color3.rgbtGreen+0.11*color3.rgbtBlue;  
			short luminance=(30L*color3.rgbtRed+59L*color3.rgbtGreen+11L*color3.rgbtBlue)/100;  
			
			color3.rgbtRed = color3.rgbtGreen = color3.rgbtBlue = luminance;
		}
		break;
		case 2:
		{
			short luminance=(color3.rgbtRed+color3.rgbtGreen+color3.rgbtBlue)/3;  
			
			color3.rgbtRed = color3.rgbtGreen = color3.rgbtBlue = luminance;
		}
		break; 
		case 3:
		{
			_fmemmove (&color,&color3,3);
			color = ConvertToGray (color);
			_fmemmove (&color3,&color,3);
		}
		break;
	}
	
{
#if ENABLETRACE
GSSiExitProg (1428);
#endif
	return color3;
}
#if ENABLETRACE
}
#endif
}  

COLORREF ConvertToGray (COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (1429);
#endif
{

	double	DMin, DMax;
	short	MinDist=5, MinVal=0, MaxVal=255; 
	short	R=GetRValue (Color);
	short	G=GetGValue (Color);
	short	B=GetBValue (Color);  
	
	
	USHORT	MaxLoops=16, nLoops=0, ii;
			
	if (ConvertToGrayTechnique == 1)
	{
		short luminance=(30L*R+59L*G+11L*B)/100;  
		Color = RGB(luminance,luminance,luminance); 
		goto Exit;
	}	
Loop:   
	nLoops++;
    DMin = DistToGray (R,G,B,MinVal);
    DMax = DistToGray (R,G,B,MaxVal);
    if (DMin < DMax)
    {
	    if (DMin <= MinDist)
	    {
			Color = RGB(MinVal,MinVal,MinVal);
			goto Exit;
		} 
		else
    		MaxVal = (MinVal + MaxVal) / 2; 
    }
    else
    {
	    if (DMax <= MinDist)
	    {
			Color = RGB(MaxVal,MaxVal,MaxVal);
			goto Exit;
		} 
		else
    		MinVal = (MinVal + MaxVal) / 2; 
    }
    if (MaxVal - MinVal <= MinDist)
    {
    	MinVal = (MinVal + MaxVal) / 2;
		Color = RGB(MinVal,MinVal,MinVal);
		goto Exit;
    }
    if (nLoops > MaxLoops)
    	ii=1;
    goto Loop;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1429);
#endif
	return Color;
}
#if ENABLETRACE
}
#endif
}  

void AdjustDIBColors (HANDLE hDib)
#if ENABLETRACE
{GSSiEnterProg (1430);
#endif
{

	RGBTRIPLE	color = { 0 };
	COLORREF	cref;
    BYTE HUGE *startrow; 
    BYTE	*startimage;
    HANDLE h;
    DWORD cnt;                         
    WORD	irow,icol,BytesPerPel, MaxVal;
    long	rowlen; 
    RGBTRIPLE	_huge	*rgb24;             
    LPBITMAPINFOHEADER	lpbi;
	GSSiCOLOR16 *rgb16;  
	BOOL	OrthosAreGray=GetGlobalBVal2 ("[%ORTHOSAREGRAY]",FALSE);
	COLORREF	SaveAutoOrthoColor = AutoOrthoColor;
	BOOL isdib32 = TRUE;

	AutoOrthoColor = (COLORREF)-1;
     
	if (!hDibIs32Bit (hDib))
	{
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		isdib32 = FALSE;
	}
	else
		lpbi =  (LPBITMAPINFOHEADER)GetDibHeader (hDib);
	if (lpbi->biBitCount <= 8)
		goto Exit; 
/*	if (!OrthosAreGray)
	{
		RFac = GetGlobalLVal ("[%ORTHORED]");
		GFac = GetGlobalLVal ("[%ORTHOGREEN]");
		BFac = GetGlobalLVal ("[%ORTHOBLUE]");
		Intensity = GetGlobalLVal ("[%ORTHOINTENSITY]");
	    if (RFac == 0 && GFac == 0 && BFac == 0 && Intensity == 0 && (!CurView || !CurView->ConvertToGray)) 
	    	goto Exit;
	    if (RFac<0)
	    	RFac = -(RFac + 100);
	    else if (RFac>0)
	    	RFac *= log((double)RFac)/log(100);
	    if (GFac<0)
	    	GFac = -(GFac + 100);
	    else
	    	GFac *= log((double)GFac)/log(100);
	    if (BFac<0)
	    	BFac = -(BFac + 100);
	    else
	    	BFac *= log((double)BFac)/log(100);
	    if (Intensity<0)
	    	Intensity = -(Intensity + 100); 
	    else
	    	Intensity *= log((double)Intensity)/log(100);
	}*/
    BytesPerPel = lpbi->biBitCount/8;
    MaxVal = lpbi->biBitCount;
	switch (lpbi->biBitCount)
    {
    	case 24:
			MaxVal = 255;
			break;
		case 16:
			MaxVal = 31;
			break;
	}
    if (lpbi->biBitCount>8)
    {
		if (!isdib32)
		{
    		startimage = (LPSTR) lpbi + (lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF));
    		startrow = startimage;
		}
    	irow = lpbi->biHeight;
    	rowlen = lpbi->biWidth*BytesPerPel;
    	if (rowlen%4) rowlen += (4-rowlen%4);
    	while (irow--)
    	{ 
			if (isdib32)
				startrow = FreeImage_GetScanLine(hDib,irow);
			else
			{
				rgb24 = (RGBTRIPLE	*)startrow; 
				rgb16 = (GSSiCOLOR16 *)startrow; 
			}
    		icol = lpbi->biWidth;
    		while (icol--)
    		{
		    switch (lpbi->biBitCount)
			    {
			    	case 24:
	    				color = *rgb24;
	    				break;
	    			case 16:
	    				color.rgbtBlue = rgb16->b;
	    				color.rgbtRed = rgb16->r;
	    				color.rgbtGreen = rgb16->g;
	    				break;
	    		}
				cref = COLORREFFromRGBTRIPLE (color);
				cref= ConvertColor (cref,-1);
			    switch (lpbi->biBitCount)
			    {
			    	case 24:
	    				rgb24->rgbtBlue = GetBValue (cref);
	    				rgb24->rgbtGreen = GetGValue (cref);
	    				rgb24++->rgbtRed = GetRValue (cref);
	    				break;
	    			case 16: 
	    				rgb16->b = GetBValue (cref);
	    				rgb16->g = GetGValue (cref);
	    				rgb16++->r = GetRValue (cref);
	    				break;
	    		}
    		}
    		startrow += rowlen;
    	}
    }
Exit:
	if (!isdib32)
		GlobalUnlock (hDib);
	AutoOrthoColor = SaveAutoOrthoColor;
	
{
#if ENABLETRACE
GSSiExitProg (1430);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

HDIB32 AdjustDIB32Colors (HDIB32 hDib)
#if ENABLETRACE
{GSSiEnterProg (1431);
#endif
{

    RGBTRIPLE	color;
	COLORREF	cref=RGB(250,240,230),cref2=cref;
    BYTE HUGE *startrow; 
    BYTE	*startimage;
    HANDLE h;
    DWORD cnt;                         
    WORD	irow,icol,BytesPerPel, MaxVal;
    long	rowlen; 
    RGBTRIPLE	*rgb24;             
    LPBITMAPINFOHEADER	lpbi;
	GSSiCOLOR16 *rgb16;  
    
	if (!hDib)
		goto Exit;
	cref = ConvertColor (cref,-1);
	if (cref == cref2)//no color conversions
		goto Exit;
	lpbi = FreeImage_GetInfoHeader(hDib);

	if (lpbi->biBitCount < 8)
		goto Exit; 
    BytesPerPel = lpbi->biBitCount/8;
    MaxVal = lpbi->biBitCount;
	switch (lpbi->biBitCount)
    {
    	case 24:
			MaxVal = 255;
			break;
		case 16:
			MaxVal = 31;
			break;
	}
    switch (lpbi->biBitCount)
    {
   /* get a pointer to the DIB bits */
	case 16:
	case 24:

    	startimage = (LPSTR) FreeImage_GetBits(hDib);
    	startrow = startimage;
    	irow = lpbi->biHeight;
    	rowlen = lpbi->biWidth*BytesPerPel;
    	if (rowlen%4) rowlen += (4-rowlen%4);
    	while (irow--)
    	{                      
			rgb24 = (RGBTRIPLE	*)startrow; 
			rgb16 = (GSSiCOLOR16 *)startrow; 
    		icol = lpbi->biWidth;
    		while (icol--)
    		{
		    switch (lpbi->biBitCount)
			    {
			    	case 24:
	    				color = *rgb24;
	    				break;
	    			case 16:
	    				color.rgbtBlue = rgb16->b;
	    				color.rgbtRed = rgb16->r;
	    				color.rgbtGreen = rgb16->g;
	    				break;
	    		}
				cref = COLORREFFromRGBTRIPLE (color);
				cref= ConvertColor (cref,-1);
			    switch (lpbi->biBitCount)
			    {
			    	case 24:
	    				rgb24->rgbtBlue = GetBValue (cref);
	    				rgb24->rgbtGreen = GetGValue (cref);
	    				rgb24++->rgbtRed = GetRValue (cref);
	    				break;
	    			case 16: 
	    				rgb16->b = GetBValue (cref);
	    				rgb16->g = GetGValue (cref);
	    				rgb16++->r = GetRValue (cref);
	    				break;
	    		}
    		}
    		startrow += rowlen;
    	}
		break;
	case 8:
		{
			RGBQUAD	*qcolor = FreeImage_GetPalette (hDib);
			RGBTRIPLE color;
			UINT	i;

			for (i=0;i<lpbi->biClrUsed;i++,qcolor++)
			{

				cref = COLORREFFromRGBQUAD (*qcolor);
 				cref= ConvertColor (cref,-1);
				*qcolor = RGBQUADFromCOLORREF (cref);
			}

		}
		break;
	default:
		break;
    }
Exit:
	
{
#if ENABLETRACE
GSSiExitProg (1431);
#endif
	return hDib;
}
#if ENABLETRACE
}
#endif
}  

HANDLE ConvertBitmap16To24 (LPBITMAPINFOHEADER  lpbi)
#if ENABLETRACE
{GSSiEnterProg (1432);
#endif
{

	LPBITMAPINFOHEADER  lpNewbi;
	HDIB	NewDIB; 
	unsigned short		ipixel, irow,i;
	char	*StartImageOrig;
	char	*StartImageNew;
	long	RowLenNew, RowLenOrig, Sizeimage;
	GSSiCOLOR16 *lp16; 
	unsigned	char	*icolor;  
	                   
	RowLenNew = lpbi->biWidth * 3;
	if (RowLenNew % 4)
		RowLenNew += 4 - (RowLenNew % 4);
		
	RowLenOrig = lpbi->biWidth * 2;
	if (RowLenOrig % 4)
		RowLenOrig += 4 - (RowLenOrig % 4);
		
	Sizeimage =	lpbi->biHeight * RowLenNew;
	NewDIB = GSSiGlobAlloc (1403,GHND, sizeof(BITMAPINFOHEADER) + Sizeimage);
	lpNewbi = (LPBITMAPINFOHEADER)GlobalLock (NewDIB);
	*lpNewbi = *lpbi;  
	lpNewbi->biPlanes=1;
	lpNewbi->biBitCount = 24;
	lpNewbi->biSizeImage = Sizeimage; 
	lpNewbi->biClrUsed = 0;
	StartImageOrig = (LPSTR)lpbi + sizeof (BITMAPINFOHEADER);
	StartImageNew = (LPSTR)lpNewbi + sizeof (BITMAPINFOHEADER);
	
	for (irow=0;irow<lpNewbi->biHeight;irow++)
	{
		icolor = (StartImageNew + (irow * RowLenNew));
		lp16 = (GSSiCOLOR16 *)(StartImageOrig + (irow * RowLenOrig));
		ipixel = lpNewbi->biWidth;
		while (ipixel--)
		{   
			*icolor++ = lp16->r * 8;
			*icolor++ = lp16->g * 8;
			*icolor++ = lp16++->b * 8;
		}
	}
	
	GlobalUnlock (NewDIB);
	
	
{
#if ENABLETRACE
GSSiExitProg (1432);
#endif
	return NewDIB;
}
#if ENABLETRACE
}
#endif
}  

HANDLE ConvertBitmap8To24 (LPBITMAPINFOHEADER  lpbi)
#if ENABLETRACE
{GSSiEnterProg (1433);
#endif
{

	LPBITMAPINFOHEADER  lpNewbi;
	HDIB	NewDIB; 
	unsigned short		ipixel, irow,i;
	char	*StartImageOrig;
	char	*StartImageNew;
	long	RowLenNew, RowLenOrig, Sizeimage;
	BYTE *lp8; 
	unsigned	char	*icolor;
	COLORREF	far *pColorMap;
	typedef struct {BITMAPINFOHEADER	bi;
		 		  RGBQUAD	colors[256];} BMHEAD;
	BMHEAD	*pbmhead=(BMHEAD *)lpbi;
	                   
	RowLenNew = lpbi->biWidth * 3;
	if (RowLenNew % 4)
		RowLenNew += 4 - (RowLenNew % 4);
		
	RowLenOrig = lpbi->biWidth;
	if (RowLenOrig % 4)
		RowLenOrig += 4 - (RowLenOrig % 4);
		
	Sizeimage =	lpbi->biHeight * RowLenNew;
	NewDIB = GSSiGlobAlloc (1405,GHND, sizeof(BITMAPINFOHEADER) + Sizeimage);
	lpNewbi = (LPBITMAPINFOHEADER)GlobalLock (NewDIB);
	*lpNewbi = *lpbi;  
	lpNewbi->biPlanes=1;
	lpNewbi->biBitCount = 24;
	lpNewbi->biSizeImage = Sizeimage;
	lpNewbi->biClrUsed = 0;
	lpNewbi->biClrImportant = 0;
	pColorMap = (COLORREF	far *)((LPSTR)lpbi + sizeof (BITMAPINFOHEADER));
	StartImageOrig = (LPSTR)lpbi + sizeof (BITMAPINFOHEADER) + lpbi->biClrUsed * sizeof(COLORREF);
	StartImageNew = (LPSTR)lpNewbi + sizeof (BITMAPINFOHEADER);
	
	for (irow=0;irow<lpNewbi->biHeight;irow++)
	{
		icolor = (StartImageNew + (irow * RowLenNew));
		lp8 = (StartImageOrig + (irow * RowLenOrig));
		ipixel = lpNewbi->biWidth;
		while (ipixel--)
		{   
			*icolor++ = GetRValue(pColorMap[*lp8]);
			*icolor++ = GetGValue(pColorMap[*lp8]); 
			*icolor++ = GetBValue(pColorMap[*lp8++]); 
		}
	}
	
	GlobalUnlock (NewDIB);
	
	
{
#if ENABLETRACE
GSSiExitProg (1433);
#endif
	return NewDIB;
}
#if ENABLETRACE
}
#endif
}

BOOL GetBMPBounds (LPSTR PathName,LPMNMXCORD pBitmapBounds,LPMNMXCORD pWBounds)
{
    BITMAPINFOHEADER DibInfo; 
    double	ScaleX, ScaleY;
    DPOINT	BitmapPoint,WorldPoint; 
    char	WorldFile[256],str[130];  
	long	ImageOffset;
    LPSTR	pDot;  
    HFILE	Fid;
	HANDLE	hTran;
	HFILE FidIn = GSSiOpenFile (PathName,0,OF_READ);
	HANDLE hDibInfo=0;

	if (FidIn != HFILE_ERROR && ReadBitMapHeader (FidIn,&hDibInfo, &ImageOffset))
	{    
		LPBITMAPINFO    pDibInfo=(LPBITMAPINFO)GlobalLock (hDibInfo);

		DibInfo = pDibInfo->bmiHeader;

		pBitmapBounds->xmn = pBitmapBounds->ymn = 0;
		pBitmapBounds->xmx = DibInfo.biWidth-1;
		pBitmapBounds->ymx = abs(DibInfo.biHeight)-1;
		_fstrcpy (WorldFile,PathName);
		if ((pDot = _fstrrchr (WorldFile,'.')))
		{
    		if (!_fstricmp (pDot,".tif"))
    			_fstrcpy (pDot,".tfw");
    		else 
    			_fstrcpy (pDot,".bpw");
			if (!ExistFile (WorldFile))
			{
				strcpy (pDot,".trn");
				if (!ExistFile (WorldFile))
					goto Exit;
				hTran = LoadTranFile (WorldFile,1,2,0,0);
				if (!hTran)
					goto Exit;
				*pWBounds = *pBitmapBounds;
				TranBounds (hTran,pWBounds);
				CloseTRANS2 (&hTran);
				return TRUE;
			}
    		if ((Fid = GSSiOpenFile(WorldFile,0,OF_READ)) == HFILE_ERROR)
    			goto Exit;
    		fgetstring (str,128,Fid);
    		ScaleX = atof (str);
    		fgetstring (str,128,Fid);
    		BitmapPoint.x = atof (str);
    		fgetstring (str,128,Fid);
    		BitmapPoint.y = atof (str);
    		fgetstring (str,128,Fid);
    		ScaleY = atof (str);
    		fgetstring (str,128,Fid);
    		WorldPoint.x = atof (str);
    		fgetstring (str,128,Fid);
    		WorldPoint.y = atof (str);
    		GSSiClose (Fid);
    		pWBounds->ymx = WorldPoint.y;
    		pWBounds->xmn = WorldPoint.x;  
    		pWBounds->xmx = WorldPoint.x + ScaleX * (DibInfo.biWidth-1);
    		pWBounds->ymn = WorldPoint.y + ScaleY * (abs(DibInfo.biHeight)-1); 
	    	GSSiClose (FidIn);
			GSSiGlobUlFree (&hDibInfo);
    		return TRUE;
		}
    }
Exit:	
   	pWBounds->xmn = pWBounds->ymn = 0;
   	pWBounds->ymx = abs(DibInfo.biHeight)-1;
   	pWBounds->xmx = DibInfo.biWidth-1;
	GSSiClose (FidIn);
 	GSSiGlobUlFree (&hDibInfo);
    return TRUE;
} 


BOOL SplitBitmap (LPSTR Infile,int numRows,int numCols,LPSTR OutDir,LPSTR OutExt,int OutOpt)
{
	BOOL rtn=FALSE;
	HANDLE	hDibInfo=0;
	long ImageOffset;
	long rowLen;
	HFILE FidIn;
#define MAXOUTFILES 8
	HFILE FidOut[MAXOUTFILES];
	long splitWidth, splitHeight;
	int	remRows, remCols, remBits;
	BOOL topDown=FALSE;
	HANDLE	hRow=0;
	LPBYTE pRow;
	int origRowLen;
	BITMAPINFOHEADER bitmapInfoOut={0};
	BITMAPFILEHEADER bmfHead={0};
	MNMXCORD BitmapBounds;
	MNMXCORD WBounds;
	double	res;
	MNMXCORD MnMx;
	int	origRow, origHeight;

	if (!GetBMPBounds (Infile,&BitmapBounds,&WBounds))
		return FALSE;

	res = (WBounds.xmx - WBounds.xmn) / (BitmapBounds.xmx - BitmapBounds.xmn);
	if (numCols < 1 || numCols > MAXOUTFILES)
		return FALSE;
	FidIn  = GSSiOpenFile (Infile,0,OF_READ);
	if (FidIn != HFILE_ERROR && ReadBitMapHeader (FidIn,&hDibInfo, &ImageOffset))
	{    
		LPBITMAPINFO    pDibInfo=(LPBITMAPINFO)GlobalLock (hDibInfo);

		if (pDibInfo->bmiHeader.biHeight < 0)
		{
			topDown = TRUE;
			pDibInfo->bmiHeader.biHeight = -pDibInfo->bmiHeader.biHeight;
		}
		origRow = origHeight = pDibInfo->bmiHeader.biHeight;
		origRowLen = pDibInfo->bmiHeader.biWidth * pDibInfo->bmiHeader.biBitCount/8;
		splitWidth = pDibInfo->bmiHeader.biWidth / numCols;
		splitHeight = pDibInfo->bmiHeader.biHeight / numRows;
		remCols = pDibInfo->bmiHeader.biWidth % numCols;
		remRows = pDibInfo->bmiHeader.biHeight % numRows;
		rowLen = splitWidth * pDibInfo->bmiHeader.biBitCount;
		remBits = rowLen % 32;
		rowLen /= 8;
		bitmapInfoOut.biBitCount = pDibInfo->bmiHeader.biBitCount;
		bitmapInfoOut.biHeight = splitHeight;
		bitmapInfoOut.biWidth = splitWidth;
		bitmapInfoOut.biSize = 40;
		bitmapInfoOut.biPlanes = pDibInfo->bmiHeader.biPlanes;
	}
	if (!remCols && !remRows && !remBits)
	{
		int iRow, iCol;
		int iRow2;

		bmfHead.bfType = 'MB';//19778
		bmfHead.bfOffBits = 54;
		bmfHead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + splitHeight * rowLen;
		GSSillseek (FidIn,ImageOffset,0);
		CreateStatusWind (hWndMain,1,"Split Bitmap");

		hRow = GSSiGlobAlloc (0,GMEM_MOVEABLE,origRowLen);
		pRow = GlobalLock (hRow);
		for (iRow=0;iRow<numRows;iRow++)
		{
			char OutName[MAX_PATH];

			for (iCol=0;iCol<numCols;iCol++)
			{
				sprintf (OutName,"%s\\Row_%i_Col_%i.bmp",OutDir,iRow,iCol);
				FidOut[iCol] = GSSiOpenFile (OutName,0,OF_CREATE);
				if (FidOut[iCol] == HFILE_ERROR)
					goto Exit;

				BigWrite (FidOut[iCol],&bmfHead,sizeof(BITMAPFILEHEADER),-1);
				BigWrite (FidOut[iCol],&bitmapInfoOut,sizeof(BITMAPINFOHEADER),-1);
				MnMx.xmn = WBounds.xmn + res * iCol * splitWidth;
				MnMx.xmx = WBounds.xmn + res * (iCol+1) * splitWidth;
				MnMx.ymn = WBounds.ymn + res * iRow * splitHeight;
				MnMx.ymx = WBounds.ymn + res * (iRow+1) * splitHeight;
				CreateBPW (OutName,MnMx,res);
			}
			for (iRow2=0;iRow2<splitHeight;iRow2++)
			{
				origRow--;
				if (topDown)
					GSSillseek (FidIn,ImageOffset+origRow*origRowLen,0);
				BigRead (FidIn,pRow,origRowLen);
				for (iCol=0;iCol<numCols;iCol++)
				{
					BigWrite (FidOut[iCol],(pRow+rowLen*iCol),rowLen,-1);
				}
				StatusWindowUpdate ("","",origHeight,origHeight - origRow);
			}
			for (iCol=0;iCol<numCols;iCol++)
			{
				GSSiClose (FidOut[iCol]);
			}
		}
	}
Exit:
	DestroyStatusWindow(0);  
	GSSiClose (FidIn);  
	GSSiGlobUlFree (&hDibInfo);
	GSSiGlobUlFree (&hRow);

	return rtn;
}

HDIB32 BMPToDIB32(HANDLE hBMP)
{
	HDIB32	dib = NULL;
	BITMAPINFOHEADER FAR *lpbi;  // pointer to BITMAPINFOHEADER
	DWORD rowLen;                // size of scanline
	WORD biBits;                 // bits per pixel
	int	ctype,i;
	LPSTR	fromLine, toLine;

	if (!hBMP)
	  return NULL;
	lpbi = GlobalLock (hBMP);

	dib = FreeImage_Allocate (lpbi->biWidth,lpbi->biHeight,lpbi->biBitCount,0,0,0);

	if (!dib)
	{
		GlobalUnlock (hBMP);
	    return NULL;
	}

	rowLen = lpbi->biSizeImage / lpbi->biHeight;

	fromLine = FindDIBBits((LPSTR)lpbi);
	for (i=0;i<lpbi->biHeight;i++,fromLine+=rowLen)
	{
		LPSTR toLine = (LPSTR)FreeImage_GetScanLine(dib,i);
		memcpy (toLine,fromLine,rowLen);
	}
	GlobalUnlock (hBMP);

	return dib;
}


