#include "ipw.h"
#include "getargs.h"

/*
** NAME
**	elevrad -- beam and diffuse radiation in elevation image
**
** SYNOPSIS
**	elevrad -z elev -t tau -w omega -g gfact -r R0 -s S0 -u cos
**	        [-n bits0[,bits1]] [image]
**
** DESCRIPTION
**	elevrad reads elevations from {image} (default: standard input)
**	and writes a 2-band image containing beam radiation (normal to
**	sun) and diffuse radiation (on a horizontal surface) to the
**	standard output.
**
**	elevrad is used mainly as a preprocessor for the toporad command.
**
** OPTIONS
**	-z	The elevation of optical depth measurement is {elev} meters.
**
**	-t	The optical depth at {elev} is {tau}.
**
**	-w	The single-scattering albedo is {omega}.
**
**	-g	The scattering asymmetry factor is {gfact}.
**
**	-r	The mean surface albedo is {R0}.
**
**	-s	The exoatmospheric solar irradiance is {S0}.  This can be
**		calculated with the IPW command "solar".
**
**	-u	The cosine of the solar zenith angle is {cos}.  This can be
**		calculated with the IPW command "sunang".
**
**	-n	The number of bits in the output band(s) is {bits0}
**		and {bits1} (default: same number as in the input image).
**
** EXAMPLES
**	To calculate beam and diffuse radiation with specified parameters
**	on a section of the Columbia basin in June of 1990:
**
**		elevrad -z 100 -t .2 -w .85 -g .55 -r .155 -s 150.214 \
**			-u 0.816876  image
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
**	4/3/88	Written by Jeff Dozier, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  gelevrad, toporad, gtoporad, twostream, solar, sunang
*/

extern void     init_tau();
extern void     elevrad();

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_z = {
		'z', "elevation of optical depth measurement",
		REAL_OPTARGS, "elev",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_t = {
		't', "optical depth at z",
		REAL_OPTARGS, "tau",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_w = {
		'w', "single-scattering albedo",
		REAL_OPTARGS, "omega",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_g = {
		'g', "scattering asymmetry parameter",
		REAL_OPTARGS, "gfact",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_r = {
		'r', "mean surface albedo",
		REAL_OPTARGS, "R0",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_s = {
		's', "exoatmospheric solar irradiance",
		REAL_OPTARGS, "S0",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_u = {
		'u', "cosine solar zenith angle",
		REAL_OPTARGS, "mu0",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_n = {
		'n', "# output bits (default same as input)",
		INT_OPTARGS, "bits",
		OPTIONAL, 1, 2
	};

	static OPTION_T operands = {
		OPERAND, "input elevation image",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_z,
		&opt_u,
		&opt_t,
		&opt_w,
		&opt_g,
		&opt_r,
		&opt_s,
		&opt_n,
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */
	int             nbits[2];	/* # output bits		 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
	     "beam and diffuse radiation (preprocessor for toporad)");
 /*
  * access input file(s)
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

	fdo = ustdout();
	no_tty(fdo);
 /*
  * evaluate nbits
  */
	switch (n_args(opt_n)) {
	case 0:			/* neither, so both <- 0	 */
		nbits[0] = nbits[1] = 0;
		break;
	case 1:			/* band 0 set, so band[1] <- 0	 */
		nbits[0] = int_arg(opt_n, 0);
		nbits[1] = 0;
		break;
	case 2:			/* both set			 */
		nbits[0] = int_arg(opt_n, 0);
		nbits[1] = int_arg(opt_n, 1);
	}
 /*
  * read headers
  */
	init_tau(fdi, fdo,
		 real_arg(opt_z, 0), real_arg(opt_u, 0),
		 real_arg(opt_t, 0), real_arg(opt_w, 0),
		 real_arg(opt_g, 0), real_arg(opt_r, 0),
		 real_arg(opt_s, 0), nbits);
 /*
  * beam and diffuse radiation for elevation image
  */
	elevrad(fdi, fdo);

	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.1 88/04/03 12:33:16 dozier Exp $";

#endif
