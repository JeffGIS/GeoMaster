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
#include "pixio.h"
#include "dither.h"

/*
 * dithimg -- dither an image down to 1 bit / pixel
 */

#define	IN_NLEVELS	256

void
DEFUN( dithimg, (i_fd, nlines, nsamps, nbands, dm, rank, o_fd),
	int             i_fd		/* input image file descriptor	 */
   AND  int             nlines		/* # image lines		 */
   AND  int             nsamps		/* # samples / image line	 */
   AND  int             nbands		/* # image bands		 */
   AND  pixel_t      ***dm		/* -> dither matrix		 */
   AND  int             rank		/* dither matrix rank		 */
   AND  int             o_fd)		/* output image file descriptor	 */
{
	pixel_t        *buf;		/* -> I/O buffer		 */
	REG_6 int       idx_mask;	/* dither matrix index mask	 */
	int             line;		/* current line #		 */

 /*
  * allocate I/O buffer
  */
 /* NOSTRICT */
	buf = (pixel_t *) ecalloc(nsamps * nbands, sizeof(pixel_t));
	if (buf == NULL) {
		error("can't allocate I/O buffer");
	}
 /*
  * construct dither matrix index mask
  * 
  * NB: this assumes rank is a power of 2
  */
	idx_mask = rank - 1;
 /*
  * process a line at a time
  */
	for (line = 0; line < nlines; ++line) {
		REG_1 pixel_t  *bufp;	/* -> current pixel in buf	 */
		REG_3 pixel_t **dm_line;/* -> dither matrix line	 */
		REG_5 int       samp;	/* current sample #		 */

		if (pvread(i_fd, buf, nsamps) != nsamps) {
			error("image read failed, line %d", line);
		}

		bufp = buf;
		dm_line = dm[line & idx_mask];

		for (samp = nsamps; --samp >= 0;) {
			REG_4 int       band;	/* current band #	 */
			REG_2 pixel_t  *dm_samp;	/* -> d.m. samp	 */

			dm_samp = dm_line[samp & idx_mask];

			band = nbands;
			do {
				if (*bufp < *dm_samp++) {
					*bufp++ = BLACK;
				}
				else {
					*bufp++ = WHITE;
				}
			} while (--band > 0);
		}

		if (pvwrite(o_fd, buf, nsamps) != nsamps) {
			error("image write failed, line %d", line);
		}
	}
 /*
  * make sure there's nothing left to be read
  */
	if (!ueof(i_fd)) {
		error("input image is larger than header indicates");
	}

	SAFE_FREE(buf);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/dither/RCS/dithimg.c,v 1.2 90/11/11 17:02:04 frew Exp $";

#endif
