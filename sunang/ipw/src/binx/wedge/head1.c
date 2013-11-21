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
  * read all headers, write output BIH
  */

#include "ipw.h"
#include "bih.h"
#include "gethdrs.h"
#include "lqh.h"

void
head1(fdi, nbits, ncoef, fdo)
	int             fdi;		/* input file descriptor	 */
	int             nbits;		/* # output bits		 */
	int             ncoef;		/* # coefficients		 */
	int             fdo;		/* input file descriptor	 */
{
	BIH_T         **i_bih;		/* input BIH			 */
	BIH_T         **o_bih;		/* output BIH			 */
	int             band;		/* band index			 */
	int             obands;		/* # output bands		 */

 /* NOSTRICT */
	static GETHDR_T h_lqh = {LQH_HNAME, (ingest_t) NULL};
	static GETHDR_T *request[] = {&h_lqh, 0};

 /*
  * read input BIH
  */
	if ((i_bih = bihread(fdi)) == NULL) {
		error("can't read input BIH");
	}
 /*
  * determine nbits if not set from command line
  */
	assert(nbits >= 0);
	if (nbits == 0) {
		int             nval;	/* max nlines - 1, nsamps - 1	 */

		nval = MAX(bih_nlines(i_bih[0]), bih_nsamps(i_bih[0])) - 1;
		assert(nval > 0);

		do {
			nval >>= 1;
			++nbits;
		} while (nval > 0);
	}
 /*
  * determine number of output bands
  */
	if (ncoef % 2) {
		error("must specify an even number of coefficients");
	}
	else {
		obands = ncoef / 2;
	}
 /*
  * create and write output BIH
  */
 /* NOSTRICT */
	o_bih = (BIH_T **) hdralloc(obands, sizeof(BIH_T *), fdo, BIH_HNAME);
	assert(o_bih != NULL);
	for (band = 0; band < obands; ++band) {
		o_bih[band] = bihmake(0, nbits,
			   bih_history(i_bih[0]), bih_annot(i_bih[0]),
				      o_bih[0],
			   bih_nlines(i_bih[0]), bih_nsamps(i_bih[0]),
				      obands);
		if (o_bih[band] == NULL) {
			error("can't make output BIH");
		}
	}
	if (bihwrite(fdo, o_bih) == ERROR) {
		error("can't write BIH");
	}
 /*
  * all other headers get passed thru, except any LQH is ignored
  */
	gethdrs(fdi, request, obands, fdo);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/wedge/RCS/head1.c,v 1.4 90/11/11 17:10:32 frew Exp $";

#endif
