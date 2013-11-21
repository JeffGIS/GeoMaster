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
** fix geodetic header
*/

#include "ipw.h"
#include "bih.h"
#include "pgm.h"

GEOH_T        **
fixgeoh(i_geoh)
	GEOH_T        **i_geoh;		/* -> input geodetic header	 */
{
	GEOH_T        **o_geoh;		/* -> output geodetic header	 */
	GEOH_T         *i_geohp;	/* -> input GEOH for a band	 */
	GEOH_T         *o_geohp;	/* -> output GEOH for a band	 */
	int             band;		/* band index			 */
	int             nbands;		/* # input bands		 */
	int             nlines;		/* # input lines		 */
	int             nsamps;		/* # input samples		 */

	nbands = hnbands(parm.i_fd);
	nlines = hnlines(parm.i_fd);
	nsamps = hnsamps(parm.i_fd);
 /*
  * if input GEOH is NULL, don't need output GEOH
  */
	if (i_geoh == (GEOH_T **) NULL) {
		return (i_geoh);
	}
 /*
  * duplicate input GEOH, then make necessary changes
  */
	o_geoh = geohdup(i_geoh, nbands);
	assert(o_geoh != NULL);

	for (band = 0; band < nbands; ++band) {
		i_geohp = i_geoh[band];
		o_geohp = o_geoh[band];

		if (i_geohp != (GEOH_T *) NULL) {
			if (parm.lines) {
				geoh_bline(o_geohp) = GEO_LINE(i_geohp,
							  nlines - 1);
				geoh_dline(o_geohp) = -geoh_dline(i_geohp);
			}
			if (parm.samps) {
				geoh_bsamp(o_geohp) = GEO_SAMP(i_geohp,
							  nsamps - 1);
				geoh_dsamp(o_geohp) = -geoh_dsamp(i_geohp);
			}
		}
	}
	return (o_geoh);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/fixgeoh.c,v 1.2 90/11/11 17:02:24 frew Exp $";

#endif
