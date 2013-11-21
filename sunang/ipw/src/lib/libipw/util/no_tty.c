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

#include "getargs.h"

/*
** NAME
**	no_tty -- exit if file descriptor is a terminal
**
** SYNOPSIS
**	void no_tty(fd)
**	int fd;
**
** DESCRIPTION
**	If the argument file descriptor is connected to a terminal, then
**	no_tty calls either usage() (if the file descriptor is the standard
**	input) or error() (for all other cases).
**
** WARNINGS
**	No_tty causes program termination if fd is connected to a terminal.
**
** APPLICATION USAGE
**	Call no_tty to guarantee that an image file descriptor is not attached
**	to a terminal.
**
** BUGS
*/

void
no_tty(fd)
	int             fd;		/* file descriptor		 */
{
	if (isatty(fd)) {
		if (fd == FD_STDIN) {
			usage();
		}

		if (fd == FD_STDOUT) {
			error("can't write image data to a terminal");
		}

		error("can't do image I/O on a terminal device");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/util/RCS/no_tty.c,v 1.4 90/11/11 17:19:53 frew Exp $";

#endif
