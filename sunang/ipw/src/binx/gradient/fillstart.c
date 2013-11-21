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
** fill ends of line 1
** fill fictitious line 0
*/
#include "ipw.h"
#include "fpio.h"
#include "pgm.h"

void
fillstart(n, k0, k1, k2)
	int             n;		/* genuine # samples	 */
	REG_3 float    *k0;		/* -> line 0		 */
	REG_1 float    *k1;		/* -> line 1		 */
	REG_4 float    *k2;		/* -> line 2		 */
{
	REG_2 int       j;		/* index		 */

 /*
  * ends of line 1
  */
	fillends(n, k1);

 /*
  * line 0
  */
	for (j = n - 1; j >= 0; --j) {
		k0[j] = k1[j] + (k1[j] - k2[j]);
	};

 /*
  * ends of line 0
  */
	fillends(n, k0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/fillstart.c,v 1.3 90/11/11 17:03:15 frew Exp $";

#endif
