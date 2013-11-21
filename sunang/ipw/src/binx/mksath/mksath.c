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
#include "sath.h"

#include "mksath.h"

/*
 * mksath -- append a SAT header to an image
 */

void
mksath(i_fd, platform, sensor, location, gmdate, gmtime,
       h_nbands, h_band, force, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	char           *platform;	/* e.g. "Landsat", "ER2", ...    */
	char           *sensor;		/* e.g. "TM", "AVIRIS", ...      */
	char           *location;	/* e.g. Landsat path, row, quad  */
	char           *gmdate;		/* YYYYMMDD			 */
	char           *gmtime;		/* hhmmss.sss...                 */
	int             h_nbands;	/* # elements in h_band	 	 */
	int            *h_band;		/* -> bands to receive a SAT hdr */
	bool_t          force;          /* don't copy input image        */
	int             o_fd;		/* input image file descriptor	 */
{
 /* NOSTRICT */
	static GETHDR_T h_sat = {SATH_HNAME, (ingest_t) sathread};
	static GETHDR_T *hv[] = {&h_sat, 0};

	BIH_T         **bihpp;		/* -> BI hdr array		 */
	int             i;		/* loop counter			 */
	SATH_T         *sathp;		/* -> new SAT header		 */
	SATH_T        **sathpp;		/* -> input SAT header array	 */
	int             nbands;		/* # image bands		 */

 /*
  * read and write BIH
  */
	bihpp = bihread(i_fd);
	if (bihpp == NULL) {
		error("can't read basic image header");
	}

	if (bihwrite(o_fd, bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * extract goodies from BIH
  */
	nbands = bih_nbands(bihpp[0]);
 /*
  * check map band #'s
  */
	for (i = 0; i < h_nbands; ++i) {
		if (h_band[i] < 0 || h_band[i] >= nbands) {
			error("bad band number: %d", h_band[i]);
		}
	}
 /*
  * process remaining headers -- get SAT header if there is one
  */
	gethdrs(i_fd, hv, nbands, o_fd);
 /*
  * make SAT header
  */
	sathp = sathmake(platform, sensor, location, gmdate, gmtime);
	if (sathp == NULL) {
		error("can't create SAT header");
	}
 /*
  * if there's already an SAT header then add to it
  */
	if (got_hdr(h_sat)) {
 /* NOSTRICT */
		sathpp = (SATH_T **) hdr_addr(h_sat);
	}
 /*
  * if there's not already an SAT header then create array
  */
	else {
 /* NOSTRICT */
		sathpp = (SATH_T **) hdralloc(nbands, sizeof(SATH_T *),
					      o_fd, SATH_HNAME);
		if (sathpp == NULL) {
			error("can't allocate SAT header array");
		}
	}
 /*
  * insert new header in SAT array
  */
	if (h_nbands < 1) {
		h_nbands = nbands;
	}

	for (i = 0; i < h_nbands; ++i) {
		int             band;	/* current band #		 */

		band = (h_band != NULL) ? h_band[i] : i;

		if (sathpp[band] != NULL) {
			warn("band %d: replacing previous SAT header", band);
		}

		sathpp[band] = sathp;
	}
 /*
  * write SAT header
  */
	if (sathwrite(o_fd, sathpp) == ERROR) {
		error("can't write SAT header");
	}
 /*
  * copy image data
  */
	if (boimage(o_fd) == ERROR) {
		error("can't terminate header output");
	}

	if (!force) {
		if (imgcopy(i_fd, o_fd) == ERROR) {
			error("can't copy image data");
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mksath/RCS/mksath.c,v 1.3 90/11/11 17:06:26 frew Exp $";

#endif
