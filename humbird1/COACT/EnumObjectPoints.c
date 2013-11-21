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
//    EnumObjectPoints.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    04/27/2009
//
//  DESCRIPTION:
//    Enumerates points for an object on a chart.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include "ChartLib.h"

//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    ClGetObjectPoint
//
//  DESCRIPTION:	
//    Get the object’s first point.  This is used for single point 
//    objects such as navaids.
//
//  Inputs:
//    unsigned long unObjId
//      Object ID returned by ClEnumNextObject.

//  Outputs:
//    int x
//      The X coordinate of the object’s point.
//    int y
//      The Y coordinate of the object’s point.
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClGetObjectPoint
(
  unsigned long unObjId,
  int *piX,
  int *piY

)
{
  return 0;
}


//**********************************************************************
//  FUNCTION:
//    ClBeginEnumObjectPoints
//
//  DESCRIPTION:	
//    Initializes the enumeration process.
//
//  Inputs:
//    unsigned long unObjId
//      Object ID returned by ClEnumNextObject.
//
//  Outputs:
//    ClEnumPointsHandle hEnum
//      A enumeration handle to be used by ClEnumNextObjectPoint, 
//      ClEndEnumObjectPoints, and ClIsEnumObjectPointsDone.
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClBeginEnumObjectPoints
(
  unsigned long unObjId,
  ClEnumPointsHandle  hEnum
)
{
  return 0;
}

//**********************************************************************
//  FUNCTION:
//    ClEnumNextObjectPoint
//
//  DESCRIPTION:	
//    Retreives the object’s next point.
//
//  Inputs:
//    ClEnumPointsHandle  hEnum
//      Handle returned by ClBeginEnumObjectPoints.
//    int *pX
//      The X coordinate of the next point in the object in pixel
//      coordinates.
//    int *pY
//      The Y coordinate of the next point in the object in pixel
//      coordinates.
//
//  Outputs:
//    ClEnumPointsHandle hEnum
//      A enumeration handle to be used by ClEnumNextObjectPoint, 
//      ClEndEnumObjectPoints, and ClIsEnumObjectPointsDone.
//
//  Returns:
//    Returns zero on success and non-zero on failure.
//
//  GLOBALS:
//    none
//**********************************************************************
int ClEnumNextObjectPoint
(
  ClEnumPointsHandle  hEnum,
  int *pX,
  int *pY
)
{
  return 0;
}

//**********************************************************************
//  FUNCTION:
//    ClIsEnumObjectPointsDone
//
//  DESCRIPTION:	
//    Checks that all points have been enumerated.
//
//  Inputs:
//    ClEnumPointsHandle  hEnum
//      Handle returned by ClBeginEnumObjectPoints.
//    int *pX
//      The X coordinate of the next point in the object in pixel
//      coordinates.
//    int *pY
//      The Y coordinate of the next point in the object in pixel
//      coordinates.
//
//  Outputs:
//    none
//
//  Returns:
//    Returns true when all points have been enumerated, otherwise
//    returns false.
//
//  GLOBALS:
//    none
//**********************************************************************
CL_BOOL ClIsEnumObjectPointsDone
(
  ClEnumPointsHandle  hEnum
)
{
  return CL_TRUE;
}

//**********************************************************************
//  FUNCTION:
//    ClEndEnumObjectPoints
//
//  DESCRIPTION:	
//    Finalizes the enumeration process.  ClEndEnumObjectPoints must be
//    called if ClBeginEnumObjectsPoints was called.
//
//  Inputs:
//    ClEnumPointsHandle  hEnum
//      Handle returned by ClBeginEnumObjectPoints.
//    int *pX
//      The X coordinate of the next point in the object in pixel
//      coordinates.
//    int *pY
//      The Y coordinate of the next point in the object in pixel
//      coordinates.
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
void ClEndEnumObjectPoints
(
  ClEnumPointsHandle  hEnum
)
{
  return;
}
