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
//    text.c
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
#include <math.h>
#include <string.h>

#include "Chart.h"
#include "Bitmap.h"
#include "ClMalloc.h"
#include "ClAssert.h"
#include "FixedPoint.h"
#include "text.h"
#include "fs.h"
#include "ByteSwap.h"

#if defined(HUMMINBIRD)
#include "fs_api.h"
#endif

//**********************************************************************
// Constants
//**********************************************************************
#define FONT_INDEX_FILE "fonts.txt"

//**********************************************************************
// Macros
//**********************************************************************
/*
  Returns non-zero if a chart bitmap's pixel color index is zero 
  ( i.e. transparent). 
*/
#define IS_TRANSPARENT( ucPixel ) ( 0 != (ucPixel) )

//**********************************************************************
// Types
//**********************************************************************
typedef struct CharFileTag
{
  unsigned int uiSize;
  char psFileName[CL_MAX_PATH];
} CharFile;


//**********************************************************************
// Function prototypes.
//**********************************************************************
static ClError TextLoadFontFileList(char *psFolderPath, FontFile **ppaFiles, int *piFileCount );
static ClError TextReadCharFile
(
  char *psFilePath,
  CharDir **ppDir,
  unsigned *puiTotalBytes
);
static void TextByteSwapDir( CharDir *pDir );
static void TextCopyDir( TextImages *pImages, CharDir *pDir );

static void TextCalcExtent
(
 TextImages *pImages,
 char *pacText,
 int iTextLen, 
 int *piTextWidth,
 int *piTextHeight
);

static void DrawShadows
(
  TextImages *pImages,
  int iTextCenterX, 
  int iTextCenterY,
  Bitmap *pDestBitmap,
  int iTextX,
  int iTextY,
  int iDegrees,
  char *pacText,
  int iTextLen,
  int iShadowColor
);

static void DrawText
(
 TextImages *pImages,
 int iTextCenterX, 
 int iTextCenterY,
 Bitmap *pDestBitmap,
 int iDestX,
 int iDestY,
 int iDegrees,
 char *pacText,
 int iTextLen,
 int iTextColorIndex
);

static void TextCalcLineHeight
(
 TextImages *pImages,
 char *pCharStart,
 char *pCharEnd,
 int *pCharHeight
);

static void TextImageCopy
(
  unsigned char *pucCharBm,
  int iCharX,
  int iCharY,
  int iCharWidth,
  int iCharHeight,
  unsigned char *pucDestBm,
  int iDestX,
  int iDestY,   
  int iDestWidth,
  int iDestHeight,
  int iForegroundColorIndex
);

static void TextImageRotateCopy
(
  unsigned char *pucCharBm,
  int iCharX,
  int iCharY,
  int iCharWidth,
  int iCharHeight,
  int iTextCenterX,
  int iTextCenterY,
  int iDegrees,
  unsigned char *pucDestBm,
  int iDestX,
  int iDestY,
  int iDestWidth,
  int iDestHeight,
  int iForegroundColorIndex
);

static void cl_freadCharDirEntry(CharDirEntry *pCharDirEntry,CLFILE *pFID);

//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    TextImagesIsValid
//
//  Description:	
//    Check to see if text images have been initialized.  TextImagesInit() must be called
//    before this function or the TextImages argument must be zeroe filled.
//
//  Inputs:
//    TextImages *pImages
//      Data for accessing text images.
//
//  Outputs:
//    None.
//  
//  Returns:
//    CL_FALSE
//      text images not initialized.
//    CL_TRUE
//      Success
//
//  GLOBALS:
//    none
//
//**********************************************************************
CL_BOOL TextImagesAreValid(TextImages *pImages )
{
  if( NULL == pImages || NULL == pImages->pucImages )
    return CL_FALSE;
  else
    return CL_TRUE;
}

//**********************************************************************
//  FUNCTION:
//    TextImagesInit
//
//  Description:	
//    Initializes a TextImages instance.
//
//  Inputs:
//    TextImages *pImages
//      Data for accessing text images.
//    char *psCharFilePath
//      Full path to the directory that contains the file that stores 
//      character images.
//    unsigned int uiCharSize
//      The size of the characters.
//
//  Outputs:
//    None.
//  
//  Returns:
//    CL_FALSE
//      Failure - Memory allocation or File I/O error.
//        Elements in pImages are not guaranteed to be invalid.
//        Memory is not dereferenced by pointers in pImages and the pointers
//        are NULL.
//
//    CL_TRUE
//      Success
//
//  GLOBALS:
//    none
//
//**********************************************************************
ClError TextImagesInit(TextImages *pImages, char *psCharFilePath, unsigned int uiCharSize )
{
  ClError error = clError_Success;

  memset( pImages, 0, sizeof( TextImages ) );
  
  error = TextLoadFontFileList( psCharFilePath, &pImages->paFiles, &pImages->iFileCount );
  if( clError_Success == error )
    error = TextChangeFontSize( pImages, psCharFilePath, uiCharSize );

  if( clError_Success != error )
    TextImagesDestroy( pImages );

  return error;
}

