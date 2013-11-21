#include <time.h>
#include <math.h>

#include "ipw.h"
#include "getargs.h"
#include "sunang.h"

#include "sunlight.h"

/*
** NAME
**	sunlight -- sunrise and sunset times and quadrature weights
**
** SYNOPSIS
**	sunlight -d year,month,day -b lat[,lat,lat] -l lon[,lon,lon] [-r]
**		[-z zone] [-y] [-q npts] [-a]
**
** DESCRIPTION
**	sunlight calculates times of sunrise and sunset, given a date,
**	latitude, and longitude.  It can optionally also calculate times,
**	weights, and sun angles for numerical integration of solar
**	radiation over the day, currently with either 15 or 21 Kronrod
**	quadrature points.  Time returned is in GMT, unless a zone (with
**	optional daylight savings flag) is specified.
**
** OPTIONS
**	-d	The date is {year}, {month}, {day}.
**
**	-b	The latitude is {lat}, specified in degress, minutes, seconds
**		unless -r is specified, in which case it is in radians.
**
**	-l	The longitude is {lon}, specified in degress, minutes, seconds
**		unless -r is specified, in which case it is in radians.
**
**	-r	The latitude and longitude options are taken to be radians
**		rather than degrees, minutes, seconds.
**
**	-z	The time zone is {zone} minutes west of Greenwich (UTC).
**
**	-y	Daylight Savings time is in effect.
**
**	-q	Quadrature points will also be printed.  The number of points
**		used will be {npts}.
**
**	-a	The output will be abbreviated for use in shell scripts.  If
**		the "-q" option is not specified, this will result in no
**		output at all.
**
** EXAMPLES
**	To calculate the day length, radian lat/lon, and sunrise/sunset times
**	for a site in the Columbia Basin on June 22, 1990:
**
**		sunlight -d 1990,6,22 -b 37 -l -112,30 -z 480
**
** FILES
**
** DIAGNOSTICS
**	-q: only 15 or 21 pts supported
**
**		The only supported values for number of quadrature points is
**		15 and 21.
**
** RESTRICTIONS
**	Using the -a option inhibits printing of sunrise and sunset times.
**
** FUTURE DIRECTIONS
**	Allow dynamic quadrature point calculation.  Perhaps use the standard
**	Guassian-Legendre quadrature, which gives greater accuracy for the
**	same number of points than Kronrod (whose advantage is the ability
**	to re-use points, i.e., when doing adaptive quadrature which this
**	program doesn't do).
**
** HISTORY
**	4/3/88	Written by Jeff Dozier, UCSB.
**	5/3/93	Use new zonetime function instead of Sun-specific timezone
**		function.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  gsunlight, solar, sunang, elevrad, toporad
*/

