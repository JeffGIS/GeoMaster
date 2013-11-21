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
**	zoom -- enlarge/reduce an IPW image by replication/subsampling
**
** SYNOPSIS
**	zoom [-l lfactor] [-s sfactor] [-f] [image]
**
** DESCRIPTION
**	zoom reads {image} (default: standard input) and writes a copy to
**	the standard output, with lines and/or samples either replicated
**	or skipped.
**
**	Positive factors specify enlargement, negative factors specify
**	reduction.
**
**	When skipping lines or samples, counting begins at (0,0); i.e., the
**	first sample in the input image is always present in the output image.
**
** OPTIONS
**	-l	Replicate each line {lfactor} times.  If {lfactor} is
**		negative, then select every {lfactor}'th line, beginning
**		with line 0.
**
**	-s	Replicate each sample {sfactor} times.  If {sfactor} is
**		negative, then select every {sfactor}'th sample,
**		beginning with sample 0, from each selected line.
**
**	-f	When replicating, force the edges of the output image
**		to coincide exactly with those of the input image.
**		This only makes sense on images with geodetic headers.
**		If the -f option is not used, the output image will be
**		offset in geodetic space.   
**
**		NOTE: when using the -f option, the zoom procedure is not 
**		reversible.  For example, 
**
**			zoom -f -l 10 -s 10 foo | zoom -l -10 -s -10
**
**		will output pixel values identical to those of foo, but
**		the geodetic extent of the output image will be 
**		different from that of foo.
**
**	At least one of -l and/or -s must be specified.
**
** EXAMPLES
**	To zoom an image by a factor of 2 in both directions:
**
**		zoom -s 2 -l 2
**
**	To histogram every 10th pixel of a large image:
**
**		zoom -s -10 -l -10 | hist
**
**	Zooming by non-integer factors in accomplished by expressing the
**	desired zoom factor as a fraction, and then using the pipeline:
**
**		zoom -{dir} {numerator} | zoom -{dir} -{denominator}
**
**	For example, to shrink an image by 3/4 horizontally (perhaps to
**	accommodate a display with a 4:3 aspect ratio):
**
**		zoom -s 3 | zoom -s 4
**
**	The fractional zoom pipeline should be specified in the order
**	shown (replication BEFORE subsampling).  Reversing the order
**	(subsampling before replication) may be faster, but results in
**	much greater loss of spatial resolution.
**
** FILES
**
** DIAGNOSTICS
**	line zoom factor must be non-zero
**	sample zoom factor must be non-zero
**
**		Zoom factors must be non-zero integers.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	8/9/94  Added -f option; Rusty Dodson, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:   window, resamp
**	Image: pbmscale, rlezoom, xv
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_l = {
		'l', "line zoom factor",
		INT_OPTARGS, "factor",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_s = {
		's', "sample zoom factor",
		INT_OPTARGS, "factor",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_f = {
		'f', "force coincident images",
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_l,
		&opt_s,
		&opt_f,
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
		 "zoom image by pixel replication or subsampling");
 /*
  * check options
  */
	opt_check(1, 2, 2, &opt_l, &opt_s);

	if (!got_opt(opt_l)) {
		parm.skip_lines = 1;
		parm.dup_lines = 1;
	}
	else {
		int             z_lines;/* line zoom factor	 */

		z_lines = int_arg(opt_l, 0);
		if (z_lines == 0) {
			error("line zoom factor must be non-zero");
		}

		parm.skip_lines = z_lines < 0 ? -z_lines : 1;
		parm.dup_lines = z_lines < 0 ? 1 : z_lines;
	}

	if (!got_opt(opt_s)) {
		parm.skip_samps = 1;
		parm.dup_samps = 1;
	}
	else {
		int             z_samps;/* sample zoom factor	 */

		z_samps = int_arg(opt_s, 0);
		if (z_samps == 0) {
			error("sample zoom factor must be non-zero");
		}

		parm.skip_samps = z_samps < 0 ? -z_samps : 1;
		parm.dup_samps = z_samps < 0 ? 1 : z_samps;
	}
	
	if (got_opt(opt_f)) {
		parm.force = 1;
		if (parm.skip_samps != 1 || parm.skip_lines != 1) {
		    warn("-f option not valid when subsampling.  Ignored.");
		    parm.force = 0;
		}
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
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * do it
  */
	headers();
	zoom();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/zoom/RCS/main.c,v 1.2 90/11/11 17:11:02 frew Exp $";

#endif
