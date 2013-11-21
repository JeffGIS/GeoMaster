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

#include "_strvec.h"

/*
** NAME
**	dupsv -- duplicate a strvec
**
** SYNOPSIS
**	STRVEC_T *name(p)
**	STRVEC_T *p;
**
** DESCRIPTION
**	dupsv duplicates the strvec pointed to by p.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to the duplicate strvec, or NULL if any memory allocations
**	fail.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	dupsv is used by the IPW image header duplication routines to
**	duplicate strvec header values.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include "ipw.h"

STRVEC_T       *
dupsv(p)
	REG_1 STRVEC_T *p;
{
	REG_3 int       i;
	REG_2 STRVEC_T *newp;

	assert(ok_sv(p));
 /*
  * allocate new strvec
  */
 /* NOSTRICT */
	newp = (STRVEC_T *) ecalloc(1, sizeof(STRVEC_T));
	if (newp == NULL) {
		return (NULL);
	}

	newp->n = p->n;
	newp->curr = p->curr;
 /*
  * allocate string vector
  */
 /* NOSTRICT */
	newp->v = (char **) ecalloc(p->n, sizeof(char *));
	if (newp->v == NULL) {
		return (NULL);
	}
 /*
  * duplicate strings
  */
	for (i = 0; i < p->n && p->v[i] != NULL; ++i) {
		newp->v[i] = strdup(p->v[i]);
		if (newp->v[i] == NULL) {
			return (NULL);
		}
	}

	return (newp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/strvec/RCS/dupsv.c,v 1.3 90/11/11 17:17:59 frew Exp $";

#endif
