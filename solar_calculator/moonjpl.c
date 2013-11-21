 

/* Expansions for the geocentric ecliptic longitude, latitude,
 * and equatorial horizontal parallax of the Moon referred to
 * the mean equinox of date.
 *
 * Corrections for nutation and light time are included.
 * Note that the difference between Ephemeris and Universal
 * time is significant here, since the Moon moves in right
 * ascension at the rate of about 2s/minute.
 *
 * - S. L. Moshier, October, 1987
 */

#include "kep.h"
#ifndef ANSIPROT
int moonll();
#endif

/* Rate of motion in R.A. and Dec., computed by this program.
 */
extern double dradt, ddecdt;

/* Obliquity of the ecliptic, computed by epsiln(), and nutations:
 */
extern double eps, coseps, sineps, nuto, nutl;

/* Answers posted by angles():
 */
extern double pq, qe, ep;
extern double pearthb[];

/* Nutation, computed by nutlo():
 */
extern double nutl, nuto;

/* The following times are set up by update() and refer
 * to the same instant.  The distinction between them
 * is required by altaz().
 */
extern double TDT; /* Ephemeris time */
extern double UT;  /* Universal time */
extern double Clightaud;

static double l;		/* Moon's ecliptic longitude */
static double B;		/* Ecliptic latitude */
static double p;		/* Parallax */
static double ra;		/* Right Ascension */
static double dec;	        /* Declination */
static double Mapp[3];          /* Apparent ecliptic coordinates */
static double Rem;              /* Earth - Moon distance in au */
static double moon_light_time;

/* geocentric equatorial velocity vector returned by kepjpl()
 */
extern double vobj[];

/* Program begins:
 */
int domoon()
{
int i, prtsave;
double x, y, z, r;
double pp[3], qq[3], re[3], pe[3];
double acos();

prtsave = prtflg;
prtflg = 0; /* disable display */
/* Get apparent coordinates for the earth.  */
for (i = 0; i < 3; i++)
  re[i] = rearth[i];
r = re[0] * re[0] + re[1] * re[1] + re[2] * re[2];
r = sqrt(r);
for (i = 0; i < 3; i++)
  re[i] /= r;
annuab( re );  /* aberration of light.  */
/* pe[0] -= STR * (20.496/(RTS*pe[2])); */
precess( re, TDT, -1 );
nutate( TDT, re );
for (i = 0; i < 3; i++)
  re[i] *= r;
lonlat( re, TDT, pe, 0 );
prtflg = prtsave; /* reenable display */

/* Calculate for present instant.
 */
moonll( TDT, pp );

/* Apparent distance is the light time.  */
if(ephprint)
  fprintf( ephfile, " %.10e", Clightaud * moon_light_time );

/* Post the ecliptic longitude and latitude, in radians,
 * and the radius in au.
 */
/*
obpolar[0] = l;
obpolar[1] = B;
obpolar[2] = Rem;
*/
if (prtflg)
  printf("Geometric lon %.5f, lat %.5f, rad %.7e\n",
	 RTD * obpolar[0], RTD * obpolar[1], obpolar[2]);

/* Find sun-moon-earth angles */
for( i=0; i<3; i++ )
	qq[i] = re[i] + pp[i];
angles( pp, qq, re );

/* Display answers
 */
if( prtflg )
	{
	printf( "Apparent geocentric longitude %.7f deg", RTD * l );
	dms( l );
	printf( "\n   Latitude %.7f deg", RTD * B );
	dms( B );
	r = Rem / Rearthau;
	printf( "\nDistance %.8f Earth-radii\n", r );
	printf( "Horizontal parallax" );
	dms( p );
	printf( "Semidiameter" );
	x = 0.272453 * p  +  0.0799/RTS; /* AA page L6 */
	dms( x );

	x = RTD * acos(-ep);
	printf( "\nElongation from sun %.2f deg,", x );
	x = 0.5 * (1.0 + pq);
	printf( "  Illuminated fraction %.2f\n", x );

/* Find phase of the Moon by comparing Moon's longitude
 * with Earth's longitude.
 *
 * The number of days before or past indicated phase is
 * estimated by assuming the true longitudes change linearly
 * with time.  These rates are estimated for the date, but
 * do not stay constant.  The error can exceed 0.15 day in 4 days.
 */
	x = l - pe[0];
	x = modtp( x ) * RTD;	/* difference in longitude */
	i = (int) (x/90);	/* number of quarters */
	x = (x - i*90.0);	/* phase angle mod 90 degrees */

/* days per degree of phase angle */
	z = Rem/(12.3685 * 0.00257357);

	if( x > 45.0 )
		{
		y = -(x - 90.0)*z;
		printf( "Phase %.1f days before ", y );
		i = (i+1) & 3;
		}
	else
		{
		y = x*z;
		printf( "Phase %.1f days past ", y );
		}

	switch(i)
		{
		case 0: printf( "Full Moon\n" ); break;
		case 1: printf( "Third Quarter\n" ); break;
		case 2: printf( "New Moon\n" ); break;
		case 3: printf( "First Quarter\n" ); break;
		}
	} /* if prtflg */

if (prtflg)
  {
    ephprint = 1;
    printf( "    Apparent:  R.A." );
    hms(ra);
    printf( "Declination" );
    dms(dec);
    ephprint = 0;
    printf( "\n" );
    printf( "RA = %.6f deg, Dec = %.6f deg, pi = %.7f deg\n",
	    RTD*ra, RTD*dec, RTD*p );
  }
/* Compute and display topocentric position (altaz.c)
 */
pp[0] = ra;
pp[1] = dec;
pp[2] = r * Rearthau;
altaz( pp, UT );
return(0);
}



