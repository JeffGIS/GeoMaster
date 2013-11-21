/*
** NAME
**	dew_point - dew point temperature (deg K)
** 
** SYNOPSIS
**	double	dew_point(e)
**	double	e;
** 
** DESCRIPTION
**	returns dew point temperature (deg K) for given vapor pressure
**	input --
**	e	vapor pressure (Pa)
** 
** RESTRICTIONS
** 
** RETURN VALUE
** 
** GLOBALS ACCESSED
** 
** ERRORS
** 
** WARNINGS
** 
** APPLICATION USAGE
** 
** HISTORY
** 	April 1989:	fixed to accept saturation over ice.
**			D. Marks, USEPA, Corvallis, OR
*/

#include "ipw.h"
#include "erlc.h"
#include "physdefs.h"

static	double	e_pass;

double
DEFUN( dew_point, (e),
    double    e)
{
	double	a;
	double	b;
	double	result;
	double	tol;

	if (e < 0 || e > 1.5*SEA_LEVEL) {
		error ("dew_point: vapor pressure = %g", e);
	}

	/* select starting guesses to span root */

	/* lower */
	a = FREEZE;
	while (e < sati(a))
		a *= .75;

	/* upper */
	b = FREEZE + 15;
	while (e > sati(b))
		b *= 1.25;

	e_pass = e;
	tol = 0;
	result = zerobr(a, b, tol, satm);

	return(result);
}

double
DEFUN( satm, (t),
    double    t)
{
	return(e_pass - sati(t));
}
