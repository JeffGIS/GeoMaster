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
  * allocate buffers returns number of possible values of costbl, etc
  */

#include "ipw.h"
#include "bih.h"
#include "fpio.h"
#include "shade.h"

void
buffers(fdi)
	int             fdi;		/* input file desc		 */
{
	int             nwrite;		/* # pixels to write		 */
	int             nbands;		/* # input bands		 */
	int            *maplen;		/* lengths of fp maps		 */
	fpixel_t      **map;		/* f.p. conversion map		 */

	maplen = fpmaplen(fdi);
	map = fpmap(fdi);
	nwrite = nget = hnsamps(fdi);
	nbands = hnbands(fdi);

 /* NOSTRICT */
	ibuf = (pixel_t *) ecalloc(nbands * nget, sizeof(pixel_t));
	if (ibuf == NULL) {
		error("can't allocate input buffer");
	}

 /* NOSTRICT */
	obuf = (pixel_t *) ecalloc(nwrite, sizeof(pixel_t));
	if (obuf == NULL) {
		error("can't allocate output buffer");
	}
 /*
  * matrix to hold values already computed (dimensions are # poss
  * slopes by # poss azimuths)
  */
 /* NOSTRICT */
	shade = (pixel_t **) allocnd(sizeof(pixel_t), 2, maplen[0], maplen[1]);
	if (shade == NULL) {
		error("can't allocate buffer for computed values");
	}
 /*
  * buffers to hold sines, cosines, and cosines of differences (sines &
  * cosines for each poss slope, cosines of diffs for each poss
  * azimuth)
  */
	sintbl = map[0];
 /* NOSTRICT */
	costbl = (float *) ecalloc(maplen[0], sizeof(float));
 /* NOSTRICT */
	cosdtbl = (float *) ecalloc(maplen[1], sizeof(float));
	if (sintbl == NULL || costbl == NULL || cosdtbl == NULL) {
		error("can't allocate buffers for trig tables");
	}
}

#ifndef lint
static char	rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/shade/RCS/buffers.c,v 1.6 90/11/11 17:08:36 frew Exp $";

#endif
