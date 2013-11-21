#include "ipw.h"

#include "getargs.h"

#include "pgm.h"

/*
** NAME
**	ipw2hds -- convert IPW image to HDS sixel format
**
** SYNOPSIS
**	ipw2hds [image]
**
** DESCRIPTION
**	ipw2hds reads an IPW image (default: standard input) and writes an
**	equivalent image in HDS sixel format to the standard output.  The
**	output is suitable for display on an HDS 3200 series terminal
**	without further processing.
**
** OPTIONS
**
** EXAMPLES
**	To display "image" on an HDS 3200 terminal:
**
**		dither image | ipw2hds
**
** FILES
**
** DIAGNOSTICS
**	input image must be single-band
**
**	input image must have 1-bit pixels
**
** RESTRICTIONS
**	The input image must have 1 band and 1 bit per pixel.
**
**	ipw2hds may be usable with other sixel-oriented devices such as
**	DEC printers, but this has not been tested.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:   demux, dither
**	Image: ppmtosixel
**
**	"HDS3200 Programmer's Reference Manual", Human Designed Systems,
**	Philadelphia, 1988, # DN-13C4-8802-1
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
	ipwenter(argc, argv, optv, "convert IPW image to HDS sixel format");
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
  * do it
  */
	headers();
	hdspic();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.1 89/04/16 17:35:01 frew Exp $";

#endif
