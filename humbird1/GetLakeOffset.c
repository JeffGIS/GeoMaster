#include	<stdlib.h>
#if !defined(HUMMINBIRD)
#include	<malloc.h>
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	"lzoconf.h"
#include	"minilzo.h"
#include  "text.h"
#include "chart.h"
#include "ClMalloc.h"
#include "fs.h"
#include "ClAssert.h"
#include "ByteSwap.h"
#include "ChartLib.h"

#if defined(HUMMINBIRD)
#include "linux_fs.h"
#if !defined(HBIRDDEMO)
#define HBIRDDEMO 0
#endif
#endif

#define	SKIP_CID_CHECK	1

extern char	CIDValue[66], dek;

int GetLakeOffset (CLFILE *Fid,CL_BOOL useBufferedRead)
{
	int		rtn = -1;
	int		i, ln, j;
	struct {int version; int id; unsigned int marker;}FileFooter;
	unsigned char	xxx[256];
	char	tst[66];

	if (!Fid)
		return rtn;
	memset (tst,0,sizeof(tst));
	if (useBufferedRead)
		db_fseek (Fid,-(int)sizeof(FileFooter),SEEK_END);
	else
		cl_fseek (Fid,-(int)sizeof(FileFooter),SEEK_END);
	db_fread2 (&FileFooter,31,sizeof(FileFooter),Fid,useBufferedRead);
	swap32 (&FileFooter.version);
	swap32 (&FileFooter.id);
	swap32 (&FileFooter.marker);
	if (FileFooter.id == 323498251 && FileFooter.marker == 0xFFFFFFFF)
	{
		if (useBufferedRead)
			db_fseek (Fid,-(256 + (int)sizeof(FileFooter)),SEEK_END);
		else
			cl_fseek (Fid,-(256 + (int)sizeof(FileFooter)),SEEK_END);
		db_fread2 (xxx,32,256,Fid,useBufferedRead);
		j = xxx[0];
		ln = xxx[2] - 3;
		i = 0;
		while (ln--)
		{
			if (j > 253)
				j = 3;
			tst[ln] = ((xxx[j] - i++) / 8) + ((xxx[j+2] - ln) / 4) * 16;
			j += 3;
		}
		if (!strcmp (tst,CIDValue) || SKIP_CID_CHECK)
			rtn = 256 + (int)sizeof(FileFooter);
	}
	else if (SKIP_CID_CHECK)
		rtn = 0;
	if (useBufferedRead)
		db_fseek (Fid,0,SEEK_SET);
	else
		cl_fseek (Fid,0,SEEK_SET);
	return rtn;
}
