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
//    Chart.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    02/10/2009
//
//  DESCRIPTION:
//    The chart module renders charts.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if defined(DEBUG)
#include <limits.h>
#endif
#include "ChartLib.h"
#include "Chart.h"
#include "lkmhbird.h"
#include "utm.h"
#include "FixedPoint.h"
#include "ClMalloc.h"
#include "ClAssert.h"
#include "StretchHBbits.h"
#include "error.h"
#include "ByteSwap.h"


//**********************************************************************
// constants
//**********************************************************************
/*
  MIN_CHART_MEM_LEN is the amount of memory that is reserved for 
  memory used by LkmToHbirdImage.c to getting charts and chart objects
  from the database.
*/
#define MIN_CHART_MEM_LEN  (253*1024) //(150*1024)

/*
  In RenderChart() there may be empty regions in the buffer if the destination
  buffer overlaps the outter boundries of the chart.  The empty regions will
  be filled in with a color.
*/
#define EMPTY_RGN_COLOR_RED   64
#define EMPTY_RGN_COLOR_BLUE  64
#define EMPTY_RGN_COLOR_GREEN  64

#define PIXEL_X_MIN (-64*1024)
#define PIXEL_X_MAX (64*1024)
#define PIXEL_Y_MIN (-64*1024)
#define PIXEL_Y_MAX (64*1024)

//**********************************************************************
// macros
//**********************************************************************
#define CL_FABS(x) ( ( (x) >= 0 ) ? (x) : -(x) )

//**********************************************************************
// Types
//**********************************************************************
/*  
  ProcessingData

  ProcessingData is used by HBDisplayTextChar() and HBDisplayBitmap() 
  to render text and icons after the chart has been rotated and copied to 
  a buffer in ClChartRender(). The HBImageHandle paramenter is a pointer to a 
  ProcessingData object.
*/
typedef struct ProcessingDataTag
{
  ChartInstance *pChart;
  Bitmap destBitmap;
  int iUtmCenterX;
  int iUtmCenterY;
  int iDegrees;
  char sLastIconFilePath[CL_MAX_PATH];
  Bitmap iconBm;
  ClError textError;
  ClError iconError;
} ProcessingData;

//**********************************************************************
// Private Data.
//**********************************************************************
#define CL_CARD_NAME_LEN 12

//**********************************************************************
// Private Function Prototypes.
//**********************************************************************
static ClError AllocateBitmapCache
(
  ChartInstance *pChart,
  CL_BOOL bIsNorthUp,
  int iWidth,
  int iHeight
);

static void ClCidtoString( ClCID *pCID, char *psCID );

static void ClClearChart(ChartInstance *pChart);
static void VerifyDestRectInBounds
(
  ChartInstance *pChart,
  int iCenterX,
  int iCenterY,
  ClBitmap *pDestBitmap,
  int iHeadingRotation,
  CL_BOOL *pbOutOfBounds
);


static void RotatePoint
(
  int iDegrees,
  int iXCenter,
  int iYCenter,
  int *piX,
  int *piY
);


//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    ClCreateInstance
//
//  DESCRIPTION:	
//    Creates an instance handle that is used for rendering charts 
//    and enumerating objects.  ClCreateInstance initializes the chart
//    library's internal data and memory.  An instance handle whose value 
//    is NULL is considered invalid.
//
//  Inputs:
//    None
//
//  Outputs:
//    None
//
//  Returns:
//    Returns a handle to a chart.
//    NULL is returned if there is an error.
//
//  GLOBALS:
//    none
//**********************************************************************
ClInstanceHandle ClCreateInstance( char *psChartPath, ClCID *pCID )
{
  ChartInstance *pChart = NULL;
  CL_BOOL bResult = CL_FALSE;
  char sCID[CL_CID_STRING_LEN];
  char sCardName[CL_CARD_NAME_LEN];

  ErrorClear();

  pChart = (ChartInstance *)cl_malloc( sizeof( ChartInstance ) );

  if( NULL != pChart )
  {  
    memset( pChart, 0, sizeof( ChartInstance ) );

    BmInit( &pChart->bm );

    if( NULL != psChartPath )
    {
      strncpy(pChart->sPath, psChartPath, CL_MAX_PATH);
      pChart->sPath[ CL_MAX_PATH - 1 ] = '\0';
      strncat(pChart->sPath,CHART_PATH,CL_MAX_PATH - 1);
    }
    else 
    {
     strncpy(pChart->sPath,CHART_PATH,CL_MAX_PATH);
     pChart->sPath[ CL_MAX_PATH - 1 ] = '\0';
    }
    
    ClCidtoString( pCID, sCID );

    pChart->iUtmZone = LKMToHBInit( pChart->sPath, sCID, sCardName );
    if (!pChart->iUtmZone)
    {
       ClDestroyInstance(pChart);
       WRITE_TO_ERROR_LOG( "LKMToHBInit() failed while creating a chart instance." );
       return NULL;
    }

    ClLakeListLoad(pChart);
  }

  if( NULL == pChart )
    WRITE_TO_ERROR_LOG( "Could not allocate memory for chart instance." );

  return PCHART_2_CHART_HINST( pChart );
}

//**********************************************************************
//  FUNCTION:
//    ClDestroyInstance
//
//  DESCRIPTION:	
//    Finalizes a chart instance.  Once ClDestroyInstance is called 
//    on an instance's handle it cannot be used by any chart library 
//    functions.
//
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
void ClDestroyInstance( ClInstanceHandle hInstance )
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);

  if( NULL == pChart )
    return;
  
  BmDestroy( &pChart->bm );
  
  cl_free( pChart );

}

//**********************************************************************
//  FUNCTION:
//    ClIsChartCardValid
//
//  DESCRIPTION:	
//    Determines if the indicated device contains is a valid LakeMaster
//    SD card.
//
//  Inputs:
//    char *psChartPath
//      Device name.
//    ClCID *pCID
//      The CID that was read from the card.
//
//  Outputs:
//    None.
//
//  Returns:
//    CL_TRUE
//      The card is valid.
//    CL_FALSE
//      The card is invalid.
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL ClIsChartCardValid( char *psChartPath, ClCID *pCID )
{
	CL_BOOL bValid = CL_FALSE;
  char sCID[CL_CID_STRING_LEN];
  char sPath[CL_MAX_PATH];

  ErrorClear();

  ClCidtoString( pCID, sCID );

  if( NULL != psChartPath ) {
    strncpy( sPath, psChartPath, CL_MAX_PATH );
    sPath[ CL_MAX_PATH - 1 ] = '\0';
    strncat( sPath, CHART_PATH, CL_MAX_PATH - 1 );
  }
  else
  {
   strncpy( sPath, CHART_PATH, CL_MAX_PATH );
   sPath[ CL_MAX_PATH - 1 ] = '\0';
  }


  if( LKMCheckCardCID( sPath, sCID ) )
    bValid = CL_TRUE;
  else
  {
    bValid = CL_FALSE;
    WRITE_TO_ERROR_LOG( "Chart card not valid." );
  }


  return bValid;
}

