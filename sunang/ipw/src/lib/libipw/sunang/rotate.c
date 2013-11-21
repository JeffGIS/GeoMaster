/* LINTLIBRARY */

/*
**  NAME
**	rotate - rotation of spherical coordinates
**
**  SYNOPSIS
**	int	rotate(mu, azm, mu_r, lam_r, muPrime, aPrime)
**	double	mu, azm, mu_r, lam_r, *muPrime, *aPrime;
**
**  DESCRIPTION
**	Calculates new spherical coordinates if system rotated about
**	origin.  Coordinates are right-hand system.  All angles are in
**	radians.
**
**	input --
**	mu	cosine of angle theta from z-axis in old coordinate system
**	azm	azimuth (+ccw from x-axis) in old coordinate system
**	mu_r	cosine of angle theta_r of rotation of z-axis
**	lam_r	azimuth (+ccw) of rotation of x-axis
**
**	output --
**	muPrime	new value of cosine of angle from z'-axis
**	aPrime	new value of azimuth from x'-axis
**
**	for example, to use this program to calculate sun angles, set:
**
**	mu	= sin(declination)
**	azm	= hour angle of sun (long. where sun is vertical)
**	mu_r	= sin(latitude)
**	lam_r	= longitude
**
**	output will be muPrime = cosZ and aPrime = solar azimuth
**
**  DIAGNOSTICS
**	returns ERROR and writes message via usrerr if error in arguments
**
*/

#include <math.h>
#include <errno.h>
#include "ipw.h"
#include "sunang.h"

int
DEFUN(rotate, (mu, azm, mu_r, lam_r, muPrime, aPrime),
	double          mu
   AND  double          azm
   AND  double          mu_r
   AND  double          lam_r
   AND  double         *muPrime
   AND  double         *aPrime)
{
 /*
  * Note: theta = arc cos(mu)
  */
	double          cosOmega;	/* cos(omega)		 */
	double          omega;		/* lam_r - azm		 */
	double          sinTheta;	/* sin(theta)		 */
	double          sinThr;		/* sin(theta-sub-r)	 */
	static double   twopi;		/* 2 * pi		 */

 /*
  * calculate twopi on first pass
  */
	if (twopi == 0) {
		twopi = 8 * atan(1.);
	}

 /*
  * Check input values: mu = cos(theta); mu_r = cos(theta-sub-r)
  */
	if (mu < -1. || mu > 1.) {
		usrerr("rotate: mu = cos(theta) = %g", mu);
		return (ERROR);
	}
	if (mu_r < -1. || mu_r > 1.) {
		usrerr("rotate: mu_rotate = cos(theta-sub-r) = %g", mu_r);
		return (ERROR);
	}
	if (fabs(azm) > twopi) {
		usrerr("rotate: azimuth = %g deg", azm * 360 / twopi);
		return (ERROR);
	}
	if (fabs(lam_r) > twopi) {
		usrerr("rotate: lam_r = axis rotation = %g deg",
		       lam_r * 360 / twopi);
		return (ERROR);
	}
 /*
  * difference between azimuth and rotation angle of x-axis
  */
	omega = lam_r - azm;
 /*
  * set Unix error indicator before calling trig routines; check after
  * calling for trig error
  */
	errno = 0;
 /*
  * sine of angle theta (cosine is an argument);
  * sine of angle theta-sub-r (cosine is an argument);
  * bug if sqrt triggers error
  */
	sinTheta = sqrt((1. - mu) * (1. + mu));
	sinThr = sqrt((1. - mu_r) * (1. + mu_r));
//	assert(errno == 0);
 /*
  * cosine of difference between azimuth and aziumth of rotation
  */
	cosOmega = cos(omega);
 /*
  * Output:
  * azimuth and cosine of angle in rotated axis system.
  * (bug if trig routines trigger error)
  */
	*aPrime = -atan2(sinTheta * sin(omega),
			 mu_r * sinTheta * cosOmega - mu * sinThr);
	*muPrime = sinTheta * sinThr * cosOmega + mu * mu_r;
//	assert(errno == 0);

	return (OK);
}

#ifndef lint
static char     rcsid[] = "$Header: rotate.c,v 1.4 89/05/30 13:04:38 dozier Exp $";

#endif
