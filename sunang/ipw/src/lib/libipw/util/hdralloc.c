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
**	hdralloc -- allocate an IPW image header
**
** SYNOPSIS
**	addr_t hdralloc(n, size, fd, name)
**	int n, fd;
**	unsigned size;
**	char *name;
**
** DESCRIPTION
**	hdralloc allocates a n objects of size bytes each.  If the allocation
**	fails, then "name" and "fd" (if not equal to ERROR) are used to
**	post error messages with usrerr() and uferr(), respectively.
**
**	Example:
**		xxhpp = hdralloc(nbands, sizeof(XXH_T *), fd, XXH_HNAME);
**		xxhp = hdralloc(1, sizeof(XXH_T), fd, XXH_HNAME);
**
** RESTRICTIONS
**
** RETURN VALUE
**	generic pointer to newly allocated header; NULL if allocation failed.
**
** GLOBALS ACCESSED
**
** ERRORS
**	can't allocate {name} header
**
** WARNINGS
**
** APPLICATION USAGE
**	hdralloc may be called by the header ingest functions ({xx}hread) to
**	allocate both individual headers and an array of header pointers.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

addr_t
hdralloc(n, size, fd, name)
	int             n;
	int             size;
	int             fd;
	CONST char     *name;
{
	addr_t          p;

	assert(n != 0);

	p = ecalloc(n, size);
	if (p == NULL) {
		if (fd != ERROR) {
			uferr(fd);
		}

		usrerr(n == 1 ?
		       "can't allocate \"%s\" header" :
		       "can't allocate array of \"%s\" header pointers",
		       name);
	}

	return (p);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/util/RCS/hdralloc.c,v 1.3 90/11/11 17:19:32 frew Exp $";

#endif
