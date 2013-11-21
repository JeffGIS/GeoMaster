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

#include "mkwinh.h"

/*
** NAME
**	mkwinh -- add a window header to an image
**
** SYNOPSIS
**	mkwinh [-b line,sample] [-d dline,dsamp] [-f] [image]
**
** DESCRIPTION
**	mkwinh constructs an IPW window header (WINH) and inserts it into
**	the headers of {image} (default: standard input).  If the specified
**	bands already have WIN headers, they are overridden, and a warning
**	message is generated.
**
**	If the -f option is specified, then only the headers are written to
**	the standard output, and any input image data is ignored.
**
** OPTIONS
**	-b	{line},{sample} are the window coordinates of line 0 and
**		sample 0 in the input image (default: 0,0).
**
**	-d	{dline},{dsamp} are the window line and sample spacings in
**		the input image (default: 1,1).
**
**	-f	Force header output only.  Do not attempt to copy pixel
**		data from {image} to the standard output.  Note that there
**		must still be at least an input BIH, and any input headers
**		(except superseded "win" headers) will still be copied to
**		the standard output.
**
** EXAMPLES
**	To add a window header indicating that the window coordinates of
**	the image origin are line 7, sample 5:
**
**		mkwinh -b 7,5
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	The "win" header will always be applied to all bands of the image.
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
**	IPW:  flip, mkbih, prhdr, rmhdr, transpose, window, zoom
*/

void
main(argc, argv)
	int             argc;		/* # arguments			 */
	char          **argv;		/* ->'s to arguments		 */
{
	static OPTION_T opt_b = {
		'b', "begin line,sample",
		REAL_OPTARGS, "coord",
		OPTIONAL, 2, 2
	};

	static OPTION_T opt_d = {
		'd', "line,sample increment",
		REAL_OPTARGS, "incr",
		OPTIONAL, 2, 2
	};

	static OPTION_T opt_f = {
		'f', "force header output; ignore any input image",
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_b,
		&opt_d,
		&opt_f,
		&operands,
		0
	};

	double          bline;		/* begin line #			 */
	double          bsamp;		/* begin sample #		 */
	double          dline;		/* line increment 		 */
	double          dsamp;		/* sample increment		 */
	int             fdi;		/* intput file descriptor	 */
	int             fdo;		/* output file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "make an IPW window header");
 /*
  * process options
  */
	if (got_opt(opt_b)) {
		bline = real_arg(opt_b, 0);
		bsamp = real_arg(opt_b, 1);
	}
	else {
		bline = DFLT_BLINE;
		bsamp = DFLT_BSAMP;
	}

	if (got_opt(opt_d)) {
		dline = real_arg(opt_d, 0);
		dsamp = real_arg(opt_d, 1);
	}
	else {
		dline = DFLT_DLINE;
		dsamp = DFLT_DSAMP;
	}
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
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	mkwinh(fdi, bline, bsamp, dline, dsamp, got_opt(opt_f), fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mkwinh/RCS/main.c,v 1.10 90/11/11 17:06:38 frew Exp $";

#endif
