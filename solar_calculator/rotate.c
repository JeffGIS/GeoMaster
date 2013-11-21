/* Matrix and rotation subroutines
 */

#include "kep.h"

#include <stdio.h>
#if __STDC__
double zatan2(double, double);
double fabs(double);
double sqrt(double);
double sin(double);
double cos(double);
double atan2(double, double);
void midentity (double M[]);
void mmpy3 (double A[], double B[], double C[]);
void mrotate (int from, int to, double M[], double theta);
void EtoM (double e[], double M[]);
void epmat (double JD, double P[]);
#else
double zatan2(), fabs(), sqrt(), sin(), cos(), atan2();
void midentity();
void mmpy3();
void mrotate();
void EtoM();
void epmat();
#endif

#define N 3

/* Construct the matrix that represents the composite of
 * successive rotations by the three Euler angles:
 *   first by an angle phi about the z axis,
 *   second by an angle theta about the new x axis,
 *   third by an angle psi about the final z axis.
 * The input array e[] contains the three angles in the order
 * phi, theta, psi.  The 3x3 output matrix M[][] is the 
 * composite transformation of the three rotations in
 * Cartesian coordinates.
 *   The matrix elements have been written out directly
 * in trigonometric form so that the derivation of the
 * inverse function MtoE() will be clear.
 */
void
EtoM( e, M )
double e[];
double M[];
{
double a, b;
double sinpsi, cospsi, sinth, costh, sinphi, cosphi;

sinpsi = sin(e[2]);
cospsi = cos(e[2]);
sinth = sin(e[1]);
costh = cos(e[1]);
sinphi = sin(e[0]);
cosphi = cos(e[0]);
a = costh*sinphi;
b = costh*cosphi;
M[0] = cospsi*cosphi - a*sinpsi;  /* M[0][0] */
M[1] = cospsi*sinphi + b*sinpsi;  /* M[0][1] */
M[2] = sinpsi*sinth;
M[3] = -sinpsi*cosphi - a*cospsi;  /* M[1][0] */
M[4] = -sinpsi*sinphi + b*cospsi;
M[5] = cospsi*sinth;
M[6] = sinth*sinphi;  /* M[2][0] */
M[7] = -sinth*cosphi;
M[8] = costh;  /* M[2][2] */
}




/* Deduce the three Euler angles e[]
 * from the input coordinate rotation matrix M[][].
 * This is done by trigonometry from the elements of M.
 *
 * Since
 *   M[2][1] = -sin(theta) * cos(phi)
 *   M[2][0] =  sin(theta) * sin(phi),
 * then phi = arctan( M[2][0]/M[2][1].
 *
 * Similarly,
 *  M[0][2] = sin(psi) * sin(theta)
 *  M[1][2] = cos(psi) * sin(theta),
 * so psi = arctan( M[0][2]/M[1][2] ).
 *
 * Finally,
 *   M[2][2] = cos(theta)
 *   M[1][2]/cos(psi) = sin(theta),
 * which gives the arctangent of theta.
 */
void
MtoE( M, e )
double M[], e[];
{
double a, b, phi, theta, psi;

/* phi = zatan2( -M[2][1], M[2][0] ); */
phi = atan2(  M[2*3+0], -M[2*3+1]);
a = M[0*3+2];
b = M[1*3+2];
/* psi = zatan2( b, a ); */
psi = atan2( a, b );

if( fabs(a) > fabs(b) )
	a = a/sin(psi);
else
	a = b/cos(psi);

/* theta = zatan2( M[2][2], a ); */
theta = atan2( a, M[2*3+2]);

e[0] = phi;
e[1] = theta;
e[2] = psi;
}


/* Display elements of the NxN matrix M.
 */
void
prtmat( M )
double M[];
{
int r, c;

for( r=0; r<N; r++ )
	{
	for( c=0; c<N; c++ )
		printf( "%18.9f ", M[N*r+c] );
	printf( "\n" );
	}
printf( "\n" );
}

