/* suninfo.c
 * Copyright (C) 2001, Norman Walsh. Derived from gkrellmoon by
 * Dale P. Smith and wmSun by Mike Henderson
 *
 * $Id: suninfo.c,v 1.1 2002/10/11 13:10:40 nwalsh Exp $
 *
 */

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "CalcEphem.h"
#include "Moon.h"

#define SUN_WIDTH 54
#define SUN_HEIGHT 54

typedef struct {
  int longitude;
  int latitude;
} Options;

static Options options;

typedef struct CTrans SunData;

typedef struct _Sun Sun;
struct _Sun {
    SunData data;
};

extern long CurrentGMTTime;

static Sun sununit;

int verbose = 0;
double delta = 0;

static void update_sun_data(Sun * sun) {
    struct tm *time_struc;	/* The tm struct is defined in <time.h> */
    gdouble local_std_time, univ_time, eot;
    glong date;
    gint day_of_month, month, year;

    CurrentGMTTime = time(NULL);
    CurrentGMTTime += (delta * 24 * 3600);

    time_struc = gmtime(&CurrentGMTTime);

    if (verbose) {
      printf("GMT time: %02d:%02d:%02d\n",
	     time_struc->tm_hour,
	     time_struc->tm_min,
	     time_struc->tm_sec);
    }

    univ_time =
	time_struc->tm_hour + time_struc->tm_min / 60.0 +
	time_struc->tm_sec / 3600.0;

    /* The date needs to be the date in UTC, i.e. in greenwich, so
     * be sure not to call the localtime function until after date
     * has been set (there's only one tm structure).  */

    year = time_struc->tm_year + 1900;
    month = time_struc->tm_mon + 1;
    day_of_month = time_struc->tm_mday;

    if (verbose) {
      printf("GMT date: %04d-%02d-%02d\n", year, month, day_of_month);
    }

    date = year * 10000 + month * 100 + day_of_month;

    time_struc = localtime(&CurrentGMTTime);
    local_std_time =
	time_struc->tm_hour + time_struc->tm_min / 60.0 +
	time_struc->tm_sec / 3600.0;

    if (verbose) {
      printf("Lcl time: %02d:%02d:%02d\n",
	     time_struc->tm_hour,
	     time_struc->tm_min,
	     time_struc->tm_sec);
    }

    sun->data.Glat = options.latitude;
    sun->data.Glon = options.longitude;

    CalcEphem(date, univ_time, &sun->data);

    sun->data.LST = local_std_time;
    sun->data.LMT = univ_time - sun->data.Glon / 15.0;
    if (sun->data.LMT < 0.0)
	sun->data.LMT += 24.0;
    if (sun->data.LMT > 24.0)
	sun->data.LMT -= 24.0;

    /* eot is the equation of time. gmst is Greenwich Sidereal
     * Time.  This equation below is correct, but confusing at
     * first.  It's easy to see when you draw the following
     * picture: A sphere with 0 and 180 degree longitude, North on
     * top, a meridian for the real sun, a meridian for a fictive
     * average sun, a meridian denoting the vernal equinox.  Note
     * that universal time is the hour angle between 180 degrees
     * and the fictive sun's meridian measured clockwise.  gmst is
     * the hour angle between 0 degrees and the meridian of the
     * vernal equinox measured clockwise.  RA_sun/15.0 is the hour
     * angle of the real sun measured counterclockwise from the
     * vernal equinox. eot is the difference between the real and
     * the fictive sun.  Looking at the picture, it's easy to see
     * that 12=RA_sun/15-gmst+eot+utc (12 hours = 180 deg.) */

    eot =
	12.0 - univ_time + sun->data.gmst - sun->data.RA_sun / 15.0;

    if (eot < 0.0)
	eot += 24.0;
    if (eot > 24.0)
	eot -= 24.0;

    sun->data.LAT = sun->data.LMT + eot;
    if (sun->data.LAT < 0.0)
	sun->data.LAT += 24.0;
    if (sun->data.LAT > 24.0)
	sun->data.LAT -= 24.0;
}

void printTOD(double tod) {
  double val = tod;
  int H, M;

  H = (int)val; val = (val-H)*60.0;
  M = (int)val;
  if (H >= 12) {
    if (H > 12) {
      H -= 12;
    }
    printf("%d:%02dp\n", H, M);
  } else {
    printf("%d:%02da\n", H, M);
  }
}

int main(int argc, char *argv[]) {
  int  i;
  double alt, noon, altperc, dayperc;
  int x, y;

    options.longitude = 72;	/* Where I live! */
    options.latitude = 42;

    for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-lat")){
	options.latitude = atof(argv[++i]);
      } else if (!strcmp(argv[i], "-lon")){
	options.longitude = atof(argv[++i]);
      } else if (!strcmp(argv[i], "-delta")){
	delta = atof(argv[++i]);
      } else if (!strcmp(argv[i], "-v")){
	verbose = 1;
      } else {
	printf("Usage: suninfo [-v] [-lat <Latitude>] [-lon <Longitude>]\n");
	printf("Set the TZ environment variable to change timezones\n");
	exit(1);
      }
    }

    update_sun_data(&sununit);

    if (verbose) {
      printf("Location: %d lat %d lon\n",
	     options.latitude, options.longitude);
      printf("Timezone: %s\n", getenv("TZ"));
      printf("Sunrise: ");
    }

    if (sununit.data.Rise) {
      printTOD(sununit.data.LTRise);
    } else {
      printf("never\n");
    }

    if (verbose) {
      printf("Sunset: ");
    }

    if (sununit.data.Set) {
      printTOD(sununit.data.LTSet);
    } else {
      printf("never\n");
    }

    if (verbose) {
      printf("Local apparent time: ");
      printTOD(sununit.data.LAT);
      printf("Local mean time: ");
      printTOD(sununit.data.LMT);
      printf("Local standard time: ");
      printTOD(sununit.data.LST);

      printf("Right ascention of the sun: %6.2f\n", sununit.data.RA_sun);
      printf("Declination of the sun: %6.2f\n", sununit.data.DEC_sun);

      if (sununit.data.Rise && sununit.data.Set) {
	alt = 90 - sununit.data.Glat + sununit.data.DEC_sun;
	if (alt > 90) {
	  alt = 90 - (alt - 90);
	}

	printf("Altitude of Sun at solar noon: %6.2f\n", alt);

	noon = sununit.data.LTRise + ((sununit.data.LTSet - sununit.data.LTRise)/2.0);
	printf("Solar noon occurs at: ");
	printTOD(noon);

	dayperc = ((sununit.data.LST - sununit.data.LTRise)
		   / (sununit.data.LTSet - sununit.data.LTRise));

	if (dayperc > 0.5) {
	  altperc = 1.0 - dayperc;
	} else {
	  altperc = dayperc;
	}

	printf("Altitude of Sun now: %6.2f\n", alt * (altperc * 2));

	/* Calculate plot pos */
	x = rint(SUN_WIDTH * dayperc);
	y = rint(SUN_HEIGHT * altperc);

	printf("Plot now: (%d, %d)\n", x, y);
      } else {
	printf("Solar noon does not occur.\n");
      }

      printf("Altitude of the moon: %6.2f\n", sununit.data.h_moon);
      printf("Azimuth of the moon: %6.2f\n", sununit.data.A_moon);
      printf("Visible: %d\n", sununit.data.Visible);
    }

    return 0;
}
