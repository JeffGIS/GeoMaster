/*
** NAME
**	budit2 -- Businger-Dyer model for turbulent transfer
**
** SYNOPSIS
**	int budit2(zu, zt, z0, t, t0, e, e0, u, p, h, le);
**	double zu;
**	double zt;
**	double z0;
**	double t;
**	double t0;
**	double e;
**	double e0;
**	double u;
**	double p;
**	double *h;
**	double *le;
**
** DESCRIPTION
**	Businger-Dyer model for turbulent transfer
**	given wind speed measurement at 1 height and temperature
**	and vapor pressure measurement at another
**
**	see Fleagle & Businger,
**	An Introduction to Atmospheric Physics (1980), ch. 6
**
**	input --
**	zu	height of wind speed measurement (m)
**	zt	height of temperature & vapor pressure measurements
**	z0	roughness length (m)
**	t	temp (K) at zt
**	t0	temp (K) at z0
**	e	vapor pressure at zt (Pa)
**	e0	vapor pressure at z0 (Pa)
**	u	wind speed (m/s) at zu
**	p	pressure (Pa)
**
**	output --
**	h	sensible heat exchange (W/m^2)
**	le	latent heat exchange (W/m^2)
**
** RETURN VALUE
**	function returns ERROR if vapor pressures reset so not to
**	exceed saturation; returns 0 on success.
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
**	6/95	Converted to IPW by J. Domingo, OSU, EPA NHEERL/WED.
**
** BUGS
**
** SEE ALSO
*/

#include	<math.h>

#include	"ipw.h"
#include	"physmacs.h"
#include	"physdefs.h"
#include	"erlc.h"

int
DEFUN(budit2, (zu, zt, z0, t, t0, e, e0, u, p, h, le),
	double	zu
   AND	double	zt
   AND	double	z0
   AND	double	t
   AND	double	t0
   AND	double	e
   AND	double	e0
   AND	double	u
   AND	double	p
   AND	double	*h
   AND	double	*le)
{
	double	cp, ustar, tstar, qstar, xlh, q, q0, aden;
	double	tgm, egm;
	int	rc;

	if (zu <= z0)
		error("budit2: zu<=z0");
	if (zt <= z0)
		error("budit2: zt<=z0");

	rc = 0;
	if (e > satw(t)) {
		rc = ERROR;
		e = satw(t);
		usrerr("budit2: e reset to saturation value");
	}
	if (e0 > sati(t0)) {
		rc = ERROR;
		e0 = sati(t0);
		usrerr("budit2: e0 reset to saturation value");
	}

	/* geometric mean t, e */
	tgm = sqrt(t*t0);
	egm = sqrt(e*e0);

	cp = CP_AIR;

	/* temperature-dependent latent heat of vaporization */
	xlh = LH_VAP(tgm);

	q = SPEC_HUM(e,p);
	q0 = SPEC_HUM(e0,p);

	/* air density based on virtual temperature at mean e */
	aden = GAS_DEN(p,MOL_AIR,VIRT(tgm,egm,p));

	/*
	 *  Previously, this function called the library function "bdf2":
	 *
	 *	\* friction velocity ustar, scaling temp tstar, scaling *\
	 *	\* specific humidity qstar *\
	 *	if (bdf2(zu,zt,z0,t,t0,q,q0,u, &ustar, &tstar, &qstar))
	 *		error("bdf2 failed in budit2");
	 *
	 *  But the "bdf2" function would simply called the library function
	 *  "bdfunc" with the same parameters that had been passed to bdf2.
	 *  Therefore, for simplicity (i.e., the avoid the need to port bdf2
	 *  from QDIPS to IPW), we just call bdfunc directly.  See the file
	 *  "bdf2.old" for details.
	 *  J. Domingo, June 1995
	 */
	bdfunc(zu, z0, t, t0, q, q0, u, 0., &ustar, &tstar, &qstar);

	*h = cp * aden * ustar * tstar;
	*le = aden * xlh * qstar * ustar;

	return(rc);
}
