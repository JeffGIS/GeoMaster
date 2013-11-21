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

/* LINTLIBRARY */

#include "ipw.h"

#include "bih.h"
#include "gethdrs.h"
#include "lqh.h"

#include "mklqh.h"

/*
 * mklqh -- append an LQ header to an image
 */

void
mklqh(i_fd, nbkpts, ibkpt, fbkpt, units, interp, h_nbands, h_band, force, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	int             nbkpts;		/* # breakpoint pairs            */
	pixel_t        *ibkpt;		/* -> integer breakpoints        */
	fpixel_t       *fbkpt;		/* -> floating-point breakpoints */
	char           *units;		/* units of measurement          */
	char           *interp;		/* interpolation function name   */
	int             h_nbands;	/* # elements in h_band	 	 */
	int            *h_band;		/* -> bands to receive a LQ hdr	 */
	bool_t          force;		/* don't copy input image        */
	int             o_fd;		/* input image file descriptor	 */
{
 /* NOSTRICT */
	static GETHDR_T h_lq = {LQH_HNAME, (ingest_t) lqhread};
	static GETHDR_T *hv[] = {&h_lq, 0};

	BIH_T         **bihpp;		/* -> BI hdr array		 */
	int             i;		/* loop counter			 */
	LQH_T          *lqhp;		/* -> new LQ header		 */
	LQH_T         **lqhpp;		/* -> input LQ header array	 */
	int             nbands;		/* # image bands		 */
	int             nbits;		/* # bits / pixel		 */

 /*
  * read and write BIH
  */
	bihpp = bihread(i_fd);
	if (bihpp == NULL) {
		error("can't read basic image header");
	}

	if (bihwrite(o_fd, bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * extract goodies from BIH
  */
	nbands = bih_nbands(bihpp[0]);
	nbits = bih_nbits(bihpp[0]);
 /*
  * check map band #'s
  */
	for (i = 0; i < h_nbands; ++i) {
		if (h_band[i] < 0 || h_band[i] >= nbands) {
			uferr(i_fd);
			error("no band %d", h_band[i]);
		}
	}
 /*
  * process remaining headers -- get LQ header if there is one
  */
	gethdrs(i_fd, hv, nbands, o_fd);
 /*
  * make LQ header
  */
	lqhp = lqhmake(nbits, nbkpts, ibkpt, fbkpt, units, interp);
	if (lqhp == NULL) {
		error("can't create LQ header");
	}
 /*
  * if there's already an LQ header then add to it
  */
	if (got_hdr(h_lq)) {
 /* NOSTRICT */
		lqhpp = (LQH_T **) hdr_addr(h_lq);
	}
 /*
  * if there's not already an LQ header then create array
  */
	else {
 /* NOSTRICT */
		lqhpp = (LQH_T **) hdralloc(nbands, sizeof(LQH_T *),
					    o_fd, LQH_HNAME);
		if (lqhpp == NULL) {
			error("can't allocate LQ header array");
		}
	}
 /*
  * insert new header in LQ array
  */
	if (h_nbands < 1) {
		h_nbands = nbands;
	}

	for (i = 0; i < h_nbands; ++i) {
		int             band;	/* current band #		 */

		band = (h_band == NULL) ? i : h_band[i];

		if (bih_nbits(bihpp[band]) != nbits) {
			uferr(i_fd);
			error("bands %d and %d have different # bits / pixel",
			      (h_band == NULL) ? 0 : h_band[0],
			      band);
		}

		if (lqhpp[band] != NULL) {
			uferr(i_fd);
			warn("band %d: replacing previous LQ header", band);
		}

		lqhpp[band] = lqhp;
	}
 /*
  * write LQ header
  */
	if (lqhwrite(o_fd, lqhpp) == ERROR) {
		error("can't write LQ header");
	}
 /*
  * copy image data
  */
	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}

	if (!force) {
		if (imgcopy(i_fd, o_fd) == ERROR) {
			error("can't copy image data");
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mklqh/RCS/mklqh.c,v 1.7 90/11/11 17:06:19 frew Exp $";

#endif
