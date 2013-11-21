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
** flip - read it; flip lines, samps or both; and write
*/

#include "ipw.h"
#include "bih.h"
#include "pgm.h"

void
flip()
{
	addr_t          buf;		/* -> 1-d or 2-d data buffer	 */
	int             sampbytes;	/* # bytes per sample		 */
	int             linebytes;	/* # bytes per line		 */
	int             nlines;		/* # lines			 */
	int             nsamps;		/* # samps			 */
	long            hold;

 /*
  * If lines are to be flipped, allocate a 2-d image buffer (this could
  * be large, and cause the program to fail here). Otherwise, allocate
  * an image buffer large enough to hold one line.
  */

	nsamps = hnsamps(parm.i_fd);
	nlines = hnlines(parm.i_fd);
	sampbytes = sampsize(parm.i_fd);
	hold = sampbytes;
	hold *= nsamps;
	linebytes = ltoi(hold);

	if (parm.lines) {
 /* NOSTRICT */
		buf = allocnd(sampbytes, 2, nlines, nsamps);
		if (buf == (addr_t) NULL) {
			error("can't allocate 2-d array");
		}
	}

	else if (parm.samps) {
		buf = (addr_t) malloc((unsigned) linebytes);
		if (buf == (addr_t) NULL) {
			error("can't allocate 1-d vector");
		}
	}

	else {
 /* shouldn't reach, as checked in main also */
		bug("neither lines nor samps switch on");
	}

	if (parm.lines) {

 /* NOSTRICT */
		flip2d(nlines, nsamps, sampbytes, (addr_t *) buf);
	}

	else if (parm.samps) {
		flip1d(nlines, nsamps, sampbytes, buf);
	}

	SAFE_FREE(buf);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/flip.c,v 1.7 90/11/11 17:02:33 frew Exp $";

#endif
