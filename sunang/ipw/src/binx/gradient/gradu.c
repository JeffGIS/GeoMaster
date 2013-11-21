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
**	This is where the work is done.  Compute derivatives, slopes and
**	apsects, and shuffle.
*/

#include "ipw.h"

#include "pgm.h"

void
gradu(nsamps, delh, dos, doa, ibuf, dx, dy, s, a, obuf)
	int             nsamps;		/* # samples in line		 */
	fpixel_t       *delh;		/* grid spacing (1-line 2-samp)	 */
	bool_t          dos;		/* ? compute slope		 */
	bool_t          doa;		/* ? compute aspect		 */
	fpixel_t      **ibuf;		/* input buffers		 */
	fpixel_t       *dx;		/* partial w.r.t. x		 */
	fpixel_t       *dy;		/* partial w.r.t. y		 */
	fpixel_t       *s;		/* sinS				 */
	fpixel_t       *a;		/* aspect			 */
	fpixel_t       *obuf;		/* output buffer		 */
{
 /*
  * end pixels of line 2
  */
	fillends(nsamps, ibuf[2]);

 /*
  * finite differences
  */
	diffxy(nsamps, delh, ibuf[0], ibuf[1], ibuf[2], dx, dy);

 /*
  * compute sinS
  */
	if (dos) {
		cslope(nsamps, dx, dy, s);
	}

 /*
  * compute A
  */
	if (doa) {
		caspect(nsamps, dx, dy, a);
	}

 /*
  * shuffle into output buffer if both slopes and aspects
  */

	if (dos && doa) {
		shuffle(nsamps, s, a, obuf);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/gradu.c,v 1.4 90/11/11 17:03:20 frew Exp $";

#endif