//**********************************************************************
//  FUNCTION:
//    TextImagesDestroy
//
//  Description:	
//    Frees resources used by a TextImages instance.
//
//  Inputs:
//    TextImages *pImages
//      Frees memory that is allocated for the character images and 
//      the image directory. 
//
//  Outputs:
//    None.
//  
//  Returns:
//    None
//
//  GLOBALS:
//    None
//
//**********************************************************************
void TextImagesDestroy( TextImages *pImages )
{
  if( NULL != pImages->pucImages )
    cl_free( pImages->pucImages );
  pImages->pucImages = NULL;
  
  if( NULL != pImages->paFiles )
    cl_free( pImages->paFiles );
  pImages->paFiles = NULL;

}

static ClError TextLoadFontFileList(char *psFolderPath, FontFile **ppaFiles, int *piFileCount )
{
  int iFileCount;  
  size_t size;
  char sFileName[ CL_MAX_PATH ];
  int i;
  CLFILE *file = NULL;
  char *sLineFormat = "%ld ,%s\n";
  int fieldsPerLine = 2;
  char sFilePath[CL_MAX_PATH] = {'\0'};
  FontFile *paFiles = NULL;
  int iResult = 0;
  ClError error = clError_Success;

  ClAssert( psFolderPath[strlen(psFolderPath) - 1] == '\\' );

  do { // Use exception pattern for error handling.

    *piFileCount = 0;
    *ppaFiles = NULL;

    // Build the font list file path.
    strncpy( sFilePath, psFolderPath, CL_MAX_PATH);
    sFilePath[ CL_MAX_PATH - 1 ] = '\0';
    strncat( sFilePath, FONT_INDEX_FILE, CL_MAX_PATH - 1 - strlen(sFilePath));

    file = cl_fopen( sFilePath, "r" );
    if( NULL == file )
    {
      error = clError_FontFileIo;
      break;
    }

    // Count the number of fonts in the font list file.
/*  iFileCount = 0;
    iResult = 0;
    while( EOF != iResult )
    {
      iResult = fscanf( file, sLineFormat, &size, &sFileName );
      if( iResult == fieldsPerLine )
        ++iFileCount;    
      else if( EOF != iResult )
      {
        error = clError_FontFileIo;
        break;
      }
    } */
    iFileCount = 3;

    if( clError_Success != error || 0 == iFileCount )
      break;

    // Alloc memory for the font list.
    paFiles = cl_malloc( sizeof( FontFile ) * iFileCount );
    if( NULL == paFiles )
    {
      error = clError_TextMemAlloc;
      break;
    }
    memset( paFiles, 0,  sizeof( FontFile ) * iFileCount );

    if( cl_fseek( file, 0, SEEK_SET ) )
    {
      error = clError_FontFileIo;
      break;
    }

    // Read the font files into paFiles.
    for( i =0; i < iFileCount; ++i )
    {
/*    if( fieldsPerLine != fscanf( file, sLineFormat, &size, &sFileName ) )
      {
        error = clError_FontFileIo;
        break;
      } */
       if (i == 0) {
          size = 14;
          strcpy(sFileName,"9pt.fnt");
       }
       else if (i == 1) {
          size = 16;
          strcpy(sFileName,"9_75pt.fnt");
       }
       else if (i == 2) {
          size = 18;
          strcpy(sFileName,"11pt.fnt");
       }
      paFiles[i].uiSize = size;
      strncpy( paFiles[i].psFileName, sFileName, CL_MAX_PATH );
      paFiles[i].psFileName[CL_MAX_PATH - 1] = '\0';
    }

  } while(0);

  if( NULL != file )
    cl_fclose(file);

  if( clError_Success == error )
  {
    *piFileCount = iFileCount;
    *ppaFiles = paFiles;
  }
  else if( NULL != paFiles )
    cl_free(paFiles);

  return error;
}

