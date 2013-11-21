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

/*
** fpixel_ting point partial derivatives in x and y directions
*/

#include "ipw.h"
#include "fpio.h"

void
diffxy(n, delh, k0, k1, k2, dx, dy)
	REG_1 int       n;		/* # samps			 */
	fpixel_t       *delh;		/* grid spacing (1-line 2-samp)	 */
	REG_3 fpixel_t *k0;		/* top line of kernel		 */
	REG_2 fpixel_t *k1;		/* middle line of kernel	 */
	REG_4 fpixel_t *k2;		/* last line of kernel		 */
	fpixel_t       *dx;		/* partial f / partial x	 */
	fpixel_t       *dy;		/* partial f / partial y	 */
{
	FREG_1 fpixel_t xfactor;	/* 1 / (2 delta h)		 */
	FREG_2 fpixel_t yfactor;	/* 1 / (2 delta h)		 */

 /*
  * divide by 2 * delta h
  */
	xfactor = 1.0 / (2.0 * delh[1]);
	yfactor = 1.0 / (2.0 * delh[0]);
 /*
  * loop
  */
	while (--n >= 0) {
		*dx++ = ((*k2++) - (*k0++)) * xfactor;
		*dy++ = ((k1[1]) - (k1[-1])) * yfactor;
		++k1;
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/diffxy.c,v 1.4 90/11/11 17:03:10 frew Exp $";

#endif
