/*
** NAME
**	refdx -- calculate refractive index from wavelengths
**
** SYNOPSIS
**	refdx [-r] [-i] -s substance[,...] [-t temp] [-u units] [-d]
**
** DESCRIPTION
**	refdx prints out the complex refractive index for the named
**	sunstance(s), for wavelengths read from the standard input.
**	Optionally, the dielectric function can be printed instead of the
**	refractive index.
**
**	The values are obtained by fitting an Akima spline to data tables.
**
** OPTIONS
**	-r	Only the real part is printed (default: print complex).
**
**	-i	Only the imaginary part is printed (default: print complex).
**
**	-s	The substance in question is {substance}.  The substances
**		supported are:
**
**			ice
**			water
**
**	-t	The temperature in degrees Celcius is {temp} (default: -5).
**
**	-u	The wavelengths are specified in {units} (default: um,
**		micrometers).  To obtain a list of supported units, use
**		"help" for the units.  These are currently:
**
**			A	Angstroms
**			nm	nanometers
**			um	micrometers
**			mm	millimeters
**			cm	centimeters
**			m	meters
**			icm	inverse centimeters
**			MHz	megahertz
**			GHz	gigahertz
**
**	-d	Print the dielectric function for {substance} (default: print
**		the refractive index).
**
** EXAMPLES
**	The refractive index of ice and water for 3.55 micrometers:
**
**		refdx -s ice,water
**		3.55
**
**	should yield:
**
**		3.55    1.4366  1.125e-02       1.3914  7.196e-03
**
**	Where at 23 cm:
**
**		refdx -s ice,water -u cm
**		23
**
**	should yield:
**
**		23      1.7865  2.166e-04       9.4017  7.677e-01
**
** FILES
**
** DIAGNOSTICS
**	maximum is {n} substances
**
**		A maximum of {n} substances can be requested in any one run.
**
**	don't know refractive index for substance {subst}
**
**		The supported substances are currently "water" and "ice".
**
**	wavelength out of range (less than zero)
**
**		The wavelength given was less than or equal to zero.
**
**	wavelength {wl} out of range
**
**		The wavelength {wl} given is too high.
**
**	units {units} not supported
**
**		A list of the supported units can be obtained by a command
**		line such as: "refdx -s ice -u help".
**
** RESTRICTIONS
**	Only water and ice are currently supported substances.
**
** FUTURE DIRECTIONS
**	Other substances should be added.
**
** HISTORY
**	1/25/89	 Written by Jeff Dozier, UCSB.
**	8/15/93	 Converted to ANSI C.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
*/

#include "ipw.h"
#include "getargs.h"
#include "pgm.h"

#define NSUBST		4
#define TEMP_DEFAULT	(-5.)

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_r = {
		'r', "real part only (default both)"
	};

	static OPTION_T opt_i = {
		'i', "imaginary part only (default both)"
	};

	static OPTION_T opt_d = {
		'd', "dielectric function instead of refractive index"
	};

	static OPTION_T opt_s = {
		's', "{ ice, water }",
		STR_OPTARGS, "substance",
		REQUIRED, 1,
	};

	static OPTION_T opt_t = {
		't', "temperature, deg C (default -5)",
		REAL_OPTARGS, "temp",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_u = {
		'u', "wavelength units: (-u help for choices)",
		STR_OPTARGS, "units",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_r,
		&opt_i,
		&opt_s,
		&opt_t,
		&opt_u,
		&opt_d,
		0
	};

	COMPLEX_T      *((*func[NSUBST]) ());
	double         EXFUN( (*xform), (double));

	COMPLEX_T      *m;		/* -> refractive index	 */
	COMPLEX_T       _m;		/* (storage for m)	 */
	bool_t          dielectric;	/* ? dielectric fn	 */
	bool_t          imagpart;	/* ? print imag part	 */
	bool_t          realpart;	/* ? print real part	 */
	char           *subst;		/* -> substance name	 */
	char           *units;		/* units of wave meas	 */
	char            lambda[64];	/* wave storage vector	 */
	double          k;		/* imag part		 */
	double          n;		/* real part		 */
	double          temp;		/* temperature (C)	 */
	double          z;		/* scaling factor	 */
	int             i;		/* index		 */
	int             n_Subst;	/* # substances		 */

	ipwenter(argc, argv, optv,
		 "refractive index information, wavelengths from standard input");

	m = &_m;

 /* Parse Options */

 /*** units for input wavelength data ***/
	units = (n_args(opt_u) <= 0) ? NULL : str_arg(opt_u, 0);
	wavechoice(units, &xform);

 /*** real and/or imaginary parts ***/
	realpart = got_opt(opt_r);
	imagpart = got_opt(opt_i);
	if (!realpart && !imagpart) {
		realpart = imagpart = 1;
	}

 /*** dielectric function instead of refractive index? ***/
	dielectric = got_opt(opt_d);

 /*** which substances? ***/
	n_Subst = n_args(opt_s);
	if (n_Subst > NSUBST) {
		error("maximum is %d substances", NSUBST);
	}
	for (i = 0; i < n_Subst; i++) {
		subst = str_arg(opt_s, i);
		if (streq(subst, "ice")) {
			func[i] = refice;
		}
		else if (streq(subst, "water")) {
			func[i] = refh2o;
		}

 /* add other substances here as we incorporate them */

		else {
			error("don't know refractive index for substance %s",
			      subst);
		}
	}

 /*** temperature ***/
	temp = (n_args(opt_t) == 0) ? TEMP_DEFAULT : real_arg(opt_t, 0);

 /*** main loop ***/
	while (scanf("%s", lambda) != EOF) {
		z = (xform != NULL) ? (*xform) (atof(lambda)) : atof(lambda);
		printf("%s", lambda);
		for (i = 0; i < n_Subst; i++) {
			m = (*func[i]) (m, z, temp);
			n = m->re;
			k = m->im;
			if (dielectric) {
				if (realpart)
					printf("\t%.4f", (n - k) * (n + k));
				if (imagpart)
					printf("\t%.3e", 2 * n * k);
			}
			else {
				if (realpart)
					printf("\t%.4f", n);
				if (imagpart)
					printf("\t%.3e", k);
			}
		}
		putchar('\n');
	}

	exit(0);
}

#ifndef lint
static char     rcsid[] = "$Header: main.c,v 1.5 89/01/25 07:42:31 dozier Exp $";

#endif
