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
#include "fpio.h"
#include "gethdrs.h"
#include "lqh.h"

/*
 * fpio -- test fpio package
 */

void
fpio(i_fd, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	int             o_fd;		/* output image file descriptor	 */
{
	static GETHDR_T h_lq = {
 /* NOSTRICT */
		LQH_HNAME, (ingest_t) lqhread
	};

	static GETHDR_T *hv[] = {
		&h_lq,
		0
	};

	BIH_T         **bihpp;		/* -> array of BI headers	 */
	fpixel_t       *buf;		/* I/O buffer			 */
	int             line;		/* current image line		 */
	LQH_T         **lqhpp;		/* -> array of LQ headers	 */
	int             nbands;		/* # bands / image sample	 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / image line	 */

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
  * copy additional headers; grab LQ header if present
  */
	gethdrs(i_fd, hv, nbands, o_fd);

 /* NOSTRICT */
	lqhpp = (LQH_T **) hdr_addr(h_lq);
	if (lqhpp != NULL) {
		if (lqhwrite(o_fd, lqhpp) == ERROR) {
			error("can't write LQ header");
		}
	}

	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}
 /*
  * read/write fpixels
  */
 /* NOSTRICT */
	buf = (fpixel_t *) ecalloc(nsamps * nbands, sizeof(fpixel_t));
	if (buf == NULL) {
		error("can't allocate fpixel I/O buffer");
	}

	for (line = 0; line < nlines; ++line) {
		if (fpvread(i_fd, buf, nsamps) != nsamps) {
			error("fpvread error, line %d", line);
		}

		if (fpvwrite(o_fd, buf, nsamps) != nsamps) {
			error("fpvwrite error, line %d", line);
		}
	}

	if (!ueof(i_fd)) {
		error("input image larger than header indicates");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/fpio/TEST/RCS/fpio.c,v 1.2 90/11/11 17:14:43 frew Exp $";

#endif
