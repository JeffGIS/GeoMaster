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
**	dysize -- number of days in a year
**
** SYNOPSIS
**	int dysize(year)
**	int year;
**
** DESCRIPTION
**	Dysize calculates the number of days in a given year.
**
** RESTRICTIONS
**	year must be positive.
**
**	The year may not be abbreviated; i.e. 89 is 89 A.D. not 1989.
**
** RETURN VALUE
**	the number of days in the argument year
**
** APPLICATION USAGE
**	dysize is a replacement for the BSD library function of the same name.
**	You should only use this version of dysize if your system doesn't
**	provide one in its standard C libraries.
**
** BUGS
*/

int
dysize(year)
	int             year;
{
//	assert(year > 0);

	return (((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ?
		366 : 365);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/dysize.c,v 1.2 90/11/11 17:20:20 frew Exp $";

#endif
