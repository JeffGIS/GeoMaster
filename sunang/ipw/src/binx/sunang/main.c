/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the Computer Systems Laboratory, University of
 * California, Santa Barbara and its contributors'' in the documentation
 * or other materials provided with the distribution and in all
 * advertising materials mentioning features or use of this software.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <math.h>
#include <time.h>
#include "ipw.h"
#include "getargs.h"
#include "sunang.h"

/*
** NAME
**	sunang -- calculate sun angles
**
** SYNOPSIS
**	sunang -t yr,mon,day,hr[,min[,sec]] -b deg[,min,[,sec]]
**	       -l deg[,min[,sec]] [-s slope] [-a azm] [-r] [-y]
**	       [-d] [-z zone] [-f]
**
**
** DESCRIPTION
**	sunang calculates the sun angle (the azimuth and zenith angles
**	of the sun's position) for a given date, time, and geodetic
**	location.  The sun angle is written to the standard output in
**	the format:
**
**		GMT {weekday} {month} {day} {hr}:{min}:{sec} {year}
**		-z {degrees} -u {cos(z)} -a {degrees}
**
**	The first line contains the date and time.  The second line
**	contains the zenith angle, the cosine of the zenith angle, and
**	the solar azimuth.  The second like is suitable for direct
**	substitution into the command lines of several other IPW
**	programs (horizon, shade, etc.).
**
**	Times are in GMT by default, unless a zone is specified.  Latitudes
**	in southern hemisphere and longitudes in western hemisphere are
**	negative.  The azimuth reference is 0 toward the south, with
**	negative azimuths west of south.
**
** OPTIONS
**
**	-t	Calculate sun angle on date {yr}/{mon}/{day} at time
**		{hr}:{min}:{sec} GMT.  {yr} must be fully specified
**		(i.e., 90 means 90 A.D., not 1990).  {hr} is 24-hour time.
**
**	-b	Calculate sun angle at latitude {deg},{min},{sec}.
**		South latitudes are negative.
**
**	-l	Calculate sun angle at longitude {deg},{min},{sec}.
**		West longitudes are negative.
**
**	All of -t, -b, and -l must be specified.  {min} and {sec} default
**	to 0 if not specified.
**
**	-s	Calculate sun angle for a surface with a slope of
**		{deg} (0 .. 90) degrees (default: 0).
**
**	-a	Calculate sun angle for a surface with an azimuth of
**		{deg} (-180 .. 180) degrees (default: 0).
**
**	-r	All angles are specified in radians (default: degrees,
**		minutes, seconds for -b and -l; decimal degrees for
**		-s and -a).
**
**	-z	Use a time zone {min} west of Greenwich (default: 0) for
**		the time specified with the -t option.
**
**	-y	Use daylight savings time (default: standard time) for
**		the time specified with the -t option.
**
**	-d	Print Earth-Sun radius vector in addition to the sun angle.
**
**	-f	Force the printing of date, time, and zenith angle, even
**		when the output is not the standard output.
**
** EXAMPLES
**	To calculate the sun angle in Santa Barbara on 15 February 1990
**	at 12:30 PM Pacific Standard Time:
**
**		sunang -b 34,25 -l -119,54 -t 1990,2,15,12,30 -z 480
**
**	The output would be:
**
**		GMT Thu Feb 15 20:30:00 1990
**		-z 47.122 -u 0.680436 -a -5.413
**
**	Given a DEM image for the Santa Barbara area, the following could
**	be used to generate an index of local beam irradiance (discounting
**	atmospheric effects):
**
**		gradient {dem} | shade `sunang {options as above} | tail -1`
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	-r does not change the output representations of zenith and
**	azimuth -- they are always printed as decimal degrees.
**
** FUTURE DIRECTIONS
**	-b and -l should accept decimal degrees as well as radians and
**	degrees,minutes,seconds.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	8/7/92	Moved support files (ephemeris, gmt, julday, rotate,
**		sunpath, tmconv, unixtime, weekday, xdysize, yrday) into
**		libipw/sunang since they are used by other programs also.
**		Dana Jacobsen, ERL-C.
**	4/9/93	Added -f option.  Dana Jacobsen, ERL-C.	
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  gradient, hor1d, horizon, shade
**	UNIX: tail
**
**	W. H. Wilson, "Solar ephemeris algorithm," Reference 80-13, 70 pp.,
**		Scripps Institution of Oceanography, University of California
**		at San Diego, La Jolla, California, 1980.
*/

