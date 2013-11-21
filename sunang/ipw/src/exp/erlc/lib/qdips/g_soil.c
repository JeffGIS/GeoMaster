/*
** NAME
**	g_soil - calcualtes heat flow between snow and soil
**
** SYNOPSIS
**	#include "erlc.h"
**
**	double g_soil(rho, tso, tg, ds, dg, pa)
**	double rho;
**	double tso;
**	double tg;
**	double ds;
**	double dg;
**	double pa;
**
** DESCRIPTION
**	Calculates heat flow between snow layer and soil accounting
**	for both conduction and vapor transport;
**	(After Anderson, 1976, pg. 45-47;)
**
**	arguments:
**		rho	snow layer density (kg/m**3)
**		tsno	snow layer temperature (K)
**		tg	soil temperature (K)
**		ds	snow layer thickness (m)
**		dg	depth of soil temp. measurement (m)
**		pa	air pressure (Pa)
**	returns:
**		g	heat transfer between soil and snow (J/m**2)
**
** HISTORY
**	August, 1984:  written by D. Marks, CSL (GSFC), UCSB;
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
*/

#include "ipw.h"
#include "erlc.h"
#include "ebdefs.h"
#include "ebmacs.h"

double
DEFUN(g_soil, (rho, tsno, tg, ds, dg, pa),
	double	rho
   AND	double	tsno
   AND	double	tg
   AND	double	ds
   AND	double	dg
   AND	double	pa)
{
	double	k_g;
	double	kcs;
	double	k_s;
	double	g;

/*	check tsno	*/
	if (tsno > FREEZE) {
		warn("tsno = %8.2f; set to 273.16\n", tsno);
		tsno = FREEZE;
	}

/*	set effective soil conductivity	*/
	k_g = efcon (KT_WETSAND, tg, pa);

/*	calculate G	*/
	/*	set snow conductivity	*/
	kcs = KTS(rho);
	k_s = efcon (kcs, tsno, pa);

	g = ssxfr (k_s, k_g, tsno, tg, ds, dg);

	return (g);
}
