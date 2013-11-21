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

#include <math.h>
#include "ipw.h"

#include "bih.h"
#include "getargs.h"
#include "horizon.h"

/*
** NAME
**	hor1d -- find angles to local horizon along rows of elevation file
**
** SYNOPSIS
**	hor1d -a phi [-b] [-z zen] [-u cos] [-d delta] [-n nbits] [image]
**
** DESCRIPTION
**	hor1d reads elevations from {image} (default: standard input)
**	and writes an image of along-line horizons to the standard
**	output.
**
**	hor1d is almost always invoked indirectly by the horizon command,
**	which allows horizons to be computed along arbitrary azimuths.
**
** OPTIONS
**	-a	direction of forward azimuth (degrees east of south).
**
**	-b	compute backward horizons (default: forward).
**
**	-d	grid spacing (default: get grid spacing from the "geo"
**		header or set to 1 if no "geo" header).
**
**	-z	mask horizon angles with solar zenith angles greater than
**		{zen} degrees (0..90).
**
**	-u	mask horizon angles with cosines greater than {cos}.
**
**	Only one of -z or -u should be specified.
**
**	-n	Use {nbits} bits per output pixel (default: 8).
**
**	All of the options but -b are the same as horizon, where they are
**	described in more detail.
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**	Diagnostics are described in the manual for horizon.
**
** RESTRICTIONS
**	The value of the "-a" option is not used by hor1d, but it is
**	essential for interpreting hor1d's output, so it is stored in
**	the header of the output image.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	 Written by James Frew, UCSB.
**	4/7/93	 Converted to ANSI C and removed cpp hack.  Fixed failure when
**		 passing float as double.  Added prototype for hypot that is
**		 necessary on AIX and Solaris 2.x.  Dana Jacobsen, ERL-C.
**	4/26/93	 Ran through Purify.  Dana Jacobsen, ERL-C.
**	7/27/95	 Added -n option.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW: horizon
*/

void
DEFUN(main, (argc, argv),
	int             argc
   AND  char          **argv)
{
	static OPTION_T opt_a = {
		'a', "direction of forward azimuth (degrees)",
		REAL_OPTARGS, "phi",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_d = {
		'd', "grid spacing (normally obtained from GEOH header)",
		REAL_OPTARGS, "delta",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_b = {
		'b', "backward direction (default: forward)"
	};

	static OPTION_T opt_z = {
		'z', "mask option, arg is solar zenith angle (degrees) or",
		REAL_OPTARGS, "zen",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_u = {
		'u', "mask option, arg is cosZ",
		REAL_OPTARGS, "cos",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_n = {
		'n', "# bits / output pixel (default: 8)",
		INT_OPTARGS, "#bits",
		OPTIONAL, 1, 1
	};

	static OPTION_T efile = {
		OPERAND, "input elevation image",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_a,
		&opt_b,
		&opt_z,
		&opt_u,
		&opt_d,
		&opt_n,
		&efile,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
	 "find angles to local horizon along rows of elevation file");
 /*
  * azimuth
  */
	parm.azimuth = azmf(real_arg(opt_a, 0));
 /*
  * spacing
  */
	if (got_opt(opt_d)) {
		parm.spacing = real_arg(opt_d, 0);
		if (parm.spacing <= 0)
			error("-d %g: must be positive", parm.spacing);
	}
	else {
		parm.spacing = 0;
	}
 /*
  * mask option, solar zenith angle
  */
	if (got_opt(opt_u)) {
		errno = 0;
		parm.zenith = acos(real_arg(opt_u, 0));
		if (errno)
			error("bad return from acos()");
		if (got_opt(opt_z))
			warn("both -u and -z specified, -z over-ridden");
	}
	else if (got_opt(opt_z)) {
		parm.zenith = zenf(real_arg(opt_z, 0));
	}
	else {
		parm.zenith = 0;
	}
 /*
  * access input file
  */
	switch (n_args(efile)) {
	case 0:			/* no operand, open std input	 */

		parm.i_fd = ustdin();
		break;
	case 1:			/* open named file	 	 */
		parm.i_fd = uropen(str_opnd(efile, 0));
		if (parm.i_fd == ERROR)
			error("can't open \"%s\"", str_opnd(efile, 0));
		break;
	default:			/* too many operands	 	 */
		usage();
	}
 /*
  * can't read or write tty
  */
	no_tty(parm.i_fd);
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * # output bits, just 1 if mask
  */
	if (got_opt(opt_n)) {
		parm.nbits = int_arg(opt_n, 0);
	} else {
		parm.nbits = (parm.zenith != 0) ? 1 : CHAR_BIT;
	}

 /*
  * backward flag; default is forward
  */
	parm.backward = got_opt(opt_b);
 /*
  * process azimuth direction
  */
	if (parm.backward) {
		if (parm.azimuth >= 0)
			parm.azimuth -= PI;
		else
			parm.azimuth += PI;
	}
 /*
  * read/write headers
  */
	headers();
 /*
  * compute horizon
  */
	horizon();
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hor1d/RCS/main.c,v 1.2 90/11/11 17:04:28 frew Exp $";

#endif
