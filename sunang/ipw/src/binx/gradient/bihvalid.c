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
 * is this BIH OK?
 */

#include "ipw.h"
#include "bih.h"

bool_t
bihvalid(bih, nb)
	BIH_T         **bih;		/* -> BIH		 */
	int             nb;		/* valid # bands	 */
{
 /*
  * make sure input image is only nb band
  */
	if (bih_nbands(bih[0]) != nb) {
		usrerr("input file has %d bands", bih_nbands(bih[0]));
		return (FALSE);
	}
 /*
  * make sure input image has spacing
  */
	if (bih_nlines(bih[0]) < 2) {
		usrerr("only %d line in input image", bih_nlines(bih[0]));
		return (FALSE);
	}
 /*
  * make sure input image lines have at least two pixels
  */
	if (bih_nsamps(bih[0]) < 2) {
		usrerr("only %d samp in input image", bih_nsamps(bih[0]));
		return (FALSE);
	}

	return (TRUE);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/bihvalid.c,v 1.2 90/11/11 17:03:03 frew Exp $";

#endif
