/* Calculate and display apparent coordinates of the sun at the
 * time given by the external variables TDT, UT.
 * Before calling this routine, the geometric position of the
 * earth must be calculated and put in rearth[].
 */

#include "kep.h"
extern struct orbit earth;
extern double rearth[];
extern double pearthb[];
extern double psunb[];
extern double coseps, sineps, nutl, dradt, ddecdt;
extern double Clightaud;

/* Apparent geocentric ra, dec of the Sun.  */
double psunapp[3];

int dosun()
{
double r, x, y, t;
double ecr[3], rec[3], pol[3];
double pearthbT[3], psunbT[3], rsunT[3], rsunL[3];
int i;
double asin(), modtp(), sqrt(), cos(), sin();

/* Display ecliptic longitude and latitude.
 */
for( i=0; i<3; i++ )
	rsunT[i] = -rearth[i];
r = eapolar[2];

if( prtflg )
	{
	lonlat( rsunT, TDT, pol, 1 );
	}

/* Rigorous light time iteration - AA page B39
 */
/* Save current earth and sun coordinates */
for( i=0; i<3; i++ )
  {
    pearthbT[i] = pearthb[i];
    psunbT[i] = psunb[i];
  }
/* Find the earth and sun at time TDT - t */
pol[2] = r;
for( i=0; i<2; i++ )
	{
	t = pol[2]/Clightaud;
	kepler( TDT-t, &earth, rsunL, pol );
	}
r = pol[2];

/* Apparent distance is the light time.  */
fprintf( ephfile, " %.10e", Clightaud * t );

for( i=0; i<3; i++ )
  {
    rsunL[i] = -rsunL[i];
/* Sun t days ago minus earth now.  */
    x = psunb[i] - pearthbT[i];
    ecr[i] = x;
/* Sun now minus earth now.  */
    y = psunbT[i] - pearthbT[i];
    rec[i] = y;
    pol[i] = y - x; /* change in position */
  }

if( prtflg )
	{
	printf( "light time %.4fm,  ", 1440.0*t );
	showcor( "aberration", ecr, pol );
	}

/* Estimate rate of change of RA and Dec
 * for use by altaz().
 */
deltap( rsunL, rsunT, &dradt, &ddecdt );  /* see dms.c */
dradt /= t;
ddecdt /= t;

/* There is no light deflection effect.
 * AA page B39.
 */

/* Annual aberration.  */
for( i=0; i<3; i++ )
    ecr[i] /= r;

annuab(ecr);

/* precess to equinox of date
 */
precess( ecr, TDT, -1 );

for( i=0; i<3; i++ )
    rec[i] = ecr[i];

/* Nutation.
 */
epsiln( TDT );
nutate( TDT, ecr );

ephprint = 1;
showrd( "    Apparent:", ecr, pol );
ephprint = 0;
for( i=0; i<3; i++ )
  psunapp[i] = pol[i];

/* Show the apparent ecliptic longitude (AA page C2).
   It is the geometric longitude plus nutation in longitude
   plus aberration. */
if( prtflg )
	{
	y  =  coseps * rec[1]  +  sineps * rec[2];
	y = zatan2( rec[0], y ) + nutl;
	printf( "Apparent longitude %.6f deg\n", RTD*y );
	}

/* Report altitude and azimuth. Here pol[] is ra and dec.
 */
pol[2] = r;
altaz( pol, UT );
return(0);
}
