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

#include "pixio.h"
#include "io.h"
#include "bih.h"

#include "_pixio.h"

/*
** NAME
**	pxclose -- close pixel I/O file descriptor
**
** SYNOPSIS
**	#include "pixio.h"
**
**	int pxclose(fd)
**	int fd;
**
** DESCRIPTION
**	pxclose resets all pixel I/O information associated with file
**	descriptor fd, then calls uclose to close the associated file.
**
** RESTRICTIONS
**
** RETURN VALUE
**	OK for success, ERROR for failure
**
** GLOBALS ACCESSED
**	_piocb[fd]	pixel I/O control block for file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	Call pxclose if you need to reclaim a file descriptor that has been
**	read|written by pvread|pvwrite.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/10/95  Added call to bihfree.  Dana Jacobsen, ERL-C.
**
** BUGS
*/

int
pxclose(fd)
	int		fd;		/* file descriptor		*/
{
	PIXIO_T		*p;		/* -> pixel I/O control block	*/
	int		level;
	BIH_T          **bihpp;

	ASSERT_OK_FD(fd);

 /*
  * Is this the right call to be making?
  */
	level = ugetlevel(fd);
	ASSERT_IO_LEVEL(FTYPE_PIXIO, level);
/*
	if (level != FTYPE_PIXIO) {
		if (level == FTYPE_UIO) {
			error("Calling pxclose instead of uclose");
		} else if (level == FTYPE_FPIO) {
			error("Calling pxclose instead of fpclose");
		} else {
			error("Calling uclose for unknown I/O level!");
		}
	}
*/

	p = _piocb[fd];
	assert (p != NULL);

	SAFE_FREE(p->rawbuf);
 /* NOSTRICT */
	SAFE_FREE((char *) p->pixsiz);
 /* NOSTRICT */
	SAFE_FREE((char *) p->bandsiz);
 /* NOSTRICT */
	SAFE_FREE((char *) p->pixmask);
 /* NOSTRICT */
	SAFE_FREE((char *) p->bandmask);

 /*
  * now free the BIH information
  */
	bihpp = bih(fd);
	if (bihpp != NULL) {
		bihfree(bihpp);
	}

 /* NOSTRICT */
	SAFE_FREE((char *) p);

	_piocb[fd] = NULL;

 /*
  * We've done our work, so the correct level for this file is longer pixio.
  */
	usetlevel(fd, -FTYPE_UIO);
	return (uclose(fd));
}

#ifndef	lint
static char	rcsid[]	= "$Header: /local/share/pkg/ipw/src/lib/libipw/pixio/RCS/pxclose.c,v 1.8 90/11/11 17:17:25 frew Exp $";
#endif
