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
**	mult -- multiply or divide bands
**
** SYNOPSIS
**	mult [-n nbits] [-r bands,...] [image]
**
** DESCRIPTION
**	mult reads a multi-band {image} (default: standard input) and
**	writes the product of its bands to the standard output.  For
**	each input sample, the corresponding output sample is:
**
**		p[0] * p[1] * ... * p[nbands-1]
**
**	where {p} is the input pixel value and {nbands} is the number
**	of input bands.
**
**	To effect division, specified bands are converted to reciprocals
**	before multiplication.  Zeroes in the denominator are replaced by
**	ones.
**
** OPTIONS
**	-n	Use {nbits} bits per output pixel (default: maximum number
**		of bits per input pixel).
**
**	-r	Use the reciprocal of the pixel value in the specified
**		{band}s.  To avoid division by 0, any 0 values in the
**		specified {band}s will be set to 1.
**
** EXAMPLES
**	To multiply two single-band images together:
**
**		mux image1 image2 | mult
**
**	To divide the first band of a 2-band image by the second band
**	(i.e. band0 / band1 ):
**
**		mult -r 1
**
** FILES
**	$TMPDIR/mult{NNNNN}
**
**		Temporary copy of the output image.
**
** DIAGNOSTICS
**	-r {band}: not that many bands
**
**		A nonexistent input band was specified with -r.
**
** RESTRICTIONS
**	Image must have more than one band.
**
**	mult creates a temporary copy of the output image since it must
**	make two passes over its output, one to determine the minima
**	and maxima in each band, and another to quantize it accordingly.
**
** FUTURE DIRECTIONS
**	There is interest in creating a program called imath that will take
**	arbitrary expressions, with special notation for operations across
**	all bands.  Mult would be done as "imath '%*' image", or alternatively
**	for a two band image could be simply "imath '%1 * %2' image".  This
**	would have the great advantage of having only one program to do the
**	job of mult, iadd, scale, and bitcom, plus the ability to do jobs
**	these programs cannot do.
**
** HISTORY
**	7/1/90	 Written by James Frew, UCSB.
**	3/24/93	 Fixed to not blow up on constant surface outputs (i.e., when
**		 multiplying by value 0 images), Dana Jacobsen, ERL-C.
**	4/26/93	 Ran through Purify.  Dana Jacobsen, ERL-C.
**	10/17/93 Multiplies are done in double precision.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  demux, mux, lincom, iadd, scale
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_r = {
		'r', "reciprocal bands",
		INT_OPTARGS, "bands",
		OPTIONAL, 1,
	};

	static OPTION_T opt_n = {
		'n', "bits per output pixel",
		INT_OPTARGS, "nbits",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_n,
		&opt_r,
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "multiply bands together");
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
	parm.nbits = (got_opt(opt_n)) ? int_arg(opt_n, 0) : 0;
	parm.nrbands = n_args(opt_r);
	parm.rband = (parm.nrbands > 0) ? int_argp(opt_r) : NULL;
 /*
  * do it
  */
	headers(FALSE);
	mult();
	headers(TRUE);
	output();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mult/RCS/main.c,v 1.2 90/11/11 17:07:08 frew Exp $";

#endif
