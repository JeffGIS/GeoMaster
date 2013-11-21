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

/* revgen - reverse vector with elements of arbitrary length */

/*
 *	First reverse the entire vector, byte by byte.  Now each of the
 *	elements also have their bytes reverse.  Reverse these back
 *	again.
 */

#include "ipw.h"

#include "pgm.h"

void
revgen(a, nelem, nb)
	register addr_t	a;		/*  -> beginning of data	*/
	int		nelem;		/*  # elements in vector	*/
	register int	nb;		/*  # bytes per element		*/
{
	register int	j;

	revchar(a, nelem*nb);
	j = nelem;
	while (--j >= 0) {
		revchar(a, nb);
		a = (char *) a + nb;
	}
}
#ifndef lint
static char	rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/revgen.c,v 1.3 90/11/11 17:02:55 frew Exp $";
#endif
