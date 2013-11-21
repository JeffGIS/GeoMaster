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

#include "transpose.h"

/*
** NAME
**	transpose -- transpose IPW image
**
** SYNOPSIS
**	transpose [ image ]
**
** DESCRIPTION
**	transpose reads {image} (default: standard input) and writes its
**	transpose to the standard output.
**
** OPTIONS
**
** EXAMPLES
**	To rotate an image 90 degrees clockwise:
**
**		flip -s | transpose
**
** FILES
**
** DIAGNOSTICS
**	output image won't fit in memory
**
**		The entire image must fit into virtual memory.
**
** RESTRICTIONS
**	The entire image must fit into (virtual) memory.
**
**	If the output image does not have the canonical IPW line (top to
**	bottom) and sample (left to right) order, then it will be given
**	an orientation ("or") header describing the non-standard ordering.
**
**	Geodetic ("geo") headers, window ("win") headers, and orientation
**	("or") headers will be modified if they are present.
**
** FUTURE DIRECTIONS
**	It should be possible to transpose images that are too large to fit
**	in memory, on non-virtual-memory machines.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  flip, skew, rotate
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "transpose an image");
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
	transpose(fdi, fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/transpose/RCS/main.c,v 1.2 90/11/11 17:10:01 frew Exp $";

#endif
