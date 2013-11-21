
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   main.c   1.4   9/3/91";
#endif

/*
** NAME
**	ipw2las -- Convert an IPW image to a LAS image
**
** SYNOPSIS
**	ipw2las [-r] [-f] -o lasname [-p projdef] [-m mask] [ipwimage]
**
** DESCRIPTION
**	ipw2las converts an image file in IPW (Image Processing Workbench)
**	format to an image in LAS (Land Analysis System) format.  Two
**	LAS files will be written: <lasname>.img and <lasname>.ddr
**	containing the LAS image data and the associated DDR.
**
**	If the -r flag is specified, the IPW pixel data will be copied as
**	their raw values; no floating point conversion will be performed.  If
**	the -f flag is specified, the IPW pixel data will be mapped to
**	floating point according the the linear quantization header and will
**	be written to the LAS image in floating point format.  If neither
**	-r or -f is specified, the IPW pixel data will be mapped to floating
**	point (if an lq header is present) and integerized prior to copying
**	to the LAS image.  (See EXAMPLES)
**
**	The projection definition file is an ASCII file containing the
**	projection parameters created by programs mkgeo, mkalbers, mkutm, etc.
**	See the man page of the appropriate "mk" command for the format of
**	this file.  If a projdef file is not specified, the projection fields
**	in the DDR file will be set to INVALID.
**
**	If an IPW mask image is specified, the pixels that correspond to
**	zero pixels in the mask image will be set to zero in the LAS image,
**	reguardless of the pixel value in the IPW input image.
**
** OPTIONS
**	-r		flag to copy raw pixels to LAS image (no FP conversion)
**	-f		flag for floating point data in LAS image
**	-o		LAS image name 
**	-p		name of projection definition file
**	-k		ASCII key in PROJ file
**	-m		mask image
**
** OPERAND
**	ipwimage	IPW image file (default: stdin)
**
** EXAMPLES
**	In the following examples, the IPW image ipwname is an 8-bit image
**	with a linear quantization header with the map (0,0),(255,1000).
**	The command
**
**		ipw2las -r -o lasname -p projfile ipwname
**
**	will create an 8-bit LAS image with pixel values in the range 0-255.
**	The projection information is set by the values in projfile.
**
**	The command
**
**		ipw2las -f -o lasname ipwname
**
**	will create a floating point LAS image with the values 0-1000.  The
**	DDR file will have no projection information.
**
**
**	The command
**		ipw2las -o lasname ipwname
**
**	will create a LAS image with 2-byte pixels with integer values 0-1000.
**
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	5/14/91  Written by Kelly Longley, Oregon State University,
**		 Environmental Research Laboratory, Corvallis OR
**	6/17/91  Changed to use IPW projdef file rather than LAS .proj file
**		 by K. Longley, OSU, EPA ERL-C
**
** BUGS
*/

#include <fcntl.h>
#include "ipw.h"
#include "mproj.h"
#include "las.h"
#include "getargs.h"


main (argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_r = {
		'r', "copy pixels in raw format - no floating point mapping"
	};

	static OPTION_T opt_f = {
		'f', "copy pixels as floating point values (default: integerize)"
	};

	static OPTION_T opt_o = {
		'o', "LAS image name",
		STR_OPTARGS, "LASname",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_p ={
		'p', "projection definition file",
		STR_OPTARGS, "projdef",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_m ={
		'm', "IPW mask image",
		STR_OPTARGS, "mask",
		OPTIONAL, 1, 1
	};

	static OPTION_T operand = {
		OPERAND, "IPWimage",
		STR_OPTARGS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_r,
		&opt_f,
		&opt_o,
		&opt_p,
		&opt_m,
		&operand,
		0
	};

	char		lasname[CMLEN];	/* -> name of LAS image		 */
	char	       *ext;		/* -> extension of LAS filename	 */
	int             fdi;		/* input image file descriptor	 */
	int		fdm;		/* mask image file descriptor	 */
	int		nlines;		/* # lines in output image	 */
	int		nsamps;		/* # samps in output image	 */
	int		nbands;		/* # bands in output image	 */
	bool_t		got_proj;	/* flag if PROJ file supplied	 */
	bool_t		got_geo;	/* flag if GEO header in image	 */
	bool_t		raw;		/* flag for raw format		 */
	bool_t		float_dtype;	/* flag for pixels in float type */
	float		maxpv;		/* max pixel value of all bands	 */
	double		bline;		/* geo y-coord of UL corner	 */
	double		bsamp;		/* geo x-coord of UL corner	 */
	double		dline;		/* pixel size in y-direction	 */
	double		dsamp;		/* pixel size in x-direction	 */
	struct projdef *prj;		/* parameters from projdef file	 */


   /* begin */

	ipwenter (argc, argv, optv, "convert LAS image to IPW image");


   /* access input image */

	if (got_opt (operand)) {
		fdi = uropen (str_arg(operand, 0));
		if (fdi < 0) {
			error ("error opening IPW image file %s",
			       str_arg(operand,0));
		}
	} else {
		fdi = ustdin();
	}
	no_tty (fdi);

   /* access the mask image if specified */

	if (got_opt (opt_m)) {
		fdm = uropen (str_arg(opt_m, 0));
		if (fdm < 0) {
			error ("error opening mask image file %s",
			       str_arg(opt_m,0));
		}
	} else {
		fdm = ERROR;
	}

   /* get raw and float options */

	raw = got_opt (opt_r);
	float_dtype = got_opt (opt_f);
	if (raw && float_dtype) {
		error ("-r and -f options are incompatible");
	}

   /* get projection parameters from PROJ file, if supplied */

	if (got_opt (opt_p)) {
		if ((prj = readproj (str_arg(opt_p, 0))) == NULL) {
			error ("error reading projdef file");
		}
		got_proj = TRUE;
	} else {
		got_proj = FALSE;
	}

   /* read input headers */

	headers (fdi, fdm, raw, &maxpv, &got_geo, &bline, &bsamp, &dline, &dsamp);

   /* construct LAS filename - append .img if needed */

	strcpy (lasname, str_arg (opt_o, 0));
	if ((ext = strrchr (lasname, '.')) == NULL) {
		strcat (lasname, ".img");
	} else if (strcmp (ext, ".img")) {
		strcat (lasname, ".img");
	}

   /* copy the image */

	copyimg (fdi, fdm, lasname, raw, float_dtype, maxpv);

   /* update the DDR */

	update_ddr (lasname, bline, bsamp, dline, dsamp, got_geo, got_proj, prj);

   /* all done */

	ipwexit (EX_OK);
}
