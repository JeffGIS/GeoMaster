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

/*
**	reverse a vector: call either specialized or general routine
**	depending on size of each element
*/

#include "ipw.h"
#include "pgm.h"

void
reverse(a, nelem, nb)
	addr_t          a;		/* -> beginning of data		 */
	int             nelem;		/* # elements in vector 	 */
	int             nb;		/* # bytes per element		 */
{
	if (nelem > 1 && nb >= 1) {
		if (nb == sizeof(char)) {
			revchar((char *) a, nelem);
		}
		else if (nb == sizeof(short)) {
 /* NOSTRICT */
			revshort((short *) a, nelem);
		}
		else if (nb == sizeof(long)) {
 /* NOSTRICT */
			revlong((long *) a, nelem);
		}
		else {
			revgen(a, nelem, nb);
		}
	}
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/reverse.c,v 1.5 90/11/11 17:02:51 frew Exp $";

#endif
