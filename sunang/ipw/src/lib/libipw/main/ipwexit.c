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
**	ipwexit -- terminate an IPW program
**
** SYNOPSIS
**	void ipwexit(status)
**	int status;
**
** DESCRIPTION
**	Ipwexit terminates the execution of an IPW program.  Open UIO files
**	are flushed and closed.  Status is passed to the operating system via
**	exit().
**
** RESTRICTIONS
**
** GLOBALS ACCESSED
**
** WARNINGS
**	Ipwexit never returns.
**
** APPLICATION USAGE
**	All IPW applications programs should call ipwexit as their last
**	executable statement.
**
** FUTURE DIRECTIONS
**	Additional cleanup actions may be incorporated into ipwexit().
**
** BUGS
*/

void
DEFUN(ipwexit, (status),
	int             status)		/* exit status			 */
{
	static bool_t   called = FALSE;	/* ? called already		 */

	if (!called) {
		REG_1 int       fd;	/* UNIX file descriptor		 */
		int		maxfd;	/* maximum file descriptor	 */

 /*
  * prevent recursion (e.g. if we call error())
  */
		called = TRUE;
 /*
  * do per-file-descriptor cleanup actions
  */
		maxfd = maxfiles();
		/*
		 * Remember, OPEN_MAX is the guaranteed number of open files,
		 * but OPEN_LIMIT is the IPW limit (i.e. this is the size of
		 * the static arrays, so even if the O/S gives us 5 million
		 * open files, we can't use them all.
		 */
		if (maxfd > OPEN_LIMIT)
			maxfd = OPEN_LIMIT - 1;

		for (fd = 0; fd < maxfd; ++fd) {
 /*
  * flush pending UIO output
  */
			if (uwflush(fd) == ERROR) {
				error("can't flush UIO buffer");
			}
		}
	}
 /*
  * terminate execution
  */
	exit(status);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/main/RCS/ipwexit.c,v 1.7 90/11/11 17:16:51 frew Exp $";

#endif
