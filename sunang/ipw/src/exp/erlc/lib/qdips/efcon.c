/*
** NAME
**	efcon - calculates equivalent thermal conductivity
**
** SYNOPSIS
**	#include "erlc.h"
**
**	double efcon (k, t, p)
**	double k;
**	double t;
**	double p;
**
** DESCRIPTION
**	Calculates equivalent thermal conductivity for a layer
**	accounting for both conduction and vapor diffusion.
**	Saturation within the layer is assumed.
**	After Anderson, 1976, pg. 46.
**	arguments:
**		k	layer thermal conductivity (J/(m K sec))
**		t	layer temperature (K)
**		p	air pressure (Pa)
**
** RETURN VALUE
**		etc	effective thermal conductivity (J/(m K sec))
**
** HISTORY
**	July, 1984:	written by D. Marks, CSL (GSFC), UCSB;
**	May 1995:	Ported to IPW.  J. Domingo, OSU, ERL-C
*/

#include	"ipw.h"
#include	"erlc.h"
#include	"ebdefs.h"
#include	"ebmacs.h"

double
DEFUN(efcon, (k, t, p),
	double	k
   AND	double	t
   AND	double	p)
{
	double	etc;
	double	de;
	double	lh;
	double	e;
	double	q;

	/*	calculate effective layer diffusion
		(see Anderson, 1976, pg. 32)		*/
	de = DIFFUS(p, t);

	/*	set latent heat from layer temp.	*/
	if(t > FREEZE)
		lh = LH_VAP(t);
	else if(t == FREEZE)
		lh = (LH_VAP(t) + LH_SUB(t)) / 2.0;
	else
		lh = LH_SUB(t);

	/*	set mixing ratio from layer temp.	*/
	e = sati(t);
	q = MIX_RATIO(e, p);

	/*	calculate effective layer conductivity	*/
	etc = k + (lh * de * q);

	return (etc);
}
