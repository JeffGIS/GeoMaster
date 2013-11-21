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

/* LINTLIBRARY */

#include "ipw.h"

#include "bitcom.h"

/*
 * m_expand -- expand pixels in last band
 */

void
m_expand(buf, nsamps, nbands)
	REG_1 pixel_t  *buf;		/* -> mask buffer		 */
	REG_3 int       nsamps;		/* # samples in buffer		 */
	REG_2 int       nbands;		/* # bands / sample		 */
{
	buf = &buf[nbands - 1];

	do {
		if (*buf != 0) {
			*buf = ~0;
		}

		buf = &buf[nbands];
	} while (--nsamps > 0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/bitcom/RCS/m_expand.c,v 1.2 90/11/11 17:00:42 frew Exp $";

#endif
