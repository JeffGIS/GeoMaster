#include "ipw.h"

#include "pgm.h"

/*
 * accum -- accumulate image sums
 *
 * This is a "generic" version that can be used as a starting point for
 * optimizing for a particular architecture.  An optimized accum() for Sun-3
 * machines (MC68020/MC68881) is in accum.fast.c.
 */

void
accum()
{
	REG_4 int       nbands = parm.i_nbands;
	REG_1 fpixel_t *pixel;		/* -> current pixel vector	 */
	REG_5 int       samp;		/* image sample #		 */

	pixel = parm.i_buf;

	for (samp = 0; samp < parm.i_nsamps; ++samp) {
		REG_3 int       band;	/* image band #			 */

		for (band = 0; band < nbands; ++band) {
			REG_2 int       band2;	/* cross-product band #	 */

			for (band2 = 0; band2 <= band; ++band2) {
				parm.sum_xy[band][band2] +=
					pixel[band] * pixel[band2];
			}

			parm.sum_x[band] += pixel[band];
		}

		pixel += nbands;
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: accum.slow.c,v 1.1 89/05/09 11:56:07 frew Exp $";

#endif
