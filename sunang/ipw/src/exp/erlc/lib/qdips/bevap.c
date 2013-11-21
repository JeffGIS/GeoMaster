/*
** NAME
**	bevap - evaporation from Bowen ratio
**
** SYNOPSIS
**	#include "erlc.h"
**
**	double bevap (netad, advec, bowen, storage, ts)
**	double netad;
**	double advec;
**	double bowen;
**	double storage;
**	double ts;
**
** DESCRIPTION
**	returns evaporation rate in kg/(sec m**2),
**	which is the specific discharge in mm/sec,
**	a saturated surface using the Bowen ratio
**	technique.
**
**	arguments:
**		netrad	net allwave radiation (W/m**2)
**		advec	advected heat	(W/m**2)
**		storage	change in heat storage (W/m**2)
**		bowen	Bowen Ratio (H/LE)
**		ts	surface temperature (K)
**
** RETURN VALUE
**		rate	evaporation (mm/sec)
**
** ERRORS
**
** HISTORY
**	April, 1984:  written by D. Marks, CSL, UCSB;
**
*/

#include	"ipw.h"
#include	"erlc.h"
#include	"physdefs.h"
#include	"physmacs.h"

double
DEFUN(bevap, (netrad, advec, bowen, storage, ts),
	double		netrad
   AND	double		advec
   AND	double		bowen
   AND	double		storage
   AND	double		ts)
{
	double 	le, rhoe, rate;

	/*	le = latent heat of vaporization at ta (J/kg)	*/
	/*	rhoe = density of liquid water at 25 C (kg/m^3)	*/

	le = LH_VAP(ts);

	rhoe = 997.07;

	rate = (netrad + advec + storage)/(rhoe * le * (1.0 + bowen));

	return(rate);
}
