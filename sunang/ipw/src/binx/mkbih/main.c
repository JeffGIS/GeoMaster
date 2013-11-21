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

#include "bih.h"
#include "getargs.h"

#include "mkbih.h"

/*
** NAME
**	mkbih -- make a standalone IPW basic image header
**
** SYNOPSIS
**	mkbih -l nlines -s nsamps [-a annot[,...]] [-b nbands]
**			[-y nbytes[,...]] [-i nbits[,...]] [-f] [data]
**
** DESCRIPTION
**	mkbih creates an IPW basic image header (BIH) and writes it to
**	the standard output.  The contents of {data} (default: standard
**	input) are then copied to the standard output.  mkbih therefore
**	allows an IPW header to be prepended to image data from a
**	non-IPW source.
**
** OPTIONS
**	-l	The BIH will indicate {nlines} lines per image.
**
**	-s	The BIH will indicate {nsamps} samples per image line.
**
**	-l and -s must always be specified.
**
**	-b	The BIH will indicate {nbands} bands per image sample
**		(default: 1).
**
**	-y	The BIH will indicate {nbytes} bytes per pixel (default:
**		1, or the minimum necessary to accommodate the specified
**		{nbits}).
**
**	-i	The BIH will indicate {nbits} bits per pixel (default:
**		8, or all bits in the specified {nbytes}).
**
**	-y and -i may have either 1 or {nbands} arguments.  If there is
**	1 argument and {nbands} is greater than 1, then the argument
**	applies to all bands.
**
**	-a	The BIH will indicate {annot} as the annotation (commentary)
**		for each band (default: no annotation).  If {nbands} is
**		greater than 1 and -a is specified, then it must have
**		exactly {nbands} arguments.
**
**	-f	Force header output only.  Do not attempt to copy {data}
**		to the standard output.  This allows creation of a
**		standalone header to which image data may later be
**		appended, or which may be passed as control information to
**		another IPW program such as lqhx.
**
** EXAMPLES
**	To make a stand-alone header for a single-band 512x512x8-bit image:
**
**		mkbih -l 512 -s 512 -f >header
**
**	To prepend an IPW BIH to a 512 line by 512 sample single-band
**	image with 8-bit pixels:
**
**		mkbih -l 512 -s 512 image
**
**	To append a header to a raw image being read by dd:
**
**		dd ... | mkbih -l 512 -s 512 >image
**
**	To prepend an IPW BIH to a 6000 line by 7000 sample single-band
**	8-bit image being read directly from tape:
**
**		mkbih -l 6000 -s 7000 -f  >image
**		dd bs=7000  <{tape-device}  >>image
**
**	Note the use of ">>" to append the image data to the file
**	containing the standalone header.
**
** FILES
**
** DIAGNOSTICS
**	{nbits} won't fit in {nbytes} bytes
**
**		Only the following combinations of {nbytes} and {nbits}
**		are allowed:
**
**			nbytes		  nbits
**			1,2,4		 1 ..  8
**			2,4		 9 .. 16
**			4		17 .. 32
**
**	input file not allowed with "-f" option
**
**		Since the -f option produces an "orphan" BIH header, there
**		is no data copied from the input file.
**
**	may specify either 1 or {nbands} values for nbytes
**	may specify either 1 or {nbands} values for nbits
**
**		The number of bytes and bits specified by the -y and -i
**		options must be either 1 or the same as the number of bands.
**
**	{nbands} bands requires {nbands} annotation strings
**
**		The number of annotation strings specified by the -a option
**		must be the same as the number of bands.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/5/91	Don't repeat band 0 header if multiple annotation strings
**		are given.  Kelly Longley, ERL-C.
**
** BUGS
**	The annotation string may not contain any commas (they will be
**	interpreted by mkbih as option-argument separators, even if quoted).
**
**	Using -i for the number of output bits is non-standard.  Use -n.
**
** SEE ALSO
**	IPW:  lqhx, prhdr, rmhdr, mkgeoh, mklqh, mksath, mk*h
**	UNIX: dd
*/

void
main(argc, argv)
	int             argc;		/* # arguments			 */
	char          **argv;		/* ->'s to arguments		 */
{
	static OPTION_T opt_l = {
		'l', "# lines / image",
		INT_OPTARGS, "#lines",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_s = {
		's', "# samples / line",
		INT_OPTARGS, "#samps",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_a = {
		'a', "annotation string(s)",
		STR_OPTARGS, "string",
		OPTIONAL,
	};

	static OPTION_T opt_b = {
		'b', "# bands (pixels) / sample (default: 1)",
		INT_OPTARGS, "#bands",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_y = {
		'y', "# bytes / pixel (default: 1, or large enough for nbits)",
		INT_OPTARGS, "#bytes",
		OPTIONAL,
	};

	static OPTION_T opt_i = {
		'i', "# bits / pixel (default: nbytes * 8)",
		INT_OPTARGS, "#bits",
		OPTIONAL,
	};

	static OPTION_T opt_f = {
		'f', "force header output; ignore any input",
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_l,
		&opt_s,
		&opt_a,
		&opt_b,
		&opt_y,
		&opt_i,
		&opt_f,
		&operands,
		0
	};

	char          **annot;		/* -> annotation strings	 */
	int             fdi;		/* input file descriptor	 */
	int             fdo;		/* output file descriptor	 */
	int             nlines;		/* image # lines		 */
	int             nsamps;		/* # samples / line		 */
	int             nbands;		/* image # bands		 */
	int            *nbytes;		/* # bytes / pixel		 */
	int            *nbits;		/* # bits / pixel		 */
	bool_t          repeat;		/* ? repeat {nbytes,nbits}[0]	 */
	bool_t		annotf;		/* ? multiple band annotation    */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "make an IPW basic image header");
 /*
  * process options
  */
	nlines = int_arg(opt_l, 0);
	nsamps = int_arg(opt_s, 0);
	nbands = got_opt(opt_b) ? int_arg(opt_b, 0) : DFLT_NBANDS;

	repeat = getyi(nbands, n_args(opt_y), int_argp(opt_y), n_args(opt_i),
		       int_argp(opt_i), &nbytes, &nbits);

	if (!got_opt(opt_a)) {
		annot = NULL;
	}
	else {
		if (n_args(opt_a) != nbands) {
			error("%d bands requires %d annotation strings",
			      nbands, nbands);
		}

		annot = str_argp(opt_a);
	}
  /* added by K. Longley 4/5/91 : */
  /* don't repeat band 0 header if multiple annotation strings given */
	if (n_args(opt_a) > 1)
		annotf = TRUE;
	else
		annotf = FALSE;


 /*
  * access input file
  */
	if (!got_opt(operands)) {
		fdi = got_opt(opt_f) ? ERROR : ustdin();
	}
	else {
		if (got_opt(opt_f)) {
			error("input file not allowed with \"-f\" option");
		}

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
	no_history(fdo);
 /*
  * do it
  */
	mkbih(fdi, nbands, repeat, annotf, nlines, nsamps, nbytes, nbits, 
	      annot, fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mkbih/RCS/main.c,v 1.8 90/11/11 17:06:00 frew Exp $";

#endif
