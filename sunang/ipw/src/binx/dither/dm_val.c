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
#include "bih.h"
#include "dither.h"

/*
 * dm_val -- calculate dither matrix value
 *
 *	Algorithm derived from recurrence relation given on p. 107 of:
 *		David F. Rogers
 *		"Procedural Elements for Computer Graphics."
 *		McGraw-Hill
 *		1985
 */

int
DEFUN( dm_val, (rank, row, col),
	int             rank		/* dither matrix rank		 */
   AND  int             row		/* matrix row index		 */
   AND  int             col)		/* matrix column index		 */
{
	static int      dm2x2[2][2] = {	/* 2 x 2 dither matrix		 */
		{0, 2},
		{3, 1}
	};

	if (rank > 2) {
		return (4 * dm_val(rank / 2, row, col)
			+ dm_val(2, (2 * row) / rank, (2 * col) / rank));
	}

	return (dm2x2[row & 1][col & 1]);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/dither/RCS/dm_val.c,v 1.2 90/11/11 17:02:07 frew Exp $";

#endif
