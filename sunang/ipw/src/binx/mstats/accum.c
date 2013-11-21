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
 * accum -- accumulate image sums
 */

void
accum()
{
	pixel_t        *c_buf = parm.c_buf;
	int            *c_npixels = parm.c_npixels;
	int             nbands = parm.i_nbands;
	REG_1 fpixel_t *pixel = parm.i_buf;
	int             samp;
	double        **sum_x = parm.sum_x;
	double       ***sum_xy = parm.sum_xy;

	for (samp = 0; samp < parm.nsamps; ++samp) {
		int             band;
		REG_4 int       class = *c_buf++;
		REG_5 double   *c_sum_x;
		REG_6 double  **c_sum_xy;

		++c_npixels[class];

		c_sum_x = sum_x[class];
		c_sum_xy = sum_xy[class];

		for (band = 0; band < nbands; ++band) {
			REG_2 int       band2;
			FREG_1 fpixel_t p_band = pixel[band];
			REG_3 double   *band_c_sum_xy = *c_sum_xy++;

			for (band2 = 0; band2 <= band; ++band2) {
				*band_c_sum_xy++ += p_band * pixel[band2];
			}

			*c_sum_x++ += p_band;
		}

		pixel += nbands;
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mstats/RCS/accum.c,v 1.2 90/11/11 17:06:47 frew Exp $";

#endif
