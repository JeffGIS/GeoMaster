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
#include "getargs.h"
#include "gethdrs.h"
#include "pixio.h"

main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_n = {
		'n', "# pixel-vectors / buffer",
		INT_OPTARGS, "#pixv",
		REQUIRED, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_n,
		0
	};

	int             band;
	BIH_T         **bihpp;
	pixel_t        *buf;
	pixel_t        *bufp;
	int             fdi;
	int             line;
	int             nbands;
	int             ngot;
	int             nlines;
	int             npixels;
	int             nsamps;
	int             samp;

	ipwenter(argc, argv, optv, "pixio test");
	npixels = int_arg(opt_n, 0);

	fdi = ustdin();

	bihpp = bihread(fdi);
	if (bihpp == NULL) {
		error("can't read BIH");
	}

	skiphdrs(fdi);

	nlines = bih_nlines(bihpp[0]);
	nsamps = bih_nsamps(bihpp[0]);
	nbands = bih_nbands(bihpp[0]);

 /* NOSTRICT */
	buf = (pixel_t *) ecalloc(npixels * nbands, sizeof(pixel_t));
	if (buf == NULL) {
		error("can't allocate pixel buffer");
	}

	line = 0;
	samp = 0;
	band = 0;

	while ((ngot = pvread(fdi, buf, npixels)) > 0) {
		(void) printf("ngot=%d\n", ngot);

		bufp = buf;

		ngot *= nbands;
		do {
			(void) printf("(%d,%d,%d)=%u\n",
				      line, samp, band, *bufp++);

			++band;
			if (band >= nbands) {
				band = 0;

				++samp;
				if (samp >= nsamps) {
					samp = 0;

					++line;
					if (line >= nlines) {
						line = 0;
					}
				}
			}
		} while (--ngot > 0);
	}

	if (ngot < 0) {
		error("pvread error at (%d,%d,%d)", line, samp, band);
	}

	exit(EX_OK);
}

#ifndef lint
static char	rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/pixio/TEST/RCS/pbug.c,v 1.4 90/11/11 17:17:08 frew Exp $";

#endif
