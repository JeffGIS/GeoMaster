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
//    StretchHBbits.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    3/31/2009
//
//  DESCRIPTION:
//    Function used for copying and resizing bitmaps.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "Chart.h"
#include "FixedPoint.h"
#include "ClAssert.h"
#include "StretchHBBits.h"

//**********************************************************************
// Types
//**********************************************************************
typedef struct _tagRECT { 
  int left; 
  int top; 
  int right; 
  int bottom; 
} _RECT;

//**********************************************************************
// macros
//**********************************************************************
#define RECT_WIDTH( rect ) \
  ( (rect).right - (rect).left )

#define RECT_HEIGHT( rect ) \
  ( (rect).bottom - (rect).top )

#define XY_TO_OFFSET( x, y, bitmap, width ) \
  ( (bitmap) + (x) + (y) * (width) )

//**********************************************************************
// Private Function Prototypes.
//**********************************************************************
static void ClipRectangles
(
	_RECT *pSrcRect, 
  _RECT *pDestRect,
	int nSrcBitmapHeight,
  int nSrcBitmapWidth,
  int nDestBitmapHeight,
  int nDestBitmapWidth,
  fixed fxHorizScale,
  fixed fxVertScale
);

void ShrinkCopyBitmap
(
  _RECT *pSrcRect, 
  _RECT *pDestRect,
  int nSrcBitmapWidth,
  int nSrcBitmapHeight,
  fixed fxHorizScale,
  fixed fxVertScale,
  unsigned char *pucSrcBitmap,
  _RGBQUAD *pSrcPalette,
  int nSrcPaletteLen,
  Bitmap *pDestBitmap,
  CL_BOOL bUseTransparentColor,
  _RGBQUAD *pTransparentColor
);

void ExpandCopyBitmap
(
  _RECT *pSrcRect, 
  _RECT *pDestRect,
  int nSrcBitmapWidth,
  int nSrcBitmapHeight,
  fixed fxHorizScale,
  fixed fxVertScale,
  unsigned char *pucSrcBitmap,
  _RGBQUAD *pSrcPalette,
  int nSrcPaletteLen,
  Bitmap *pDestBitmap,
  CL_BOOL bUseTransparentColor,
  _RGBQUAD *pTransparentColor
);

static void SetDestPixelColor
(
  CL_BOOL bFirstTime,
  unsigned char  *pucDestPixel,
  unsigned char  ucSrcPixel,
  _RGBQUAD *pSrcPalette,
  PaletteMap  *pPaletteMap,
  Bitmap  *pDestBitmap,
  fixed   fxHorizScale,
  fixed   fxVertScale
);

//**********************************************************************
// Functions
//**********************************************************************
//**********************************************************************
//  FUNCTION:
//    StretchHBBits
//
//  DESCRIPTION:	
//    Copies a section of a chart's tile to a specific location of a bitmap.
//    If the source and destination rectangles differ then the pixels in the
//    destinatin will be scaled.
//
//    The origin of both bitmaps is the lower left corner.  XDest, YDest,
//    XSrc, and YSrc are releative to the upper left corner of the bitmap's.
//
//  Inputs:
//    HANDLE HBImageHandle
//      Pointer to a Bitmap struct.
//    int XDest
//      Specifies the x-coordinate, in pixels, of the upper-left corner of the
//      composite bitmap.
//    int YDest
//      Specifies the y-coordinate, in pixels, of the upper-left corner of 
//      the composite bitmap.
//    int nDestWidth
//      Specifies the width, in pixels, of the composite bitmap. 
//    int nDestHeight
//      Specifies the height, in pixels, of the composite bitmap.
//    int XSrc
//      Specifies the x-coordinate, in pixels, of the source rectangle in the
//      image tile.
//    int YSrc
//      Specifies the y-coordinate, in pixels, of the source rectangle in the
//      image tile.
//    int nSrcWidth
//      Specifies the width, in pixels, of the source rectangle in the image
//      tile.
//    int nSrcHeight
//      Specifies the height, in pixels, of the source rectangle in the image
//      tile.
//    unsigned char *puBytes
//      Pointer to the image tile bytes. All image tiles are 8bits per pixel.
//    int TileHeight
//      The height of the tile bitmap.
//    int TileWidth
//      The width of the tile bitmap.
//    int TilePaletteLen
//      The number of entries in the color palette.
//    _RGBQUAD *TilePalette
//      Pointer to the color palette for this tile. 
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
int StretchHBBits
(
  HANDLE HBImageHandle,
  int XDest,
  int YDest,
  int nDestWidth,
  int nDestHeight,
	int XSrc,
  int YSrc,
  int nSrcWidth,
  int nSrcHeight, 
	unsigned char *pucBytes,
	int TileHeight,
  int TileWidth,
	int TilePaletteLen,
	_RGBQUAD *TilePalette
)
{
  _RGBQUAD color = {0,0,0,0}; 

  Bitmap *pDestBitmap = &((ChartInstance *)HBImageHandle)->bm;

  return TransparentStretchHBBits
  (
    pDestBitmap,
    XDest,
    YDest,
    nDestWidth,
    nDestHeight,
	  XSrc,
    YSrc,
    nSrcWidth,
    nSrcHeight, 
	  pucBytes,
	  TileHeight,
    TileWidth,
	  TilePaletteLen,
	  TilePalette,
    CL_FALSE,
    &color
  );

}