//**********************************************************************
//  FUNCTION:
//    TextChangeFontSize
//
//  Description:	
//    Loads a character image file for the new font size.
//
//  Inputs:
//    TextImages *pImages
//      Data for accessing text images.
//    char *psCharFilePath
//      Full path to the directory that contains the file that stores 
//      character images.
//    unsigned int uiCharSize
//      The size of the characters.
//
//  Outputs:
//    None.
//  
//  Returns:
//    CL_FALSE
//      Failure - Memory allocation or File I/O error.
//        Elements in pImages are not guaranteed to be invalid.
//        Memory is not dereferenced by pointers in pImages and the pointers
//        are NULL.
//
//    CL_TRUE
//      Success
//
//  GLOBALS:
//    none
//
//**********************************************************************
ClError TextChangeFontSize(TextImages *pImages, char *psCharFilePath, unsigned int uiCharSize )
{
  unsigned uiTotalBytes;
  CharDir *pDir = NULL;
  unsigned char *pucImagesBeg = NULL;
  unsigned char *pucImagesEnd = NULL;
  char sPath[CL_MAX_PATH] = "";
  int iCharFilePathLen;
  int charFileIndex;
  ClError error = clError_Success;

  ClAssert( NULL != pImages );
  ClAssert( NULL != psCharFilePath );
  ClAssert( strlen( psCharFilePath ) < CL_MAX_PATH );

  do { // Use exception style error handling

    if( uiCharSize == pImages->uiCharSize )
    {
      // Don't need to change fonts.
      break;
    }    

    // Free up old memory.
    if( NULL != pImages->pucImages )
      cl_free( pImages->pucImages );
    pImages->pucImages = NULL;

    /*  
      Search for a character image file for the current character size.
    */
    for( charFileIndex = 0; charFileIndex < pImages->iFileCount; ++charFileIndex )
    {
      if( uiCharSize == pImages->paFiles[charFileIndex].uiSize )
      {
        iCharFilePathLen = strlen(psCharFilePath);
        strncpy( sPath, psCharFilePath, CL_MAX_PATH );
        sPath[ CL_MAX_PATH - 1 ] = '\0';
        strncat( sPath + iCharFilePathLen, pImages->paFiles[charFileIndex].psFileName, CL_MAX_PATH - 1 - iCharFilePathLen );

        break;
      }
    }

    if( '\0' == sPath[0] )
    {
      error = clError_FontFileMissing;
      break;
    }

    /*
      Read the characters images from a character image file.
    */
    error = TextReadCharFile( sPath, &pDir, &uiTotalBytes );
    if( error != clError_Success )
      break;
    
    TextCopyDir( pImages, pDir );

    /*
      Allocate memory for the character images and copy the images to pImages.
    */
    pucImagesBeg = IMAGES_BEG_PTR( pDir );
    pucImagesEnd = IMAGES_END_PTR( pDir, uiTotalBytes );

    pImages->pucImages = (unsigned char *)cl_malloc( pucImagesEnd - pucImagesBeg );
    if( NULL == pImages->pucImages )
    {
      error = clError_TextMemAlloc;
      break;
    }

    memmove( pImages->pucImages, pucImagesBeg, pucImagesEnd - pucImagesBeg );

  } while(0);

  if( clError_Success == error )
    pImages->uiCharSize = uiCharSize;

  if( NULL != pDir )
    cl_free( pDir );

  return error;
}
//**********************************************************************
//  FUNCTION:
//    TextReadCharFile
//
//  Description:	
//    Read a file that contains character images.  The output is a
//    buffer that contains the image directory and the images.
//
//  Inputs:
//    char *psFilePath
//      The full path to the file that contains images.
//
//  Outputs:
//    CharDir **ppDir  
//      Contains a pointer to the image directory.  The images follow
//      the image directory in memory.  The IMAGES_BEG_PTR and IMAGES_END_PTR
//      macros can be used to get pointers to the begin and end of the images.
//
//    unsigned *puiTotalBytes
//      The total number of bytes used for the directory and the images.
//
//  Returns:
//    CL_FALSE
//      Failure - Memory allocation or File I/O error.
//        Elements in pImages are not guaranteed to be invalid.
//        Memory is not dereferenced by pointers in pImages and the pointers
//        are NULL.
//
//    CL_TRUE
//      Success
//
//  GLOBALS:
//    none
//
//**********************************************************************
static ClError TextReadCharFile
(
  char *psFilePath,
  CharDir **ppDir,
  unsigned *puiTotalBytes
)
{
  CLFILE *charFile = NULL;
  ClError error = clError_Success;
  int iDirIndex = 0;
  size_t bytesToRead;
  size_t bytesRead;
  unsigned char *pucImagesBeg;
  unsigned char *pucImagesEnd;

  ClAssert( NULL != psFilePath );
  ClAssert( NULL != ppDir );
  ClAssert( NULL != puiTotalBytes );

  do {

    *ppDir = NULL;

    charFile = cl_fopen( psFilePath, "rb" );
    if( NULL == charFile )
    {  
      error = clError_FontFileIo;
      break;
    }

    // Read the number of total image directory and bitmap bytes.
    *puiTotalBytes = cl_freadS32( charFile );

    // Allocate memory for the image directory and the images.
    *ppDir = (CharDir *)cl_malloc( *puiTotalBytes );
    if( NULL == *ppDir )
    {
      error = clError_TextMemAlloc;
      break;
    }

    // Read the image directory.
    (*ppDir)->iDirLen = cl_freadS32( charFile );
    for( iDirIndex = 0; iDirIndex < (*ppDir)->iDirLen; ++iDirIndex )
      cl_freadCharDirEntry( (*ppDir)->aDirEntries + iDirIndex, charFile );

    // Read the character images.
    pucImagesBeg = IMAGES_BEG_PTR( *ppDir );
    pucImagesEnd = IMAGES_END_PTR( *ppDir, *puiTotalBytes );
    bytesToRead = pucImagesEnd - pucImagesBeg;
    bytesRead = cl_fread( pucImagesBeg, 1, bytesToRead, charFile );
    if( bytesToRead != bytesRead )
    {
      error = clError_FontFileIo;
      break;
    }

  } while( 0 );

  if( NULL != charFile )
    cl_fclose( charFile );

  if( error != clError_Success )
  { 
    if( *ppDir != NULL )
    {
      cl_free( *ppDir );
      *ppDir = NULL;
    }

    *puiTotalBytes = 0;
  } // if( error != clError_Success )

  return error;

}

