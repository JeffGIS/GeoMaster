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
//    EnumObjects.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    04/27/2009
//
//  DESCRIPTION:
//    Enumerates objects on a chart.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include "ChartLib.h"

//**********************************************************************
// Pseudo code for enumerating objects.
//**********************************************************************
/*
BeginEnumObjects( instanceHandle, scale, latCoord, longCoord, range, &enumHandle );
while( !IsEnumObjectsDone( enumHandle ) )
{
	EnumNextObject( enumHandle, &objectId, &objectType );
ProcessChartObject( &objectId, &objectType );
}

EndEnumObjects( enumHandle );

*/
//**********************************************************************

//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    ClBeginEnumObjects
//
//  DESCRIPTION:	
//    Initializes the enumeration process.
//
//  Inputs:
//    ClInstanceHandle hInstance
//      Handle returned by ClCreateInstance.
//    double *punScale
//      The scale will be measured in pixel’s per meter.  The routine
//      will search for a chart that is closest to the scale.
//    double *pdblLatitude
//      Latitudinal coordinate of a point on a chart.
//    double *pdblLongitude
//      Longitudinal coordinate of a point on a chart.
//    unsigned uiRange
//      The distance from the point at the specified latitude and longitude.
//
//  Outputs:
//    ClEnumObjHandle hEnum
//      A enumeration handle to be used by ClEnumNextObject, 
//      ClEndEnumObjects, and ClIsEnumObjectsDones.
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClBeginEnumObjects
(
  ClInstanceHandle hInstance,
  double        *punScale,
  double        *pdblLatitude,
  double        *pdblLongitude,
  unsigned      uiRange,
  ClEnumObjHandle    hEnum
 )
{
  return 0;
}

//**********************************************************************
//  FUNCTION:
//    ClEnumNextObject
//
//  DESCRIPTION:	
//    Retreives the next chart object.
//
//  Inputs:
//    Enumeration Handle
//      Handle returned by ClBeginEnumObjects.
//
//  Outputs:
//    unsigned long *punObjId
//      Used internally by chart library to uniquely identify an object
//      and manage storage.  The application can pass the object id to
//      GetObjectPoint or BeginEnumObjectPoints.
//    unsigned char *punObjType
//      Land area, land boundary, and navaid
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClEnumNextObject
(
  ClEnumObjHandle hEnum,
  unsigned long *punObjId,
  unsigned char *punObjType
)
{
  return 0;
}

//**********************************************************************
//  FUNCTION:
//    ClIsEnumObjectsDone
//
//  DESCRIPTION:	
//    Checks that all objects have been enumerated.
//
//  Inputs:
//    Enumeration Handle
//      Handle returned by ClBeginEnumObjects.
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
CL_BOOL ClIsEnumObjectsDone
(
  ClEnumObjHandle hEnum
)
{
  return CL_TRUE;
}

//**********************************************************************
//  FUNCTION:
//    ClEndEnumObjects
//
//  DESCRIPTION:	
//    CFinalizes the enumeration process.  EndEnumObjects must be called
//    if BeginEnumObjects was called.
//
//  Inputs:
//    Enumeration Handle
//      Handle returned by ClBeginEnumObjects.
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
void ClEndEnumObjects
(
 ClEnumObjHandle hEnum
)
{
  return;
}
