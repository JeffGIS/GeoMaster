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

#include <varargs.h>

#ifdef	IPW
#include "ipw.h"
#else
#include <stdio.h>
#endif

/*
** NAME
**	vfprintf -- fprintf with varargs
**
** SYNOPSIS
**	#include <varargs.h>
**
**	int vfprintf(stream, format, ap)
**	FILE *stream;
**	char *format;
**	va_list ap;
**
** DESCRIPTION
**	Vfprintf is the same as fprintf, except that instead of being called
**	with a variable number of arguments, it is called with an argument
**	list as defined by the <varargs.h> header file.
**
** RETURN VALUE
**	(see below)
**
** WARNINGS
**	THIS IS A 4.2BSD IMPLEMENTATION OF A SVID FUNCTION!  Do not use this
**	implementation on any system which already has this function in its
**	standard C library.
**
** BUGS
**	The return value of vfprintf is undefined.  It should be cast to (void)
**	when invoked.
*/

int
vfprintf(stream, format, ap)
	FILE           *stream;
	char           *format;
	va_list         ap;
{
	return (_doprnt(format, ap, stream));
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/vfprintf.c,v 1.5 90/11/11 17:20:40 frew Exp $";

#endif
