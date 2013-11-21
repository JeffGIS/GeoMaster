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
**	uwopen -- open UNIX file for writing
**
** SYNOPSIS
**	int uwopen(name)
**	char *name;
**
** DESCRIPTION
**	uwopen opens the file name for writing.  The file is truncated if it
**	already exists.
**
** RESTRICTIONS
**
** RETURN VALUE
**	A writable UNIX file descriptor is returned.  ERROR is returned for
**	failure.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	Use uwopen wherever you would normally use the UNIX system call
**	creat() to open a file for writing.
**
** FUTURE DIRECTIONS
**	A default protection mode (usually -rw-r--r--) is applied to the
**	created file; it may be desirable to allow the caller to override
**	this.
**
** BUGS
*/

extern int EXFUN(creat, (CONST char *path, mode_t mode));

int
uwopen(name)
	CONST char     *name;		/* file name			 */
{
	int             fd;		/* file descriptor		 */

	assert(name != NULL);

	fd = creat(name, CREAT_MODE);
	if (fd == SYS_ERROR) {
		syserr();
		return (ERROR);
	}

	if (_uioinit(fd, name, UIO_WRITE) == ERROR) {
		return (ERROR);
	}

	return (fd);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/RCS/uwopen.c,v 1.4 90/11/11 17:19:16 frew Exp $";

#endif
