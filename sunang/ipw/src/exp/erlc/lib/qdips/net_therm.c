/*
	returns net thermal radiation
	(after Marks and Dozier, 1979;)

	inputs:
		ta		air temperature (K)
		ts		surface temperature (K)
		pa		air pressure (Pa)
		ea		vapor pressure (Pa)
		surfemiss	surface emissivity (0.0 - 1.0)
		skyfac		terrain skyfactor (0.0 - 1.0)
		lapse		temp. lapse rate (deg/m)
				(negative up!)
		z		elevation (m)
	output:
		net		net thermal radiation (W/m**2)
*/

#include	"ipw.h"
#include	"erlc.h"
#include	"physdefs.h"
#include	"physmacs.h"

double
DEFUN(net_therm, (ta, ts, pa, ea, surfemiss, skyfac, lapse, z),
	double	ta
   AND	double	ts
   AND	double	pa
   AND	double	ea
   AND	double	surfemiss
   AND	double	skyfac
   AND	double	lapse
   AND	double	z)

{
	double	airemiss;
	double	in;
	double	landfac;
	double	landin;
	double	net;
	double	skyin;

	landfac = 1.0 - skyfac;
	
	airemiss = brutsaert(ta, lapse, ea, z, pa);

	skyin = (airemiss * STEFBO * (ta*ta*ta*ta)) * skyfac;
	landin = (surfemiss * STEFBO * (ts*ts*ts*ts)) * landfac;

	in = skyin + landin;

	net = surfemiss * (in - STEFBO * (ts*ts*ts*ts));

	return(net);
}
