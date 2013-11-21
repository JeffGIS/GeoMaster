/* LINTLIBRARY */

#include <time.h>
#include "ipw.h"
#include "sunang.h"
#include "pgm.h"

/*
** NAME
**	solar -- exoatmospheric solar irradiance
**
** SYNOPSIS
**	solar(n, range, date)
**	int n;
**	double *range;
**	int *date;
**
** DESCRIPTION
**	Solar prints values of exoatmospheric solar irradiance.  The
**	vector range of length 2 contains the range over which
**	irradiance is to be integrated.  If n is 1, then spectral
**	irradiance at range[0] is calculated.  If range is NULL,
**	monochromatic values are calculated for wavelengths read from
**	stdin.  The vector date contains yr, mo, day.  If not NULL, the
**	radius vector for a specific date is included in the
**	calculations.  Otherwise, a radius vector of 1.0 is used.
**
** RESTRICTIONS
**
** RETURN VALUE
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

void
DEFUN( solar, (n, range, date, abbrev),
	int             n		/* # values in range	 */
   AND  double         *range		/* range of wavelengths	 */
   AND  int            *date		/* yr, mo, day		 */
   AND  bool_t          abbrev)		/* ? abbreviated output	 */
{
	double          rvsq;		/* (radius vector)^2		 */
	double          declin;		/* declination (not used)	 */
	double          omega;		/* solar longitude (not used)	 */
	double          wave;		/* wavelength (um)		 */
	bool_t          tty;		/* ? input from tty		 */
	struct tm      *time;

 /*
  * if specific date, calculate radius vector
  */
	if (date != NULL) {
		time = unixtime(date[0], date[1], date[2], 12, 0, 0, 0);
		if (time == NULL)
			error("unixtime");
		if (ephemeris(time, &rvsq, &declin, &omega) == ERROR) {
			error("ephemeris error");
		}
		rvsq *= rvsq;
		SAFE_FREE(time);
	}
	else {
		rvsq = 1;
	}
 /*
  * integral over a wavelength range, or specific wavelengths
  */
	if (range != NULL) {
		switch (n) {
		case 2:
			printf("%g", solint(range[0], range[1]) / rvsq);
			if (!abbrev)
				printf(" W m^-2");
			putchar('\n');
			break;
		case 1:
			printf("%g", solval(range[0]) / rvsq);
			if (!abbrev)
				printf(" W m^-2 um^-1");
			putchar('\n');
			break;
		default:
			bug("n != 1  and  n != 2");
		}
	}
	else {
		if ((tty = isatty(fileno(stdin))))
			fprintf(stderr, "enter wavelengths from stdin\n");
		fprintf(stderr, "\nvalues in W m^-2 um^-1\n");
		while (scanf("%lf", &wave) == 1) {
			if (!tty)
				printf("%g\t", wave);
			printf("%g\n", solval(wave) / rvsq);
		}
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: solar.c,v 1.4 88/04/04 16:57:57 dozier Exp $";

#endif