//**********************************************************************
//  FUNCTION:
//    ClChartCardName
//
//  DESCRIPTION:	
//    Returns short name of chart card in the indicated device
//
//  Inputs:
//    char *psChartPath
//      Device name.
//    int iMaxLen
//      The maximum number of characters to return in the card name.
//  Outputs:
//    char *psCardName
//      Returns the card name.
//
//  Returns:
//    CL_TRUE
//      Success.
//    CL_FALSE
//      Failure.
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL ClChartCardName( char *psChartPath, char *psCardName, int iMaxLen )
{
	CL_BOOL bValid = CL_FALSE;
  char sPath[CL_MAX_PATH];

  ErrorClear();

  if( NULL != psChartPath ) {
    strncpy( sPath, psChartPath, CL_MAX_PATH );
    sPath[ CL_MAX_PATH - 1 ] = '\0';
    strncat( sPath,CHART_PATH,CL_MAX_PATH - 1 );
  }
  else 
  {
   strncpy( sPath, CHART_PATH, CL_MAX_PATH );
   sPath[ CL_MAX_PATH - 1 ] = '\0';
  }

  if( LKMGetCardName( sPath, psCardName, iMaxLen ) )
    return CL_TRUE;
  else
  {
    WRITE_TO_ERROR_LOG( "Could not read the cart name." );
    return CL_FALSE;
  }

}


//**********************************************************************
//  FUNCTION:
//    ClCidtoString
//
//  DESCRIPTION:	
//    Copies a CID to a string.
//
//  Inputs:
//    ClCID *pCID
//      Point to a card identification string.
//    char *psCID
//      Returns the CID string.  The string will be at most 
//      CID_STRING_LEN characters in length.      
//
//  Outputs:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
static void ClCidtoString( ClCID *pCID, char *psCID )
{
  int i;

  for( i = 0; i < pCID->CIDLength; ++i )
    sprintf( psCID + 2*i, "%-2.2x", pCID->CID[i] );
  psCID[2*pCID->CIDLength] = '\0';

}
//**********************************************************************
//  FUNCTION:
//    ClGetChartPalette
//
//  DESCRIPTION:	
//    Retreives a pointer to a palette in the chart library. 
//
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//  
//  Outputs:
//    Palette **pPalette
//      Returns a pointer to the chart’s internal palette.  The 
//      colors in the palette may change without notice.  The 
//      pointer will become invalid when DestroyInstance is called.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClGetChartPalette( ClInstanceHandle hInstance, ClPalette **ppPalette )
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);

  *ppPalette = &pChart->bm.palette;

  return 0;
}


//**********************************************************************
//  FUNCTION:
//    AllocateBitmapCache
//
//  DESCRIPTION:	
//    Allocates memory for the bitmap cache.
//
//  Inputs:
//    int iWidth
//      The minimum width of the bitmap in pixels.
//
//    int iHeight
//      The minimum height of the bitmap in pixels.
//    
//  Outputs:
//    None
//
//  Return:
//    clError_ChartMemAlloc.
//
//  GLOBALS:
//    None
//
//**********************************************************************
static ClError AllocateBitmapCache
(
  ChartInstance *pChart,
  CL_BOOL bIsNorthUp,
  int iWidth,
  int iHeight
)
{
  CL_BOOL bIsMemAlloc;
  fixed fxDiagnal;
  unsigned long unFreeBytes;
  int iBitmapBytes;
  int iNewWidth;
  int iNewHeight;
  ClError error = clError_Success;

  ClAssert( NULL != pChart );
  ClAssert( iWidth > 0 );
  ClAssert( iHeight > 0 );

  do { // use exception pattern of error handling.

    if( !bIsNorthUp )
    {
      /*
        The destination bitmap must be large enough to hold a chart
        that is rotated 45 degrees.
      */
      // diagnal = sqrt(width**2 + height**2)
      fxDiagnal = itofx( iWidth * iWidth ) + itofx( iHeight * iHeight );
      fxDiagnal = FxSqrt( fxDiagnal );
      iWidth = iHeight = fxtoi( fxDiagnal );
    }

    /*
      Is enough memory allocated for the chart?
    */
    bIsMemAlloc = CL_FALSE;
    if( BmIsValid( &pChart->bm ) )
    {
      if( bIsNorthUp )
      {
        if( iWidth == pChart->bm.nWidth && iHeight == pChart->bm.nHeight )
          bIsMemAlloc = CL_TRUE;
      }
      else
      {
        if( iWidth <= pChart->bm.nWidth && iHeight <= pChart->bm.nHeight )
          bIsMemAlloc = CL_TRUE;
      }
    }

    if( bIsMemAlloc )
      break;

    /*
      Force the chart to be rendered since the current chart is 
      being discarded.
    */
    pChart->bIsRendered = CL_FALSE;

    if( BmIsValid( &pChart->bm ) )
      BmDestroy( &pChart->bm );

    /*
      See if there is enough memory for the bitmap.
    */
    unFreeBytes = ClFreeMemorySize();
    iBitmapBytes = ( unFreeBytes - MIN_CHART_MEM_LEN );

    if( iBitmapBytes <= 0 || ( iBitmapBytes <  iWidth * iHeight  ) )
    {
      error = clError_ChartMemAlloc;
      WRITE_TO_ERROR_LOG2( error );
      break;
    }

    if( bIsNorthUp )
    {
      /*
        Allocate the minimum memory that is required.
        Char rendering speed is greatly increased by 
      */
      iNewWidth = iWidth;
      iNewHeight = iHeight;
    }
    else
    {
      iNewHeight = fxtoi( FxSqrt( itofx(iBitmapBytes) ) );
      iNewWidth = iNewHeight;
    }

    if( !BmCreate
        ( 
          &pChart->bm,
          iNewWidth,
          iNewHeight,
          NULL 
        )
    )
    {
      error = clError_ChartMemAlloc;
      WRITE_TO_ERROR_LOG2( error );
    }
    
  } while(0);

  return error;
}

