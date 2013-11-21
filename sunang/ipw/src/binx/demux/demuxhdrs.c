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

#include "hdrio.h"

#include "demux.h"

/*
 * demuxhdrs -- demultiplex image headers
 */

void
demuxhdrs(i_fd, i_nbands, o_fd, o_nbands, o2i_band)
	int             i_fd;		/* input image file descriptor	 */
	int             i_nbands;	/* # input bands		 */
	int             o_fd;		/* output image file descriptor	 */
	int             o_nbands;	/* # output bands		 */
	int            *o2i_band;	/* input[output] band #		 */
{
	char           *hname;		/* current header name		 */
	int            *i2o_band;	/* output[input] band #		 */
	int             i_band;		/* current input band #		 */
	int             o_band;		/* current output band #	 */

 /*
  * construct output[input] band map (default: NO_BAND => unselected)
  */
 /* NOSTRICT */
	i2o_band = (int *) ecalloc(i_nbands, sizeof(int));
	if (i2o_band == NULL) {
		error("can't allocate output[input] band map");
	}

	for (i_band = 0; i_band < i_nbands; ++i_band) {
		i2o_band[i_band] = NO_BAND;
	}

	for (o_band = 0; o_band < o_nbands; ++o_band) {
		i2o_band[o2i_band[o_band]] = o_band;
	}
 /*
  * process headers
  */
	while ((hname = hrname(i_fd)) != NULL && strdiff(hname, BOIMAGE)) {
		i_band = hrband(i_fd);

		if (i_band < 0 || i_band >= i_nbands) {
			error("\"%s\" header, band %d: no such band",
			      hname, i_band);
		}
 /*
  * change band number
  */
		o_band = i2o_band[i_band];
 /*
  * if not extracting this band then skip this header
  */
		if (o_band == NO_BAND) {
			if (hrskip(i_fd) == ERROR) {
				error("can't skip %s header", hname);
			}
		}
 /*
  * if extracting this band then copy this header, with new band number
  */
		else {
			if (hwprmb(o_fd, hname, o_band, hrvers(i_fd))
			    == ERROR) {
				error("\"%s\" header, band %d: can't write preamble",
				      hname, o_band);
			}

			if (hpass(i_fd, o_fd) == ERROR) {
				error("\"%s\" header, band %d: can't copy",
				      hname, o_band);
			}
		}
	}

	if (hname == NULL) {
		error("header read error");
	}

	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}

	SAFE_FREE(i2o_band);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/demux/RCS/demuxhdrs.c,v 1.3 90/11/11 17:01:51 frew Exp $";

#endif
