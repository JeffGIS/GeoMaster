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
**	viewcalc -- compute sky view and terrain configuration factors
**
** SYNOPSIS
**	viewcalc [-s gradient] [-h horizons]
**
** DESCRIPTION
**	viewcalc reads a gradient image (calculated by "gradient") and
**	a horizon angle image (calculated by "horizon") and writes a
**	2-band image of the corresponding sky view and terrain
**	configuration factors to the standard output.
**
**	The input horizon angle image should be multiband, with each
**	band representing the horizon in one of {nbands} equiangularly
**	spaced directions (i.e., the result of "mux"ing together several
**	output images from "horizon").  They need to be evenly spaced,
**	but need not be in consecutive order.
**
**	Band 0 of the output image is the sky view factor, defined as
**	the fraction (0 .. 1) of a pixel's overlying hemisphere (centered
**	at the pixel's zenith) that is subtended by sky (as opposed to
**	surrounding terrain).
**
**	Band 1 of the output image is the terrain configuration factor,
**	defined as:
**
**		1 + cos(slope)
**	       ----------------   -  (sky view factor)
**	              2
**
** OPTIONS
**	-s	Read slopes and aspects from {gradient} (default: standard
**		input).
**
**	-h	Read horizon angles from {horizons} (default: standard
**		input).
**
**	At least one of -s and/or -h must be specified.
**
** EXAMPLES
**	viewcalc is almost always invoked by viewf, which first calculates
**	the necessary gradient and horizon images.
**
** FILES
**
** DIAGNOSTICS
**	# samples different in two input files
**	files unequal # pixels
**
**		The gradient and horizon angle images must have the same
**		number of lines and samples.
**
**	band 0 of input not slope
**	input slope/azimuth file must have 2 bands
**	slope/azm image has no LQH
**
**		The gradient image must be a valid output from the gradient
**		program.
**
**	horizon image has no HORH
**	horizon image has no LQH
**
**		The horizon image must be composed of valid outputs from the
**		horizon program.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  gradient, hor1d, horizon, viewf
**
**	Jeff Dozier and James Frew, "Rapid Calculation of Terrain Parameters
**		for Radiation Modeling from Digital Elevation Data," in
**		IGARSS '89 12th Canadian Symposium on Remote Sensing, vol. 3,
**		pp. 1769-1774, 1989.
*/

#include "ipw.h"
#include "getargs.h"

#include "view.h"

void
DEFUN(main,(argc, argv),
	int             argc
   AND  char          **argv)
{
	static OPTION_T opt_s = {
		's', "input slope/aspect image (output from gradient)",
		STR_OPTARGS, "slope/aspect",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_h = {
		'h', "input multi-angle horizon image",
		STR_OPTARGS, "horizons",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_s,
		&opt_h,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
		 "sky view & terrain configuration factors");

 /*
  * access input files
  */
	if (!got_opt(opt_s)) {
		parm.i_fds = ustdin();
	}
	else {
		parm.i_fds = uropen(str_arg(opt_s, 0));
		if (parm.i_fds == ERROR) {
			error("can't open \"%s\"", str_arg(opt_s, 0));
		}
	}
	if (!got_opt(opt_h)) {
		if (!got_opt(opt_s))
			error("must specify at least one of -s -h");
		parm.i_fdh = ustdin();
	}
	else {
		parm.i_fdh = uropen(str_arg(opt_h, 0));
		if (parm.i_fdh == ERROR) {
			error("can't open \"%s\"", str_arg(opt_h, 0));
		}
	}

	no_tty(parm.i_fds);
	no_tty(parm.i_fdh);
 /*
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * process headers
  */
	headers();
 /*
  * allocate buffers
  */
	buffers();
 /*
  * values in trig tables
  */
	trigtbl();
 /*
  * read input data and calculate view factors, and write
  */
	viewf();
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/viewcalc/RCS/main.c,v 1.2 90/11/11 17:10:15 frew Exp $";

#endif
