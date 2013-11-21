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

#ifndef PGM_H
#define PGM_H

#include "geoh.h"
#include "lqh.h"

#ifdef BIH_H
extern bool_t   EXFUN(bihvalid, (BIH_T **bih, int nb));
#endif
extern void	EXFUN(caspect, (int n, fpixel_t *dx, fpixel_t *dy, fpixel_t *a));
extern void	EXFUN(cslope, (int n, fpixel_t *dx, fpixel_t *dy, fpixel_t *s));
extern void	EXFUN(diffxy, (int n, fpixel_t *delh, fpixel_t *k0,
			fpixel_t *k1, fpixel_t *k2, fpixel_t *dx, 
			fpixel_t *dy));
extern void	EXFUN(fillends, (int n, fpixel_t *k));
extern void	EXFUN(fillstart, (int n, fpixel_t *k0, fpixel_t *k1,
			fpixel_t *k2));
extern void	EXFUN(gradient, (int fdi, int fdo, bool_t slope, bool_t aspect,
			fpixel_t *spacing));
extern void	EXFUN(gradu, (int nsamps, fpixel_t *delh, bool_t dos,
			bool_t doa, fpixel_t **ibuf, fpixel_t *dx,
			fpixel_t *dy, fpixel_t *s, fpixel_t *a,
			fpixel_t *obuf));
extern void	EXFUN(headers, (int fdi, int fdo, bool_t slope, bool_t aspect,
			int *nbits, fpixel_t *spacing));
extern GEOH_T **EXFUN(newgeoh, (int nbands, int fdo, GEOH_T **i_geoh,
			 fpixel_t *spacing));
extern LQH_T **	EXFUN(newlqh, (int fdo, bool_t slope, bool_t aspect));
extern void	EXFUN(shuffle, (int n, fpixel_t *s, fpixel_t *a, fpixel_t *o));
#ifdef GETARGS_H
extern void	EXFUN(options, (OPTION_T *opt_s, OPTION_T *opt_a,
			OPTION_T *opt_d, OPTION_T *opt_i, bool_t *s, 
			bool_t *a, int *nbits, fpixel_t *spacing));
#endif

/* $Header: /local/share/pkg/ipw/src/bin/gradient/RCS/pgm.h,v 1.2 90/11/11 17:03:36 frew Exp $ */
#endif
