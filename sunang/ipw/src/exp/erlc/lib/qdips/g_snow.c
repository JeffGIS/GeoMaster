/*
** NAME
**	g_snow -- calcualtes heat flow between two snow layers
**
** SYNOPSIS
**	#include "erlc.h"
**
**	double g_snow(rho1, rho2, ts1, ts2, ds1, ds2, pa)
**	double rho1;
**	double rho2;
**	double ts1;
**	double ts2;
**	double ds1;
**	double ds2;
**	double pa;
**
** DESCRIPTION
**	Calculates heat flow between two snow layers
**	for both conduction and vapor transport;
**	(After Anderson, 1976, pg. 46-47;)
**
**	arguments:
**		rho1	upper snow layer density (kg/m**3)
**		rho2	lower snow layer density (kg/m**3)
**		ts1	upper snow layer temperature (K)
**		ts2	lower snow layer temperature (K)
**		ds1	upper snow layer thickness (m)
**		ds2	lower snow layer thickness (m)
**		pa	air pressure (Pa)
**	returns:
**		g	heat transfer between snow layers (J/m**2)
**
** HISTORY
**	August, 1986:	written by D. Marks, CSL, UCSB;
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
*/

#include "ipw.h"
#include "erlc.h"
#include "ebdefs.h"
#include "ebmacs.h"

double
DEFUN(g_snow, (rho1, rho2, ts1, ts2, ds1, ds2, pa),
	double	rho1
   AND	double	rho2
   AND	double	ts1
   AND	double	ts2
   AND	double	ds1
   AND	double	ds2
   AND	double	pa)
{
	double	kcs1;
	double	kcs2;
	double	k_s1;
	double	k_s2;
	double	g;


/*	calculate G	*/
	if (ts1 == ts2)
		g = 0.0;
	else {
	/*	set snow conductivity	*/
		kcs1 = KTS(rho1);
		kcs2 = KTS(rho2);
		k_s1 = efcon (kcs1, ts1, pa);
		k_s2 = efcon (kcs2, ts2, pa);

	/*	calculate g	*/
		g = ssxfr (k_s1, k_s2, ts1, ts2, ds1, ds2);
	}

	return (g);
}
