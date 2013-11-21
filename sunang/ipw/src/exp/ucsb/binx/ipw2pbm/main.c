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
**	ipw2pbm -- convert IPW image to PGM or PPM formats
**
** SYNOPSIS
**	ipw2pbm [ image ]
**
** DESCRIPTION
**	ipw2pbm reads {image} (default: standard input) and writes its
**	equivalent to the standard output in either the PGM "portable graymap"
**	format (if {image} has 1 band) or the PPM "portable pixmap" format (if
**	{image} has 3 bands).
**
** OPTIONS
**
** EXAMPLES
**	To display an IPW image using "xv" on an X window system display:
**
**		ipw2pbm image | xv
**
** FILES
**
** DIAGNOSTICS
**	The following diagnostics indicate violations of restrictions imposed
**	by the PGM or PPM image formats:
**
**	{nbits}-bit pixels (must be <= 8)
**
**		"raw"-format PGM or PPM images cannot have more than 8 bits
**		per pixel
**
**	{nbands} bands (must be either 1 or 3)
**
**		PGM images must have 1 band.  PPM images must have 3 bands.
**
**	pixel sizes differ: bands 0, {band}
**
**		All 3 bands in a PPM image must have the same number of bits
**		per pixel.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**	The programs ipw2pgm and ipw2ppm overcome some of the restrictions
**	of ipw2pbm and are more flexible.
**
**	There should be programs ipw2pnm and pnm2ipw to do conversion from
**	any of the PBMplus formats to IPW.
**
** HISTORY
**	11/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:   pbm2ipw, ipw2pgm, ipw2ppm
**	UNIX:  pbm, pgm, ppm
**	Image: xv, xloadimage
**
**	"PBMPLUS -- Extended Portable Bitmap Toolkit", Jef Poskanzer,
**		10 December, 1991.  Anonymous ftp from export.lcs.mit.edu:
**		/contrib/pbmplus.tar.Z.
**
**	xv and xloadimage are also available on export.lcs.mit.edu.
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "convert IPW image to PGM or PPM formats");
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
	ipw2pbm();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/ipw2pbm/RCS/main.c,v 1.2 90/11/11 17:04:45 frew Exp $";

#endif
