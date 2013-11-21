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
**	ltou -- convert long to unsigned, catching overflow
**
** SYNOPSIS
**	unsigned ltou(i)
**	long i;
**
** DESCRIPTION
**	Ltou converts its long argument "i" to an unsigned.  If "i" won't fit
**	in an unsigned, execution terminates with an IPW error message.
**
** RETURN VALUE
**	The unsigned equivalent of "i" is returned.
**
** ERRORS
**	<i> won't fit in an "unsigned"
**
** WARNINGS
**	Ltou will cause program termination if its argument won't fit in an
**	unsigned.
**
** APPLICATION USAGE
**	Ltou is useful for converting long values in IPW image headers into
**	unsigneds (e.g. for use in memory-allocation functions) without
**	generating scads of superfluous "lint" warnings.
**
** FUTURE DIRECTIONS
**	Ltou should really return 0 and set some external error indicator
**	(errno <- ERANGE?), instead of just dying, if "i" won't fit in an
**	unsigned.  (Until ltou actually fails, I'm not going to worry about
**	this too much).
**
** BUGS
*/

unsigned int
ltou(i)
	long            i;		/* long to be converted to unsigned */
{
	if ( i < 0 ) {
		error("%ld won't fit in an \"unsigned\"", i);
	}
	/* CONSTCOND */
	if (  (sizeof(long) > sizeof(int)) && (i > (unsigned) UINT_MAX)  ) {
		error("%ld won't fit in an \"unsigned\"", i);
	}

 /* NOSTRICT */
	return ((unsigned) i);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/util/RCS/ltou.c,v 1.4 90/11/11 17:19:49 frew Exp $";

#endif