//**********************************************************************
//  FUNCTION:
//    TextCopyDir
//
//  Description:	
//    Copies a directory that was read by TextReadCharFile().
//  
//
//  Inputs:
//    TextImages *pImages
//      pImages->pDirEntries is the destination directory.
//
//    CharDir *pDir
//      Source directory read by TextReadCharFile().
//
//  Outputs:
//    None.
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
static void TextCopyDir( TextImages *pImages, CharDir *pDir )
{

  CharDirEntry *pSrcEntry = NULL;
  CharDirEntry *pDestEntry = NULL;
  int i;

  ClAssert( NULL != pImages );
  ClAssert( NULL != pDir );
  ClAssert( 0 != pDir->iDirLen );

  /*
    Initialize pImages->pDirEntries.  CharDirEntry::iC field
    in every entry must be set to 0.  iC = 0 indicates that
    there is not bitmap for the character.
  */
  memset( pImages->aDirEntries, 0, DIR_ENTRY_BYTE_LEN );
  
  /*
    Copy directory entries from pDir to pImages.
  */
  for( i = 0; i < pDir->iDirLen; ++i )
  {
    pSrcEntry = pDir->aDirEntries + i;
    
    // Expect printable ascii characters.
    ClAssert( FIRST_ASCII_CHAR <= pSrcEntry->iC && pSrcEntry->iC < LAST_ASCII_CHAR );

    pDestEntry = DIR_ENTRY_GET( pImages->aDirEntries, pSrcEntry->iC );

    pDestEntry->iC      =   pSrcEntry->iC;
    pDestEntry->iOffset =   pSrcEntry->iOffset;
    pDestEntry->iOffset -=  IMAGES_BEG_OFFSET(pDir);
    pDestEntry->iWidth  =   pSrcEntry->iWidth;
    pDestEntry->iHeight =   pSrcEntry->iHeight;

  }

}

//**********************************************************************
//  FUNCTION:
//    TextRender
//
//  Description:	
//    Renders a string of characters on a chart.
//  
//  Inputs:
//    TextImages *pImages
//      Contains the text character images.
//    Bitmap *pDestBitmap
//      The bitmap that the characters are rendered on.
//    char *pucText
//      The string that will be rendered.  Might not be null terminated.
//    int iTextLen
//      The number of characters in the string.
//    CL_BOOL bIsCenterPoint
//      CL_TRUE - (BitmapX, BitmapY) is location of the center point of the text.
//      CL_FALSE - BitmapX is the location of the left edge of the text.
//                 The text's vertical mid-point is centered on BitmapY.
//    int iCenterX
//      The string's x coordinate on the destination bitmap, in pixels, 
//      of the center point.
//    int iCenterY
//      The string's y coordinate on the destination bitmap, in pixels, 
//      of the center point.
//    int iDegrees
//      iDegrees > 0: Rotate counter clockwise.
//      iDegrees < 0: Rotate clockwise.
//    int iSize
//      The required size of the characters in points.
//    int iTextColor
//      The rgb value of the text foreground color.  See 
//      CL_COLORREF_TO_COLOR macro for the format of the text color. 
//    int iShadowColor
//      The rgb value of the shadow color.  See CL_COLORREF_TO_COLOR macro
//      for the format if the shadow color.  A show color of zero means
//      that a shadow will not be rendered.
//
//  Outputs:
//    None.
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
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
)
{

  int iTextX;
  int iTextY;
  int iTextWidth;
  int iTextHeight;
  ClColor textColor = {0,0,0};
  int iTextColorIndex;  

  ClAssert( NULL != pImages );
  ClAssert( NULL != pDestBitmap );
  ClAssert( NULL != pucText);

  /*
    Add the text foreground color to the chart's palette in case it is not 
    already in there.  The color will not be added if it already exists.
  */
  CL_COLORREF_TO_COLOR( textColor, iTextColor );
  PaletteColorAdd( &pDestBitmap->palette, textColor, &iTextColorIndex );

  /*
    Calculate the width and height of the text.
  */
  TextCalcExtent
  (
    pImages,
    pucText,
    iTextLen,
    &iTextWidth,
    &iTextHeight
  );

  /*********************
    Calculate the coordinate of the upper left corner of the text area.
  **********************/
  /*
    The origin of the chart's bitmap is in the lower left corner so the
    text's y coordinate needs to be translated.
  */
  iCenterY = ( pDestBitmap->nHeight - iCenterY );

  if( bIsCenterPoint )
  {
    // Move iTextX to the left edge of the text area.
    iTextX = iCenterX - (iTextWidth >> 1);
  }
  else
  {
    /*
      iTextX is the location of the left edge of the text.
      The text's vertical mid-point is centered on iTextY.      
    */
    iTextX = iCenterX;
  }
  // Move Y to the upper left corner of the first character in the text area.
  iTextY = iCenterY + (iTextHeight >> 1);

  
  /*********************
    Draw the text
  **********************/
  DrawShadows( pImages, iCenterX, iCenterY, pDestBitmap, iTextX, iTextY, iRotation, pucText, iTextLen, iShadowColor );
  DrawText( pImages, iCenterX, iCenterY, pDestBitmap, iTextX, iTextY, iRotation, pucText, iTextLen, iTextColorIndex );

}

