
#include <math.h>
#include "ipw.h"
#include "erlc.h"
#include "physdefs.h"
#include "physmacs.h"

/******************************************************************
**
**  NAME
**	brutsaert - calculates incoming longwave radiation
**
**  SYNOPSIS
**	double	brutsaert(ta, lambda, ea, z, pa)
**	double	ta;
**	double	lambda;
**	double	ea;
**	double	z;
**	double	pa;
**
**  DESCRIPTION
**	calculates atmospheric emissivity as a function of:
**
**		ta	air temp (K)
**		lambda	temperature lapse rate (deg/m)
**			(negative!)
**		ea	vapor pressure (Pa)
**		z	elevation (m)
**		pa	air pressure (Pa)
**
**	using a modified form of the equations by W. Brutsaert 
**	(See "On a derivable formula for long-wave radiation
**	from clear skies", Water Resources Research, vol 11,
**	no 5, pp 742-744, 1975).
**	(After Marks and Dozier, "A clear-sky longwave radiation
**	model for remote alpine areas" Archiv fur Meteorologie,
**	Geophysik und Bioklimatologie, Series B vol 27, no 23,
**	pp 159-187, 1979).
**
**  RESTRICTIONS
**
**  RETURN VALUE
**	returns atmospheric emissivity
**
**  GLOBALS ACCESSED
**
**  ERRORS
**	
**  WARNINGS
**	checks for vapor pressures > sat.;
**
**  APPLICATION USAGE
**
**  HISTORY
**	Dec. 1983:	Written by D. Marks, CSL/UCSB;
**	Oct. 1992:	Converted to IPW by D. Marks, USGS, EPA ERL/C;
**
**  FUTURE DIRECTIONS
**
**  BUGS
**
*/

double 
DEFUN( brutsaert, (ta, lambda, ea, z, pa),
    double   ta
AND double   lambda
AND double   ea
AND double   z
AND double   pa)
{
	double 	t_prime;
	double 	rh;
	double 	e_prime;
	double 	air_emiss;

	t_prime = ta - (lambda * z);
	rh = ea / sati(ta);
	if (rh > 1.0) {
		if (rh > 1.02)
			warn("ea=%lg > sat; RH set to 1.0",ea);
		rh = 1.0;
	}
	e_prime = (rh * sati(t_prime))/100.0;
/*	e_prime = rh * sati(t_prime);	*/

	air_emiss = (1.24*pow((e_prime/t_prime), 1./7.))*pa/SEA_LEVEL;
/* "if" statement below is new */
	if (air_emiss > 1.0) {
                air_emiss = 1.0;
        }

	return(air_emiss);
}
