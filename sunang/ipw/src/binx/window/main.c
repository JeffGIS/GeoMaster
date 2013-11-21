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

#include "window.h"

/*
** NAME
**	window -- extract image window
**
** SYNOPSIS
**	window [-b line,samp] [-c line,samp] [-e line,samp]
**	       [-n nlines,nsamps] [-w band] [-g band] [image]
**
** DESCRIPTION
**	window reads an image from {image} (default: standard input) and
**	writes a specified window (subimage) to the standard output.
**
** OPTIONS
**	-b	The beginning (upper-left corner) of the window is
**		{line},{samp} (default: beginning of input image).
**
**		If only -b is specified, then the window extends diagonally
**		from {line},{samp} to the last sample on the last line of
**		the input image.
**
**	-c	The center of the window is {line},{samp}.  The center of
**		an even number of lines or samples is the larger of the two
**		possible values; e.g., the center line of lines 0..511 is
**		256, not 255.
**
**		If only -c is specified, then the window is the largest
**		possible odd number of input lines and samples centered
**		on {line},{samp}.
**
**	-e	the end (lower-right corner) of the window is {line},{samp}
**		(default: end of input image).
**
**		If only -e is specified, then the window extends diagonally
**		from the first sample on the first line of the input image
**		to {line},{samp}.
**
**	-n	The window has {nlines} lines and {nsamps} samples per
**		line (default: remainder of image).
**
**		If only -n is specified, then the window begins at the
**		first sample of the first line of the input image.
**
**	At least one, and no more than two, of -b, -c, -e, and/or -n
**	must be specified.
**
**	-w	The arguments to -b, -c, and/or -e are specified in the
**		coordinates of band {band}'s window ("win") header.
**
**	-g	The arguments to -b, -c, and/or -e are specified in the
**		coordinates of band {band}'s geodetic ("geo") header.
**
**	At most one of -w or -g may be specified.
**
** EXAMPLES
**	To produce a 2x enlargement of the center of a 512x512 image:
**
**		window -c 256,256 -n 256,256 | zoom -l 2 -s 2
**
**	To extract a 1 km by 1 km window from a 5-meter DEM, beginning
**	at UTM northing 4051800 and easting 349350:
**
**		window -g 0 -b 4051800,349350 -e 4050805,350345
**
** FILES
**	$TMPDIR/windo{NNNNN}
**
**		Temporary copy of all input headers.
**
** DIAGNOSTICS
**	specified window exceeds boundaries of input image
**
**		Windows are not clipped to fit the input image.  The
**		specification must be correct to start with.
**
**	no geodetic header for band {band}
**	no window header for band {band}
**
**		-g or -w was specified and there was no corresponding
**		header for the specified input band.
**
** RESTRICTIONS
**	Input "win" and "geo" headers are transformed correctly; other
**	input headers are copied verbatim to the output image.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  mkgeoh, mkwinh, zoom
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_b = {
		'b', "begin line,sample",
		REAL_OPTARGS, "coord",
		OPTIONAL, 2, 2
	};

	static OPTION_T opt_c = {
		'c', "center line,sample",
		REAL_OPTARGS, "coord",
		OPTIONAL, 2, 2
	};

	static OPTION_T opt_e = {
		'e', "end line,sample",
		REAL_OPTARGS, "coord",
		OPTIONAL, 2, 2
	};

	static OPTION_T opt_n = {
		'n', "output image size (#lines,#samples)",
		INT_OPTARGS, "size",
		OPTIONAL, 2, 2
	};

	static OPTION_T opt_w = {
		'w', "window specified re: this band's window header",
		INT_OPTARGS, "band",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_g = {
		'g', "window specified re: this band's geodetic header",
		INT_OPTARGS, "band",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_b,
		&opt_c,
		&opt_e,
		&opt_n,
		&opt_w,
		&opt_g,
		&operands,
		0
	};

	static XWSPEC_T  xw;		/* window specification (init 0) */

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "extract image window");

	opt_check(1, 2, 4, &opt_b, &opt_c, &opt_e, &opt_n);
	opt_check(0, 1, 2, &opt_w, &opt_g);
 /*
  * collect options
  */
	if (got_opt(opt_b)) {
		xw.bline = real_arg(opt_b, 0);
		xw.bsamp = real_arg(opt_b, 1);

		xw.flags |= GOT_BEGIN;
	}

	if (got_opt(opt_c)) {
		xw.cline = real_arg(opt_c, 0);
		xw.csamp = real_arg(opt_c, 1);

		xw.flags |= GOT_CENTER;
	}

	if (got_opt(opt_e)) {
		xw.eline = real_arg(opt_e, 0);
		xw.esamp = real_arg(opt_e, 1);

		xw.flags |= GOT_END;
	}

	if (got_opt(opt_n)) {
		xw.nlines = int_arg(opt_n, 0);
		xw.nsamps = int_arg(opt_n, 1);

		xw.flags |= GOT_SIZE;
	}

	if (got_opt(opt_w)) {
		xw.band = int_arg(opt_w, 0);

		xw.flags |= GOT_XWIN;
	}
	else if (got_opt(opt_g)) {
		xw.band = int_arg(opt_g, 0);

		xw.flags |= GOT_XGEO;
	}
 /*
  * access input file(s)
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

	no_tty(fdi);
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	window(fdi, &xw, fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/window/RCS/main.c,v 1.2 90/11/11 17:10:48 frew Exp $";
#endif
