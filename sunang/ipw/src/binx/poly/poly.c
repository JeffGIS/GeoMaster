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

#define	INC_ALLOC	5000

#ifdef	DEBUG
static void
dumparray(a, nx, ny)
	int             nx;
	int             ny;
	char          **a;
{
	int             i;

	for (i = 0; i < nx; ++i) {
		int             j;

		for (j = 0; j < ny; ++j) {
			putchar((a[i][j] == MARK) ? '*' : '.');
		}

		putchar('\n');
	}
}

#endif

void
poly(fpi, sflag, xseed, yseed)
	FILE           *fpi;
	bool_t          sflag;
	int             xseed;
	int             yseed;
{
	int             i;
	int             j;
	unsigned        maxverts;
	int             nverts;
	int             x;
	int             xmax;
	int             xmin;
	int             xrange;
	int            *xvert;
	int             y;
	int             ymax;
	int             ymin;
	int             yrange;
	int            *yvert;

 /*
  * allocate vertex arrays
  */
	maxverts = INC_ALLOC;

 /* NOSTRICT */
	xvert = (int *) malloc(maxverts * sizeof(int));
	if (xvert == NULL) {
		error("can't allocate X-vertices array");
	}

 /* NOSTRICT */
	yvert = (int *) malloc(maxverts * sizeof(int));
	if (yvert == NULL) {
		error("can't allocate Y-vertices array");
	}
 /*
  * read vertices
  */
	xmin = INT_MAX;
	ymin = INT_MAX;
	xmax = INT_MIN;
	ymax = INT_MIN;
	nverts = 0;

	for (nverts = 0; fscanf(fpi, "%d %d", &x, &y) == 2; ++nverts) {
 /*
  * compute bounding rectangle
  */
		if (x < xmin) {
			xmin = x;
		}

		if (x > xmax) {
			xmax = x;
		}

		if (y < ymin) {
			ymin = y;
		}

		if (y > ymax) {
			ymax = y;
		}

		if (nverts >= maxverts) {
 /*
  * extend vertex arrays
  */
			maxverts += INC_ALLOC;

 /* NOSTRICT */
			xvert = (int *) realloc((char *) xvert,
						maxverts * sizeof(int));
			if (xvert == NULL) {
				error("can't enlarge X-vertices array");
			}

 /* NOSTRICT */
			yvert = (int *) realloc((char *) yvert,
						maxverts * sizeof(int));
			if (yvert == NULL) {
				error("can't enlarge Y-vertices array");
			}
		}

		xvert[nverts] = x;
		yvert[nverts] = y;
	}

	if (!feof(fpi)) {
		error("bad input");
	}

	if (nverts < 1) {
		return;
	}
 /*
  * allocate array for rasterized vectors
  */
	xrange = xmax - xmin + 1;
	yrange = ymax - ymin + 1;

 /* NOSTRICT */
	buf = (char **) allocnd(sizeof(char), 2, xrange, yrange);
	if (buf == NULL) {
		error("can't allocate array for rasterized vectors");
	}
 /*
  * rasterize vectors
  */
	for (i = 0, j = 1; j < nverts; ++i, ++j) {
		bresenham(xvert[i] - xmin, yvert[i] - ymin,
			  xvert[j] - xmin, yvert[j] - ymin);
	}

#ifdef	DEBUG
	dumparray(buf, xrange, yrange);
#endif

 /*
  * optional polygon fill
  */
	if (sflag) {
		if (xvert[0] != xvert[nverts - 1] ||
		    yvert[0] != yvert[nverts - 1]) {
			error("input figure not closed: can't fill");
		}

		if (xseed < xmin || xseed > xmax ||
		    yseed < ymin || yseed > ymax) {
			error("seed (%d, %d) is outside of polygon",
			      xseed, yseed);
		}

		fill(xseed - xmin, yseed - ymin);
	}
	SAFE_FREE(xvert);
	SAFE_FREE(yvert);

#ifdef	DEBUG
	dumparray(buf, xrange, yrange);
#endif

 /*
  * print rasterized coordinates
  */
	for (i = 0; i < xrange; ++i) {
		for (j = 0; j < yrange; ++j) {
			if (buf[i][j] == MARK) {
				(void) printf("%d %d\n", i + xmin, j + ymin);
			}
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/poly/RCS/poly.c,v 1.7 90/11/11 17:07:47 frew Exp $";

#endif
