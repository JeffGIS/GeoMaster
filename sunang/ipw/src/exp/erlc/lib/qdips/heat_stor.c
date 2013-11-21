/*
** NAME
**	heat_stor - calculates change in heat storage in a layer
**
** SYNOPSIs
**	#include	"erlc.h"
**
**	double heat_stor (cp, spm, tdif)
**
** DESCRIPTION
**	Calculates change in heat storage in a homogeneous layer
**	as a function of the change in average layer temperature.
**	arguments:
**		cp	specific heat of layer (J/kg K)
**		spm	layer specific mass (kg/m**2) -
**			(thickness (m) * density (kg/m**3))
**		tdif	temperature change (K)
**	returns:
**		stor	heat storage (J/m**2)
**
** HISTORY
**	August, 1984:  	written by D. Marks, CSL (GSFC), UCSB;
**	May 1995	Ported to IPW.  J. Domingo, OSU, ERL-C
*/

#include "ipw.h"

double
DEFUN(heat_stor, (cp, spm, tdif),
	double	cp
   AND	double	spm
   AND	double	tdif)
{
	double	stor;

	stor = cp * spm * tdif;

	return (stor);
}
