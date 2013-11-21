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
#include "skewh.h"
#include "skew.h"

/*
** NAME
**	skew -- skew image lines
**
** SYNOPSIS
**	skew [-a angle] [-h] [image]
**
** DESCRIPTION
**	skew copies {image} (default: standard input) to the standard
**	output, skewing the origin of successive lines by a specified
**	angle.
**
** OPTIONS
**	-a	Introduce horizontal skew such the the left edge of the output
**		image is tilted "angle" degrees clockwise from vertical.  The
**		resulting "dead space" in the output image is 0-filled.  A
**		skew header is written to the output image.
**
**	If -a is not specified, then the skew indicated by the input skew
**	header is removed.
**
**	-h	ignore input skew header.  This allows an image to be skewed
**		more than once, e.g. during rotation by shearing.
**
**	-h may not be specified unless -a is also specified.
**
** EXAMPLES
**	The command:
**
**		skew -a 30
**
**	causes the following transformation:
**
**		+-----------+		+---------------+
**		|           |		|000/          /|
**		|   input   |		|00/  output  /0|
**		|   image   |		|0/   image  /00|
**		|           |		|/          /000|
**		+-----------+		+---------------+
**
** FILES
**	$TMPDIR/skew{NNNNN}
**
**		Temporary copy of all input headers.
**
** DIAGNOSTICS
**	image is already skewed
**
**		-a was specified, and the input image contains a "skew"
**		header.
**
**	image is not skewed
**
**		-a was not specified, and the input image does not contain
**		a "skew" header.
**
**	band {band} has no skew header
**	different skew angles: bands 0, {band}
**
**		If -a is not specified, then all input bands must have a
**		"skew" header, and they must be the same.
**
** RESTRICTIONS
**	The skew angle given by -a must be between -45 and 45 degrees.
**
**	If -a is specified then the input image must NOT have a skew header,
**	unless -h is also specified, and a "skew" header is written to the
**	output image.
**
**	If -a is NOT specified then the input image must contain a "skew"
**	header with the same skew angle for all bands.  The skew indicated
**	by this header is removed from the output image, and no "skew"
**	header is written to the output image.
**
** FUTURE DIRECTIONS
**	skew may be used in conjunction with flip and transpose to reorient
**	an image's scan lines at an arbitrary angle with respect to the
**	original scan lines.  This is known as "Paeth" rotation, and might
**	be implemented as a separate program.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Converted to ANSI C, ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:   flip, horizon, transpose, viewf
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_a = {
		'a', "skew angle",
		REAL_OPTARGS, "angle",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_h = {
		'h', "ignore input skew header",
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_a,
		&opt_h,
		&operands,
		0
	};

	double          angle;		/* skew angle			 */
	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "skew image lines");

	angle = 0.0;
 /*
  * process options
  */
	if (got_opt(opt_a)) {
		angle = real_arg(opt_a, 0);
		if (angle < SKEW_MIN || angle > SKEW_MAX) {
			error("skew angle must be >= %f and <= %f",
			      SKEW_MIN, SKEW_MAX);
		}
	}
	else if (got_opt(opt_h)) {
		error("can't specify -h without -a");
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
	skew(fdi, got_opt(opt_a), angle, got_opt(opt_h), fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Id: main.c,v 1.4 90/11/11 17:09:09 frew Exp $";

#endif
