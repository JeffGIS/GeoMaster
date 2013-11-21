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
#include "bih.h"
#include "fpio.h"

#include "pgm.h"

/*
 * lqhx -- read old floating-pt values, convert to new, and write
 */

void
lqhx()
{
	fpixel_t       *fpbuf;		/* -> I/O buffer		 */
	int             nsamps;		/* # samples per line		 */
	int             nlines;		/* # lines in image		 */
	int             nbands;		/* # bands per sample		 */

	nbands = hnbands(parm.i_fd);
	nsamps = hnsamps(parm.i_fd);
	nlines = hnlines(parm.i_fd);
 /*
  * allocate I/O buffer
  */
 /* NOSTRICT */
	fpbuf = (fpixel_t *) ecalloc(nsamps * nbands, sizeof(fpixel_t));
	if (fpbuf == NULL) {
		error("can't allocate I/O buffer");
	}
 /*
  * read, convert, write
  */
	while (--nlines >= 0) {
		if (fpvread(parm.i_fd, fpbuf, nsamps) != nsamps) {
			error("read error, line %d",
			      hnlines(parm.i_fd) - nlines + 1);
		}
		if (fpvwrite(parm.o_fd, fpbuf, nsamps) == ERROR) {
			error("write error, line %d",
			      hnlines(parm.o_fd) - nlines + 1);
		}
	}

	SAFE_FREE(fpbuf);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/lqhx/RCS/lqhx.c,v 1.2 90/11/11 17:05:40 frew Exp $";

#endif
