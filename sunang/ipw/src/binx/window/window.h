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

#ifndef	WINDOW_H
#define	WINDOW_H

/*
 * window specification: extrinsic (window, geodetic, ...) coordinates
 */
typedef struct {
	unsigned        flags;		/* bit flags (see below)	 */
	int             band;		/* extrinsic coords band #	 */

	double          bline;		/* begin line #			 */
	double          bsamp;		/* begin sample #		 */

	double          cline;		/* center line #		 */
	double          csamp;		/* center sample #		 */

	double          eline;		/* end line #		 	 */
	double          esamp;		/* end sample #		 	 */

	int             nlines;		/* # output lines		 */
	int             nsamps;		/* # output samples		 */
} XWSPEC_T;

/*
 * window specification: intrinsic (line,sample; 0-relative) coordinates
 */
typedef struct {
	unsigned        flags;		/* bit flags (see below)	 */

	int             bline;		/* begin line #			 */
	int             bsamp;		/* begin sample #		 */

	int             cline;		/* center line #		 */
	int             csamp;		/* center sample #		 */

	int             eline;		/* end line #		 	 */
	int             esamp;		/* end sample #		 	 */

	int             nlines;		/* # output lines		 */
	int             nsamps;		/* # output samples		 */
} WSPEC_T;

#define	GOT_BEGIN	bit(0)
#define	GOT_CENTER	bit(1)
#define	GOT_END		bit(2)
#define	GOT_SIZE	bit(3)
#define	GOT_XWIN	bit(4)
#define	GOT_XGEO	bit(5)

#define	SCR_PREFIX	"windo"

#if defined(WIN_H) && defined(GEO_H)
extern WSPEC_T *EXFUN(cvt_wspec, (XWSPEC_T *xwp, int nbands, WINH_T **winhpp,
			GEOH_T **geohpp));
#endif
extern int      EXFUN(fix_wspec, (WSPEC_T *wp, int nlines, int nsamps));
extern void     EXFUN(window, (int i_fd, XWSPEC_T *xwp, int o_fd));

/* $Header: /local/share/pkg/ipw/src/bin/window/RCS/window.h,v 1.2 90/11/11 17:10:53 frew Exp $ */
#endif
