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

#include "pgm.h"

/*
** NAME
**	lqhx -- transform image to new linear quantization (LQ) header
**
** SYNOPSIS
**	lqhx [-h header] [-i image]
**
** DESCRIPTION
**	lqhx copies {image} to the standard output, requantizing its
**	pixels according to the quantization parameters of the {header}
**	image.  Specifically, the output bands will have the same pixel
**	sizes as {header}, and will receive (and be quantized according
**	to) any corresponding "lq" (linear quantization) headers in
**	{header}.
**
** OPTIONS
**	-h	Read quantization parameters from {header} (default:
**		standard input).  Pixel size information (number of
**		bytes, number of bits) is obtained from the BIH (basic
**		image header).  Any "lq" headers in {header} (there must
**		be at least one) are copied to the output image.  Any
**		other headers or image data in {header} is ignored.
**
**	-i	Read image data from {image} (default: standard input).
**
**	At least one of -h and/or -i must be specified.
**
** EXAMPLES
**	To requantize "image2" to the same pixel sizes and ranges of
**	values as in "image1":
**
**		lqhx -h image1 -i image2
**
**	To requantize a single-band "image" such that the input values
**	0 .. 1 are distributed over 10 bits:
**
**		mkbih -s 1 -l 1 -i 10 -f  |  mklqh -m 0,0,1023,1 -f  | \
**			lqhx -i image
**
**	Note that the -s and -l options are required by mkbih, even
**	though those header fields are subsequently ignored.
**
** FILES
**
** DIAGNOSTICS
**	new LQH not valid
**
**		The {header} file has an invalid "lq" header, or does not
**		have any "lq" headers.
**
**	input image and LQH file have different # bands
**
**		The {header} and {image} files must have the same number
**		of bands.
**
** RESTRICTIONS
**	lqhx does not check that the quantization borrowed from {header}
**	is appropriate for {image}, i.e., that the pixel values in {image}
**	lie in the ranges specified by the "lq" headers in {header}.
**	Pixels below (above) the range of output values will be set to
**	the lowest (highest) output value.
**
** FUTURE DIRECTIONS
**	Perhaps this should be implemented in combination with lutx.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  mkbih, mklqh
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_h = {
		'h', "new LQ header",
		STR_OPTARGS, "LQH",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_i = {
		'i', "input image file",
		STR_OPTARGS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_h,
		&opt_i,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
	    "transform image to new linear quantization (LQ) header");
 /*
  * access input LQH
  */
	if (!got_opt(opt_h)) {
		parm.h_fd = ustdin();
	}
	else {
		parm.h_fd = uropen(str_arg(opt_h, 0));
		if (parm.h_fd == ERROR) {
			error("can't open \"%s\"", str_arg(opt_h, 0));
		}
	}

	no_tty(parm.h_fd);
 /*
  * access input image
  */
	if (!got_opt(opt_i)) {
		if (!got_opt(opt_h)) {
			error("either -h or -i must be specified");
		}
		parm.i_fd = ustdin();
	}
	else {
		parm.i_fd = uropen(str_arg(opt_i, 0));
		if (parm.i_fd == ERROR) {
			error("can't open \"%s\"", str_arg(opt_i, 0));
		}
	}

	no_tty(parm.i_fd);
 /*
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * do it
  */
	headers();
	lqhx();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/lqhx/RCS/main.c,v 1.2 90/11/11 17:05:42 frew Exp $";

#endif
