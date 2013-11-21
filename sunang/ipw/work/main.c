
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   main.c   4.6   9/3/91";
#endif

/*
** NAME
**	itrbxfr -- calculate H & LE using Brutsaert's method
**
** SYNOPSIS
**	itrbxfr -d delta_t [-z upper_height] [-m mask] [image]
**
** DESCRIPTION
**	Calculates turbulent transfer using Brutsaert's description
**	of the Businger-Dyer approach, using the Obukhov length for
**	stability determination: (Refs in hle1)
**	Air pressure is set from elevation at each grid point.
**	Reads five-band image from specified file (or stdin):
**
**		RH, ta, u, elev, z0
**
**		RH   = relative humidity
**		ta   = air temperature (C)
**		elev = site elevation (m)
**		u    = upper wind speed (m/sec)
**		z0   = roughness length (m)
**
**	Assumptions:
**		Lower wind speed (u0) = 0.0 m/s
**		Surface temperature (ts) = air temperature (ta)
**		Lower vapor pressure (es) = saturation vapor pressure at ts
**		Upper vapor pressure (ea) is calculated from RH: ea = RH * es
**
**	Writes three-band output image to stdout:
**
**		h  = sens heat flux (+ to surf) (W/m^2)
**		le = latent heat flux (+ to surf) (W/m^2)
**		mm = water gain/loss (+/-mm m^-2)
**
** OPTIONS
**	d	delta time (hrs)
**	z	upper height (meters) - defaults to 3.0 meters
**	m	mask image
**
** OPERANDS
**	image	six-band input image containing z0, t, t0, e, e0, u
**		(defaults to stdin)
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**	terminates with error message;
**
** RESTRICTIONS
**
** HISTORY
**	4/19/90  Modified D. Marks' point model (QDIPS program trbxfr) to run
**		 over an IPW image, by Kelly Longley, Oregon State University,
**		 Environmental Research Laboratory, Corvallis
**	7/3/90	 Can now process classified image bands, by K. Longley, OSU, EPA
**	10/19/90 Added -z option to specify upper height, K. Longley, OSU, EPA
**
** FUTURE DIRECTIONS
**
*/

#include	"ipw.h"
#include	"newdefs.h"
#include 	"getargs.h"
#include	"itrbxfr.h"

void
main (argc, argv)
	int	argc;
	char	**argv;
{
	static OPTION_T opt_z = {
		'z', "upper height (meters)",
		REAL_OPTARGS, "upper_height",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_d = {
		'd', "delta time (hrs)",
		REAL_OPTARGS, "delta_t",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_m = {
		'm', "mask image",
		STR_OPTARGS, "mask",
		OPTIONAL, 1, 1
	};

	static OPTION_T operand = {
		OPERAND, "input image (defaults to stdin)",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_d,
		&opt_z,
		&opt_m,
		&operand,
		0
	};

	int	fdi;		/* input image file descriptor	*/
	int	fdm;		/* mask image file descriptor	*/
	int	fdo;		/* output image file descriptor */
	double	delta_t;	/* delta time (s)		*/
	double	z;		/* upper height (m)		*/


   /* get args */

	if (getargs (argc, argv, optv,
	     "calculate H & LE using Brutsaert's method") == NULL) {
		usage();
	}

   /* process args */

	delta_t = real_arg (opt_d, 0);
	delta_t *= HRS_2_SEC;

	if (got_opt (opt_z)) {
		z = real_arg (opt_z, 0);
	} else {
		z = Z_DEFAULT;
	}

   /* access mask image if specified */

	if (got_opt (opt_m)) {
		fdm = uropen (str_opnd(opt_m, 0));
		if (fdm == ERROR)
			error ("Can't open \"%s\"", str_opnd(opt_m, 0));
	} else {
		fdm = ERROR;
	}
  
   /* access input image */

	if (got_opt (operand)) {
		fdi = uropen (str_opnd(operand, 0));
		if (fdi == ERROR)
			error ("Can't open \"%s\"", str_opnd(operand, 0));
	} else {
		fdi = ustdin();
	}

    /* can't read or write tty */

	no_tty (fdi);
	fdo = ustdout ();
	no_tty (fdo);

   /* read/write headers */

	headers (fdi, fdm, fdo, IBANDS, OBANDS);

   /* read input image, perform calculations and write output image */

	itrbxfr (fdi, fdm, fdo, delta_t, z);

   /* all done */

	ipwexit (EX_OK);
}
