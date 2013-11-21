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
** orchestrate computation of horizon
**/

#include <math.h>
#include "ipw.h"
#include "bih.h"
#include "pixio.h"
#include "fpio.h"

#include "horizon.h"

void
DEFUN_VOID(horizon)
{
	fpixel_t        fspace;		/* (fpixel_t)spacing		 */
	fpixel_t        thresh = 0;	/* threshold for mask		 */
	fpixel_t       *hcos = NULL;	/* horizon cosines 		 */
	fpixel_t       *zbuf;		/* elevation buffer		 */
	int             ngot;		/* # pixels read		 */
	int             npix;		/* # pixels in row		 */
	int            *hbuf;		/* horizon index		 */
	pixel_t        *hmask = NULL;	/* output mask			 */
	int  EXFUN( (*horfun), (int n, fpixel_t *z, int *h) );
					/* forward or backward horz fun	 */

 /*
  * initialize buffers for i/o and horizon computation
  */
	npix = hnsamps(parm.i_fd);
 /* NOSTRICT */
	hbuf = (int *) ecalloc(npix, sizeof(int));
 /* NOSTRICT */
	zbuf = (fpixel_t *) ecalloc(npix, sizeof(fpixel_t));
	if (hbuf == NULL || zbuf == NULL)
		bug("buffer allocation");
 /*
  * if horizon cosines to be computed, need to allocate storage vector
  */
	if (parm.nbits > 1) {
 /* NOSTRICT */
		hcos = (fpixel_t *) ecalloc(npix, sizeof(fpixel_t));
		if (hcos == NULL)
			bug("buffer allocation");
	}
 /*
  * otherwise if mask to be computed, need to allocate integer storage
  * vector and set threshold
  */
	else {
 /* NOSTRICT */
		hmask = (pixel_t *) ecalloc(npix, sizeof(pixel_t));
		if (hmask == NULL)
			bug("buffer allocation");
		thresh = tan(PI / 2 - parm.zenith);
	}
 /*
  * forward or backward horizon function
  */
	horfun = (parm.backward) ? hor1b : hor1f;
 /*
  * main loop
  */
	fspace = parm.spacing;
	while ((ngot = fpvread(parm.i_fd, zbuf, npix)) > 0) {
		if (ngot != npix)
			error("premature end of row");
 /*
  * find points that form horizons
  */
		(void) (*horfun) (ngot, zbuf, hbuf);
 /*
  * if not mask output, compute and write horizons along each row
  */
		if (parm.nbits > 1) {
			horval(ngot, zbuf, fspace, hbuf, hcos);
			if (fpvwrite(parm.o_fd, hcos, ngot) != ngot) {
				error("fpvwrite error");
			}
		}
 /*
  * if mask output, set mask and write
  */
		else {
			hormask(ngot, zbuf, fspace, hbuf, thresh, hmask);
			if (pvwrite(parm.o_fd, hmask, ngot) != ngot) {
				error("pvwrite error");
			}
		}
	}

	SAFE_FREE(zbuf);
	SAFE_FREE(hbuf);
	if (parm.nbits > 1) {
		SAFE_FREE(hcos);
	} else {
		SAFE_FREE(hmask);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/horizon.c,v 1.4 90/11/11 17:04:18 frew Exp $";

#endif
