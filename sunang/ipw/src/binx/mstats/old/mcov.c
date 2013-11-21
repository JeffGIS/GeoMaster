#include "ipw.h"

#include "pgm.h"

/*
 * mcov -- print means + variance-covariance matrix
 */

void
mcov()
{
	int             band;		/* band #			*/

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
			      parm.sum_x[band] / parm.i_npixels,
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
				      (parm.sum_xy[band][band2] -
				       ((parm.sum_x[band] *
					 parm.sum_x[band2]) /
					parm.i_npixels)) /
				      parm.i_npixels,
				      band2 == band ? "\n" : " ");
		}
	}

	(void) printf("\n");
 /*
  * stuff for future support of per-class statistics
  */
	(void) printf("* 0\n\n");

	(void) printf("%d\n\n", parm.i_npixels);
}

#ifndef	lint
static char     rcsid[] = "$Header: mcov.c,v 1.1 89/05/09 11:56:10 frew Exp $";

#endif
