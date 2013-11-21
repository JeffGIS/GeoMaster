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
**	Calculate values of cosines of angles to horizons, measured
**	from zenith, from elevation difference and distance.  Let
**	G be the horizon angle from horizontal and note that:
**
**		sin G = z / sqrt( z^2 + dis^2);
**
**	This result is the same as cos H, where H measured from zenith.
*/

#include <math.h>
#include "ipw.h"
#include "fpio.h"
#include "horizon.h"

extern double EXFUN(hypot, (double x, double y));
 
void
DEFUN(horval, (n, z, delta, h, hcos),
	int             n		/* length of horizon vector	 */
   AND  fpixel_t       *z		/* elevations			 */
   AND  fpixel_t        delta		/* spacing			 */
   AND  int            *h		/* horizon function		 */
   AND  fpixel_t       *hcos)		/* cosines of angles to horizon	 */
{
	int             d;		/* difference in indices	 */
	int             i;		/* index of point		 */
	int             j;		/* index of horizon point	 */
	fpixel_t        diff;		/* elevation difference		 */

	for (i = 0; i < n; ++i) {

 /* # grid points to horizon */
		j = h[i];
		d = j - i;

 /* point is its own horizon */
		if (d == 0) {
			*hcos++ = 0;
		}

 /* else need to calculate sine */
		else {
			if (d < 0)
				d = -d;
			diff = z[j] - z[i];
			*hcos++ = diff / (fpixel_t) hypot(diff, d * delta);
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/horval.c,v 1.3 90/11/11 17:04:25 frew Exp $";

#endif
