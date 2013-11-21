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

/*
** parse command line arguments
*/

#include <math.h>
#include <time.h>
#include "ipw.h"
#include "getargs.h"
#include "sunang.h"

#define MIN_DEG	60
#define SEC_DEG 3600


struct tm      *
sunang_opts(t, b, l, s, a, r, z, y, lat, lon, slope, azm)
	OPTION_T       *t;		/* time			 */
	OPTION_T       *b;		/* latitude		 */
	OPTION_T       *l;		/* longitude		 */
	OPTION_T       *s;		/* local slope		 */
	OPTION_T       *a;		/* local azimuth	 */
	OPTION_T       *r;		/* ? radians		 */
	OPTION_T       *z;		/* zone			 */
	OPTION_T       *y;		/* ? daylight savings	 */
	double         *lat;		/* latitude		 */
	double         *lon;		/* longitude		 */
	double         *slope;		/* local slope		 */
	double         *azm;		/* local azimuth	 */
{
	struct tm      *pt;		/* time structure	 */
	struct tm      *newt;		/* GMT structure	 */
	double          pio4;
	int             zone;		/* time zone		 */

	pio4 = atan(1.);
 /*
  * latitude: if radians just get the number from command line;
  * otherwise parse degrees, minutes, and seconds. The minutes and
  * seconds are optional; degrees, minutes, or seconds are not
  * restricted to integers. Latitudes in the southern hemisphere are
  * negative.
  */
	if (got_opt(*r)) {		/* radians	 */
		*lat = real_arg(*b, 0);
	}
	else {				/* degrees	 */
		*lat = fabs(real_arg(*b, 0));
		if (n_args(*b) > 1)	/* minutes	 */
			*lat += real_arg(*b, 1) / MIN_DEG;
		if (n_args(*b) > 2)	/* seconds	 */
			*lat += real_arg(*b, 2) / SEC_DEG;
		if (real_arg(*b, 0) < 0)/* southern hem	 */
			*lat = -(*lat);
		if (fabs(*lat) > 90) {
			error("latitude (%g) must be between +/- 90",
			      *lat);
		}
		*lat *= pio4 / 45;	/* convert to radians	 */
	}

 /*
  * longitude: parse just like latitude above. Longitudes in the
  * western hemisphere are negative.
  */
	if (got_opt(*r)) {		/* radians      */
		*lon = real_arg(*l, 0);
	}
	else {				/* degrees      */
		*lon = fabs(real_arg(*l, 0));
		if (n_args(*l) > 1)	/* minutes	 */
			*lon += real_arg(*l, 1) / MIN_DEG;
		if (n_args(*l) > 2)	/* seconds	 */
			*lon += real_arg(*l, 2) / SEC_DEG;
		if (real_arg(*l, 0) < 0)/* western hem	 */
			*lon = -(*lon);
		if (fabs(*lon) > 180) {
			error("longitude (%g) must be between +/- 180",
			      *lon);
		}
		*lon *= pio4 / 45;	/* convert to radians	 */
	}
 /*
  * local slope and azimuth: either radians or decimal degrees
  */
	if (got_opt(*s)) {
		*slope = real_arg(*s, 0);
		if (!got_opt(*r)) {
			*slope *= pio4 / 45;	/* convert to radians */
		}
	}
	else {
		*slope = 0;
	}
	if (got_opt(*a)) {
		*azm = real_arg(*a, 0);
		if (!got_opt(*r)) {
			*azm *= pio4 / 45;	/* convert to radians */
		}
	}
	else {
		*azm = 0;
	}
 /*
  * date and time: pt = unixtime(yr, mo, day, hr, min, sec, isdst);
  */
	pt = unixtime(int_arg(*t, 0), int_arg(*t, 1), int_arg(*t, 2),
		      int_arg(*t, 3),
		      (n_args(*t) > 4) ? int_arg(*t, 4) : 0,
		      (n_args(*t) > 5) ? int_arg(*t, 5) : 0,
		      got_opt(*y));
	if (pt == NULL)
		error("unixtime");
 /*
  * time zone
  */
	zone = (got_opt(*z)) ? int_arg(*z, 0) : 0;
 /*
  * convert to GMT
  */
	if (zone || got_opt(*y)) {
		newt = gmt(pt, zone);
		if (newt == NULL)
			error("gmt");
		SAFE_FREE(pt);
		pt = newt;
	}

	return (pt);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/sunang/RCS/sunang_opts.c,v 1.6 90/11/11 17:09:31 frew Exp $";

#endif
