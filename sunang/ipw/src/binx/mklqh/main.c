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

#include "mklqh.h"

/*
** NAME
**	mklqh -- append an LQ header to an image
**
** SYNOPSIS
**	mklqh -m in,out[,in,out,...] [-u units] [-i interp] [-b band[,...]]
**            [-f] [image]
**
** DESCRIPTION
**	mklqh constructs an IPW linear quantization header (LQH) and inserts
**	it into the headers of {image} (default: standard input).  If the
**	specified bands already have LQ headers, they are overridden, and 
**	a warning message is generated.
**
**	If the -f option is specified, then only the headers are written to
**	the standard output, and any input image data is ignored.
**
** OPTIONS
**	-m	Construct a linear mapping between the breakpoints
**		{in,out},...  At least 1 {in,out} pair must be supplied.
**		The breakpoint pairs 0,0 and 2^nbits-1,0 are assumed
**		unless explicitly overridden.
**
**	The -m option must always be specified.
**
**	-u	The floating-point pixel values are expressed in units of
**		{units} (e.g., "W m^-2", "nm") (default: none).  This field
**		is for annotation only.
**
**	-i	Use {interp} to interpolate floating-point values between
**		breakpoints (default: "linear").  Currently only "linear"
**		is supported.
**
**	-b	The "lq" header will be applied only to the specified
**		{band}s (default: all).
**
**	-f      Force header output only.  Do not attempt to copy pixel
**		data from {image} to the standard output.  Note that there
**		must still be at least an input BIH, and any input headers
**		(except superseded "lq" headers) will still be copied to
**		the standard output.
**
** EXAMPLES
**	To construct an "lq" header that will map 8-bit pixels (0 .. 255)
**	into the floating point range 0 .. 1:
**
**		mklqh -m 0,0,255,1
**
**	To construct an "lq" header that will map 12-bit pixels (0 .. 4095)
**	into the range 2762 .. 3417:
**
**		mklqh -m 0,2762,4095,3417
**
** FILES
**
** DIAGNOSTICS
**	bands {band1} and {band2} have different # bits / pixel
**
**		All bands to which the "lq" header is applied must have
**		the same number of bits per pixel.
**
**	must specify pixel,fpixel pairs for -m
**
**		-m must have an even number of arguments.
**
**	no band {band}
**
**		A nonexistent input band was specified with -b.
**
**	band {band}: replacing previous LQ header
**
**		Band {band} already had a "lq" header, which was replaced
**		by the newly-created one.
**
** RESTRICTIONS
**	There are not yet any standard identifiers for {units}.
**
**	The default breakpoints can lead to unexpected quantization mappings
**	or errors from mklqh.  For example, the mapping
**
**		mklqh -m 0,0,127,1
**
**	succeeds for 7-bit pixels, but for 8-bit pixels there would be an
**	additional implicit breakpoint at 255,0, which would make the mapping
**	non-monotonic.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**	The old manual page gave the example:
**
**		mklqh -m 255,1
**
**	and noted that the breakpoint 0,0 was assumed.  This results in an
**	error and no LQ header produced.  This should be fixed.
**
** SEE ALSO
**	IPW:  gradient, hor1d, lincom, lqhx, mkbih, mstats, mult, prhdr,
**	      rmhdr, shade, viewcalc, wedge
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_m = {
		'm', "pixel,fpixel breakpoint pairs",
		REAL_OPTARGS, "val",
		REQUIRED, 2
	};

	static OPTION_T opt_u = {
		'u', "fpixel units of measure",
		STR_OPTARGS, "units",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_i = {
		'i', "interpolation function (default: \"linear\")",
		STR_OPTARGS, "interp",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_b = {
		'b', "band #s to receive map (default: all)",
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
		&opt_m,
		&opt_u,
		&opt_i,
		&opt_b,
		&opt_f,
		&operands,
		0
	};

	char           *interp;		/* interpolation function name	 */
	char           *units;		/* fpixel units of measurement	 */
	fpixel_t       *fbkpt;		/* floating-point breakpoints	 */
	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */
	int             i;		/* loop counter			 */
	int             j;		/* loop counter			 */
	int             nbkpts;		/* # breakpoint pairs		 */
	pixel_t        *ibkpt;		/* integer breakpoints		 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
		 "construct a linear quantization (LQ) header");
 /*
  * crack options
  */
	if (n_args(opt_m) & 1) {
		error("must specify pixel,fpixel pairs for -m");
	}

	nbkpts = n_args(opt_m) / 2;
 /* NOSTRICT */
	ibkpt = (pixel_t *) ecalloc(nbkpts, sizeof(pixel_t));
 /* NOSTRICT */
	fbkpt = (fpixel_t *) ecalloc(nbkpts, sizeof(fpixel_t));

	if (ibkpt == NULL || fbkpt == NULL) {
		error("can't allocate breakpoint arrays");
	}

	j = 0;
	for (i = 0; i < nbkpts; ++i) {
		ibkpt[i] = real_arg(opt_m, j);
		++j;
		fbkpt[i] = real_arg(opt_m, j);
		++j;
	}

	units = got_opt(opt_u) ? str_arg(opt_u, 0) : (char *) NULL;
	interp = got_opt(opt_i) ? str_arg(opt_i, 0) : (char *) NULL;
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
	mklqh(fdi, nbkpts, ibkpt, fbkpt, units, interp, n_args(opt_b),
	      int_argp(opt_b), got_opt(opt_f), fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mklqh/RCS/main.c,v 1.6 90/11/11 17:06:17 frew Exp $";

#endif
