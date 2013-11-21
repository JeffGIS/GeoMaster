//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// NNN     NN  PPPPPPP  EEEEEE
// NN NN   NN  PP    PP EE
// NN  NN  NN  PPPPPPP  EEEEE
// NN   NN NN  PP       EE
// NN     NNN  PP       EEEEEEE
//
// Copyright © 2008 by North Pole Engineering, Inc.  All rights reserved.
// Printed in the United States of America.  Except as permitted under the
// United States Copyright Act of 1976, no part of this software may be
// reproduced or distributed in any form or by any means, without the prior
// written permission of North Pole Engineering, Inc., unless such copying is
// expressly permitted by federal copyright law.
//
// Address copying inquires to:
// North Pole Engineering, Inc.
// Attn: Joe Meyer
// 221 North 1st Street Suite 310
// Minneapolis, Minnesota 55401
//
// Information contained in this software has been created or obtained by North
// Pole Engineering, Inc. from sources believed to be reliable. However, North
// Pole Engineering, Inc. does not guarantee the accuracy or completeness of the
// information published herein nor shall North Pole Engineering, Inc. be liable
// for any errors, omissions, or damages arising from the use of this software.
//
//
//	MODULE:
//    Bitmap.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    12/22/2008
//
//  DESCRIPTION:
//    Functions for manipulating bitmap images.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "Bitmap.h"
#include "pBitmap.h"
#include "FixedPoint.h"
#include "fs.h"
#include "ClMalloc.h"
#include "ClAssert.h"
#include "ByteSwap.h"

#if !defined(HUMMINBIRD)
#pragma warning(disable : 4996) 
#endif
  
//**********************************************************************
// types
//**********************************************************************
#if !defined(HUMMINBIRD)
#pragma pack(push,2)
#endif
typedef struct ClBitmapFileHeaderTag
{
  unsigned short  usType;
  unsigned long   unSize;
  unsigned short  usReserved1;
  unsigned short  usReserved2;
  unsigned long   unOffBits;
} ClBitmapFileHeader;
#if !defined(HUMMINBIRD)
#pragma pack(pop)
#endif

typedef struct ClBitmapInfoHeaderTag
{
  unsigned long     unSize;
  long              nWidth;
  long              nHeight;
  unsigned short    usPlanes;
  unsigned short    usBitCount;
  unsigned long     unCompression;
  unsigned long     unSizeImage;
  long              nXPelsPerMeter;
  long              nYPelsPerMeter;
  unsigned long     unClrUsed;
  unsigned long     unClrImportant;
} ClBitmapInfoHeader;


//**********************************************************************
// Private Function Prototypes.
//**********************************************************************
static int PixelTotalOffset( Bitmap *pBitmap, long x, long y );
static void ReadClBitmapFileHeader( ClBitmapFileHeader *pHeader, CLFILE *pFile );
static void ReadClBitmapInfoHeader( ClBitmapInfoHeader *pHeader, CLFILE *pFile );
static void BmFlipVertical( Bitmap *pBitmap );

//**********************************************************************
// Public functions.
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    BmInit
//
//  DESCRIPTION:	
//    Initializes a bitmap.  This functions must be called before the
//    bitmap can be used by any other function except BmCreate.  The 
//    bitmap width and height will be set to zero.  The bitmap will 
//    have no data.
//
//  Inputs:
//    Bitmap *pBitmap
//      A pointer to the bitmap.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void BmInit( Bitmap *pBitmap )
{
  ClAssert( NULL != pBitmap );

  memset( pBitmap, 0, sizeof( Bitmap ) );
}

