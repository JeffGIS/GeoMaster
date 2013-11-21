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

#include "_strvec.h"

/*
** NAME
**	walksv -- step through strings in strvec
**
** SYNOPSIS
**	char *walksv(p, reset)
**	STRVEC_T *p;
**	bool_t reset;
**
** DESCRIPTION
**	walksv sequentially accesses the strings in strvec p.  If reset is
**	true, then the first string is accessed.  If reset is false, then the
**	next string is accessed.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to string in p.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**	Calling addsv or delsv may confuse walksv's notion of what the "next"
**	string is.  Calling walksv with reset == TRUE avoids this problem.
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

char           *
walksv(p, reset)
	REG_1 STRVEC_T *p;
	bool_t          reset;
{
	REG_2 int       i;

	assert(ok_sv(p));
 /*
  * if resetting then return 0'th string and reset curr
  */
	if (reset) {
		i = 0;
		p->curr = 1;
	}
 /*
  * if continuing then return current string and bump curr
  */
	else {
		i = p->curr;
		if (i < p->n - 1) {
			++p->curr;
		}
	}

	return (p->v[i]);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/strvec/RCS/walksv.c,v 1.2 90/11/11 17:18:03 frew Exp $";

#endif
