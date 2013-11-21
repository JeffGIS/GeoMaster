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
//    Bitmap.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    12/22/2008
//
//  DESCRIPTION:
//    Data types and functions to use for bitmaps.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef _BITMAP_H
#define _BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ChartLib.h"
#include "Palette.h"
#include "types.h"

//**********************************************************************
// Data Types
//**********************************************************************


//**********************************************************************
// bitmap macros
//**********************************************************************
// Calculate the amount of memory to used for Bitmap::lpData.
#define CALC_BITMAP_BYTE_SIZE( iWidth, iHeight ) \
  ( ( iWidth ) * ( iHeight ) )

/*
  Get the pointer to the first pixel from a row of the bitmap.
*/
#define BM_ROW_GET( pBitmap, rowNumber ) \
  ( ( pBitmap )->pucData + ( rowNumber ) * ( pBitmap )->nWidth )

//**********************************************************************
// Bitmap definition
//********************************************************************

typedef struct BitmapTag
{
  // 8 bit per pixel color palette.
  ClPalette palette;
  // Width in pixels.
  int nWidth; 
  // Height in pixels
  int nHeight;
  // Bitmap pixels.
	unsigned char *pucData;

} Bitmap;

//**********************************************************************
// Function prototypes
//**********************************************************************

CL_BOOL BmIsValid( Bitmap *pBitmap);

void BmInit( Bitmap *pBitmap );
CL_BOOL BmCreate
(
  Bitmap *pBitmap,
  int nWidth,
  int nHeight,
  ClPalette *pPalette
);

void BmDestroy( Bitmap *pBitmap );

ClError BmLoad( Bitmap *pBitmap, char *psFilePath );

void BmRotateCopy
(
  Bitmap *pSrcBitmap,
  int iSrcXCenter,
  int iSrcYCenter,
  int iDegrees,
  unsigned char *pucDestBitmap,
  int iDestWidth,
  int iDestHeight
);

void BmClear( Bitmap *pBitmap, unsigned char ucClearColor);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _BITMAP_H
