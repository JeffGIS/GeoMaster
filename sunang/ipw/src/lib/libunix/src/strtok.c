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
**	strtok -- decompose string into tokens
**
** SYNOPSIS
**	char *strtok(str, seps)
**	char *str, *seps;
**
** DESCRIPTION
**	"Strtok" considers the string "str" to consist of a sequence of zero or
**	more text tokens separated by spans of one or more characters from
**	the separator string "seps".  "Strtok" finds the beginning of a token
**	by skipping any characters in "str" that are also in "seps".  Strtok
**	then delimits the token by skipping characters in str UNTIL a character
**	which is also in "seps" is found; "strtok" sets this character to EOS.
**
**	If "str" is NULL, then "strtok" begins searching for the next token
**	immediately after the end of the previously encountered token.
**
**	The "seps" string may be different from call to call.  Remember that
**	"strtok"'s FIRST whenever called is to skip characters in "str" that
**	are also in "seps".
**
** RETURN VALUE
**	"Strtok" returns a pointer to the next token in "str", or NULL if
**	there are no more tokens.
*/

char           *
strtok(str, seps)
	register char  *str;		/* -> string to parse		 */
	char           *seps;		/* -> set of token separators	 */
{
	static char    *savestr;	/* -> 1 past end of last token	 */

	char           *tokp;		/* -> current token		 */

	if (str == NULL) {
 /*
  * use previously supplied string, starting after last token processed
  */
		str = savestr;
	}

	if (str == NULL || *str == EOS) {
 /*
  * no more tokens
  */
		return (NULL);
	}
 /*
  * skip to beginning of next token
  */
	str += strspn(str, seps);
	tokp = str;
 /*
  * skip to end of current token
  */
	str += strcspn(str, seps);
	if (*str != EOS) {
 /*
  * terminate current token; move 1 past end
  */
		*str++ = EOS;
	}

	savestr = str;
	return (tokp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libunix/src/RCS/strtok.c,v 1.5 90/11/11 17:20:35 frew Exp $";

#endif