//**********************************************************************
//  FUNCTION:
//    CalcTextExtent
//
//  Description:	
//   Calculates the width and height of a rendered text string.
//
//  Inputs:
//    TextImages *pImages
//      Contains the image for each character.
//    char *pacText
//      Text string.  Might not be null terminated.
//    int iTextLen
//      The number of characters in the text string.
//  Outputs:
//    int *piTextWidth
//      The width of the text string when it is rendered.
//    int *piTextHeight
//      The height of the text string when it is rendered.
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
static void TextCalcExtent
(
 TextImages *pImages,
 char *pacText,
 int iTextLen, 
 int *piTextWidth,
 int *piTextHeight
)
{
  int i;
  int iLineWidth = 0;
  int iLineHeight = 0; // The height of the current line.
  CharDirEntry *pEntry = NULL;
  
  *piTextWidth = 0;
  *piTextHeight = 0;

  for( i = 0; i < iTextLen; ++i )
  {

    if( '\n' == pacText[i] )
    {
      /*
        Accumulate the height of the current line.
      */
      *piTextWidth = CL_MAX( iLineWidth, *piTextWidth );
      *piTextHeight += iLineHeight;
      // Start a new line.
      iLineWidth = 0;
      iLineHeight = 0;

    }
    else
    {
      // Get the characters dimenions.
      pEntry = DIR_ENTRY_GET
      (
        pImages->aDirEntries,
        pacText[i]
      );

      if( CHAR_EXISTS(pEntry) )
      {
        // Expand the height and width to contain the character.
        iLineWidth += pEntry->iWidth;
        iLineHeight = CL_MAX( iLineHeight, pEntry->iHeight );      
      }
    }
  }

  /*
    Add the last line.
  */
  *piTextWidth = CL_MAX( iLineWidth, *piTextWidth );
  *piTextHeight += iLineHeight;

}

//**********************************************************************
//  FUNCTION:
//    DrawShadows
//
//  Description:	
//    Draws a shadow around each text character.  The shadow outlines
//    every line in the character.
//
//  Inputs:
//    TextImages *pImages
//      Contains the image for each character.
//    int iTextCenterX
//      The x coordinate of the center point of the entire string.
//    int iTextCenterY
//      The y coordinate of the center point of the entire string.
//    int iDegrees
//      Measured in degrees.
//      Rotation > 0: Rotate counter clockwise.
//      Rotation < 0: Rotate clockwise.
//    Bitmap *pDestBitmap
//      The bitmap that the characters are rendered on.
//    int iTextX
//    int iTextY
//      The lower left corner of the text string.
//    int iDegrees
//      iDegrees > 0: Rotate counter clockwise.
//      iDegrees < 0: Rotate clockwise.
//    char *pacText
//      Text string.  Might not be null terminated.
//    int iTextLen
//      The number of characters in the text string.
//    int iShadowColor
//      The rgb value of the shadow color.  See CL_COLORREF_TO_COLOR macro
//      for the format if the shadow color.  A show color of zero means
//      that a shadow will not be rendered.
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
static void DrawShadows
(
  TextImages *pImages,
  int iTextCenterX, 
  int iTextCenterY,
  Bitmap *pDestBitmap,
  int iTextX,
  int iTextY,
  int iDegrees,
  char *pacText,
  int iTextLen,
  int iShadowColor
)
{
  CharDirEntry *pDirEntry = NULL;

  ClColor shadow;
  int iShadow = 0;

  ClAssert( NULL != pImages );
  ClAssert( NULL != pDestBitmap );
  ClAssert( NULL != pacText );
  ClAssert( iTextLen >= 0 );

  if( iShadowColor )
  {
    /*
      Add the shadow color to the palette.
      If the color is already in the palette it will not be added.
    */
    CL_COLORREF_TO_COLOR(shadow, iShadowColor);
    PaletteColorAdd( &pDestBitmap->palette, shadow, &iShadow );

    // Shift the text to the right.
    DrawText( pImages, iTextCenterX, iTextCenterY, pDestBitmap, iTextX + 1, iTextY, iDegrees, pacText, iTextLen, iShadow );
    // Shift the text to the left.
    DrawText( pImages, iTextCenterX, iTextCenterY, pDestBitmap, iTextX - 1, iTextY, iDegrees, pacText, iTextLen, iShadow );
    // Shift the text to up.
    DrawText( pImages, iTextCenterX, iTextCenterY, pDestBitmap, iTextX, iTextY + 1, iDegrees, pacText, iTextLen, iShadow );
    // Shift the text to down.
    DrawText( pImages, iTextCenterX, iTextCenterY, pDestBitmap, iTextX, iTextY - 1, iDegrees, pacText, iTextLen, iShadow );

  }

}

