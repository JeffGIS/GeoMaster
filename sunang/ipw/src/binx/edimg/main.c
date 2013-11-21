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

#include "edimg.h"

/*
** NAME
**	edimg -- replace image pixels
**
** SYNOPSIS
**	edimg [-r] [-k const] [-c coords] [-i image]
**
** DESCRIPTION
**	edimg copies the input {image} to the standard output, replacing
**	specified pixel values.  Replacement values are read from the
**	text file {coords}.  Each value is represented by as a single
**	line with the format:
**
**		line sample constant
**
**	indicating that the value of the pixel at location {line},{sample}
**	is to be replaced with {constant}.  If {constant} is missing, the
**	default value specified by -k is used.
**
** OPTIONS
**	-r	Replacement values are "raw" (i.e., should not be converted
**		from floating-point before storing in pixels).  Any input
**		"lq" headers will be ignored.
**
**	-k	The default replacement pixel value is {const} (default: 0).
**
**	-c	Coordinates are read from {coords} (default: standard input).
**
**	-i	Image data is read from {image} (default: standard input).
**
**	At least one of -c and/or -i must be specified.
**
** EXAMPLES
**	To set the pixel(s) at (100,200) in "image" to 255:
**
**		edimg -r -i image
**		100 200 255
**
** FILES
**
** DIAGNOSTICS
**	invalid coordinates: {text}
**
**		The {text} line is not a valid replacement value
**		specification.
**
**	unsorted coordinates: {line}, {sample}
**
**		The coordinates must be sorted in order of increasing
**		line numbers.
**
** RESTRICTIONS
**	The coordinates must be sorted in order of increasing line numbers
**	(the order of the samples is unimportant).
**
**	If the input image has more than one band, then ALL bands at a
**	specified location are set to the corresponding replacement value.
**
**	The input {line} and {sample} values are always raw.  They are
**	unaffected by any coordinate system headers such as "geo", "win",
**	etc. in the input image.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  poly
**	UNIX: sort
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_r = {
		'r', "replacement values are raw (i.e., not floating-point)",
	};

	static OPTION_T opt_k = {
		'k', "default replacement pixel value",
		REAL_OPTARGS, "const",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_c = {
		'c', "coordinate file",
		STR_OPTARGS, "coords",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_i = {
		'i', "input image",
		STR_OPTARGS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_r,
		&opt_k,
		&opt_c,
		&opt_i,
		0
	};

	double          k;		/* default replacement value	 */
	int             c_fd;		/* coordinate file descriptor	 */
	int             i_fd;		/* input image file descriptor	 */
	int             o_fd;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "edit image");

	opt_check(1, 2, 2, &opt_c, &opt_i);
 /*
  * process options
  */
	k = got_opt(opt_k) ? real_arg(opt_k, 0) : DFLT_CONST;
 /*
  * access input image file
  */
	if (!got_opt(opt_i)) {
		i_fd = ustdin();
	}
	else {
		i_fd = uropen(str_arg(opt_i, 0));
		if (i_fd == ERROR) {
			error("can't open \"%s\"", str_arg(opt_i, 0));
		}
	}

	no_tty(i_fd);
 /*
  * access coordinate file
  */
	if (!got_opt(opt_c)) {
		c_fd = ustdin();
	}
	else {
		c_fd = uropen(str_arg(opt_c, 0));
		if (c_fd == ERROR) {
			error("can't open \"%s\"", str_arg(opt_c, 0));
		}
	}
 /*
  * access output file
  */
	o_fd = ustdout();
	no_tty(o_fd);
 /*
  * do it
  */
	edimg(i_fd, c_fd, got_opt(opt_r), k, o_fd);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/edimg/RCS/main.c,v 1.8 90/11/11 17:02:22 frew Exp $";

#endif
