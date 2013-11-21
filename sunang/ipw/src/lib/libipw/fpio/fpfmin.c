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

#include "_fpio.h"

/*
** NAME
**	fpfmin -- access fpio minimum fpixel values
**
** SYNOPSIS
**	#include "fpio.h"
**
**	fpixel_t *fpfmin(fd)
**	int fd;
**
** DESCRIPTION
**	fpfmin accesses the array of per-band minimum fpixel values
**	associated with file descriptor fd.
**
** RESTRICTIONS
**	fd must be an image file descriptor.
**
** RETURN VALUE
**	pointer to band-indexed array of minimum fpixel values, or NULL for
**	error.
**
** GLOBALS ACCESSED
**
** ERRORS
**	(see _fpioinit)
**
** WARNINGS
**	If fpfmin is called BEFORE any floating-point header I/O has
**	occurred on fd, then a default pixel<->fpixel conversion will be
**	established, which will be unaffected by subsequent floating-point
**	header I/O.
**
** APPLICATION USAGE
**	IPW programs use fpfmin, in conjunction with fpmap, if they wish to
**	modify the default conversion(s) from pixels to fpixels.  Such
**	programs typically read pixels with pvread, apply the map(s)
**	explicitly, then write fpixels with fpvwrite.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

fpixel_t *
fpfmin(fd)
	int             fd;		/* image file descriptor	 */
{
	FPIO_T         *p;		/* -> fd's fpio control block	 */

	p = _fpioinit(fd, FPIO_NOIO, 0);
	return (p == NULL ? NULL : p->fmin);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/fpio/RCS/fpfmin.c,v 1.2 90/11/11 17:15:00 frew Exp $";

#endif
