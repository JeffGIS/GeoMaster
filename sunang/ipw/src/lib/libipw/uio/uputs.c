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
**	uputs -- write a string to a UNIX file descriptor
**
** SYNOPSIS
**	int uputs(fd, buf)
**	int fd;
**	char *buf;
**
** DESCRIPTION
**	uputs writes an EOS-terminated string from buf to fd.  The trailing
**	EOS is not written.
**
** RESTRICTIONS
**
** RETURN VALUE
**	number of characters written, or ERROR for errors or end-of-file.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	Use uputs for efficient output of ASCII text (e.g., image headers)
**	to IPW data files.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
DEFUN(uputs, (fd, buf),
	int             fd		/* input file descriptor	 */
   AND  CONST char     *buf)		/* -> input buffer		 */
{
	int             nbytes;		/* #bytes to read		 */

	assert(buf != NULL);

	nbytes = strlen(buf);
	if (nbytes > 0) {
		nbytes = uwrite(fd, (CONST addr_t) buf, nbytes);
	}

	return (nbytes);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/RCS/uputs.c,v 1.3 90/11/11 17:18:56 frew Exp $";

#endif
