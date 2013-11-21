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

#include "bih.h"

#include "_bih.h"

/*
** NAME
**	hnsamps -- number of samples in image line
**
** SYNOPSIS
**	int hnsamps(fd)
**	int fd;
**
** DESCRIPTION
**	hnsamps determines the number of samples in the image currently being
**	read/written on file descriptor fd.
**
** RESTRICTIONS
**	hnsamps may not be called before a BIH is read/written from/to fd.
**
** RETURN VALUE
**	number of samples in image line
**
** GLOBALS ACCESSED
**	_bih[fd]	BIH associated with file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	hnsamps may be called by routines that do not have direct access to
**	the corresponding BIH_T pointer.
**
** FUTURE DIRECTIONS
**	hnsamps and related routines will completely replace the BIH at the
**	application level.
**
** BUGS
*/

int
hnsamps(fd)
	int             fd;		/* image file descriptor	 */
{
	ASSERT_OK_FD(fd);
	assert(_bih[fd] != NULL);

	return (bih_nsamps(_bih[fd][0]));
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/bih/RCS/hnsamps.c,v 1.5 90/11/11 17:14:03 frew Exp $";

#endif
