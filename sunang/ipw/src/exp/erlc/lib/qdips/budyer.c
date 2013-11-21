/*
** NAME
**      budyer -- Businger-Dyer model for turbulent transfer
**
** SYNOPSIS
**      int budyer(z, z0, t, t0, e, e0, u, u0, p, h, le);
**      double z;
**      double z0;
**      double t;
**      double t0;
**      double e;
**      double e0;
**      double u;
**      double u0;
**      double p;
**      double *h;
**      double *le;
**
** DESCRIPTION
**	Businger-Dyer model for turbulent transfer
**	see Fleagle & Businger,
**	An Introduction to Atmospheric Physics (1980), ch. 6
**
**	z	upper height of wind speed & temp measurement (m)
**	z0	lower height of wind speed & temp measurement
**		(may be roughness length)
**	t	temp (K) at z
**	t0	temp (K) at z0
**	e	vapor pressure at z (Pa)
**	e0	vapor pressure at z0 (Pa)
**	u	wind speed (m/s) at z
**	u0	wind speed (m/s) at z0 (may be zero)
**	p	pressure (Pa)
**	h	sensible heat exchange (W/m^2)
**	le	latent heat exchange (W/m^2)
**
** RETURN VALUE
**	function returns OK if all ok, returns ERROR if e or e0
**	exceeds saturation vapor pressure at appropriate temperature
**	(fixes these and proceeds with calculation)
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
**      6/95    Converted to IPW by J. Domingo, OSU, EPA NHEERL/WED
**
** BUGS
**
** SEE ALSO
*/


#include	<math.h>

#include	"ipw.h"
#include	"erlc.h"
#include	"physmacs.h"
#include	"physdefs.h"

int
DEFUN(budyer, (z, z0, t, t0, e, e0, u, u0, p, h, le),
	double	z
    AND	double	z0
    AND	double	t
    AND	double	t0
    AND	double	e
    AND	double	e0
    AND	double	u
    AND	double	u0
    AND	double	p
    AND	double	*h
    AND	double	*le)
{
	double	cp, xlh,  q, q0, aden, esat, ustar, tstar, qstar;
	int	rc;

	rc = 0;
	cp = CP_AIR;

	/* latent heat of vaporization at geometric mean temp */
	xlh = LH_VAP(sqrt(t*t0));

	/* check vapor pressures, reset if > saturation */
	esat = satw(t);
	if (e > esat) {
		rc = ERROR;
		e = esat;
	}
	esat = sati(t0);
	if (e0 > esat) {
		rc = ERROR;
		e0 = esat;
	}

	q = SPEC_HUM(e,p);
	q0 = SPEC_HUM(e0,p);

	/* air density based on virtual temperature at mean e */
	aden = GAS_DEN(p,MOL_AIR,VIRT(sqrt(t*t0),sqrt(e*e0),p));

	/* friction velocity ustar, scaling temp tstar, scaling */
	/* specific humidity qstar */
	bdfunc(z,z0,t,t0,q,q0,u,u0, &ustar, &tstar, &qstar);

	*h = aden * cp * ustar * tstar;
	*le = aden * xlh * ustar * qstar;

	return(rc);
}
