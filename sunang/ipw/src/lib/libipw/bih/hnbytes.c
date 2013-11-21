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
**	hnbytes -- number of bytes in image band
**
** SYNOPSIS
**	int hnbytes(fd, band)
**	int fd, band;
**
** DESCRIPTION
**	hnbytes determines the number of bytes per band in the specified band
**	of the image currently being read/written on file descriptor fd.
**
** RESTRICTIONS
**	hnbytes may not be called before a BIH is read/written from/to fd.
**
** RETURN VALUE
**	number of bytes per band
**
** GLOBALS ACCESSED
**	_bih[fd]	BIH associated with file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	hnbytes may be called by routines that do not have direct access to
**	the corresponding BIH_T pointer.
**
** FUTURE DIRECTIONS
**	hnbytes and related routines will completely replace the BIH at the
**	application level.
**
** BUGS
*/

int
hnbytes(fd, band)
	int             fd;		/* image file descriptor	 */
	int             band;		/* image band #			 */
{
	ASSERT_OK_FD(fd);
	assert(_bih[fd] != NULL);

	assert(band >= 0 && band < bih_nbands(_bih[fd][0]));

	return (bih_nbytes(_bih[fd][band]));
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/bih/RCS/hnbytes.c,v 1.3 90/11/11 17:13:59 frew Exp $";

#endif
