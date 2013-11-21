#ifndef	RASTERFILE_H
#define	RASTERFILE_H

#if defined(sun) && !CC_BIGENDIAN
#include <rasterfile.h>
#else

#if CC_BIGENDIAN
struct rasterfile {
	int             ras_magic;
	int             ras_width;
	int             ras_height;
	int             ras_depth;
	int             ras_length;
	int             ras_type;
	int             ras_maptype;
	int             ras_maplength;
};

#define	RAS_MAGIC	0x59a66a95

#define	RT_OLD		0
#define	RT_STANDARD	1
#define	RT_BYTE_ENCODED	2
#define	RT_EXPERIMENTAL	0xffff

#define	RMT_RAW		2
#define	RMT_NONE	0
#define	RMT_EQUAL_RGB	1

#else
Cant create a Sun rasterfile on a little-endian machine
#endif

/* ! little-endian sun */
#endif

/* $Header: /usr/home/ipw/src/bin/sunras/RCS/rasterfile.h,v 1.2 89/10/22 18:55:52 frew Exp $ */
#endif
