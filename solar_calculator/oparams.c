/* Calculate orbital elements from position and velocity
 */
#include "kep.h"

extern double RTD, DTR;
extern double coseps, sineps, J2000;
/* square of Gaussian gravitational constant */
#define GMsun 2.959122082855911025e-4
#if (DE400 | DE403 | DE404 | DE405 | DE406 | DE406CD | LIB403)
/* DE403 GM values (from ephemeris file header)  */
double masses[9] = {
 4.91254745145081187e-11, /* Mercury */
 7.24345248616270270e-10, /* Venus */
#if (DE405 | DE406 | DE406CD)
 8.99701134671249882e-10, /* Earth + Moon */
#else
 8.99701137429187710e-10, /* Earth + Moon */
#endif
/* GMearth =  8.887692461152135779326e-10 */
/* GMmoon = 1.093189238570932273024e-11 */
 9.54953510577925806e-11, /* Mars */
 2.82534590952422643e-07, /* Jupiter */
 8.45971518568065874e-08, /* Saturn */
 1.29202491678196939e-08, /* Uranus */
 1.52435890078427628e-08, /* Neptune */
 2.18869976542596968e-12, /* Pluto */
};
/* Solar mass ratios
6.023599999999999968622e+06
4.085237100000000076250e+05
3.289005584999999654769e+05
3.098708000000008070629e+06
1.047348599999994977261e+03
3.497897999999657838721e+03
2.290293999940287419115e+04
1.941224000026145472120e+04
1.349999999835724255972e+08
*/
#endif
#if DE245
/* DE245 GM values (double precision, from ephemeris file)
       GMS =  2.959122082855910945351e-04;  */
double masses[9] = {
 4.912547451450811873975e-11,
 7.243452486162702697978e-10,
 8.997011385009229269848e-10,
 9.549535105779258058519e-11,
 2.825345909524226428462e-07,
 8.459715185680658737900e-08,
 1.292027173338034899520e-08,
 1.524358900784276282261e-08,
 2.191942283863699240499e-12
 };
/* Solar mass ratios from above
6.023599999999999968622e+06
4.085237100000000076250e+05
3.289005599999999903389e+05
3.098708000000008070629e+06
1.047348599999994977261e+03
3.497897999999657838721e+03
2.290293999940287419115e+04
1.941224000026145472120e+04
1.349999999835724255972e+08
*/
#endif

/* Note, these are not the DE102 masses.  */
#if DE200 | DE200CD | SSYSTEM | DE102
/* DE200 GM values */
double masses[] = {
4.912547451450812e-011,
7.243456209632766e-010,
8.997011658557308e-010,
9.549528942224058e-011,
2.825342103445926e-007,
8.459468504830660e-008,
1.288816238138035e-008,
1.532112481284276e-008,
2.276247751863699e-012
};
/* Almanac mass ratios */
/*
GMsun/6023600.,
GMsun/408523.5,
GMsun/328900.5,
GMsun/3098710.,
GMsun/1047.355,
GMsun/3498.5,
GMsun/22869.,
GMsun/19314.,
GMsun/3000000.
*/

/* Equivalent DE200 mass ratios
 6023600.
  408523.5
  328900.55
 3098710.
 1.047350010905519e+003
 3.497999999841770e+003
 2.296000000070594e+004
 1.931400023825575e+004
 1.300000002386868e+008
*/
#endif

/*
double DTR = 1.7453292519943295769e-2;
double RTD = 5.7295779513082320877e1;
*/
#define TPI 6.2831853071795864769

/* GM for earth + moon system */
#if DE102
#define GMemb 8.997012205653517873626E-10
#endif
#if DE200 | DE200CD | SSYSTEM
#define GMemb 8.997011658557308e-10
#endif
#if DE245
#define GMemb 8.9970113850092293e-10
#endif
#if DE400
#define GMemb 8.997011426041440837480e-10
#endif
#if DE403 | LIB403 | DE404
#define GMemb 8.99701137429187710e-10
#endif
#if DE405 | DE406 | DE406CD
#define GMemb 8.99701134671249882e-10
#endif

/* Gaussian gravitational constant k */
#define KG 0.01720209895
/* 180 k / pi */
#define GMN 0.985607668601424903218
/* (180 k /pi)**2 */
#define GMS 0.971422476405936217038

