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

#include "pixio.h"

#include "lutx.h"

/*
 * lutximg--
 */

void
lutximg(i_fd, nlines, nsamps, nbands, lut, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / image line	 */
	REG_6 int       nbands;		/* # image bands		 */
	REG_3 pixel_t **lut;		/* -> LUT array		 	 */
	int             o_fd;		/* output image file descriptor	 */
{
	pixel_t        *buf;		/* -> I/O buffer		 */
	int             line;		/* current line #		 */

 /*
  * allocate I/O buffer
  */
 /* NOSTRICT */
	buf = (pixel_t *) ecalloc(nsamps * nbands, sizeof(pixel_t));
	if (buf == NULL) {
		error("can't allocate image I/O buffer");
	}
 /*
  * I/O loop
  */
	for (line = 0; line < nlines; ++line) {
		REG_1 pixel_t  *bufp;	/* -> current pixel		 */
		REG_5 int       samp;	/* loop counter			 */

 /*
  * read line
  */
		if (pvread(i_fd, buf, nsamps) != nsamps) {
			error("image read failed, line %d", line);
		}

		bufp = buf;
 /*
  * loop through samples (sample # unimportant)
  */
		samp = nsamps;
		do {
			REG_4 int       band;	/* current band #	 */

 /*
  * loop through bands
  */
			for (band = 0; band < nbands; ++band) {
				REG_2 pixel_t  *lutp;	/* -> lut[pixel] */

 /*
  * map pixel value through lookup table
  */
				lutp = lut[*bufp];
				*bufp++ = lutp[band];
			}
		} while (--samp > 0);
 /*
  * write line
  */
		if (pvwrite(o_fd, buf, nsamps) != nsamps) {
			error("image write failed, line %d", line);
		}
	}

	SAFE_FREE(buf);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/lutx/RCS/lutximg.c,v 1.2 90/11/11 17:05:53 frew Exp $";

#endif
