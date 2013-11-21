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

#include "hist.h"

/*
 * histx -- accumulate histogram of 1 image line
 */

void
histx(i_buf, m_buf, nsamps, nbands, histo)
	REG_1 pixel_t  *i_buf;		/* -> image line		 */
	REG_3 pixel_t  *m_buf;		/* -> mask line			 */
	REG_6 int       nsamps;		/* # samples / line		 */
	REG_5 int       nbands;		/* # bands/ sample		 */
	REG_2 hist_t   **histo;		/* -> histogram array		 */
{
	do {
		REG_4 int       band;	/* band counter			 */

		band = 0;
		do {
			if (m_buf == NULL || *m_buf++ != 0) {
				++histo[*i_buf++][band];
			}
		} while (++band < nbands);
	} while (--nsamps > 0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hist/RCS/histx.c,v 1.4 90/11/11 17:03:57 frew Exp $";

#endif
