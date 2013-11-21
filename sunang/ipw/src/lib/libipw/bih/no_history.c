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

#include "_nohist.h"

/*
** NAME
**	no_history -- turn off history mechanism in bihwrite
**
** SYNOPSIS
**	void no_history(fd)
**	int fd;
**
** DESCRIPTION
**	no_history turns off the history mechanism.  If called for file
**	descriptor "fd" BEFORE bihwrite is called for the same file
**	descriptor, the normal printing of the history by bihwrite will
**	be suppressed.
**
** RESTRICTIONS
**
** RETURN VALUE
**	number of bands in image
**
** GLOBALS ACCESSED
**	_no_hist[fd]	flag associated with file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	no_history is normally called by programs (e.g. mux) that would
**	otherwise produce voluminous history records
**
** FUTURE DIRECTIONS
**
** BUGS
*/

void
no_history(fd)
	int             fd;		/* image file descriptor	 */
{
	ASSERT_OK_FD(fd);
	_no_hist[fd] = 1;
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/bih/RCS/no_history.c,v 1.3 90/11/11 17:14:12 frew Exp $";

#endif
