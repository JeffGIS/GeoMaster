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
#include "bih.h"
#include "getargs.h"
#include "dither.h"

/*
** NAME
**	dither -- create bilevel image using ordered dithering
**
** SYNOPSIS
**	dither [-r rank] [image]
**
** DESCRIPTION
**	Dither converts the input image into a bilevel (1 bit per pixel)
**	output image, using ordered dithering.  "Black" output pixels are
**	assigned the value 0; "white" output pixels are assigned the value 1.
**
** OPTIONS
**	-r	Use a dither matrix of rank {rank} (default 4).  Possible
**		values are 4, 8, or 16, for simulating 16, 64, or 256 gray
**		levels, respectively.
**
** EXAMPLES
**	To display "image" on a monochrome Sun workstation under SunView:
**
**		dither image | ipw2sun | rastool
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	Before using dither, make sure that the output device (e.g.,
**	Postscript printer) or display software (e.g. xv, xloadimage)
**	does not already incorporate a better halftoning algorithm.
**
** FUTURE DIRECTIONS
**	Other dithering algorithms may be supported.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Converted to ANSI C.  Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:   rastool, ipw2sun
**	Image: ppmdither, rledither, to8, xv, xloadimage
**
**	David F. Rogers, "Procedural Elements for Computer Graphics",
**		McGraw-Hill, 1985, p. 107.
*/

void
main(argc, argv)
	int             argc;
	char           **argv;
{
	static OPTION_T opt_r = {
		'r', "rank of dither matrix (default: 4)",
		INT_OPTARGS, "rank",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_r,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output file descriptor	 */
	int             rank;		/* dither matrix rank		 */
 /*
  * begin
  */
	ipwenter(argc, argv, optv, "convert to 1-bit pixels by dithering");
 /*
  * collect options
  */
	if (got_opt(opt_r)) {
		rank = int_arg(opt_r, 0);
		switch (rank) {

		case 4:
		case 8:
		case 16:
			break;

		default:
			error("%d: illegal dither matrix rank", rank);
		}
	}
	else {
		rank = DFLT_RANK;
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

	no_tty(fdi);
 /*
  * access output file
  */
	fdo = ustdout();

	no_tty(fdo);
 /*
  * do it
  */
	dither(fdi, rank, fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/dither/RCS/main.c,v 1.3 90/11/11 17:02:09 frew Exp $";

#endif
