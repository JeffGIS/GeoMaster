#include "ipw.h"

#include "getargs.h"

#include "pgm.h"

/*
** NAME
**	ipw2xim -- convert IPW image to "xim" format
**
** SYNOPSIS
**	ipw2xim [image]
**
** DESCRIPTION
**	ipw2xim reads an IPW image (default: standard input) and writes it
**	it to the standard output in "xim" format.  An image in xim format
**	may be displayed in a X window by the xim or xxim commands. 
**
**	Since xv and xloadimage support many more operations and formats
**	than the xim program, including modified versions of each that
**	support IPW images directly, ipw2xim is archaic.  ipw2ppm will
**	convert images into the industry standard PPM format, including
**	support for color images.
**
** OPTIONS
**
** EXAMPLES
**	To display the IPW image "zot" in an X window:
**
**		ipw2xim zot | xim
**
** FILES
**
** DIAGNOSTICS
**	image must have 1 band
**
**		ipw2xim supports only single band images.
**
**	band 0 has more than 256 levels per pixel
**
**		The xim format does not support more than 8 bits per color
**
** RESTRICTIONS
**	Color images are not currently supported.
**
**	xim and xxim automatically requantize their input (by dithering) to
**	the resolution of the X display device.
**
** FUTURE DIRECTIONS
**	ipw2xim should be modified to create color xim images.  Since the
**	xim color image format is band sequential, the easiest way to do
**	this would be to have ipw2xim process 3 single-band input images:
**
**		ipw2xim red green blue
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:   xim, ipw2ppm
**	Image: ximtoppm
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
	ipwenter(argc, argv, optv, "convert IPW image to xim format");
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
	ipw2xim();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr.MC68020/home/ipw/adm/snoopy/src/bin/ipw2xim/RCS/main.c,v 1.1 89/10/25 19:53:53 frew Exp $";

#endif
