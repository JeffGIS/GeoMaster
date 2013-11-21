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

#include "skewh.h"

/*
** NAME
**	skewhmake -- make an IPW SKEWH header
**
** SYNOPSIS
**	#include "skewh.h"
**
**	SKEWH_T *skewhmake(angle)
**	double angle;
**
** DESCRIPTION
**	skewhmake allocates an IPW SKEW header.  The header is initialized with
**	the remaining arguments.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to new SKEW header; NULL if error
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

SKEWH_T        *
skewhmake(angle)
	double          angle;		/* skew angle (degrees)		 */
{
	SKEWH_T        *skewhp;		/* -> SKEW header		 */

	assert(angle >= SKEW_MIN && angle <= SKEW_MAX);
 /*
  * allocate header
  */
 /* NOSTRICT */
	skewhp = (SKEWH_T *) hdralloc(1, sizeof(SKEWH_T), ERROR, SKEWH_HNAME);
	if (skewhp == NULL) {
		return (NULL);
	}
 /*
  * initialize header
  */
	skewhp->angle = angle;

	return (skewhp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/skewh/RCS/skewhmake.c,v 1.2 90/11/11 17:17:45 frew Exp $";

#endif
