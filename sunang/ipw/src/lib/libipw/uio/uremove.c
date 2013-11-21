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
**	uremove -- remove file
**
** SYNOPSIS
**	int uremove(filename)
**	char *filename;
**
** DESCRIPTION
**	remove cause the file whose name is the string pointed to by filename
**	to be removed.  Subsequent attempts to open the file will fail,
**	unless it is created anew.
**
** RESTRICTIONS
**
** RETURN VALUE
**	OK for success, ERROR for failure
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**	If the file is open, then the behavior of remove is
**	implementation-defined.
**
** APPLICATION USAGE
**	uremove is typically used by application programs to delete scratch
**	files.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
DEFUN(uremove, (filename),
	CONST char	*filename)	/* name of file to remove	*/
{
	if (remove(filename) == SYS_ERROR) {
		syserr();
		return (ERROR);
	}

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/RCS/uremove.c,v 1.2 90/11/11 17:19:01 frew Exp $";

#endif
