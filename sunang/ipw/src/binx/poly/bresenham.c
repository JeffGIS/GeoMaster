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

#include "ipw.h"

#include "poly.h"

#define	plot(x, y)	( buf[x][y] = MARK )

static int
sign(x)
	int             x;
{
	if (x > 0) {
		return (1);
	}

	if (x < 0) {
		return (-1);
	}

	return (0);
}

void
bresenham(x1, y1, x2, y2)
	int             x1;
	int             x2;
	int             y1;
	int             y2;
{
	REG_3 int       deltax;
	int             deltay;
	REG_2 int       err;
	REG_1 int       i;
	REG_6 int       interchange;
	int             s1;
	int             s2;
	int             tmp;
	REG_4 int       x;
	REG_5 int       y;

	x = x1;
	y = y1;
	deltax = abs(x2 - x1);
	deltay = abs(y2 - y1);
	s1 = sign(x2 - x1);
	s2 = sign(y2 - y1);

	/* interchange deltax,deltay depending on slope of line */

	if (deltay > deltax) {
		tmp = deltax;
		deltax = deltay;
		deltay = tmp;

		interchange = TRUE;
	}
	else {
		interchange = FALSE;
	}

	/* initialize error term to compensate for nonzero intercept */

	err = 2 * deltay - deltax;

	/* main loop */

	for (i = 1; i <= deltax; ++i) {
		plot(x, y);

		while (err >= 0) {
			if (interchange == 1) {
				x += s1;
			}
			else {
				y += s2;
			}

			err -= 2 * deltax;
		}

		if (interchange == 1) {
			y += s2;
		}
		else {
			x += s1;
		}

		err += 2 * deltay;
	}

	/* force last point to be plotted */

	plot(x2, y2);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/poly/RCS/bresenham.c,v 1.3 90/11/11 17:07:34 frew Exp $";

#endif
