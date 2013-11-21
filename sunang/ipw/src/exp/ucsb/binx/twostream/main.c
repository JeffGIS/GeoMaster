/*
** NAME
**	twostream -- twostream radiative transfer model of homogeneous layer
**
** SYNOPSIS
**	twostream [-u cos] [-t tau] [-w omega] [-g g] [-r R0] [-s S0] [-d]
**
** DESCRIPTION
**	Twostream calculates reflectance and transmittance of a
**	homogeneous single-layer medium with specified boundary
**	conditions.  Arguments not read from command line are read from
**	standard input, in the same order as the arguments appear in
**	the command line message.  All arguments and standard input
**	values are echoed to standard output.
**
** OPTIONS
**	-u	The cosine of the incidence angle is {cos} (default: read
**		value from the standard input).
**
**	-t	The optical depth is {tau} (default: read value from the
**		standard input).  0 implies an infinite optical depth.
**
**	-w	The single-scattering albedo is {omega} (default: read value
**		from the standard input).
**
**	-g	The asymmetry factor is {g} (default: read value from the
**		standard input).
**
**	-r	The reflectance of the substrate is {R0} (default: read value
**		from the standard input).  If {R0} is negative, it will be
**		set to zero.
**
**	-s	The direct beam irradiance is {S0} (default: read value from
**		the standard input).  If {S0} is negative, it will be set to
**		1/{cos}, or 1 if {cos} is not specified.
**
**	-d	The delta-Eddington method will be used (default: use the
**		Meador-Weaver hybrid method).
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**	u0 must be > 0 and <= 1
**
**		{cos} must be greater than 0, and less than or equal to 1.
**
**	betanaught: no convergence - u0=1 g=1 sum= {. . .}
**
**		The betanaught procedure was unable to get the Legendre
**		polynomial to converge.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	1/25/89	Written by Jeff Dozier, UCSB.
**
** BUGS
**
** SEE ALSO
*/

#include <math.h>
#include "ipw.h"
#include "getargs.h"
#include "Dozier.h"

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_u = {
		'u', "cosine of incidence angle",
		REAL_OPTARGS, "cos",
		OPTIONAL, 1, 1
	};
	static OPTION_T opt_o = {
		'w', "single-scattering albedo",
		REAL_OPTARGS, "omega",
		OPTIONAL, 1, 1
	};
	static OPTION_T opt_t = {
		't', "optical depth (0 -> infinite)",
		REAL_OPTARGS, "tau",
		OPTIONAL, 1, 1
	};
	static OPTION_T opt_g = {
		'g', "asymmetry factor",
		REAL_OPTARGS, "g",
		OPTIONAL, 1, 1
	};
	static OPTION_T opt_r = {
		'r', "reflectance of substrate (if <= 0, set to 0)",
		REAL_OPTARGS, "R0",
		OPTIONAL, 1, 1
	};
	static OPTION_T opt_s = {
		's', "direct beam irradiance (if <= 0, set to mu_0 * S0 = 1)",
		REAL_OPTARGS, "S0",
		OPTIONAL, 1, 1
	};
	static OPTION_T opt_m = {
		'd', "delta-Eddington method (Meador-Weaver hybrid is default)"
	};
	static OPTION_T *optv[] = {
		&opt_u,
		&opt_t,
		&opt_o,
		&opt_g,
		&opt_r,
		&opt_s,
		&opt_m,
		0
	};
	double          g;		/* asymmetry parameter		 */
	double          ga;		/* asymmetry parameter		 */
	double          gam[4];		/* scattering function params	 */
	double          rho0;		/* substrate reflectance	 */
	double          s0;		/* solar illumination		 */
	double          tau0;		/* optical depth		 */
	double          t0;		/* optical depth		 */
	double          u0;		/* cos (illum angle)		 */
	double          omega;		/* single-scat albedo		 */
	double          wa;		/* single-scat albedo		 */
	double          refl;		/* reflectance (output)		 */
	double          trans;		/* transmittance (output)	 */
	double          btrans;		/* direct transmittance (out)	 */
	bool_t          got_omega;	/* ? omega from command line	 */
	bool_t          got_asymm;	/* ? g from command line	 */
	bool_t          got_tau0;	/* ? tau0 from command line	 */
	bool_t          got_mu0;	/* ? u0 from command line	 */
	bool_t          got_r0;		/* ? rho0 from command line	 */
	bool_t          got_s0;		/* ? s0 from command line	 */
	bool_t          mw;		/* ? Meador-Weaver hybrid	 */

	ipwenter(argc, argv, optv,
	   "twostream radiative transfer model of homogeneous layer");

