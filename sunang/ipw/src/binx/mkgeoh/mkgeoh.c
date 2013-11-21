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
#include "geoh.h"

#include "mkgeoh.h"

/*
 * mkgeoh -- append an GEO header to an image
 */

void
mkgeoh(i_fd, origin, incr, units, csys, h_nbands, h_band, force, o_fd)
	int             i_fd;		/* input image file descriptor	 */
	double         *origin;		/* u,v coords of image origin	 */
	double         *incr;		/* u,v increment per line,samp	 */
	char           *units;		/* u,v units of measurement	 */
	char           *csys;		/* u,v coord sys identifier	 */
	int             h_nbands;	/* # elements in h_band	 	 */
	int            *h_band;		/* -> bands to receive a GEO hdr */
	bool_t          force;          /* don't copy input image        */
	int             o_fd;		/* input image file descriptor	 */
{
 /* NOSTRICT */
	static GETHDR_T h_geo = {GEOH_HNAME, (ingest_t) geohread};
	static GETHDR_T *hv[] = {&h_geo, 0};

	BIH_T         **bihpp;		/* -> BI hdr array		 */
	int             i;		/* loop counter			 */
	GEOH_T         *geohp;		/* -> new GEO header		 */
	GEOH_T        **geohpp;		/* -> input GEO header array	 */
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
  * process remaining headers -- get GEO header if there is one
  */
	gethdrs(i_fd, hv, nbands, o_fd);
 /*
  * make GEO header
  */
	geohp = geohmake(origin[0], origin[1], incr[0], incr[1], units, csys);
	if (geohp == NULL) {
		error("can't create GEO header");
	}
 /*
  * if there's already an GEO header then add to it
  */
	if (got_hdr(h_geo)) {
 /* NOSTRICT */
		geohpp = (GEOH_T **) hdr_addr(h_geo);
	}
 /*
  * if there's not already an GEO header then create array
  */
	else {
 /* NOSTRICT */
		geohpp = (GEOH_T **) hdralloc(nbands, sizeof(GEOH_T *),
					      o_fd, GEOH_HNAME);
		if (geohpp == NULL) {
			error("can't allocate GEO header array");
		}
	}
 /*
  * insert new header in GEO array
  */
	if (h_nbands < 1) {
		h_nbands = nbands;
	}

	for (i = 0; i < h_nbands; ++i) {
		int             band;	/* current band #		 */

		band = (h_band != NULL) ? h_band[i] : i;

		if (geohpp[band] != NULL) {
			warn("band %d: replacing previous GEO header", band);
		}

		geohpp[band] = geohp;
	}
 /*
  * write GEO header
  */
	if (geohwrite(o_fd, geohpp) == ERROR) {
		error("can't write GEO header");
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
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mkgeoh/RCS/mkgeoh.c,v 1.4 90/11/11 17:06:12 frew Exp $";

#endif