//**********************************************************************
//  FUNCTION:
//    BmCreate
//
//  DESCRIPTION:	
//    Creates a bitmap. BmInit need not be called before this function.
//    BmClear should be called after this function to erase the data that
//    happens to be in the bitmap's memory.
//
//    BmLoad can be called instead to create a bitmap from a BMP file.
//
//  Inputs:
//    Bitmap *pBitmap
//      The bitmap that is initialized.
//    int nWidth
//      Width of the bitmap in pixels.
//    int nHeight
//      Height of the bitmap in pixels.
//    Palette *pPalette
//      The palette in copied into the bitmap's palette.  If NULL
//      then the bitmap's palette will be empty.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL BmCreate
(
  Bitmap *pBitmap,
  int nWidth,
  int nHeight,
  ClPalette *pPalette
)
{
  int iSize = CALC_BITMAP_BYTE_SIZE( nWidth, nHeight );

  ClAssert( NULL != pBitmap );
  ClAssert( nWidth >= 0 );
  ClAssert( nHeight >= 0 );

  BmInit( pBitmap );

  if( NULL == pPalette )
  {
    PaletteClear( &pBitmap->palette );
    pBitmap->palette.nColorCount = 0;
  }
  else
  {
    PaletteCopy( &pBitmap->palette, pPalette );
  }

  pBitmap->pucData = (unsigned char *)cl_malloc( iSize );
  if( NULL != pBitmap->pucData )
  {
    pBitmap->nWidth = nWidth;
    pBitmap->nHeight = nHeight;

    return CL_TRUE;
  }
  else
    return CL_FALSE;
}

//**********************************************************************
//  FUNCTION:
//    BmDestroy
//
//  DESCRIPTION:	
//    Frees memory used for the bitmap data.
//
//  Inputs:
//    Bitmap *pBitmap
//      A pointer to the bitmap.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void BmDestroy( Bitmap *pBitmap)
{
  ClAssert( NULL != pBitmap );

	// Deinit members
	if ( NULL != pBitmap->pucData )
	{
		cl_free(pBitmap->pucData);
    /*
      pucData must be NULL so that BmIsValid will know that the
      bitmap is invalid.
    */
		pBitmap->pucData = NULL;
	}
}


//**********************************************************************
//  FUNCTION:
//    BmLoad
//
//  DESCRIPTION:	
//    Loads a bitmap from a BMP file.  Memory is allocated for the 
//    for the bitmap data.  A palette is loaded if one exists.  The image
//    will be loaded into the bitmap bottom up ( the first byte is the lower
//    left hand corner of the image.)
//
//  Inputs:
//    Bitmap *pBitmap
//      A pointer to the bitmap that the images will be loaded into.
//    LPCTSTR lpcszBitmapFile
//      Path to the BMP file.
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
void BmLoad(Bitmap *pBitmap, char *psFilePath )
{
  ClBitmapInfoHeader bih;
  ClBitmapFileHeader bfh;
  CLFILE* file = NULL;
  _RGBQUAD palette[PALETTE_LEN];
  int iPadSize;
  int iRowIndex;
  unsigned char *pucBits = NULL;

  ClAssert( NULL != psFilePath );

	// Open .BMP file
	file = cl_fopen(psFilePath, "rb");
	if (file != NULL)
	{
    ReadClBitmapFileHeader( &bfh, file );
    ReadClBitmapInfoHeader( &bih, file );
    
    BmCreate
    (
      pBitmap,
      bih.nWidth,
      /* bih.biHeight will be negative if the image is stored top down
        ( the origin in the upper left corner).
        The image needs to be stored bottom up.
      */
      bih.nHeight < 0 ? -bih.nHeight : bih.nHeight,
      NULL // Do not initialize the palette.
    );

		// Read palette info
		cl_fread( palette, sizeof(_RGBQUAD), sizeof(palette)/sizeof(palette[0]), file );

    PaletteCopyFromRgbQuad
    (
      &pBitmap->palette,
      palette,
      ( bih.unClrUsed == 0 ) ? 256 : (unsigned short)bih.unClrUsed 
    );

		/*
      Read image data. Rows are padded to 4 byte boundries.
      Do not copy the padding into pBitmap->pucData.
    */
    cl_fseek(file, bfh.unOffBits, SEEK_SET );
    iPadSize = 4 - bih.nWidth & 0x3;
    pucBits = pBitmap->pucData;
    for( iRowIndex = 0; iRowIndex < pBitmap->nHeight; ++iRowIndex )
    {
		  cl_fread( pucBits, 1, pBitmap->nWidth, file );
      pucBits += pBitmap->nWidth;
      cl_fseek(file, iPadSize, SEEK_CUR );
    }
    
    /*
      If the image is stored top down flip it so that it is stored bottom up.
      The first pixel in lpData points to the upper left hand corner of a 
      top down image.
      The first pixel in lpData points to the lower left hand corner of a 
      bottom up image.
    */
    if( bih.nHeight < 0 )
      BmFlipVertical(pBitmap);

		// Close .BMP file
		cl_fclose(file);
  }
	
}