/* Calculate latitude, longitude, and horizontal parallax
 * of the Moon at Julian date J (time scale is TDT).
 */

int moonll( J, pp )
double J, pp[];
{
double y, z, c, s;
double pol[3], pp0[3], qq[3];
int i, k, prtsave;

kepler(J, (struct orbit *) 0, pp, pol); /* J2000 coordinates */

for(i = 0; i < 3; i++)
  pp0[i] = pp[i];

/* if( prtflg ) */
prtsave = prtflg;
prtflg = 0;
lonlat( pp, TDT, obpolar, 1 );
prtflg = prtsave;

/* Light time.  */
for(k=0; k<3; k++)
  {
    z = pp[0] * pp[0] + pp[1] * pp[1] + pp[2] * pp[2];
    z = sqrt(z) / Clightaud;
    kepler(J-z, (struct orbit *) 0, pp, pol);
  }
moon_light_time = z;
if (prtflg)
  printf( "Light time = %.5f sec\n", moon_light_time * 86400.0 );

/* Approximate rates of change in R.A., Dec.  */
deltap( pp, pp0, &dradt, &ddecdt );  /* see dms.c */
dradt /= z;
ddecdt /= z;

/* Find Euclidean vectors and angles between earth, object, and the sun
 */
for( i=0; i<3; i++ )
	qq[i] = rearth[i] + pp[i];
angles( pp, qq, rearth );

/* Make pp a unit vector.  */
for (i = 0; i < 3; i++)
  pp[i] /= EO;

/* Light deflection (ignore).  */
/* relativity( pp, qq, rearth ); */

/* Annual aberration.  */
/*annuab (pp);*/

/* Precession of the equinox and ecliptic
 * from J2000.0 to ephemeris date
 */
precess( pp, TDT, -1 );

/* Correct for nutation at date TDT.
 */
nutate( TDT, pp );

/* Restore earth-moon distance
   and save for apparent ecliptic coordinates.  */
for( i=0; i<3; i++ )
  {
    z = pp[i] * EO;
    Mapp[i] = z;
    pp[i] = z;
  }

#if 0
/* Return ecliptic polar coordinates l, B, p
   re ecliptic and mean equinox of date   */
prtsave = prtflg;
prtflg = 0;
lonlat( pp, TDT, pol, 0 );
prtflg = prtsave;
#else
/* For apparent ecliptic coordinates, rotate from the true
   equator back into the ecliptic of date.  */
c = cos(eps+nuto);
s  = sin(eps+nuto);
y = c * pp[1] + s * pp[2];
z = -s * pp[1] + c * pp[2];
pol[0] = zatan2( pp[0], y );
pol[1] = asin(z/EO);
#endif

l = pol[0];
B = pol[1];
Rem = pol[2];
p = asin( Rearthau/Rem );

/* Equatorial polar */
ra = zatan2(pp[0],pp[1]);
dec = asin(pp[2]/Rem);
return 0;
}
