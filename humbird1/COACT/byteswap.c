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
#include "error.h"
#include <stdlib.h>
#include <string.h>

int BufferedRead (CLFILE *Fid,int iFile,int lRead,unsigned char	*pData);
extern	int	CurrentOpenFile;
extern	int	FileLength[33];
extern	int	FilePos[33];

static	char	openFile[128]="";
static	int		nOpenFiles=0;

extern int ipcptime,iptltime,ipdeptime,numseek,numreads,totread,actualRead,totreadtime,maxseektime,numopen,maxopentime;
//**********************************************************************
// Function prototypes
//**********************************************************************

//**********************************************************************
// Functions
//**********************************************************************

//**********************************************************************
//  FUNCTION:
//    cl_ByteSwapNeeded
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
CL_BOOL cl_ByteSwapNeeded()
{
   long one= 1;
   return (!(*((char *)(&one)))) ? CL_TRUE : CL_FALSE;
}

CLFILE *db_fopen( const char *filename, const char *mode )
{
	CLFILE *fid;
	int	StartTime = GetClockTicks();
	int	ii;

	if (*openFile)
		ii=1;
	fid = cl_fopen (filename, mode);

  if( NULL == fid )
  {
	int ier;
	char	path[128];
	_get_doserrno (&ier);
	_fullpath (path,"",128);
    WRITE_TO_ERROR_LOG2( clError_FileIo );
    WRITE_TO_ERROR_LOG( (char *) filename );
  }
  else
  {
	  nOpenFiles++;
	  strcpy (openFile,filename);
  }
	maxopentime = CL_MAX (maxopentime,GetClockTicks()-StartTime);
	numopen++;

	return fid;
}
int db_ftell (CLFILE * fid)
{
	if (CurrentOpenFile >= 0)
		return FilePos[CurrentOpenFile] ;
	return -1;
}

int db_fclose (CLFILE * fid)
{
	*openFile = 0;
	nOpenFiles--;
	return cl_fclose (fid);
}

int	db_fseek (CLFILE * fid,int loc,int pos)
{
	int rtn;
	int	StartTime = GetClockTicks();

	if (CurrentOpenFile >= 0)
	{
		switch (pos)
		{
			case SEEK_END:
				FilePos[CurrentOpenFile] = loc + FileLength[CurrentOpenFile];
				break;
			case SEEK_CUR:
				break;
			case SEEK_SET:
				FilePos[CurrentOpenFile] = loc;
				break;
		}
		return 0;
	}
	rtn = cl_fseek (fid,loc,pos);

  if( rtn )
    WRITE_TO_ERROR_LOG2( clError_FileIo );

	maxseektime = CL_MAX (maxseektime,GetClockTicks()-StartTime);
	numseek++;
	return rtn;
}
 
int db_fread(void * pbuf, int size,int count, CLFILE * fid)
{
	int rtn;
	int	StartTime = GetClockTicks();
	extern	int	readloc[36];
	extern	int	readtim[36];
	extern	int	filetim[32];
	extern	int	filecnt[32];
	int		tim;

	readloc[size]++;
	rtn = BufferedRead (fid,CurrentOpenFile,count,pbuf);
	numreads++;
	totread += count;
	tim = (GetClockTicks()-StartTime);
	totreadtime += tim;
	readtim[size]+=tim;
	filetim[CurrentOpenFile]+=tim;
	filecnt[CurrentOpenFile]+=count;

	return rtn;
}

//**********************************************************************
//  FUNCTION:
//    ByteSwapInt
//
//  Description:	
//    Converts an little endian integer to a big endian integer.
//    Swapping is only done if this routine detects a big endian 
//    processor.
//
//  Inputs:
//    int i
//      The value to byte swap.
//
//  Outputs:
//    None.
//
//  Returns:
//    Returns a byte swapped integer if a big endian procesor was detected.
//
//  GLOBALS:
//    none
//
//**********************************************************************
int ByteSwapInt( int i )
{
  char temp;
  char *c = (char *)&i;

  if( cl_ByteSwapNeeded() )
  {
    temp = c[0];
    c[0] = c[3];
    c[3] = temp;

    temp = c[1];
    c[1] = c[2];
    c[2] = temp;
  }

  return i;
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

   if (cl_ByteSwapNeeded()) {
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

   if (cl_ByteSwapNeeded()) {
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

   if (cl_ByteSwapNeeded()) {
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

