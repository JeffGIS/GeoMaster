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

#include "hdrio.h"
#include "bih.h"
#include "orh.h"

/*
** NAME
**	orhwrite -- write an IPW ORH header
**
** SYNOPSIS
**	#include "file.h"
**
**	int orhwrite(fd, orhpp)
**	int fd;
**	ORH_T **orhpp;
**
** DESCRIPTION
**	orhwrite writes the array of ORH headers pointed to by orhpp to file
**	descriptor fd.
**
** RESTRICTIONS
**
** RETURN VALUE
**	OK for success, ERROR for failure
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**      orhwrite is called by IPW application programs to write
**      ORH headers.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
orhwrite(fd, orhpp)
	int             fd;		/* output file descriptor	 */
	ORH_T         **orhpp;		/* -> array of ORH pointers	 */
{
	REG_2 int       band;		/* current header band #	 */
	int             nbands;		/* # bands in output image	 */

	nbands = hnbands(fd);

	assert(orhcheck(orhpp, nbands));
 /*
  * loop through possible output bands
  */
	for (band = 0; band < nbands; ++band) {
		REG_1 ORH_T    *orhp;	/* -> current ORH		 */

		orhp = orhpp[band];
		if (orhp == NULL) {
			continue;
		}
 /*
  * write preamble
  */
		if (hwprmb(fd, ORH_HNAME, band, ORH_VERSION) == ERROR) {
			return (ERROR);
		}
 /*
  * write fields
  */
		if (hputrec(fd, (char *) NULL, ORH_OR, orhp->orient)
		    == ERROR) {
			return (ERROR);
		}

		if (hputrec(fd, (char *) NULL, ORH_ORIGIN, orhp->origin)
		    == ERROR) {
			return (ERROR);
		}
	}

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/orh/RCS/orhwrite.c,v 1.3 90/11/11 17:17:03 frew Exp $";

#endif
