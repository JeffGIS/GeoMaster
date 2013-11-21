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

#include "pgm.h"

/*
 * ipw2pbm -- write PBM header, then copy pixels
 */

void
ipw2pbm()
{
	char            hdr[MAX_CHAR];	/* PBM header buffer		 */

 /*
  * build and write PBM header
  */
	(void) sprintf(hdr, "%s\n%d %d\n%d\n",
		       parm.nbands == 1 ? PGM_COOKIE : PPM_COOKIE,
		       parm.nsamps, parm.nlines, ipow2(parm.nbits) - 1);

	if (uputs(parm.o_fd, hdr) == ERROR) {
		error("PBM header write failed");
	}
 /*
  * copy raw pixel data (IPW storage and interleaving is same as PBM)
  */
	if (ucopy(parm.i_fd, parm.o_fd, imgsize(parm.i_fd)) == ERROR) {
		error("image data copy failed");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/ipw2pbm/RCS/ipw2pbm.c,v 1.2 90/11/11 17:04:42 frew Exp $";

#endif
