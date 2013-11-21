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

#include "getargs.h"
#include "fpio.h"
#include "pixio.h"

/*
 * header file for "lincom" program
 */

typedef struct {
	areal_t        *coef;		/* -> coefficients		 */
	areal_t        *const;		/* -> constants			 */
	char           *tmpnam;		/* name of scratch file		 */
	fpixel_t       *mmval;		/* min, max of output vector	 */
	fpixel_t     ***ftbl;		/* lookup tables for fp conv	 */
	int             i_fd;		/* input image file descriptor	 */
	int             nbits;		/* # output bits per pixel	 */
	int             ncoef;		/* # coefficients		 */
	int             nconst;		/* # constants			 */
	int             o_fd;		/* output image file descriptor	 */
}               PARM_T;

extern PARM_T   parm;

extern void     coeffs();
extern void     headers();
extern void     lincom();
extern void     output();
extern fpixel_t ***re_map();

/* $Header: /local/share/pkg/ipw/src/bin/lincom/JUD/RCS/pgm.h,v 1.2 90/11/11 17:05:16 frew Exp $ */
#endif
