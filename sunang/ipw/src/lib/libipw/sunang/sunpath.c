/* LINTLIBRARY */

/*
**  NAME
**	sunpath - calculates sun angle
**
**  SYNOPSIS
**	int	sunpath(lat, lon, declin, omega, cosZ, azm)
**	double	lat, lon, declin, omega, *cosZ, *azm;
**
**  DESCRIPTION
**	Calculates sun angle on horizontal surface for point on Earth's
**	surface.
**
**	input --
**	lat	latitude of point, radians
**	lon	longitude of point, radians
**	declin	declination, radians
**	omega	longitude of solar subpoint, radians
**
**	output --
**	cosZ	cosine of solar zenith angle
**	azm	solar azimuth, radians; 0 -> south, positive to east
**
**  DIAGNOSTICS
**	returns 1 if sun below horizon and cosZ is negative
**	Returns ERROR and writes message via usrerr if error in arguments
**
*/

#include <math.h>
#include "ipw.h"
#include "sunang.h"

#define MAX_DECLIN	(23.6)

int
DEFUN(sunpath, (lat, lon, declin, omega, cosZ, azm),
	double          lat		/* latitude (radians)		 */
   AND  double          lon		/* longitude (radians)		 */
   AND  double          declin		/* solar declination (radians)	 */
   AND  double          omega		/* solar longitude (radians)	 */
   AND  double         *cosZ		/* cosine of solar zenith	 */
   AND  double         *azm)		/* solar azimuth (radins)	 */
{
	static double   pio2;		/* pi over 2			 */

 /*
  * get value of pio2 on first pass
  */
	if (pio2 == 0) {
		pio2 = asin(1.);
	}

 /*
  * Check arguments: latitude must be between +/- 90 degrees, longitude
  * between +/- 180 degrees, declination between +/- MAX_DECLIN
  * (defined above), and solar longitude between +/- 180 degrees. If
  * outside range, write appropriate error message via usrerr() and
  * return (ERROR).
  */
	if (fabs(lat) > pio2) {
		usrerr("sunpath: latitude=%g deg",
		       lat * 90 / pio2);
		return (ERROR);
	}
	if (fabs(lon) > 2 * pio2) {
		usrerr("sunpath: longitude=%g deg",
		       lon * 90 / pio2);
		return (ERROR);
	}
	if (fabs(declin) > MAX_DECLIN * pio2 / 90) {
		usrerr("sunpath: declination=%g deg",
		       declin * 90 / pio2);
		return (ERROR);
	}
	if (fabs(omega) > 2 * pio2) {
		usrerr("sunpath: longitude of sun = %g deg",
		       omega * 90 / pio2);
		return (ERROR);
	}
 /*
  * Use general axis rotation routine to compute sun angle.
  */
	if (rotate(sin(declin), omega, sin(lat), lon, cosZ, azm) == ERROR) {
		return (ERROR);
	}
 /*
  * set flag if sun is below horizon
  */
	if (*cosZ < 0.) {
		return (1);
	}
 /*
  * normal return
  */
	return (0);
}

#ifndef lint
static char     rcsid[] = "$Header: sunpath.c,v 1.3 89/05/30 13:02:39 dozier Exp $";

#endif
