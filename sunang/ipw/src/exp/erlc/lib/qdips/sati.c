/*
	SCCS version: @(#)   sati.c   1.2   10/19/90 

********************************************************************************
**
**  name
**      satw, sati - saturation vapor pressure over water and ice
**
**  synopsis
**      double  satw(tk)
**      double  tk;
**
**      double  sati(tk)
**      double  tk;
**
**  description
**      Satw returns saturation vapor pressure over water (in Pa)
**      as a function of temperature (degrees K).
**
**      Sati returns saturation vapor pressure over ice (in Pa)
**      as a function of temperature (degrees K).
**
**  diagnostics
**      Sets Qerrno and writes message to Qerrstr.
**
**  history
**      July 1982: written by J. Dozier, Department of Geography, UCSB
**
**  bugs
*/

#include <math.h>
#include "ipw.h"
#include "erlc.h"
#include "physdefs.h"

double
DEFUN( sati, (tk),
    double  tk)
{
        double  l10;
        double  x;

        if (tk <= 0.) {
                usrerr ("temp = %g", tk);
                return(0.);
        }

        if (tk > FREEZE) {
                x = satw(tk);
                return(x);
        }

        errno = 0;
        l10 = log(1.e1);

        x = pow(1.e1,-9.09718*((FREEZE/tk)-1.) - 3.56654*log(FREEZE/tk)/l10 +
            8.76793e-1*(1.-(tk/FREEZE)) + log(6.1071)/l10);

        if (errno) {
                syserr();
                usrerr ("bad return from log or pow");
        }

        return(x*1.e2);
}