//**********************************************************************
//  FUNCTION:
//    TransparentStretchHBBits
//
//  DESCRIPTION:	
//    Copies a section of a bitmap to a specific location of a destination 
//    bitmap.  If the source and destination rectangles differ then the 
//    pixels in the destinatin will be scaled.
//
//    If bUseTransparentColor is CL_TRUE then any pixel containing 
//    *pTransparentColor will not be copied.
//
//  Inputs:
//    Bitmap *pDestBitmap
//      The destination bitmap.
//    int XDest
//      Specifies the x-coordinate, in pixels, of the upper-left corner of the
//      composite bitmap.
//    int YDest
//      Specifies the y-coordinate, in pixels, of the upper-left corner of 
//      the composite bitmap.
//    int nDestWidth
//      Specifies the width, in pixels, of the composite bitmap. 
//    int nDestHeight
//      Specifies the height, in pixels, of the composite bitmap.
//    int XSrc
//      Specifies the x-coordinate, in pixels, of the source rectangle in the
//      image tile.
//    int YSrc
//      Specifies the y-coordinate, in pixels, of the source rectangle in the
//      image tile.
//    int nSrcWidth
//      Specifies the width, in pixels, of the source rectangle in the image
//      tile.
//    int nSrcHeight
//      Specifies the height, in pixels, of the source rectangle in the image
//      tile.
//    unsigned char *puBytes
//      Pointer to the image tile bytes. All image tiles are 8bits per pixel.
//    int TileHeight
//      The height of the tile bitmap.
//    int TileWidth
//      The width of the tile bitmap.
//    int TilePaletteLen
//      The number of entries in the color palette.
//    _RGBQUAD *TilePalette
//      Pointer to the color palette for this tile. 
//    CL_BOOL bUseTransparentColor
//      CL_TRUE - Do not copy source pixels that are the transparent color.
//      CL_FALSE - Copy all pixels.
//    _RGBQUAD *pTransparentColor
//      The color that will be treated as transparent.
//
//  Outputs:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
int TransparentStretchHBBits
(
  Bitmap *pDestBitmap,
  int XDest,
  int YDest,
  int nDestWidth,
  int nDestHeight,
	int XSrc,
  int YSrc,
  int nSrcWidth,
  int nSrcHeight, 
	unsigned char *pucBytes,
	int TileHeight,
  int TileWidth,
	int TilePaletteLen,
	_RGBQUAD *TilePalette,
  CL_BOOL bUseTransparentColor,
  _RGBQUAD *pTransparentColor
)
{
  _RECT srcRect;
  _RECT destRect;
  fixed fxHorizScale = 0;
  fixed fxVertScale = 0;

  ClAssert( NULL != pDestBitmap );
  ClAssert( NULL != pucBytes );
  ClAssert( TileHeight >= 0 ); 
  ClAssert( TileWidth >= 0 );
  ClAssert( NULL != TilePalette );
  ClAssert( 
    !bUseTransparentColor || bUseTransparentColor && NULL != pTransparentColor
  );

  srcRect.left = XSrc;
  srcRect.top = YSrc;
  srcRect.right = XSrc + nSrcWidth;
  srcRect.bottom = YSrc + nSrcHeight;
  destRect.left = XDest;
  destRect.top = YDest;
  destRect.right = XDest + nDestWidth;
  destRect.bottom = YDest + nDestHeight;


	// Dest bitmap must be valid.
	if( !BmIsValid(pDestBitmap) || 
      nSrcWidth <= 0 || nSrcHeight <= 0 || 
      nDestWidth <= 0 || nDestHeight <= 0  
  )
	  return 0;

  /*
    The rectangles must be in the bitmap.
  */
  if(
    srcRect.right < 0 || srcRect.left > TileWidth  || 
    srcRect.bottom < 0 || srcRect.top > TileHeight ||    
    destRect.right < 0 || destRect.left > pDestBitmap->nWidth ||
    destRect.bottom < 0 || destRect.top > pDestBitmap->nHeight
  )
    return 0;

  /*
    Computing scaling factors.
    This must be done before the source and destination rectangles
    are clipped.
  */
  fxHorizScale = Divfx( nSrcWidth, nDestWidth );
  fxVertScale =  Divfx( nSrcHeight, nDestHeight );

  ClipRectangles
  (
    &srcRect,
    &destRect,
    TileHeight,
    TileWidth,
    pDestBitmap->nHeight,
    pDestBitmap->nWidth,
    fxHorizScale,
    fxVertScale
   );

  nSrcHeight = RECT_HEIGHT(srcRect);
  nDestHeight = RECT_HEIGHT(destRect);

  if( 
    RECT_WIDTH( srcRect ) > 0 && srcRect.left >= 0 &&
    srcRect.right <= TileWidth &&
    nSrcHeight > 0 && srcRect.top >= 0 &&
    srcRect.bottom <= TileHeight &&
    RECT_WIDTH(destRect) > 0 && destRect.left >= 0 &&
    destRect.right <= pDestBitmap->nWidth &&
    nDestHeight > 0 && destRect.top >= 0 &&
    destRect.bottom <= pDestBitmap->nHeight
  )
  {

    srcRect.top = TileHeight - srcRect.top - nSrcHeight;
    srcRect.bottom = srcRect.top + nSrcHeight;
    destRect.top = pDestBitmap->nHeight - destRect.top - nDestHeight;
    destRect.bottom = destRect.top + nDestHeight;

    if( fxHorizScale >  itofx(1) )
      ShrinkCopyBitmap
      (
        &srcRect,
        &destRect,
        TileWidth,
        TileHeight,
        fxHorizScale,
        fxVertScale,
        pucBytes,
        TilePalette,
        TilePaletteLen,
        pDestBitmap,
        bUseTransparentColor,
        pTransparentColor
      );
    else
      ExpandCopyBitmap
      (
        &srcRect,
        &destRect,
        TileWidth,
        TileHeight,
        fxHorizScale,
        fxVertScale,
        pucBytes,
        TilePalette,
        TilePaletteLen,
        pDestBitmap,
        bUseTransparentColor,
        pTransparentColor
      );
  }

	return 0;
}

