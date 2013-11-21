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

#include "mkgeoh.h"

/*
** NAME
**	mkgeoh -- add a geodetic header to an image
**
** SYNOPSIS
**	mkgeoh  -o u,v  -d du,dv  -u units  -c csys
**	       [-b band[,...]]  [-f]  [image]
**
** DESCRIPTION
**	mkgeoh constructs an IPW geodetic header (GEOH) and inserts it into
**	the headers of {image} (default: standard input).  If the specified
**	bands already have GEO headers, they are overridden, and a warning
**	message is generated.
**
**	If the -f option is specified, then only the headers are written to
**	the standard output, and any input image data is ignored.
**
** OPTIONS
**	-o	The coordinates of image line 0 and sample 0 in {csys} are
**		{u} and {v}, respectively.
**
**	-d	The distances between adjacent image lines and samples in
**		{csys} are {du} and {dv}, respectively.
**
**	-u	{u}, {v}, {du}, and {dv} are specified in {units} (e.g.,
**		"meters", "km").  No default value.
**
**	-c	The geodetic coordinate system identifier is {csys} (e.g.,
**		"UTM", "Lambert").  See the manual for mkproj for a list
**		of standard names for coordinate systems.  No default value.
**
**	-o, -d, -u, and -c must always be specified.
**
**	-b	The "geo" header will be applied only to the specified
**		{band}s (default: all).
**
**	-f	Force header output only.  Do not attempt to copy pixel
**		data from {image} to the standard output.  Note that there
**		must still be at least an input BIH, and any input headers
**		(except superseded "geo" headers) will still be copied to
**		the standard output.
**		
**
** EXAMPLES
**	To create a "geo" header appropriate for a 5-meter grid located
**	in the Sierra Nevada, California:
**
**		mkgeoh -c UTM -u meters -o 4051800,349350 -d -5,5
**
**	Note the negative line spacing:  UTM northings run in the opposite
**	direction from IPW line numbers.
**
** FILES
**
** DIAGNOSTICS
**	bad band number: {band}
**
**		A nonexistent input band was specified with -b.
**
**	band {band}: replacing previous GEO header
**
**		Band {band} already had a "geo" header, which was replaced
**		by the newly-created one.
**
** RESTRICTIONS
**	There are not yet any standard identifiers for {csys} and {units},
**	although some IPW programs such as gradient assume that any units
**	beginning with "m" are meters.  There is work underway to develop
**	standard {csys} names.  See mkproj.
**
** FUTURE DIRECTIONS
**	Use new routines to identify {csys} names.  This work is underway.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  mkproj, reproj, flip, gradient, hor1d, horizon, mkbih, prhdr,
**	       rmhdr, transpose, viewcalc, window, zoom
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_o = {
		'o', "u,v coordinates of image origin",
		REAL_OPTARGS, "coord",
		REQUIRED, 2, 2
	};

	static OPTION_T opt_d = {
		'd', "u,v increment per line,sample",
		REAL_OPTARGS, "incr",
		REQUIRED, 2, 2
	};

	static OPTION_T opt_u = {
		'u', "u,v units of measure",
		STR_OPTARGS, "units",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_c = {
		'c', "u,v coordinate system identifier",
		STR_OPTARGS, "csys",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_b = {
		'b', "band #s to receive header (default: all)",
		INT_OPTARGS, "band",
		OPTIONAL, 1
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
		&opt_o,
		&opt_d,
		&opt_u,
		&opt_c,
		&opt_b,
		&opt_f,
		&operands,
		0
	};

	char           *csys;		/* u,v coord sys identifier	 */
	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */
	char           *units;		/* u,v units of measurement	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "add a geodetic header to an image");
 /*
  * crack options
  */
	units = got_opt(opt_u) ? str_arg(opt_u, 0) : (char *) NULL;
	csys = got_opt(opt_c) ? str_arg(opt_c, 0) : (char *) NULL;
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
	mkgeoh(fdi, real_argp(opt_o), real_argp(opt_d), units, csys,
	       n_args(opt_b), int_argp(opt_b), got_opt(opt_f), fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mkgeoh/RCS/main.c,v 1.4 90/11/11 17:06:09 frew Exp $";

#endif
