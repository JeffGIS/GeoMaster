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
** Flip lines of image:
** Read entire image into 2-d memory.
** If necessary, flip samples within each line.
** Then flip dope vector that references beginning of each line.
*/

#include "ipw.h"
#include "pgm.h"

void
flip2d(nl, ns, nb, buf)
	int             nl;		/* # lines in image		 */
	int             ns;		/* # samples per line		 */
	int             nb;		/* # bytes per sample		 */
	addr_t         *buf;		/* data buffer			 */
{
	int             tbytes;		/* # bytes per image		 */
	int             linebytes;	/* # bytes per line		 */
	int             ngot;		/* # bytes read/written		 */
	int             j;		/* line counter			 */
	long            hold;


	hold = ns;
	hold *= nb;
	linebytes = ltoi(hold);

	hold *= nl;
	tbytes = ltoi(hold);

 /*
  * read the entire image into the buffer
  */

	ngot = uread(parm.i_fd, buf[0], tbytes);
	if (ngot != tbytes) {
		if (ngot < 0)
			error("image read error");
		else
			error("unexpected EOF on input image");
	}

	if (parm.samps) {

 /*
  * flip order of samples within lines
  */

		j = nl;
		while (--j >= 0) {
			reverse(buf[j], ns, nb);
		}
	}

 /*
  * flip order of lines
  */

 /* NOSTRICT */
	reverse((addr_t) buf, nl, sizeof(addr_t));


 /*
  * write the image data
  */

	for (j = 0; j < nl; ++j) {
		ngot = uwrite(parm.o_fd, buf[j], linebytes);
		if (ngot != linebytes)
			error("image write error");
	}
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/flip2d.c,v 1.3 90/11/11 17:02:37 frew Exp $";

#endif