//**********************************************************************
//  FUNCTION:
//    DrawShadows
//
//  Description:	
//    Renders a text string on a bitmap.
//
//  Inputs:
//    TextImages *pImages
//      Contains the image for each character.
//    int iTextCenterX
//      The x coordinate of the center point of the entire string.
//    int iTextCenterY
//      The y coordinate of the center point of the entire string.
//    int iDegrees
//      Measured in degrees.
//      Rotation > 0: Rotate counter clockwise.
//      Rotation < 0: Rotate clockwise.
//    Bitmap *pDestBitmap
//      The bitmap that the characters are rendered on.
//    int iDestX
//    int iDestY
//      The lower left corner of the text string in the destination
//      bitmap.
//    int iDegrees
//      iDegrees > 0: Rotate counter clockwise.
//      iDegrees < 0: Rotate clockwise.
//    char *pacText
//      Text string.  Might not be null terminated.
//    int iTextLen
//      The number of characters in the text string.
//    int iForegroundColorIndex
//      The index of the foreground color in the destination
//      bitmaps palette.
//
//  Outputs:
//    None
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
static void DrawText
(
 TextImages *pImages,
 int iTextCenterX, 
 int iTextCenterY,
 Bitmap *pDestBitmap,
 int iDestX,
 int iDestY,
 int iDegrees,
 char *pacText,
 int iTextLen,
 int iTextColorIndex
)
{
  char *pChar = NULL;
  char *pCharEnd = NULL;
  int iCharX = 0;
  int iLineHeight = 0; // The height of the current line.
  int iDestXStart = iDestX;
  CharDirEntry *pDirEntry = NULL;
  unsigned char *pucCharBm = NULL;

  ClAssert( NULL != pImages );
  ClAssert( NULL != pDestBitmap );
  ClAssert( NULL != pacText );
  
  /*
    Render each character.
  */
  pCharEnd = pacText + iTextLen;
  for( pChar = pacText; pChar < pCharEnd; ++pChar )
  {

    /*
      First pass through the line,
      Measure the text height.
    */
    iLineHeight = 0;
    TextCalcLineHeight
    (
      pImages,
      pChar,
      pCharEnd,
      &iLineHeight
    );

    /*
      (iDestX, iDestY ) is at the lower left corner of the first character.
    */
    iDestX = iDestXStart;
    iDestY -= iLineHeight;


    /*
      Second pass through the line.
      Draw the text.
    */
    for( ;pChar < pCharEnd && *pChar != '\n'; ++pChar )
    {
      /*
        Get the characters dimensions and image.
      */
      pDirEntry = DIR_ENTRY_GET( pImages->aDirEntries, *pChar );
      if( !CHAR_EXISTS(pDirEntry) )
        continue;

      pucCharBm = pImages->pucImages + pDirEntry->iOffset;

      /*
        Render the character.
      */
#if ENABLE_TEXT_ROTATION
      if( iDegrees )
        TextImageRotateCopy
        (
          pucCharBm,
          iCharX,
          0,
          pDirEntry->iWidth,
          pDirEntry->iHeight,
          iTextCenterX,
          iTextCenterY,
          iDegrees,
          pDestBitmap->pucData,
          iDestX,
          iDestY,
          pDestBitmap->nWidth,
          pDestBitmap->nHeight,
          iTextColorIndex
       );
      else
#endif
        TextImageCopy
        (
          pucCharBm,
          iCharX,
          0,
          pDirEntry->iWidth,
          pDirEntry->iHeight,
          pDestBitmap->pucData,
          iDestX,
          iDestY,
          pDestBitmap->nWidth,
          pDestBitmap->nHeight,
          iTextColorIndex
        );

      /*
        Draw the next character to the right of the previous character.
      */
      iDestX += pDirEntry->iWidth;
    }

  }

}

