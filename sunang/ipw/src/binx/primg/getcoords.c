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

#include "primg.h"

/*
 * getcoords -- read next pair of coordinates
 */

#define	LINELEN		MAX_CHAR

int
getcoords(fd, nlines, nsamps, linep, sampp)
	int             fd;		/* coordinate file descriptor	 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line		 */
	int            *linep;		/* -> line coordinate		 */
	int            *sampp;		/* -> sample coordinate		 */
{
	static char    *buf = NULL;	/* coordinate input buffer	 */
	static double   old_line = 0;	/* previous line coordinate	 */

	double          line;		/* current line coordinate	 */
	double          samp;		/* current sample coordinate	 */

 /*
  * if first time then allocate input buffer
  */
	if (buf == NULL) {
		buf = (char *) ecalloc(LINELEN, sizeof(char));
		if (buf == NULL) {
			error("can't allocate coordinate input buffer");
		}
	}
 /*
  * read line
  */
	if (ugets(fd, buf, LINELEN) == NULL) {
		if (ueof(fd)) {
			return (EOF);
		}

		error("can't read line from coordinate file");
	}
 /*
  * extract coords
  */
	if (sscanf(buf, "%lf %lf", &line, &samp) != 2) {
		error("bad coordinate file line: %s", buf);
	}
 /*
  * make sure coords are sorted by line
  */
	if (line < old_line) {
		error("unsorted coordinates: %g,%g", line, samp);
	}

	old_line = line;
 /*
  * NB: would do coord transform here
  */
	*linep = line;
	*sampp = samp;
 /*
  * make sure coords are valid for this image
  */
	if (*linep < 0 || *linep >= nlines || *sampp < 0 || *sampp >= nsamps) {
		error("bad coordinates (not on image): %g,%g", line, samp);
	}

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/primg/RCS/getcoords.c,v 1.4 90/11/11 17:07:54 frew Exp $";

#endif