/*
 * which arguments on command line?
 */
	got_omega = got_opt(opt_o);
	got_asymm = got_opt(opt_g);
	got_tau0 = got_opt(opt_t);
	got_mu0 = got_opt(opt_u);
	got_r0 = got_opt(opt_r);
	got_s0 = got_opt(opt_s);
	mw = !got_opt(opt_m);
/*
* get arguments from command line
*/
	if (got_mu0) {
		u0 = real_arg(opt_u, 0);
		if ( (u0 <= 0) || (u0 > 1) ) {
			error("u0 must be > 0 and <= 1");
		}
	}
	if (got_tau0) {
		tau0 = real_arg(opt_t, 0);
		if (tau0 <= 0)
			tau0 = HUGE_VAL;
	}
	if (got_omega)
		omega = real_arg(opt_o, 0);
	if (got_asymm)
		g = real_arg(opt_g, 0);
	if (got_r0) {
		rho0 = real_arg(opt_r, 0);
		if (rho0 < 0)
			rho0 = 0;
	}
	if (got_s0) {
		s0 = real_arg(opt_s, 0);
		if (s0 <= 0)
			s0 = (got_mu0 && u0 != 0) ? 1 / u0 : 1;
	}

 /*
  * delta-Eddington scaling
  */
	if (got_omega && got_asymm && got_tau0) {
		wa = omega;
		ga = g;
		t0 = tau0;
		if (!mw)
			delted(&wa, &ga, &t0);
	}

 /*
  * gamma's for phase function for all input; 0 = delta-Edd option; 1 =
  * M-W hybrid
  */
	if (got_omega && got_mu0 && got_asymm && got_tau0)
		mwgamma(u0, wa, ga, gam, mw);

/*
* just one problem if all input from command line
*/
	if (got_omega && got_asymm && got_tau0 && got_mu0 && got_r0 && got_s0) {

 /*
  * solution to twostream
  */
		if (twostream(gam, wa, u0, t0, rho0, &refl, &trans, &btrans)
		    == ERROR) {
			error("twostream error");
		}
 /*
  * output
  */
		printf("reflectance %g\n", refl);
		printf("transmittance %g\n", trans);
		printf("direct transmittance %g\n", btrans);
		printf("upwelling irradiance %g\n", refl * u0 * s0);
		printf("total irradiance at bottom %g\n", trans * u0 * s0);
		printf("direct irradiance normal to beam %g\n", btrans * s0);
	}

 /*
  * otherwise some input from stdin
  */
	else {
 /*
  * output order
  */
		if (isatty(fileno(stdout))) {
			printf("mu_0, tau_0, omega_0, g, rho_0, S_0");
			printf(", refl, trans, btrans");
			printf(", up flux top, down flux bot, dir bot\n");
		}

		for (;;) {
 /*
  * read values from stdin
  */
			if (!got_mu0) {
				if (scanf("%lf", &u0) == EOF)
					break;
			}
			if (!got_tau0) {
				if (scanf("%lf", &tau0) == EOF)
					break;
			}
			if (!got_omega) {
				if (scanf("%lf", &omega) == EOF)
					break;
			}
			if (!got_asymm) {
				if (scanf("%lf", &g) == EOF)
					break;
			}
			if (!got_r0) {
				if (scanf("%lf", &rho0) == EOF)
					break;
			}
			if (!got_s0) {
				if (scanf("%lf", &s0) == EOF)
					break;
			}
 /*
  * echo input
  */
			printf("%g %g %g %g %g %g",
			       u0, tau0, omega, g, rho0, s0);
 /*
  * delta-Eddington scaling
  */
			if (!got_omega || !got_asymm || !got_tau0) {
				t0 = tau0;
				wa = omega;
				ga = g;
				if (!mw)
					delted(&wa, &ga, &t0);
			}
 /*
  * gamma's for phase function
  */
			if (!got_omega || !got_mu0 || !got_asymm || !got_tau0)
				mwgamma(u0, wa, ga, gam, mw ? 1 : 0);
 /*
  * solution to twostream
  */
			if (twostream(gam, wa, u0, t0, rho0,
				   &refl, &trans, &btrans) == ERROR) {
				error("twostream error");
			}
 /*
  * print output
  */
			printf(" %g %g %g %g %g %g\n",
			       refl, trans, btrans,
			       refl * u0 * s0,
			       trans * u0 * s0,
			       btrans * s0);
		}
	}

	exit(0);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.5 89/01/25 11:31:50 dozier Exp $";

#endif
