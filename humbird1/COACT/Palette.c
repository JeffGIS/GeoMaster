//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// NNN     NN  PPPPPPP  EEEEEE
// NN NN   NN  PP    PP EE
// NN  NN  NN  PPPPPPP  EEEEE
// NN   NN NN  PP       EE
// NN     NNN  PP       EEEEEEE
//
// Copyright © 2009 by North Pole Engineering, Inc.  All rights reserved.
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
//    Palette.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    04/10/2009
//
//  DESCRIPTION:
//    Functions for accessing palette's.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include "Palette.h"
#include "ClAssert.h"

//**********************************************************************
// Macros
//**********************************************************************

//**********************************************************************
// Parameters
//**********************************************************************

//**********************************************************************
// Private Function Prototypes.
//**********************************************************************
static CL_BOOL PaletteColorAppend
(
  ClPalette *pPalette,
  ClColor color,
  int *pnColorIndex
);

static CL_BOOL PaletteColorAt
(
  ClPalette *pPalette,
  ClColor color,
  int *pnColorIndex
);

//**********************************************************************
// functions.
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    PaletteCopy
//
//  DESCRIPTION:	
//    Copy a palette.
//
//  Inputs:
//    pSrcPalette
//      Source palette.
//
//    pDestPalette
//      Destination palette.
//
//  Outputs:
//    none:
//
//  GLOBALS:
//    none
//**********************************************************************
void PaletteCopy( ClPalette *pDestPalette, ClPalette *pSrcPalette )
{
  ClAssert( NULL != pDestPalette );
  ClAssert( NULL != pSrcPalette );
  ClAssert( pSrcPalette->nColorCount >= 0 );

  memcpy( pDestPalette->colors, pSrcPalette->colors, PALETTE_BYTE_LEN );
  pDestPalette->nColorCount = pSrcPalette->nColorCount;

}

//**********************************************************************
//  FUNCTION:
//    PaletteClear
//
//  DESCRIPTION:	
//    Fills a palette with rgb=(0,0,0);
//
//  Inputs:
//    pPalette
//      Pointer to a palette to zero fill.
//
//  Outputs:
//    pNumColorsUsed
//      Returns the number of colors used.
//
//  GLOBALS:
//    none
//**********************************************************************
void PaletteClear( ClPalette *pPalette )
{
  ClAssert( NULL != pPalette );

  memset( pPalette->colors, 0, PALETTE_BYTE_LEN );
  pPalette->nColorCount = 0;
}

//**********************************************************************
//  FUNCTION:
//    PaletteMapInit
//
//  DESCRIPTION:	
//    Initializes a palette map.  The each item is set to 0xffff 
//    to signify it is not indexed.
//
//  Inputs:
//    panMap
//      Pointer to an array of palette indexes.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void PaletteMapInit(PaletteMap *pMap)
{
  ClAssert( NULL != pMap );

  memset( pMap, 0, sizeof( PaletteMap ) );
}

//**********************************************************************
//  FUNCTION:
//    PaletteColorIndexGet
//
//  DESCRIPTION:	
//    Retreives a color index from the palette map.
//
//  Inputs:
//    Bitmap *pBitmap
//      The palette that the color is read from.
//      If a color is not indexed then it will be added to the bitmap's
//      palette and the indexed.
//    int *anPaletteMap
//      Palette index.
//    Color *pPalette
//      
//    unsigned char ucPixel
//
//    unsigned char *pucPixel
//
//  Returns:
//    CL_FALSE
//      The palette is full.
//    CL_TRUE
//      The color was added to the palette.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL PaletteColorIndexGet
(
 ClPalette *pPalette,
 PaletteMap *pMap,
 ClColor color,
 unsigned char ucSrcColorInd,
 unsigned char *pucDestColorInd
)
{
  int nColorIndex = 0;
  CL_BOOL bResult = CL_TRUE;
  
  ClAssert( NULL != pPalette );
  ClAssert( NULL != pMap );
  ClAssert( NULL != pucDestColorInd );

  if( IS_COLOR_MAPPED( pMap, ucSrcColorInd ) )
    *pucDestColorInd = pMap->aucIndexes[ucSrcColorInd];
  else
  {
    bResult = PaletteColorAdd( pPalette, color, &nColorIndex );
    if( bResult )
    {
      SET_COLOR_MAPPED_BIT( pMap, ucSrcColorInd );
      pMap->aucIndexes[ucSrcColorInd] = nColorIndex;

      *pucDestColorInd = nColorIndex;
    }
  }

  return bResult;
}

//**********************************************************************
//  FUNCTION:
//    PaletteColorAppend
//
//  DESCRIPTION:	
//    Added a color to the end of a palette.
//
//  Inputs:
//    ClPalette *pPalette
//      The palette.
//    Color color
//      The color that will be added to the bitmap's palette.
//
//  Outputs:
//    int *pnColorIndex
//      Returns the color's index in the bitmap's palatte.
//
//  Returns:
//    CL_FALSE
//      The palette is full.
//    CL_TRUE
//      The color was added to the palette.
//
//  GLOBALS:
//    none
//**********************************************************************
static CL_BOOL PaletteColorAppend
(
  ClPalette *pPalette,
  ClColor color,
  int *pnColorIndex
)
{
  ClColor *pColor = NULL;
  CL_BOOL bResult = CL_FALSE;

  ClAssert( NULL != pPalette );
  ClAssert( pPalette->nColorCount >= 0 && pPalette->nColorCount <= PALETTE_LEN );
  ClAssert( NULL != pnColorIndex );

  if( pPalette->nColorCount < PALETTE_LEN )
  {
    // Add the color to the end of the palette.
    pColor = pPalette->colors + pPalette->nColorCount;

    pColor->ucRed = color.ucRed;
    pColor->ucGreen = color.ucGreen;
    pColor->ucBlue = color.ucBlue;

    *pnColorIndex = pPalette->nColorCount;
    ++pPalette->nColorCount;

    bResult = CL_TRUE;
  }
    
  return bResult;

}

