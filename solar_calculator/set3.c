
#include "kep.h"

/* Compile this file only if DE102 ephemeris.  */
#if DE102

#define DEBUGG 0
#define AORB 0
int oparams(), set3del(), set3cor();

/* Convert DE118 to DE200 */
double j2000mat[] = {
 0.9999256791774783, -0.0111815116768724, -0.0048590038154533,
 0.0111815116959975,  0.9999374845751042, -0.0000271625775175,
 0.0048590037714450, -0.0000271704492210,  0.9999881946023742
};

int set3( re, redot, JD, objnum, o )
double re[]; /* equatorial */
double redot[];
double JD;
int objnum;
struct orbit *o;
{
double r[3];
double deltas3[6];
double *pr, *pm;
double u, v, JDE;
int i, j;
#if AORB
double rect[3], vel[3], polar[3]
double *pv;
#endif

JDE = B1950;
oparams( re, redot, JD, JDE, objnum, o );
set3del( JD, objnum, o, deltas3, AORB );
set3cor( re, redot, o, deltas3 );

#if AORB
kepv(JD, o, rect, vel, polar, 0); /* 0 = don't precess to J2000 */
#endif
/*
precess( rect, JDE, -1 );
precess( vel, JDE, -1 );
*/
pm = j2000mat;
for( i=0; i<3; i++ )
	{
	u = 0.0;
	v = 0.0;
#if AORB
	pr = rect;
	pv = vel;
#else
	pr = re;
#endif

	for( j=0; j<3; j++ )
		{
#if AORB
		u += *pm * *pr++;
		v += *pm++ * *pv++;
#else
		u += *pm++ * *pr++;
#endif
		}
	r[i] = u;
#if AORB
	redot[i] = v;
#endif
	}
for( i=0; i<3; i++ )
	re[i] = r[i];
o->equinox = J2000;
return(0);
}

/* Set III parameters (Brouwer and Clemence):
 * dl_0 is a change in the mean anomaly
 * dp is a differential rotation about the x axis
 * dq is a differential rotation about the y axis
 * dr is a differential rotation about the z axis
 * da is a change in the semi-major axis
 * de is a change in the eccentricity, e
 *
 * In terms of orbital orientation, if
 * dI is a change in the inclination, I
 * dW is a change in the node, W
 * dw is a change in the the argument of the perihelion, w
 * A = dI
 * B = sin IdW
 * then the following are differential formulas for the rotations:
 * dp =  A cos w + B sin w
 * dq = -A sin w + B cos w
 * dr = dw + cos IdW
 *
 * Coefficients in the table are ordered t^2, t, 1;
 * dl_0 + dr, dp, dq, e dr, da/a, de
 */
static double set3cof[] = {
 .00010, -.03330, -.63997,  .00002, -.00049, -.18676,
-.00001,  .00056, -.10567,  .00000,  .00020, -.13231,
 .00000,  .00000,  .00000,  .00000,  .00000, -.00020,

 .00022, -.04294, -.65261,  .00000, -.00026, -.15210,
 .00000,  .00000,  .08000,  .00000,  .00005, -.00422,
 .00000,  .00000,  .00000,  .00000,  .00000, -.00033,

 .00036, -.03791, -.65706,  .00000,  .00034, -.15399,
 .00000,  .00093,  .03259,  .00000,  .00008, -.01080,
 .00000,  .00000,  .00000,  .00000, -.00007,  .00013,

 .00000, -.00664, -.66103,  .00000, -.00101,  .08275,
 .00007, -.00087, -.14208,  .00000,  .00073, -.06178,
 .00000,  .00000,  .00000,  .00000,  .00019,  .00000,

 .00000, -.39304, -.67338,  .00000, -.00143, -.02460,
 .00000, -.00070, -.15128,  .00000,  .00000, -.04013,
 .00000,  .00000,  .00559,  .00000,  .00000,  .02466,

 .04676, -.19254, -.68056,  .00242, -.00252, -.08530,
-.00316, -.00388,  .06382,  .03115, -.05626, -.05454,
 .00000,  .00000,  .02418,  .00000,  .00000,  .00000,

-.24203, -.37558, -.60940, -.02195, -.04178,  .04462,
-.00958, -.01306,  .10435,  .14047,  .19064,  .00000,
 .03814,  .03943,  .00000,  .05512,  .00000,  .01286,

-1.11469,2.70019, -.52662, -.03502,  .03764, -.05354,
-.03346,  .01100, -.10839, -.11951,  .29747, -.26007,
-.14544,  .04393, -.52933, -.19803, -.14760,  .31788,

 .00000, 1.53554,  .31296,  .00000,  .01414, -.14721,
 .00890, -.00774,  .32918, -.11620,  .00000,  .09530,
 .12534, -.06691, -.59697,  .14817, -.14922,  .55239,

1.18544, -.62624, -.57722,  .00000,  .00000,  .00000,
 .00000,  .00000,  .00000,  .00000,  .00000, -.03745,
 .00000,  .00000, -.01621,  .00000,  .00000,  .00000,
};


