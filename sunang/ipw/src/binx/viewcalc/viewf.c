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
 * read input slope/azimuth data, horizon data, & compute
 * sky view factor and terrain configuration factor
 */

#include "ipw.h"
#include "bih.h"
#include "pixio.h"
#include "fpio.h"

#include "view.h"

void
viewf()
{
	FREG_1 float    cosS;		/* cosine slope		 */
	FREG_2 float    sinS;		/* sine slope		 */
	FREG_3 float    intgrnd;	/* values inside intgrl	 */
	REG_1 int       hd;		/* horizon dir index	 */
	REG_2 fpixel_t *b;		/* output value		 */
	REG_3 pixel_t  *hp;		/* -> horizon index	 */
	REG_4 pixel_t  *sp;		/* -> slope index	 */
	REG_5 int       nhorz;		/* # horizon directions	 */
	int             j;		/* loop counter		 */
	int             nget;		/* # pixels to read	 */
	int             ngot;		/* # pixels read	 */
	int             nwrite;		/* # pixels to write	 */

	nhorz = hnbands(parm.i_fdh);

 /* default values */
	nwrite = nget = hnsamps(parm.i_fds);

 /*
  * read pixels until done
  */

	while ((ngot = pvread(parm.i_fds, parm.sbuf, nget)) > 0) {
		if (ngot < nget) {
			nwrite = ngot;
		}
		ngot = pvread(parm.i_fdh, parm.hbuf, nwrite);
		if (ngot != nwrite) {
			error("files unequal # pixels");
		}

 /* -> input */
		sp = parm.sbuf;
		hp = parm.hbuf;

 /* -> output */
		b = parm.obuf;

		for (j = 0; j < nwrite; ++j) {
			cosS = parm.cstbl[*sp];
			sinS = parm.sstbl[*sp++];	/* sp now -> aspect */
			*b = 0;
			for (hd = nhorz; --hd >= 0;) {
				intgrnd = cosS * parm.sh2tbl[hd][*hp] +
					sinS * parm.cosdtbl[hd][*sp] *
					parm.hdtbl[hd][*hp];
				if (intgrnd > 0)
					*b += intgrnd;
				++hp;
			}
			++sp;		/* sp -> slope of next pixel */
 /*
  * sky view factor
  */
			*b /= nhorz;
 /*
  * terrain configuration factor
  */
			b[1] = (1 + cosS) / 2 - *b;
			b += 2;
		}

		if (fpvwrite(parm.o_fd, parm.obuf, nwrite) != nwrite) {
			error("write error");
		}
	}

	if (ngot != 0) {
		error("read error");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/viewcalc/RCS/viewf.c,v 1.2 90/11/11 17:10:29 frew Exp $";

#endif
