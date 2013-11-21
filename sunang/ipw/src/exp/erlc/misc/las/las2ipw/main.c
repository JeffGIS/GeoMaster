
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   main.c   1.7   9/3/91";
#endif

/*
** NAME
**	las2ipw -- Convert a LAS image to an IPW image
**
** SYNOPSIS
**	las2ipw -i lasname [-r] [-h header] [-g]
**
** DESCRIPTION
**	las2ipw converts an image file in LAS (Land Analysis System) format
**	to IPW (Image Processing Workbench) format.  If an IPW header is
**	provided by the -h option or stdin, the given headers will be written
**	to the IPW file (except a GEO header if -g is not specified);
**	otherwise the contents of the basic image (BI), geodetic (GEO) and
**	linear quantization (LQ) headers are derived from the LAS DDR file.
**	If both the -r flag and a header file are specified, the given
**	linear quantization header will be written to the output image, but
**	the data values will be copied as raw pixel values bypassing the
**	floating point mapping (see examples).  The IPW output image is
**	written to stdout.
**
** OPTIONS
**	-i		LAS image name
**	-r		force raw pixel values to equal LAS image pixel
**			values; bypass floating point conversion if used
**			in conjunction with -h option
**			(can't be used with REAL*4 LAS images) 
**	-h		input header file (default: stdin)
**	-g		flag to override LAS geographic area with GEO header
**			in given header file
**
** EXAMPLES
**	Suppose lasfile.img is a LAS image with pixel data type BYTE with 
**	values ranging from 0-255 and that header is an IPW file with
**	an LQ header defining a linear mapping of pixel value to floating
**	point with map (0,0) and (255,1.0).  The command:
**
**		las2ipw -i lasfile -r -h header -o ipwfile
**
**	will create an 8-bit IPW image file ipwfile with raw pixel values
**	0-255 mapped to floating point 0.0 - 1.0.  The command:
**
**		las2ipw -i lasfile -o ipwfile
**
**	will create the IPW image file ipwfile with raw pixel values 0-255
**	mapped to floating point 0.0 - 255.0.
**
**	Suppose lasreal.img is a LAS image with data type REAL*4 and values
**	ranging from 0.0 to 1000.0, and that real.head is an IPW header file
**	specifying 12-bit pixels with a floating point map of
**	(0,0),(4096,1000.0).  The command:
**
**		las2ipw -i lasreal.img -h real.head -o ipwreal
**
**	will create a 12-bit IPW image ipwreal with raw pixel values 0-4096
**	mapped to floating point values 0.0-1000.0.  (Note that the pixel
**	values will have the approximate values of the original LAS values
**	with an error of no more than 0.244141 (1000/4096)).  The command:
**
**		las2ipw -i lasreal.img -o ipwreal
**
**	will create an 8-bit IPW image ipwreal with raw pixel values 0-255
**	mapped to floating point values 0.0-1000.  Since no header was
**	provided, an 8-bit image (default) was created.
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	The input LAS image must have a '.img' filename extension (though it
**	need not be specified) and a valid DDR file with the same name prefix
**	as the image with '.ddr' extension.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	5/8/91   Written by Kelly Longley, Oregon State University,
**		 Environmental Research Laboratory, Corvallis OR
**	5/14/91  Now works for all LAS data types, K. Longley, OSU, EPA ERL-C
**	6/17/91  Now used IPW projdef file (ASCII) instead of LAS .proj file
**		 for projection definition parameters, K. Longley, OSU, EPA
**
** BUGS
*/

#include <fcntl.h>
#include <unistd.h>
#include "ipw.h"
#include "las.h"
#include "getargs.h"
#include "proj.h"



main (argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_i = {
		'i', "LAS image name",
		STR_OPTARGS, "LASimage",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_r = {
		'r', "raw pixel values  (don't convert to linear quantization)"
	};

	static OPTION_T opt_h = {
		'h', "header file (default: stdin)",
		STR_OPTARGS, "header",
		OPTIONAL, 1, 1,
	};

	static OPTION_T opt_g = {
		'g', "override LAS geographic area with given header"
	};

	static OPTION_T *optv[] = {
		&opt_i,
		&opt_r,
		&opt_h,
		&opt_g,
		0
	};

	int             fdo;		/* output image file descriptor	 */
	int             fdh;		/* LQ file descriptor		 */
	int		pixsiz;		/* pixel size in bytes		 */
	bool_t		override_geo;	/* flag to use geo in header	 */
	bool_t		raw;		/* flag for raw mode		 */
	char		lasname[CMLEN];	/* -> name of LAS image		 */
	char	       *ext;		/* -> extension of LAS filename	 */
	struct DDR	ddr;		/* LAS image DDR		 */


   /* begin */

	ipwenter (argc, argv, optv, "convert LAS image to IPW image");

   /* access header file, if given */

	if (got_opt (opt_h)) {
		if ((fdh = uropen (str_arg(opt_h, 0))) < 0) {
			error ("error opening header file %s", str_arg(opt_h,0));
		}
	} else {
		fdh = ustdin();
		if (isatty(fdh))
			fdh = ERROR;
	}

   /* get flags */

	override_geo = got_opt (opt_g);
	if (override_geo && fdh == ERROR) {
		warn ("-g option ignored - no header file given");
		override_geo = FALSE;
	}

	raw = got_opt (opt_r);

   /* access output image */

	fdo = ustdout();
	no_tty (fdo);

   /* construct LAS filename - make sure LAS image exists */

	strcpy (lasname, str_arg (opt_i, 0));
	if ((ext = strrchr (lasname, '.')) == NULL) {
		strcat (lasname, ".img");
	} else if (strcmp (ext, ".img")) {
		strcat (lasname, ".img");
	}
	if (access (lasname, R_OK)) {
		error ("error accessing %s", lasname);
	}

   /* read DDR of input image */

	if (c_getddr (lasname, &ddr) != E_SUCC) {
		error ("error reading DDR file for image %s", lasname);
	}

	switch (ddr.dtype) {
	    case 1:
		pixsiz = sizeof(char);
		break;
	    case 2:
		pixsiz = sizeof(short);
		break;
	    case 3:
		pixsiz = sizeof(long);
		break;
	    case 4:
		pixsiz = sizeof(float);
		break;
	}

   /* can't copy REAL pixels as raw pixels */

	if (ddr.dtype == 4 && raw) {
		warn ("raw flag ignored for LAS image with REAL pixels");
		raw = FALSE;
	}

   /* Write output headers */

	headers (lasname, fdo, fdh, ddr, override_geo, raw);

   /* Copy the image */

	copyimg (lasname, fdo, ddr, pixsiz, raw);

   /* All done */

	ipwexit (EX_OK);
}
