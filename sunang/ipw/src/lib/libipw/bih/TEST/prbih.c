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
prbih(bihpp)
	BIH_T		**bihpp;
{
	int		band;

	for (band = 0; band < bih_nbands(bihpp[0]); ++band) {
		BIH_T          *bihp;	/* -> bihpp[band]		 */

		bihp = bihpp[band];

		fprintf(stderr, "\tband %d:\n", band);

		fprintf(stderr, "\t\tbyteorder = %s\n", bih_byteorder(bihp));
		fprintf(stderr, "\t\tnlines    = %d\n", bih_nlines(bihp));
		fprintf(stderr, "\t\tnpixels   = %d\n", bih_npixels(bihp));
		fprintf(stderr, "\t\tnbands    = %d\n", bih_nbands(bihp));
		fprintf(stderr, "\t\tnbytes    = %d\n", bih_nbytes(bihp));
		fprintf(stderr, "\t\tnbits     = %d\n", bih_nbits(bihp));
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/bih/TEST/RCS/prbih.c,v 1.3 90/11/11 17:13:23 frew Exp $";

#endif
