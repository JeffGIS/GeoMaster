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
//    ChartLib.h
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    4/13/2009
//
//  DESCRIPTION:
//    Chart Library function and data type definitions.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifndef __CHART_LIB_H
#define  __CHART_LIB_H

//**********************************************************************
// Options
//**********************************************************************
// Call debug versions of cl_malloc and cl_free
// DEBUG_MEM is not a flag so it must either be #defined or not 
// defined beacause it is checked with #if defined(DEBUG_MEM) and #if 
// !defined(DEBUG_MEM).
//#define DEBUG_MEM

// Enable rotation for depth contour text.
#define ENABLE_TEXT_ROTATION      1

#define ENABLE_ERROR_LOG          0

#if ENABLE_ERROR_LOG

#if defined(HUMMINBIRD)
#ifndef linux
#define LOG_FILE_NAME "mmc:0:\\error.log"
#else
#define LOG_FILE_NAME "/mnt/mmc0/error.log"
#endif
#else
#define LOG_FILE_NAME "c:\\LkMaster\\error.log"
#endif
// The color of the error messages that may appear on the chart.
#define ERROR_TEXT_COLORREF 0x000000ff // format: 00 bb gg rr 

#endif //ENABLE_ERROR_LOG

//**********************************************************************
// macros
//**********************************************************************
#define CL_MIN(x,y) ( (x)<(y) ? (x) : (y) )
#define CL_MAX(x,y) ( (x)>(y) ? (x) : (y) )

//**********************************************************************
// Data types
//**********************************************************************

typedef int CL_BOOL;

#ifndef CL_TRUE
#define CL_TRUE  1
#endif

#ifndef CL_FALSE
#define CL_FALSE 0 
#endif

//**********************************************************************
// Palette defines.
//**********************************************************************
#define PALETTE_LEN 256
#define PALETTE_BYTE_LEN ( PALETTE_LEN * sizeof(ClColor) )

typedef struct ClColorTag
{
  unsigned char ucRed;
  unsigned char ucGreen; 
  unsigned char ucBlue;
} ClColor;

typedef struct ClPaletteTag
{
	int nColorCount;
	ClColor colors[PALETTE_LEN];
} ClPalette;

//**********************************************************************
// Error Numbers and messages
//**********************************************************************
typedef enum ClErrorTag
{
  clError_Success,
  clError_InvalidLatLon,
  clError_ChartMemAlloc,
  clError_ChartDbAccess,
  clError_ChartFileIo,
  clError_IconMemAlloc,
  clError_TextMemAlloc,
  clError_FontFileIo,
  clError_FontFileMissing,
  clError_FontFileEmpty,
  clError_FontFileInvalid,
  clError_FileIo,
  clError_MemAlloc,
  clError_ChartRenderAbort
} ClError;

#if ENABLE_ERROR_LOG

#define CLERROR_MSG_SUCCESS               ""
#define CLERROR_MSG_INVALID_LATLON        "Invalid Lat/Lon Coordinate."
#define CLERROR_MSG_CHART_MEM_ALLOC       "Chart memory allocation error."
#define CLERROR_MSG_CHART_DB_ACCESS       "Chart database access error."
#define CLERROR_MSG_CHART_FILE_IO         "Chart file io error."
#define CLERROR_MSG_ICON_MEM_ALLOC        "Not enough memory for rendering icons."
#define CLERROR_MSG_TEXT_MEM_ALLOC        "Not enough memory for rendering text."
#define CLERROR_MSG_FONT_FILE_IO          "Could not load the font list."
#define CLERROR_MSG_FONT_FILE_MISSING     "Could not find font the font list."
#define CLERROR_FONT_FILE_EMPTY           "The font list is empty."
#define CLERROR_FONT_FILE_INVALID         "Font list format is invalid."
#define CLERROR_MSG_FILE_IO               "General file io error."
#define CLERROR_MSG_MEM_ALLOC             "General memory allocation error."
#define CLERROR_MSG_CHART_RENDER_ABORT    "The chart rendering process was aborted."
#define CLERROR_UNKNOWN                   "Unknown error."

/*
  Location of the error message on a chart.  The chart's origin
  is in the upper left hand corner.
*/
#define ERROR_MESSAGE_COORD( bitmapWidth, bitmapHeight, x, y ) \
{ \
  (x) = (bitmapWidth) / 2; \
  (y) = 100; \
}

#endif


//**********************************************************************
// Memory management
//**********************************************************************
CL_BOOL ClSetHeap( void *pHeap, unsigned long unHeapSize );
CL_BOOL ClReleaseHeap( void **ppHeap );

//**********************************************************************
// Chart Initialization and Finalization
//**********************************************************************
typedef void* ClInstanceHandle;

#define CL_CID_BYTE_LEN 16
// Number of characters to format CID's as a string.
#define CL_CID_STRING_LEN ( 2 * CL_CID_BYTE_LEN + 1 )
typedef struct ClCIDTag
{
	int CIDLength;
	unsigned char CID[CL_CID_BYTE_LEN];
} ClCID;

ClInstanceHandle ClCreateInstance( char *psChartPath, ClCID *pCID );
CL_BOOL ClIsChartCardValid( char *psChartPath, ClCID *pucCID );
CL_BOOL ClChartCardName( char *psChartPath, char *psCardName, int iMaxLen );
void ClDestroyInstance( ClInstanceHandle hInstance );
int ClGetChartPalette( ClInstanceHandle hInstance, ClPalette **ppPalette );

//**********************************************************************
// Bitmap Rendering
//**********************************************************************

