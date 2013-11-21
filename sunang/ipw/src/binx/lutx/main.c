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

#include "lutx.h"

/*
** NAME
**	lutx -- apply lookup table to image
**
** SYNOPSIS
**	lutx [-l lut] [-i image]
**
** DESCRIPTION
**	lutx copes the input {image} to the standard output, transforming
**	its pixel values according to the lookup table {lut}.
**
**	A lookup table, like a histogram, is a single-line IPW image.  To
**	transform a pixel from a given band through a lookup table, the
**	value of the pixel is interpreted as an image sample coordinate.
**	The appropriate band at that sample in the lookup table supplies
**	the replacement pixel value.
**
** OPTIONS
**	-l	Read lookup table from {lut} (default: standard input).
**
**	-i	Read image from {image} (default: standard input).
**
**	At least one of -l and/or -i must be specified.
**
** EXAMPLES
**	To produce a histogram-equalized version of "image":
**
**		hist image | histeq | lutx -i image
**
**	To convert "image" with 12-bit pixels to 8-bit pixels, with
**	linear scaling:
**
**		interp | mklut -i 12 | lutx -i image
**		0 0
**		4095 255
**
** FILES
**
** DIAGNOSTICS
**	{n}-element LUT can't map {nbits}-bit pixels
**
**		The number of elements (samples) in the lookup table
**		must be at least as large as the number of possible
**		pixel values in any band of the input image.
**
**	image and LUT have different # bands
**
**		{image} and {lut} must have the same number of bands.
**
**	not a lookup table (# lines > 1)
**
**		{lut} must be a 1-line IPW image.
**
** RESTRICTIONS
**	All input image headers are copied to the output image, even those
**	(such as "lq" headers) whose contents may be invalidated by lutx's
**	arbitrary modifications to the input pixel values.  There seems to
**	be no simple solution to this problem.
**
** FUTURE DIRECTIONS
**	Pass through to the output image those headers that are invariant
**	under a lookup-table transform.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  hist, histeq, interp, mklut
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_i = {
		'i', "input image file",
		STR_OPTARGS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_l = {
		'l', "lookup table file",
		STR_OPTARGS, "LUT",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_i,
		&opt_l,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdl;		/* LUT file descriptor		 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "apply lookup table to image");

	opt_check(1, 2, 2, &opt_i, &opt_l);
 /*
  * access input image
  */
	if (got_opt(opt_i)) {
		fdi = uropen(str_arg(opt_i, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(opt_i, 0));
		}
	}
	else {
		fdi = ustdin();
	}

	no_tty(fdi);
 /*
  * access LUT
  */
	if (got_opt(opt_l)) {
		fdl = uropen(str_arg(opt_l, 0));
		if (fdl == ERROR) {
			error("can't open \"%s\"", str_arg(opt_l, 0));
		}
	}
	else {
		fdl = ustdin();
	}

	no_tty(fdl);
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	lutx(fdi, fdl, fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/lutx/RCS/main.c,v 1.2 90/11/11 17:05:56 frew Exp $";

#endif
