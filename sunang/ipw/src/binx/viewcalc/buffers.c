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
 * allocate buffers
 */

#include "ipw.h"
#include "bih.h"
#include "fpio.h"
#include "view.h"

void
buffers()
{
	int             npix;		/* # pixels to read/write	 */
	int             horbnd;		/* # horizon bands		 */
	int            *smaplen;	/* length of slope map		 */
	int            *hmaplen;	/* length of horizon map	 */
	fpixel_t      **map;		/* slope map			 */
	fpixel_t      **hmap;		/* horizon map			 */
	int             j;

	smaplen = fpmaplen(parm.i_fds);
	hmaplen = fpmaplen(parm.i_fdh);
	map = fpmap(parm.i_fds);
	hmap = fpmap(parm.i_fdh);

	npix = hnsamps(parm.i_fds);
	if (npix != hnsamps(parm.i_fdh)) {
		error("# samples different in two input files");
	}
	if (hnbands(parm.i_fds) != 2) {
		error("input slope/azm file must have 2 bands");
	}
	horbnd = hnbands(parm.i_fdh);

 /* NOSTRICT */
	parm.sbuf = (pixel_t *) ecalloc(2 * npix, sizeof(pixel_t));
	if (parm.sbuf == NULL) {
		error("can't allocate slope/azm buffer");
	}
 /* NOSTRICT */
	parm.hbuf = (pixel_t *) ecalloc(horbnd * npix, sizeof(pixel_t));
	if (parm.hbuf == NULL) {
		error("can't allocate horizon buffer");
	}

 /* NOSTRICT */
	parm.obuf = (fpixel_t *) ecalloc(npix * 2, sizeof(fpixel_t));
	if (parm.obuf == NULL) {
		error("can't allocate output buffer");
	}
 /*
  * buffers for trig values
  */

	parm.sstbl = map[0];
 /* NOSTRICT */
	parm.cstbl = (float *) ecalloc(smaplen[0], sizeof(float));
	parm.chtbl = hmap;
 /* NOSTRICT */
	parm.cosdtbl = (float **) allocnd(sizeof(float), 2, horbnd, smaplen[1]);
 /* NOSTRICT */
	parm.sh2tbl = (float **) ecalloc(horbnd, sizeof(float *));
 /* NOSTRICT */
	parm.hdtbl = (float **) ecalloc(horbnd, sizeof(float *));
	if (parm.sh2tbl == NULL || parm.cstbl == NULL ||
	    parm.chtbl == NULL || parm.cosdtbl == NULL) {
		error("can't allocate buffers for trig tables");
	}
	for (j = 0; j < horbnd; ++j) {
 /* NOSTRICT */
		parm.sh2tbl[j] = (float *) ecalloc(hmaplen[j], sizeof(float));
 /* NOSTRICT */
		parm.hdtbl[j] = (float *) ecalloc(hmaplen[j], sizeof(float));
		if (parm.sh2tbl[j] == NULL || parm.hdtbl[j] == NULL) {
			error("can't allocate buffers for trig tables");
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/viewcalc/RCS/buffers.c,v 1.2 90/11/11 17:10:10 frew Exp $";

#endif
