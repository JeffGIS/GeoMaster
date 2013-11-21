#include "ipw.h"

#include "pgm.h"

/*
 * mstats -- image multivariate statistics
 */

void
mstats()
{
	int             line;		/* image line #			 */

	init();

	for (line = 0; line < parm.i_nlines; ++line) {
		if (fpvread(parm.i_fd, parm.i_buf, parm.i_nsamps)
		    != parm.i_nsamps) {
			error("image read error, line %d", line);
		}

		accum();
	}

	mcov();
}

#ifndef	lint
static char     rcsid[] = "$Header: mstats.c,v 1.1 89/05/09 11:56:10 frew Exp $";

#endif
