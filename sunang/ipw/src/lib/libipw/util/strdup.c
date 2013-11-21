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

/*
** NAME
**	strdup -- duplicate string
**
** SYNOPSIS
**	char *strdup(s)
**	char *s;
**
** DESCRIPTION
**	strdup creates a duplicate copy of the string pointed to by s.  The
**	exact number of bytes needed to store the string and its terminating
**	null character are obtained from ecalloc().
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to the duplicate string, or NULL if ecalloc fails.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	strdup should be used whenever multiple data structures need to access
**	a copy of the same string.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

char *
DEFUN(strdup, (s),
	CONST char     *s)
{
	char           *sdup;

	if (s == NULL) {
		return (NULL);
	}
 /*
  * allocate space for duplicate string
  */
	sdup = (char *) ecalloc((int) strlen(s) + 1, sizeof(char));
	if (sdup == NULL) {
		return (NULL);
	}
 /*
  * copy old string; return -> duplicate
  */
	return (strcpy(sdup, s));
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/util/RCS/strdup.c,v 1.5 90/11/11 17:20:02 frew Exp $";

#endif
