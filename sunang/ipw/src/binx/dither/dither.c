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
#include "dither.h"

/*
 * dither -- orchestrate image dithering
 */

void
DEFUN( dither, (i_fd, rank, o_fd),
	int             i_fd		/* input image file descriptor	 */
   AND  int             rank		/* dither matrix rank		 */
   AND  int             o_fd)		/* output image file descriptor	 */
{
	int             band;		/* current band #		 */
	pixel_t      ***dm;		/* -> dither matrix		 */
	BIH_T         **i_bihpp;	/* -> input BIH			 */
	int             nbands;		/* # image bands		 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / image line	 */
	BIH_T         **o_bihpp;	/* -> output BIH		 */

 /*
  * read input BIH
  */
	i_bihpp = bihread(i_fd);
	if (i_bihpp == NULL) {
		error("can't read basic image header");
	}

	nlines = bih_nlines(i_bihpp[0]);
	nsamps = bih_nsamps(i_bihpp[0]);
	nbands = bih_nbands(i_bihpp[0]);
 /*
  * create and write output BIH (same as input except 1 byte, 1 bit per
  * pixel)
  */
	o_bihpp = bihdup(i_bihpp);
	if (o_bihpp == NULL) {
		error("can't allocate output BIH");
	}

	for (band = 0; band < nbands; ++band) {
		bih_nbytes(o_bihpp[band]) = 1;
		bih_nbits(o_bihpp[band]) = 1;
	}

	if (bihwrite(o_fd, o_bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * copy remaining headers
  */
	copyhdrs(i_fd, nbands, o_fd);

	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}
 /*
  * construct dither matrix
  */
	dm = mk_dm(rank, nbands, i_bihpp);
 /*
  * dither the image
  */
	dithimg(i_fd, nlines, nsamps, nbands, dm, rank, o_fd);
 /*
  * clean up
  */
	SAFE_FREE(dm);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/dither/RCS/dither.c,v 1.2 90/11/11 17:02:00 frew Exp $";

#endif
