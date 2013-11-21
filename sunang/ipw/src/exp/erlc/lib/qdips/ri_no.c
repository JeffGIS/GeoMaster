/*
** NAME
**	ri_no -- bulk Richardson number at geometric mean height
**		 between z2 and z1
**
** SYNOPSIS
**	double ri_no(z2, z1, t2, t1, u2, u1);
**	double z2;
**	double z1;
**	double t2;
**	double t1;
**	double u2;
**	double u1;
**
** DESCRIPTION
**	arguments--
**	z2, z1	upper and lower heights (m) (z1 may be roughness length)
**	t2, t1	virtual temperatures (K) at z2 and z1
**	u2, u1	wind speeds (m/s) at z2 and z1 (u1 may be zero)
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
**		(Originally named "Ri_no").
**
** BUGS
**
** SEE ALSO
*/

#include	<math.h>

#include	"ipw.h"
#include	"erlc.h"
#include	"physdefs.h"

double
DEFUN(ri_no, (z2, z1, t2, t1, u2, u1),
	double	z2
   AND	double	z1
   AND	double	t2
   AND	double	t1
   AND	double	u2
   AND	double	u1)
{
	double	tgm;
	double	du;

	if (z2 <= z1 || z2 <= 0. || z2 <= 0.)
		error("z2=%g <= z1", z2);
	if (u2 <= u1 || u2 <= 0. || u1 < 0.)
		error("u2=%g <= u1", u2);

	if (u2 == 0.)
		return(0.);

	/* geometric mean virtual temperature */
	tgm = sqrt(t2*t1);

	/* wind speed difference */
	du = u2 - u1;

	return(GRAV*(z2-z1)*(t2-t1)/(tgm*du*du));
}