//**********************************************************************
//  FUNCTION:
//    PaletteColorAt
//
//  DESCRIPTION:	
//    Searches for a color in the bitmap's palette and returns the 
//    index if the color is found.
//
//  Inputs:
//    ClPalette *pPalette
//      The palette that will be searched.
//    Color color
//      The color to search for.
//
//  Outputs:
//    int *pnColorIndex
//      Returns the color's index in the bitmap's palatte.
//
//  Returns:
//    CL_TRUE - The color was found in the bitmap's palette.
//    CL_FALSE - The color was not found.  Assume pnColorIndex is 
//            invalid.
//  GLOBALS:
//    none
//**********************************************************************
static CL_BOOL PaletteColorAt
(
  ClPalette *pPalette,
  ClColor color,
  int *pnColorIndex
)
{ 
  CL_BOOL bFound = CL_FALSE;
  int nIndex;
  ClColor *pColor = NULL;

  ClAssert( NULL != pPalette );
  ClAssert( NULL != pnColorIndex );
  ClAssert(
    pPalette->nColorCount >= 0 && 
    pPalette->nColorCount <= PALETTE_LEN
  );

  *pnColorIndex = 0;

  for( nIndex = 0; nIndex < pPalette->nColorCount; ++nIndex )
  {    
    pColor = pPalette->colors + nIndex;

    if
    (
      pColor->ucRed == color.ucRed &&
      pColor->ucGreen == color.ucGreen &&
      pColor->ucBlue == color.ucBlue
    )
    {
      bFound = CL_TRUE;
      *pnColorIndex = nIndex;
      break;
    }

  }
  
  return bFound;

}

//**********************************************************************
//  FUNCTION:
//    PaletteColorAdd
//
//  DESCRIPTION:	
//    Adds a color to the bitmap if it doesn't already exist.
//
//  Inputs:
//    ClPalette *pPalette
//      The palette that color will be added to.
//    Color color
//      The color to add.
//
//  Outputs:
//    int *pnColorIndex
//      Returns the color's index in palatte.
//
//  Returns:
//    CL_FALSE
//      The palette is full.
//    CL_TRUE
//      The color was added to the palette.
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL PaletteColorAdd
(
  ClPalette *pPalette,
  ClColor color,
  int *pnColorIndex
)
{
  CL_BOOL bResult = CL_TRUE;

  if( !PaletteColorAt( pPalette, color, pnColorIndex ) )
    bResult = PaletteColorAppend( pPalette, color, pnColorIndex );

  return bResult;
}

//**********************************************************************
//  FUNCTION:
//    PaletteCopyFromRgbQuad
//
//  DESCRIPTION:	
//    Copies a colors from a RGBQUAD palette to a ClColor palette.
//
//  Inputs:
//    Palette *pPalette
//      Destination palette.
//    _RGBQUAD *pPalette
//      Source palette.
//    int iPaletteLen
//      Number of colors in the rgb quad palette.
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void PaletteCopyFromRgbQuad( ClPalette *pPalette, _RGBQUAD *pRgbQuad, int iPaletteLen )
{
  int i;
  ClColor *pColors = NULL;

  ClAssert( NULL != pPalette );
  ClAssert( NULL != pRgbQuad );

  pColors = pPalette->colors;

  pPalette->nColorCount = CL_MIN( iPaletteLen, PALETTE_LEN );
  for( i = 0; i < pPalette->nColorCount; ++i )
    CL_RGBQUAD_TO_COLOR( pColors[i],  pRgbQuad[i] );

}

//**********************************************************************
//  FUNCTION:
//    PaletteCopyToRgbQuad
//
//  DESCRIPTION:	
//    Copies a colors from a ClColor palette to a RGBQUAD palette.
//
//  Inputs:
//    _RGBQUAD *pPalette
//      Source palette.
//    int iPaletteLen
//      Number of colors in the rgb quad palette.
//    Palette *pPalette
//      Destination palette.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void PaletteCopyToRgbQuad( _RGBQUAD *pRgbQuad, int iPaletteLen, ClPalette *pPalette )
{
  int i;
  ClColor *pColors = pPalette->colors;

  ClAssert( NULL != pRgbQuad );
  ClAssert( NULL != pPalette );
  ClAssert( pPalette->nColorCount <= PALETTE_LEN );

  iPaletteLen = CL_MIN( iPaletteLen, pPalette->nColorCount);
  for( i = 0; i < iPaletteLen; ++i )
    CL_COLOR_TO_RGBQUAD( pRgbQuad[i], pColors[i] );

}
