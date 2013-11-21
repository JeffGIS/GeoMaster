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
#include "hdrio.h"

/*
** NAME
**	horhmake -- make an IPW HORH header
**
** SYNOPSIS
**	#include "horh.h"
**
**	HORH_T *horhmake(azimuth)
**	double azimuth;
**
** DESCRIPTION
**	Horhmake allocates a horizon header.  The header is initialized
**	with the azimuth argument.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to new HORH header; NULL if error
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

#define TOL	(16 * FLT_EPSILON)

HORH_T         *
horhmake(azm)
	double          azm;		/* direction to horizon		 */
{
	HORH_T         *horhp;		/* -> HORH array		 */
	double          pi;

	pi = 4 * atan(1.);

	if (fabs(azm) > pi) {
		if (fabs(azm) < pi + TOL) {
			if (azm > 0)
				azm = pi;
			else
				azm = -pi;
		}
		else {
			usrerr("horhmake: abs(azm) (%g) > pi", azm);
			return (NULL);
		}
	}
 /*
  * allocate header
  */
 /* NOSTRICT */
	horhp = (HORH_T *) hdralloc(1, sizeof(HORH_T), ERROR, HORH_HNAME);
	if (horhp == NULL) {
		return (NULL);
	}

	horhp->azimuth = azm;

	return (horhp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/horh/RCS/horhmake.c,v 1.3 90/11/11 17:16:17 frew Exp $";

#endif