//**********************************************************************
//  FUNCTION:
//    ClipRectangles
//
//  DESCRIPTION:	
//    Clips source and destination rectangles to a source and destination
//    bitmap.
//
//  Inputs:
//    _RECT *pSrcRect
//      Rectangle in a source bitmap.
//    _RECT *pDestRect
//      Rectangle in a destination bitmap.
//    int nSrcBitmapHeight
//      Height of source bitmap.
//    int nSrcBitmapWidth
//      Width of source bitmap.
//    int nDestBitmapHeight
//      Height of destination bitmap.
//    int nDestBitmapWidth
//      Width of destination bitmap.
//    fixed fxHorizScale
//      Horizontal scale = nSrcWidth / nDestWidth
//    fixed fxVertScale
//      Verticale scale = nSrcHeight / nDestHeight
//
//  Outputs:
//    _RECT *pSrcRect
//      Outputs a clipped rectagle.
//    _RECT *pDestRect
//      Outputs a clipped rectagle.
//
//  GLOBALS:
//    none
//**********************************************************************
static void ClipRectangles
(
	_RECT *pSrcRect, 
  _RECT *pDestRect,
	int nSrcBitmapHeight,
  int nSrcBitmapWidth,
  int nDestBitmapHeight,
  int nDestBitmapWidth,
  fixed fxHorizScale,
  fixed fxVertScale
)
{
  ClAssert( NULL != pSrcRect );
  ClAssert( NULL != pDestRect );
  ClAssert( fxHorizScale > 0 );
  ClAssert( fxVertScale > 0 );

  /**********
    Clip source and destination rectangles on the source bitmap.
  **********/

  /*
    Clip the source rectangle to the left edge of the source bitmap.
  */
  if ( pSrcRect->left < 0 )
  {
    /*
      Clip the corresponding region from the destination rectangle.
      pDestRect->left -= pSrcRect->left / fxHorizScale;
    */
    pDestRect->left += fxtoi( Divfx( itofx( -pSrcRect->left ), fxHorizScale ) );
    pSrcRect->left = 0;
  }

  /*
    Clip the right edge of the source rectangle to the source bitmap.
  */
  if( pSrcRect->right > nSrcBitmapWidth )
  {
    /*
      Clip the corresponding region from the destination rectangle.
      pDestRect->right -= nDelta / fxHorizScale;
    */
    int nDelta = pSrcRect->right - nSrcBitmapWidth;
    pDestRect->right -= fxtoi( Divfx( itofx(nDelta), fxHorizScale ) );
    pSrcRect->right = nSrcBitmapWidth;
  }

  /**********
    Clip source and destination rectangles on the destination bitmap.
  **********/
  /*
    Clip the source rectangle to the top edge of the source bitmap.
  */
  if ( pSrcRect->top < 0 )
  {
    /*
      Clip the corresponding region from the destination rectangle.
      pDestRect->top = itofx( pSrcRect->top ) / fxVertScale;
    */
    pDestRect->top += fxtoi( Divfx( itofx( -pSrcRect->top ), fxVertScale ) );
    pSrcRect->top = 0;
  }

  /*
    Clip the bottom edge of the source rectangle to the source bitmap.
  */
  if( pSrcRect->bottom > nSrcBitmapHeight )
  {
    /*
      Clip the corresponding region from the destination rectangle.
      pDestRect->bottom -= nDelta / fxVertScale;
    */
    int nDelta = pSrcRect->bottom - nSrcBitmapHeight;
    pDestRect->bottom -= fxtoi( Divfx( itofx(nDelta), fxVertScale ) );
    pSrcRect->bottom = nSrcBitmapHeight;
  }


  /*
    Clip the dest rectangle to the left edge of the destination bitmap.
  */
  if ( pDestRect->left < 0 )
  {
    /*
      Clip the corresponding region from the source rectangle.
      pSrcRect->left -= pDestRect->left * fxHorizScale;
    */
    pSrcRect->left += fxtoi( -pDestRect->left * fxHorizScale );
    pDestRect->left = 0;
  }


  /*
    Clip the right edge of the destination rectangle to the destination bitmap.
  */
  if( pDestRect->right > nDestBitmapWidth )
  {
    /*
      Clip the corresponding region from the source rectangle.
      pSrcRect->right -= nDelta * fxHorizScale;
    */
    int nDelta = pDestRect->right - nDestBitmapWidth;
    pSrcRect->right -= fxtoi( nDelta * fxHorizScale );
    pDestRect->right = nDestBitmapWidth;
  }

  /*
    Clip the dest rectangle to the top edge of the destination bitmap.
  */
  if ( pDestRect->top < 0 )
  {
    /*
      Clip the corresponding region from the source rectangle.
      pSrcRect->top = pDestRect->top / fxVertScale;
    */
    pSrcRect->top += fxtoi( -pDestRect->top * fxVertScale );
    pDestRect->top = 0;
  }

  /*
    Clip the bottom edge of the destination rectangle to the destination bitmap.
  */
  if( pDestRect->bottom > nDestBitmapHeight )
  {
    /*
      Clip the corresponding region from the source rectangle.
      pSrcRect->bottom -= nDelta * fxVertScale;
    */
    int nDelta = pDestRect->bottom - nDestBitmapHeight;
    pSrcRect->bottom -= fxtoi( nDelta * fxVertScale );
    pDestRect->bottom = nDestBitmapHeight;
  }

}


