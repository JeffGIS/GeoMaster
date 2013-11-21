/* This program calculates orbits of planetary bodies and reduces
 * the coordinates of planets or stars to geocentric and topocentric
 * place.  An effort has been made to use rigorous methods throughout.
 *
 * References to AA page numbers are to The Astronomical Almanac, 1986
 * published by the U.S. Government Printing Office.
 *
 * The version named "aa200" finds planetary, lunar, and solar
 * positions from the DE200 numerical integration of the solar
 * system produced by the Jet Propulsion Laboratory.
 *
 * The program named "aa" uses as a default the orbital perturbations
 * given in Jean Meeus, "Astronomical Formulae for Calculators",
 * 3rd ed.,  Willmann-Bell, Inc., 1985.  It can also be configured
 * to use the tables from Bretagnon and Simon, "Planetary Programs
 * and Tables from -4000 to +2800."
 *
 * Warning! Your atan2() function may not work the same as the one
 * assumed by this program.
 * atan2(x,y) computes atan(y/x), result between 0 and 2pi.
 *
 * S. L. Moshier, September, 1989 (v4.3)
 * November, 1987 (v4.2)
 * November, 1993 (v5.2)
 */




#include "kep.h"
#if __STDC__
#include <stdlib.h>
void librations (void);
void EtoM (double e[], double M[]);
void midentity (double M[]);
void mmpy3 (double A[], double B[], double C[]);
void MtoE (double M[], double e[]);
void mrotx ( double M[], double theta);
#else
void librations();
void EtoM();
void midentity();
void mmpy3();
void MtoE();
void mrotx();
#endif
#ifdef _MSC_VER
#if _MSC_VER > 1000
#include <stdlib.h>
#endif
#endif
/* Conversion factors between degrees and radians */
double DTR = 1.7453292519943295769e-2;
double RTD = 5.7295779513082320877e1;
double RTS = 2.0626480624709635516e5; /* arc seconds per radian */
double STR = 4.8481368110953599359e-6; /* radians per arc second */
double PI = 3.14159265358979323846;
extern double PI;

/* Standard epochs.  Note Julian epochs (J) are measured in
 * years of 365.25 days.
 */
double J2000 = 2451545.0;	/* 2000 January 1.5 */
double B1950 = 2433282.42345905; /* 1950 January 0.923 Besselian epoch */
double J1900 = 2415020.0;	/* 1900 January 0, 12h UT */

/*  These values are recalculated in kfiles.c from the astronomical
   constants in deXXX.h  */
/* Speed of light, in astronomical units per day. */
double Clightaud = 173.144632720536344565;
/* Speed of light, 299792.458 meters per second */
double Clight; 
/* Equatorial radius of Earth, in au. */
double Rearthau =  4.26352325064817808471e-5;

/* approximate motion of right ascension and declination
 * of object, in radians per day
 */
double dradt = 0.0;
double ddecdt = 0.0;

/* Space for star description read from a disc file.
 */
struct star fstar;

/* Space for orbit read from a disc file. Entering 99 for the
 * planet number yields a prompt for a file name containg ASCII
 * strings specifying the elements.
 */
struct orbit forbit;

/* Orbits for each planet.  The indicated orbital elements are
 * not actually used, since they are now calculated
 * from the Meeus expansions.  Magnitude and semidiameter
 * are still used.
 */
 /* Programs to compute perturbations. */
#if !POLYN
int omercury();
#endif
#if !DEPOLYN
int cmercury();
#endif
struct orbit mercury = {
"Mercury        ",
2446800.5, /* January 5.0, 1987 */
7.0048,
48.177,
29.074,
0.387098,
4.09236,
0.205628,
198.7199,
2446800.5,
-0.42,
3.36,
#if POLYN
0,
#else
omercury,
#endif
#if DEPOLYN
0,
#else
cmercury,
#endif
0.0,
0.0,
0.0
};

#if !POLYN
int ovenus();
#endif
#if !DEPOLYN
int cvenus();
#endif
struct orbit venus = {
"Venus          ",
2446800.5,
3.3946,
76.561,
54.889,
0.723329,
1.60214,
0.006757,
9.0369,
2446800.5,
/* Note the calculated apparent visual magnitude for Venus
 * is not very accurate.
 */
-4.40,
8.34,
#if POLYN
0,
#else
ovenus,
#endif
#if DEPOLYN
0,
#else
cvenus,
#endif
0.0,
0.0,
0.0
};

/* Meeus purturbations are invoked for earth if POLYN = 0.
 * If POLYN = 1, the more accurate B&S or other polynomial expansion
 * is computed.
 * Fixed numerical values will be used if read in from a file
 * named earth.orb.
 * See kfiles.c, kep.h, oearth.c, and pearth.c.
 */
