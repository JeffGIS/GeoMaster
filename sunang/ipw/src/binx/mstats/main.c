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
**	mstats -- image multivariate statistics
**
** SYNOPSIS
**	mstats [-c classes] image
**
** DESCRIPTION
**	mstats computes the band-wise mean vector and variance-covariance
**	matrix for the input {image} (default: standard input).  The
**	statistics are written in text to the standard output, in the
**	following format:
**
**		item				description
**		-------------------------	--------------------------
**		#<stats>			mstats output identifier
**
**		number_of_bands			# bands in the input image
**
**		mean_0 mean_1 ...		mean pixel value vector
**
**		variance_0			variance-covariance matrix
**		covariance_1,0 variance_1
**		...
**
**		* class				class number
**
**		number_of_pixels		number of samples in {class}
**
** OPTIONS
**	-c	Read classes from {classes} image (default: standard input).
**		This must be a single-band image with the same dimensions
**		as the input image.  A separate set of statistics is
**		accumulated and output for each unique pixel value in the
**		class image.  The pixel at (line,sample) in the input image
**		is assigned to the class of the pixel in the {classes} image.
**
** EXAMPLES
**	If "image" is a multiband satellite image, and "basin" is a
**	registered mask of a drainage basin, then
**
**		mstats -c basin image
**
**	will compute the separate multivariate statistics for areas inside
**	and outside the basin.
**
** FILES
**
** DIAGNOSTICS
**	input and class image must have same # lines
**	input and class image must have same # samples
**
**		The input and class images must have the same dimensions.
**
**	class image must have only 1 band
**
** RESTRICTIONS
**	mstats will always use any linear quantization ("lq") headers in
**	the input image to transform the input values.  If statistics for
**	the raw pixel values are desired, then any "lq" headers must be
**	removed from the input image before is is passed to mstats:
**
**		rmhdr -d lq | mstats
**
**	Computing variances involves accumulating a sum of squares of all
**	input values.  the larger the input image, the more likely that
**	the output variances and covariances will contain rounding errors.
**
**	Means and (co-)variances will be printed with the maximum precision
**	supported by the hose architecture.  The low-order digits should
**	probably be ignored.
**
** FUTURE DIRECTIONS
**	Add flag to suppress conversion of pixels to floating-point values.
**
**	There is considerable overlap between mstats and hist;  they may
**	be integrated someday.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  demux, edimg, hist, mux, rmhdr
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_c = {
		'c', "class image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_c,
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "image multivariate statistics");
 /*
  * process options
  */
	if (!got_opt(opt_c)) {
		parm.c_fd = ERROR;
	}
	else {
		parm.c_fd = uropen(str_arg(opt_c, 0));
		if (parm.c_fd == ERROR) {
			error("can't open \"%s\"", str_arg(operands, 0));
		}

		no_tty(parm.c_fd);
	}

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
  * do it
  */
	headers();
	mstats();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mstats/RCS/main.c,v 1.2 90/11/11 17:06:54 frew Exp $";

#endif
