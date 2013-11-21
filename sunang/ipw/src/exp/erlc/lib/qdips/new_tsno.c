/*
** NAME
**	new_tsno -- calculates new layer temperature
**
** SYNOPSIS
**	#include	"erlc.h"
**
**	double new_tsno (spm, t0, ccon)
**	double spm;
**	double t0;
**	double ccon;
**
** DESCRIPTION
**	Calculates a new layer temperature from adjusted cold content,
**	and last layer temperature.
**	arguments:
**		spm	layer specific mass (kg/m**2) -
**			(thickness (m) * density (kg/m**3))
**		t0	last snow layer temperature (K)
**		ccon	adjusted snow layer cold content (J/m**2)
**	returns:
**		tsno	new snow layer temperature (K)
**
** HISTORY
**	August, 1984:   written by D. Marks, CSL (GSFC), UCSB;
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
*/

#include "ipw.h"
#include "txtio.h"
#include "ebdefs.h"
#include "ebmacs.h"

double
DEFUN(new_tsno, (spm, t0, ccon),
	double	spm
   AND	double	t0
   AND	double	ccon)
{
	double	tsno;
	double	cp;
	double	tdif;

	cp = CP_ICE(t0);

	tdif = ccon / (spm * cp);
	tsno = tdif + FREEZE;

	return (tsno);
}
