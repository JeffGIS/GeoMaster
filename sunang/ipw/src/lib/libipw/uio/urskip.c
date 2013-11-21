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
**	urskip -- skip input on UNIX file descriptor
**
** SYNOPSIS
**	long urskip(fd, nbytes)
**	int fd;
**	long nbytes;
**
** DESCRIPTION
**	urskip causes nbytes bytes of input on fd to be skipped.
**
** RESTRICTIONS
**	nbytes must be > 0 (i.e., only forward skips are allowed).
**
** RETURN VALUE
**	number of bytes skipped, or ERROR for failure
**
** GLOBALS ACCESSED
**	_uiocb[fd]	UIO control block for file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	Use urskip instead of the UNIX system call lseek to skip input data.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

long
DEFUN(urskip, (fd, nbytes),
	int             fd		/* input file descriptor	 */
   AND  long            nbytes)		/* #bytes to skip		 */
{
	REG_1 UIO_T    *p;		/* -> uio control block		 */
	REG_2 int       ngot;		/* #bytes got this time around	 */
	REG_3 long      nwant;		/* #bytes still needed		 */

	ASSERT_OK_FD(fd);
	p = &_uiocb[fd];

	assert(p->flags & UIO_READ);
 /*
  * first try to do it by seeking
  */
	if (p->flags & UIO_CANSEEK) {
		if (lseek(p->fd, nbytes - p->nbytes, SEEK_CUR)
		    == (long) SYS_ERROR) {
			syserr();
			uferr(fd);
			return (ERROR);
		}

		p->nbytes = 0;

		return (nbytes);
	}

	ngot = 0;

 /*
  * can't seek, so loop until we've read # bytes requested
  */
	for (nwant = nbytes; nwant > 0; nwant -= ngot) {
		if (p->nbytes <= 0) {
			if (_uread(p) <= 0) {
				break;
			}
		}
 /*
  * skip # bytes available or # bytes requested, whichever is smaller
  */
 /* NOSTRICT */
		ngot = MIN(p->nbytes, nwant);
 /*
  * update UIO buffer pointer and byte count
  */
		p->bufp += ngot;
		p->nbytes -= ngot;
	}
 /*
  * return actual # bytes skipped (or ERROR if most recent read failed)
  */
	if (p->nbytes < 0) {
		return (ERROR);
	}

	return (nbytes - nwant);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/RCS/urskip.c,v 1.8 90/11/11 17:19:06 frew Exp $";

#endif