//**********************************************************************
//  FUNCTION:
//    ShrinkCopyBitmap
//
//  DESCRIPTION:	
//    Copies a source bitmap to the destination bitmap.  Source
//    and destination rectangles must be clipped to their respective
//    bitmaps.  Antialiasing and scaling are performed as bits are copied.
//
//  Inputs:
//    _RECT *pSrcRect
//      Rectangle in the source bitmap
//    _RECT *pDestRect
//      Rectangle in the destination bitmap
//    int nSrcBitmapWidth
//      Width of source bitmap.
//    int nSrcBitmapHeight
//      Height of source bitmap.
//    fixed fxHorizScale
//      Horizontal scale = nSrcBitmapWidth / nDestBitmapWidth
//    fixed fxVertScale
//      Verticale scale = nSrcBitmapHeight / nDestBitmapHeight
//    unsigned char *pucSrcBitmap
//    _RGBQUAD *pSrcPalette
//      Color palette 
//    int nSrcPaletteLen
//      The number of colors used. The value cannot be larger than
//      BM_PALETTE_LEN_MAX.
//    Bitmap *pDestBitmap
//      Contains width, height and palette.  The source palette is merged
//      into the destination bitmap pallette.
//  Outputs:
//    Bitmap *pDestBitmap
//      The destination bitmap contains pixels from the source bitmap.
//
//  GLOBALS:
//    none
//**********************************************************************
void ShrinkCopyBitmap
(
  _RECT *pSrcRect, 
  _RECT *pDestRect,
  int nSrcBitmapWidth,
  int nSrcBitmapHeight,
  fixed fxHorizScale,
  fixed fxVertScale,
  unsigned char *pucSrcBitmap,
  _RGBQUAD *pSrcPalette,
  int nSrcPaletteLen,
  Bitmap *pDestBitmap,
  CL_BOOL bUseTransparentColor,
  _RGBQUAD *pTransparentColor
)
{
  PaletteMap paletteMap;
  int nPrevDestRowInd = -1;
  int nDestRowInd;
  int nDestColInd;

  int nSrcRowInd;
  int nSrcColInd;  

  int nDestWidth = RECT_WIDTH( *pDestRect );
  int nDestHeight = RECT_HEIGHT( *pDestRect );

  unsigned char *pucSrcPixels = NULL;
  unsigned char *pucDestPixels = NULL;

  unsigned char *pucSrcRow = NULL;
  unsigned char *pucDestRow = NULL;

  ClAssert( NULL != pSrcRect );
  ClAssert( NULL != pDestRect );
  ClAssert( nDestWidth >= 0 );
  ClAssert( nDestHeight >= 0 );
  ClAssert( fxHorizScale > 0 );
  ClAssert( fxVertScale > 0 );
  ClAssert( NULL != pucSrcBitmap );
  ClAssert( NULL != pSrcPalette );
  ClAssert( NULL != pDestBitmap );

  PaletteMapInit( &paletteMap );

  /*
    Go to top row in the source rectangle.
  */
  pucSrcRow = XY_TO_OFFSET
  (
    pSrcRect->left,
    pSrcRect->top,
    pucSrcBitmap,
    nSrcBitmapWidth
  );

  /*
    Go to top row in the destination rectangle.
  */
  pucDestRow = XY_TO_OFFSET
  (
    pDestRect->left,
    pDestRect->top,
    pDestBitmap->pucData,
    pDestBitmap->nWidth
  );

  /*
    Invert the scale so that division can be used instead of multiplication
    when computing the destination pixel coordinate.
  */
  fxVertScale  = Divfx( itofx( 1 ), fxVertScale );
  fxHorizScale = Divfx( itofx( 1 ), fxHorizScale );

  
  /**********
    Copy each row in the source rectangle to the destination rectangle.
  *********/
  for( nSrcRowInd = 0 ; nSrcRowInd < RECT_HEIGHT( *pSrcRect ); ++nSrcRowInd )
  {
    
    pucSrcPixels = pucSrcRow;

    nDestRowInd = fxtoi( fxround(nSrcRowInd  * fxVertScale) );

    // Check for destination bitmap overrun.
    if( nDestRowInd >= RECT_HEIGHT( *pDestRect ) )
      break;

    /**********
      Copy the pixels in the current row.
    *********/
    for( nSrcColInd = 0; nSrcColInd < RECT_WIDTH( *pSrcRect ); ++nSrcColInd )
    {

      if( bUseTransparentColor &&
          pTransparentColor->rgbRed == pSrcPalette[ pucSrcPixels[nSrcColInd] ].rgbRed &&
          pTransparentColor->rgbGreen == pSrcPalette[ pucSrcPixels[nSrcColInd] ].rgbGreen &&
          pTransparentColor->rgbBlue == pSrcPalette[ pucSrcPixels[nSrcColInd] ].rgbBlue 
      )
        continue;

      /*
        Get the destination pixel.
      */
      nDestColInd = fxtoi( fxround(nSrcColInd  * fxHorizScale) );

      // Check for destination bitmap overrun.
      if( nDestColInd >= RECT_WIDTH( *pDestRect ) )
        break;

      /**********
        Go to the location in the destination bitmap.
      **********/      
      pucDestPixels = XY_TO_OFFSET
      (
        nDestColInd,
        nDestRowInd,
        pucDestRow,
        pDestBitmap->nWidth
      );

      /*
        Zomming out.
        Copy the source pixel to the destination without aliasing.
      */
      SetDestPixelColor
      (
        nDestRowInd != nPrevDestRowInd ? CL_TRUE: CL_FALSE,
        pucDestPixels,
        pucSrcPixels[nSrcColInd],
        pSrcPalette,
        &paletteMap,
        pDestBitmap,
        fxHorizScale,
        fxVertScale          
       );
    }

    // Go to next row.
    pucSrcRow += nSrcBitmapWidth;    
    nPrevDestRowInd = nDestRowInd;
  }

}


