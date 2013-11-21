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
 * trigonometric look-up table
 */

#include <math.h>
#include "ipw.h"
#include "bih.h"
#include "fpio.h"

#include "view.h"

void
trigtbl()
{
	fpixel_t      **smap;		/* slope/azm map		 */
	int             band;
	int             horbnd;		/* # horizon bands		 */
	int             j;
	int            *hmaplen;	/* length of horizon tbl	 */
	int            *smaplen;	/* length of slope tbl		 */

	smaplen = fpmaplen(parm.i_fds);
	hmaplen = fpmaplen(parm.i_fdh);
	horbnd = hnbands(parm.i_fdh);
	smap = fpmap(parm.i_fds);

 /*
  * cosines of slopes
  */
	for (j = smaplen[0] - 1; j >= 0; --j) {
		parm.cstbl[j] = sqrt((1 - parm.sstbl[j]) * (1 + parm.sstbl[j]));
	}
 /*
  * sines and values of horizon angles from zenith
  */
	for (band = 0; band < horbnd; ++band) {
		for (j = hmaplen[band] - 1; j >= 0; --j) {
			parm.sh2tbl[band][j] = (1 - parm.chtbl[band][j]) *
				(1 + parm.chtbl[band][j]);
			parm.hdtbl[band][j] = acos(parm.chtbl[band][j]) -
				sqrt(parm.sh2tbl[band][j]) *
				parm.chtbl[band][j];
		}
	}
 /*
  * cosines of differences between horizon azimuth & slope aspect
  */
	for (band = 0; band < horbnd; ++band) {
		for (j = smaplen[1] - 1; j >= 0; --j) {
			parm.cosdtbl[band][j] = cos(parm.hazm[band] -
						    smap[1][j]);
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/viewcalc/RCS/trigtbl.c,v 1.2 90/11/11 17:10:22 frew Exp $";

#endif