void
prtvec( v )
double v[];
{
int i;

for( i=0; i<N; i++ )
	printf( "%18.9f ", v[i] );
printf( "\n\n" );
}




/* Fill the array M with an NxN identity matrix.
 */
void
midentity( M )
double M[];
{
double *p;
int i;

p = &M[0];
for( i=0; i<(N*N); i++ )
	*p++ = 0.0;
for( i=0; i<N; i++ )
	M[i*(N+1)] = 1.0;
}



/* Matrix transpose.
 * B, the output, can occupy the same storage locations as
 * A, the input.
 */
void
mtransp( A, B )
double A[], B[];
{
double x, y;
int r, c, rc, cr;

for( r=0; r<N; r++ )
	{
	for( c=0; c<=r; c++ )
		{
		rc = N*r+c;
		cr = N*c+r;
		x = A[rc];
		y = A[cr];
		B[rc] = y;
		B[cr] = x;
		}
	}
}


/* 3x3 matrix multiply
 * C = A * B
 * A is on the left.
 * C may occupy the same storage as either A or B.
 */
void
mmpy3( A, B, C )
double A[], B[], C[];
{
double ans[9];
double s;
int i, r, c;

for( r=0; r<3; r++ )
	{
	for( c=0; c<3; c++ )
		{
		s = 0.0;
		for ( i=0; i<3; i++ )
			s += A[3*r+i] * B[3*i+c];
		ans[3*r+c] = s;
		}
	}

for( i=0; i<9; i++ )
	{
	C[i] = ans[i];
	}
}



/* Modify matrix M to include a rotation of theta radians
 * in the from-to plane.  The sense of the rotation is
 * from the "from" axis (labeled 0, 1, or 2) to the "to" axis.
 */
void
mrotate( from, to, M, theta )
int from, to;
double M[];
double theta;
{
double c;
double R[N*N];

if( from == to )
	{
	printf( "mrotate: from and to must be different\n" );
	return;
	}
midentity( R );
c = cos(theta);
R[N*from+from] = c;
R[N*to+to] = c;
c = sin(theta);
R[N*from+to] = c;
R[N*to+from] = -c;
mmpy3( R, M, M );
}


/* Modify the input matrix M to include a rotation
 * of theta radians about the x axis.
 */
void
mrotx( M, theta )
double M[];
double theta;
{
mrotate( 1, 2, M, theta );
}

/* Modify the input matrix M to include a rotation
 * of theta radians about the y axis.
 */
void
mroty( M, theta )
double M[];
double theta;
{
mrotate( 2, 0, M, theta );
}

/* Modify the input matrix M to include a rotation
 * of theta radians about the z axis.
 */
void
mrotz( M, theta )
double M[];
double theta;
{
mrotate( 0, 1, M, theta );
}



/* Transform the input vector v by rotating the coordinate system
 * theta radians about the x axis.
 */
void
rotx( v, theta )
double v[];
double theta;
{
double y, sinth, costh;

sinth = sin(theta);
costh = cos(theta);
y = costh*v[1] + sinth*v[2];
v[2] = -sinth*v[1] + costh*v[2];
v[1] = y;
}



/* Transform the input vector v by rotating the coordinate system
 * theta radians about the y axis.
 */
void
roty( v, theta )
double v[];
double theta;
{
double x, sinth, costh;

sinth = sin(theta);
costh = cos(theta);
x = costh*v[0] - sinth*v[2];
v[2] = sinth*v[0] + costh*v[2];
v[0] = x;
}


/* Transform the input vector v by rotating the coordinate system
 * theta radians about the z axis.
 */
void
rotz( v, theta )
double v[];
double theta;
{
double x, sinth, costh;

sinth = sin(theta);
costh = cos(theta);
x = costh*v[0] + sinth*v[1];
v[1] = -sinth*v[0] + costh*v[1];
v[0] = x;
}


/* Return the largest element-by-element difference between
 * two NxN arrays A and B.
 */
