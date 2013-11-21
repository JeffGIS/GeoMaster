#include "ipw.h"
#include "getargs.h"

/*
** NAME
**	toporad -- topographic distribution of solar radiation
**
** SYNOPSIS
**	toporad [-n] [-r R0] [image]
**
** DESCRIPTION
**	toporad calculates the topographic distribution of solar radiation
**	at a single time, using input beam and diffuse radiation calculations
**	supplied by elevrad.  The input image has the following bands:
**
**		beam irradiance
**		diffuse irradiance
**		local illumination angle
**		sky view factor
**		terrain configuration factor
**		surface albedo
**
**	The input image should also have a "sun" header specifying the
**	cosine of the solar zenith angle.
**
**	The first five bands are usually calculated using other programs.
**	elevrad calculates beam and diffuse radiation, shade calculates
**	the local illumination angle (with input from gradient), and viewf
**	calculates the sky view and terrain configuration factors.
**
** OPTIONS
**	-n	Radiation output will be net, rather than incoming.
**
**	-r	reflectance of the substrate (if not specified, albedo at
**		the point will be used to calculate reflectance of the
**		adjacent terrain)
**
** EXAMPLES
**	Running toporad over a section of the Columbia Basin on June 22, 1990,
**	with input files muxed together:
**
**		mux beam-diffuse illcos skyview-terrain albedo | toporad
**
** FILES
**
** DIAGNOSTICS
**	input image has no LQH
**
**	input image has no SUNH, mu0 set to 0.5
**	input band {band} has no SUNH, mu0 set to 0.5
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/5/89	Written by Jeff Dozier, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  gtoporad, elevrad, shade, gradient, viewf, horizon, hor1d
*/

extern void toporad();

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_n = {
		'n', "net radiation instead of incoming",
	};

	static OPTION_T opt_r = {
		'r', "Reflectance of the substrate",
		REAL_OPTARGS, "R0",
		OPTIONAL, 1, 1
	};

	static OPTION_T operands = {
		OPERAND, "beam & diffuse irrad, mu_s, Vf, Ct, albedo",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_n,
		&opt_r,
		&operands,
		0
	};

	int             fdi;		/* input file descriptor	 */
	int             fdo;		/* output file descriptor	 */
	float		R0;		/* reflectance of the substrate	 */

 /*
  * begin
  */

	ipwenter(argc, argv, optv,
		 "topographic distribution of solar radiation");

 /*
  * access input files
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
  * get options
  */
	if (got_opt(opt_r))
		R0 = real_arg (opt_r, 0);
	else
		R0 = 0.0;
 /*
  * read input data, calculate, and write
  */

	toporad(fdi, fdo, got_opt(opt_n), got_opt(opt_r), R0);

 /*
  * all done
  */

	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/dozier/ipw/src/bin/toporad/RCS/main.c,v 1.1 89/07/05 13:25:24 dozier Exp $";

#endif
