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

typedef struct {
	int		force;		/* flag for force coincident	 */
	int             i_fd;		/* input image file descriptor	 */
	int             o_fd;		/* output image file descriptor	 */
	int             skip_lines;	/* line shrink factor		 */
	int             dup_lines;	/* line zoom factor		 */
	int             skip_samps;	/* sample shrink factor		 */
	int             dup_samps;	/* sample zoom factor		 */
	int             nbands;		/* # image bands		 */
	int             i_nlines;	/* # input lines		 */
	int             i_nsamps;	/* # samples / input line	 */
	int             o_nlines;	/* # output lines		 */
	int             o_nsamps;	/* # samples / output line	 */
	pixel_t        *buf;		/* -> image line buffer		 */
} PARM_T;

extern PARM_T   parm;

extern void     fixhdrs();
extern void     headers();
extern void     replicate();
extern void     subsamp();
extern void     zoom();

/* $Header: /local/share/pkg/ipw/src/bin/zoom/RCS/pgm.h,v 1.3 90/11/11 17:11:07 frew Exp $ */
#endif
