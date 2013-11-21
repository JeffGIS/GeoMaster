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
 * trigonometric look-up table
 */

#include <math.h>
#include "ipw.h"
#include "fpio.h"

void
trigtbl(fdi, phi, sintbl, costbl, cosdtbl)
	int             fdi;		/* input file descriptor	 */
	double          phi;		/* solar illumination aziumth	 */
	fpixel_t       *sintbl;		/* all possible values of sinS	 */
	float          *costbl;		/* all possible values of cosS	 */
	float          *cosdtbl;	/* all values of cos(phi-A)	 */
{
	FREG_1 double   x;		/* sinS or phi			 */
	REG_1 unsigned  j;		/* loop counter and B.F.	 */
	REG_2 float    *fp1;
	REG_3 fpixel_t *bfp;
	REG_4 float    *fp2;
	fpixel_t      **map;
	int            *maplen;

	map = fpmap(fdi);
	maplen = fpmaplen(fdi);

 /*
  * sines and cosines of each possible slope
  */

	j = maplen[0] - 1;
	fp2 = &costbl[j];
	++j;
	do {
		--j;
		x = sintbl[j];
		*fp2-- = sqrt((1 - x) * (1 + x));
	} while (j != 0);

 /*
  * cosine differences for each possible azimuth
  */

	j = maplen[1] - 1;
	fp1 = &cosdtbl[j];
	bfp = map[1];
	x = phi;
	++j;
	do {
		--j;
		*fp1-- = cos(x - bfp[j]);
	} while (j != 0);
}

#ifndef lint
static char	rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/shade/RCS/trigtbl.c,v 1.4 90/11/11 17:09:04 frew Exp $";

#endif
