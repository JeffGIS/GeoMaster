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

#include "ipw.h"

#include "edimg.h"

/*
 * getedit -- get next image edit coordinates
 */

void
getedit(c_fd, nlines, nsamps, k, eline, esamp, repl)
	int             c_fd;		/* coordinate file descriptor	 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line		 */
	double          k;		/* default replacement val	 */
	int            *esamp;		/* next sample to edit		 */
	int            *eline;		/* next line to edit		 */
	double         *repl;		/* replacement value		 */
{
	static double   old_line = 0;	/* previous edit line		 */

	char            coords[MAX_CHAR];	/* input coord string	 */
	double          line;		/* edit line			 */
	int             ncoords;	/* # coords on input line	 */
	double          samp;		/* edit sample			 */

 /*
  * read line from coordinate file
  */
	if (ugets(c_fd, coords, sizeof(coords)) == NULL) {
 /*
  * if EOF then return with un-matchable edit line coord
  */
		if (ueof(c_fd)) {
			*eline = (-1);
			return;
		}

		error("coordinate read failed");
	}
 /*
  * extract edit coordinates from input line
  */
	ncoords = sscanf(coords, "%lf %lf %lf", &line, &samp, repl);
	if (ncoords == 2) {
		*repl = k;
	}
	else if (ncoords != 3) {
		error("invalid coordinates: %s", coords);
	}
 /*
  * make sure coordinates are sorted by line
  */
	if (line < old_line) {
		error("unsorted coordinates: %g,%g", line, samp);
	}

	old_line = line;
 /*
  * NB: would do coord transform here
  */
	*eline = line;
	*esamp = samp;
 /*
  * make sure coordinates are on the image
  */
	if (*eline < 0 || *eline >= nlines || *esamp < 0 || *esamp >= nsamps) {
		error("bad coordinates (not on image): %g,%g", line, samp);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/edimg/RCS/getedit.c,v 1.4 90/11/11 17:02:19 frew Exp $";

#endif
