/*
**  NAME
**	sunrise - compute sunrise time (GMT)
**	sunset - compute sunset time (GMT)
**
**  SYNOPSIS
**	#include <time.h>
**
**	struct tm *sunrise(lat, lon, year, month, day);
**	struct tm *sunset(lat, lon, year, month, day);
**	double lat, lon;
**	int year, month, day;
**
**  DESCRIPTION
**	Sunrise and sunset return time structures (see <time.h>) for
**	sunrise and sunset times at a given latitude and longitude.
**	The time structure contains GMT time of local sunrise/sunset
**	(but not accounting for local horizons).  The input latitude
**	(south negative) and longitude (west negative) are in radians.
**
**  DIAGNOSTICS
**	Returns NULL and writes error message via usrerr().
*/

#include <time.h>
#include <math.h>
#include <errno.h>
#include <memory.h>
#include "ipw.h"
#include "sunang.h"

#include "sunlight.h"

static double   xlat;
static double   xlon;
static long     iclock;			/* secs since 1970/1/1	 */

struct tm      *
DEFUN( sunrise, (lat, lon, year, month, day),
	double          lat
   AND  double          lon
   AND  int             year
   AND  int             month
   AND  int             day)
{
	return (sunpos(lat, lon, year, month, day, RISE));
}

struct tm      *
DEFUN( sunset, (lat, lon, year, month, day),
	double          lat
   AND  double          lon
   AND  int             year
   AND  int             month
   AND  int             day)
{
	return (sunpos(lat, lon, year, month, day, SET));
}

struct tm *
DEFUN( sunpos, (lat, lon, year, month, day, flag),
	double          lat
   AND  double          lon
   AND  int             year
   AND  int             month
   AND  int             day
   AND  int             flag)		/* RISE or SET */
{
	double          a;		/* lower guess for root	 */
	double          b;		/* upper guess for root	 */
	double          cosom;		/* cosine omega		 */
	double          declin;		/* declination		 */
	double          dsec;		/* seconds-of-day	 */
	double          pi;
	double          rv;		/* radius vec (not used) */
	double          sollon;		/* solar lon (not used)	 */
	struct tm      *tp;
	time_t	t;

	xlat = lat;
	xlon = lon;
	pi = 4 * atan(1.);
 /*
  * seconds since 1970/1/1 to midnight of this day
  */
	iclock = SEC_DAY * (julday(year, month, day) - julday(1970, 1, 1));
 /*
  * value of declination at noon GMT on same day
  */
	tp = unixtime(year, month, day, 12, 0, 0, 0);
	if (tp == NULL)
		return (NULL);
	if (ephemeris(tp, &rv, &declin, &sollon) == ERROR) {
		return (NULL);
	}
 /*
  * approximate value of cos(omega) at rise/set
  */
	cosom = -tan(xlat) * tan(declin);
 /*
  * if no rise or set
  */
	if (fabs(cosom) > 1) {
		a = (flag == RISE) ? 0 : SEC_DAY;
		b = SEC_DAY / 2;
	}

	else {
 /*
  * approximate half-daylength, seconds (i.e. 43200 omega / pi)
  */
		dsec = (SEC_DAY / 2) * acos(cosom) / pi;
 /*
  * convert to local seconds-of-day, midnight = 0
  */
		if (flag == RISE)
			dsec = SEC_DAY / 2 - dsec;
		else
			dsec += SEC_DAY / 2;
 /*
  * set high and low values for root search
  */
		a = (dsec > SEC_HR / 2) ? dsec - SEC_HR / 2 : 0;
		b = (dsec < SEC_DAY - SEC_HR / 2) ? dsec + SEC_HR / 2 : SEC_DAY;
	}

 /*
  * convert to seconds GMT
  */
	if (xlon != 0.) {
		a -= 12 * SEC_HR * xlon / pi;
		b -= 12 * SEC_HR * xlon / pi;
	}
 /*
  * If root not spanned, we have either 24 h darkness or 24 h daylight.
  */
	if (sunhorz(a) * sunhorz(b) > 0) {
 /*
  * 24 hrs daylight: set answer to 00:00:00 if sunrise, 24:00:00 if
  * sunset
  */
		if (sunhorz(a) > 0) {
			dsec = (flag == SET) ? SEC_DAY : 0;
		}
 /*
  * 24 hrs darkness: set answer to 00:00:00
  */
		else {
			dsec = 0;
		}
	}
 /*
  * Find time in seconds-of-day.
  */
	else {
		dsec = zerobr(a, b, TOL, sunhorz);
		if (errno) {
			return (NULL);
		}
	}
 /*
  * convert answer to (long) seconds since 1970/1/1
  */
	iclock += dsec + 0.5;
 /*
  * convert answer to time structure and return
  */
	t = iclock;
	(void) memcpy((char *) tp, (char *) gmtime(&t), sizeof(struct tm));
	if (tp->tm_year < 70) {
		tp->tm_wday = weekday(1900 + tp->tm_year,
				      1 + tp->tm_mon, tp->tm_mday);
	}

	return (tp);
}

 /*
  * Returns cosine of solar zenith angle, as function of time-of-day in
  * seconds.  Cosine is negative if sun below horizon.
  */

double
DEFUN( sunhorz, (tsec),
	double          tsec)
{
	double          azm;		/* solar azimuth (not used)	 */
	double          cz;		/* solar cosine			 */
	double          declin;		/* declination			 */
	double          omega;		/* solar longitude		 */
	double          rv;		/* radius vector (not used)	 */
	long            lclock;		/* local copy of clock		 */
	struct tm      *tp;		/* -> time structure		 */
	time_t			t;

 /*
  * convert time-of-day to seconds since 1970/1/1
  */
	lclock = iclock + tsec + 0.5;
 /*
  * Convert to time structure. This gets over-written with each call,
  * but no harm.
  */
	t = lclock;
	tp = gmtime(&t);
 /*
  * Solar declination and longitude of sun.
  */
	if (ephemeris(tp, &rv, &declin, &omega) == ERROR) {
		errno = ERROR;
		return (HUGE_VAL);
	}
 /*
  * compute cosine of solar zenith angle, 0.0 at sunrise/set
  */
	if (sunpath(xlat, xlon, declin, omega, &cz, &azm) == ERROR) {
		errno = ERROR;
		return (HUGE_VAL);
	}

	return (cz);
}
