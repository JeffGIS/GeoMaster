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

#include "sath.h"

/*
** NAME
**	sathcheck -- validate components of IPW SATH header
**
** SYNOPSIS
**	#include "sath.h"
**
**	bool_t sathcheck(sathpp, nbands)
**	SATH_T **sathpp;
**	int nbands;
**
** DESCRIPTION
**	sathcheck checks that sathpp points to an array of nbands pointers
**	to valid SATH headers.
**
** RESTRICTIONS
**
** RETURN VALUE
**	TRUE if sathpp checks OK, else FALSE.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	sathcheck is not meant to be called by IPW application programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

bool_t
sathcheck(sathpp, nbands)
	SATH_T         **sathpp;		/* -> array of -> SATH header	 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	bool_t          found;		/* ? found at least 1 header	 */

	assert(sathpp != NULL);
 /*
  * loop through possible bands
  */
	found = FALSE;

	for (band = 0; band < nbands; ++band) {
		SATH_T          *sathp;	/* -> SATH for current band	 */

		sathp = sathpp[band];
		if (sathp == NULL) {
			continue;
		}

		found = TRUE;

 /*
  * header must have at least one field
  */
		if (sathp->platform != NULL) {
			return (TRUE);
		}
		if (sathp->sensor != NULL) {
			return (TRUE);
		}
		if (sathp->location != NULL) {
			return (TRUE);
		}
		if (sathp->gmdate != NULL) {
			return (TRUE);
		}
		if (sathp->gmtime != NULL) {
			return (TRUE);
		}
		
	}

	assert(found);

	return (FALSE);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/sath/RCS/sathcheck.c,v 1.2 90/11/11 17:17:28 frew Exp $";

#endif
