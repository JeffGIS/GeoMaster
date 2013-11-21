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
 * convert azimuth in degrees to radians
 * check for error, correct for possible rounding error
 */

#include <math.h>
#include "ipw.h"
#include "horizon.h"

double
DEFUN(azmf, (azd),
	double		azd)		/* azimuth in degrees	*/
{
	double		azimuth;	/* azimuth in radians	*/

	if (fabs(azd) > 180) {
		error("-a %g: must be between -180 and +180", azd);
	}
	azimuth = azd * PI / 180;

	/* correct for slight rounding error */
	if (azimuth > PI) {
		azimuth = PI;
	}
	else if (azimuth < -PI) {
		azimuth = -PI;
	}

	return (azimuth);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/azmf.c,v 1.2 90/11/11 17:04:04 frew Exp $";

#endif
