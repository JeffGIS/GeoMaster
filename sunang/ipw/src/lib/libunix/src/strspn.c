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
#define	EOS		'\0'
#define	NULL		( (char *)0 )
#endif

/*
** NAME
**	strspn -- skip specified characters
**
** SYNOPSIS
**	int strspn(str, set)
**	char *str, *set;
**
** RETURN VALUE
**	"Strspn" returns the the length of the initial segment of string "str"
**	which consists entirely of characters from string "set".
*/

int
strspn(str, set)
	char           *str;		/* -> string to search		 */
	char           *set;		/* -> set of chars to look for	 */
{
	register char  *setp;		/* fast -> set			 */
	register char  *strp;		/* fast -> str			 */

	if (str == NULL || set == NULL) {
		return (0);
	}
 /*
  * step through search string
  */
	for (strp = str; *strp != EOS; ++strp) {
 /*
  * compare current char with each char in target set if we don't find a
  * match, we're done
  */
		for (setp = set; *setp != *strp; ++setp) {
			if (*setp == EOS) {
				goto done;
			}
		}
	}
done:
	return (strp - str);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/strspn.c,v 1.4 90/11/11 17:20:33 frew Exp $";

#endif
