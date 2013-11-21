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

#include "bitcom.h"

/*
** NAME
**	bitcom -- bitwise band combination
**
** SYNOPSIS
**	bitcom [-a] [-o] [-x]  [-m] [image]
**
** DESCRIPTION
**	bitcom writes a bitwise combination of the input image bands on the
**	standard output.
**
** OPTIONS
**	-a	output image is bitwise AND of input bands.  The output
**		image will have a non-0 value only if the matching pixel
**		from all input images have a non-0 value.
**
**	-o	output image is bitwise inclusive-OR of input bands.  The
**		output image will have a non-0 value if any of the matching
**		pixels from the input images have a non-0 value.
**
**	-x	output image is bitwise exclusive-OR of input bands.  The
**		output image will have a 0 value if all inputs are 0 or if
**		the input images match exactly.
**
**	Exactly one of -a, -o, -x must be specified.
**
**	-m	the last (highest-numbered) input band is assumed to be a
**		mask.  All non-0 pixels are set to ~0 before the bit
**		operations are performed.
**
** EXAMPLES
**	To mask a DEM "elev" such that pixels outside the drainage basin
**	"basin" are set to 0:
**
**		mux elev basin | bitcom -a -m
**
** FILES
**
** DIAGNOSTICS
**	single-band input image
**
**		The input image must have at least 2 bands
**
**	different # bits / pixel: bands 0, {band}
**
**		All input bands must have the same number of bits per
**		pixel (except the last band, if -m is specified).
**
** RESTRICTIONS
**	All input bands must have the same number of bits per pixel (if -m is
**	specified, then the last band is exempt from this restriction).
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB
**
** BUGS
**
** SEE ALSO
**	IPW: mux
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_a = {
		'a', "do bitwise AND of input bands"
	};

	static OPTION_T opt_o = {
		'o', "do bitwise OR of input bands"
	};

	static OPTION_T opt_x = {
		'x', "do bitwise XOR of input bands"
	};

	static OPTION_T opt_m = {
		'm', "last band is a mask"
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_a,
		&opt_o,
		&opt_x,
		&opt_m,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */
	void            (*op) ();	/* bitwise operation code	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "bitwise band combination");

	opt_check(1, 1, 3, &opt_a, &opt_o, &opt_x);
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
  * set operator
  */
	if (got_opt(opt_a)) {
		op = bx_and;
	}
	else if (got_opt(opt_o)) {
		op = bx_or;
	}
	else if (got_opt(opt_x)) {
		op = bx_xor;
	}
 /*
  * do it
  */
	bitcom(fdi, got_opt(opt_m), op, fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/bitcom/RCS/main.c,v 1.4 90/11/11 17:00:44 frew Exp $";

#endif