int set3del( JD, objnum, o, deltas3, oflag )
double JD;
int objnum;
struct orbit *o;
double deltas3[];
int oflag; /* 1 = adjust the orbit, 0 = don't. */
{
double *p, *q;
double T, c, d, A, B;
double sinw, cosw, sinW, cosW, sini, cosi;
int i, j;
double sqrt(), sin(), cos();

c = DTR * o->w;
sinw = sin(c);
cosw = cos(c);
c = DTR * o->W;
sinW = sin(c);
cosW = cos(c);
c = DTR * o->i;
sini = sin(c);
cosi = cos(c);

T = (JD - 2433282.5)/36525.0;
p = &set3cof[ 18 * (objnum - 1) ];
q = deltas3;
for( i=0; i<6; i++ )
	{
	d = *p++;
	for( j=0; j<2; j++ )
		{
		d = d * T  +  *p++;
		}
	*q++ = d * STR;
#if DEBUGG
	printf( "%.2e ", *(q-1) );
#endif
	}
#if DEBUGG
printf( "\n" );
#endif

if( objnum == 10 )
	{ /* periodic corrections */
	c = 2.0 * PI * 36525.0 * T;
	A = c / 6798.36;
	B = c / 3231.48;
	deltas3[0] += .01289 * STR * cos(A);
	c = STR * cos(B);
	d = STR * sin(B);
	deltas3[1] += .07427 * c + .13866 * d;
	deltas3[2] += .14255 * c - .07364 * d;
	}

c = deltas3[1] * cosw - deltas3[2] * sinw;
c *= RTD;
A = (deltas3[1] * sinw + deltas3[2] * cosw) / sini;
A *= RTD;
d = deltas3[3]/o->ecc;
d *= RTD;
B = RTD * deltas3[0] - d;
if( oflag )
	{
	o->i += c;
	o->W += A;
	o->w += d - A * cosi;
	o->M += B;
	o->a *= 1.0 + deltas3[4];
	o->ecc += deltas3[5];
	}
#if DEBUGG
printf( "di %.2e dW %.2e dw %.2e dM %.2e da %.2e de %.2e\n",
 c, A, d, B, deltas3[4], deltas3[5] );
#endif
return(0);
}



int set3cor( re, redot, o, deltas3 )
double re[];
double redot[];
struct orbit *o;
double deltas3[];
{
double *p, *q;
double c, d, e;
double sinw, cosw, sinW, cosW, sini, cosi;
double Px, Py, Pz, Qx, Qy, Qz, Rx, Ry, Rz;
double H, K, r, rrdot;
double difmat[18];
int i, j;
double sqrt(), sin(), cos();

c = DTR * o->w;
sinw = sin(c);
cosw = cos(c);
c = DTR * o->W;
sinW = sin(c);
cosW = cos(c);
c = DTR * o->i;
sini = sin(c);
cosi = cos(c);

Px =  cosw * cosW - sinw * sinW * cosi;
Qx = -sinw * cosW - cosw * sinW * cosi;
Rx =  sinW * sini;
Py =  (cosw * sinW + sinw * cosW * cosi) * coseps
     - sinw * sini * sineps;
Qy = (-sinw * sinW + cosw * cosW * cosi) * coseps
     -cosw * sini * sineps;
Ry = -cosW * sini * coseps - cosi * sineps;
Pz =  (cosw * sinW + sinw * cosW * cosi) * sineps
     + sinw * sini * coseps;
Qz = (-sinw * sinW + cosw * cosW * cosi) * sineps
     + cosw * sini * coseps;
Rz = -cosW * sini * sineps + cosi * coseps;
r = re[0] * re[0] + re[1] * re[1] + re[2] * re[2];
r = sqrt(r);
rrdot = re[0] * redot[0] + re[1] * redot[1] + re[2] * redot[2];

e = o->ecc;
d = 1.0 - e * e;
H = (r - o->a * (1.0 + e * e))
  / (o->a * e * d);
K = 1.0 + r / (o->a * d);
d = DTR * o->dm;
c = o->a * d;
K = K * rrdot / (c * c * e);

p = difmat;
*p++ = redot[0]/d;
*p++ = Py * re[2] - Pz * re[1];
*p++ = Qy * re[2] - Qz * re[1];
*p++ = (-redot[0]/d + Ry * re[2] - Rz * re[1])/e;
*p++ = re[0];
*p++ = H * re[0] + K * redot[0];

*p++ = redot[1]/d;
*p++ = Pz * re[0] - Px * re[2];
*p++ = Qz * re[0] - Qx * re[2];
*p++ = (-redot[1]/d + Rz * re[0] - Rx * re[2])/e;
*p++ = re[1];
*p++ = H * re[1] + K * redot[1];

*p++ = redot[2]/d;
*p++ = Px * re[1] - Py * re[0];
*p++ = Qx * re[1] - Qy * re[0];
*p++ = (-redot[2]/d + Rx * re[1] - Ry * re[0])/e;
*p++ = re[2];
*p++ = H * re[2] + K * redot[2];

p = difmat;
for( i=0; i<3; i++ )
	{
	q = deltas3;
	d = 0.0;
	for( j=0; j<6; j++ )
		{
		d += *p++  *  *q++;
		}
	re[i] += d;
	}
return(0);
}

#endif  /* DE102 */
