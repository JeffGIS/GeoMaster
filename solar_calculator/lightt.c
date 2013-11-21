/* Correction for light time from object to earth
 * including gravitational retardation due to the Sun.
 * AA page B36.
 */

#include "kep.h"
extern double pobjb[], pobjh[], pearthb[];
extern double Clightaud;

/* Calculated light time, in days.  */
double lighttime;

int lightt( elemnt, q, e )
double e[], q[];	/* rectangular position vectors */
struct orbit *elemnt;	/* orbital elements of object q */
{
double p[3], p0[3], ptemp[3], vearth0[3];
double P, Q, E, t, x, y, yh;
int i, k;


/* save initial q-e vector for display */
for( i=0; i<3; i++ )
	{
	p0[i] = q[i] - e[i];
	vearth0[i] = vearth[i];
	}

/* e = heliocentric earth at time TDT */
E = 0.0;
for( i=0; i<3; i++ )
	E += e[i]*e[i];
E = sqrt(E);

for( k=0; k<2; k++ )
	{
	P = 0.0;
	Q = 0.0;
	for( i=0; i<3; i++ )
		{
		y = pobjb[i];
/*
		if( k > 0 )
			y = pobjb[i];
		else
			y = q[i];
*/
		yh = q[i];
		x = y - pearthb[i];
		p[i] = x;
		Q += yh * yh;
		P += x * x;
		}
	P = sqrt(P);
	Q = sqrt(Q);
/* Note the following blows up if object equals sun. */
	t = (P + 1.97e-8 * log( (E+P+Q)/(E-P+Q) ) )/Clightaud;
	kepler( TDT-t, elemnt, q, ptemp );
	}

lighttime = t;
if( prtflg )
	printf( "light time %.4fm,  ", 1440.0*t );

/* Final object-earth vector and the amount by which it changed.
 */
for( i=0; i<3; i++ )
	{
	x = q[i] - e[i];
	p[i] = x;
	dp[i] = x - p0[i];
	}
showcor( "aberration", p0, dp );

/* Calculate dRA/dt and dDec/dt.
 * The desired correction of apparent coordinates is relative
 * to the equinox of date, but the coordinates here are
 * for J2000.  This introduces a slight error.
 *
 * Estimate object-earth vector t days ago.  We have
 * p(?) = q(J-t) - e(J), and must adjust to
 * p(J-t)  =  q(J-t) - e(J-t)  =  q(J-t) - (e(J) - Vearth * t)
 *         =  p(?) + Vearth * t.
 */
#if 0
/* Don't call this, because it overwrites pobj[]. */
velearth(TDT);
#endif
for( i=0; i<3; i++ )
	p[i] += vearth0[i]*t;

deltap( p, p0, &dradt, &ddecdt );  /* see dms.c */
dradt /= t;
ddecdt /= t;
return(0);
}
