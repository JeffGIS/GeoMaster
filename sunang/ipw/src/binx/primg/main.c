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

#include "primg.h"

/*
** NAME
**	primg -- print image pixel values as ASCII text
**
** SYNOPSIS
**	primg [-a] [-r] [-c coords] [-i image]
**
** DESCRIPTION
**	primg prints input image pixels as text to the standard output,
**	one sample per line.  The pixel values for each band are printed
**	in band order from left to right, separated by white space.
**
** OPTIONS
**	-a	Print all pixels in image (default: print only pixels
**		specified in {coords} or on standard input).
**
**	-c	Read pixel coordinates {coords} (default: standard input).
**
**	At most one of -a or -c may be specified.
**
**	-i	Read image data from {image} (default: standard input).
**
**	At least one of -a, -c, and/or -i must be specified.
**
**	-r	Print raw pixel values, bypassing any conversion to
**		floating point (such as via input "lq" headers).
**
** EXAMPLES
**	To interactively examine pixel values in "image", type:
**
**		primg -i image
**
**	then type the pixel coordinates on the standard input (but note
**	that the coordinates must be typed in increasing line order).
**
**	If "basin" contains the (line,sample) coordinates of the corners
**	of a drainage basin in the DEM "dem", and {line},{samp} are the
**	coordinates of an arbitrary point within the drainage basin, then:
**
**		poly -s {line},{samp} basin | primg -i dem
**
**	will print all of the pixel values in "dem" that lie within the
**	drainage basin.
**
** FILES
**
** DIAGNOSTICS
**	bad coordinate file line: {text}
**
**		{coords} contains a line {text} that cannot be parsed as
**		two non-negative integers.
**
**	bad coordinates (not on image): {line},{sample}
**
**		{line},{sample} are illegal coordinates for the input
**		image.  Note that the coordinates are raw coordinates,
**		they are not defined in terms of any "geo" or "win"
**		headers.  Use cell2point if you must use those.
**
**	unsorted coordinates: {line},{sample}
**
**		The input coordinates must be sorted in ascending line
**		order (sample order in unimportant).
**
** RESTRICTIONS
**	The input coordinates must be sorted in ascending line order (sample
**	order is unimportant).
**
**	-a without -r can be very slow.
**
** FUTURE DIRECTIONS
**	Allow pixel coordinates to be specified in terms of a window or
**	geodetic header.  The program "cell2point" does this.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  cell2point, poly
**	UNIX: sort
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_a = {
		'a', "print all pixels",
	};

	static OPTION_T opt_r = {
		'r', "print raw values (i.e., don't convert to floating-point)",
	};

	static OPTION_T opt_c = {
		'c', "input coordinate file (default: stdin)",
		STR_OPTARGS, "coordfile",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_i = {
		'i', "input image file (default: stdin)",
		STR_OPTARGS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_a,
		&opt_r,
		&opt_c,
		&opt_i,
		0
	};

	int             fdc;		/* coordinate file descriptor	 */
	int             fdi;		/* input image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "print image pixels");

	opt_check(1, 2, 3, &opt_i, &opt_c, &opt_a);
	opt_check(0, 1, 2, &opt_c, &opt_a);
 /*
  * access input file(s)
  */
	if (!got_opt(opt_i)) {
		fdi = ustdin();
	}
	else {
		fdi = uropen(str_arg(opt_i, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(opt_i, 0));
		}
	}

	no_tty(fdi);

	if (got_opt(opt_a)) {
		fdc = ERROR;
	}
	else if (!got_opt(opt_c)) {
		fdc = ustdin();
	}
	else {
		fdc = uropen(str_arg(opt_c, 0));
		if (fdc == ERROR) {
			error("can't open \"%s\"", str_arg(opt_c, 0));
		}
	}
 /*
  * do it
  */
	primg(fdi, fdc, got_opt(opt_r));
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/primg/RCS/main.c,v 1.5 90/11/11 17:07:56 frew Exp $";

#endif
