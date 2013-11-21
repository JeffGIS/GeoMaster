/*
** NAME
**	ssxfr - calculates steady state heat transfer
**
** SYNOPSIS
**	#include "erlc.h"
**
**	double ssxfr (k1, k2, t1, t2, d1, d2)
**	double	k1, k2;
**	double	t1, t2;
**	double	d1, d2;
**
** DESCRIPTION
**	calculates steady state heat transfer between two layers;
**
**	arguments:
**		k1,k2	thermal conductivities (J / (m K sec))
**		t1,t2	average layer temperature (K)
**		d1,d2	layer thickness (m)
**
** RETURN VALUE
**		xfr	heat transfer between layers (W/m**2)
**
** HISTORY
**	July, 1984	written by D. Marks, CSL (GSFC), UCSB;
**	May 1995	Ported to IPW.  J. Domingo, OSU, ERL-C
*/

#include "ipw.h"
#include "erlc.h"

double
DEFUN(ssxfr, (k1, k2, t1, t2, d1, d2),
	double	k1
   AND	double	k2
   AND	double	t1
   AND	double	t2
   AND	double	d1
   AND	double	d2)
{
	double	xfr;

	xfr = 2.0 * (k1 * k2 * (t2 - t1)) / ((k2 * d1) + (k1 * d2));

	return (xfr);
}