//**********************************************************************
//  FUNCTION:
//    ClRenderChart
//
//  DESCRIPTION:	
//    The Render Chart function will render a two dimensional chart in 
//    top down view.  The routine will search the database for the 
//    'best fit' chart for a specific scale and position.  The chart will
//    be rendered onto a bitmap supplied by the Humminbird application.
//
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//
//  double *pdblLatitude
//    Actual Latitudinal coordinate of the point that will appear in the
//    center of the display.
//
//  double *pdblLongitude
//    Inputs the longitudinal coordinate of the point that will appear in
//    the center of the display.
//
//  double *pdblScale
//    The routine will search for a chart that is closest to this scale.
//    The scale will be measured in meters per pixel
//
//  int *puiHeadingRotation
//    The heading rotation is the change in the watercraft’s heading 
//    measured in degrees.  North is zero degrees and east is 90 degrees.
//    The heading rotation resolution will be less than 1 degree.
//
//  int iWantDepthColors
//    Indicates whether the chart is to be colored by depth.
//    Values:
//      0 - Entire lakes are a single light blue color.
//      1 - Each depth polygon will be colored according to its depth, with
//          darker blues indicating deeper depths.
//
//  int iWantContourLines
//    Indicates whether contour lines and elevation text will be displayed. 
//      Values: 
//      0 - Lines and text will not be displayed.
//      1 - Lines and text will be displayed.
//
//  int iHighlightDepth
//    Center of a range of depths that are to be highlighted.  A value of 0 
//    turns off depth highlighting.
//
//  int iHighlightDepthRange
//    Specifies the range of depths to be highlighted. For example, a 
//    HighlightDepth of 25 and a HighlightDepthRange of 5 would highlight all
//    depth polygons with depths between 20 and 30.  When depths are 
//    highlighted they are dark blue and all other depths are light blue.
//
//  ClBitmap *pDestBitmap
//    A pointer to a Bitmap structure.  The chart will be rendered in the array
//    pointed to by the pixels member.
//
//  ClChartRenderCb pfCallBack
//    Specifies the address of a call back function that will be called while
//    the chart is being rendered.
//
//  void *pCallBackData 
//    A variable or pointer to data that is passed to the call back function.
//
//  Outputs:
//  double *pdblLatitude
//    Actual Latitudinal coordinate of the point that will appear in the center
//    of the display.
//  double *pdblLongitude,
//    Actual longitudinal coordinate of the point that will appear in the
//    center of the display.
//  double *pdblScale,
//    The actual scale of the chart that was rendered onto the bitmap.
//  int *puiHeadingRotation,
//    The actual heading rotation of the chart that was rendered onto the
//    bitmap.
//
//  Return values:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
ClError ClRenderChart
(
  ClInstanceHandle hInstance,
  ClRenderChartParams *pParams,
  ClBitmap *pDestBitmap,
  ClChartRenderCb pfCallBack,
  void *pCallBackData 
)
{
  int iUtmCenterX;
  int iUtmCenterY;
  int iCenterX;
  int iCenterY;
  CL_BOOL bOutOfBounds = CL_FALSE;
  double dblScaleArg;
  int iHeadingRotation;
  int iDeltaRotation;
  ClRenderChartParams *pLastParams = NULL;
  ProcessingData procData;
  ClPalette backupPalette;
  int hbResult = 0;
  CL_BOOL bContinue = CL_TRUE;
  CL_BOOL bResult = CL_TRUE;
  ClError error = clError_Success;

  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);

  ClAssert( NULL != pChart );
  ClAssert( NULL != pParams );
  ClAssert( NULL != pDestBitmap );

  g_bIsClChartRenderCalled = CL_TRUE;
  pLastParams = &pChart->lastParams;
  
  if( pParams->bIsNorthUp )
    iHeadingRotation = 0;
  else
    iHeadingRotation = pParams->iHeadingRotation;

  ErrorClear();

  /*
    Set the ClRenderChart parameters in the error module.
    If there is an error then the parameters will be written to the error log.
  */
  SetClRenderChartParams
  (
    hInstance,
    pParams,
    pDestBitmap,
    pfCallBack,
    pCallBackData 
  );
  

  /*
    Backup the chart palette.  If there are any errors the palette will need
    to be restored.  ClGetChartPalette() needs to read the last valid palette.
  */
  if( BmIsValid( &pChart->bm ) )
    PaletteCopy( &backupPalette, &pChart->bm.palette );
  else
  {
    ClColor emptyRgn;
    int iColorIndex;
    CL_BOOL retCode;

    PaletteClear( &backupPalette );

    emptyRgn.ucRed = EMPTY_RGN_COLOR_RED;
    emptyRgn.ucGreen = EMPTY_RGN_COLOR_GREEN;
    emptyRgn.ucBlue = EMPTY_RGN_COLOR_BLUE;

    retCode = PaletteColorAdd( &backupPalette, emptyRgn, &iColorIndex );
    ClAssert( retCode ); // It's OK if this fails.  The chart did not have a valid palette anyway.
  }
  
  /*
    The chart bitmap cache should be large enough such that when the chart 
    can be copied to pDestBitmap after its has been rotated +- 45 degrees.

    If the chart is not large enough then the corners of pDestBitmap will be blank.

    Allocate space for sqrt( width*width + height*height )

  */
  error = AllocateBitmapCache( pChart, pParams->bIsNorthUp,pDestBitmap->iWidth, pDestBitmap->iHeight );

  if( clError_Success == error )
  {    

    /*
      Save the scale argument in pScale after the bitmap is rendered.
      LakeMasterToHBirdImage() changes *pdblScale.  The scale
      will be saved in pLastParams->dblScale after LakeMasterToHBirdImage() is called.
    */
    dblScaleArg = pParams->dblScale;

    error = GeoToBitmapCoord
    (
      pParams->dblLatitude,
      pParams->dblLongitude,
      pChart->iUtmZone,
      &iUtmCenterX,
      &iUtmCenterY
    );

  } // if( clError_Success != error )

  if( error == clError_Success )
  {
    /*
      Find the center point in pixels.
    */
    if( pChart->bIsRendered )
    {

      iCenterX = (int)( itofx( iUtmCenterX - pChart->iUtmX ) / dtofx( pChart->dblActualScale ) );
      iCenterY = (int)( itofx( iUtmCenterY - pChart->iUtmY ) / dtofx( pChart->dblActualScale ) );

      /*
        If the dest bitmap overlaps the buffer boundries then a new chart needs to be loaded.
      */
      if( iCenterX < 0 || iCenterY < 0 )
        bOutOfBounds = CL_TRUE;
      else
        VerifyDestRectInBounds
        (
          pChart,
          iCenterX,
          iCenterY,
          pDestBitmap,
          iHeadingRotation,
          &bOutOfBounds
        );

    }
    else
    {
      iCenterX = pChart->bm.nWidth / 2;
      iCenterY = pChart->bm.nHeight / 2;
    }

    /*
      Retreive a new chart if:
        - A chart has not been rendered.
        - Part of the destination bitmap rectangle lies outside of the chart.
        - Scale changes.
        - Rotation changed more than +- 90 degrees.
        - Contour lines are toggled.
        - Depth colors are toggled.
        - Highlight depth options change.
        - Safety depth options change.
        - Lake level option changes.
    */
    iDeltaRotation = iHeadingRotation - pLastParams->iHeadingRotation;
    iDeltaRotation = ( iDeltaRotation < 0 ) ? -iDeltaRotation : iDeltaRotation;

  } // if( error == clError_Success )

  if(
    clError_Success == error && (
      !pChart->bIsRendered                                                ||
      bOutOfBounds                                                        ||
      pLastParams->dblScale != dblScaleArg                                ||
      pLastParams->bIsNorthUp != pParams->bIsNorthUp                      ||
      iDeltaRotation > 90                                                 ||
      pLastParams->iLakeLevelOffset != pParams->iLakeLevelOffset          ||
      pLastParams->iHighLightLMLakes != pParams->iHighLightLMLakes        ||
      pLastParams->iSeamless != pParams->iSeamless                        ||
      pLastParams->iWantContourLines != pParams->iWantContourLines        ||
      pLastParams->iWantDepthColors != pParams->iWantDepthColors          ||
      pLastParams->iHighlightDepth != pParams->iHighlightDepth            ||
      pLastParams->iHighlightDepthRange != pParams->iHighlightDepthRange  ||
      pLastParams->bShowSafetyDepths != pParams->bShowSafetyDepths        ||
      pLastParams->iSafetyDepth != pParams->iSafetyDepth
    )
  )
  {

    g_bIsRenderingChart = CL_TRUE;

    ClClearChart(pChart);

    /*
      Retreive the new chart.
    */
    bContinue = LakeMasterToHBirdImage
    (
      (HANDLE)pChart,
      pChart->bm.nWidth, 
      pChart->bm.nHeight,
      iUtmCenterX,
      iUtmCenterY,
      &pParams->dblScale,
      pParams->iLakeLevelOffset,
      pParams->iHighLightLMLakes,
      pParams->iSeamless,
      iHeadingRotation,
      pParams->iWantDepthColors,
      pParams->iWantContourLines,
      pParams->iHighlightDepth,
      pParams->iHighlightDepthRange,
      pParams->bShowSafetyDepths ? 1 : 0,
      pParams->iSafetyDepth,
      1,
      pfCallBack,
      pCallBackData,
      &hbResult
    );

    g_bIsRenderingChart = CL_FALSE;

    iCenterX = pChart->bm.nWidth / 2;
    iCenterY = pChart->bm.nHeight / 2;

    if( 0 != hbResult )
    {
      error = clError_ChartDbAccess;
      WRITE_TO_ERROR_LOG2( error );
    }
    else if( !bContinue )
    {
      error = clError_ChartRenderAbort;
      WRITE_TO_ERROR_LOG( "LakeMasterToHBirdImage() aborted." );
    }
    else
    {

      /*
        Save the current settings. If the settings change the next time this 
        function is caled then LakeMasterToHBirdImage() will need to be called 
        again.
      */
      pChart->bIsRendered = CL_TRUE;
      pChart->iUtmX = iUtmCenterX - fxtoi( fxround( iCenterX * dtofx(pParams->dblScale) ) );
      pChart->iUtmY = iUtmCenterY - fxtoi( fxround( iCenterY * dtofx(pParams->dblScale) ) );

      memmove( pLastParams, pParams, sizeof( ClRenderChartParams ) );
      pLastParams->dblScale = pParams->dblScale;
      pLastParams->iHeadingRotation = iHeadingRotation;
      pChart->dblActualScale = pParams->dblScale;   
    } // if( bContinue )

    if( clError_Success != error )
    {
       /*
        The palette needs to be restored if there was an error.
        ClGetChartPalette() needs to read the last good palette.
      */
      PaletteCopy( &pChart->bm.palette, &backupPalette );
    }

  } // if( error == clError_Success && ...

  if( error == clError_Success )
  {
    g_bIsRotatingAndCopyingChart = CL_TRUE;
    /*
      Copy the bitmap to the destination.
    */
    BmRotateCopy
    (
      &pChart->bm,
      iCenterX,
      iCenterY,
      iHeadingRotation,
      pDestBitmap->pData,
      pDestBitmap->iWidth,
      pDestBitmap->iHeight
    );
    g_bIsRotatingAndCopyingChart = CL_FALSE;

    /*
      Gather data for rendering text and icons.
    */
    procData.pChart = pChart;
    procData.destBitmap.nWidth = pDestBitmap->iWidth;
    procData.destBitmap.nHeight = pDestBitmap->iHeight;
    procData.destBitmap.pucData = pDestBitmap->pData;
    PaletteCopy( &procData.destBitmap.palette, &pChart->bm.palette );
    procData.iUtmCenterX = iUtmCenterX;
    procData.iUtmCenterY = iUtmCenterY;
    procData.iDegrees = iHeadingRotation;
    procData.sLastIconFilePath[0] = '\0';
    BmInit( &procData.iconBm );
    procData.textError = clError_Success;
    procData.iconError = clError_Success;

    g_bIsTextImagesInitCalled = CL_TRUE;
    error = TextImagesInit( &pChart->images, pChart->sPath, 18 );
    g_bIsTextImagesInitCalled = CL_FALSE;

    g_bIsChartPostProcessing = CL_TRUE;

    /*
      Call LKMPostRotationProcessing() even if text image initialization fails.
      LKMPostRotationProcessing() calls HBDisplayTextChar() and HBDisplayBitmap().
      HBDisplayTextChar() will skip text rendering if text images are not initialized.
      Icons need to be rendered no matter what.
    */
    bContinue = LKMPostRotationProcessing
    (
      (HANDLE)&procData,
      pChart->bm.nWidth,
      pChart->bm.nHeight,
      iUtmCenterX,
      iUtmCenterY,
      pChart->dblActualScale,
      pParams->iLakeLevelOffset,
      pParams->iHighLightLMLakes,
      pParams->iWantDepthColors,
      pParams->iWantContourLines,
      pParams->iHighlightDepth,
      pParams->iHighlightDepthRange,
      pParams->bShowSafetyDepths ? 1 : 0,
      pParams->iSafetyDepth,
      pfCallBack,
      pCallBackData
    );

    g_bIsChartPostProcessing = CL_FALSE;

    if( !bContinue )
    {
      error = clError_ChartRenderAbort;
      WRITE_TO_ERROR_LOG( "LKMPostRotationProcessing() aborted." );
    }

		if( clError_Success == error  )
    {
      // Change the error code only if TextImagesInit() was successful.
      if( clError_Success != procData.textError )
      {
        error = procData.textError;
        WRITE_TO_ERROR_LOG2(error);
      }
      else if( clError_Success != procData.iconError )
      {
        error = procData.iconError;
        WRITE_TO_ERROR_LOG2(error);
      }
    }

    BmDestroy( &procData.iconBm );
    TextImagesDestroy( &pChart->images );

    /*
      Colors were added to the temporary palette when rendering text and icons.
      Make sure that the new colors are copied back into the chart palette.
    */    
    PaletteCopy( &pChart->bm.palette, &procData.destBitmap.palette );

  } // if( clError_Success == error && bContinue )

  if( clError_Success != error )
    pChart->bIsRendered = CL_FALSE;

  //ClDisplayChartErrorMsg( pChart, pDestBitmap, error,  18 );

  if( g_bWasErrorLogged && clError_Success == error )
    WRITE_TO_ERROR_LOG( "ClRenderChart():\tAn error was not handled during the ClRenderChart() function call." );

  DisplayErrorOnBitmap
  (
   pChart->sPath,
   &pChart->images,
   pDestBitmap,
   &pChart->bm.palette,
   18
  );

  g_bIsClChartRenderCalled = CL_FALSE;
