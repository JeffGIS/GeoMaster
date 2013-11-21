/*
** NAME
**	bowen -- returns Bowen ratio (H/LE)
**
** SYNOPSIS
**	#include "erlc.h"
**
**	double bowen (p, ta, ts, ea, es)
**	double p;
**	double ta;
**	double ts;
**	double ea;
**	double es;
**
** DESCRIPTION
**
**	Returns the Bowen ratio
**
**	input --
**	p	air pressure
**	ta	air temperature, K (potential)
**	ts	surface temperature, K (potential)
**	ea	vapor pressure
**	es	vapor pressure at surface
**
** NOTES
**	p, ea, and es must be in same units
*/

#include	"ipw.h"
#include	"erlc.h"
#include	"physdefs.h"
#include	"physmacs.h"

double
DEFUN(bowen, (p,ta,ts,ea,es),
	double	p
   AND	double	ta
   AND	double  ts
   AND  double  ea
   AND	double	es)
{
	double	cb, tmean;

	tmean = (ta + ts)/2.;
	cb = p * CP_AIR / ((MOL_H2O/MOL_AIR)*LH_VAP(tmean));

	if (ea == es)
		error("ea = es = %lg",ea);

	return(cb*(ta-ts)/(ea-es));
}
