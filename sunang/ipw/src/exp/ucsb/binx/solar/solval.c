/*
**  NAME
**	solval - spectral value of solar constant, W ^ m^-2 um^-1
**	solwn - spectral value of solar constant, W ^ m^-2 cm
**
**  SYNOPSIS
**	double solval(lambda)
**	double lambda;
**	double solwn(waveno)
**	double waveno;
**
**  DESCRIPTION
**	Solval returns spectral value of solar constant at wavelength
**	lambda (in micrometers).  Solwn returns spectral value at
**	wavenumber waveno (in cm^-1).
**	Data are either from
**	Thekaekara, NASA TR-R-351, 1979
**	or
**	Makarova and Kharitinov, NASA TT F-803
**	depending on flag in solar.h
**
**  DIAGNOSTICS
**
**  BUGS
*/

#include "ipw.h"
#include "solar.h"
#include "pgm.h"

double
DEFUN( solval, (lambda),
	double          lambda)
{
	static int      already = 0;
	static double   c[3 * NSOL];

	double         *wave;
	double         *val;
	double          value;


	wave = &x[0][0];
	val = &x[1][0];

 /*
  * first pass: initialization
  */
	if (!already) {
		already = 1;

 /*
  * transpose to trace mode
  */
		vtdbl(wave, NSOL, 2);

 /*
  * calculate splines
  */
		if (akcoef(wave, val, NSOL, c) == ERROR)
			error("solval: akcoef error");
	}

	if (lambda < wave[0] || lambda > wave[NSOL - 1])
		return (0.);

	value = spval1(lambda, wave, val, NSOL, c);

	return (value * SOL_CON);
}

double
DEFUN( solwn, (eta),
	int             eta)
{
	double          lambda;
	double          result;

 /*
  * convert inverse cm to wavelength in um
  */
	lambda = WAVELEN(eta);

	result = solval(lambda);
 /*
  * convert W m^-2 um^-1 to W m^-2 cm
  */
	lambda *= 1.e-6;		/* -> meters */
	result *= 1.e-6;		/* -> per meter */
	result *= lambda * lambda;	/* -> W m^-2 m */
	result *= 1.e2;			/* -> W m^-2 cm */

	return (result);
}