#if ENABLE_ERROR_LOG
  if( g_bWasErrorLogged && clError_Success == error )
    WriteActionToErrorLog( "ClRenderChart() returning.\n");
#endif

  /*
    Stop writing parameters to the error log.
  */
  ClearClRenderChartParams();
  // Make sure that all of the action flags are cleared.
  ErrorClear();

  return error;
}

//**********************************************************************
//  FUNCTION:
//    ClClearChart
//
//  DESCRIPTION:	
//    Flood fills the chart with a grey back ground.
//
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart that has been rendered by ClRenderChart().
//
//  double dblLatitude
//  double dblLongitude
//    The geographic location of a pixel.
//
//  ClBitmap *pDestBitmap
//    The destination bitmap that contains the pixel.  The bitmap should be the same
//    one passed to the last call to ClRenderChart().
//
//  Outputs:
//    int *piPixelX
//    int *piPixelY
//      The coordinate of the pixel in the destination bitmap.
//
//  Return values:
//    CL_FALSE
//      The chart was not rendered therefore an accurate pixel location cannot
//      be generated.
//
//    CL_TRUE
//      The pixel location was calculated.
//
//  GLOBALS:
//    none
//**********************************************************************
static void ClClearChart(ChartInstance *pChart)
{
  ClColor emptyRgn;
  int colorIndex;
  CL_BOOL retCode = CL_FALSE;

  /*
  Clear the bitmap with the empty region color.

  There may be empty regions in the buffer if the destination buffer
  overlaps the outter boundries of the chart.  The empty regions will
  contain garbage or the contents of the last chart that was rendered.     
  The last chart data will be overwritten.
  */

  /*
  LakeMasterToHBirdImage() will build the palette for the chart.
  Clear the palette to make room for the new colors.
  */
  PaletteClear( &pChart->bm.palette );
  /*
  The color for the empty region must be the first one in the palette.
  */

  emptyRgn.ucRed = EMPTY_RGN_COLOR_RED;
  emptyRgn.ucGreen = EMPTY_RGN_COLOR_GREEN;
  emptyRgn.ucBlue = EMPTY_RGN_COLOR_BLUE;

  retCode = PaletteColorAdd( &pChart->bm.palette, emptyRgn, &colorIndex );
  ClAssert( retCode );

  BmClear( &pChart->bm, (unsigned char)colorIndex );
}
//**********************************************************************
//  FUNCTION:
//    ClCalcPixelCoord
//
//  DESCRIPTION:	
//    Computes a pixel coordinate from a lat/lon coordinate.  The coordinate
//    is a location in the destination bitmap.  The bitmap should be the same 
//    bitmap passed to the last call to ClRenderChart().  The coordinate is
//    computed from parameters used in the last ClRenderChart call.  
//
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart that has been rendered by ClRenderChart().
//
//  double dblLatitude
//  double dblLongitude
//    The geographic location of a pixel.
//
//  ClBitmap *pDestBitmap
//    The destination bitmap that contains the pixel.  The bitmap should be the same
//    one passed to the last call to ClRenderChart().
//
//  Outputs:
//    int *piPixelX
//    int *piPixelY
//      The coordinate of the pixel in the destination bitmap.
//
//  Return values:
//    CL_FALSE
//      The chart was not rendered therefore an accurate pixel location cannot
//      be generated.
//
//    CL_TRUE
//      The pixel location was calculated.
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL ClCalcPixelCoord
(
  ClInstanceHandle hInstance,
  ClRenderChartParams *pParams,
  ClBitmap *pDestBitmap,
  double dblLatitude,
  double dblLongitude,
  int *piPixelX,
  int *piPixelY
)
{
  int iCenterX;
  int iCenterY;
  int iUtmCenterX;
  int iUtmCenterY;
  int iUtmX;
  int iUtmY;
  int iDelta;
  CL_BOOL bResult = CL_TRUE;

  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);

  ErrorClear();

  ClAssert( NULL != pChart );
  ClAssert( NULL != pParams );

  *piPixelX = 0;
  *piPixelY = 0;

  /*
    Find the center point in pixels.
  */
  if( !pChart->bIsRendered )
    bResult = CL_FALSE;
  else
  {
    /*
      Get UTM coordinate of the chart's center point.
    */
    GeoToBitmapCoord
    (
      pParams->dblLatitude,
      pParams->dblLongitude,
      pChart->iUtmZone,
      &iUtmCenterX,
      &iUtmCenterY
    );

    /*
      Get UTM coordinate of requested location.
    */
    GeoToBitmapCoord
    (
      dblLatitude,
      dblLongitude,
      pChart->iUtmZone,
      &iUtmX,
      &iUtmY
    );

    /*
      Calculate coordinate of the center pixel of the bitmap.
    */
    iCenterX = pDestBitmap->iWidth / 2;
    iCenterY = pDestBitmap->iHeight / 2;

    iDelta = iUtmCenterX - iUtmX;
    *piPixelX = iCenterX - itofx(iDelta) / dtofx(pChart->dblActualScale);

    iDelta = iUtmCenterY - iUtmY;
    *piPixelY = iCenterY - itofx(iDelta) /  dtofx(pChart->dblActualScale);

    RotatePoint
    ( 
      pParams->iHeadingRotation, 
      iCenterX, 
      iCenterY, 
      piPixelX, 
      piPixelY
    );

    /*
      Clip the point to an acceptable range.
    */
    *piPixelX = CL_MAX( *piPixelX, PIXEL_X_MIN );
    *piPixelX = CL_MIN( *piPixelX, PIXEL_X_MAX );

    *piPixelY = CL_MAX( *piPixelY, PIXEL_Y_MIN );
    *piPixelY = CL_MIN( *piPixelY, PIXEL_Y_MAX );

  }

  return bResult;

}

