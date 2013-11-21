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

#include <math.h>
#include "ipw.h"

#include "horh.h"

/*
** NAME
**	horhcheck -- validate components of IPW HORH header
**
** SYNOPSIS
**	#include "horh.h"
**
**	bool_t horhcheck(horhpp, nbands)
**	HORH_T **horhpp;
**	int nbands;
**
** DESCRIPTION
**	horhcheck checks that horhpp points to an array of nbands pointers
**	to valid HORH headers.
**
** RESTRICTIONS
**
** RETURN VALUE
**	TRUE if horhpp checks OK, else FALSE.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	horhcheck is not meant to be called by IPW application programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#define TOL (FLT_EPSILON * 16)

bool_t
horhcheck(horhpp, nbands)
	HORH_T        **horhpp;		/* -> array of -> HORH header	 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	bool_t          found;		/* ? found at least 1 header	 */
	double          pi;

	pi = 4 * atan(1.);

	assert(horhpp != NULL);
 /*
  * loop through possible bands
  */
	found = FALSE;

	for (band = 0; band < nbands; ++band) {
		HORH_T         *horhp;	/* -> HORH for current band	 */

		horhp = horhpp[band];
		if (horhp == NULL) {
			continue;
		}

		found = TRUE;
 /*
  * check if azimuth in range, but correct for rounding error
  */
		if (fabs(horhp->azimuth) > pi) {
			if (fabs(horhp->azimuth) < pi + TOL) {
				if (horhp->azimuth > 0)
					horhp->azimuth = pi;
				else
					horhp->azimuth = -pi;
			}
			else {
				usrerr(
				       "\"%s\" header, band %d: %g: bad azimuth",
				    HORH_HNAME, band, horhp->azimuth);
				return (FALSE);
			}
		}
	}

	assert(found);

	return (TRUE);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/horh/RCS/horhcheck.c,v 1.3 90/11/11 17:16:14 frew Exp $";

#endif
