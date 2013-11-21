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
#include "fpio.h"

#include "pgm.h"

/*
 * mstats -- image multivariate statistics
 */

void
mstats()
{
	int             line;		/* image line #			 */

	init();

	for (line = 0; line < parm.nlines; ++line) {
		if (fpvread(parm.i_fd, parm.i_buf, parm.nsamps)
		    != parm.nsamps) {
			error("image read error, line %d", line);
		}

		if (parm.c_fd != ERROR) {
			if (pvread(parm.c_fd, parm.c_buf, parm.nsamps)
			    != parm.nsamps) {
				error("image read error, line %d", line);
			}
		}

		accum();
	}

	mcov();
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mstats/RCS/mstats.c,v 1.2 90/11/11 17:06:58 frew Exp $";

#endif
