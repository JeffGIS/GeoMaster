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

#include "demux.h"

/*
** NAME
**	demux -- demultiplex (extract bands from) IPW image
**
** SYNOPSIS
**	demux -b band[,...] [image]
**
** DESCRIPTION
**	demux extracts the specified bands from image, or from the standard
**	input.  The image consisting of only the specified bands is written
**	to the standard output.
**
** OPTIONS
**	-b	extract the specified bands.  The bands will appear in the
**		output image in the order specified.
**
** EXAMPLES
**	To reverse the order of a 2 band image:
**
**		demux -b 1,0 image
**
**	To create a 3 band image consisting of the first 3 bands of "image":
**
**		demux -b 0,1,2 image
**
** FILES
**
** DIAGNOSTICS
**	{band}: bad input band number
**
**		The input image does not contain the specified band.
**
**	"{header}" header, band {band}: no such band
**
**		The specified {header} in the input image pertains to a
**		nonexistent band (i.e., the input image is corrupted).
**
** RESTRICTIONS
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
**	IPW:  mux
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_b = {
		'b', "bands to extract",
		INT_OPTARGS, "band",
		REQUIRED,
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_b,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "extract image bands");
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
	demux(fdi, n_args(opt_b), int_argp(opt_b), fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/demux/RCS/main.c,v 1.3 90/11/11 17:01:55 frew Exp $";

#endif
