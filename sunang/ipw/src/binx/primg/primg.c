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
#include "pixio.h"

#include "primg.h"

static addr_t   buf;			/* input buffer			 */
static int      nbands;			/* # bands / image sample	 */

static void
dprint(samp)
	int             samp;
{
	REG_2 int       band;
	REG_1 pixel_t  *pixel;
	REG_3 int       lastband;

 /* NOSTRICT */
	pixel = (pixel_t *) buf;
	pixel += samp * nbands;

	lastband = nbands - 1;

	for (band = 0; band < nbands; ++band) {
		(void) printf("%u%c", *pixel++, band == lastband ? '\n' : ' ');
	}
}

static void
gprint(samp)
	int             samp;
{
	REG_2 int       band;
	REG_1 fpixel_t *fpixel;
	REG_3 int       lastband;

 /* NOSTRICT */
	fpixel = (fpixel_t *) buf;
	fpixel += samp * nbands;

	lastband = nbands - 1;

	for (band = 0; band < nbands; ++band) {
		(void) printf("%.*g%c", FLT_DIG,
			      *fpixel++, band == lastband ? '\n' : ' ');
	}
}

/*
 * primg -- print image pixels
 */

void
primg(fdi, fdc, raw)
	int             fdi;		/* input image file descriptor	 */
	int             fdc;		/* coordinate file descriptor	 */
	bool_t          raw;		/* ? print raw values		 */
{
	BIH_T         **bihpp;		/* -> BI header array		 */
	int             cline;		/* line # of next pixel to print */
	int             csamp;		/* samp # of next pixel to print */
	int             line;		/* current image line #		 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line		 */
	int             (*in) ();	/* -> image read function	 */
	void            (*out) ();	/* -> pixel output function	 */
	int             pixsize;	/* sizeof input pixel		 */

 /*
  * read BIH
  */
	bihpp = bihread(fdi);
	if (bihpp == NULL) {
		error("can't read basic image header");
	}

	nbands = hnbands(fdi);
	nlines = hnlines(fdi);
	nsamps = hnsamps(fdi);
 /*
  * initialize for raw or floating-point pixels
  */
	if (raw) {
		skiphdrs(fdi);

		pixsize = sizeof(pixel_t);
		in = pvread;
		out = dprint;
	}
	else {
		fphdrs(fdi, NO_BAND, ERROR);

		pixsize = sizeof(fpixel_t);
		in = fpvread;
		out = gprint;
	}

	buf = ecalloc(nsamps * nbands, pixsize);
	if (buf == NULL) {
		error("can't allocate image input buffer");
	}

 /*
  * read 1st coordinate pair
  */
	if (fdc != ERROR) {
		if (getcoords(fdc, nlines, nsamps, &cline, &csamp) == EOF) {
			return;
		}
	}
 /*
  * read image lines
  */
	for (line = 0; line < nlines; ++line) {
		if ((*in) (fdi, buf, nsamps) != nsamps) {
			error("image read error, line %d", line);
		}
 /*
  * fdc == ERROR means print everything
  */
		if (fdc == ERROR) {
			int             samp;	/* current image sample # */

			for (samp = 0; samp < nsamps; ++samp) {
				(*out) (samp);
			}
		}
 /*
  * otherwise, only print as long as this line matches current coords
  */
		else {
			while (cline == line) {
				(*out) (csamp);

				if (getcoords(fdc, nlines, nsamps,
					      &cline, &csamp)
				    == EOF) {
					return;
				}
			}
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/primg/RCS/primg.c,v 1.11 90/11/11 17:07:59 frew Exp $";

#endif
