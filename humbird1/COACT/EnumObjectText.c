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
//    EnumObjectText.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    04/27/2009
//
//  DESCRIPTION:
//    Enumerates text for an object on a chart.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include "ChartLib.h"
#include "Chart.h"
#include "lkmhbird.h"
#include "ClAssert.h"

//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    ClBeginEnumObjectsText
//
//  DESCRIPTION:	
//    Initializes the enumeration process.
//
//  Inputs:
//    ClInstanceHandle hInstance
//      Handle returned by CreateInstance.
//    double *pdblLatitude
//      Latitudinal coordinate of a point on a chart.
//    double *pdblLongitude
//      Longitudinal coordinate of a point on a chart.
//    double dblRange
//      The distance from the point at the specified latitude and longitude.
//
//  Outputs:
//    ClEnumTextHandle hEnum
//      A enumeration handle to be used by ClEnumNextObjectText,
//      ClEndEnumObjectsText, and ClIsEnumObjectsTextDone.
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClBeginEnumObjectsText
(
  ClInstanceHandle hInstance,
  double          dblLatitude,
  double          dblLongitude,
  double          dblRange,
  ClEnumTextHandle    *phEnum
)
{
  ChartInstance *pChart = CHART_HINST_2_PCHART(hInstance);
  int iUtmCenterX;
  int iUtmCenterY;
  BOOL bResult;

  ClAssert( NULL != hInstance );
  ClAssert( NULL != phEnum );

  GeoToBitmapCoord
  (
    dblLatitude,
    dblLongitude,
    &iUtmCenterX,
    &iUtmCenterY
   );

  bResult = LKMBeginEnumObjectsText
  (
    iUtmCenterX,
    iUtmCenterY,
    dblRange,
    pChart->dblActualScale,
    (HANDLE *)phEnum
  );
  
  return bResult ? CL_TRUE : CL_FALSE;
}


//**********************************************************************
//  FUNCTION:
//    ClEnumNextObjectText
//
//  DESCRIPTION:	
//    Retreives a string containing the description of the next chart 
//    object.
//
//  Inputs:
//    ClEnumTextHandle hEnum
//      Handle returned by ClBeginEnumObjectsText.
//    char *psObjText 
//      Pointer to a string that will be populated with a text string 
//      describing the next chart object.  The string must be long 
//      enough to include a null terminator.
//    int uiMaxTextLen
///     The number of characters including the null terminator that
//      can be stored in the string.
//
//  Outputs:
//    none
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClEnumNextObjectText
(
  ClEnumTextHandle  hEnum,
  char          *psObjText,
  int           uiMaxTextLen
)
{
  BOOL bResult;

  ClAssert( NULL != hEnum );
  ClAssert( NULL != psObjText );

  bResult = LKMEnumNextObjectText( (HANDLE)hEnum, psObjText , uiMaxTextLen - 1 );

  return bResult ? CL_TRUE : CL_FALSE;
}

//**********************************************************************
//  FUNCTION:
//    ClEndEnumObjectsText
//
//  DESCRIPTION:	
//    Finalizes the enumeration process.   ClEndEnumObjectsText must 
//    be called if ClBeginEnumObjectsText was called.
//
//  Inputs:
//    ClEnumTextHandle hEnum
//      Handle returned by ClBeginEnumObjectsText.
//
//  Outputs:
//    none
//
//  Returns:
//    none
//
//  GLOBALS:
//    none
//**********************************************************************
void ClEndEnumObjectsText
(
  ClEnumTextHandle  hEnum
)
{
  ClAssert( NULL != hEnum );

  LKMEndEnumObjectsText( (ClEnumTextHandle)hEnum );
}

