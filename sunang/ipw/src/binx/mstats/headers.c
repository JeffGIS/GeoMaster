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

#include "ipw.h"

#include "bih.h"
#include "gethdrs.h"
#include "lqh.h"

#include "pgm.h"

/*
 * headers -- process image headers
 */

void
headers()
{
 /*
  * need to read the LQ headers to initialize fpio
  */
 /* NOSTRICT */
	static GETHDR_T h_lqh = {LQH_HNAME, (ingest_t) lqhread};
	static GETHDR_T *request[] = {&h_lqh, NULL};

	BIH_T         **c_bihpp;	/* -> class BIH array		 */
	BIH_T         **i_bihpp;	/* -> input BIH array		 */

 /*
  * read input BIH
  */
	i_bihpp = bihread(parm.i_fd);
	if (i_bihpp == NULL) {
		error("can't read basic image header");
	}

	parm.nlines = bih_nlines(i_bihpp[0]);
	parm.nsamps = bih_nsamps(i_bihpp[0]);
	parm.i_nbands = bih_nbands(i_bihpp[0]);
 /*
  * process remaining input headers
  */
	gethdrs(parm.i_fd, request, NO_COPY, ERROR);
 /*
  * return now if no class file
  */
	if (parm.c_fd == ERROR) {
		parm.c_nclasses = 1;
		return;
	}
 /*
  * read class BIH
  */
	c_bihpp = bihread(parm.c_fd);
	if (c_bihpp == NULL) {
		error("can't read basic image header");
	}
 /*
  * check class image sizes
  */
	if (parm.nlines != bih_nlines(c_bihpp[0])) {
		error("input and class image must have same # lines");
	}

	if (parm.nsamps != bih_nsamps(c_bihpp[0])) {
		error("input and class image must have same # samples");
	}

	if (bih_nbands(c_bihpp[0]) != 1) {
		error("class image must have only 1 band");
	}

	parm.c_nclasses = ipow2(bih_nbits(c_bihpp[0]));
 /*
  * skip remaining class headers
  */
	skiphdrs(parm.c_fd);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mstats/RCS/headers.c,v 1.2 90/11/11 17:06:49 frew Exp $";

#endif
