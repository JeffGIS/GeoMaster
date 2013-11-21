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
#include "orh.h"

/*
** NAME
**	orhmake -- make an IPW ORH header
**
** SYNOPSIS
**	#include "orh.h"
**
**	ORH_T *orhmake(orient, origin)
**	char *orient, *origin;
**
** DESCRIPTION
**	orhmake allocates an OR header.  The header is initialized with the
**	remaining arguments.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to new OR header; NULL if error
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

ORH_T          *
orhmake(orient, origin)
	char           *orient;		/* orientation			 */
	char           *origin;		/* corner of origin		 */
{
	ORH_T          *orhp;		/* -> OR header			 */

 /*
  * allocate header
  */
 /* NOSTRICT */
	orhp = (ORH_T *) hdralloc(1, sizeof(ORH_T), ERROR, ORH_HNAME);
	if (orhp == NULL) {
		return (NULL);
	}
 /*
  * initialize header
  */
	if (orient != NULL && (orhp->orient = hstrdup(orient,
				       ORH_HNAME, NO_BAND)) == NULL) {
		return (NULL);
	}

	if (origin != NULL && (orhp->origin = hstrdup(origin,
				       ORH_HNAME, NO_BAND)) == NULL) {
		return (NULL);
	}


	return (orhp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/orh/RCS/orhmake.c,v 1.6 90/11/11 17:16:58 frew Exp $";

#endif
