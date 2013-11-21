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

 /*
  * read and write headers
  */
#include "ipw.h"

#include "gethdrs.h"

#include "bih.h"
#include "lqh.h"
#include "orh.h"
#include "geoh.h"
#include "fpio.h"
#include "pgm.h"

#define IBANDS		1		/* # bands in input image	 */
#define OBANDS		2		/* # bands in output image	 */

void
headers(fdi, fdo, slope, aspect, nbits, spacing)
	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */
	bool_t          slope;		/* ? compute slopes		 */
	bool_t          aspect;		/* ? compute aspects		 */
	int            *nbits;		/* # bits in output LQ		 */
	fpixel_t       *spacing;	/* grid spacing (meters)	 */
{
	BIH_T         **i_bih;		/* -> input BIH			 */
	BIH_T         **o_bih;		/* -> output BIH		 */
	GEOH_T        **o_geoh;		/* -> output geodetic header	 */
	LQH_T         **o_lqh;		/* -> output LQH		 */
	int             j;		/* counter			 */
	int             obands;		/* # bands in output		 */

 /* NOSTRICT */
	static GETHDR_T h_lqh = {LQH_HNAME, (ingest_t) lqhread};
 /* NOSTRICT */
	static GETHDR_T h_geo = {GEOH_HNAME, (ingest_t) geohread};
 /* NOSTRICT */
	static GETHDR_T h_orh = {ORH_HNAME, (ingest_t) orhread};
	static GETHDR_T *request[] = {&h_lqh, &h_geo, &h_orh, 0};

	assert(slope || aspect);
	obands = (slope && aspect) ? OBANDS : 1;
	o_geoh = NULL;
 /*
  * read BIH
  */
	if ((i_bih = bihread(fdi)) == NULL) {
		error("can't read BIH");
	}
 /*
  * check input bih
  */
	if (!bihvalid(i_bih, IBANDS)) {
		error("not valid BIH for this program");
	}
 /*
  * create output BIH, # bytes will be set by bihmake
  */
 /* NOSTRICT */
	o_bih = (BIH_T **) hdralloc(obands, sizeof(BIH_T *), fdo, BIH_HNAME);
	for (j = 0; j < obands; ++j) {
		o_bih[j] = bihmake(0, nbits[j],
				   bih_history(i_bih[0]), bih_annot(i_bih[0]),
				   o_bih[0],
				   bih_nlines(i_bih[0]), bih_nsamps(i_bih[0]),
				   obands);
	}
 /*
  * write output BIH
  */
	if (bihwrite(fdo, o_bih) == ERROR) {
		error("Can't write output BIH");
	}
 /*
  * ingest LQH, check for presence of ORH, modify GEOH; other headers get
  * passed thru
  */
	gethdrs(fdi, request, IBANDS, fdo);

	if (hdr_addr(h_lqh) == NULL)
		warn("input file has no LQH; raw values used");
 /*
  * there should not be an ORH
  */
	if (hdr_addr(h_orh) != NULL)
		warn("Elevation file should be standard orientation");
 /*
  * get spacing from geodetic header; make 2-band output geodetic header
  */
	if (hdr_addr(h_geo) != NULL) {
 /* NOSTRICT */
		if ((o_geoh = newgeoh(bih_nbands(o_bih[0]), fdo,
			    (GEOH_T **) hdr_addr(h_geo), spacing)) == NULL) {
			error("Can't make new GEOH");
		}
 /*
  * write new geodetic header
  */
		if (geohwrite(fdo, o_geoh) == ERROR) {
			error("Can't write GEOH");
		}
	}
 /*
  * check that we read a GEOH, set spacing to 1.0 if none specified
  */
	else if (spacing[0] == 0) {
		warn("Elevation file has no GEOH, spacing set to 1.0");
		spacing[0] = spacing[1] = 1;
	}
 /*
  * output LQH
  */
	o_lqh = newlqh(fdo, slope, aspect);
	if (o_lqh == NULL) {
		error("Can't create new LQH");
	}
 /*
  * write output LQH
  */
	if (lqhwrite(fdo, o_lqh) == ERROR) {
		error("Can't write output LQH");
	}
 /*
  * get ready for rest of output
  */
	if (boimage(fdo) == ERROR) {
		error("Can't terminate header output");
	}

 /*
  * clean up
  */
	if (hdr_addr(h_geo) != NULL) {
		SAFE_FREE(o_geoh);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/headers.c,v 1.7 90/11/11 17:03:23 frew Exp $";

#endif