//**********************************************************************
//  FUNCTION:
//    ReadClBitmapFileHeader
//
//  DESCRIPTION:	
//    Reads a bitmap file header from a file and convert the fields
//    to little format.
//
//  Inputs:
//    ClBitmapFileHeader *pHeader
//      Bitmap file header    
//    CLFILE *pFile
//      Pointer to the file that header is read from.
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
static void ReadClBitmapFileHeader( ClBitmapFileHeader *pHeader, CLFILE *pFile )
{
  pHeader->usType = cl_freadS16(pFile);
  pHeader->unSize = cl_freadS32(pFile);
  pHeader->usReserved1 = cl_freadS16( pFile );
  pHeader->usReserved2 = cl_freadS16( pFile );
  pHeader->unOffBits = cl_freadS32(pFile);
}

//**********************************************************************
//  FUNCTION:
//    ReadClBitmapInfoHeader
//
//  DESCRIPTION:	
//    Reads a bitmap info header from a file and convert the fields
//    to little format.
//
//  Inputs:
//    ClBitmapInfoHeader *pHeader
//      Bitmap info header    
//    CLFILE *pFile
//      Pointer to the file that header is read from.
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
static void ReadClBitmapInfoHeader( ClBitmapInfoHeader *pHeader, CLFILE *pFile )
{
  pHeader->unSize = cl_freadS32( pFile );
  pHeader->nWidth = cl_freadS32( pFile );
  pHeader->nHeight = cl_freadS32( pFile );
  pHeader->usPlanes = cl_freadS16( pFile );
  pHeader->usBitCount = cl_freadS16( pFile );
  pHeader->unCompression = cl_freadS32( pFile );
  pHeader->unSizeImage = cl_freadS32( pFile );
  pHeader->nXPelsPerMeter = cl_freadS32( pFile );
  pHeader->nYPelsPerMeter = cl_freadS32( pFile );
  pHeader->unClrUsed = cl_freadS32( pFile );
  pHeader->unClrImportant = cl_freadS32( pFile );
}

//**********************************************************************
//  FUNCTION:
//    BmFlipVertical
//
//  DESCRIPTION:	
//    Turns the bitmap upside down.  Converts a top down bitmap to a
//    bottom up bitmap or visa-versa.
//    The first pixel in pBitmap->lpData will points to the upper left hand 
//    corner of a top down image.
//    The first pixel in pBitmap->lpData points to the lower left hand corner 
//    of a bottom up image.
//
//  Inputs:
//    Bitmap *pBitmap
//      A pointer to the bitmap.
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
static void BmFlipVertical( Bitmap *pBitmap )
{
  // Pointer to the top row.
  unsigned char *pTop = pBitmap->pucData;
  // pointer to bottom row.
  unsigned char *pBottom = BM_ROW_GET( pBitmap, ( pBitmap->nHeight - 1 ) );
  // temporary storage for a row.
  unsigned char *pTemp = (unsigned char *)cl_malloc(pBitmap->nWidth);

  if( NULL != pTemp )
  {

    while( pTop < pBottom )
    {
      // Swap pTop and pBottom data.
      memmove(pTemp, pTop, pBitmap->nWidth);
      memmove(pTop, pBottom, pBitmap->nWidth);
      memmove(pBottom, pTemp, pBitmap->nWidth);

      // Move up one row.
      pTop += pBitmap->nWidth;
      // Move down one row.
      pBottom -= pBitmap->nWidth;
    }

    cl_free(pTemp);
  }
}

