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

#include "hist.h"

/*
** NAME
**	hist -- compute image histogram
**
** SYNOPSIS
**	hist [-m mask] [image]
**
** DESCRIPTION
**	hist reads an IPW image (default: standard input) and computes its
**	histogram.  The histogram is written to the standard output as a
**	single-line IPW image, whose sample offsets represent the pixel values
**	in the input image, and whose pixel values are frequency counts.
**
** OPTIONS
**	-m	histogram only those pixels which are non-zero in {mask}.
**
** EXAMPLES
**	Given a DEM "elev" and a drainage basin mask "basin", the following
**	command will calculate a histogram of the elevations within the
**	basin:
**
**		hist -m basin elev
**
**	To generate a single-band histogram and convert it to text for
**	further processing by non-IPW software:
**
**		demux -b {band} | hist | rmhdr | btoa -4
**
** FILES
**
** DIAGNOSTICS
**	different size pixels: bands 0,{band}
**
**		All input bands must have the same number of bits and
**		bytes per pixel.  This is because the number of possible
**		pixel values in the input image governs the number of
**		samples in the single output line.
**
**	mask is not same size as input image
**
**		If -m is specified, then {mask} must have the same number
**		of lines and samples as the input image.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	 1/23/89	Written by James Frew, UCSB.
**	11/11/90	Output pixel size in each band is minimum necessary
**			to hold the largest output value.  James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:   btoa, grhist, histeq, rmhdr
**	Image: imhist, pgmhist, ppmhist, rlehisto, fbhist, xv
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_m = {
		'm', "mask image file",
		STR_OPERANDS, "mask",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_m,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdm;		/* mask image file descriptor	 */
	int             fdo;		/* output file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "compute image histogram");
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
  * access mask file
  */
	if (!got_opt(opt_m)) {
		fdm = ERROR;
	}
	else {
		fdm = uropen(str_arg(opt_m, 0));
		if (fdm == ERROR) {
			error("can't open \"%s\"", str_arg(opt_m, 0));
		}
	}

	no_tty(fdm);
 /*
  * access output file
  */
	fdo = ustdout();

	no_tty(fdo);
 /*
  * do histogram
  */
	hist(fdi, fdm, fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/hist/RCS/main.c,v 1.9 90/11/11 17:03:59 frew Exp $";

#endif
