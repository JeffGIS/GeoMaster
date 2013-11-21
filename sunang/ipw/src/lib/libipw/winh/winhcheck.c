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

#include "winh.h"

/*
** NAME
**	winhcheck -- validate components of IPW WINH header
**
** SYNOPSIS
**	#include "winh.h"
**
**	bool_t winhcheck(winhpp, nbands)
**	WINH_T **winhpp;
**	int nbands;
**
** DESCRIPTION
**	winhcheck checks that winhpp points to an array of nbands pointers
**	to valid WINH headers.
**
** RESTRICTIONS
**
** RETURN VALUE
**	TRUE if winhpp checks OK, else FALSE.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	winhcheck is not meant to be called by IPW application programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

bool_t
winhcheck(winhpp, nbands)
	WINH_T         **winhpp;	/* -> array of -> WINH header	 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	bool_t          found;		/* ? found at least 1 header	 */

	assert(winhpp != NULL);
 /*
  * loop through possible bands
  */
	found = FALSE;

	for (band = 0; band < nbands; ++band) {
		WINH_T         *winhp;	/* -> WINH for current band	 */

		winhp = winhpp[band];
		if (winhp == NULL) {
			continue;
		}

		found = TRUE;

		if (winhp->dline == 0.0) {
			usrerr(
			   "\"%s\" header, band %d: %g: bad line increment",
			       WINH_HNAME, band, winhp->dline);
			return (FALSE);
		}

		if (winhp->dsamp == 0.0) {
			usrerr(
			 "\"%s\" header, band %d: %g: bad sample increment",
			       WINH_HNAME, band, winhp->dsamp);
			return (FALSE);
		}
	}

	assert(found);

	return (TRUE);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/winh/RCS/winhcheck.c,v 1.5 90/11/11 17:20:06 frew Exp $";

#endif
