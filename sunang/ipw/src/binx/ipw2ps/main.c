#include "ipw.h"

#include "getargs.h"

#include "pspic.h"

/*
** NAME
**	ipw2ps -- convert IPW image to PostScript
**
** SYNOPSIS
**	ipw2ps [-r] [-w width] [-h height] [image]
**
** DESCRIPTION
**	ipw2ps converts the input IPW image to a PostScript stream suitable
**	for rendering the image on a PostScript output device (e.g. Apple
**	LaserWriter, Sun NeWS, etc.).
**
** OPTIONS
**	-r	Rotate the image 90 degrees on output.  This is useful to
**		maximize the displayed size of an image that is wider than it
**		is high.
**
**	-h	The PostScript image should be no more than {height} inches
**		high (default: 9.5).
**
**	-w	The PostScript image should be no more than {width} inches
**		wide (default: 7.0).
**
**	Note that -h and -w define a bounding box for the PostScript image.
**	They do NOT change the image's aspect ratio.
**
** EXAMPLES
**	To render "image" on a PostScript printer:
**
**		ipw2ps image | lpr
**
** FILES
**
** DIAGNOSTICS
**	image height {height} too large (max 11 inches)
**	image width {width} too large (max 8.5 inches)
**
**		The output PostScript image must fit on an 8.5 by 11 inch
**		sheet of paper.
**
**	image height {height} too small
**	image width {width} too small
**
**		{height} and {width} must be greater than 0.
**
**	input image must have only 1 band
**	input image must have only 1 byte per pixel
**
**		These are limitations imposed by PostScript's "image"
**		operator.
**
** RESTRICTIONS
**	The input image must have only one band.
**
**	PostScript output devices use halftone screening to simulate multiple
**	gray levels.  Aliasing will occur as the pixel density of the output
**	image approaches the halftone screen frequency.
**
**	Alternatives to ipw2ps exist.  The program ipw2pgm will convert IPW
**	images to a PGM image which can be converted to PostScript through
**	a filter by pnmtops, or with a GUI using interactive scaling using
**	xv.  The program ipw2ppm can be used to create color images.
**
** FUTURE DIRECTIONS
**	There should be command line options controlling the size and position
**	of the output image on the PostScript virtual page.
**
**	The restriction of 8.5 x 11 inches should be relaxed -- not everyone
**	lives in the United States.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:   ipw2pgm, ipw2ppm, ipw2sun
**	UNIX:  lpr
**	Image: pnmtops, xv, ras2ps, rash, rletops, tiff2ps
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_r = {
		'r', "rotate output image 90 degrees",
	};

	static OPTION_T opt_h = {
		'h', "output image height (inches)",
		REAL_OPTARGS, "height",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_w = {
		'w', "output image width (inches)",
		REAL_OPTARGS, "width",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 0, 1,
	};

	static OPTION_T *optv[] = {
		&opt_r,
		&opt_h,
		&opt_w,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	double          h_img;		/* PostScript image height	 */
	double          h_page;		/* PostScript page height	 */
	double          w_img;		/* PostScript image width	 */
	double          w_page;		/* PostScript page width	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "convert image to PostScript");
 /*
  * collect options
  */
	h_page = PAGE_HEIGHT;
	w_page = PAGE_WIDTH;

	if (got_opt(opt_h)) {
		h_img = real_arg(opt_h, 0);

		if (h_img <= 0.0) {
			error("image height %g too small", h_img);
		}

		if (h_img > IMG_HEIGHT) {
			error("image height %g too large (max %g inches)",
			      h_img, IMG_HEIGHT);
		}
	}
	else {
		h_img = IMG_HEIGHT;
	}

	if (got_opt(opt_w)) {
		w_img = real_arg(opt_w, 0);

		if (w_img <= 0.0) {
			error("image width %g too small", h_img);
		}

		if (w_img > IMG_WIDTH) {
			error("image width %g too large (max %g inches)",
			      w_img, IMG_WIDTH);
		}
	}
	else {
		w_img = IMG_WIDTH;
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

	no_tty(fdi);
 /*
  * do it
  */
	pspic(fdi, h_page, w_page, h_img, w_img, got_opt(opt_r));
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.11 87/11/02 13:52:55 frew Exp $";

#endif
