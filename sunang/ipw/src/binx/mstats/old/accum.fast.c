#include "ipw.h"

#include "pgm.h"

/*
 * accum -- accumulate pixel sums and cross-products
 *
 * NOTE: this version of accum() has been optimized for an MC68020/MC68881
 *       architecture.  A generic version is provided in the file accum.slow.c
 *       in case these optimizations are inappropriate for other systems.
 */

void
accum()
{
	register fpixel_t *buf = parm.i_buf;
	register fpixel_t *pixel;
	register fpixel_t *pixel2;
	register double **sum_xy = parm.sum_xy;

	register int    band;
	register int    band2;
	register int    nsamps = parm.i_nsamps;
	register int    nbands = parm.i_nbands;
	register int    samp;

 /*
  * This is important -- the intermediate sums MUST be computed in double
  * precision (even if the "register" is ignored).  An earlier version of this
  * program screwed up badly due to rounding errors.  Ya been warned ...
  */
	register double sum;

 /*
  * We assume that there will usually be more samples than bands, therefore we
  * structure things so the sample loops are the innermost and are as tight as
  * possible.
  */
	band = nbands - 1;
	do {
 /*
  * accumulate sum-of-pixels for current band
  */
		pixel = &buf[band];
		sum = 0;

		samp = nsamps;
		do {
			sum += *pixel;
			pixel += nbands;
		} while (--samp > 0);

		parm.sum_x[band] += sum;
 /*
  * accumulate sum of squares and cross-products for current band
  */
		band2 = band;
		do {
			pixel = &buf[band];
			pixel2 = &buf[band2];
			sum = 0;

			samp = nsamps;
			do {
				sum += *pixel * *pixel2;
				pixel += nbands;
				pixel2 += nbands;
			} while (--samp > 0);

			sum_xy[band][band2] += sum;
		} while (--band2 >= 0);
	} while (--band >= 0);
}

#ifndef	lint
static char     rcsid[] = "$Header: accum.fast.c,v 1.1 89/05/09 11:56:07 frew Exp $";

#endif
