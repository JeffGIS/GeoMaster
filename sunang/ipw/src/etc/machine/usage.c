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

#include <stdio.h>

extern int      exit();

void
usage(prog)
    char           *prog;
{
    fprintf(stderr, "%s --\n", prog);
    fprintf(stderr,
	    "Prints out machine constants for integer and floating-point\n");
    fprintf(stderr,
	    "arithmetic or queries whether machine meets IEEE 754 standard\n\n");
    fprintf(stderr,
       "Usage:  %s [ -limits ] [ -float ] [ -ieee ]\n", prog);
    fprintf(stderr, "\nPipe output through 'indent'.\n\n");
    fprintf(stderr, "Argument:\n");
    fprintf(stderr,
	"\t-limits\tconstants necessary for ANSI C <limits.h>\n");
    fprintf(stderr,
	"\t-float\tconstants necessary for ANSI C <float.h>\n");
    fprintf(stderr,
	"\t-ieee\t#defines CC_IEEE_754 appropriately\n");
    fprintf(stderr,
	"\n\t(Note: one of -limits -float -ieee must be present)\n");
    (void) exit(1);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/usage.c,v 1.3 90/11/19 14:50:03 frew Exp $";

#endif
