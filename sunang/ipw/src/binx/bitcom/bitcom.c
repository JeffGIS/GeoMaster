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

#include "bitcom.h"

/*
 * bitcom -- orchestrate bitwise band combination
 */

void
bitcom(i_fd, mflag, op, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	bool_t          mflag;		/* ? last band is a mask	 */
	void            (*op) ();	/* -> bitwise operator function	 */
	int             o_fd;		/* output image file descriptor	 */

{
	int             band;		/* current input band #		 */
	BIH_T         **i_bihpp;	/* input BIH			 */
	int             nbands;		/* # image bands		 */
	int             nbits;		/* # bits / band		 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line		 */
	BIH_T          *o_bihp;		/* output BIH			 */

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

	if (nbands < 2) {
		uferr(i_fd);
		error("single-band input image");
	}

	nbits = bih_nbits(i_bihpp[0]);
 /*
  * check that all input bands have same # bits / pixel (exception: last band
  * may differ if -m was specified)
  */
	for (band = 1; band < nbands; ++band) {
		if (nbits != bih_nbits(i_bihpp[band])
		    && !(mflag && band == nbands - 1)) {
			uferr(i_fd);
			error("different # bits / pixel: bands 0, %d", band);
		}
	}
 /*
  * write output BIH
  */
	o_bihp = bihmake(bih_nbytes(i_bihpp[0]), nbits,
			 bih_history(i_bihpp[0]),
			 bih_annot(i_bihpp[0]),
			 (BIH_T *) NULL, nlines, nsamps, 1);
	if (o_bihp == NULL) {
		error("can't create basic image header");
	}

	if (bihwrite(o_fd, &o_bihp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * copy headers
  */
	copyhdrs(i_fd, 1, o_fd);

	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}
 /*
  * start shovelling
  */
	bitio(i_fd, nlines, nsamps, nbands, mflag, op, o_fd);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/bitcom/RCS/bitcom.c,v 1.4 90/11/11 17:00:26 frew Exp $";

#endif
