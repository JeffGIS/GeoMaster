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

void
testbih(fdi, fdo)
	int             fdi;		/* input file descriptor	 */
	int             fdo;		/* output file descriptor	 */
{
	extern void     prbih();
	extern void     prhn();

	int             band;		/* loop counter			 */
	BIH_T         **ppd;		/* -> duplicate BIH array	 */
	BIH_T         **ppm;		/* -> "handmade" BIH array	 */
	BIH_T         **ppr;		/* -> input BIH array		 */

 /*
  * test bihread
  */
	ppr = bihread(fdi);
	if (ppr == NULL) {
		error("bihread failed");
	}

	fprintf(stderr, "from bihread:\n");
	prbih(ppr);
	prhn(fdi);
 /*
  * test bihdup
  */
	ppd = bihdup(ppr);
	if (ppd == NULL) {
		error("bihdup failed");
	}

	fprintf(stderr, "from bihdup:\n");
	prbih(ppd);
 /*
  * test bihmake
  */
	ppm = bihmake(bih_nlines(ppr[0]), bih_npixels(ppr[0]),
		      bih_nbands(ppr[0]), bih_nbytes(ppr[0]),
		      bih_nbits(ppr[0]));
	if (ppm == NULL) {
		error("bihmake failed");
	}

	for (band = 1; band < bih_nbands(ppr[0]); ++band) {
		bih_nlines(ppm[band]) = bih_nlines(ppr[band]);
		bih_npixels(ppm[band]) = bih_npixels(ppr[band]);
		bih_nbands(ppm[band]) = bih_nbands(ppr[band]);
		bih_nbytes(ppm[band]) = bih_nbytes(ppr[band]);
		bih_nbits(ppm[band]) = bih_nbits(ppr[band]);
	}

	fprintf(stderr, "from bihmake:\n");
	prbih(ppm);
 /*
  * test bihwrite
  */
	if (bihwrite(fdo, ppr) == ERROR) {
		error("bihwrite failed");
	}

	fprintf(stderr, "from bihwrite:\n");
	prhn(fdo);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/bih/TEST/RCS/testbih.c,v 1.4 90/11/11 17:13:28 frew Exp $";

#endif
