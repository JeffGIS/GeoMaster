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
#include "getargs.h"

/*
** NAME
**	shade -- calculate cosine of local illumination angle
**
** SYNOPSIS
**	shade [-z zenith] [-u cos] -a azimuth [image]
**
** DESCRIPTION
**	shade reads a 2-band slope and aspect image (the output from
**	the IPW gradient command) from {image} (default: standard input),
**	and writes an image of local illumination cosines (relative to
**	a specified solar position) to the standard output.
**
** OPTIONS
**	-z	The solar zenith angle is {zenith} (0 .. 90) degrees.
**
**	-u	The cosine of the solar zenith angle is {cos} (0 .. 1).
**
**	At least on of -z or -u must be specified.  If both are specified,
**	then -z is ignored.
**
**	-a	The solar azimuth is {azimuth} degrees (-180 .. 180) counter-
**		clockwise from south (positive east, negative west).
**
** EXAMPLES
**	To produce a nice-looking shaded-relief map from a DEM (i.e.,
**	one with the sun in the cartographically traditional, but, for
**	the northern hemisphere, physically impossible position of 45
**	degrees above the northwest horizon):
**
**		gradient | shade -z 45 -a -135
**
** FILES
**
** DIAGNOSTICS
**	input file has no LQH
**
**		An "lq" header is necessary to convert the quantized
**		slopes and aspects to their actual values.
**
**	band 0 of input not slope
**
**		The range of floating-point values in band 0 is
**		inappropriate for quantized slopes.  shade will continue
**		executing, but the output values will almost certainly
**		be incorrect.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	8/28/90	Changed shade[s][a] calculation to prevent rounding.
**		Kelly Longley, ERL-C.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  gradient
**
**	Jeff Dozier and James Frew, "Rapid Calculation of Terrain Parameters
**		for Radiation Modeling from Digital Elevation Data," in
**		IGARSS '89 12th Canadian Symposium on Remote Sensing, vol. 3,
**		pp. 1769-1774, 1989.
*/

extern void     init_shade();
extern void     cosines();

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_a = {
		'a', "solar azimuth, degrees_ccw from south (+ E, - W)",
		REAL_OPTARGS, "azimuth",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_z = {
		'z', "solar zenith angle, degrees or",
		REAL_OPTARGS, "zenith",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_u = {
		'u', "cosine solar zenith angle",
		REAL_OPTARGS, "cos",
		OPTIONAL, 1, 1
	};

	static OPTION_T operand = {
		OPERAND, "input slope/azimuth file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_z,
		&opt_u,
		&opt_a,
		&operand,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */

	ipwenter(argc, argv, optv, "cosine of illumination angle");

 /*
  * access input file
  */
	if (!got_opt(operand)) {
		fdi = ustdin();
	}
	else {
		fdi = uropen(str_opnd(operand, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_opnd(operand, 0));
		}
	}

	no_tty(fdi);
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);

 /*
  * initialize - read headers & allocate buffers
  */

	init_shade(fdi, fdo, &opt_z, &opt_u, &opt_a);

 /*
  * read input data, calculate sun angle, and write
  */

	cosines(fdi, fdo);

 /*
  * all done
  */

	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/shade/RCS/main.c,v 1.3 90/11/11 17:08:52 frew Exp $";

#endif
