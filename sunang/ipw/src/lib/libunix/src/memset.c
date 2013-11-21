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

#ifdef	IPW
#include "ipw.h"
#else
#define	REG_1		register
#define	REG_2		register
#define	REG_3		register
#endif

/*
** NAME
**	memset -- set block of memory to constant value
**
** SYNOPSIS
**	char *memset(mem, c, nchars)
**	char *mem;
**	int c, nchars;
**
** DESCRIPTION
**	"Memset" sets the first "nchars" characters in memory area "mem" to the
**	value of character "c".
**
** RETURN VALUE
**	"Memset" returns "mem".
*/

char           *
memset(s, c, n)
	char           *s;		/* -> memory area		 */
	int             c;		/* value to load into mem	 */
	int             n;		/* #chars to set to "c"		 */
{
	REG_2 int       r_n;

	r_n = n;
	if (r_n > 0) {
		REG_1 char     *r_s;
		REG_3 int       r_c;

		r_s = s;
		r_c = c;

		do {
			*r_s++ = r_c;
		} while (--r_n > 0);
	}

	return (s);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/memset.c,v 1.6 90/11/11 17:20:26 frew Exp $";

#endif
