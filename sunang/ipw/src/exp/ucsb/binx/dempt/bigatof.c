/*
**	convert Fortran D-format input to E-format
**	also make sure NULL at end of string
*/


#include <ctype.h>
#include <math.h>

#include "ipw.h"
#include "pgm.h"

double
bigatof(s)
	char           *s;
{
	register        j;
	register char  *p;
	char            temp[BIGFSIZ + 1];

	temp[BIGFSIZ] = (char) NULL;

	p = temp;
	for (j = 0; j < BIGFSIZ; j++) {
		if (s[j] == 'D')
			*p++ = 'E';
		else
			*p++ = s[j];
	}

	return (atof(temp));
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/bin/dempt/RCS/bigatof.c,v 1.2 89/11/12 16:19:19 frew Exp $";

#endif
