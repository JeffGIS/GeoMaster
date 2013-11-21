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
** fix orientation header
*/

#include "ipw.h"
#include "bih.h"
#include "pgm.h"

 /* codes for diagonals */
static int      diag[] = {0, 3, 4, 1, 2};

 /* codes for samples in normal orientation */
static int      xcol[] = {0, 2, 1, 4, 3};

 /* codes for lines in normal orientation */
static int      xrow[] = {0, 4, 3, 2, 1};

static ORH_T   *
neworh()
{
	ORH_T          *orhp;		/* -> orientation header	 */

	if (parm.lines && parm.samps) {
		orhp = orhmake(IPW_ORIENT, ORIG_3);
	}
	else if (parm.lines) {
		orhp = orhmake(IPW_ORIENT, ORIG_4);
	}
	else if (parm.samps) {
		orhp = orhmake(IPW_ORIENT, ORIG_2);
	}
 /*
  * shouldn't reach, error will be caught in main
  */
	else {
		bug("(shouldn't reach: neither -l nor -s specified");
	}
	return (orhp);
}

ORH_T         **
fixorh(i_orh)
	ORH_T         **i_orh;		/* -> input ORH array			 */
{
	ORH_T          *orhp0;		/* -> orientation header	 */
	ORH_T         **o_orh;		/* -> output ORH array		 */
	bool_t          needed;		/* ? ORH needed			 */
	int             band;		/* band index			 */
	int             nbands;		/* # bands in input file	 */
	int             icode;		/* code for input origin	 */
	int             ocode;		/* code for output origin	 */

	nbands = hnbands(parm.i_fd);
	icode = 0;
	ocode = 0;
 /*
  * no existing orientation header, so create one
  */

	if (i_orh == (ORH_T **) NULL) {
 /* NOSTRICT */
		o_orh = (ORH_T **) hdralloc(nbands, sizeof(ORH_T *),
					    parm.o_fd, ORH_HNAME);
		orhp0 = neworh();
 /*
  * copy to all bands
  */
		for (band = 0; band < nbands; ++band) {
			o_orh[band] = orhp0;
		}
	}

	else {
 /*
  * duplicate existing header, then fix
  */
		o_orh = orhdup(i_orh, nbands);
 /*
  * change existing orientation header for each band
  */
		for (band = 0; band < nbands; ++band) {
 /*
  * if this band doesn't have an orientation header, create one
  */
			if (o_orh[band] == (ORH_T *) NULL) {
				o_orh[band] = neworh();
			}
 /*
  * numerical code for input origin 1,2,3,4 clockwise from upper left
  */
			else if (streq(orh_origin(o_orh[band]), ORIG_1)) {
				icode = 1;
			}
			else if (streq(orh_origin(o_orh[band]), ORIG_2)) {
				icode = 2;
			}
			else if (streq(orh_origin(o_orh[band]), ORIG_3)) {
				icode = 3;
			}
			else if (streq(orh_origin(o_orh[band]), ORIG_4)) {
				icode = 4;
			}
 /*
  * numerical code for output origin
  */

			if (parm.lines && parm.samps) {
				ocode = diag[icode];
			}

			else if (parm.lines) {
				ocode = streq(orh_orient(o_orh[band]), ROW) ?
					xrow[icode] : xcol[icode];
			}

			else if (parm.samps) {
				ocode = streq(orh_orient(o_orh[band]), ROW) ?
					xcol[icode] : xrow[icode];
			}
 /*
  * insert string descriptor for output origin
  */
			SAFE_FREE( orh_origin(o_orh[band]) );

			switch (ocode) {
			case 1:
				orh_origin(o_orh[band]) =
					hstrdup(ORIG_1, ORH_HNAME, band);
				break;
			case 2:
				orh_origin(o_orh[band]) =
					hstrdup(ORIG_2, ORH_HNAME, band);
				break;
			case 3:
				orh_origin(o_orh[band]) =
					hstrdup(ORIG_3, ORH_HNAME, band);
				break;
			case 4:
				orh_origin(o_orh[band]) =
					hstrdup(ORIG_4, ORH_HNAME, band);
				break;
			default:
				bug("shouldn't reach: invalid ocode");
			}
			if (orh_origin(o_orh[band]) == NULL) {
				error("can't duplicate origin field");
			}
		}
	}
 /*
  * Check output header.  If we've created an image in standard IPW
  * format, set the header to NULL
  */
	needed = FALSE;
	for (band = 0; band < nbands; ++band) {
		if (strdiff(IPW_ORIENT, orh_orient(o_orh[band])) ||
		    strdiff(IPW_ORIGIN, orh_origin(o_orh[band]))) {
			needed = TRUE;
		}
		else {
			o_orh[band] = NULL;
		}
	}

	return (needed == FALSE ? (ORH_T **) NULL : o_orh);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/fixorh.c,v 1.4 90/11/11 17:02:26 frew Exp $";

#endif
