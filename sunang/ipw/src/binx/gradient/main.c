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

#include "fpio.h"
#include "getargs.h"
#include "orh.h"
#include "pgm.h"

/*
** NAME
**	gradient -- slope and aspect of image
**
** SYNOPSIS
**	gradient [-s] [-a] [-d deltaL[,deltaS]] [-i Sbits[,Abits]] [image]
**
** DESCRIPTION
**	gradient reads elevations from {image} (default: standard input)
**	and writes a 2-band image of the slope and aspect (the magnitude
**	and direction of the gradient) to the standard output.
**
**	The 2-band output image has slope as its first band and
**	aspect as its second.  Slope is stored as sin(S) with range
**	from 0 to 1.  Aspect is stored as radians from south (aspect 0
**	is toward the south) with range from -pi to pi, with negative
**	values to the west and positive values to the east.
**
** OPTIONS
**	-s	compute slope image only (default: compute both slope and
**		aspect images).
**
**	-a	compute aspect image only (default: compute both slope and
**		aspect images).
**
**	-d	grid spacing (default: get grid spacing from the "geo"
**		header or set to 1 if no "geo" header).  If {deltaS} is
**		also specified, use {deltaL} for the input line spacing
**		and {deltaS} for the input sample spacing.
**
**	-i	Use {Sbits} per output pixel (default: 8).  If {Abits} is
**		also specified, use {Sbits} bits per slope pixel and
**		{Abits} bits per aspect pixel.
**
** EXAMPLES
**	To produce a nice-looking shaded-relief map from the DEM "elev.utah"
**	we can use the program shade, which needs slopes and aspects:
**
**		gradient elev.utah | shade -z 45 -a -135
**
** FILES
**
** DIAGNOSTICS
**	# bits must be >= 1
**	-d {delta},{delta} : must be positive
**
**		The arguments to the -d and -i options must be positive
**		nonzero integers.
**
**	input file has {nbands} bands
**
**		The input image must have only 1 band.
**
**	Elevation file has no GEOH, spacing set to 1.0
**	input file has no LQH; raw values used
**	input units "{units}", should be "m"
**
**		These deficiencies in the input image will introduce
**		linear errors into the slope calculations.
**
**	Elevation file should be standard orientation
**
**		The output azimuth values will have a systematic bias
**		corresponding to the non-standard orientation of the
**		input image.
**
**	spacing in geodetic header ignored
**
**		The -d option overrides any pixel spacing information
**		in the input image.
**
** RESTRICTIONS
**	gradient is optimized for terrain calculations (e.g., storing
**	slopes as sines offers increased precision for shallow slopes,
**	which are more common in nature), and may therefore be less
**	than ideal as a generic image derivative program.
**
** FUTURE DIRECTIONS
**	This version of the program is not heavily optimized.  For a
**	sample image of the Emerald Lake watershed it takes 5-6 times
**	as long as the heavily optimized QDIPS slope.expo program.
**	However, this one works!  Future work toward optimization is
**	planned.  We can try all sorts of methods to make this go
**	faster, but this version can be used as a benchmark.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**	4/30/93	Changed aspect calculation to explicitly handle cases of
**		dx or dy being zero.  Dana Jacobsen, ERL-C.
**
** BUGS
**	-i should not be used for bits in output!  Use -n or -b, but that
**	would break all the scripts that depend on gradient as it is.
**
** SEE ALSO
**	IPW:  demux, mkgeoh, mklqh, shade
*/

void
main(argc, argv)
	int             argc;
	char           *argv[];
{
	static OPTION_T opt_s = {
		's', "compute slope image only (default both)"
	};

	static OPTION_T opt_a = {
		'a', "compute aspect image only"
	};

	static OPTION_T opt_d = {
		'd',
		"grid spacing (line & samp, normally gotten from GEOH header)",
		REAL_OPTARGS, "delta",
		OPTIONAL, 1, 2
	};

	static OPTION_T opt_i = {
		'i',
		"# bits in output linear quantization (default CHAR_BIT)",
		INT_OPTARGS, "bits",
		OPTIONAL, 1, 2
	};

	static OPTION_T operand = {
		OPERAND, "input elevation image",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_s,
		&opt_a,
		&opt_i,
		&opt_d,
		&operand,
		0
	};

	int             fdi = 0;	/* input image file descriptor	 */
	int             fdo = 0;	/* output image file descriptor	 */
	int             nbits[2];	/* # bits in output LQ's	 */
	bool_t          s;		/* ? compute slopes		 */
	bool_t          a;		/* ? compute aspects		 */
	fpixel_t        spacing[2];	/* grid spacing			 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "slope and aspect of image");
 /*
  * access file
  */
	switch (n_args(operand)) {

	case 0:			/* no operand, open std input	 */
		fdi = ustdin();
		break;

	case 1:			/* open named file		 */
		fdi = uropen(str_opnd(operand, 0));
		if (fdi == ERROR)
			error("Can't open \"%s\"", str_opnd(operand, 0));
		break;

	default:			/* too many operands		 */
		usage();
	}
 /*
  * can't read or write tty
  */
	no_tty(fdi);
	fdo = ustdout();
	no_tty(fdo);
 /*
  * parse options
  */
	options(&opt_s, &opt_a, &opt_d, &opt_i, &s, &a, nbits, spacing);
 /*
  * read/write headers, either use spacing provided or get from
  * geodetic header
  */
	headers(fdi, fdo, s, a, nbits, spacing);
 /*
  * read elevation data, compute partial x & partial y, compute and
  * write gradient
  */
	gradient(fdi, fdo, s, a, spacing);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/main.c,v 1.6 90/11/11 17:03:25 frew Exp $";

#endif
