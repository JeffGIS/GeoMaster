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
** fix window header
*/

#include "ipw.h"
#include "bih.h"
#include "pgm.h"

WINH_T        **
fixwinh(i_winh)
	WINH_T        **i_winh;		/* -> input window header	 */
{
	WINH_T        **o_winh;		/* -> output window header	 */
	WINH_T         *i_winhp;	/* -> input WINH for a band	 */
	WINH_T         *o_winhp;	/* -> output WINH for a band	 */
	int             band;		/* band index			 */
	int             nbands;		/* # input bands		 */
	int             nlines;		/* # input lines		 */
	int             nsamps;		/* # input samples		 */

	nbands = hnbands(parm.i_fd);
	nlines = hnlines(parm.i_fd);
	nsamps = hnsamps(parm.i_fd);
 /*
  * if input WINH is NULL, don't need output WINH
  */
	if (i_winh == (WINH_T **) NULL) {
		return (i_winh);
	}
 /*
  * duplicate input WINH, then make necessary changes
  */
	o_winh = winhdup(i_winh, nbands);
	assert(o_winh != NULL);

	for (band = 0; band < nbands; ++band) {
		i_winhp = i_winh[band];
		o_winhp = o_winh[band];

		if (i_winhp != (WINH_T *) NULL) {
			if (parm.lines) {
				winh_bline(o_winhp) = WIN_LINE(i_winhp,
							  nlines - 1);
				winh_dline(o_winhp) = -winh_dline(i_winhp);
			}
			if (parm.samps) {
				winh_bsamp(o_winhp) = WIN_SAMP(i_winhp,
							  nsamps - 1);
				winh_dsamp(o_winhp) = -winh_dsamp(i_winhp);
			}
		}
	}
	return (o_winh);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/fixwinh.c,v 1.4 90/11/11 17:02:31 frew Exp $";

#endif
