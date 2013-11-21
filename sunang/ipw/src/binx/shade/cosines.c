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
 * read input slope/azimuth data, compute cosine of
 * illumination angle, and write
 */

#include "ipw.h"

#include "shade.h"

void
cosines(fdi, fdo)
	int             fdi;		/* input file desc	 */
	int             fdo;		/* output file desc	 */
{
	REG_1 pixel_t  *sp;		/* -> slope index	 */
	REG_2 int       j;		/* loop counter		 */
	REG_3 pixel_t  *b;		/* output value		 */
	int             ngot;		/* # pixels read	 */
	int             nwrite;		/* # pixels to write	 */

 /* default values */
	nwrite = nget;

 /*
  * read pixels until done
  */

	while ((ngot = pvread(fdi, ibuf, nget)) > 0) {
		if (ngot < nget) {
			nwrite = ngot;
		}

 /* -> input */
		sp = ibuf;

 /* -> output */
		b = obuf;

		for (j = nwrite; --j >= 0;) {

 /*
  * fetch value from look-up table first band is slope, second azimuth
  * (afterward sp -> slope of next sample)
  */

			*b++ = shade[*sp][sp[1]];
			sp += 2;
		}

		if (pvwrite(fdo, obuf, nwrite) != nwrite) {
			error("write error");
		}
	}

	if (ngot != 0) {
		error("read error");
	}
}

#ifndef lint
static char	rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/shade/RCS/cosines.c,v 1.4 90/11/11 17:08:39 frew Exp $";

#endif
