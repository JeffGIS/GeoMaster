#include "ipw.h"

#include "getargs.h"

#include "pgm.h"

/*
** NAME
**	dempt -- convert USGS DEM to IPW image
**
** SYNOPSIS
**	dempt [-v] [-r] [-d USGS_DEM] [-h LQH]
**
** DESCRIPTION
**	Dempt reads a USGS Digital Elevation Model from the standard
**	input, or from a file, and writes an IPW image to the standard
**	output.  A linear quantization header may be provided (see
**	mklqh); otherwise the output LQH is determined from the input
**	values.  Either the USGS DEM or the LQH may be read from the
**	standard input.
**
** OPTIONS
**	-v	Request extra informative output be printed to the
**		standard error.
**
**	-r	print raw pixel values, bypassing any conversion to
**		floating-point.
**
**	-d	input USGS DEM
**
**	-h	input linear quantization header
**
**	At least one of -d and/or -h must be specified.
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**	-r and -h (or LQH read from stdin) not compatible
**
**		Using an LQH and the -r option together is not acceptable.
**
**	DEM level code = {demlevel}, out of range
**
**		The DEM levels must be between 1 and 3.
**
**	elevation pattern code = {dempat}, out of range
**
**		The DEM pattern code must be either 1 or 2.
**
**	DEM reference system code = {refsys}, out of range
**
**		The DEM planimetric reference system code must be between
**		0 and 20.
**
**	reference units code = {refunits}, out of range
**
**		The DEM units of measure for reference system code must 
**		be between 0 and 3.
**
**	elevation units code = {elevunits}, out of range
**
**		The DEM units of measure for elevations code must be
**		either 1 or 2.
**
**	# polygon sides = {polysides}, not supported
**
**		dempt does not support DEMS with other than 4 sides.
**
**	minimum elevation negative, ({zmin}), therefore can't use -r option
**
**		Raw values are not supported when elevations can be negative.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	8/1/89	Written by Jeff Dozier, UCSB.
**	4/12/93	Modifed reference system warning.  Dana Jacobsen, ERL-C
**
** BUGS
**	Seems to have problems with DEM level 2 files.
**
** SEE ALSO
**	IPW:  tmpt
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_v = {
		'v', "verbose output"
	};

	static OPTION_T opt_r = {
		'r', "raw values (don't convert to linear quantization)"
	};

	static OPTION_T opt_d = {
		'd', "input USGS DEM",
		STR_OPTARGS, "USGS_DEM",
		OPTIONAL, 1, 1,
	};

	static OPTION_T opt_h = {
		'h', "input linear quantization header",
		STR_OPTARGS, "LQH",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_v,
		&opt_r,
		&opt_d,
		&opt_h,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "converts USGS DEM to IPW image");
 /*
  * verbose option
  */
	parm.verbose = got_opt(opt_v);
 /*
  * raw values
  */
	parm.raw = got_opt(opt_r);
 /*
  * access input DEM file
  */
	if (!got_opt(opt_d)) {
		parm.i_fp = stdin;
	}
	else {
		parm.i_fp = fopen(str_arg(opt_d, 0), "r");
		if (parm.i_fp == NULL) {
			error("can't open \"%s\"", str_arg(opt_d, 0));
		}
	}

	no_tty(fileno(parm.i_fp));
 /*
  * access input LQH, if there is one, and set flag
  */
	if (!got_opt(opt_h)) {
		if (got_opt(opt_d)) {
			parm.h_fd = ustdin();
			parm.islqh = !isatty(parm.h_fd);
		}
		else {
			parm.islqh = FALSE;
		}
	}
	else {
		parm.h_fd = uropen(str_arg(opt_h, 0));
		if (parm.h_fd == ERROR) {
			error("can't open \"%s\"", str_arg(opt_h, 0));
		}
		parm.islqh = TRUE;
	}
 /*
  * raw flags and specified LQH are not compatible
  */
	if (parm.raw && parm.islqh) {
		error("-r and -h (or LQH read from stdin) not compatible");
	}
 /*
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * do it
  */
	demhdr();
	profiles();
	headers();
	output();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.2 89/01/08 09:32:29 dozier Exp $";

#endif
