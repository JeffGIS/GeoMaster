#ifndef CALC_EPHEM_H
#define CALC_EPHEM_H

/* Copyright (C) 2001, Norman Walsh. Derived from gkrellmoon by
 * Dale P. Smith and wmSun by Mike Henderson
 *
 * $Id: CalcEphem.h,v 1.1 2002/10/11 13:04:14 nwalsh Exp $
 *
 */

#include <glib.h>

#include <stdio.h>
#include <math.h>

#define DegPerRad       57.29577951308232087680
#define RadPerDeg        0.01745329251994329576


typedef struct Vector {
	gdouble x;
	gdouble y;
	gdouble z;
} Vector;


typedef struct Position {
	gdouble x;
	gdouble y;
	gdouble z;
} Position;


typedef struct CTrans {
	gdouble UT;		/* Universal Time (in decimal hours) */
	gint year;		/* 2 digit year */
	gint month;		/* 2 digit month of year */
	gint day;		/* 2 digit day of month */
	gint doy;		/* 3 digit Day Of Year */
	gint dow;		/* 1 digit day of week */
	gchar dowstr[80];	/* Day of week String (e.g. "Sun") */
	gdouble gmst;		/* Greenwich Mean Sidereal Time */
	gdouble eccentricity;	/* Eccentricity of Earth-Sun orbit */
	gdouble epsilon;	/* Obliquity of the ecliptic (in radians) */
	gdouble lambda_sun;	/* Ecliptic Long. of Sun (in radians) */
	gdouble earth_sun_dist;	/* Earth-Sun distance (in units of earth radii) */
	gdouble RA_sun;		/* Right Ascention of Sun (in degrees) */
	gdouble DEC_sun;	/* Declination of Sun (in degrees) */
	Vector Sun;		/* direction of Sun in GEI system (unit vector) */
	Vector EcPole;		/* direction of Ecliptic Pole in GEI system (unit vector) */
	gdouble psi;		/* Geodipole tilt angle (in radians) */
	gdouble Dipole_Gcolat;	/* Geographic colat of centered dipole axis (deg.) */
	gdouble Dipole_Glon;	/* Geographic long. of centered dipole axis (deg.) */

	gdouble RA_moon;	/* Right Ascention of Moon (in degrees) */
	gdouble DEC_moon;	/* Declination of Moon (in degrees) */
	gdouble MoonPhase;	/* The Phase of the Moon (in days) */
	gdouble MoonAge;	/* Age of Moon in Days */
	gdouble EarthMoonDistance;	/* Distance between the Earth and Moon (in earth-radii) */

	gdouble Glat;		/* Geographic Latitude of Observer */
	gdouble Glon;		/* Geographic Longitude of Observer */
	gdouble h_moon;		/* Altitude of Moon (in degrees) */
	gdouble A_moon;		/* Azimuth of Moon (in degrees) */
	gint Visible;		/* Whether or not moon is above horizon */

	/* Additional Data for glunarclock */
	gdouble SinGlat;	/* data calc. in CalcEphem, used in MoonRise */
	gdouble CosGlat;	/* data calc. in CalcEphem, used in MoonRise */

	gdouble LAT;		/* Local Apparent Time (sundial time) */
	gdouble LMT;		/* Local Mean Time (eqn of time corrected LAT) */
	gdouble LST;		/* Local Standard Time */
	/* end additional data */

	int Rise;               /* Does the sun rise? */
	gdouble LTRise;         /* Time the sun rises */
	int Set;                /* Does the sun set? */
	gdouble LTSet;          /* Time the sun sets */
} CTrans;


void CalcEphem(glong date, gdouble UT, CTrans * c);
gdouble jd(gint ny, gint nm, gint nd, gdouble UT);
gdouble hour24(gdouble hour);
gdouble angle2pi(gdouble angle);

#endif				/* CALC_EPHEM_H */