//**********************************************************************
//  FUNCTION:
//    ExpandCopyBitmap
//
//  DESCRIPTION:	
//    Copies a source bitmap to the destination bitmap.  Source
//    and destination rectangles must be clipped to their respective
//    bitmaps.  Antialiasing and scaling are performed as bits are copied.
//
//  Inputs:
//    _RECT *pSrcRect
//      Rectangle in the source bitmap
//    _RECT *pDestRect
//      Rectangle in the destination bitmap
//    int nSrcBitmapWidth
//      Width of source bitmap.
//    int nSrcBitmapHeight
//      Height of source bitmap.
//    fixed fxHorizScale
//      Horizontal scale = nSrcBitmapWidth / nDestBitmapWidth
//    fixed fxVertScale
//      Verticale scale = nSrcBitmapHeight / nDestBitmapHeight
//    unsigned char *pucSrcBitmap
//    _RGBQUAD *pSrcPalette
//      Color palette 
//    int nSrcPaletteLen
//      The number of colors used. The value cannot be larger than
//      BM_PALETTE_LEN_MAX.
//    Bitmap *pDestBitmap
//      Contains width, height and palette.  The source palette is merged
//      into the destination bitmap pallette.
//  Outputs:
//    Bitmap *pDestBitmap
//      The destination bitmap contains pixels from the source bitmap.
//
//  GLOBALS:
//    none
//**********************************************************************
void ExpandCopyBitmap
(
  _RECT *pSrcRect, 
  _RECT *pDestRect,
  int nSrcBitmapWidth,
  int nSrcBitmapHeight,
  fixed fxHorizScale,
  fixed fxVertScale,
  unsigned char *pucSrcBitmap,
  _RGBQUAD *pSrcPalette,
  int nSrcPaletteLen,
  Bitmap *pDestBitmap,
  CL_BOOL bUseTransparentColor,
  _RGBQUAD *pTransparentColor
)
{
  PaletteMap paletteMap;
  int nDestRowInd;
  int nPrevDestRowInd = -1;
  int nDestColInd;

  int nSrcRowInd;
  int nSrcColInd;  

  int nDestWidth = RECT_WIDTH( *pDestRect );
  int nDestHeight = RECT_HEIGHT( *pDestRect );

  unsigned char ucSrcPixel;
  ClColor srcColor;
  unsigned char *pucDestPixels = NULL;

  unsigned char *pucSrcRow = NULL;
  unsigned char *pucDestRow = NULL;

  ClAssert( NULL != pSrcRect );
  ClAssert( NULL != pDestRect );
  ClAssert( nSrcBitmapWidth >= 0 );
  ClAssert( nSrcBitmapHeight >= 0 );
  ClAssert( fxHorizScale > 0 );
  ClAssert( fxVertScale > 0 );
  ClAssert( NULL != pucSrcBitmap );
  ClAssert( NULL != pSrcPalette );
  ClAssert( NULL != pDestBitmap );

  PaletteMapInit( &paletteMap );

  /*
    Go to top row in the source rectangle.
  */
  pucSrcRow = XY_TO_OFFSET
  (
    pSrcRect->left,
    pSrcRect->top,
    pucSrcBitmap,
    nSrcBitmapWidth
  );

  /*
    Go to top row in the destination rectangle.
  */
  pucDestRow = XY_TO_OFFSET
  (
    pDestRect->left,
    pDestRect->top,
    pDestBitmap->pucData,
    pDestBitmap->nWidth
  );

  /**********
    Copy each row in the source rectangle to the destination rectangle.
  *********/
  for( nDestRowInd = 0 ; nDestRowInd < RECT_HEIGHT( *pDestRect ); ++nDestRowInd )
  {
    
    pucDestPixels = pucDestRow;

    nSrcRowInd = fxtoi( nDestRowInd  * fxVertScale );
    
    // Check for source bitmap overrun.
    if( nSrcRowInd >= nSrcBitmapHeight )
      break;

    /**********
      Copy the pixels in the current row.
    *********/
    for( nDestColInd = 0; nDestColInd < RECT_WIDTH( *pDestRect ); ++nDestColInd )
    {

      /*
        Get the source pixel.
      */
      nSrcColInd = fxtoi( nDestColInd  * fxHorizScale );

      // Check for source bitmap overrun.
      if( nSrcColInd >= nSrcBitmapWidth )
        break;

      /**********
        Get the corresponding pixel in the source bitmap.
      **********/      
      ucSrcPixel = *XY_TO_OFFSET
      (
        nSrcColInd,
        nSrcRowInd,
        pucSrcRow,
        nSrcBitmapWidth
      );

      if( bUseTransparentColor &&
          pTransparentColor->rgbRed == pSrcPalette[ ucSrcPixel ].rgbRed &&
          pTransparentColor->rgbGreen == pSrcPalette[ ucSrcPixel ].rgbGreen &&
          pTransparentColor->rgbBlue == pSrcPalette[ ucSrcPixel ].rgbBlue 
      )
        continue;

      CL_RGBQUAD_TO_COLOR( srcColor, pSrcPalette[ ucSrcPixel ] );

      PaletteColorIndexGet
      (
        &pDestBitmap->palette,
        &paletteMap,
        srcColor,
        ucSrcPixel,
        pucDestPixels + nDestColInd
      );

    }

    // Go to next row.
    pucDestRow += pDestBitmap->nWidth;
  }

}


