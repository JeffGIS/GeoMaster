/* Program to solve Keplerian orbit
 * given orbital parameters and the time.
 * Returns Heliocentric equatorial rectangular coordinates of
 * the object.
 *
 * This program detects several cases of given orbital elements.
 * If a program for perturbations is pointed to, it is called
 * to calculate all the elements.
 * If there is no program, then the mean longitude is calculated
 * from the mean anomaly and daily motion.
 * If the daily motion is not given, it is calculated
 * by Kepler's law.
 * If the eccentricity is given to be 1.0, it means that
 * meandistance is really the perihelion distance, as in a comet
 * specification, and the orbit is parabolic.
 *
 * Reference: Taff, L.G., "Celestial Mechanics, A Computational
 * Guide for the Practitioner."  Wiley, 1985.
 */

#include "kep.h"
extern double pobjb[], pobjh[], vobjb[], vobjh[];

extern struct orbit earth;	/* orbital elements of the earth */

extern double eps, coseps, sineps; /* obliquity of ecliptic */

#if DEPOLYN

#ifndef ANSIPROT
int kepler0();
#endif

int kepler(J, e, rect, polar)
double J;
struct orbit *e;
double rect[];
double polar[];
{
int k;

if( (e == &earth) || (objnum != 99) )
	{
	k = kepjpl( J, e, rect, polar );
	return(k);
	}
else
	kepler0(J, e, rect, polar);
return(0);
}



