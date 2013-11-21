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

#include <math.h>

#include "ipw.h"

/*
** NAME
**	frand -- generate uniform random numbers in range [0,1)
**	frinit -- initialize random number generator
**
** SYNOPSIS
**	float frand();
**	frinit();
**
** DESCRIPTION
**	Frand generates uniform random numbers in the range [0,1).  A
**	'random' seed based on the time of day can be gotten by calling
**	frinit(); otherwise the same seed is always used and the number
**	sequence is the same.
**
** RETURN VALUE
**	Frand() returns the random number.
**	Frinit() returns nothing.
*/

static float    denom = (float) LONG_MAX;

void
frinit()
{
 /* NOSTRICT */
	srandom((unsigned) time((xtime_t *) NULL));
}

float
frand()
{
 /* NOSTRICT */
	return ((float) random() / denom);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/util/RCS/frand.c,v 1.6 90/11/11 17:19:26 frew Exp $";

#endif
