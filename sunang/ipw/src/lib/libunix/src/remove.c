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

#ifdef	IPW
#include "ipw.h"
#endif

/*
** NAME
**	remove -- remove file
**
** SYNOPSIS
**	int remove(filename)
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
**	SYS_OK for success, SYS_ERROR for failure
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**	If the file is open, then the behavior of remove is
**	implementation-defined.
**
**	THIS IS A PORTABLE UNIX IMPLEMENTATION OF AN ANSI C FUNCTION!  Do not
**	use this implementation on any system which already has this function
**	in its standard C library.
**
** APPLICATION USAGE
**	IPW application programs should use uremove instead of remove.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
remove(filename)
	CONST char     *filename;	/* UNIX file name		 */
{
	return (unlink(filename));
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/remove.c,v 1.3 90/11/11 17:20:29 frew Exp $";

#endif
