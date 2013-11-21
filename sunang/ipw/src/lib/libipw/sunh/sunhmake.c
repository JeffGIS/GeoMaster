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

#include "sunh.h"

/*
** NAME
**	sunhmake -- make an IPW SUNH header
**
** SYNOPSIS
**	#include "sunh.h"
**
**	SUNH_T *sunhmake(cos_zen, azm)
**	double cos_zen;
**	double azm;
**
** DESCRIPTION
**	Sunhmake allocates an IPW SUN header.  The header is
**	initialized with the remaining arguments.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to new SUN header; NULL if error
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#define TOL	(8 * FLT_EPSILON)

SUNH_T  *
DEFUN( sunhmake, (cos_zen, azm),
	double          cos_zen		/* cosine solar zenith angle	 */
   AND  double          azm)		/* solar azimuth (rad from S)	 */
{
	SUNH_T         *sunhp;		/* -> SUN header		 */
	double          pi;

	pi = 4 * atan(1.);
 /*
  * range of cosine
  */
	if (fabs(cos_zen) > 1) {
		if (fabs(cos_zen) < 1 + TOL) {
			cos_zen = (cos_zen > 1)? 1 : -1;
		}
		else {
			usrerr("sunhmake: bad cosine %g", cos_zen);
			return (NULL);
		}
	}
 /*
  * range of azimuth
  */
	if (fabs(azm) > pi) {
		if (fabs(azm) < pi + TOL) {
			azm = (azm > pi) ? pi : -pi;
		}
		else {
			usrerr("sunhmake: bad azimuth %g", azm);
			return (NULL);
		}
	}
 /*
  * allocate header
  */
 /* NOSTRICT */
	sunhp = (SUNH_T *) hdralloc(1, sizeof(SUNH_T), ERROR, SUNH_HNAME);
	if (sunhp == NULL) {
		return (NULL);
	}
 /*
  * initialize header
  */
	sunhp->cos_sun = cos_zen;
	sunhp->zen_sun = acos(cos_zen);
	sunhp->azm_sun = azm;

	return (sunhp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/sunh/RCS/sunhmake.c,v 1.3 90/11/11 17:18:08 frew Exp $";

#endif
