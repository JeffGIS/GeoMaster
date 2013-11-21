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

#include "ipw.h"

#include "bih.h"
#include "fpio.h"
#include "gethdrs.h"

#include "edimg.h"

/*
 * edimg -- replace selected image pixels
 */

void
edimg(i_fd, c_fd, raw, k, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	int             c_fd;		/* coordinate file descriptor	 */
	bool_t          raw;		/* ? raw replacement values	 */
	double          k;		/* default replacement val	 */
	int             o_fd;		/* output image file descriptor	 */
{
	BIH_T         **bihpp;		/* -> BIH			 */
	fpixel_t       *buf;		/* -> I/O buffer		 */
	int             eline;		/* next line to edit		 */
	int             line;		/* current input line #		 */
	int             nbands;		/* # image bands		 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line		 */
	double          repl;		/* replacement value		 */
	int             esamp;		/* next sample to edit		 */

 /*
  * read/write BIH
  */
	bihpp = bihread(i_fd);
	if (bihpp == NULL) {
		error("can't read basic image header");
	}

	nlines = bih_nlines(bihpp[0]);
	nsamps = bih_nsamps(bihpp[0]);
	nbands = bih_nbands(bihpp[0]);

	if (bihwrite(o_fd, bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * process remaining headers
  */
	if (raw) {
		copyhdrs(i_fd, nbands, o_fd);
	}
	else {
		fphdrs(i_fd, nbands, o_fd);
	}

	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}
 /*
  * allocate I/O buffer
  */
 /* NOSTRICT */
	buf = (fpixel_t *) ecalloc(nsamps * nbands, sizeof(fpixel_t));
	if (buf == NULL) {
		error("can't allocate I/O buffer");
	}
 /*
  * initialize edit coordinates
  */
	getedit(c_fd, nlines, nsamps, k, &eline, &esamp, &repl);
#if DEBUG
	fprintf(stderr, "edit %d,%d <- %g\n", eline, esamp, repl);
#endif
 /*
  * process image lines
  */
	for (line = 0; line < nlines; ++line) {
		if (fpvread(i_fd, buf, nsamps) != nsamps) {
			error("image read failed, line %d", line);
		}
 /*
  * if current line matches edit line then edit it
  */
		while (eline == line) {
			REG_2 int       band;	/* current band #	 */
			REG_1 fpixel_t *bufp;	/* -> current pixel	 */

			bufp = &buf[pixidx(nbands, esamp, 0)];

			for (band = 0; band < nbands; ++band) {
				*bufp++ = (fpixel_t) repl;
			}

			getedit(c_fd, nlines, nsamps, k, &eline, &esamp, &repl);
#if DEBUG
			fprintf(stderr, "edit %d,%d <- %g\n",
				eline, esamp, repl);
#endif
		}

		if (fpvwrite(o_fd, buf, nsamps) != nsamps) {
			error("image write failed, line %d", line);
		}
	}

	if (!ueof(i_fd)) {
		error("input image larger than header indicates");
	}

	SAFE_FREE(buf);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/edimg/RCS/edimg.c,v 1.10 90/11/11 17:02:14 frew Exp $";

#endif
