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
#include "pixio.h"

#include "lutx.h"

/*
 * lutx -- orchestrate LUT-driven image transform
 */

void
lutx(i_fd, L_fd, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	int             L_fd;		/* LUT file descriptor		 */
	int             o_fd;		/* output image file descriptor	 */
{
	BIH_T         **L_bihpp;	/* -> LUT header		 */
	int             L_nsamps;	/* # samples / LUT line		 */
	int             band;		/* loop counter			 */
	BIH_T         **i_bihpp;	/* -> input image header	 */
	pixel_t       **lut;		/* -> LUT array		 	 */
	int             nbands;		/* # image bands		 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / image line	 */
	BIH_T         **o_bihpp;	/* -> output image header	 */

 /*
  * read image BIH
  */
	i_bihpp = bihread(i_fd);
	if (i_bihpp == NULL) {
		error("can't read basic image header");
	}

	nlines = bih_nlines(i_bihpp[0]);
	nsamps = bih_nsamps(i_bihpp[0]);
	nbands = bih_nbands(i_bihpp[0]);
 /*
  * read lookup table BIH
  */
	L_bihpp = bihread(L_fd);
	if (L_bihpp == NULL) {
		error("can't read basic image header");
	}

	L_nsamps = bih_nsamps(L_bihpp[0]);
 /*
  * LUT must have exactly 1 line
  */
	if (bih_nlines(L_bihpp[0]) != 1) {
		uferr(L_fd);
		error("not a lookup table (# lines > 1)");
	}
 /*
  * LUT and image must have same # bands
  */
	if (bih_nbands(L_bihpp[0]) != nbands) {
		error("image and LUT have different # bands");
	}
 /*
  * # samples / LUT line must be >= 2**(# bits / image pixel), for all image
  * bands
  */
	for (band = 0; band < nbands; ++band) {
		if (ipow2(bih_nbits(i_bihpp[band])) > L_nsamps) {
			error("%d-element LUT can't map %d-bit pixels",
			      L_nsamps, bih_nbits(i_bihpp[band]));
		}
	}
 /*
  * create output BIH: nbytes, nbits copied from LUT header
  */
	o_bihpp = bihdup(i_bihpp);
	if (o_bihpp == NULL) {
		error("can't create basic image header");
	}

	for (band = 0; band < nbands; ++band) {
		bih_nbytes(o_bihpp[band]) = bih_nbytes(L_bihpp[band]);
		bih_nbits(o_bihpp[band]) = bih_nbits(L_bihpp[band]);
	}
 /*
  * write BIH
  */
	if (bihwrite(o_fd, o_bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * copy remaining image headers
  */
	copyhdrs(i_fd, nbands, o_fd);

	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}
 /*
  * skip remaining LUT headers
  */
	skiphdrs(L_fd);
 /*
  * allocate LUT buffer
  */
 /* NOSTRICT */
	lut = (pixel_t **) allocnd(sizeof(pixel_t), 2, L_nsamps, nbands);
	if (lut == NULL) {
		error("can't allocate lookup table");
	}
 /*
  * read LUT
  */
	if (pvread(L_fd, lut[0], L_nsamps) != L_nsamps) {
		error("lookup table read failed");
	}
 /*
  * do the LUT transform
  */
	lutximg(i_fd, nlines, nsamps, nbands, lut, o_fd);

 /*
  * clean up
  */

	SAFE_FREE(lut);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/lutx/RCS/lutx.c,v 1.2 90/11/11 17:05:49 frew Exp $";

#endif
