#include "ipw.h"

#include "getargs.h"

#include "sunras.h"

/*
** NAME
**	ipw2sun -- convert IPW image to Sun rasterfile
**
** SYNOPSIS
**	ipw2sun [image]
**
** DESCRIPTION
**	ipw2sun reads an IPW image (default: standard input) and writes it
**	to the standard output in Sun Microsystems "rasterfile" format.
**
** OPTIONS
**
** EXAMPLES
**	To dither a single-band image and display it on a Sun:
**
**		dither image | ipw2sun | screenload
**
** FILES
**
** DIAGNOSTICS
**	input image has {nbands} bands (only 1 allowed)
**	input image has {nbits}-bit pixels (only 1 or 8 allowed)
**	input image has {bytes}-byte pixels (only 1 allowed)
**
**		These restrictions are inherent in the Sun rasterfile
**		formats supported.
**
** RESTRICTIONS
**	The input image must have 1 band and 1 or 8 bits per pixel.
**
** FUTURE DIRECTIONS
**	Support more of the rasterfile formats.  This is not really
**	needed, since conversion to PPM format makes the image accessible
**	to more programs, and is more flexible.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/18/93	Only use Sun's header file when on a 386i.  Dana Jacobsen, ERLC.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:   dither, rastool, ipw2pgm, ipw2ppm
**	UNIX:  screenload
**	Image: pnmtorast, rasttopnm, rastorle, rletorast, xv, xloadimage
**
**	rasterfile(5) in "UNIX Interface Reference Manual", part number 
**	800-1303-04, Sun Microsystems, Inc.
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

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "convert IPW image to Sun rasterfile");
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
	sunras(fdi, fdo);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.2 87/11/02 13:55:07 frew Exp $";

#endif
