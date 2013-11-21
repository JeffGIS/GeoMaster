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
 * create new LQH
 */

#include <math.h>

#include "ipw.h"
#include "bih.h"
#include "lqh.h"

LQH_T         **
newlqh(fdo, slope, aspect)
	int             fdo;		/* output file desc		 */
	bool_t          slope;		/* ? compute slope 		 */
	bool_t          aspect;		/* ? compute aspect 		 */
{
	LQH_T         **lqhpp;		/* -> output LQH		 */
	fpixel_t        fbkpt[2][2];	/* limits of LQ			 */
	pixel_t         ibkpt[2][2];	/* integer break points		 */
	char          **units;		/* units			 */
	double          pi;
	int             j;

 /*
  * preamble
  */
 /* NOSTRICT */
	units = (char **) ecalloc(2, sizeof(char *));
	if (units == NULL) {
		return (NULL);
	}
	pi = 4 * atan(1.0);
 /*
  * if slope to be computed
  */
	if (slope) {
		fbkpt[0][0] = 0;
		fbkpt[0][1] = 1;
		units[0] = NULL;
		if (aspect) {
			fbkpt[1][1] = pi *
				(1 - ldexp(1.0, 1 - hnbits(fdo, 1)));
			fbkpt[1][0] = -pi;
			units[1] = "radians";
		}
	}
 /*
  * must be aspects only
  */
	else if (aspect) {
		fbkpt[0][1] = pi *
			(1 - ldexp(1.0, 1 - hnbits(fdo, 0)));
		fbkpt[0][0] = -pi;
		units[0] = "radians";
	}
 /*
  * programmer brain damage
  */
	else {
		bug("neither slope nor aspect");
	}
 /*
  * return the new LQH
  */
	ibkpt[0][0] = ibkpt[1][0] = 0;
	ibkpt[0][1] = ipow2(hnbits(fdo, 0)) - 1;
	if (hnbands(fdo) > 1)
		ibkpt[1][1] = ipow2(hnbits(fdo, 1)) - 1;
 /* NOSTRICT */
	lqhpp = (LQH_T **) hdralloc(hnbands(fdo), sizeof(LQH_T *),
				    fdo, LQH_HNAME);
	for (j = hnbands(fdo); --j >= 0;)
		lqhpp[j] = lqhmake(hnbits(fdo, j), 2,
				ibkpt[j], fbkpt[j], units[j], (char *) NULL);

	SAFE_FREE(units);

	return (lqhpp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/newlqh.c,v 1.4 90/11/11 17:03:30 frew Exp $";

#endif
