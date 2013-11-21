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

#include "_uio.h"

/*
** NAME
**	ufilename -- get UNIX file name associated with file descriptor
**
** SYNOPSIS
**	char *ufilename(fd)
**	int fd;
**
** DESCRIPTION
**	ufilename obtains the UNIX filename previously associated with file
**	descriptor fd by uropen or uwopen.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to string containing UNIX file name, or NULL if there is no
**	associated filename
**
** GLOBALS ACCESSED
**	_uiocb[fd]	UIO control block for file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	ufilename may be called by error handlers outside the UIO package, but
**	is not intended for use by IPW application programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

char           *
ufilename(fd)
	int             fd;
{
	ASSERT_OK_FD(fd);

	return (_uiocb[fd].name);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/RCS/ufilename.c,v 1.3 90/11/11 17:18:51 frew Exp $";

#endif
