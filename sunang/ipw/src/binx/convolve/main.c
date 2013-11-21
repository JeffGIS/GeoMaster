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

#include "convolve.h"

/*
** NAME
**	convolve -- image convolution
**
** SYNOPSIS
**	convolve [-i image] [-c coefs]
**
** DESCRIPTION
**	Convolve performs spatial convolution on an IPW image, using an
**	arbitrary kernel read from a file or from the standard input.
**
** OPTIONS
**	-i	Use {image} as the input image file (default: standard input).
**
**	-c	Use {coefs} as the kernel coefficient file (default: standard
**		input).  This file is described in the FILES section below.
**
**	At least one of -i and/or -c must be specified.
**
** EXAMPLES
**	To sharpen an image by adding it's Laplacian, reading the kernel
**	from the standard input:
**
**		convolve -i image
**		3 3
**		0 -1 0
**		-1 5 -1
**		0 -1 0
**
** FILES
**	{coefs}
**
**		This is the kernel coefficient file and is given as input
**		to the program.  It is an ASCII file formatted:
**
**			value		description
**			-----		-----------
**			1		#rows in kernel
**			2		#columns in kernel
**			3		coefficient[0][0]
**			...		...
**			#cols+2		coefficient[0][#cols-1]
**			#cols+3		coefficient[1][0]
**			...		...
**
**		Values may be separated by any sequence of blanks, tabs,
**		and newlines.  If the kernel does not sum to zero, it is
**		normalized to sum to 1.0.
**
** DIAGNOSTICS
**	{rows}x{cols} kernel is bigger than {nlines}x{nsamps} image
**
**		The kernel cannot be bigger than the image
**
**	Sorry, only single-band input images accepted
**
**		The input image may not have more than 1 band
**
**	bad kernel size: {rows}x{cols}
**
**		The kernel dimensions must be nonzero positive integers
**
**	both kernel dimensions must be odd
**
** RESTRICTIONS
**	#rows and #columns must both be odd.
**
**	There must be enough virtual memory to accommodate #rows output lines.
**
**	The normalization process can cause kernels that differ only
**	slightly to produce radically different results
**
** FUTURE DIRECTIONS
**	Relax kernel size restrictions.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/7/93	Converted to ANSI prototypes, Dana Jacobsen, ERL-C.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**	convolve.c: kmap has a morass of pointers to allocated space that has
**	no way of being cleanly freed.
**
** SEE ALSO
**	Image: pnmconvol
*/

void
DEFUN(main, (argc, argv),
	int             argc
   AND  char          **argv)
{
	static OPTION_T opt_i = {
		'i', "input image file (default: stdin)",
		STR_OPTARGS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_c = {
		'c', "input kernel coefficient file (default: stdin)",
		STR_OPTARGS, "kernel",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_i,
		&opt_c,
		0
	};

	int             fdc;		/* kernel file descriptor	 */
	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "image convolution");

	opt_check(1, 2, 2, &opt_i, &opt_c);
 /*
  * access input file(s)
  */
	if (!got_opt(opt_i)) {
		fdi = ustdin();
	}
	else {
		fdi = uropen(str_arg(opt_i, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(opt_i, 0));
		}
	}

	no_tty(fdi);

	if (!got_opt(opt_c)) {
		fdc = ustdin();
	}
	else {
		fdc = uropen(str_arg(opt_c, 0));
		if (fdc == ERROR) {
			error("can't open \"%s\"", str_arg(opt_c, 0));
		}
	}
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	convolve(fdi, fdc, fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/convolve/RCS/main.c,v 1.4 90/11/11 17:01:02 frew Exp $";

#endif
