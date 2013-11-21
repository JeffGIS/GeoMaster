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
//    ByteSwap.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    6/28/2008
//
//  DESCRIPTION:
//    Functions for byte swapping.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include "types.h"
#include "fs.h"
#include "byteswap.h"
//#include "error.h"

//**********************************************************************
// Function prototypes
//**********************************************************************

//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    ByteSwapNeeded
//
//  Description:	
//    Checks so see that conversion from little endian to big endian
//    is required.
//
//  Inputs:
//    None.
//
//  Outputs:
//    None.
//
//  Returns:
//    CL_TRUE
//      Conversion is required.
//    CL_FALSE
//      Conversion is not required.
//
//  GLOBALS:
//    none
//
//**********************************************************************
CL_BOOL IsByteSwapNeeded()
{
   long one= 1;
   return (!(*((char *)(&one)))) ? CL_TRUE : CL_FALSE;
}

//**********************************************************************
//  FUNCTION:
//    swap16
//
//  Description:	
//    Bytes swaps a little endian 16 bit integer.
//    Swapping is only done if this routine detects a big endian 
//    processor.
//
//  Inputs:
//    short *val
//      The value to byte swap.
//
//  Outputs:
//    short *val
//      Returns the value in big endian format.
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
void swap16(short *val)
{
   unsigned char tempChar;
   union {
      unsigned char u8[2];
      short s16;
   } buf;

   if( IsByteSwapNeeded() )
   {
	  buf.s16 = *val;

    tempChar = buf.u8[1];
    buf.u8[1] = buf.u8[0];
    buf.u8[0] = tempChar;

	  *val = buf.s16;
   }
   return;
}

//**********************************************************************
//  FUNCTION:
//    swap32
//
//  Description:	
//    Bytes swaps a little endian 32 bit integer.
//    Swapping is only done if this routine detects a big endian 
//    processor.
//
//  Inputs:
//    long *val
//      The value to byte swap.
//
//  Outputs:
//    long *val
//      Returns the value in big endian format.
//
//  Returns:
//    None
//
//  GLOBALS:
//    none
//
//**********************************************************************
void swap32(long *lval)
{
   unsigned char tempChar;
   union {
      unsigned char u8[4];
      long s32;
   } buf;

   if( IsByteSwapNeeded() )
   {
	  buf.s32 = *lval;

    tempChar = buf.u8[3];
    buf.u8[3] = buf.u8[0];
    buf.u8[0] = tempChar;
    tempChar = buf.u8[2];
    buf.u8[2] = buf.u8[1];
    buf.u8[1] = tempChar;

	  *lval = buf.s32;
   }

}

//**********************************************************************
//  FUNCTION:
//    cl_freadS16
//
//  Description:	
//    Reads an 16 bit integer from a file and converts it to a big endian.
//    Swapping is only done if this routine detects a big endian 
//    processor.
//
//  Inputs:
//    CLFILE *pFID
//      The input file.
//
//  Outputs:
//    None.
//
//  Returns:
//    Returns a byte swapped 16 bit integer in the processors endian
//    format.
//
//  GLOBALS:
//    none
//
//**********************************************************************
short cl_freadS16(CLFILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 2;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[2];
      short s16;
   } buf;

   buf.s16 = 0;

   numBytesRead = db_fread (&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead)
   {
     WRITE_TO_ERROR_LOG2( clError_FileIo );
     return buf.s16;
   }

   if (IsByteSwapNeeded()) {
      tempChar = buf.u8[1];
      buf.u8[1] = buf.u8[0];
      buf.u8[0] = tempChar;
   }

   return buf.s16;
}

//**********************************************************************
//  FUNCTION:
//    cl_freadS32
//
//  Description:	
//    Reads an 32 bit integer from a file and converts it to a big endian.
//    Swapping is only done if this routine detects a big endian 
//    processor.
//
//  Inputs:
//    CLFILE *pFID
//      The input file.
//
//  Outputs:
//    None.
//
//  Returns:
//    Returns a byte swapped 32 bit integer in the processors endian
//    format.
//
//  GLOBALS:
//    none
//
//**********************************************************************
long cl_freadS32(CLFILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 4;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[4];
      long s32;
   } buf;
static	int	nCalls=0;

nCalls++;
   buf.s32 = 0;

   numBytesRead = db_fread (&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead)
   {
     WRITE_TO_ERROR_LOG2( clError_FileIo );
     return buf.s32;
   }

   if (IsByteSwapNeeded()) {
      tempChar = buf.u8[3];
      buf.u8[3] = buf.u8[0];
      buf.u8[0] = tempChar;
      tempChar = buf.u8[2];
      buf.u8[2] = buf.u8[1];
      buf.u8[1] = tempChar;
   }

   return buf.s32;
}

//**********************************************************************
//  FUNCTION:
//    cl_freadF64
//
//  Description:	
//    Reads an double from a file and converts it to a big endian.
//    Swapping is only done if this routine detects a big endian 
//    processor.
//
//  Inputs:
//    CLFILE *pFID
//      The input file.
//
//  Outputs:
//    None.
//
//  Returns:
//    Returns a byte swapped double in the processors endian
//    format.
//
//  GLOBALS:
//    none
//
//**********************************************************************
double cl_freadF64(CLFILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 8;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[8];
      double f64;
   } buf;

   buf.f64 = 0;

   numBytesRead = db_fread (&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead)
   {
      WRITE_TO_ERROR_LOG2( clError_FileIo );
      return buf.f64;
   }

   if (IsByteSwapNeeded()) {
      tempChar = buf.u8[7];
      buf.u8[7] = buf.u8[0];
      buf.u8[0] = tempChar;
      tempChar = buf.u8[6];
      buf.u8[6] = buf.u8[1];
      buf.u8[1] = tempChar;
      tempChar = buf.u8[5];
      buf.u8[5] = buf.u8[2];
      buf.u8[2] = tempChar;
      tempChar = buf.u8[4];
      buf.u8[4] = buf.u8[3];
      buf.u8[3] = tempChar;
   }

   return buf.f64;
}

