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

#include "ipw.h"

#include "pgm.h"

/*
 * init -- initialize dynamic data structures
 */

void
init()
{
 /* NOSTRICT */
	parm.i_buf = (fpixel_t *) ecalloc(parm.nsamps * parm.i_nbands,
					  sizeof(fpixel_t));
	if (parm.i_buf == NULL) {
		error("can't allocate input buffer");
	}

 /* NOSTRICT */
	parm.c_buf = (pixel_t *) ecalloc(parm.nsamps, sizeof(pixel_t));
	if (parm.c_buf == NULL) {
		error("can't allocate input buffer");
	}

 /* NOSTRICT */
	parm.c_npixels = (int *) ecalloc(parm.c_nclasses, sizeof(int));
	if (parm.c_npixels == NULL) {
		error("can't allocate pixel tally buffer");
	}

 /* NOSTRICT */
	parm.sum_x = (double **) allocnd(sizeof(double), 2,
					 parm.c_nclasses, parm.i_nbands);
	if (parm.sum_x == NULL) {
		error("can't allocate sum-of-band buffer");
	}
 /*
  * NB:  Only the lower triangle of the array parm.sum_xy is used.  If space
  * becomes a problem for images with humongous # bands, then we may want to
  * store this array in lower-triangular form ...
  */
 /* NOSTRICT */
	parm.sum_xy = (double ***) allocnd(sizeof(double), 3,
					   parm.c_nclasses,
					   parm.i_nbands, parm.i_nbands);
	if (parm.sum_xy == NULL) {
		error("can't allocate sum-of-band*band buffer");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mstats/RCS/init.c,v 1.2 90/11/11 17:06:51 frew Exp $";

#endif
