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

#include "rmhdr.h"

/*
** NAME
**	rmhdr -- strip IPW headers from input image
**
** SYNOPSIS
**	rmhdr [-d header[,...]] [image]
**
** DESCRIPTION
**	rmhdr reads an IPW image {image} (default: standard input) and
**	writes only the image data to the standard output.
**
** OPTIONS
**	-d	Delete only the specified {header}s (default: all).
**		{header} should be the name of the header exactly as
**		it appears in the header itself ("lq" for a linear
**		quantization header).  Nonspecified headers are copied
**		to the standard output.
**
** EXAMPLES
**	For a single-band image with {nbytes} bytes per pixel,
**
**		rmhdr | btoa -{nbytes}
**
**	would be equivalent to
**
**		primg -r -a
**
**	To delete the "lq" header from an image before running mstats
**	(so the statistics will be computed for the quantized pixel values):
**
**		rmhdr -d lq | mstats
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	The default output of rmhdr is not a valid IPW image, since it has
**	no BIH (nor any other header).  However, if -d is specified, then
**	rmhdr's output is a value IPW image, unless "basic_image" or
**	"basic_image_i"are explicitly specified as {header} arguments.
**
**	For the header names that will be recognized by the -d option,
**	see the {header}H_HNAME macro definition in $IPW/h/{header}h.h.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  btoa, mk*h, prhdr
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_d = {
		'd', "header to delete (default: all)",
		STR_OPTARGS, "header",
		OPTIONAL
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_d,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "strip IPW image headers");
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
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	rmhdr(fdi, n_args(opt_d), str_argp(opt_d), fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/rmhdr/RCS/main.c,v 1.11 90/11/11 17:08:27 frew Exp $";

#endif
