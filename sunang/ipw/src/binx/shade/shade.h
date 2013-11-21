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
 * variables passed externally between
 *	init_shade()
 */

#include "ipw.h"
#include "fpio.h"
#include "pixio.h"

extern pixel_t **shade;		/* store already computed values */
extern pixel_t  *obuf;		/* binary fraction output buffer */
extern float    *cosdtbl;	/* all values of cos(phi-A)	 */
extern float    *costbl;	/* all possible values of cosS	 */
extern fpixel_t *sintbl;	/* all possible values of sinS	 */
extern int       nget;		/* # pixels to read		 */
extern pixel_t  *ibuf;		/* binary frac slope/azm buffers */

/* $Header: /local/share/pkg/ipw/src/bin/shade/RCS/shade.h,v 1.4 90/11/11 17:08:59 frew Exp $ */
