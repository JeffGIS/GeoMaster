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
#include "sunh.h"

#include "sunh.h"

void
mksunh(fdi, azimuth, cos_zen, force, fdo)
	int             fdi;		/* input file descriptor	 */
	double		azimuth;	/* solar azimuth		 */
	double		cos_zen;	/* cosine of solar zenith	 */
	bool_t          force;          /* don't copy input image        */
	int             fdo;		/* output file descriptor	 */
{
	static GETHDR_T h_sunh = {SUNH_HNAME, NULL};
	static GETHDR_T *hv[] = {&h_sunh, NULL};

	int             band;		/* loop counter			 */
	BIH_T         **bihpp;		/* -> basic image header	 */
	int             nbands;		/* image # bands		 */
	SUNH_T         *sunhp;		/* -> SUN header	 	 */
	SUNH_T        **sunhpp;		/* -> SUN header pointer array	 */
	bool_t		madehdr;	/* did we allocate header space? */

 /*
  * read and write BIH
  */
	bihpp = bihread(fdi);
	if (bihpp == NULL) {
		error("can't read basic image header");
	}

	nbands = bih_nbands(bihpp[0]);

	if (bihwrite(fdo, bihpp) != OK) {
		error("can't write basic image header");
	}
 /*
  * make sun header
  */
	sunhp = sunhmake(cos_zen, azimuth);
	if (sunhp == NULL) {
		error("can't make sun header");
	}
 /*
  * copy remaining headers: shortstop any sun headers
  */
	gethdrs(fdi, hv, nbands, fdo);
 /*
  * if existing sun header then use it
  */
	madehdr = FALSE;

	if (got_hdr(h_sunh)) {
 /* NOSTRICT */
		sunhpp = (SUNH_T **) hdr_addr(h_sunh);
	}
 /*
  * if no existing sun header then allocate pointer array
  */
	else {
 /* NOSTRICT */
		sunhpp = (SUNH_T **) hdralloc(nbands, sizeof(SUNH_T *),
					      ERROR, SUNH_HNAME);
		if (sunhpp == NULL) {
			error("can't allocate sun header pointer array");
		}
		madehdr = TRUE;
	}
 /*
  * link sun header into pointer array
  */
	for (band = 0; band < nbands; ++band) {
		sunhpp[band] = sunhp;
	}
 /*
  * write sun header
  */
	if (sunhwrite(fdo, sunhpp) != OK) {
		error("can't write sun header");
	}

	if (boimage(fdo) != OK) {
		error("can't terminate header output");
	}
 /*
  * copy image
  */
	if (!force) {
		if (imgcopy(fdi, fdo) == ERROR) {
			error("image copy failed");
		}
	}

	if (madehdr) {
		SAFE_FREE(sunhpp);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mksunh/RCS/mksunh.c,v 1.4 90/11/11 17:06:33 frew Exp $";

#endif