double l1diff( A, B )
double *A, *B;
{
double x, max;
int i;

max = 0;
for( i=0; i<N*N; i++ )
	{
	x = fabs( *A++ - *B++ );
	if( x > max )
		max = x;
	}
return( max );
}


/* The following subprogram, epmat( JD, P ),
 * constructs the precession matrix in ecliptic coordinates
 * to go from the epoch J2000.0 to the date JD.  Output P is the
 * 3x3 rotation matrix.  To go in the opposite direction,
 * from JD to J2000.0, the required matrix is just the transpose of P.
 * Note that the obliquity of Earth's equator is not required here
 * since the coordinates are ecliptic, not equatorial.
 */

#if (DE403 | DE404 | LIB403 | DE405 | DE406 | DE406CD)
/* James G. Williams, "Contributions to the Earth's obliquity rate,
   precession, and nutation,"  Astron. J. 108, 711-72s4 (1994).
   The values used here are slightly different, for DE403.  */
  /* 0.000000   5028.791959   1.105414   0.000076  -0.000024 */
static double pAcof[] = {
 -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
 -0.235316, 0.076, 110.5414, 50287.91959};

/* Pi from Williams' 1994 paper, in radians.  No change in DE403.  */
/* 629543.967373   -867.919986   0.153382   0.000026  -0.000004 */
static double nodecof[] = {
6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 1.9e-10, 
-3.54e-9, -1.8103e-7,  1.26e-7,  7.436169e-5,
-0.04207794833,  3.052115282424};

/* pi from Williams' 1994 paper, in radians.  No change in DE403.  */
/* 0.000000     46.997570  -0.033506  -0.000124   0.000000 */
static double inclcof[] = {
1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
-5.4000441e-11, 1.32115526e-9, -6.012e-7, -1.62442e-5,
 0.00227850649, 0.0 };

#else /* not DE403 */
/* Accumulated precession in longitude.  Coefficients are from:
 * J. Laskar, "Secular terms of classical planetary theories
 * using the results of general theory," Astronomy and Astrophysics
 * 157, 59070 (1986).
 */
static double pAcof[] = {
 -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
 -0.235316, 0.07732, 111.1971, 50290.966 };

/* Node and inclination of the earth's orbit computed from
 * Laskar's data as explained in the following paper:
 * P. Bretagnon and G. Francou, "Planetary theories in rectangular
 * and spherical variables. VSOP87 solutions," Astronomy and
 * Astrophysics 202, 309-315 (1988).
 */
static double nodecof[] = {
6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 6.3190131e-10, 
-3.48388152e-9, -1.813065896e-7, 2.75036225e-8, 7.4394531426e-5,
-0.042078604317, 3.052112654975 };

static double inclcof[] = {
1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
-5.4000441e-11, 1.32115526e-9, -5.998737027e-7, -1.6242797091e-5,
 0.002278495537, 0.0 };
#endif /* not DE403 */

/* radians per arc second */
#define STR 4.8481368110953599359e-6

void
epmat( JD, P )
double JD;
double P[];
{
double T, pA, i, Omega;
double e[3];
double *p;
int j;

/* Thousands of years from J2000.0.  */
T = (JD - 2451545.0)/365250.0;

/* Accumulated precession in longitude.  */
p = pAcof;
pA = *p++;
for( j=0; j<9; j++ )
	pA = pA * T + *p++;
pA *= STR * T;

/* Node of the moving ecliptic on the J2000 ecliptic.  */
p = nodecof;
Omega = *p++;
for( j=0; j<10; j++ )
	Omega = Omega * T + *p++;

/* Inclination of the moving ecliptic to the J2000 ecliptic.  */
p = inclcof;
i = *p++;
for( j=0; j<10; j++ )
	i = i * T + *p++;

/* Set up the Euler angles. */
e[0] = Omega;
e[1] = i;
e[2] = -(Omega + pA );

/* Construct the rotation matrix
  to go from J2000 to JD  */
EtoM( e, P );
}