#if !POLYN
int oearth();
#endif
#if !DEPOLYN
int cearth();
#endif
struct orbit earth = {
"Earth          ",
2446800.5,
0.0,
0.0,
102.884,
0.999999,
0.985611,
0.016713,
1.1791,
2446800.5,
-3.86,
0.0,
#if POLYN
0,
#else
oearth,
#endif
#if DEPOLYN
0,
#else
cearth,
#endif
0.0,
0.0,
0.0
};
extern struct orbit earth;


#if !DEPOLYN
int omars(), cmars();
#endif
struct orbit mars = {
"Mars           ",
2446800.5,
1.8498,
49.457,
286.343,
1.523710,
0.524023,
0.093472,
53.1893,
2446800.5,
-1.52,
4.68,
#if DEPOLYN
0,
0,
#else
omars,
cmars,
#endif
0.0,
0.0,
0.0
};

#if !POLYN
int ojupiter();
#endif
#if !DEPOLYN
int cjupiter();
#endif
struct orbit jupiter = {
"Jupiter        ",
2446800.5,
1.3051,
100.358,
275.129,
5.20265,
0.0830948,
0.048100,
344.5086,
2446800.5,
-9.40,
98.44,
#if POLYN
#if DEPOLYN
0,
0,
#else
0,
cjupiter,
#endif
#else
ojupiter,
0,
#endif
0.0,
0.0,
0.0
};

#if !DEPOLYN
int osaturn(), csaturn();
#endif
struct orbit saturn = {
"Saturn         ",
2446800.5,
2.4858,
113.555,
337.969,
9.54050,
0.0334510,
0.052786,
159.6327,
2446800.5,
-8.88,
82.73,
#if DEPOLYN
0,
0,
#else
osaturn,
csaturn,
#endif
0.0,
0.0,
0.0
};

#if !DEPOLYN
int ouranus(), curanus();
#endif
struct orbit uranus = {
"Uranus         ",
2446800.5,
0.7738,
73.994,
98.746,
19.2233,
0.0116943,
0.045682,
84.8516,
2446800.5,
-7.19,
35.02,
#if DEPOLYN
0,
0,
#else
ouranus,
curanus,
#endif
0.0,
0.0,
0.0
};

#if !DEPOLYN
int oneptune(), cneptune();
#endif
struct orbit neptune = {
"Neptune        ",
2446800.5,
1.7697,
131.677,
250.623,
30.1631,
0.00594978,
0.009019,
254.2568,
2446800.5,
-6.87,
33.50,
#if DEPOLYN
0,
0,
#else
oneptune,
cneptune,
#endif
0.0,
0.0,
0.0
};

/* Note there are no perturbation formulas for Pluto.
 * The program automatically uses the given numerical
 * values since the pointers to perturbation subroutines
 * are null.
 * For some reason the J2000 orbit given for Pluto in the AA does
 * not give the same results as the "of date" orbit.  Yet, results
 * are the same for the other planets.
 */
struct orbit pluto = {
"Pluto          ",
2446640.5,
17.1346,
110.204,
114.21,
39.4633,
0.00397570,
0.248662,
355.0554,
2446640.5,
-1.0,
2.07,
0,
0,
0.0,
0.0,
0.0
};

/*
int otest(), ctest();
*/
struct orbit test = {
"Test orbit     ",
2446800.5,
1.8498,
49.457,
286.343,
1.523710,
0.524023,
0.093472,
53.1893,
2446800.5,
-1.52,
4.68,
/*
otest,
ctest,
*/
0,0,
0.0,
0.0,
0.0
};


/* coordinates of object
 */
int objnum = 0;	/* I.D. number of object */
double robject[3] = {0.0}; /* position */
/* ecliptic polar coordinates:
 * longitude, latitude in radians
 * radius in au
 */
double obpolar[3] = {0.0};

/* coordinates of Earth
 */
/* Heliocentric rectangular equatorial position
 * of the earth at time TDT re equinox J2000
 */
double rearth[3];
/* Corresponding polar coordinates of earth:
 * longitude and latitude in radians, radius in au
 */
double eapolar[3] = {0.0};

/* Julian date of ephemeris
 */
double JD = 0.0;
double TDT = 0.0;
double UT = 0.0;

/* flag = 0 if TDT assumed = UT,
 *      = 1 if input time is TDT,
 *      = 2 if input time is UT.
 */
int jdflag = 0;

/* correction vector, saved for display  */
double dp[3] = {0.0};

/* display formats for printf()
 */
extern char *intfmt, *dblfmt;

/* display enable flag
 */
int prtflg = 1;

/* log file enable flag
 */
int ephprint = 0;

/* Tabulation parameters
 */
static double  jdstart;
static double djd = 1.0;
static int ntab = 1;
static int itab;
struct orbit *elobject;	/* pointer to orbital elements of object */

/* Main program starts here.
 */
