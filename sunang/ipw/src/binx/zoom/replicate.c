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

#include "ipw.h"

#include "pgm.h"

/*
 * replicate -- replicate pixels in current image line, in-place
 */

void
replicate()
{
	REG_1 pixel_t  *i_p;		/* -> current input pixel	 */
	REG_4 int       nbands;		/* # image bands		 */
	REG_2 pixel_t  *o_p;		/* -> current output pixel	 */
	REG_6 int       samp;		/* sample counter		 */

	nbands = parm.nbands;

	i_p = &parm.buf[parm.i_nsamps * nbands];
	o_p = &parm.buf[parm.o_nsamps * nbands];
 /*
  * loop through input samples
  */
	samp = parm.i_nsamps;
	do {
		REG_5 int       nreps;	/* # replications		 */

 /*
  * loop for # replications
  */
		nreps = parm.dup_samps;
		for (;;) {
			REG_3 int       band;	/* band counter		 */

 /*
  * loop through bands
  */
			band = nbands;
			do {
				*--o_p = *--i_p;
			} while (--band > 0);

			if (--nreps <= 0) {
				break;
			}
 /*
  * if still replicating then repeat current input pixel
  */
			i_p += nbands;
		}
	} while (--samp > 0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/zoom/RCS/replicate.c,v 1.3 90/11/11 17:11:10 frew Exp $";

#endif
