/*
** NAME
**	bdfunc -- calculates parameters needed to evaluate sensible
**		  and latent heat exchange
**
** SYNOPSIS
**	void bdfunc(z, z0, t, t0, q, q0, u, u0, ustar, tstar, qstar);
**	double z;
**	double z0;
**	double t;
**	double t0;
**	double q;
**	double q0;
**	double u;
**	double u0;
**	double *ustar;
**	double *tstar;
**	double *qstar;
**
** DESCRIPTION
**	input	--
**	z, z0	heights of measurements (z0 may be roughness length)
**	t, t0	temperatures at z, z0
**	q, q0	specific humidities at z, z0
**	u, u0	wind speeds at z, z0 (u0 may be zero)
**
**	output	--
**	ustar	friction velocity
**	tstar	scaling temperature
**	qstar	scaling humidity
**
** RETURN VALUE
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
**	6/95	Converted to IPW by J. Domingo, OSU, EPA NHEERL/WED
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

void
DEFUN(bdfunc, (z, z0, t, t0, q, q0, u, u0, ustar, tstar, qstar),
	double z
   AND	double z0
   AND	double t
   AND	double t0
   AND	double q
   AND	double q0
   AND	double u
   AND	double u0
   AND	double *ustar
   AND	double *tstar
   AND	double *qstar)
{
	double	alpha, beta, lzz0, k, pi2, Ri, Ricrit, zeta, psi1, psi2, x, y;

	alpha = 16;
	beta = 4.7;
	Ricrit = 1 / beta;
	lzz0 = log(z/z0);
	k = VON_KARMAN;
	pi2 = PI/2.;

	/* convert t to potential temp, w.r.t. surface */
	t += DALR * z;

	/* bulk Richardson no at geom mean ht */
	Ri = ri_no(z,z0,t,t0,u,u0);

	/* laminar flow if Ri >= Ricrit */
	if (Ri >= Ricrit) {
		*ustar = *tstar = *qstar = 0.0;
		return;
	}

	if (Ri == 0)
		*tstar = psi1 = psi2 = 0;
	else {
		/* dimensionless ht zeta=z/L, L = Obukhov length */
		if (Ri < 0.) {
			zeta = Ri;

			/* x = 1/phi-sub-m */
			y = sqrt(1.-alpha*zeta);
			x = sqrt(y);

			/* the Businger-Dyer psi-functions */
			psi1 = 2 * log((1+x)/2) + log((1+y)/2) -
			    2 * atan(x) + pi2;
			psi2 = 2 * log((1+y)/2);
		}

		else {
			zeta = Ri / (1.-beta*Ri);
			psi1 = psi2 = -beta * zeta;
		}

		*tstar = k * (t-t0) / (lzz0 - psi2);
	}

	*ustar = k * (u-u0) / (lzz0 - psi1);
	*qstar = k * (q-q0) / (lzz0 - psi2);

	return;
}
