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
 * conv1d -- 1-dimensional convolution
 */

void
DEFUN(conv1d, (ibuf, nsamps, kernel, ncols, obuf),
	REG_3 pixel_t  *ibuf		/* -> input line		 */
   AND  int             nsamps		/* # samples / line		 */
   AND  REG_2 fpixel_t **kernel		/* -> convolution kernel maps	 */
   AND  REG_5 int       ncols		/* # kernel columns		 */
   AND  REG_4 fpixel_t *obuf)		/* -> output line		 */
{
 /*
  * make obuf -> first convolvable sample
  */
	obuf += ncols / 2;
 /*
  * loop through convolvable samples
  */
	nsamps -= ncols - 1;
	do {
		FREG_1 fpixel_t sum;	/* fast FP accumulator		 */
		REG_6 int       col;	/* kernel column counter	 */

		sum = *obuf;
 /*
  * loop through kernel columns
  */
		col = ncols;
		do {
			REG_1 fpixel_t *kp;	/* fast -> kernel map	 */

 /*
  * if current kernel element is non-0, then accumulated weighted value of
  * current input pixel
  */
			kp = *kernel++;
			if (kp != NULL) {
				sum += kp[*ibuf];
			}

			++ibuf;
		} while (--col > 0);

		*obuf++ = sum;

		ibuf -= ncols;
		++ibuf;

		kernel -= ncols;
	} while (--nsamps > 0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/convolve/RCS/conv1d.c,v 1.4 90/11/11 17:00:49 frew Exp $";

#endif
