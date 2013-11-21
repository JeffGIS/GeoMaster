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

#include "geoh.h"
#include "gethdrs.h"
#include "winh.h"

#include "pgm.h"

/*
 * fixhdrs -- adjust spatial headers
 */

void
fixhdrs()
{
 /* NOSTRICT */
	static GETHDR_T h_geo = {GEOH_HNAME, (ingest_t) geohread};
 /* NOSTRICT */
	static GETHDR_T h_win = {WINH_HNAME, (ingest_t) winhread};

	static GETHDR_T *hv[] = {&h_geo, &h_win, NULL};

	double          zline;		/* line zoom factor		 */
	double          zsamp;		/* sample zoom factor		 */

 /*
  * ingest GEO and/or WIN headers; copy all others
  */
	gethdrs(parm.i_fd, hv, parm.nbands, parm.o_fd);

	if (!got_hdr(h_geo) && !got_hdr(h_win)) {
		return;
	}
 /*
  * compute spacing adjustment factors
  */
	zline = parm.skip_lines > 1 ?
		parm.skip_lines :
		1.0 / (double) parm.dup_lines;

	zsamp = parm.skip_samps > 1 ?
		parm.skip_samps :
		1.0 / (double) parm.dup_samps;
 /*
  * adjust GEO header
  */
	if (got_hdr(h_geo)) {
		int             band;	/* band counter			 */
		GEOH_T        **geohpp;	/* -> GEO header array		 */

 /* NOSTRICT */
		geohpp = (GEOH_T **) hdr_addr(h_geo);

		for (band = 0; band < parm.nbands; ++band) {
			if (geohpp[band] != NULL) {


 /* next 2 statements added by Rusty Dodson 8/9/94 to make geodetic
  * extents line up when replicating pixels
  */

		 	  if (parm.force) {  
                               geohpp[band]->bline -= ((geohpp[band]->dline
                               / 2.0) - (geohpp[band]->dline / 2.0 * zline));
                               geohpp[band]->bsamp -= ((geohpp[band]->dsamp
                               / 2.0) - (geohpp[band]->dsamp / 2.0 * zsamp));
			   }

                       	    geoh_dline(geohpp[band]) *= zline;
  			    geoh_dsamp(geohpp[band]) *= zsamp;

			}
		}

		if (geohwrite(parm.o_fd, geohpp) == ERROR) {
			error("can't write GEO header");
		}
	} 
	else {     				/* no geo header */
		if (parm.force) {
		    warn("No geo header.  -f option ignored.");
		}
	}
 /*
  * adjust WIN header
  */
	if (got_hdr(h_win)) {
		int             band;	/* band counter			 */
		WINH_T        **winhpp;	/* -> WIN header array		 */

 /* NOSTRICT */
		winhpp = (WINH_T **) hdr_addr(h_win);

		for (band = 0; band < parm.nbands; ++band) {
			if (winhpp[band] != NULL) {
				winh_dline(winhpp[band]) *= zline;
				winh_dsamp(winhpp[band]) *= zsamp;
			}
		}

		if (winhwrite(parm.o_fd, winhpp) == ERROR) {
			error("can't write WIN header");
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/zoom/RCS/fixhdrs.c,v 1.3 90/11/11 17:10:57 frew Exp $";

#endif
