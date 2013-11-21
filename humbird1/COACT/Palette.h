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
//    Palette.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    04/10/2009
//
//  DESCRIPTION:
//    Functions for accessing palette.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef _PALETTE_H
#define _PALETTE_H

#include "ChartLib.h"

//**********************************************************************
// Parameters
//**********************************************************************

#define COLOR_INDEXES_LEN (PALETTE_LEN)
#define MAPPED_COLOR_FLAG_LEN 8
#define IS_COLOR_MAPPED( pMap, pixel ) \
  ( (pMap)->mappedFlags[(pixel) >> 5] & ( 1 << ( (pixel) & 0x1f ) ) )
#define SET_COLOR_MAPPED_BIT( pMap, pixel ) \
  ( (pMap)->mappedFlags[pixel >> 5] |= ( 1 << ( (pixel) & 0x1f ) ) )


//**********************************************************************
// Types
//**********************************************************************

typedef struct _tagRGBQUAD {
  unsigned char    rgbBlue;
  unsigned char    rgbGreen;
  unsigned char    rgbRed;
  unsigned char    rgbReserved;
} _RGBQUAD;

#define CL_RGBQUAD_TO_COLOR( color, rgbQuad ) \
{ \
  (color).ucRed = (rgbQuad).rgbRed; \
  (color).ucGreen = (rgbQuad).rgbGreen; \
  (color).ucBlue = (rgbQuad).rgbBlue; \
}

#define CL_COLOR_TO_RGBQUAD( rgbQuad, color ) \
{ \
  (rgbQuad).rgbRed = (color).ucRed; \
  (rgbQuad).rgbGreen = (color).ucGreen; \
  (rgbQuad).rgbBlue = (color).ucBlue; \
  (rgbQuad).rgbReserved = 0; \
}

#define CL_COLORREF_TO_COLOR( color, colorRef ) \
{ \
  (color).ucRed = (unsigned char)( (colorRef) & 0xff ); \
  (color).ucGreen = (unsigned char)( (colorRef) >> 8 & 0xff ); \
  (color).ucBlue = (unsigned char)( (colorRef) >> 16 & 0xff ); \
}

typedef struct PaletteMapTag
{
  unsigned char aucIndexes[COLOR_INDEXES_LEN];
  unsigned long mappedFlags[MAPPED_COLOR_FLAG_LEN];
} PaletteMap;

//**********************************************************************
// Function prototypes
//**********************************************************************

void PaletteCopy( ClPalette *pDestPalette, ClPalette *pSrcPalette );

void PaletteClear( ClPalette *pPalette );

void PaletteMapInit( PaletteMap *pMap );

CL_BOOL PaletteColorIndexGet
(
 ClPalette *pPalette,
 PaletteMap *pMap,
 ClColor color,
 unsigned char ucPixel,
 unsigned char *pucPixel
);

CL_BOOL PaletteColorAdd
(
  ClPalette *pPalette,
  ClColor color,
  int *pnColorIndex
);

void PaletteCopyFromRgbQuad( ClPalette *pPalette, _RGBQUAD *pRgbQuad, int iPaletteLen );
void PaletteCopyToRgbQuad( _RGBQUAD *pRgbQuad, int iPaletteLen, ClPalette *pPalette );

#endif // _PALETTE_H
