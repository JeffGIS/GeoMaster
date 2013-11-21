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
** NAME
**	init_shade -- read & write headers and allocate buffers for shade
**
** DESCRIPTION
**	Init_shade reads the input slope/azimuth header, allocates
**	necessary buffers, and writes output header.
**
** GLOBALS ACCESSED
**	BIH of input and output files
**	LQH of input and output files
**	tables of sines, cosines, and cosines of differences
**
*/

#include <math.h>

#include "ipw.h"
#include "getargs.h"

#include "shade.h"

extern double   azmf();
extern double   zenf();
extern void     buffers();
extern void     headers();
extern void     shadetbl();
extern void     trigtbl();
extern void	invzenf();

void
init_shade(fdi, fdo, opt_z, opt_u, opt_a)
	int             fdi;		/* input file descriptor	 */
	int             fdo;		/* output file descriptor	 */
	OPTION_T       *opt_z;
	OPTION_T       *opt_u;
	OPTION_T       *opt_a;
{
	double          zen;		/* solar zenith angle (deg)	 */
	double          azm;		/* solar azimuth (deg)		 */
	double          zenith;		/* solar zenith, rad		 */
	double          azimuth;	/* solar azimuth, rad		 */
	double          ctheta;		/* cosine solar zenith		 */
	double          stheta;		/* sine solar zenith		 */

 /*
  * process options
  */
	azm = real_arg(*opt_a, 0);
	azimuth = azmf(azm);
	if (got_opt(*opt_u)) {
		if (got_opt(*opt_z))
			warn("both -z and -u specified, -z ignored");
		ctheta = real_arg(*opt_u, 0);
		invzenf(ctheta, &zenith, &zen);
		stheta = sin(zenith);
	}
	else if (got_opt(*opt_z)) {
		zen = real_arg(*opt_z, 0);
		zenith = zenf(zen);
		ctheta = cos(zenith);
		stheta = sin(zenith);
	}
	else {
		error("must specify either -z or -u");
	}
 /*
  * process headers
  */
	headers(fdi, fdo, ctheta, azimuth);
 /*
  * allocate buffers
  */
	buffers(fdi);
 /*
  * values in trig tables
  */
	trigtbl(fdi, azimuth, sintbl, costbl, cosdtbl);
 /*
  * values in lookup table
  */
	shadetbl(fdi, fdo, (float) ctheta, (float) stheta,
		 costbl, sintbl, cosdtbl, shade);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/shade/RCS/init_shade.c,v 1.2 90/11/11 17:08:46 frew Exp $";

#endif
