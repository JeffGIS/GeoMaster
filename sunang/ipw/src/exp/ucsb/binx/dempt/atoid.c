/*
**	just like atoi(), but make sure NULL at end of string and limited
**	to 6 characters
*/

#include "ipw.h"

#include "pgm.h"

int
atoid(s)
	char           *s;
{
	register        j;
	register char  *p;
	char            temp[ISIZE + 1];

	temp[ISIZE] = (char) NULL;

	p = temp;
	for (j = 0; j < ISIZE; j++) {
		*p++ = *s++;
	}

	return (atoi(temp));
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/bin/dempt/RCS/atoid.c,v 1.2 89/11/12 16:18:52 frew Exp $";

#endif
