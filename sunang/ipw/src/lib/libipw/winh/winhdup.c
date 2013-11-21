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
**	winhdup -- duplicate an IPW WINH header
**
** SYNOPSIS
**	#include "winh.h"
**
**	WINH_T **name(old)
**	WINH_T **old;
**
** DESCRIPTION
**	winhdup creates a duplicate of the IPW WINH header pointed to by
**	"old".
**
** RESTRICTIONS
**
** RETURN VALUE
**	A pointer to the duplicate header is returned.  NULL is returned if
**	an error occurs.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	winhdup is an easy way for applications programs to create a new WINH
**	(e.g., for an output image, after having read the WINH for an input
**	image).
**
** FUTURE DIRECTIONS
**
** BUGS
*/

WINH_T        **
winhdup(oldhpp, nbands)
	WINH_T        **oldhpp;		/* -> old WINH array		 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	WINH_T        **newhpp;		/* -> new WINH array		 */

 /*
  * source WINH must be valid
  */
	assert(winhcheck(oldhpp, nbands));
 /*
  * allocate new pointer array
  */
 /* NOSTRICT */
	newhpp = (WINH_T **) hdralloc(nbands, sizeof(WINH_T *), ERROR,
				      WINH_HNAME);
	if (newhpp == NULL) {
		return (NULL);
	}
 /*
  * duplicate headers
  */
	for (band = 0; band < nbands; ++band) {
		WINH_T         *newhp;	/* -> new WINH			 */
		WINH_T         *oldhp;	/* -> new WINH			 */

		oldhp = oldhpp[band];
 /* NOSTRICT */
		newhp = (WINH_T *) hdralloc(1, sizeof(WINH_T), ERROR,
					    WINH_HNAME);
		if (newhp == NULL) {
			return (NULL);
		}
 /*
  * duplicate scalar fields
  */
 /* NOSTRICT */
		bcopy((char *) oldhp, (char *) newhp, sizeof(WINH_T));
#if 0
 /*
  * duplicate string fields
  */
		if (oldhp->yy != NULL && (newhp->yy = hstrdup(oldhp->yy,
					 WINH_HNAME, band)) == NULL) {
			return (NULL);
		}
#endif
		newhpp[band] = newhp;
	}

	return (newhpp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/winh/RCS/winhdup.c,v 1.4 90/11/11 17:20:09 frew Exp $";

#endif