//**********************************************************************
//  FUNCTION:
//    TextCalcLineHeight
//
//  Description:	
//    Measures the height of a line of text.
//
//  Inputs:
//    TextImages *pImages
//      Contains the image for each character.
//    char *pCharStart
//      Pointer to the first character in a string.
//    char *pCharEnd
//      Pointer to the character after the last text character.
//      
//  Outputs:
//    int *pCharHeight
//      The height of the line.
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
static void TextCalcLineHeight
(
 TextImages *pImages,
 char *pCharStart,
 char *pCharEnd,
 int *pCharHeight
)
{
  char *pChar = NULL;
  int iLineHeight = 0; // The height of the current line.
  CharDirEntry *pDirEntry = NULL;
  unsigned char *pucCharBm = NULL;

  ClAssert( NULL != pCharStart );
  ClAssert( pCharStart < pCharEnd );
  ClAssert( NULL != pCharHeight );

  *pCharHeight = 0;

  /*
    Render each character.
  */
  for( pChar = pCharStart; pChar < pCharEnd && *pChar != '\n'; ++pChar )
  {
    /*
      Compute the height of the current line.
    */    
    pDirEntry = DIR_ENTRY_GET( pImages->aDirEntries, *pChar );
    if( CHAR_EXISTS(pDirEntry) )      
      *pCharHeight = CL_MAX(*pCharHeight, pDirEntry->iHeight);
  }

}
//**********************************************************************
//  FUNCTION:
//    TextImageCopy
//
//  DESCRIPTION:	
//    Copies a character to a bitmap.  The character may be rotated.
//
//  Inputs:
//    unsigned char *pucCharBm
//      The character's bitmap.
//    int iCharX
//    int iCharY
//      The lower left corner of the character in the source bitmap.
//    int iCharWidth
//    int iCharHeight
//      The size of the character.
//    unsigned char *pucDestBm
//      Destination bitmap.
//    int iDestX
//    int iDestY
//      The lower left corner of the text string in the destination bitmap.
//    int iDestWidth
//    int iDestHeight
//      The size of the destination.
//    int iTextColorIndex
//      The index of the foreground color in the destination
//      bitmaps palette.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
static void TextImageCopy
(
  unsigned char *pucCharBm,
  int iCharX,
  int iCharY,
  int iCharWidth,
  int iCharHeight,
  unsigned char *pucDestBm,
  int iDestX,
  int iDestY,
  int iDestWidth,
  int iDestHeight,
  int iTextColorIndex
)
{  
  int iXIndex;
  int iWidth;
  int iYIndex;
  int iHeight;
	int iCharOffset = 0;

  unsigned char *pucSrcPixels = NULL;
  unsigned char *pucDestPixels = NULL;
	
  ClAssert( NULL          != pucCharBm );
  ClAssert( iCharWidth    >= 0 );
  ClAssert( iCharHeight   >= 0 );
  ClAssert( NULL          != pucDestBm );
  ClAssert( iDestWidth    >= 0 );
  ClAssert( iDestHeight   >= 0 );

  /*
    Clip any text that extents beyond the left or right of the bitmap.
    Use iWidth to store the character width.  iWidth will be modified
    if the text needs to be clipped.  iChartWidth cannot be changed
    because it is used to calculate row pointers to the characters
    bitmap.
  */
  iWidth = iCharWidth;
  if( iDestX < 0 )
  {
    iCharX -= iDestX;
    iWidth += iDestX;
    iDestX = 0;
  }
  if( iDestX + iCharWidth > iDestWidth )
    iWidth = iDestWidth - iDestX;


  /*
    Clip any text that extents beyond the top or bootom of the bitmap.
    Use iHeight to store the character width.  iHeight will be modified
    if the text needs to be clipped.  iChartHeight cannot be changed
    because it is used to calculate row pointers to the characters
    bitmap.
  */
  iHeight = iCharHeight;
  if( iDestY < 0 )
  {
    iCharY -= iDestY;
    iHeight += iDestY;
    iDestY = 0;
  }
  if( iDestY + iHeight > iDestHeight )
    iHeight = iDestHeight - iDestY;

  if
  ( 
    iCharX >= 0 && iCharX < iCharWidth && 
    iCharY >= 0 && iCharY < iCharHeight
  )
  {
    /*
      Copy the text.
    */
    pucSrcPixels = pucCharBm + iCharY * iCharWidth + iCharX;
    pucDestPixels = pucDestBm + iDestY * iDestWidth + iDestX;
	  for ( iYIndex = 0; iYIndex < iHeight; iYIndex++)
	  {
		  for ( iXIndex = 0; iXIndex < iWidth; iXIndex++)
		  {
        /*
          Don't copy transparent pixels.
        */
        if( 0 == pucSrcPixels[iXIndex] )
          pucDestPixels[iXIndex] = iTextColorIndex;
		  }
      /*
        Go to the next row.
      */
      pucSrcPixels += iCharWidth;
      pucDestPixels += iDestWidth;

    }
  }

}

