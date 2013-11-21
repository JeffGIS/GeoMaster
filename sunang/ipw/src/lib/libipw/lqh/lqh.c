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

#include "lqh.h"

#include "_lqh.h"

/*
** NAME
**	lqh -- get LQ header associated with file descriptor
**
** SYNOPSIS
**	#include "lqh.h"
**
**	LQH_T **lqh(fd)
**	int fd;
**
** DESCRIPTION
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to the LQ header array associated with file descriptor fd; or
**	NULL if there are no LQ headers associated with file descriptor fd.
**
** GLOBALS ACCESSED
**	_lqh	array of LQ pointers, indexed by file descriptor
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

LQH_T **
lqh(fd)
	int		fd;		/* image file descriptor	*/
{
	ASSERT_OK_FD(fd);
	return (_lqh[fd]);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/lqh/RCS/lqh.c,v 1.2 90/11/11 17:16:31 frew Exp $";

#endif
