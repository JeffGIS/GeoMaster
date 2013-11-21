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
 * mk_dm -- make dither matrix
 */

pixel_t      ***
DEFUN( mk_dm, (rank, nbands, bihpp),
	int             rank		/* dither matrix rank		 */
   AND  int             nbands		/* # image bands		 */
   AND  BIH_T         **bihpp)		/* -> basic image header	 */
{
	pixel_t      ***dm;		/* -> dither matrix		 */
	int             line;		/* current line #		 */

 /*
  * allocate dither matrix
  */
 /* NOSTRICT */
	dm = (pixel_t ***) allocnd(sizeof(pixel_t), 3, rank, rank, nbands);
	if (dm == NULL) {
		error("can't allocate %dx%dx%d dither matrix",
		      rank, rank, nbands);
	}
 /*
  * initialize dither matrix: each band is scaled according to #bits / pixel
  * in that band
  */
	for (line = 0; line < rank; ++line) {
		int             samp;	/* current sample #		 */

		for (samp = 0; samp < rank; ++samp) {
			int             band;	/* current band #	 */

			for (band = 0; band < nbands; ++band) {
				dm[line][samp][band] =
					dm_val(rank, line, samp) *
					ipow2(bih_nbits(bihpp[band])) /
					(rank * rank);
			}
		}
	}

	return (dm);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/dither/RCS/mk_dm.c,v 1.2 90/11/11 17:02:12 frew Exp $";

#endif