//**********************************************************************
//  FUNCTION:
//    BmRotateAndCopy
//
//  DESCRIPTION:	
//    Copies to a destination bitmap.  The destination will look like
//    a rotated version of the source bitmap.
//
//    The soruce bitmap will not be altered.
//
//  Inputs:
//    Bitmap *pSrcBitmap
//      A pointer to the bitmap.
//    int iSrcXCenter
//    int iSrcYCenter
//      The point that is at the axis of rotation in the source bitmap.
//    int iDegrees
//      The number of degrees to rotate the bitmap as it is being copied.
//      If degrees > 0 then counter-clockwise rotation.
//      If degrees < 0 then clockwise rotation.
//    unsigned char *pucDestBitmap
//      Bytes are rotated and copied into this buffer.
//    int iDestWidth
//      Total Width of the destination bitmap.
//    int iDestHeight
//      Total height of the destination bitmap.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void BmRotateCopy
(
  Bitmap *pSrcBitmap,
  int iSrcXCenter,
  int iSrcYCenter,
  int iDegrees,
  unsigned char *pucDestBitmap,
  int iDestWidth,
  int iDestHeight
)
{  
  fixed fxAngle;
  fixed fxCos;
  fixed fxSin;

	fixed fxSrcCenterX;
	fixed fxSrcCenterY;

  int iSrcX;
  int iSrcY;
  fixed fxSrcXIndex;
  fixed fxSrcYIndex;
  fixed fxRowStartXIndex;
  fixed fxRowStartYIndex;
  fixed fxCurSrcYIndex; 
  int iYIncr;
  int iYIncr2;

  int iDestXIndex;
  int iDestYIndex;

	unsigned char *pucSrcRow = NULL;
	unsigned char *pucSrcPixel = NULL;
	unsigned char *pucDestPixel = NULL;

  ClAssert( NULL != pSrcBitmap );
  ClAssert( NULL != pucDestBitmap );
  ClAssert( iSrcXCenter >= 0 );
  ClAssert( iSrcYCenter >= 0 );
  ClAssert( iDestWidth >= 0 );
  ClAssert( iDestHeight >= 0 );

	// Check for valid bitmap
	if ( BmIsValid(pSrcBitmap) )
	{
		/*
      Convert degrees to radians.
      Radians must be negated.  First we rotate backwards from the destination
      to the source coordinate.  Then we copy a pixel from the source
      coordinate to the destination coordinate.
    */
    fxAngle = -Mulfx(Divfx(iDegrees,180), FX_PI);

		fxCos = FxCos(fxAngle);
		fxSin = FxSin(fxAngle);

		fxSrcCenterX = itofx( iSrcXCenter );
		fxSrcCenterY = itofx( iSrcYCenter );

    /*
      Compute the location of the source pixel that will be copied to 
      pixel (0,0) in the destination bitmap.
    */
    {
		  int fxX = -itofx( iDestWidth >> 1 );
		  int fxY = -itofx( iDestHeight >> 1 );
		  fixed fxSrcX = Mulfx(fxX,fxCos) - Mulfx(fxY,fxSin) + f_0_5 + fxSrcCenterX;
		  fixed fxSrcY = Mulfx(fxX,fxSin) + Mulfx(fxY,fxCos) + f_0_5 + fxSrcCenterY;
		  iSrcX  = fxtoi(fxSrcX);
		  iSrcY = fxtoi(fxSrcY);
    }

    /*********************************
      Setup source pointers and indexes.
    *********************************/

    /*
      Compute the offset of the source pixel in the source bitmap's byte array.
    */
    pucSrcRow = pSrcBitmap->pucData + iSrcY*pSrcBitmap->nWidth;
    pucSrcPixel = pucSrcRow;

    /*
      Store the location of the first pixel in a row.
      After each row is copied fxRowStartXIndex and fxRowStartYIndex
      are used to compute pucSrcRow for the next row.
    */
    fxRowStartXIndex = fxSrcXIndex = itofx(iSrcX);
    fxRowStartYIndex = fxSrcYIndex = itofx(iSrcY);
    fxCurSrcYIndex = fxSrcYIndex;

    /*
      iYIncr is added to pucSrcPixel when the source row increases by 1 while 
      iterating through pixels in a row.
      iYIncr2 is added to pucSrcRow before each row is copied.
    */
    if( fxSin > 0 )
      iYIncr = pSrcBitmap->nWidth; // Angles in quadrant 3 and 4
    else
      iYIncr = -pSrcBitmap->nWidth;  // Angles in quadrant 1 and 2

    if( fxCos >= 0 )
      iYIncr2 = pSrcBitmap->nWidth;  // Angles in quadrant 1 and 4
    else
      iYIncr2 = -pSrcBitmap->nWidth;  // Angles in quadrant 2 and 3

    /*********************************
      Setup the destination pointer.
    *********************************/

    pucDestPixel = pucDestBitmap;

    /*********************************
      Rotate and copy the bitmap.
    *********************************/
    /*
      For each pixel in the destination, compute its location in the
      source bitmap then copy the pixel.
    */
		for ( iDestYIndex = 0; iDestYIndex < iDestHeight; iDestYIndex++ )
		{
			for ( iDestXIndex = 0; iDestXIndex < iDestWidth; iDestXIndex++ )
			{
        // Skip source pixels that are out side of the bitmap.
        if( 
            fxtoi(fxSrcXIndex + f_0_5) >= 0 && fxtoi(fxSrcYIndex + f_0_5) >= 0 &&
            fxtoi(fxSrcXIndex + f_0_5) < pSrcBitmap->nWidth && fxtoi(fxSrcYIndex + f_0_5) < pSrcBitmap->nHeight
        )
          *pucDestPixel = *( pucSrcPixel + fxtoi(fxSrcXIndex + f_0_5) );

        pucDestPixel++;

        fxSrcXIndex += fxCos;
        fxSrcYIndex += fxSin;

        if( fxround(fxSrcYIndex) - fxround(fxCurSrcYIndex) )
        {
          pucSrcPixel += iYIncr;
          fxCurSrcYIndex = fxSrcYIndex;
        }

			}

      /*********************************
        Setup source pointers and indexes for the next row.
      *********************************/
      fxRowStartXIndex = fxRowStartXIndex - fxSin;
      fxSrcXIndex = fxRowStartXIndex;

      fxCurSrcYIndex = fxRowStartYIndex;
      fxRowStartYIndex = fxRowStartYIndex + fxCos;
      fxSrcYIndex = fxRowStartYIndex;

      if( fxround(fxSrcYIndex) - fxround(fxCurSrcYIndex) )
      {
        pucSrcRow += iYIncr2;
        fxCurSrcYIndex = fxSrcYIndex;
      }

      pucSrcPixel = pucSrcRow;

		}
	}
}

