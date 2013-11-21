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
 * mcov -- print means + variance-covariance matrix
 */

void
mcov()
{
	int             class;		/* class #			 */
	int             band;		/* band #			 */

	for (class = 0; class < parm.c_nclasses; ++class) {
		if (parm.c_npixels[class] <= 0) {
			continue;
		}
 /*
  * magic cookie
  */
		(void) printf("#<stats>\n\n");
 /*
  * # bands
  */
		(void) printf("%d\n\n", parm.i_nbands);
 /*
  * mean vector
  */
		for (band = 0; band < parm.i_nbands; ++band) {
			(void) printf("%.*lg%s", DBL_DIG,
			     parm.sum_x[class][band] / parm.c_npixels[class],
				   band == parm.i_nbands - 1 ? "\n\n" : " ");
		}
/**
  * variance-covariance matrix.  Element [band1][band2] is:
  *
  *                      sum(band1) * sum(band2)
  * sum(band1 * band2) - -----------------------
  *                                 N
  * ---------------------------------------------
  *                       N
  */
		for (band = 0; band < parm.i_nbands; ++band) {
			int             band2;

			for (band2 = 0; band2 <= band; ++band2) {
				(void) printf("%.*lg%s", DBL_DIG,
					   (parm.sum_xy[class][band][band2] -
					    ((parm.sum_x[class][band] *
					      parm.sum_x[class][band2]) /
					     parm.c_npixels[class])) /
					      parm.c_npixels[class],
					      band2 == band ? "\n" : " ");
			}
		}

		(void) printf("\n");

		(void) printf("* %d\n\n", class);

		(void) printf("%d\n\n", parm.c_npixels[class]);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mstats/RCS/mcov.c,v 1.2 90/11/11 17:06:56 frew Exp $";

#endif
