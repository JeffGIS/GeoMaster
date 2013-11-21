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

#ifdef	lint

#include "ipw.h"

/*
**
*/

#undef	optarg;
char   *optarg;

#undef	opterr
int     opterr;

#undef	optind
int     optind;

#undef	optopt
int     optopt;

#undef	random
long	random() { return 0; }

#undef	srandom
void	srandom(seed) unsigned seed; { return; }

#undef	sys_errlist
char   *sys_errlist[1];

#undef	sys_nerr
int     sys_nerr;

/* lint */
#endif

#ifndef	lint
static char rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/syslint/RCS/misc.c,v 1.2 90/11/11 17:20:47 frew Exp $";

#endif
