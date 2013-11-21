/* LINTLIBRARY */

#include "ipw.h"
/*
** NAME
**	delted -- delta-Eddington transformation
**
** SYNOPSIS
**	delted(omega, g, tau)
**	double *omega, *g, *tau;
**
** DESCRIPTION
**	Delted computes the delta-Eddington transformations of its
**	arguments.
**
** RESTRICTIONS
**
** RETURN VALUE
**
** GLOBALS ACCESSED
**
** ERRORS
**	Dies with message on error.
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

void
delted(omega, g, tau)
	double         *omega;		/* single-scattering albedo	 */
	double         *g;		/* scattering asymmetry param	 */
	double         *tau;		/* optical depth		 */
{
	double          gp1;		/* g + 1			 */
	double          gstar;		/* g sup star			 */
	double          omg;
	double          ostar;		/* omega sup star		 */

	if (*omega < 0 || *omega > 1)
		error("omega = %g", *omega);
	if (*g < 0 || *g > 1)
		error("g = %g", *g);
	if (*tau <= 0)
		error("tau = %g", *tau);

	gp1 = 1 + *g;
	gstar = *g / gp1;
	omg = 1 - *g * *g * *omega;
	ostar = ((1 - *g) * gp1 * *omega) / omg;

	*tau *= omg;
	*omega = ostar;
	*g = gstar;
}

#ifndef	lint
static char     rcsid[] = "$Header: delted.c,v 1.2 88/03/28 12:53:40 dozier Exp $";

#endif
