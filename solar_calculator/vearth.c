
/* Calculate the velocity vector of the earth
 * as it moves in its elliptical orbit.
 * The difference in velocity between this and assuming
 * a circular orbit is only about 1 part in 10**4.
 *
 * Note that this gives heliocentric, not barycentric, velocity.
 *
 * Input is Julian date.  Output left in global array vearth[].
 */

/* Barycentric position and velocity of the earth
 * at time jvearth.
 */
double jvearth = -1.0;
double vearth[3] = {0.0};
double pearthb[3] = {0.0};
extern double vearth[], pearthb[];

#include "kep.h"

extern struct orbit earth;

int velearth( J )
double J;
{
double e[3], p[3];
#if DEBUG
double x[3], A, q;
int i;
#endif

if( J == jvearth )
	return(0);

jvearth = J;

#if DEPOLYN
kepler( TDT, &earth, e, p );
#else
/* calculate heliocentric position of the earth
 * as of a short time ago.
 */
q = 0.005;
kepler( TDT-q, &earth, e, p );
for( i=0; i<3; i++ )
	vearth[i] = (rearth[i] - e[i])/q;
#endif
#if DEBUG
/* Generate display for comparison with Almanac values. */
A = 0.0;
for( i=0; i<3; i++ )
	{
	q = vearth[i];
	A += q*q;
	x[i] = q;
	}
A = sqrt(A);
precess( x, TDT, 1 );
printf( "Vearth %.6e, X %.6f, Y %.6f, Z %.6f\n", A, x[0], x[1], x[2] );
#endif
return(0);
}
