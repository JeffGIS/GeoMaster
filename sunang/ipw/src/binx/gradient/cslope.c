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
** compute slopes
**
**	Note that tan S = sqrt(dx*dx + dy*dy)
**	If (tanS == a)
**		sinS = sqrt(a/(1+a));
*/

#include <math.h>
#include "ipw.h"
#include "fpio.h"

void
cslope(n, dx, dy, s)
	REG_4 int       n;		/* length of vectors	 */
	REG_1 fpixel_t *dx;		/* vector of df/dx	 */
	REG_2 fpixel_t *dy;		/* vector of df/dy	 */
	REG_3 fpixel_t *s;		/* vector of sinS	 */
{
	FREG_1 fpixel_t a;

	while (--n >= 0) {
		if (*dx != 0 || *dy != 0) {
			a = *dx * *dx + *dy * *dy;
			*s++ = sqrt(a / (1 + a));
		}
		else {
			*s++ = 0;
		}
		++dx;
		++dy;
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/cslope.c,v 1.3 90/11/11 17:03:08 frew Exp $";

#endif
