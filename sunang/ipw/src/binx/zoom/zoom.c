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
#include "pixio.h"

#include "pgm.h"

/*
 * zoom -- image zooming ({mag,min}ify by {replication,subsampling})
 */

void
zoom()
{
	int             i_line;		/* input image line counter	 */
	int             o_line;		/* output image line counter	 */

 /*
  * allocate buffer; misc. initializations
  */
 /* NOSTRICT */
	parm.buf = (pixel_t *) ecalloc(MAX(parm.i_nsamps, parm.o_nsamps),
				       parm.nbands * sizeof(pixel_t));
	if (parm.buf == NULL) {
		error("can't allocate image line buffer");
	}

	o_line = 0;
 /*
  * loop through input lines
  */
	for (i_line = 0; i_line < parm.i_nlines; ++i_line) {
		int             n;	/* loop counter			 */

		if (pvread(parm.i_fd, parm.buf, parm.i_nsamps)
		    != parm.i_nsamps) {
			error("image read error, line %d", i_line);
		}
 /*
  * if minifying lines then possibly skip this line
  */
		if (i_line % parm.skip_lines != 0) {
			continue;
		}
 /*
  * skip samples
  */
		if (parm.skip_samps > 1) {
			subsamp();
		}
 /*
  * replicate samples
  */
		else if (parm.dup_samps > 1) {
			replicate();
		}
 /*
  * write line (if magnifying lines then repeat)
  */
		n = parm.dup_lines;
		do {
			if (pvwrite(parm.o_fd, parm.buf, parm.o_nsamps)
			    != parm.o_nsamps) {
				error("image write error, line %d", o_line);
			}

			++o_line;
		} while (--n > 0);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/zoom/RCS/zoom.c,v 1.2 90/11/11 17:11:15 frew Exp $";

#endif
