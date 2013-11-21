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
 * create new LQH
 */

#include <math.h>

#include "ipw.h"
#include "bih.h"
#include "lqh.h"
#include "view.h"

LQH_T         **
DEFUN(newlqh,(fdi, fdo),
	int             fdi		/* input file descriptor	 */
   AND  int             fdo)		/* output file descriptor	 */
{
	LQH_T         **lqhp;		/* -> output LQH		 */
	char            units[32];	/* units			 */
	pixel_t         ival[2];
	fpixel_t        fval[2];	/* ranges for sky view factor	 */
	fpixel_t        tval[2];	/* ranges for terrain config
					 * fac */

	(void) sprintf(units, "sky view factor");
	ival[0] = 0;
	ival[1] = ipow2(hnbits(fdi, 0)) - 1;
	fval[0] = 0;
	fval[1] = 1;
	tval[0] = 1;
	tval[1] = 0;

 /* NOSTRICT */
	lqhp = (LQH_T **) hdralloc(2, sizeof(LQH_T *), fdo, LQH_HNAME);
	assert(lqhp != NULL);
	lqhp[0] = lqhmake(hnbits(fdi, 0), 2, ival, fval, units, (char *) NULL);

	(void) sprintf(units, "terrain config factor");
	lqhp[1] = lqhmake(hnbits(fdi, 0), 2, ival, tval, units, (char *) NULL);

	return (lqhp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/viewcalc/RCS/newlqh.c,v 1.2 90/11/11 17:10:17 frew Exp $";

#endif
