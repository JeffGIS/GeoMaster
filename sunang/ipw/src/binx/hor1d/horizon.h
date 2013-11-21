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

#include "bih.h"
#include "lqh.h"

/*
 * header file for "hor1d" program
 */

typedef struct {
	bool_t          backward;	/* ? backward direction		 */
	double          azimuth;	/* azimuth (radians)		 */
	double          spacing;	/* elev grid spacing same units	 */
	double          zenith;		/* solar zenith angle (radians)	 */
	int             i_fd;		/* input image file descriptor	 */
	int             nbits;		/* # bits in output pixel	 */
	int             o_fd;		/* output image file descriptor	 */
} PARM_T;

extern PARM_T   parm;

extern bool_t   EXFUN(bihvalid, (BIH_T **bihpp, int nb));
extern double   EXFUN(azmf, (double azd));
extern double   EXFUN(zenf, (double zend));
extern int	EXFUN(hor1b, (int n, fpixel_t *z, int *h));
extern int	EXFUN(hor1f, (int n, fpixel_t *z, int *h));
extern void	EXFUN(hormask, (int n, fpixel_t *z, fpixel_t delta, int *h,
			fpixel_t thresh, pixel_t *hmask));
extern void	EXFUN(horval, (int n, fpixel_t *z, fpixel_t delta, int *h,
			fpixel_t *hcos));
extern void     EXFUN(headers, (void));
extern void     EXFUN(horizon, (void));
extern LQH_T  **EXFUN(newlqh, (int fdo));

#define NBANDS	1

/* $Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/horizon.h,v 1.2 90/11/11 17:04:20 frew Exp $ */
#endif
