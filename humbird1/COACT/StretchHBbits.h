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
//    StretchHBbits.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    6/19/2009
//
//  DESCRIPTION:
//    Function used for copying and resizing bitmaps.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef STRETCH_HBBITS_H
#define STRETCH_HBBITS_H


//**********************************************************************
// Function prototypes
//**********************************************************************

int StretchHBBits (HANDLE HBImageHandle,
				   int XDest, int YDest, int nDestWidth, int nDestHeight,
				   int XSrc,  int YSrc,  int nSrcWidth,  int nSrcHeight, 
				   unsigned char *pucBits,
				   int TileHeight, int TileWidth,
				   int TilePaletteLen,
				   _RGBQUAD *TilePalette);

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
);

#endif // STRETCH_HBBITS_H