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

#include "convolve.h"

/*
 * putkernel -- print kernel on stderr, for debugging
 */

void
DEFUN(putkernel, (kernel, nrows, ncols),
	fpixel_t      **kernel		/* -> kernel array		 */
   AND  int             nrows		/* # kernel rows		 */
   AND  int             ncols)		/* # kernel columns		 */
{
	int             row;
	int             col;

	for (row = 0; row < nrows; ++row) {
		if (kernel[row] == NULL) {
			(void) fprintf(stderr, "(all 0s)\n");
		}
		else {
			for (col = 0; col < ncols; ++col) {
				(void) fprintf(stderr, "%*.*f ",
					FLT_DIG + 3, FLT_DIG,
					kernel[row][col]);
			}

			(void) fprintf(stderr, "\n");
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/convolve/RCS/putkernel.c,v 1.2 90/11/11 17:01:04 frew Exp $";

#endif