//**********************************************************************
//  FUNCTION:
//    TextImageCopy
//
//  DESCRIPTION:	
//    Copies a character to a bitmap.  The character may be rotated.
//
//  Inputs:
//    unsigned char *pucCharBm
//      The character's bitmap.
//    int iCharX
//    int iCharY
//      The lower left corner of the character in the destination bitmap.
//    int iCharWidth
//    int iCharHeight
//      The size of the character.
//    int iTextCenterX
//      The x coordinate of the center point of the entire string.
//    int iTextCenterY
//      The y coordinate of the center point of the entire string.
//    fixed fxDegrees
//      Measured in degrees.
//      Fixed point value in 24.8 format.
//      Rotation > 0: Rotate counter clockwise.
//      Rotation < 0: Rotate clockwise.
//    unsigned char *pucDestBm
//      Destination bitmap.
//    int iDestX
//    int iDestY
//      The lower left corner of the text string in the destination bitmap.
//    int iDestWidth
//    int iDestHeight
//      The size of the destination.
//    int iForegroundColorIndex
//      The index of the foreground color in the destination
//      bitmaps palette.
//
//  Outputs:
//    LPBYTE pDestBitmap
//      The memory buffer that the rotated bitmap will be copied to.
//      The buffer must be the same byte size as the image data in the 
//      bitmap.
//
//  GLOBALS:
//    none
//**********************************************************************
static void TextImageRotateCopy
(
  unsigned char *pucCharBm,
  int iCharX,
  int iCharY,
  int iCharWidth,
  int iCharHeight,
  int iRotationAxisX,
  int iRotationAxisY,
  int iDegrees,
  unsigned char *pucDestBm,
  int iDestX,
  int iDestY,
  int iDestWidth,
  int iDestHeight,
  int iForegroundColorIndex
)
{  
  fixed iRadians;
	fixed fxCos;
	fixed fxSin;
  int iSrcX;
  int iSrcXMax;
  int iSrcY;
  int iSrcYMax;
  fixed fxRotationAxisX;
  fixed fxRotationAxisY;
  fixed fxFracX;
  fixed fxFracY;
  CL_BOOL bClipX;
  CL_BOOL bClipY;

  int iXIndex;
  int iYIndex;
	int iCharOffset = 0;
	
  ClAssert( NULL          != pucCharBm );
  ClAssert( iCharWidth    >= 0 );
  ClAssert( iCharHeight   >= 0 );
  ClAssert( NULL          != pucDestBm );
  ClAssert( iDestWidth    >= 0 );
  ClAssert( iDestHeight   >= 0 );


  iSrcX = iDestX - iRotationAxisX;
  iSrcXMax = iSrcX + iCharWidth;
  iSrcY = iDestY - iRotationAxisY;
  iSrcYMax = iSrcY + iCharHeight;

  fxRotationAxisX = itofx( iRotationAxisX );
  fxRotationAxisY = itofx( iRotationAxisY );

	// Calculate angle in radians.
  iRadians = Mulfx( itofx(iDegrees) / 180, FX_PI);

	fxCos = FxCos(iRadians);
	fxSin = FxSin(iRadians);

  /*
    Rotate and copy the bitmap.

    For each pixel in the source, compute its source bitmap location after
    rotation.  Translate that location to the destination bitmap location.
    Finally copy the pixel.
  */
	for ( iYIndex = iSrcY; iYIndex < iSrcYMax; iYIndex++)
	{
		for ( iXIndex = iSrcX; iXIndex < iSrcXMax; iXIndex++)
		{
			/*
        Compute the location of the pixel in the source bitmap after rotation
      */
			fixed fxX = itofx( iXIndex );
			fixed fxY = itofx( iYIndex );
			fixed fxRotatedX = Mulfx(fxX,fxCos) - Mulfx(fxY,fxSin) + f_0_5 + fxRotationAxisX;
			fixed fxRotatedY = Mulfx(fxX,fxSin) + Mulfx(fxY,fxCos) + f_0_5 + fxRotationAxisY;
			int iDestRotatedX = fxtoi(fxRotatedX);
			int iDestRotatedY = fxtoi(fxRotatedY);

      if( 0 == pucCharBm[iCharOffset] )
      {        
			  if ((iDestRotatedX >= 0) && (iDestRotatedX < iDestWidth) && 
            (iDestRotatedY >= 0) && (iDestRotatedY < iDestHeight ))
			  {
          // Compute the destination location of the pixel and copy.
				  int iDestOffset = iDestRotatedY*iDestWidth + iDestRotatedX;
          pucDestBm[iDestOffset] = iForegroundColorIndex;

          /*
            Sometimes a source pixel will overlap multiple destination pixels,
            in other words the destination x and y coordinates are not integers.
            The source pixel's color will be copied to each overlapped 
            destination pixel.  The source pixel has already been copied to
            the lower left overlapped pixel in the destination.
          */
          fxFracX = fxfract( fxRotatedX );
          fxFracY = fxfract( fxRotatedY );
          /*
            Make not to go outside of the bitmap.
          */
          bClipX = ( iDestRotatedX + 1 >= iDestWidth ) ? CL_TRUE : CL_FALSE;
          bClipY = ( iDestRotatedY + 1 >= iDestHeight ) ? CL_TRUE : CL_FALSE;

          /*
            Color in overlapped pixels.
          */
          if( fxFracX >= f_0_5 && !bClipX )
            /*
              Lower right pixel.
            */
            pucDestBm[iDestOffset + 1] = iForegroundColorIndex;
          if( fxFracY >= f_0_5 && !bClipY )
            /*
              Upper left pixel.
            */
            pucDestBm[iDestOffset + iDestWidth] = iForegroundColorIndex;
          if( fxFracX >= f_0_5 && fxFracY >= f_0_5 && !bClipX && !bClipY )
            /*
              Upper right pixel.
            */
            pucDestBm[iDestOffset + 1 + iDestWidth] = iForegroundColorIndex;
			  }
      }

			/*
        Go to the next source pixel. 
        iDstTotalOffset = yIndex * pBitmap->nWidth + xIndex;
      */
			++iCharOffset;
		}
  }

}

//**********************************************************************
//  FUNCTION:
//    cl_freadCharDirEntry
//
//  DESCRIPTION:	
//    Reads a dir entry.  The fields are bytes swapped if the processor
//    uses the big endian format
//    
//
//  Inputs:
//    CLFILE *pFID
//      Input file.
//
//  Outputs:
//    CharDirEntry *pCharDirEntry
//      Contains the bytes swapped dir entry
//
//  GLOBALS:
//    none
//**********************************************************************
static void cl_freadCharDirEntry( CharDirEntry *pCharDirEntry, CLFILE *pFID )
{
   pCharDirEntry->iC = cl_freadS32(pFID);
   pCharDirEntry->iOffset = cl_freadS32(pFID);
   pCharDirEntry->iWidth = cl_freadS32(pFID);
   pCharDirEntry->iHeight = cl_freadS32(pFID);
}

