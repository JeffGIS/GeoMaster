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

#include "wedge.h"

/*
** NAME
**	wedge -- linear combination of line and sample coordinates
**
** SYNOPSIS
**	wedge -c lcoef,scoef[,...] [-n nbits] [image]
**
** DESCRIPTION
**	wedge reads the headers from {image} (default: standard input) and
**	writes an image with the same number of lines and samples to the
**	standard output.  The value of each output pixel is a linear
**	combination of the pixel's raw line and sample coordinates.
**
**	The number of coefficients specified must be a multiple of 2, and
**	the number of output bands will be half the number of coefficients.
**
** OPTIONS
**	-c	Output pixels are assigned the value:
**
**			line * {lcoef}  +  sample * {scoef}
**
**		Each pair of {lcoef},{scoef} will be used to generate
**		a new output band.
**
**	-n	Use {nbits} per output pixel (default:
**		log2(max(nlines,nsamps)), where nlines and nsamps are the
**		dimensions of {image}).
**
** EXAMPLES
**	To produce a 512 by 512 single-band diagonal wedge with 8-bit pixels
**	increasing in intensity from the upper left corner:
**
**		mkbih -l 512 -s 512 -f | wedge -c 1,1 -n 8
**
**	Note that the input image can be a standalone BIH since only the
**	header is required.
**
**	To produce an 8-bit image with 2 wedge channels, the first
**	increasing horizontally, and the second deceasing horizontally,
**	sized to match an existing "image":
**
**		wedge -c 0,1,0,-1 -n 8 image
**
**	To produce a 16 by 16 8-bit image varying from .5 to 1.5 vertically:
**
**		mkbih -l 16 -s 16 -f | wedge -c 1,0 -n 8 |  \
**			scale -o div -v 15 | scale -o add -v .5
**
** FILES
**
** DIAGNOSTICS
**	must specify an even number of coefficients
**
**		-c must have an even number of arguments
**
**	band {band}: both coefficients cannot be 0
**
** RESTRICTIONS
**	Any optional headers, other than "lq" headers, will be copied from
**	the input image to the corresponding bands of the output image.
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
**	IPW:  mkbih
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_c = {
		'c', "line,samp coefficients",
		REAL_OPTARGS, "coef",
		REQUIRED, 2
	};

	static OPTION_T opt_n = {
		'n',
		"# bits / output pixel (default based on #lines, #samples)",
		INT_OPTARGS, "#bits",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image header",
		STR_OPERANDS, "header",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_c,
		&opt_n,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
	"linear combination of line, sample coordinates");
 /*
  * access input image file
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
	wedge(fdi,
	       got_opt(opt_n) ? int_arg(opt_n, 0) : 0,
	       n_args(opt_c), real_argp(opt_c),
	       fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/wedge/RCS/main.c,v 1.4 90/11/11 17:10:36 frew Exp $";

#endif
