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
**	lincom -- linear combination of bands
**
** SYNOPSIS
**	lincom [-c coef,...] [-n nbits] [image]
**
** DESCRIPTION
**	lincom reads a multi-band image (default: standard input) and
**	writes a linear combination of its bands to the standard output.
**	For each input sample, the corresponding output sample is:
**
**		p[0] * k[0] + ... + p[nbands-1] * k[nbands-1]
**
**	where p is the input pixel value, k is a user-specified
**	coefficient, and nbands is the number of input bands.
**
** OPTIONS
**	-c	per-band coefficients (default: 1/nbands).  If only one
**		coefficient is specified, the same coefficient is used for
**		all bands.  Otherwise, the number of coefficients must be a
**		multiple of nbands.  If the number of coefficients is larger
**		than nbands, each successive group of coefficients will be
**		used to create a new output band.
**
**	-n	Use {nbits} bits per output pixel (default: maximum number
**		of bits per input pixel).
**
** EXAMPLES
**	To create an average of all the input bands:
**
**		lincom
**
**	To subtract band 1 from band 0 of a 2-band image:
**
**		lincom -c 1,-1
**
**	To create a 2-band output image from a 2-band input image, in
**	which the first output band is a sum and the second output band
**	is a difference:
**
**		lincom -c .5,.5,1,-1
**
** FILES
**	$TMPDIR/lin{NNNNN}
**
**		Temporary copy of the output image.
**
** DIAGNOSTICS
**	# coefficients specified = {ncoef}; must be zero, 1, or modulo nbands
**
**		The number of coefficients specified must be 0, 1, or a
**		multiple of the number of bands in the input image.
**
** RESTRICTIONS
**	lincom creates a temporary copy of the output image since it must
**	make two passes over its output, one to determine the minima and
**	maxima in each band, and another to quantize it accordingly.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90   Written by James Frew, UCSB.
**	8/5/92   Use obands instead of nbands in gethdrs.  Kelly Longley, ERL-C.
**	4/19/93  Resolve minval == maxval problem.  Dana Jacobsen, ERL-C.
**	4/26/93  Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  bitcom, mult
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_c = {
		'c', "band coefficients (default 1/nbands)",
		REAL_OPTARGS, "coef",
		OPTIONAL, 1
	};

	static OPTION_T opt_n = {
		'n', "# bits / output pixel (default from input image)",
		INT_OPTARGS, "#bits",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_c,
		&opt_n,
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "linear combination of bands");
 /*
  * access input file(s)
  */
	if (!got_opt(operands)) {
		parm.i_fd = ustdin();
	}
	else {
		parm.i_fd = uropen(str_arg(operands, 0));
		if (parm.i_fd == ERROR) {
			error("can't open \"%s\"", str_arg(operands, 0));
		}
	}

	no_tty(parm.i_fd);
 /*
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * process arguments
  */
	parm.nbits = got_opt(opt_n) ? int_arg(opt_n, 0) : 0;
	parm.ncoef = n_args(opt_c);
	parm.coef = (parm.ncoef > 0) ? real_argp(opt_c) : NULL;
 /*
  * do it
  */
	headers(FALSE);
	coeffs();
	lincom();
	headers(TRUE);
	output();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/lincom/RCS/main.c,v 1.2 90/11/11 17:05:25 frew Exp $";

#endif
