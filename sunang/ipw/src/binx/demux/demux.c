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

#include "demux.h"

/*
 * demux -- orchestrate image demultiplexing
 */

void
demux(i_fd, o_nbands, o2i_band, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	int             o_nbands;	/* # output bands		 */
	int            *o2i_band;	/* input[output] band #		 */
	int             o_fd;		/* output image file descriptor	 */
{
	int             i_band;		/* current input band #		 */
	BIH_T         **i_bihpp;	/* -> input BIH			 */
	int             i_nbands;	/* # input bands		 */
	int             o_band;		/* current output band #	 */
	BIH_T         **o_bihpp;	/* -> output BIH		 */

 /*
  * read BIH
  */
	i_bihpp = bihread(i_fd);
	if (i_bihpp == NULL) {
		error("can't read basic image header");
	}
 /*
  * check band selections
  */
	i_nbands = bih_nbands(i_bihpp[0]);

	for (o_band = 0; o_band < o_nbands; ++o_band) {
		i_band = o2i_band[o_band];
		if (i_band < 0 || i_band >= i_nbands) {
			uferr(i_fd);
			error("%d: bad input band number", i_band);
		}
	}
 /*
  * make output BIH
  */
	o_bihpp = bihdup(i_bihpp);
	if (o_bihpp == NULL) {
		error("can't allocate output basic image header");
	}

	for (o_band = 0; o_band < o_nbands; ++o_band) {
		BIH_T		*bihp;	/* temporary -> BIH		*/

		i_band = o2i_band[o_band];
 /*
  * reorder output BIHs
  */
		bihp = o_bihpp[o_band];
		o_bihpp[o_band] = o_bihpp[i_band];
		o_bihpp[i_band] = bihp;
 /*
  * reset output # bands
  */
		bih_nbands(o_bihpp[o_band]) = o_nbands;
	}
 /*
  * write output BIH
  */
	if (bihwrite(o_fd, o_bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * demultiplex remaining headers
  */
	demuxhdrs(i_fd, i_nbands, o_fd, o_nbands, o2i_band);
 /*
  * demultiplex image data
  */
	demuximg(i_fd,
		 bih_nlines(i_bihpp[0]), bih_nsamps(i_bihpp[0]), i_nbands,
		 o_fd,
		 o_nbands, o2i_band);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/demux/RCS/demux.c,v 1.4 90/11/11 17:01:46 frew Exp $";

#endif