void
DEFUN( main, (argc, argv),
	int             argc
   AND  char          **argv)
{
	static OPTION_T opt_d = {
		'd', "date (year, month, day)",
		INT_OPTARGS, "i",
		REQUIRED, 3, 3
	};

	static OPTION_T opt_b = {
		'b', "latitude (deg, min, sec unless -r specified)",
		REAL_OPTARGS, "lat",
		REQUIRED, 1, 3
	};

	static OPTION_T opt_l = {
		'l', "longitude (deg, min, sec unless -r specified)",
		REAL_OPTARGS, "lon",
		REQUIRED, 1, 3
	};

	static OPTION_T opt_r = {
		'r', "lat/lon in radians"
	};

	static OPTION_T opt_z = {
		'z', "time zone, minutes west of Greenwich",
		INT_OPTARGS, "zone",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_y = {
		'y', "daylight savings flag"
	};

	static OPTION_T opt_a = {
		'a', "abbreviated output for shell scripts"
	};

	static OPTION_T opt_q = {
		'q', "# quadrature points",
		INT_OPTARGS, "npts",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_d,
		&opt_b,
		&opt_l,
		&opt_r,
		&opt_z,
		&opt_y,
		&opt_q,
		&opt_a,
		0
	};

	bool_t          isdst;		/* ? daylight savings		 */
	double          daylen;		/* day length (Hrs)		 */
	double          latitude;	/* latitude (radians)	 	 */
	double          longitude;	/* longitude (radians)	 	 */
	double          pio4;		/* pi / 4			 */
	int             day;		/* day (1-31)		 	 */
	int             jt;		/* time index			 */
	int             kt;		/* # Kronrod pts		 */
	int             month;		/* month (1-12)		 	 */
	int             year;		/* year (1902-2037)	 	 */
	int             zone;		/* minutes west of Greenwich	 */
	struct qt     **qtime;		/* quadrature times/wts		 */
	struct tm      *tprise;		/* -> structure to print	 */
	struct tm      *tprnt;		/* -> structure to print	 */
	struct tm      *tpset;		/* -> structure to print	 */
	struct tm      *trise;		/* -> time structure	 	 */
	struct tm      *tset;		/* -> time structure	 	 */

	pio4 = atan(1.);
 /*
  * begin
  */
	ipwenter(argc, argv, optv,
	 "sunrise and sunset times and Kronrod quadrature weights\n");
 /*
  * arguments
  */
	sunlight_opts(&opt_d, &opt_b, &opt_l, &opt_r, &opt_z, &opt_y,
	   &zone, &isdst, &latitude, &longitude, &year, &month, &day);
 /*
  * compute sunrise/set
  */
	trise = sunrise(latitude, longitude, year, month, day);
	if (trise == NULL) {
		error("sunrise");
	}
	tset = sunset(latitude, longitude, year, month, day);
	if (tset == NULL) {
		error("sunset");
	}
	daylen = (double) tdiff(tset, trise) / SEC_HR;
 /*
  * convert to local time?
  */
	if (zone != 0 && daylen != 24 && daylen != 0) {
		tprise = loct(trise, zone, isdst);
		if (tprise == NULL)
			error("loct, sunrise");
		tpset = loct(tset, zone, isdst);
		if (tpset == NULL)
			error("loct, sunset");
	}
	else {
		tprise = trise;
		tpset = tset;
	}
 /*
  * Results
  */
	if (!got_opt(opt_a)) {
		printf("Time zone is %s; day length is %.2f hrs\n",
		       zonetime(zone, isdst), daylen);
		printf("Latitude/longitude (radians): %.10g %.10g\n",
		       latitude, longitude);
		printf("Sunrise: %s", asctime(tprise));
		printf("Sunset:  %s", asctime(tpset));
	}
 /*
  * quadrature times and weights
  */
	if (got_opt(opt_q)) {
 /*
  * only 15 or 21 pts supported
  */
		kt = int_arg(opt_q, 0);
		switch (kt) {
		case 15:
		case 21:
			break;
		default:
			error("-q: only 15 or 21 pts supported");
		}
 /*
  * Kronrod pts
  */
		assert((qtime = krontime(kt, trise, tset,
				       latitude, longitude)) != NULL);
 /*
  * print
  */
		if (!got_opt(opt_a)) {
			printf("\n%d%s%s\n",
			       kt,
			       "-pt Kronrod quadrature weights, ",
			       "sun angles, and times.");
		}
		for (jt = kt; --jt >= 0;) {
 /*
  * convert to local time?
  */
			if (zone != 0 && daylen != 24 && daylen != 0) {
				tprnt = loct((*qtime)->q_tp, zone, isdst);
				if (tprnt == NULL)
					error("loct - quad times");
			}
			else {
				tprnt = (*qtime)->q_tp;
			}
			printf("%g %g %g",
			       (*qtime)->q_wt,
			       (*qtime)->q_cos,
			       (*qtime)->q_azm * 45 / pio4);
			if (!got_opt(opt_a))
				printf(" %s", asctime(tprnt));
			else
				putchar('\n');
			++qtime;
		}
	}
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.3 88/04/03 11:07:03 dozier Exp $";

#endif
