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
** flip samples only, so can do line by line
*/

#include "ipw.h"
#include "pgm.h"

void
flip1d(nl, ns, nb, buf)
	int             nl;		/* # lines in image		 */
	int             ns;		/* # samples per line		 */
	int             nb;		/* # bytes per sample		 */
	addr_t          buf;		/* data buffer			 */

{
	int             linebytes;	/* # bytes per line		 */
	int             ngot;		/* # bytes read/written		 */
	long            hold;

	hold = ns;
	hold *= nb;
	linebytes = ltoi(hold);

	while (--nl >= 0) {

 /* read line */
		ngot = uread(parm.i_fd, buf, linebytes);
		if (ngot != linebytes) {
			if (ngot < 0)
				error("image read error");
			else
				error("unexpected EOF on input");
		}

 /* reverse line */
		reverse(buf, ns, nb);

 /* write line */
		ngot = uwrite(parm.o_fd, buf, linebytes);
		if (ngot != linebytes) {
			error("image write error");
		}
	}
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/flip1d.c,v 1.3 90/11/11 17:02:35 frew Exp $";

#endif
