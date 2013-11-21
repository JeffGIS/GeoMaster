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

#include "mksunh.h"

/*
** NAME
**	mksunh -- add a solar position header to an image
**
** SYNOPSIS
**	mksunh -z cosz -a azimuth [-f] [image]
**
** DESCRIPTION
**	mksunh constructs an IPW solar position header (SUNH) and inserts it
**	into the headers of {image} (default: standard input).  If the
**	specified bands already have SUN headers, they are overridden, and a
**	warning message is generated.
**
**	If the -f option is specified, then only the headers are written to
**	the standard output, and any input image data is ignored.
**
** OPTIONS
**	-z	The cosine of the solar zenith angle is {cosz}.
**
**	-a	The solar azimuth is {azimuth}, in radians from south,
**		positive east.
**
**	Both -z and -a must always be specified.
**
**	-f	Force header output only.  Do not attempt to copy pixel
**		data from {image} to the standard output.  Note that there
**		must still be at least an input BIH, and any input headers
**		(except superseded "sun" headers) will still be copied to
**		the standard output.
**
** EXAMPLES
**	To create a "sun" header for a Landsat image acquired with a solar
**	elevation of 57 degrees and a solar compass bearing of 119 degrees:
**
**		mksunh -z 0.838671 -a 1.064651 image
**
**	where  0.838671 = cos(90-57)  and  1.064651 = (180-119) * (PI/180)
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	The "sun" header will always be applied to all bands of the image.
**
** FUTURE DIRECTIONS
**	mksunh should accept different angular units.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW: mkbih, mk*h, prhdr, rmhdr
*/

void
main(argc, argv)
	int             argc;		/* # arguments			 */
	char          **argv;		/* ->'s to arguments		 */
{
	static OPTION_T opt_a = {
		'a', "solar azimuth from south",
		REAL_OPTARGS, "azimuth",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_z = {
		'z', "cosine of solar zenith angle",
		REAL_OPTARGS, "cos(zenith)",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_f = {
		'f', "force header output; ignore any input image",
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_a,
		&opt_z,
		&opt_f,
		&operands,
		0
	};

	double          azimuth;	/* solar azimuth from south 	 */
	int             fdi;		/* intput file descriptor	 */
	int             fdo;		/* output file descriptor	 */
	double          cos_zen;	/* cosine of solar zenith angle	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "make an IPW sun header");
 /*
  * process options
  */
	if (got_opt(opt_a)) {
		azimuth = real_arg(opt_a, 0);
	}
	else {
		error("must specify solar azimuth");
	}

	if (got_opt(opt_z)) {
		cos_zen = real_arg(opt_z, 0);
	}
	else {
		error("must specify cosine of solar zenith");
	}
 /*
  * access input file
  */
	if (!got_opt(operands)) {
		fdi = ustdin();
	}
	else {
		fdi = uropen(str_arg(operands, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(operands, 0));
		}
	}
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	mksunh(fdi, azimuth, cos_zen, got_opt(opt_f), fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mksunh/RCS/main.c,v 1.3 90/11/11 17:06:31 frew Exp $";

#endif
