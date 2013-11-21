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
#include "io.h"

#include "_uio.h"

/*
** NAME
**	uclose -- close UNIX file descriptor
**
** SYNOPSIS
**	int uclose(fd)
**	int fd;
**
** DESCRIPTION
**	uclose closes file descriptor fd and clears the associated UIO control
**	block.  Any pending buffered output is flushed.
**
** RESTRICTIONS
**
** RETURN VALUE
**	OK for success, ERROR for failure
**
** GLOBALS ACCESSED
**	_uiocb[fd]	UIO control block for file descriptor fd
**
** ERRORS
**
** WARNINGS
**	Pending buffered input is discarded.
**
** APPLICATION USAGE
**	Use uclose wherever you would normally use the UNIX system call
**	close().
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
uclose(fd)
	int             fd;		/* file descriptor		 */
{
	UIO_T          *p;		/* -> UIO control block		 */

	ASSERT_OK_FD(fd);
	p = &_uiocb[fd];
 /*
  * if already closed then do nothing
  */
	if (p->flags == 0) {
		return (OK);
	}
 /*
  * check to see if this is the right close to be calling
  */

	ASSERT_IO_LEVEL(FTYPE_UIO, p->level);

 /*
  * flush pending output
  *
  * NB: should we drain input too?
  */
	if (uwflush(fd) == ERROR) {
		return (ERROR);
	}
 /*
  * do a UNIX close
  */
	if (close(fd) == SYS_ERROR) {
		syserr();
		uferr(fd);
		return (ERROR);
	}
 /*
  * free dynamic arrays, then clear UIO control block
  */
	SAFE_FREE(p->name);
	SAFE_FREE(p->buf);

 /* NOSTRICT */
	bzero((char *) p, sizeof(*p));

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/RCS/uclose.c,v 1.5 90/11/11 17:18:42 frew Exp $";

#endif
