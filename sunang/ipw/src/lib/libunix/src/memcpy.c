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
**	memcpy -- copy memory
**
** SYNOPSIS
**	char *memcpy(s1, s2, n)
**	char *s1, s2;
**	int n;
**
** DESCRIPTION
**	"Memcpy" copies "n" characters from memory area "s2" to "s1".
**
** RETURN VALUE
**	"Memset" returns "s1".
*/

char           *
memcpy(s1, s2, n)
	char           *s1;		/* -> destination memory area	 */
	char           *s2;		/* -> source memory area	 */
	int             n;		/* #chars to copy		 */
{
	REG_3 int       r_n;

	r_n = n;
	if (r_n > 0) {
		REG_1 char     *r_s1;
		REG_2 char     *r_s2;

		r_s2 = s2;
		r_s1 = s1;

		do {
			*r_s1++ = *r_s2++;
		} while (--r_n > 0);
	}

	return (s1);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/memcpy.c,v 1.7 90/11/11 17:20:24 frew Exp $";

#endif