typedef struct ClBitmapTag
{
  // Width in pixels.
  int iWidth; 
  // Height in pixels
  int iHeight;
  // Bitmap pixels.
	unsigned char *pData;
	
} ClBitmap;


typedef CL_BOOL (*ClChartRenderCb)
( 
  ClInstanceHandle hInstance,
  void *pData
);

typedef struct ClRenderChartParamsTag
{
  double dblLatitude;
  double dblLongitude;
  double dblScale;
  CL_BOOL bIsNorthUp;
  int iHeadingRotation;
  int iLakeLevelOffset;
  int iHighLightLMLakes;
  int iSeamless;
  int iWantDepthColors;
  int iWantContourLines;
  int iHighlightDepth;
  int iHighlightDepthRange;
  CL_BOOL bShowSafetyDepths;
  int iSafetyDepth;
} ClRenderChartParams;

ClError ClRenderChart
(
  ClInstanceHandle hInstance,
  ClRenderChartParams *pParams,
  ClBitmap *pDestBitmap,
  ClChartRenderCb pfCallBack,
  void *pCallBackData 
);

CL_BOOL ClCalcPixelCoord
(
  ClInstanceHandle hInstance,
  ClRenderChartParams *pParams,
  ClBitmap *pDestBitmap,
  double dblLatitude,
  double dblLongitude,
  int *piPixelX,
  int *piPixelY
);

double ClAdjustToClosestScale
(
  double dblScale
);

//**********************************************************************
// Object Text enumeration
//**********************************************************************

typedef void* ClEnumTextHandle;

int ClBeginEnumObjectsText
(
  ClInstanceHandle hInstance,
  double          dblLatitude,
  double          dblLongitude,
  double          dblRange,
  ClEnumTextHandle    *phEnum
);

int ClEnumNextObjectText
(
  ClEnumTextHandle  hEnum,
  char          *psObjText,
  int           uiMaxTextLen
);

void ClEndEnumObjectsText
(
  ClEnumTextHandle  hEnum
);

//**********************************************************************
// Object Enumeration
//**********************************************************************

typedef void* ClEnumObjHandle;
typedef void* ClObjHandle;

/*
  Object types.
  The EnumObjectType can be used for the iObjType parameter in 
  ClBeginEnumObjects() and ClGetChartText().
*/
typedef enum EnumObjectTypeTag
{
  CHART_OBJECTS   = 3,
  CHART_TEXT      = 4,
  HIGHWAY_SHIELDS = 5,
  LAKE_LIST       = 100
} EnumObjectType;

CL_BOOL ClBeginEnumObjects
(
  ClInstanceHandle hInstance,
  int           iObjType,
  double        dblLatitude,
  double        dblLongitude,
  double        dblRange,
  double        dblScale,
  ClEnumObjHandle *phEnum
 );

int ClEnumNextObject
(
  ClEnumObjHandle hEnum,
  ClObjHandle *phObject
);

void ClEndEnumObjects
(
  ClEnumObjHandle hEnum
);

int ClGetChartText( ClEnumObjHandle hEnum, int iObjType, int *pX,int *pY, char *psText , int iMaxTextLen );
void ClSelectMap (char *pMapID);

//**********************************************************************
// Object Point Enumeration
//**********************************************************************
typedef void* ClEnumPointsHandle;

int ClGetObjectPoint
(
  unsigned long unObjId,
  int *piX,
  int *piY
);

int ClBeginEnumObjectPoints
(
  unsigned long unObjId,
  ClEnumPointsHandle  hEnum
);

int ClEnumNextObjectPoint
(
  ClEnumPointsHandle  hEnum,
  int *pX,
  int *pY
);

CL_BOOL ClIsEnumObjectPointsDone
(
  ClEnumPointsHandle  hEnum
);


void ClEndEnumObjectPoints
(
  ClEnumPointsHandle  hEnum
);

//**********************************************************************
// Icon Rendering
//**********************************************************************
int ClRenderIcon
(
  unsigned long unObjId,
  int x,
  int y,
  ClBitmap *pBitmap
);

//**********************************************************************
// Lake List
//**********************************************************************
int ClLakeListGetNumberOfRecords
(
  ClInstanceHandle hInstance
);

CL_BOOL ClLakeListIsRecordValid
(
  ClInstanceHandle hInstance,
  int iIndex
);

char *ClLakeListGetLakeName
(
  ClInstanceHandle hInstance,
  int iIndex
);

CL_BOOL ClLakeListIsHiDef
(
  ClInstanceHandle hInstance,
  int iIndex
);

char *ClLakeListGetHiDefCode
(
  ClInstanceHandle hInstance
);

char *ClLakeListGetCountyName
(
  ClInstanceHandle hInstance,
  int iIndex
);

char *ClLakeListGetType
(
  ClInstanceHandle hInstance,
  int iIndex
);

double ClLakeListGetLatitude
(
  ClInstanceHandle hInstance,
  int iIndex
);

double ClLakeListGetLongitude
(
  ClInstanceHandle hInstance,
  int iIndex
);

double ClLakeListGetWidthInMeters
(
  ClInstanceHandle hInstance,
  int iIndex
);

double ClLakeListGetHeightInMeters
(
  ClInstanceHandle hInstance,
  int iIndex
);

double ClLakeListGetAcres
(
  ClInstanceHandle hInstance,
  int iIndex
);


void* ClLKMPilotInit(ClInstanceHandle hInstance, double fLatitude, double fLongitude, double fRangeInMeters, 
                      int maxTrackPoints, double trackPointSpacing, char *pickText, int *pnazimuthFromStart, 
                      int *pndeeperFlag, int *pnuserPicked);

#endif // __CHART_LIB_H