int
main()
{
int i;

kinit();

loop:

prtflg = 1;
printf( "Enter starting date of tabulation\n" );
JD = zgetdate(); /* date */
JD += gethms();	/* time of day */
update(); /* find UT and ET */
printf( "Julian day %.7f\n", JD );

getnum( "Enter interval between tabulations in days", &djd, dblfmt );
getnum( "Number of tabulations to display", &ntab, intfmt );
if( ntab <= 0 )
	ntab = 1;

loop1:

#if LIB403
librations();
goto loop;
#endif


getnum( "Planet number 0-9 or 88 to read star, 99 to read orbit",
	&objnum, intfmt );

switch(objnum)
	{
	case -1:
	  fclose( ephfile );
	  exit(0);
	case 0: elobject = 0;
		printf( "\n                   The Sun\n" );
		break;
	case 1: elobject = &mercury; break;
	case 2: elobject = &venus; break;
	case 3: elobject = 0;
		printf( "\n                   The Moon\n" );
		break;
	case 4: elobject = &mars; break;
	case 5: elobject = &jupiter; break;
	case 6: elobject = &saturn; break;
	case 7: elobject = &uranus; break;
	case 8: elobject = &neptune; break;
	case 9: elobject = &pluto; break;
	case 10: elobject = &test; break;
	case 11: librations(); goto loop;
	case 88:
morstar:	elobject = (struct orbit *)&fstar;
		i = getstar( (struct star *) elobject );
		if( i == 1 )
			goto loop1;
		if( i == 0 )
			break;
		goto operr;
	case 99:
		elobject = &forbit;
		i = getorbit( elobject );
		if( i == 1 )
			goto loop1;
		if( i == 0 )
			break;
	default:
operr:		printf( "Operator error.\n" );
		goto loop;
	}

if( elobject == (struct orbit *)&fstar )
	showcname( &elobject->obname[0] );
else if( elobject )
	printf( "\n                  %s\n", &elobject->obname[0] );

jdstart = JD;
for( itab=0; itab<ntab; itab++ )
	{
	  JD = jdstart  +  itab * djd;
	printf( "\nJD %.2f,  ", JD );
	update();
	if(ephfile)
	  fprintf( ephfile, "%.7f", JD );
/* Always calculate heliocentric position of the earth */
	kepler( TDT, &earth, rearth, eapolar );

	switch( objnum )
		{
		case 0: dosun(); iter_trnsit( dosun ); break;
		case 3: domoon(); iter_trnsit( domoon ); break;
		case 88: dostar(); iter_trnsit( dostar ); goto morstar;
		default: doplanet(); iter_trnsit( doplanet ); break;
		}
	printf( "\n" );
	if(ephfile)
	  fprintf( ephfile, "\n" );
	}
goto loop;
#if _MSC_VER
  return 0;
#endif
}

void EtoM(), MtoE(), mrotx(), midentity(), mmpy3();

void
librations()
{
  int i;
  double M[9], P[9], x, t;

#if LIB403
#define OBJNUMBER 1
#else
#define OBJNUMBER 13
#endif
  if( (i = jpl( JD, OBJNUMBER, robject, dp )) < 0 )
    printf("Error, jpl() returned %d\n", i);
  printf("Equatorial J2000 lunar libration angles:\n");
  printf("phidot   %24.16e\n", dp[0]);
  printf("phi      %24.16e\n", robject[0]);
  printf("thetadot %24.16e\n", dp[1]);
  printf("theta    %24.16e\n", robject[1]);
  printf("psidot   %24.16e\n", dp[2]);
  printf("psi      %24.16e", robject[2]);
  x = robject[2];
  x = x - 2.0 * PI * floor(0.5 + x / (2.0*PI));
  robject[2] = x;
  printf(" = %.16e\n", x);
  EtoM (robject, M);
  midentity(P);
  x = -84381.406173 * STR;
  printf("x = %.16e\n", x);
  mrotx (P, x);
  mmpy3 (M, P, M);
  MtoE (M, robject);
  printf("Ecliptic J2000 lunar libration angles:\n");
  printf("phi      %24.16e\n", robject[0]);
  printf("theta    %24.16e\n", robject[1]);
  printf("psi      %24.16e\n", robject[2]);
  t = (JD - J2000)/36525.0;
  /*  x = 5.28906030633174547188e8 + 1.73255933116181742528e9 * t; */

  x = 3.358283908822e5 + 1.73952725049277642528e9 * t;
  x = x - 1.296e6 * floor(0.5 + x / 1.296e6);
  x = x * STR;
  t = (x - robject[2]) * RTS;
	if (t < -6.48e5)
	  t += 1.296e6;
	if (t > 6.48e5)
	  t -= 1.296e6;
  printf("linear approx = %.5e rad, dif = %.3e\"\n", x, t);
}
