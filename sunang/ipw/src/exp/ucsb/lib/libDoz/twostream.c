/*
** NAME
**	twostream -- twostream solution for horizontal surface
**
** SYNOPSIS
**
**	int twostream(gamma, omega, mu0, tau, r0, refl, trans, btrans)
**	double *gamma;
**	double omega;
**	double mu0;
**	double tau;
**	double r0;
**	double *refl;
**	double *trans;
**	double *btrans;
**
** DESCRIPTION
**	Provides twostream solution for single-layer atmosphere over
**	horizontal surface, using solution method in:
**	W. E. Meador & W. R. Weaver, Two-stream approximations to
**	radiative transfer in planetary atmospheres: a unified
**	description of existing methods and a new improvement, J.
**	Atmos. Sci., 37, 630-643, 1980.
**
**	Input variables
**		gamma[4]	vector of gamma coefficients
**		omega		single-scattering albedo
**		mu0		cosine of incidence angle
**		tau		optical depth of layer
**		r0		reflectance of substrate
**	Output variables
**		refl		reflectance of layer
**		trans		total transmittance of layer
**				(compensated for mu0)
**		btrans		direct transmittance of layer
**				(not compensated for mu0)
**
** RESTRICTIONS
**
** RETURN VALUE
**	0 for success, -1 for failure, 1 if layer is semi-infinite
**
** ERRORS
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include <math.h>
#include "ipw.h"

#define SEMI	1

int
twostream(gamma, omega, mu0, tau, r0, refl, trans, btrans)
	double         *gamma;
	double          omega;
	double          mu0;
	double          tau;
	double          r0;
	double         *refl;
	double         *trans;
	double         *btrans;
{
	double          alph1;
	double          alph2;
	double          denrt;
	double          em;
	double          ep;
	double          et;
	double          gam1;
	double          gam2;
	double          gam3;
	double          gam4;
	double          gmx;
	double          gpx;
	double          omx;
	double          opx;
	double          rm;
	double          rp;
	double          xi;

 /*
  * intermediate variables
  */
	gam1 = gamma[0];
	gam2 = gamma[1];
	gam3 = gamma[2];
	gam4 = gamma[3];
	alph1 = gam1 * gam4 + gam2 * gam3;
	alph2 = gam2 * gam4 + gam1 * gam3;
	if (gam1 < gam2) {
		usrerr("gam1 (%g) < gam2 (%g)", gam1, gam2);
		return (ERROR);
	}
 /*
  * (Hack - gam1 = gam2 with conservative scattering.  Need to fix
  * this.
  */
	if (gam1 == gam2) {
		gam1 += DBL_EPSILON;
	}
	xi = sqrt((gam1 - gam2) * (gam2 + gam1));
	em = exp(-tau * xi);
	et = exp(-tau / mu0);
	ep = exp(tau * xi);
	gpx = xi + gam1;
	opx = mu0 * xi + 1;

 /*
  * semi-infinite?
  */
	if ((em == 0 && et == 0) || isinf(ep)) {
		*refl = omega * (gam3 * xi + alph2) / (gpx * opx);
		*btrans = *trans = 0;
		return (SEMI);
	}
	else {

 /*
  * more intermediate variables, needed only for finite case
  */
		omx = 1 - mu0 * xi;
		gmx = gam1 - xi;
		rm = gam2 - gmx * r0;
		rp = gam2 - gpx * r0;

 /*
  * denominator for reflectance and transmittance
  */
		denrt = ep * gpx * rm - em * gmx * rp;

 /*
  * reflectance
  */
		*refl = (omega * (ep * rm * (gam3 * xi + alph2) / opx
				- em * rp * (alph2 - gam3 * xi) / omx)
			 + 2 * et * gam2
			 * (r0 - ((alph1 * r0 - alph2) * mu0 + gam4
		    * r0 + gam3) * omega / (omx * opx)) * xi) / denrt;

 /*
  * transmittance
  */
		*trans = (et * (ep * gpx * (gam2 - omega * (alph2
						   - gam3 * xi) / omx)
				- em * gmx * (gam2 - omega * (gam3 * xi + alph2) / opx))
		      + 2 * gam2 * (alph1 * mu0 + gam4) * omega * xi /
			  (omx * opx)) / denrt;

 /*
  * direct transmittance
  */
		*btrans = et;

		assert(*refl >= 0);
		assert(*trans >= 0);
		assert(*btrans >= 0);
		assert(*trans >= *btrans * mu0);

		return (OK);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/dozier/src/src.lib/libD/RCS/twostream.c,v 1.4 88/10/16 23:29:06 dozier Exp Locker: dozier $";

#endif