//**********************************************************************
//  FUNCTION:
//    ClAdjustToClosestScale
//
//  DESCRIPTION:	
//    Computes the actual scale that will be used for an input requested scale.
//
//  Input:
//    double dblScale
//      The requested scale value.
//
//  Output:
//    None.
//
//  Return value:
//    The actual scale value.
//
//  GLOBALS:
//    none
//**********************************************************************
double ClAdjustToClosestScale
(
  double dblScale
)
{
  return AdjustToClosestScale(dblScale);
}

//**********************************************************************
//  FUNCTION:
//    GeoToBitmapCoord
//
//  DESCRIPTION:	
//    The Render Chart function will render a two dimensional chart in 
//    top down view.  The routine will search the database for the 
//    'best fit' chart for a specific scale and position.  The chart will
//    be rendered onto a bitmap supplied by the Humminbird application.
//
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//
//  double *pdblLatitude
//    Actual Latitudinal coordinate of the point that will appear in the
//    center of the display.
//
//  double *pdblLongitude
//    Inputs the longitudinal coordinate of the point that will appear in
//    the center of the display.
//
//  double dblLattitude,
//  double dblLongitude,
//  int *piX,
//  int *piY
//
//  Return values:
//    clError_Success
//    clError_InvalidLatLong
//
//  GLOBALS:
//    none
//**********************************************************************
ClError GeoToBitmapCoord
(
  double dblLattitude,
  double dblLongitude,
  int iUtmZone,
  int *piX,
  int *piY
)
{
  long nZone;
  char cHemisphere;
  double dblEasting;
  double dblNorthing;
  ClError error = clError_Success;
  long nStatus;
  
  nStatus = Convert_Geodetic_To_UTM
  (
    dblLattitude,
    dblLongitude,
    iUtmZone,
    &nZone,
    &cHemisphere,
    &dblEasting,
    &dblNorthing
  );

  if( 0 != nStatus )
  {
    error = clError_InvalidLatLon;
    WRITE_TO_ERROR_LOG2( error );
  }
  else
  {
    *piX = (int)dblEasting;
    *piY = (int)dblNorthing;
  }

  return clError_Success;
}

