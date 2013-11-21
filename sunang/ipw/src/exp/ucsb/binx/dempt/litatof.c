/*
**	just like atof(), but make sure NULL at end of string
*/

#include <math.h>

#include "ipw.h"

#include "pgm.h"

double
litatof(s)
	char           *s;
{
	register        j;
	register char  *p;
	char            temp[LITFSIZ + 1];

	temp[LITFSIZ] = (char) NULL;

	p = temp;
	for (j = 0; j < LITFSIZ; j++) {
		*p++ = *s++;
	}

	return (atof(temp));
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/bin/dempt/RCS/litatof.c,v 1.2 89/11/12 16:19:22 frew Exp $";

#endif