//**********************************************************************
//  FUNCTION:
//    BmClear
//
//  DESCRIPTION:	
//   Clears a bitmap.  Every pixel is set to a color.
//
//  Inputs:
//    Bitmap *pBitmap
//      A pointer to the bitmap.
//    unsigned char ucClearColor
//      The color in RGB format.
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
void BmClear( Bitmap *pBitmap, unsigned char ucClearColor)
{
  ClAssert( NULL != pBitmap );

	// Check for valid bitmap
	if ( BmIsValid(pBitmap) )
		// Clear bitmap
		memset
    (
      pBitmap->pucData,
      ucClearColor,
      CALC_BITMAP_BYTE_SIZE( pBitmap->nWidth, pBitmap->nHeight )
    );
}

//**********************************************************************
//  FUNCTION:
//    BmIsValid
//
//  DESCRIPTION:	
//    Used to check if a bitmap is valid.
//
//  Inputs:
//    Bitmap *pBitmap
//      A pointer to the bitmap.
//
//  Outputs:
//    Returns TRUE if the bitmap is valid.
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL BmIsValid( Bitmap *pBitmap )
{
  ClAssert( NULL != pBitmap );

  return (pBitmap->pucData != NULL);
}

#if !defined(HUMMINBIRD)
#pragma warning(default : 4996) 
#endif

#ifdef __cplusplus
}
#endif

