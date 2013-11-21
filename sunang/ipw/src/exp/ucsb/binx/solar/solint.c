/*
**	solint - integral of solar constant from wavelengths a to b
**		(in micrometers)
**	solwnt - integral of solar constant from wavenumbers a to b
**		(in cm^-1)
**
**	Data are from either
**	Thekaekara, NASA TR-R-351, 1979
**	or
**	Makarova and Kharitinov, NASA TT F-803
**	depending on whether
**
**		#define THEKAEKARA
**	or	#define MAKAROVA
**	is specified in solar.h
**
**	These are adjusted to whatever value is #defined as
**	solar constant in solar.h
*/

#include <math.h>
#include "ipw.h"
#include "solar.h"
#include "pgm.h"

double
DEFUN( solint, (a, b),
	double   a
   AND  double   b)
{
	static int      already = 0;
	static double   c[3 * NSOL];

	double         *wave;
	double         *val;
	double          intgrl;

	wave = &x[0][0];
	val = &x[1][0];

 /*
  * first pass: initialize
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
			error("solint: akcoef error");
	}

	errno = 0;
	intgrl = fabs(splint(wave, val, NSOL, c, a, b));
	if (errno)
		error("solint: splint error");

	return (intgrl * SOL_CON);
}

double
DEFUN( solwnt, (a, b),
	int             a
   AND  int             b)
{
	static int      already = 0;
	static double   c[3 * NSOL];
	static double   wn[NSOL];
	static double   wnval[NSOL];
	double          intgrl;
	int             j;

 /*
  * first pass: initialize
  */
	if (!already) {
		already = 1;

 /*
  * convert wavelengths to wavenumbers
  */
		for (j = 0; j < NSOL; j++)
			wn[j] = WAVENO(x[0][j]);

 /*
  * sort into increasing order
  */
		SORT_ALG(wn, NSOL, sizeof(double), updbl);

 /*
  * solar const in W m^-2 cm
  */
		for (j = 0; j < NSOL; j++)
			wnval[j] = solwn((int) wn[j]);

 /*
  * calculate splines
  */
		if (akcoef(wn, wnval, NSOL, c) == ERROR)
			error("solwnt: akcoef error");
	}

	errno = 0;
	intgrl = fabs(splint(wn, wnval, NSOL, c, (double) a, (double) b));
	if (errno)
		error("solwnt: splint error");

	return (intgrl);
}
