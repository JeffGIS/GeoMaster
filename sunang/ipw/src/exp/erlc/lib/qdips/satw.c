/*
	SCCS version: @(#)   satw.c   1.2   10/19/90 

********************************************************************************
**
**  NAME
**      satw, sati - saturation vapor pressure over water and ice
**
**  SYNOPSIS
**      double  satw(tk)
**      double  tk;
**
**      double  sati(tk)
**      double  tk;
**
**  DESCRIPTION
**      Satw returns saturation vapor pressure over water (in Pa)
**      as a function of temperature (degrees K).
**
**      Sati returns saturation vapor pressure over ice (in Pa)
**      as a function of temperature (degrees K).
**
**  DIAGNOSTICS
**      Calls syserr() and/or usrerr()
**
**  HISTORY
**      July 1982: written by J. Dozier, Department of Geography, UCSB
**
**  BUGS
*/

#include <math.h>
#include "ipw.h"
#include "erlc.h"
#include "physdefs.h"

double
DEFUN( satw, (tk),
    double  tk)
{
        double  x;
        double  l10;

        if (tk <= 0.) {
                usrerr ("temp = %g", tk);
                return(0.);
        }

        errno = 0;
        l10 = log(1.e1);

        x = -7.90298*(BOIL/tk-1.) + 5.02808*log(BOIL/tk)/l10 -
            1.3816e-7*(pow(1.e1,1.1344e1*(1.-tk/BOIL))-1.) +
            8.1328e-3*(pow(1.e1,-3.49149*(BOIL/tk-1.))-1.) +
            log(SEA_LEVEL)/l10;

        x = pow(1.e1,x);

        if (errno) {
		syserr();
                usrerr ("bad return from log or pow");
        }

        return(x);
}
