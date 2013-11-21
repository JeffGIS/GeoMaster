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

#include "orh.h"

/*
** NAME
**	orhcheck -- validate components of IPW orientation header
**
** SYNOPSIS
**	#include "orh.h"
**
**	bool_t orhcheck(orhpp, nbands)
**	ORH_T **orhpp;
**	int nbands;
**
** DESCRIPTION
**	orhcheck checks that orhpp points to an array of nbands pointers
**	to valid ORH headers.
**
** RESTRICTIONS
**
** RETURN VALUE
**	TRUE if orhpp checks OK, else FALSE.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	orhcheck is not meant to be called by IPW application programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

bool_t
orhcheck(orhpp, nbands)
	ORH_T         **orhpp;		/* -> array of -> ORH header	 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	bool_t          found;		/* ? found at least 1 header	 */

	assert(orhpp != NULL);
 /*
  * loop through possible bands
  */
	found = FALSE;

	for (band = 0; band < nbands; ++band) {
		ORH_T          *orhp;	/* -> ORH for current band	 */

		orhp = orhpp[band];
		if (orhp == NULL) {
			continue;
		}

		found = TRUE;
 /*
  * check orientation
  */
		if (orhp->orient == NULL) {
			usrerr("\"%s\" header, band %d: missing orientation",
			       ORH_HNAME, band);
			return (FALSE);
		}

		if (strdiff(orhp->orient, ROW) &&
		    strdiff(orhp->orient, COLUMN)) {
			usrerr(
			   "\"%s\" header, band %d: bad orientation \"%s\"",
			       ORH_HNAME, band, orhp->orient);
			return (FALSE);
		}
 /*
  * check origin
  */
		if (orhp->origin == NULL) {
			usrerr("\"%s\" header, band %d: missing origin",
			       ORH_HNAME, band);
			return (FALSE);
		}

		if (strdiff(orhp->origin, ORIG_1) &&
		    strdiff(orhp->origin, ORIG_2) &&
		    strdiff(orhp->origin, ORIG_3) &&
		    strdiff(orhp->origin, ORIG_4)) {
			usrerr("\"%s\" header, band %d: bad origin \"%s\"",
			       ORH_HNAME, band, orhp->orient);
			return (FALSE);
		}
	}

	assert(found);

	return (TRUE);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/orh/RCS/orhcheck.c,v 1.2 90/11/11 17:16:54 frew Exp $";

#endif