//**********************************************************************
//  FUNCTION:
//    VerifyDestRectInBounds
//
//  DESCRIPTION:	
//    Verify that the destination bitmap rectangle is inside of the
//    chart's rectangle.
//
//  Inputs:
//    Chart *pChart
//      The destination rectangle must be inside of pChart->bm's rectangle.
//    int iCenterX
//    int iCenterY
//      The coordinates of the destination rectangle's center point 
//      in the chart bitmap's rectangle.
//    ClBitmap *pDestBitmap
//      The destination bitmap.
//    int iHeadingRotation
//      The watercraft's heading.
//
//  Output:
//    CL_BOOL *pbOutOfBounds
//      CL_TRUE
//        The destination bitmap rectangle contains points outside of the chart
//        bitmap.
//      CL_FALSE
//        The entire destination bitmap rectangle is inside of the chart
//        bitmap.
//
//  Return values:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
static void VerifyDestRectInBounds
(
  ChartInstance *pChart,
  int iCenterX,
  int iCenterY,
  ClBitmap *pDestBitmap,
  int iHeadingRotation,
  CL_BOOL *pbOutOfBounds
)
{
  struct Point
  {
    int iX;
    int iY;
  };

  int iHalfWidth = pDestBitmap->iWidth / 2;
  int iHalfHeight = pDestBitmap->iHeight / 2;
  int i;

  struct PointTag
  {
    int iX;
    int iY;
  } p[4];
  
  p[0].iX = -iHalfWidth;
  p[0].iY = -iHalfHeight;
  p[1].iX = -iHalfWidth;
  p[1].iY = iHalfHeight;
  p[2].iX = iHalfWidth;
  p[2].iY = iHalfHeight;
  p[3].iX = iHalfWidth;
  p[3].iY = -iHalfHeight;

  *pbOutOfBounds = CL_FALSE;

  /*
    Rotate each destination rectangle vertice and 
    compute the distance outside of the chart's bitmap.
  */
  for( i = 0; i < ( sizeof(p)/sizeof(p[0]) ); ++i ) 
  {
    int iX = p[i].iX;
    int iY = p[i].iY;

    RotatePoint( -iHeadingRotation, 0, 0, &iX, &iY );

    /*
      Compute the vertice coordinate relative to the destination
      rectangle's center point.
      The center of the distination rectangle is at the same
      place as the center of the chart.
    */
    iX += iCenterX;
    iY += iCenterY;

    if( 
      iX < 0 || 
      iX > pChart->bm.nWidth || 
      iY < 0 || 
      iY > pChart->bm.nHeight
    )
    {
      *pbOutOfBounds = CL_TRUE;
      break;
    }

  } // for each point in p[].

}

//**********************************************************************
//  FUNCTION:
//    RotatePoint
//
//  DESCRIPTION:	
//   Rotates a point around a center point.
//
//  Inputs:
//    int iDegrees
//      Rotation angle.
//      If iDegrees > 0 then clockwise rotation.
//      If iDegrees < 0 then counter-clockwise rotation.
//    int iXCenter,
//    int iYCenter
//      The center point
//    int *piX
//    int *piY
//      The point to rotate.
//
//  Outputs:
//    int *piX
//    int *piY
//      The coordinates of the point after rotation.
//
//  Return values:
//    None
//
//  GLOBALS:
//    none
//**********************************************************************
static void RotatePoint( int iDegrees, int iXCenter, int iYCenter, int *piX, int *piY )
{
  if( iDegrees != 0 )
  {
    fixed fxAngle = Mulfx(Divfx(iDegrees,180), FX_PI);
    fixed fxCos = FxCos( fxAngle );
    fixed fxSin = FxSin( fxAngle );
    fixed fxXCenter = itofx(iXCenter);
    fixed fxYCenter = itofx(iYCenter);
    fixed fX = itofx( *piX ) - fxXCenter;
    fixed fY = itofx( *piY ) - fxYCenter;
    fixed f_xRotated = Mulfx(fX,fxCos) - Mulfx(fY,fxSin) + f_0_5 + fxXCenter;
    fixed f_yRotated = Mulfx(fX,fxSin) + Mulfx(fY,fxCos) + f_0_5 + fxYCenter;
    *piX = fxtoi(f_xRotated);
    *piY = fxtoi(f_yRotated);
  }
}


//**********************************************************************
//  FUNCTION:
//    HBDisplayTextChar
//
//  Description:	
//    Renders a string of characters on a chart.  Called by 
//    LKMPostRotationProcessing() after a chart has been rendered
//    and rotated.
//  
//  Inputs:
//    HANDLE HBImageHandle
//      Pointer to a TextRenderData object.
//    char *pucText
//      The string that will be rendered.  Might not be null terminated.
//    int iTextLen
//      The number of characters in the string.
//    int iUtmCenterX
//      The string's x center point in UTM coordinates.
//    int iUtmCenterY
//      The string's y center point in UTM coordinates.
//    int iRotation
//      10 times the rotation angle.  Rotation/10 is measured in degrees.
//      Rotation > 0: Rotate counter clockwise.
//      Rotation < 0: Rotate clockwise.
//    int iSize
//      The required size of the characters in points.
//    int iTextColor
//      The rgb value of the text foreground color.  See 
//      CL_COLORREF_TO_COLOR macro for the format of the text color. 
//    int iShadowColor
//      The rgb value of the shadow color.  See CL_COLORREF_TO_COLOR macro
//      for the format of the shadow color.  A shadow color of zero means
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
void HBDisplayTextChar
(
  HANDLE HBImageHandle,
  char *pucText, 
  int iTextLen,
  int iUtmCenterX, 
  int iUtmCenterY,
  int iRotation,
  int iSize,
  int iTextColor,
  int iShadowColor
)
{
  ProcessingData *pData = NULL;
  ChartInstance *pChart = NULL;
  Bitmap *pDestBitmap = NULL;
  int iDelta;
  int iCenterX;
  int iCenterY;
  ClError error = clError_Success;

  ClAssert( NULL != HBImageHandle );

  pData = (ProcessingData *)HBImageHandle;
  pChart = pData->pChart;
  pDestBitmap = &pData->destBitmap;
   
  ClAssert( NULL != pChart );
  ClAssert( NULL != pDestBitmap );
  ClAssert( 0 != pChart->dblActualScale );

  if( !TextImagesAreValid( &pChart->images ) )
  {
    /*
      Text initialization failed before LKMPostRotationProcessing() was called.
      Just skip text rendering.
    */
    return;
  }

  error = TextChangeFontSize( &pChart->images, pChart->sPath, (unsigned int)iSize  );

  if( clError_Success == error )
  {
    /*
      Compute the text center point in pixels.
    */

    iDelta = pData->iUtmCenterX - iUtmCenterX;
    iCenterX = pDestBitmap->nWidth / 2 - itofx(iDelta) / dtofx(pChart->dblActualScale);

    iDelta = pData->iUtmCenterY - iUtmCenterY;
    iCenterY = pDestBitmap->nHeight / 2 + itofx(iDelta) /  dtofx(pChart->dblActualScale);
    
    RotatePoint
    (
      -pData->iDegrees,
      pDestBitmap->nWidth / 2,
      pDestBitmap->nHeight / 2,
      &iCenterX,
      &iCenterY
    );

  #if ENABLE_TEXT_ROTATION
    /*
      iRotation = degrees * 10.
    */
    if( iRotation )
    {
      iRotation += pData->iDegrees * 10;
      /*
        Normalize the rotation between 0 and 360 degrees.
      */
      iRotation %= 3600;
      if( iRotation < 0 )
        iRotation = 3600 + iRotation;

      /*
         If the text is about to tilt downward then rotate
         it 180 degrees so it is about to tilt upward.
      */
      if( iRotation > 900 && iRotation < 2700  )
        iRotation += 1800;

      /*
        Change iRotation to degrees.
      */
      if( iRotation % 10 >= 5 ) // round nearest.
        iRotation += 10;
      iRotation /= 10;
    }

  #endif

    TextRender
    (
      &pChart->images,
      pDestBitmap,
      pucText, 
      iTextLen,
      CL_TRUE,
      iCenterX, 
      iCenterY,
      iRotation,
      iSize,
      iTextColor,
      iShadowColor
    );

  } //  if( clError_Success == error )

  if( clError_Success != error ) // Don't overwrite failure codes.
    pData->textError = error; 

}

