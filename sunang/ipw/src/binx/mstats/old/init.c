#include "ipw.h"

#include "pgm.h"

/*
 * init -- initialize dynamic data structures
 */

void
init()
{
	parm.i_npixels = parm.i_nlines * parm.i_nsamps;

 /* NOSTRICT */
	parm.i_buf = (fpixel_t *) ecalloc(parm.i_nsamps * parm.i_nbands,
					  sizeof(fpixel_t));
	if (parm.i_buf == NULL) {
		error("can't allocate input buffer");
	}

 /* NOSTRICT */
	parm.sum_x = (double *) ecalloc(sizeof(double), parm.i_nbands);
	if (parm.sum_x == NULL) {
		error("can't allocate sum-of-band buffer");
	}
 /*
  * NB:  Only the lower triangle of the array parm.sum_xy is used.  If space
  * becomes a problem for images with humongous # bands, then we may want to
  * store this array in lower-triangular form ...
  */
 /* NOSTRICT */
	parm.sum_xy = (double **) allocnd(sizeof(double), 2,
					  parm.i_nbands, parm.i_nbands);
	if (parm.sum_xy == NULL) {
		error("can't allocate sum-of-band*band buffer");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: init.c,v 1.1 89/05/09 11:56:09 frew Exp $";

#endif
