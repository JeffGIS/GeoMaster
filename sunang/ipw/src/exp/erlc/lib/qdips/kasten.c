/*
** NAME
**	kasten --
**
** SYNOPSIS
**
** DESCRIPTION
**
** HISTORY
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
*/

#include <math.h>

#include "ipw.h"
#include "erlc.h"

void
DEFUN(kasten, (z,cz,ma,mw,mo),
	double	 z
   AND	double	 cz
   AND	double	*ma
   AND	double	*mw
   AND	double	*mo)
{
	z = 9.e1 - (z*1.8e2/(4.e0*atan(1.e0)));
	*ma = 1.e0/(cz+1.5e-1*pow(z+3.885e0,-1.253e0));
	*mw = 1.e0/(cz+5.48e-2*pow(z+2.65e0,-1.452e0));
	*mo = 3.5e1/sqrt(1.224e3*cz*cz+1.e0); 
}