//**********************************************************************
//  FUNCTION:
//    SetDestPixelColor
//
//  DESCRIPTION:	
//    Copies a pixel from a source bitmap to a destination bitmap.
//    The source bitmap is larger than the destination bitmap by
//    horizontal and vertical scale factors that are >= 1.
//
//    Multiple source pixels map to a single destination pixel.  Typically
//    the destination pixel's color is the average of the source pixels' color.
//    Since each pixel stores a color palette index, the destination pixel
//    is the index from the source pixel closes to the average color.
//
//    Ex. The source bitmap is twice the size of the destination bitmap.
//      nHorizScale = 2 and nVertScale = 2
//      
//      4 (i.e nHorizScale * nVertScale ) source pixels map to one destination 
//      pixel.
//
//      Source Pixels
//        ---------------------
//        |  0      |    1    |
//        | RGB1    | RGB2    | 
//        |________ |_________|                 RGB1 + RGB2 + RGB3 + RGB4
//        |  2      |    3    | Average Color = -------------------------
//        | RGB3    | RGB4    |                 4 (i.e nHorizScale * nVertScale)
//        |         |         |
//        ---------------------
//
//     Destination          ||
//     Pixel                \/
//
//        ---------------------
//        |                   |
//        |                   | pixel vale =  Palette index of source pixel
//        |                   |               color that is nearest to Average
//        |                   |               Color.
//        |                   |           
//        |                   |
//        ---------------------
//
//      If the source pixel is pixel #0:
//        The source pixel is copied to the distination pixel.
//      If the source pixel is pixels #1 - 3:
//        The pixel's color is divided by nHorizScale * nVertScale to get
//        the pixel's the contribution of to the average color.  The 
//        quotient is compared to the source and destination pixel colors.
//        The color closest to the quotient is used in the distination pixel.
//
//  Inputs:
//    unsigned char ucSrcPixel
//      The source pixel is an index into pPalette.
//    int nSrcRowInd
//      The row index that the pixel belongs to.
//    _RGBQUAD *pPalette
//      Pointer to a list of colors.
//    int *panPalletteMap
//      Maps indexes from a source bitmap palette to a destination bitmap palette.
//    int nPaletteMapLen
//      The number of indexes in the palette map.  The value cannot be larger 
//      than BM_PALETTE_LEN_MAX.
//    int nHorizScale
//      Horizontal scale factor 
//        = source bitmap width in pixels / destination bitmap width in pixels.
//    int nVertScale
//      Vertical scale factor 
//        = source bitmap height in pixels / destination bitmap height in
//          pixels.
//  Outputs:
//    unsigned char *pucDestPixel
//      The location of the destination pixel.  The pixel is an index into
//      pPalette.
//
//  GLOBALS:
//    none
//**********************************************************************
static void SetDestPixelColor
(
  CL_BOOL bFirstTime,
  unsigned char  *pucDestPixel,
  unsigned char  ucSrcPixel,
  _RGBQUAD *pSrcPalette,
  PaletteMap  *pPaletteMap,
  Bitmap  *pDestBitmap,
  fixed   fxHorizScale,
  fixed   fxVertScale
)
{
  _RGBQUAD *pSrcColor;
  ClColor srcColor;
  ClColor destColor;

  _RGBQUAD scaledRgbQuad;
  ClColor scaledColor;

  int blueDist, greenDist, redDist;
  int srcColorDist, destColorDist;

  fixed fxColorScale = Mulfx( fxHorizScale, fxVertScale );

  ClAssert( NULL != pucDestPixel );
  ClAssert( NULL != pSrcPalette );
  ClAssert( NULL != pPaletteMap );
  ClAssert( NULL != pDestBitmap );
  ClAssert( fxHorizScale > 0 );
  ClAssert( fxVertScale > 0 );

  pSrcColor = pSrcPalette + ucSrcPixel;
  CL_RGBQUAD_TO_COLOR( srcColor, *pSrcColor );

  if( bFirstTime )
  {
    PaletteColorIndexGet
    (
      &pDestBitmap->palette,
      pPaletteMap,
      srcColor,
      ucSrcPixel,
      pucDestPixel
    );
  }
  else
  {

    destColor = pDestBitmap->palette.colors[*pucDestPixel];

    /*
      Compute the contribution of the source pixel color to
      to the destination pixel color.
      scaled color = source pixel color / ( nHorizScale * nVertScale )
    */
    scaledRgbQuad.rgbBlue = (unsigned char)fxtoi( pSrcColor->rgbBlue * fxColorScale );
    scaledRgbQuad.rgbGreen = (unsigned char)fxtoi( pSrcColor->rgbGreen * fxColorScale );
    scaledRgbQuad.rgbRed = (unsigned char)fxtoi( pSrcColor->rgbRed * fxColorScale );
    
    /*
      Source color distance from scaled color.
      srcColorDist = magnitude_squared( scaledColor - pSrcColor )
    */
    redDist   = (int)scaledRgbQuad.rgbRed   - pSrcColor->rgbRed;
    greenDist = (int)scaledRgbQuad.rgbGreen - pSrcColor->rgbGreen;
    blueDist  = (int)scaledRgbQuad.rgbBlue  - pSrcColor->rgbBlue;
    srcColorDist = redDist * redDist + greenDist * greenDist + blueDist * blueDist;

    /*
      Dest color distance from scaled source color.
      destColorDist = magnitude_squared( scaledColor - destColor )
    */
    CL_RGBQUAD_TO_COLOR( scaledColor, scaledRgbQuad );

    redDist   = scaledColor.ucRed   - destColor.ucRed;
    greenDist = scaledColor.ucGreen - destColor.ucGreen;
    blueDist  = scaledColor.ucBlue  - destColor.ucBlue;
    destColorDist = redDist * redDist + greenDist * greenDist + blueDist * blueDist;

    // The destination color will be color closest to the average source color.      
    if( srcColorDist < destColorDist )
    {
      PaletteColorIndexGet
      (
        &pDestBitmap->palette,
        pPaletteMap,
        srcColor,
        ucSrcPixel,
        pucDestPixel
      );
    }

  }


}     

