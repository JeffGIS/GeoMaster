/*
** NAME
**	evap - calculates evaporation rate 
**
** SYNOPSIS
**	double evap (le, ts);
**	double le;
**	double ts;
**
** DESCRIPTION
**	Calculates evaporation or condensation rate as kg/(sec m**2),
**	which is the specific discharge in mm/sec;
**
**	arguments:
**		le	latent heat transfer (W/m**2)
**		ts	surface temperature (K)
**	
** RETURN VALUE
**		rate 	rate of water gain or loss (mm/sec)
**
** EXAMPLES
**
** GLOBALS ACCESSED
**
** DIAGNOSTICS
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	July, 1984	written by D. Marks, CSL (GSFC), UCSB;
**	June 1995	Converted to IPW by J. Domingo, OSU, EPA NHEERL/WED
**
** BUGS
**
** SEE ALSO
*/

#include "ipw.h"
#include "ebdefs.h"
#include "ebmacs.h"

double
DEFUN(evap, (le,ts),
	double	le
   AND	double	ts)
{
	double	rate;
	double	lh;

	if(ts > FREEZE)
		lh = LH_VAP(ts);
	else if(ts == FREEZE)
		lh = (LH_VAP(ts) + LH_SUB(ts)) / 2.0;
	else
		lh = LH_SUB(ts);

	rate = le / lh;

	return(rate);
}
