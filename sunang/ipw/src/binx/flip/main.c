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
#include "pgm.h"

/*
** NAME
**	flip -- flip IPW image
**
** SYNOPSIS
**	flip [-l] [-s] [ image ]
**
** DESCRIPTION
**	This program flips an IPW image.  Either the lines themselves
**	can be flipped, or the samples within the lines, or both.
**
** OPTIONS
**	-l	Reverse order of image lines
**
**	-s	Reverse order of image samples in each line
**
**	At least one of -l and/or -s must be specified.
**
** EXAMPLES
**	An image may be rotated in multiples of 90 degrees as follows:
**
**		flip -s | transpose		#  90 degrees clockwise
**		flip -l -s			# 180 degrees
**		flip -l | transpose		# 270 degrees
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	If -l is specified then the entire image must fit into memory.
**
**	If the output image does not have the canonical IPW line (top to
**	bottom) and sample (left to right) order, then it will be given
**	an orientation ("or") header describing the non-standard ordering.
**
**	Geodetic ("geo") headers, window ("win") headers, and orientation
**	("or") headers will be modified if they are present.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**	4/30/93	Cast void pointer to character pointer.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:   transpose, skew
**	Image: pnmflip, pnmrotate, rleflip, fant, imflip, imrotate
*/

extern void     flip();

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_l = {
		'l', "flip order of lines in file"
	};

	static OPTION_T opt_s = {
		's', "flip order of samples within lines"
	};

	static OPTION_T operand = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_l,
		&opt_s,
		&operand,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "flip IPW image");
 /*
  * access input file
  */
	if (!got_opt(operand)) {
		parm.i_fd = ustdin();
	}
	else {
		parm.i_fd = uropen(str_arg(operand, 0));
		if (parm.i_fd == ERROR) {
			error("can't open \"%s\"", str_arg(operand, 0));
		}
	}

	no_tty(parm.i_fd);
 /*
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * collect options
  */
	parm.lines = got_opt(opt_l);
	parm.samps = got_opt(opt_s);

	if (!parm.lines && !parm.samps) {
		error("must specify -l or -s (or both)");
	}
 /*
  * do it
  */
	headers();
	flip();
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/flip/RCS/main.c,v 1.6 90/11/11 17:02:42 frew Exp $";

#endif