int kepler0(J, e, rect, polar)
#else
int kepler(J, e, rect, polar)
#endif
double J, rect[], polar[];
struct orbit *e;
{
double alat, E, M, W, v, temp;
double epoch, inclination, ascnode, argperih;
double meandistance, dailymotion, eccent, meananomaly;
double r, coso, sino, cosa, sina, sinW, cosv, sinv;
int k;

#if !DEPOLYN
/* Compute orbital elements if a program for doing so
 * is supplied
 */
if( e->oelmnt )
	{
	k = (*(e->oelmnt) )(e,J);
	if( k == -1 )
		goto dobs;
	}
else if( e->celmnt )
	{ /* call B & S algorithm */
dobs:
	(*(e->celmnt) )( J, polar );
	polar[0] = modtp( polar[0] );
	E = polar[0]; /* longitude */
	e->L = E;
	W = polar[1]; /* latitude */
	r = polar[2]; /* radius */
	e->r = r;
	e->epoch = J;
	e->equinox = J;
	goto kepdon;
	}
#endif

/* Decant the parameters from the data structure
 */
epoch = e->epoch;
inclination = DTR * e->i;
ascnode = DTR * e->W;
argperih = DTR * e->w;
meandistance = e->a; /* semimajor axis */
dailymotion = e->dm;
eccent = e->ecc;
meananomaly = e->M;
/* Check for parabolic orbit. */
if( eccent == 1.0 )
	{
/* meandistance = perihelion distance, q
 * epoch = perihelion passage date
 */
	temp = meandistance * sqrt(meandistance);
	W = (J - epoch ) * 0.0364911624 / temp;
	E = 0.0;
	M = 1.0;
	while( fabs(M) > 1.0e-15 )
		{
		temp = E * E;
		temp = (2.0 * E * temp + W)/( 3.0 * (1.0 + temp));
		M = temp - E;
		if( temp != 0.0 )
			M /= temp;
		E = temp;
		}
	r = meandistance * (1.0 + E * E );
	M = atan( E );
	M = 2.0 * M;
	alat = M + argperih;
	v = M;
	goto parabcon;
	}
/* Calculate the daily motion, if it is not given.
 */
if( dailymotion == 0.0 )
	{
	dailymotion = 0.985607828/( meandistance * sqrt(meandistance) );
	}
dailymotion *= J - epoch;
/* M is proportional to the area swept out by the radius
 * vector of a circular orbit during the time between
 * perihelion passage and Julian date J.
 * It is the mean anomaly at time J.
 */
M = DTR*( meananomaly + dailymotion );
M = modtp(M);
/* If mean longitude was calculated, adjust it also
 * for motion since epoch of elements.
 */
if( e->L )
	{
	e->L += dailymotion;
	e->L = mod360( e->L );
	}

/* By Kepler's second law, M must be equal to
 * the area swept out in the same time by an
 * elliptical orbit of same total area.
 * Integrate the ellipse expressed in polar coordinates
 *     r = a(1-e^2)/(1 + e cosW)
 * with respect to the angle W to get an expression for the
 * area swept out by the radius vector.  The area is given
 * by the mean anomaly; the angle is solved numerically.
 * 
 * The answer is obtained in two steps.  We first solve
 * Kepler's equation
 *    M = E - eccent*sin(E)
 * for the eccentric anomaly E.  Then there is a
 * closed form solution for W in terms of E.
 */

E = M; /* Initial guess is same as circular orbit. */
temp = 1.0;
do
	{
/* The approximate area swept out in the ellipse */
	temp = E - eccent * sin(E)
/* ...minus the area swept out in the circle */
		- M;
/* ...should be zero.  Use the derivative of the error
 * to converge to solution by Newton's method.
 */
	E -= temp/(1.0 - eccent*cos(E));
	}
while( fabs(temp) > 1.0e-11 );

/* The exact formula for the area in the ellipse is
 *    2.0*atan(c2*tan(0.5*W)) - c1*eccent*sin(W)/(1+e*cos(W))
 * where
 *    c1 = sqrt( 1.0 - eccent*eccent )
 *    c2 = sqrt( (1.0-eccent)/(1.0+eccent) ).
 * Substituting the following value of W
 * yields the exact solution.
 */
temp = sqrt( (1.0+eccent)/(1.0-eccent) );
v = 2.0 * atan( temp * tan(0.5*E) );

/* The true anomaly.
 */
v = modtp(v);

meananomaly *= DTR;
/* Orbital longitude measured from node
 * (argument of latitude)
 */
if( e->L )
	alat = (e->L)*DTR + v - meananomaly - ascnode;
else
	alat = v + argperih; /* mean longitude not given */

/* From the equation of the ellipse, get the
 * radius from central focus to the object.
 */
cosv = cos( v );
r = meandistance*(1.0-eccent*eccent)/(1.0+eccent*cosv);

parabcon:

/* The heliocentric ecliptic longitude of the object
 * is given by
 *   tan( longitude - ascnode )  =  cos( inclination ) * tan( alat ).
 */
coso = cos( alat );
sino = sin( alat );
W = sino * cos( inclination );
E = zatan2( coso, W ) + ascnode;

/* The ecliptic latitude of the object
 */
sinW = sino * sin( inclination );
W = asin(sinW);

#if !DEPOLYN
kepdon:
/* Apply perturbations, if a program is supplied.
 */
if( e->celmnt )
	{
	e->L = E;
	e->r = r;
	(*(e->celmnt) )(e);
	E = e->L;
	r = e->r;
	W += e->plat;
	}

/* If earth, Adjust from earth-moon barycenter to earth
 * by AA page E2
 * unless orbital elements are calculated by formula.
 * (The Meeus perturbation formulas include this term for the moon.)
 */
if( (e == &earth) && (e->oelmnt == 0) )
	{
	temp = (J-2451545.0)/36525.0;
	temp = DTR*(298. + 445267.*temp); /* elongation of Moon from Sun */
	r += 3.076e-5 * cos(temp); /* au */
	E += 3.12e-5 * sin(temp); /* radians */
/*  same operation on rectangular coordinates:
	temp = DTR*(218. + 481268.*temp);
	rect[0] -= 3.12e-5*cos(temp);
	rect[1] -= 3.12e-5*sin(temp);
*/
	}
sinW = sin(W);
#endif

/* Output the polar cooordinates
 */
polar[0] = E; /* longitude */
polar[1] = W; /* latitude */
polar[2] = r; /* radius */


/* Convert to rectangular coordinates,
 * using the perturbed latitude.
 */
rect[2] = r * sinW;
cosa = cos(W);
rect[1] = r * cosa * sin(E);
rect[0] = r * cosa * cos(E);

/* Convert from heliocentric ecliptic rectangular
 * to heliocentric equatorial rectangular coordinates
 * by rotating eps radians about the x axis.
 */
epsiln( e->equinox );
W = coseps*rect[1] - sineps*rect[2];
M = sineps*rect[1] + coseps*rect[2];
rect[1] = W;
rect[2] = M;

/* Compute the velocity vector
 * from the orbital parameters
 */
coso = cos( ascnode );
sino = sin( ascnode );
cosa = cos( argperih );
sina = sin( argperih );
temp = cos( inclination );
cosv = r * cos(v);
sinv = r * sin(v);
vobjh[0] = (coso * cosa - sino * sina * temp ) * cosv
		- ( coso * sina + sino * cosa * temp ) * sinv;
vobjh[1] = ( sino * cosa + coso * sina * temp ) * cosv
		- ( sino * sina - coso * cosa * temp ) * sinv;
temp = sin( inclination );
vobjh[2] = sina * temp * cosv + cosa * temp * sinv;

/* Precess the position
 * to ecliptic and equinox of J2000.0
 * if not already there.
 */
precess( rect, e->equinox, 1 );
precess( vobjh, e->equinox, 1 );

for( k=0; k<3; k++ )
	{
	M = rect[k];
	pobjb[k] = M;
	pobjh[k] = M;
	vobjb[k] = vobjh[k];
	}
return(0);
}