extern struct tm *sunang_opts();

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_t = {
		't', "date & time: year, month, day, hour [, min, sec]",
		INT_OPTARGS, "i",
		REQUIRED, 4, 6
	};

	static OPTION_T opt_b = {
		'b', "latitude: deg, min, sec unless -r specified",
		REAL_OPTARGS, "lat",
		REQUIRED, 1, 3
	};

	static OPTION_T opt_l = {
		'l', "longitude: deg, min, sec unless -r specified",
		REAL_OPTARGS, "lon",
		REQUIRED, 1, 3
	};

	static OPTION_T opt_s = {
		's', "slope of surface: deg unless -r specified",
		REAL_OPTARGS, "slope",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_a = {
		'a', "azimuth of surface: deg unless -r specified",
		REAL_OPTARGS, "azm",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_r = {
		'r', "lat/lon in radians"
	};

	static OPTION_T opt_z = {
		'z', "time zone, minutes west of Greenwich (GMT if omitted)",
		INT_OPTARGS, "zone",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_y = {
		'y', "daylight savings flag"
	};

	static OPTION_T opt_d = {
		'd', "print Earth-Sun radius vector also"
	};

	static OPTION_T opt_f = {
		'f', "force printing of GMT and z even if not a tty"
	};

	static OPTION_T *optv[] = {
		&opt_t,
		&opt_b,
		&opt_l,
		&opt_s,
		&opt_a,
		&opt_r,
		&opt_z,
		&opt_y,
		&opt_d,
		&opt_f,
		0
	};

	double          AzmSlope;	/* local azimuth (radians)	 */
	double          AzmSun;		/* solar azimuth		 */
	double          Declination;	/* solar declination		 */
	double          Latitude;	/* latitude (radians)		 */
	double          Longitude;	/* longitude (radians)		 */
	double          Omega;		/* solar longtiude		 */
	double          RadVec;		/* radius vec			 */
	double          SlopeAng;	/* local slope (radians)	 */
	double          mu0;		/* cosine solar zenith		 */
	double          mu;		/* cosine local zenith		 */
	double          phi;		/* local solar azimuth		 */
	double          pio4;		/* pi / 4			 */
	int             rtn;		/* return code			 */
	struct tm      *tp;		/* time structure		 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "calculates sun angles\n");
 /*
  * arguments
  */
	tp = sunang_opts(&opt_t, &opt_b, &opt_l, &opt_s, &opt_a, &opt_r,
	     &opt_z, &opt_y, &Latitude, &Longitude, &SlopeAng, &AzmSlope);
	if (tp == NULL)
		error("options");
 /*
  * calculate ephemeris parameters
  */
	if (ephemeris(tp, &RadVec, &Declination, &Omega) == ERROR)
		error("ephemeris calculation");
 /*
  * calculate sun angle
  */
	rtn = sunpath(Latitude, Longitude, Declination, Omega,
		      &mu0, &AzmSun);
	if (rtn == ERROR) {
		error("sunpath error");
	}
 /*
  * adjust for local gradient if specified
  */
	if (got_opt(opt_a) || got_opt(opt_s)) {
		if (rotate(mu0, AzmSun, cos(SlopeAng), AzmSlope, &mu, &phi) == ERROR) {
			error("rotate");
		}
	}
	else {
		mu = mu0;
		phi = AzmSun;
	}
 /*
  * print
  */
	pio4 = atan(1.);
	if ( isatty(fileno(stdout)) || got_opt(opt_f) ) {
		(void) printf("\nGMT %s", asctime(tp));
		if (rtn == 1) {
			(void) printf("(below horizon) ");
		}
		(void) printf("-z %.5g ", acos(mu) * 45 / pio4);
	}
	(void) printf("-u %f", mu);
	(void) printf(" -a %.5g", phi * 45 / pio4);
	if (got_opt(opt_d)) {
		(void) printf(" -d %.5g", RadVec);
	}
	putchar('\n');
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/sunang/RCS/main.c,v 1.7 90/11/11 17:09:26 frew Exp $";

#endif
