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

void
pixiotest(fdi, fdo)
	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */
{
	BIH_T         **bih;		/* -> BIH array			 */
	pixel_t        *buf;		/* -> I/O buffer		 */
	int             line;		/* current line #		 */
	int             nbands;		/* # image bands		 */
	int             nlines;		/* # image lines		 */
	int             npixv;		/* # pixel vectors / current line */
	int             nsamps;		/* # samples / line		 */

 /*
  * process headers
  */
	bih = bihread(fdi);
	if (bih == NULL) {
		error("can't read basic image header");
	}

	nlines = bih_nlines(bih[0]);
	nsamps = bih_nsamps(bih[0]);
	nbands = bih_nbands(bih[0]);

	if (bihwrite(fdo, bih) == ERROR) {
		error("can't write basic image header");
	}

	copyhdrs(fdi, nbands, fdo);

	if (boimage(fdo) == ERROR) {
		error("can't terminate header output");
	}
 /*
  * allocate I/O buffer
  */
 /* NOSTRICT */
	buf = (pixel_t *) calloc((unsigned) (nsamps * nbands),
				 sizeof(pixel_t));
 /*
  * copy input lines -> output
  */
	for (line = 0; line < nlines; ++line) {
		npixv = pvread(fdi, buf, nsamps);
		if (npixv == ERROR) {
			error("pvread error, line %d", line);
		}

		if (pvwrite(fdo, buf, npixv) == ERROR) {
			error("pvwrite error, line %d", line);
		}
	}

	if (pxclose(fdi) == ERROR || pxclose(fdo) == ERROR) {
		error("pxclose failed");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/pixio/TEST/RCS/pixiotest.c,v 1.4 90/11/11 17:17:10 frew Exp $";

#endif
