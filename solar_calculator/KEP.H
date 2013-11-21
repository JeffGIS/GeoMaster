/* General definitions for aa program. */

#include <stdio.h>

/* Indicate the endian-ness of your computer's float format.
   IBM PC is little endian, 68K and SPARC are big endian.  */
#define LITTLEEND 1
#define BIGEND 0
/* If you have to swap ends to read the ephemeris file,
   then define ENDIANNESS nonzero.  For example, the files deNNN.unx
   from JPL are bigendian and must be byte reversed for Intel format. */
#define ENDIANNESS 1
/* Define 1 if ephemeris tape is DEC PDP-11/VAX float format,
   but your computer has IEEE float format.  */
#define DECDATA 0

/* Describe your software system.
   See kepjpl.c and kfiles.c for system-dependent I/O. */
/* Microsoft C */
#ifdef _MSC_VER
#define IBMPC 1
#if _MSC_VER > 1000
#include <stdlib.h>
#endif
#endif
/* Unix, DJGPP, GNU */
#ifdef unix
#define UNIX 1
#endif

#ifdef __GNUC__
#ifndef UNIX
#define UNIX 1
#endif
#endif

/* Please send patches for other systems.  */

/* DEBUG = 1 to enable miscellaneous printouts. */
#define DEBUG 0

/* Interpret the configuration as defined in compiler command line.
   Don't change these; do it in the makefile.  */
#ifdef DE200
#undef DE200
#define DE200 1
#else
#define DE200 0
#endif

#ifdef DE200CD
#undef DE200CD
#define DE200CD 1
#else
#define DE200CD 0
#endif

#ifdef DE102
#undef DE102
#define DE102 1
#else
#define DE102 0
#endif

#ifdef DE245
#undef DE245
#define DE245 1
#else
#define DE245 0
#endif

#ifdef DE400
#undef DE400
#define DE400 1
#else
#define DE400 0
#endif

#ifdef DE403
#undef DE403
#define DE403 1
#else
#define DE403 0
#endif

#ifdef LIB403
#undef LIB403
#define LIB403 1
#undef DE403
#define DE403 1
#else
#define LIB403 0
#endif

#ifdef DE404
#undef DE404
#define DE404 1
#else
#define DE404 0
#endif

#ifdef DE405
#undef DE405
#define DE405 1
#else
#define DE405 0
#endif

/* DE406 CD published by Willman-Bell */
#ifdef DE406CD
#undef DE406CD
#define DE406CD 1
#else
#define DE406CD 0
#endif

/* DE406 files from JPL's ftp archive site.  */
#ifdef DE406
#undef DE406
#define DE406 1
#else
#define DE406 0
#endif

#ifdef SSYSTEM
#undef SSYSTEM
#define SSYSTEM 1
#else
#define SSYSTEM 0
#endif


struct orbit
	{
	char obname[16]; /* name of the object */
	double epoch;	/* epoch of orbital elements */
	double i;	/* inclination	*/
	double W;	/* longitude of the ascending node */
	double w;	/* argument of the perihelion */
	double a;	/* mean distance (semimajor axis) */
	double dm;	/* daily motion */
	double ecc;	/* eccentricity */
	double M;	/* mean anomaly */
	double equinox;	/* epoch of equinox and ecliptic */
	double mag;	/* visual magnitude at 1AU from earth and sun */
	double sdiam;	/* equatorial semidiameter at 1au, arc seconds */
/* The following used by perterbation formulas: */
	int (*oelmnt )(); /* address of program to compute elements */
	int (*celmnt )(); /* program to correct longitude and radius */
	double L;	/* computed mean longitude */
	double r;	/* computed radius vector */
	double plat;	/* perturbation in ecliptic latitude */
	};

struct star
	{
	char obname[32];	/* Object name (31 chars) */
	double epoch;		/* Epoch of coordinates */
	double ra;		/* Right Ascension, radians */
	double dec;		/* Declination, radians */
	double px;		/* Parallax, radians */
	double mura;		/* proper motion in R.A., rad/century */
	double mudec;		/* proper motion in Dec., rad/century */
	double v;		/* radial velocity, km/s */
	double equinox;		/* Epoch of equinox and ecliptic */
	double mag;		/* visual magnitude */
	};
/* Note the items for a star are in different measurement units
 * in the ASCII file description.
 */

/* aa.c */
extern double DTR;
extern double RTD;
extern double RTS;
extern double STR;
extern double PI;
extern double J2000;
extern double B1950;
extern double J1900;
extern double Caud;
extern double Rearthau;
extern double JD;
extern double TDT;
extern double UT;
extern double dradt, ddecdt;
extern int objnum, jdflag, prtflg;
extern double obpolar[];
extern double eapolar[];
extern double rearth[];
extern double dp[];
/* angles.c */
extern double SE, SO, EO, pq, ep, qe;
/* nutate.c */
extern double jdnut, nutl, nuto;
/* epsiln.c */
extern double jdeps, eps, coseps, sineps;
/* vearth.c */
extern double jvearth, vearth[];
/* dms.c */
double mod360(), modtp();

/* OFDATE = 1 for equinox of date in Meeus planetary
 * orbit perturbation formulas.
 * OFDATE = 0 for equinox J2000.
 */
#define OFDATE 0

#define POLYN 1
#define DEPOLYN (DE102 | DE200 | DE200CD | DE245 | DE400 | DE403 | DE404 | DE405 | DE406 | DE406CD | SSYSTEM)

#if __STDC__
#include "protos.h"
#define ANSIPROT
#else
int showrd(), showcor(), dms(), hms(), jtocal(), epsiln();
int fk4fk5(), kepler(), kepjpl(), kinit(), getnum(), deltap();
int lonlat(), nutate(), precess(), reduce(), rstar();
int lightt(), velearth(), diurpx(), diurab(), update();
int relativity(), showcname(), annuab(), angles(), altaz();
int dosun(), doplanet(), dostar(), iter_trnsit();
double mod360(), modtp();
int domoon(), jpl(), cvtdec();
int getstar(), getorbit(), trnsit();
double sidrlt(), refrac();
void prvec();
double tdb(), zgetdate(), gethms();
double acos(), asin(), atan(), zatan2(), cos(), sin();
double tan(), sqrt(), fabs(), log(), floor(), polevl();
char *whatconstel();
double zgetdate(), gethms();
int update(), getnum(), getstar(), getorbit(), showcname(), kepler();
int dosun(), iter_trnsit(), domoon(), dostar(), doplanet(), jpl();
void EtoM(), MtoE(), midentity(), mtransp(), mmpy3();
void mrotx(), mroty(), mrotz();
#endif

/* ASCII ephemeris output file. */
extern FILE *ephfile;
extern int ephprint;
