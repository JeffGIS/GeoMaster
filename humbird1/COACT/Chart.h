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
//    Chart.h
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
#ifndef __CHART_H
#define __CHART_H


#include "types.h"
#include "Bitmap.h"
#include "Text.h"
#include "fs.h"


#define CHART_HINST_2_PCHART( hInstance ) \
  (ChartInstance *)(hInstance)
#define PCHART_2_CHART_HINST( pChart ) \
  (ClInstanceHandle *)(pChart)


typedef struct LakeListItemTag {
  char *psName;
  char *psCounty;
  double fLatitude;
  double fLongitude;
  int iWidth;
  int iHeight;
  int iAcres;
  CL_BOOL bProMap;
} LakeListItem;


typedef struct LakeListTag {
  CL_BOOL bIsLoaded;
  int iNumRecords;
  char *pData;
  LakeListItem *pRecord;
} LakeList;


typedef struct ChartInstanceTag {
  char sPath[CL_MAX_PATH];
  Bitmap bm;
  int iUtmX;
  int iUtmY;
  int iUtmZone;
  CL_BOOL bIsRendered;
  double dblActualScale;
  ClRenderChartParams lastParams;
  TextImages images;
  LakeList lakeList;
} ChartInstance;


ClError GeoToBitmapCoord
(
  double dblLattitude,
  double dblLongitude,
  int iUtmZone,
  int *piX,
  int *piY
);


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
);

void HBDisplayBitmap
(
  HANDLE HBImageHandle,
  char *psBitmapPathName,
  int iPercentSize,
  int iUtmCenterX,
  int iUtmCenterY
);

CL_BOOL ClLakeListLoad(
  ChartInstance *pChart
);

#endif //  __CHART_H
