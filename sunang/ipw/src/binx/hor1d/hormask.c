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
**	Calculate whether tan(angle to horizon) is >= thresh.
**	If it is, sun is hidden and mask is zero, otherwise mask is non-zero
*/

#include <math.h>
#include "ipw.h"
#include "fpio.h"
#include "horizon.h"

void
DEFUN(hormask, (n, z, delta, h, thresh, hmask),
	int             n		/* length of horizon vector	 */
   AND  fpixel_t       *z		/* elevations			 */
   AND  fpixel_t        delta		/* spacing			 */
   AND  int            *h		/* horizon function		 */
   AND  fpixel_t        thresh		/* threshold			 */
   AND  pixel_t        *hmask)		/* output mask			 */
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
			*hmask++ = 1;
		}

 /* else need to compare with threshold */
		else {
			if (d < 0)
				d = -d;
			diff = z[j] - z[i];
			*hmask++ = (diff / (d * delta) > thresh) ? 0 : 1;
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/hormask.c,v 1.2 90/11/11 17:04:22 frew Exp $";

#endif