int oparams( re, rdote, J, JDE, objnum, orb )
double re[]; /* position vector, equatorial */
double rdote[]; /* velocity vector, equatorial */
double J; /* Julian day number */
double JDE; /* epoch of equinox of orbit */
struct orbit *orb;
int objnum;
{
double r[3]; /* position vector, ecliptic */
double rdot[3]; /* velocity vector, ecliptic */
double a; /* semimajor axis */
double e; /* eccentricity */
/* double w; */ /* arg perihelion */
double i; /* inclination */
double W; /* ascending node */
/* double T; */ /* time of perihelion */
double n; /* daily motion */
double E; /* eccentric anomaly */
double M; /* mean anomaly */
double v; /* true anomaly */
double GM;
double rdsq, rm, rrdot, p, q, c, s;
double cosW, sinW;
int j;
double fabs(), sqrt(), sin(), cos(), acos(), asin(), zatan2();

if( objnum == 10 )
	{
	GM = GMemb;
	}
else
	{
	GM = GMsun + masses[objnum-1];
	}
/* Convert from equatorial to ecliptic coordinates
 */
epsiln(JDE);
p = re[1];
q = re[2];
r[0]  = re[0];
r[1]  =  coseps * p  +  sineps * q;
r[2]  = -sineps * p  +  coseps * q;
p = rdote[1];
q = rdote[2];
rdot[0]  = rdote[0];
rdot[1]  =  coseps * p  +  sineps * q;
rdot[2]  = -sineps * p  +  coseps * q;

rdsq = 0.0; /* squared magnitude of velocity vector */
rm = 0.0; /* magnitude of radius vector */
rrdot = 0.0; /* inner product of r and rdot */
for( j=0; j<3; j++ )
	{
	p = rdot[j];
	rdsq += p * p;
	q = r[j];
	rm += q * q;
	rrdot += p * q;
	}
/*
 *                        2
 *   1       2      |rdot|
 *  ---  =  ---  -  ------
 *   a      |r|       GM
 *
 */
rm = sqrt(rm);
p = rdsq / GM;
q = 2.0/rm - p;
if( q <= 0.0 )
	{
notellipse:
	printf( "Motion not elliptical\n" );
	return(-1);
	}	
a = 1.0 / q;
n = sqrt( GM / (a*a*a) ); /* radians per day */

/*
 *              r . rdot 
 * e sin(E)  =  --------
 *                   2
 *                n a
 *
 *
 *                  |r|
 * e cos(E)  =  1 - ---
 *                   a
 *
 *          2  3
 *  GM  =  n  a
 *
 */
q = rm * p - 1.0;
p = rrdot / sqrt( GM * a );
E = zatan2( q , p );
e = sqrt( p * p + q * q );
if( e >= 1.0 )
	goto notellipse;
/*
 *  E - e sin(E) = M (mean anomaly)
 */
s = sin(E);
M = E - e * s;

/* True anomaly
 */
c = cos(E);
q = 1.0 - e * c;
p = s * sqrt( 1.0 - e * e ) / q;
q = (c - e)/q;
v = zatan2( q, p );

/*
 *  L = angular momentum = r X rdot,   X = vector cross product
 *
 *     2                2
 *  |L|   =  GM a (1 - e )
 *
 *
 *            (  sin(i) sin(W) )     ( r[2] rdot[3] - rdot[2] r[3] )
 *  L  =  |L| ( -sin(i) cos(W) )  =  ( r[3] rdot[1] - rdot[3] r[1] )
 *            (  cos(i)        )     ( r[1] rdot[2] - rdot[1] r[2] )
 *
 * 1, 2, 3 <-> x, y, z
 */

p = r[1] * rdot[2] - rdot[1] * r[2];
q = r[2] * rdot[0] - rdot[2] * r[0];
s = r[0] * rdot[1] - rdot[0] * r[1];
W = zatan2( -q, p );

rm = a * GM * (1.0 - e * e );
rm = sqrt( rm );

/*
rm = p * p  +  q * q  +  s * s;
rm = sqrt( rm );
*/

sinW = sin(W);
cosW = cos(W);
if( fabs(p) > fabs(q) )
	p = p / sinW;
else
	p = -q / cosW;

c = s / rm;	/* cos(i) */
s = p / rm;	/* sin(i) */
i = asin( s );

/* Argument of latitude
 */
p = r[0] * cosW + r[1] * sinW;    /* = r cos(s) */
q = (-r[0] * sinW + r[1] * cosW); /* = r sin(s) */
q = q * c  +  r[2] * s;
s = zatan2( p, q );

/* Arg perihelion + true anomaly = argument of latitude
 */
p = s - v;
/*w = p + W;*/
if( p >= TPI )
	p -= TPI;
if( p < 0.0 )
	p += TPI;

/* Mean longitude = perihelion + mean anomaly
 */
/*
s = M + w;
if( s >= TPI )
	s -= TPI;
if( s < 0.0 )
	s += TPI;
*/

orb->epoch = J;
orb->i = RTD * i;
orb->W = RTD * W;
orb->w = RTD * p;
orb->a = a;
orb->dm = RTD * n;
orb->ecc = e;
orb->M = RTD * M;
orb->equinox = JDE;
orb->oelmnt = 0;
orb->celmnt = 0;
orb->L = 0.0;
orb->r = 0.0;
orb->plat = 0.0;
return(0);
}
