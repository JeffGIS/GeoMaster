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
**  Function to print string.
*/
#include <stdio.h>
#include "tabsize.h"

extern int      strlen();

void
Cprint(var, constant)
    char           *var;
    char           *constant;
{
    if (constant != NULL && strlen(constant) > 0) {
	if (strlen(var) < TABSIZE) {
	    printf("#define %s\t\t%s\n", var, constant);
	}
	else {
	    printf("#define %s\t%s\n", var, constant);
	}
    }
    else {
	printf("#define %s\n", var);
    }
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/Cprint.c,v 1.3 90/11/19 14:49:16 frew Exp $";

#endif
