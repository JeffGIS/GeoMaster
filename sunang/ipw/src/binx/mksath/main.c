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

#include "mksath.h"

/*
** NAME
**	mksath -- add a satellite header to an image
**
** SYNOPSIS
**	mksath [-p platform] [-s sensor] [-l location] [-d date] [-t time]
**	       [-b band[,...]] [-f] [image]
**
** DESCRIPTION
**	mksath constructs an IPW satellite header (SATH) and inserts in into
**	the headers of {image} (default: standard input).  If the specified
**	bands already have SAT headers, they are overridden, and a warning
**	message is generated.
**
**	If the -f option is specified, then only the headers are written to
**	the standard output, and any input image data is ignored.
**
** OPTIONS
**
**	-s	The image data were acquired by {sensor} (e.g., "TM", "SPOT",
**		"AVIRIS", etc.).
**
**	-p	The {sensor} was mounted on {platform} (e.g., "Landsat-5",
**		"ER-2", "C-130", etc.).
**
**	-l	The image data were acquired at or over {location}.  This
**		should not be a geodetic specification (use mkgeoh for that),
**		but a sensor-, platform-, or project-specific identifier
**		(e.g., experimental site name, Landsat path/row, etc.).
**
**	-d	The image data were acquired on {date}.  The standard
**		format for this is {YYYYMMDD}.
**
**	-t	The image data were acquired at {time}.  The standard
**		format for this is {hhmmss.sss...}.
**
**	At least one of -s, -p, -l, -d, and/or -t must be specified.
**
**	-b	The "sat" header will be applied only to the specified
**		{band}s (default: all).
**
**	-f	Force header output only.  Do not attempt to copy pixel
**		data from {image} to the standard output.  Note that there
**		must still be at least an input BIH, and any input headers
**		(except superseded "sat" headers) will still be copied to
**		the standard output.
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**	bad band number: {band}
**
**		A nonexistent input band was specified with -b.
**
**	band {band}: replacing previous SAT header
**
**		Band {band} already had a "sat" headers, which was replaced
**		by the newly-created one.
**
** RESTRICTIONS
**	There are not yet any standard identifiers for {sensor}, {platform},
**	or {location}.  The {date} and {time} arguments are recommended but
**	not enforced
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  mkbih, prhdr, rmhdr, tmpt
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_p = {
		'p', "platform",
		STR_OPTARGS, "platform",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_s = {
		's', "sensor",
		STR_OPTARGS, "sensor",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_l = {
		'l', "location",
		STR_OPTARGS, "location",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_d = {
		'd', "date (YYYYMMDD)",
		STR_OPTARGS, "date",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_t = {
		't', "time (hhmmss.sss...)",
		STR_OPTARGS, "time",
		OPTIONAL, 1, 1
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
		&opt_p,
		&opt_s,
		&opt_l,
		&opt_d,
		&opt_t,
		&opt_b,
		&opt_f,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	*/
	int             fdo;		/* output image file descriptor	*/
	char           *gmdate;		/* date (YYYYMMDD)		*/
	char           *gmtime;		/* time (hhmmss.sss)		*/
	char           *location;	/* location			*/
	char           *platform;	/* platform			*/
	char           *sensor;		/* sensor			*/

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "make satellite header");

	opt_check(1, 5, 5, &opt_p, &opt_s, &opt_l, &opt_d, &opt_t);
 /*
  * crack options
  */
	platform = got_opt(opt_p) ? str_arg(opt_p, 0) : (char *) NULL;
	sensor = got_opt(opt_s) ? str_arg(opt_s, 0) : (char *) NULL;
	location = got_opt(opt_l) ? str_arg(opt_l, 0) : (char *) NULL;
	gmdate = got_opt(opt_d) ? str_arg(opt_d, 0) : (char *) NULL;
	gmtime = got_opt(opt_t) ? str_arg(opt_t, 0) : (char *) NULL;
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
	mksath(fdi, platform, sensor, location, gmdate, gmtime,
	        n_args(opt_b), int_argp(opt_b), got_opt(opt_f), fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mksath/RCS/main.c,v 1.3 90/11/11 17:06:24 frew Exp $";

#endif
