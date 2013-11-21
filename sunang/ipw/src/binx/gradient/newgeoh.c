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
 * create new window header and get spacing from old
 */

#include <math.h>

#include "ipw.h"
#include "fpio.h"
#include "geoh.h"

#define TOL1		(1.e-2)
#define TOL2		(2 * FLT_EPSILON)

GEOH_T        **
newgeoh(nbands, fdo, i_geoh, spacing)
	int             nbands;		/* # bands in output image	 */
	int             fdo;		/* output file desc		 */
	GEOH_T        **i_geoh;		/* -> input geodetic header	 */
	fpixel_t       *spacing;	/* grid spacing (m)		 */
{
	double          linespa;	/* line spacing			 */
	double          sampspa;	/* sample spacing		 */
	GEOH_T        **o_geoh;		/* -> output geodetic header	 */
	int             j;

 /*
  * make sure units are meters
  */
	if (strdiffn(geoh_units(i_geoh[0]), "m", 1)) {
		warn("input units \"%s\", should be \"m\"",
		     geoh_units(i_geoh[0]));
	}
 /*
  * don't change spacing if already specified
  */
	if (spacing[0] != 0 && spacing[1] != 0) {
		warn("spacing in geodetic header ignored");
	}
	else {
		linespa = geoh_dline(i_geoh[0]);
		sampspa = geoh_dsamp(i_geoh[0]);
		spacing[0] = fabs(linespa);
		spacing[1] = fabs(sampspa);
	}
 /*
  * make new geodetic header, copy of old
  */
 /* NOSTRICT */
	o_geoh = (GEOH_T **) hdralloc(nbands, sizeof(GEOH_T *), fdo,
				      GEOH_HNAME);
	if (o_geoh != NULL) {
		for (j = 0; j < nbands; ++j) {
			o_geoh[j] = i_geoh[0];
		}
	}

	return (o_geoh);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/newgeoh.c,v 1.5 90/11/11 17:03:28 frew Exp $";

#endif
