/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the Computer Systems Laboratory, University of
 * California, Santa Barbara and its contributors'' in the documentation
 * or other materials provided with the distribution and in all
 * advertising materials mentioning features or use of this software.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	DITHER_H
#define	DITHER_H

#define	DFLT_RANK	4		/* dither matrix default rank	 */

#define	MAX_RANK	16
#define	MIN_RANK	2

#define	BLACK		0
#define	WHITE		1

extern int         EXFUN( dm_val, (int rank, int row, int col));
extern void        EXFUN( dither, (int i_fd, int rank, int o_fd));
extern void        EXFUN( dithimg, (int i_fd, int nlines, int nsamps,
                                    int nbands, pixel_t ***dm, int rank,
                                    int o_fd));
extern pixel_t *** EXFUN( mk_dm, (int rank, int nbands, BIH_T **bihpp));
extern void        EXFUN( main, (int argc, char **argv));

/* $Header: /local/share/pkg/ipw/src/bin/dither/RCS/dither.h,v 1.2 90/11/11 17:02:02 frew Exp $ */
#endif
