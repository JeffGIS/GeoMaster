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
//    text.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    05/04/2009
//
//  DESCRIPTION:
//    Functions for drawining text on a bitmap.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef __TEXT_H
#define __TEXT_H

#include "types.h"
#include "Bitmap.h"
#include "fs.h"

//**********************************************************************
// Character image file structures
//**********************************************************************

/*
  A character image file contains a list of character images.  The first 4 
  bytes contains the number of characters.  Next, a character directory lists
  each character that has a image.

  File structure:
    byte offset   field
    0             totalBytes - Total number of bytes used by character directory and 
                  character bitmaps.
    4             Directory Length - The number of entries in the directory.
    8             First directory entry.
    24            second directory entry if there are two or more entries.
    ...
    imageStartByte = 8 + directory entry bytes * number of directory entries
                  First byte of images.
    totalBytes + 4 - 1
                  Last ImageByte
*/

/*
  Character directory entry.
*/
typedef struct CharDirEntryTag
{
  /*
    iC is a 8 bit value.  It is delcare as an int to 
    guarantee 4 byte alignment.
  */
  int iC;
  int iOffset;
  int iWidth;
  int iHeight;
} CharDirEntry;

/*
  Character directory.
*/
typedef struct CharDirTag
{
  int iDirLen;
  CharDirEntry aDirEntries[1];
} CharDir;

/**************************************************
  Macros that access images from a character file.
****************************************************/
#define DIR_BEG_OFFSET (sizeof(int))
/*
  Returns the pointer to the first character image.
*/
#define IMAGES_BEG_PTR( pDir ) \
  ( (unsigned char *)(pDir)->aDirEntries + (pDir)->iDirLen * sizeof( CharDirEntry ) )

#define IMAGES_BEG_OFFSET( pDir ) \
  ( (int)( DIR_BEG_OFFSET + sizeof(CharDir) + ( (pDir)->iDirLen - 1 ) * sizeof( CharDirEntry ) ) )

/*
  Returns the pointer to the byte after the last character image.
*/
#define IMAGES_END_PTR( pDir, uiByteCount ) \
  ( (unsigned char *)(pDir) + (uiByteCount) )

//**********************************************************************
//  Text Data
//**********************************************************************

#define FIRST_ASCII_CHAR 0x20
#define LAST_ASCII_CHAR 0x80
/*
  The number of entries in TextImages::paDirEntries array.
  There is one entry for each printable ascii character and
  one dummy entry.
*/
#define DIR_ENTRY_LEN (LAST_ASCII_CHAR - FIRST_ASCII_CHAR + 1) 

/*
  The number of bytes used by the TextImages::paDirEntries array.
*/
#define DIR_ENTRY_BYTE_LEN ( DIR_ENTRY_LEN * sizeof(CharDirEntry) ) 

typedef struct FontFileTag
{
  unsigned int uiSize;
  char psFileName[CL_MAX_PATH];
} FontFile;

/*
  Data needed to display text.  TextImages instances are initialized by 
  TextImagesInit() and finalized by TextImagesDestroy().
*/
typedef struct TextImagesTag
{
  
  FontFile *paFiles;
  int iFileCount;

  char uiCharSize;
  unsigned char *pucImages;
  /*
    Strores dir entries for the 96 printable ascii character.
    The first element in the array stores character 0x20 and
    the last element stores character 0x7f.  Characters are
    stored in numerical order.

    Use the DIR_ENTRY_GET macro to retrieve a dir entry for a 
    character.

    If paDirEntries[i].iC is 0 there is no image for that character.
    aDirEntries[0] is a dummy entry whose width and height is zero
    and aDirEntries[0].iC is 0.

  */
  CharDirEntry aDirEntries[DIR_ENTRY_LEN];


} TextImages;

/*
  Macros to get a dir entry for an ASCII character.
*/
#define DIR_ENTRY_GET( paDirEntries, c ) \
  ( ( (c) < FIRST_ASCII_CHAR || (c) >= LAST_ASCII_CHAR ) ? (paDirEntries) : ( (paDirEntries) + ( c - FIRST_ASCII_CHAR + 1 ) ) )
#define CHAR_EXISTS( pDirEntry ) \
  ( ( pDirEntry->iC ) ? CL_TRUE : CL_FALSE )

//**********************************************************************
// Function prototypes.
//**********************************************************************
CL_BOOL TextImagesAreValid(TextImages *pImages );
ClError TextImagesInit(TextImages *pImages, char *psCharFilePath, unsigned int uiCharSize );
void TextImagesDestroy( TextImages *pData );
ClError TextChangeFontSize(TextImages *pImages, char *psCharFilePath, unsigned int uiCharSize );

void TextRender
(
  TextImages *pImages,
  Bitmap *pDestBitmap,
  char *pucText, 
  int iTextLen,
  CL_BOOL bIsCenterPoint,
  int iCenterX, 
  int iCenterY,
  int iRotation,
  int iSize,
  int iTextColor,
  int iShadowColor
);


#endif // __TEXT_H