//**********************************************************************
//  FUNCTION:
//    HBDisplayBitmap
//
//  Description:	
//    Renders icon's bitmap on a chart.  Called by 
//    LKMPostRotationProcessing() after a chart has been rendered
//    and rotated.
//  
//  Inputs:
//    HANDLE HBImageHandle
//      Pointer to a BitmapRenderData object.
//    char * BitmapPathName
//      Path to the bitmap being rendered.
//    int iPercentSize
//      The percent size of the displayed bitmap versus the size of the
//      bitmap from the file.
//    int iUtmCenterX
//    int iUtmCenterY
//      The UTM coordinate of the bitmap's center point.
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
void HBDisplayBitmap
(
  HANDLE HBImageHandle,
  char *psBitmapPathName,
  int iPercentSize,
  int iUtmCenterX,
  int iUtmCenterY
)
{

  ProcessingData *pData = NULL;
  ChartInstance *pChart = NULL;
  Bitmap *pDestBitmap = NULL;
  fixed fxPercentSize;
  int iDelta;
  int iCenterX;
  int iCenterY;
  int iDestX;
  int iDestY;
  int iDestWidth;
  int iDestHeight;
  _RGBQUAD palette[PALETTE_LEN];
  _RGBQUAD transparent;
  ClError error = clError_Success;

  ClAssert( NULL != HBImageHandle );
  ClAssert( NULL != psBitmapPathName );
  ClAssert( iPercentSize >= 0 );

  pData = (ProcessingData *)HBImageHandle;
  pChart = pData->pChart;
  pDestBitmap = &pData->destBitmap;
   
  ClAssert( NULL != pChart );
  ClAssert( NULL != pDestBitmap );

  /*
    Compute the icon bitmap center point in pixels.
  */
  iDelta = pData->iUtmCenterX - iUtmCenterX;
  iCenterX = pDestBitmap->nWidth / 2 - itofx(iDelta) / dtofx(pChart->dblActualScale);

  iDelta = pData->iUtmCenterY - iUtmCenterY;
  iCenterY = pDestBitmap->nHeight / 2 + itofx(iDelta) /  dtofx(pChart->dblActualScale);


  RotatePoint
  (
    -pData->iDegrees,
    pDestBitmap->nWidth / 2,
    pDestBitmap->nHeight / 2,
    &iCenterX,
    &iCenterY
  );

  /*
    Load the the icon file.
  */

  if( strncmp( pData->sLastIconFilePath, psBitmapPathName, CL_MAX_PATH ) )
  {

    BmDestroy( &pData->iconBm );
    error = BmLoad( &pData->iconBm, psBitmapPathName );
    if( clError_Success == error )
    {
      strncpy( pData->sLastIconFilePath, psBitmapPathName, CL_MAX_PATH );
      pData->sLastIconFilePath[ CL_MAX_PATH - 1] = '\0';
    }
  }

  if( clError_Success == error )
  {
    memset( palette, 0, PALETTE_LEN );
    PaletteCopyToRgbQuad( palette, pData->iconBm.palette.nColorCount, &pData->iconBm.palette );

    /*
      Compute the location, width, and height of the icon for the current scale.
    */  
    fxPercentSize = itofx( iPercentSize ) / 100;

    iDestWidth  = fxtoi( Mulfx( itofx(pData->iconBm.nWidth),  fxPercentSize ) + f_0_5 );
    iDestHeight = fxtoi( Mulfx( itofx(pData->iconBm.nHeight), fxPercentSize ) + f_0_5 );

    iDestX = iCenterX - iDestWidth / 2;
    iDestY = iCenterY - iDestHeight / 2;

    /*
      White areas of the icon are to appear transparent
      on the chart.
    */
    transparent.rgbRed    = 255;
    transparent.rgbGreen  = 255;
    transparent.rgbBlue   = 255;

    /*
      Copy the icon to the chart.
    */
    

    TransparentStretchHBBits
    (
      pDestBitmap,
      iDestX,
      iDestY,
      iDestWidth,
      iDestHeight,
      0,
      0,
      pData->iconBm.nWidth,
      pData->iconBm.nHeight,
      pData->iconBm.pucData,
      pData->iconBm.nHeight,
      pData->iconBm.nWidth,
      pData->iconBm.palette.nColorCount,
      palette,
      CL_TRUE,
      &transparent
    );

  } // if( clError_Success == error )

  if( clError_Success != error ) // Don't overwrite failure codes.
    pData->iconError = error;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListCorrectCapitalization
//
//  Description:	
//    Corrects the capitalization in a lake list item's name.
//  
//  Inputs:
//    char *pcSrc
//      Pointer to the name of a lake list item.
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
static void ClLakeListCorrectCapitalization(
  char *pcSrc
)
{
  char cCurr;
  int nPos;
  CL_BOOL bNewWord;

  if (!pcSrc) return;

  bNewWord = CL_TRUE;
  for (nPos=0; nPos<(int) strlen(pcSrc); nPos++) {
     cCurr = pcSrc[nPos];
     if (isspace(cCurr)) {
        bNewWord = CL_TRUE;
        continue;
     }
     if (isalpha(cCurr)) {
        if (bNewWord) {
           cCurr = toupper(cCurr);
        }
        else {
           cCurr = tolower(cCurr);
        }
        bNewWord = CL_FALSE;
     }
     pcSrc[nPos] = cCurr;
  }

  return;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListLoad
//
//  Description:	
//    Loads the lake list data for the specified chart instance.
//  
//  Inputs:
//    ChartInstance *pChart
//      A pointer to a chart structure.
//
//  Outputs:
//    None.
//
//  Returns:
//    Returns CL_TRUE if the lake list information was sucessfully loaded, CL_FALSE otherwise.
//
//  GLOBALS:
//    none
//
//**********************************************************************
CL_BOOL ClLakeListLoad(
  ChartInstance *pChart
)
{
  int nFileSize = 0;
  int nBytesLeft = 0;
  int nCurrRecord = 0;
  char psFileName[CL_MAX_PATH];
  char *psCurr = NULL;
  char *psFirst = NULL;
  char *psSave = NULL;
  CLFILE *pFile = NULL;
  LakeList *pLL = &pChart->lakeList;
  LakeListItem *pLLI;

  ErrorClear();

  // if there is already a loaded lake list, delete it
  pLL->bIsLoaded = CL_FALSE;
  pLL->iNumRecords = 0;
  if (pLL->pData) {
     cl_free(pLL->pData);
     pLL->pData = NULL;
  }
  if (pLL->pRecord) {
     cl_free(pLL->pRecord);
     pLL->pRecord = NULL;
  }

  // read the entire lake list file into memory
  strcpy(psFileName,pChart->sPath);
  strcat(psFileName,"lakelist.txt");
  pFile = db_fopen(psFileName,"rb");
  if (!pFile) return CL_FALSE;

  db_fseek(pFile,0,SEEK_END);
  nFileSize = cl_ftell(pFile);
  db_fseek(pFile,0,SEEK_SET);
  pLL->pData = cl_malloc(nFileSize);
  db_fread(pLL->pData,1,nFileSize,pFile);
  cl_fclose(pFile);
  pFile = NULL;

  // determine the number of records and allocate a structure for each of them
  psCurr = pLL->pData;
  nBytesLeft = nFileSize;
  psFirst = NULL;
  while (psCurr && (nBytesLeft > 0)) {
     pLL->iNumRecords++;
     psSave = psCurr;
     psCurr = memchr(psCurr,'\n',nBytesLeft);
     if (psCurr) {
        psCurr++;
        nBytesLeft -= (psCurr - psSave);
        if (!psFirst) psFirst = psCurr;
     }
     else {
        nBytesLeft = 0;
     }
  }
  pLL->iNumRecords--;
  pLL->pRecord = cl_malloc(pLL->iNumRecords * sizeof(LakeListItem));
  if (!pLL->pRecord) WRITE_TO_ERROR_LOG2( clError_MemAlloc );

  //fill in the information for each record
  psCurr = psFirst;
  nBytesLeft = nFileSize;
  nCurrRecord = -1;
  for (nCurrRecord=0; nCurrRecord<pLL->iNumRecords ; nCurrRecord++) {
     pLLI = &(pLL->pRecord[nCurrRecord]);

     // lake name
     pLLI->psName = strtok(psFirst,"\t\n");
     ClLakeListCorrectCapitalization(pLLI->psName);
     psFirst = NULL;

     // county
     pLLI->psCounty = strtok(NULL,"\t\n");

     // latitude
     psCurr = strtok(NULL,"\t\n");
     sscanf(psCurr,"%lf",&pLLI->fLatitude);

     // longitude
     psCurr = strtok(NULL,"\t\n");
     sscanf(psCurr,"%lf",&pLLI->fLongitude);

     // width
     psCurr = strtok(NULL,"\t\n");
     sscanf(psCurr,"%d",&pLLI->iWidth);

     // height
     psCurr = strtok(NULL,"\t\n");
     sscanf(psCurr,"%d",&pLLI->iHeight);

     // area
     psCurr = strtok(NULL,"\t\n");
     sscanf(psCurr,"%d",&pLLI->iAcres);

     // type
     psCurr = strtok(NULL,"\t\n");
     if (strncmp(psCurr,"PM",2) == 0) {
        pLLI->bProMap = CL_TRUE;
     }
     else {
        pLLI->bProMap = CL_FALSE;
     }
  }

  pLL->bIsLoaded = CL_TRUE;
  return CL_TRUE;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetNumberOfRecords
//
//  Description:	
//    Returns the number of elements in the lake list.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//
//  Outputs:
//    None.
//
//  Returns:
//    Returns the number of elements in the lake list.
//
//  GLOBALS:
//    none
//
//**********************************************************************
int ClLakeListGetNumberOfRecords(
  ClInstanceHandle hInstance
)
{
  int nCountTotal = 0;
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (pLL->bIsLoaded) nCountTotal = pLL->iNumRecords;

  return nCountTotal;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListIsRecordValid
//
//  Description:	
//    Indicates if the selected lake list item is valid.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    CL_TRUE if the index represents a valid lake list item, CL_FALSE otherwise.
//
//  GLOBALS:
//    none
//
//**********************************************************************
CL_BOOL ClLakeListIsRecordValid(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;
  CL_BOOL bValid = CL_FALSE;

  ErrorClear();

  if (pLL->bIsLoaded && (nIndex < ClLakeListGetNumberOfRecords(hInstance))) bValid = CL_TRUE;

  if( !bValid )
    WRITE_TO_ERROR_LOG( "Lake List record is not valid." );

	return bValid;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetLakeName
//
//  Description:	
//    Gets the name of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The name of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
char *ClLakeListGetLakeName(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return NULL;

  return pLL->pRecord[nIndex].psName;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListIsHiDef
//
//  Description:	
//    Indicates if the selected lake has hi-def data.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    CL_TRUE if the index represents a hi-def lake list item, CL_FALSE otherwise.
//
//  GLOBALS:
//    none
//
//**********************************************************************
CL_BOOL ClLakeListIsHiDef(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return CL_FALSE;

  return pLL->pRecord[nIndex].bProMap;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetHiDefCode
//
//  Description:	
//    Gets an abbreviation (e.g., "HD") indicating a hi-def dataset.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//
//  Outputs:
//    None.
//
//  Returns:
//    The abbreviation (e.g., "HD") indicating a hi-def dataset.
//
//  GLOBALS:
//    none
//
//**********************************************************************
char *ClLakeListGetHiDefCode(
  ClInstanceHandle hInstance)
{
  static char psHDCode[] = "HD";

  return psHDCode;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetCountyName
//
//  Description:	
//    Gets the county of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The county of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
char *ClLakeListGetCountyName(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return NULL;

  return pLL->pRecord[nIndex].psCounty;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetType
//
//  Description:	
//    Gets the type of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The type of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
char *ClLakeListGetType(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;
  static char psType[10];

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return NULL;

  if (pLL->pRecord[nIndex].bProMap) {
    strcpy(psType,"Hi-Def ProMap");
  }

  return psType;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetLatitude
//
//  Description:	
//    Gets the latitude of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The latitude of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
double ClLakeListGetLatitude(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return 0.0;

  return pLL->pRecord[nIndex].fLatitude;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetLongitude
//
//  Description:	
//    Gets the longitude of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The longitude of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
double ClLakeListGetLongitude(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return 0.0;

  return pLL->pRecord[nIndex].fLongitude;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetWidthInMeters
//
//  Description:	
//    Gets the width in meters of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The width in meters of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
double ClLakeListGetWidthInMeters(
  ClInstanceHandle hInstance,
  int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return 0.0;

  return pLL->pRecord[nIndex].iWidth;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetHeightInMeters
//
//  Description:	
//    Gets the height in meters of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The height in meters of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
double ClLakeListGetHeightInMeters(
   ClInstanceHandle hInstance,
   int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return 0.0;

  return pLL->pRecord[nIndex].iHeight;
}


//**********************************************************************
//  FUNCTION:
//    ClLakeListGetAcres
//
//  Description:	
//    Gets the area in acres of the specified lake.
//  
//  Inputs:
//    ClInstanceHandle hInstance
//      A handle to a chart.
//    int nIndex
//      Element index in the lake list.
//
//  Outputs:
//    None.
//
//  Returns:
//    The area in acres of the specified lake.
//
//  GLOBALS:
//    none
//
//**********************************************************************
double ClLakeListGetAcres(
   ClInstanceHandle hInstance,
   int nIndex)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  LakeList *pLL = &pChart->lakeList;

  if (!ClLakeListIsRecordValid(hInstance,nIndex)) return 0.0;

  return pLL->pRecord[nIndex].iAcres;
}
