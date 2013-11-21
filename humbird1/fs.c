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
//    fs.c
//
//	WRITTEN BY:
//    Shawn Wiltz
//
//	DATE:
//    09/15/2009
//
//  DESCRIPTION:
//    File system functions
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "ChartLib.h"
#include "fs.h"
#include "error.h"

//**********************************************************************
// External Globals
//**********************************************************************
extern int ipcptime,iptltime,ipdeptime,numseek,numreads,totread,actualRead,totreadtime,maxseektime,numopen,maxopentime;
extern	int	CurrentOpenFile;
extern	int	FileLength[33];
extern	int	FilePos[33];

static	char	openFile[128]="";
static	int		nOpenFiles=0;

//**********************************************************************
// Prototypes
//**********************************************************************
extern int GetClockTicks (void);

//**********************************************************************
// Functions
//**********************************************************************

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
	else
		return cl_ftell (fid);
	return -1;
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
int db_fclose (CLFILE * fid)
{
	*openFile = 0;
	nOpenFiles--;
	CurrentOpenFile = -1;
	return cl_fclose (fid);
}
