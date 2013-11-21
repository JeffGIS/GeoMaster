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
#include "horizon.h"

LQH_T         **
DEFUN(newlqh, (fdo),
	int             fdo)		/* output file desc		 */
{
	int             nbands;
	LQH_T         **lqhpp;		/* -> output LQH		 */
	fpixel_t        fbkpt[2][2];	/* limits of LQ			 */
	pixel_t         ibkpt[2][2];	/* integer break points		 */
	char            units[2][32];	/* units			 */
	int             j;

 /*
  * preamble, we use reverse quantization so that dark pixels are those with
  * large horizon angles, i.e. they are shaded
  */
	nbands = hnbands(fdo);
	for (j = 0; j < nbands; ++j) {
		(void) sprintf(units[j], "cos H");

		fbkpt[j][0] = 1;
		fbkpt[j][1] = 0;
		ibkpt[j][0] = 0;
		ibkpt[j][1] = ipow2(hnbits(fdo, j)) - 1;
	}
 /*
  * return the new LQH
  */
 /* NOSTRICT */
	lqhpp = (LQH_T **) hdralloc(nbands, sizeof(LQH_T *),
				    fdo, LQH_HNAME);
	for (j = 0; j < nbands; ++j) {
		lqhpp[j] = lqhmake(hnbits(fdo, j), 2, ibkpt[j], fbkpt[j],
				   units[j], (char *) NULL);
	}

	return (lqhpp);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/newlqh.c,v 1.3 90/11/11 17:04:30 frew Exp $";

#endif
