/*
** NAME
**	psychrom - solve psycrometric equation
**
** SYNOPSIS
**	#include "file.h"
**
**	psychrom(tdry, twet, press)
**	double	tdry;
**	double	twet;
**	double	press;
**
** DESCRIPTION
**	solve psycrometric equation using wet/dry bulb, air press data
**	calculates vapor pressure as function of wet & dry bulb temps;
**	works at all temperatures (accounts for both fusion and vaporization);
**
**	input--
**	tdry	dry bulb temp	(K)
**	twet	wet bulb temp	(K)
**	press	total pressure	(Pa)
**
**	output--
**		vapor pressure	(Pa)
**
** RESTRICTIONS
**
** RETURN VALUE
**	vapor pressure for success, ERROR for failure
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
**	1983  		written by J. Dozier, CSL;
**      1986		modified by D. Marks, CSL;
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include <math.h>
#include "ipw.h"
#include "erlc.h"
#include "physdefs.h"
#include "physmacs.h"

double
DEFUN( psychrom, (tdry, twet, press),
    double    tdry
AND double    twet
AND double    press)
{
	double	qa;
	double	qw;
	double	xlh;
	double	xlhv;
	double	xlhf;
	double	fu_fac;
	double	e;

	if (tdry <= 0. || twet <= 0. || press <= 0. || tdry < twet)
		error("tdry, twet, press negative or zero");

	/* find latent heat of vaporization, or vaporization + fusion */
	if (tdry <= FREEZE) {
		xlhv = LH_VAP((tdry + twet) / 2.0);
		xlhf = LH_FUS((tdry + twet) / 2.0);
		xlh  = xlhv + xlhf;
	}
	else if (twet <= FREEZE) {
		xlhv = LH_VAP((tdry + twet) / 2.0);
		xlhf = LH_FUS((FREEZE + twet) / 2.0);
		fu_fac = ((FREEZE - twet) / (tdry - twet));
		xlh  = xlhv + (fu_fac * xlhf);
	}
	else
		xlh = LH_VAP((tdry+twet)/2);

	/* saturation specific humidity at twet */
	e = sati(twet);
	qw = SPEC_HUM(e,press);

	/* specific humidity */
	qa = qw - (CP_AIR/xlh) * (tdry-twet);

	/* check result */
	if (qa < 0.0)
		error("combination of tdry, twet & press not possible");

	/* convert to vapor pressure */
	e = INV_SPH(qa,press);

	return(e);
}
