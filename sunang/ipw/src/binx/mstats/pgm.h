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

#ifndef	PGM_H
#define	PGM_H

/*
 * header file for "mstats" program
 */

typedef struct {
	int             i_fd;		/* input image file descriptor	 */
	int             nlines;		/* #lines / image		 */
	int             nsamps;		/* #samples / line	 	 */
	int             i_nbands;	/* #bands / input sample	 */
	fpixel_t       *i_buf;		/* -> input line buffer		 */
	double        **sum_x;		/* sum[class][band]		 */
	double       ***sum_xy;		/* sum[class][band][band]	 */
	pixel_t        *c_buf;		/* -> class line buffer		 */
	int            *c_npixels;	/* #pixels[class]		 */
	int             c_fd;		/* class image file descriptor	 */
	int             c_nclasses;	/* #classes			 */
} PARM_T;

extern PARM_T   parm;

extern void     accum();
extern void     headers();
extern void     init();
extern void     mcov();
extern void     mstats();

/* $Header: /local/share/pkg/ipw/src/bin/mstats/RCS/pgm.h,v 1.2 90/11/11 17:07:03 frew Exp $ */
#endif
