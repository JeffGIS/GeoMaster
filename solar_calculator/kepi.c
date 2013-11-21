/* Read and interpolate de118i ephemeris.  */


#include "kep.h"

/* Define this or not in the makefile.  Use kepjpl.c when not defined.  */
#if SSYSTEM

/* Interpolator degree */
#define NINTERP 12

/* Include constants for appropriate ephemeris */
#include "de118i.h"
/* long rec0 = 0L; */
/* long recsiz = 584L; */
double JDi, JDb;
extern double au;  /* Kilometers per au */
extern double emrat; /* Earth/Moon mass ratio */

/* System header files required for I/O.
   Define IBMPC, etc. 1 or 0 in kep.h.  */

#ifdef _MSC_VER
#if _MSC_VER >= 1000
#include <stdlib.h>
#endif
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#endif

#if UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

extern double pearthb[];
static long bcnt = 0;
extern struct orbit earth;
extern double coseps, sineps, J2000;

int defd = 0;
extern int defd;
extern char defile[];

double zatan2(), asin(), sqrt();

/* heliocentric velocity of given object (geocentric, if moon) */
double pobjb[3];
double pobjh[3];
double vobjb[3];
double vobjh[3];
double psunb[3];
double vsunb[3];


int kepjpl(J, e, rect, polar)
double J, rect[], polar[];
struct orbit *e;
{
double JD, ratio, x, y, z;
double pm[3], vm[3];
int nobj, i;

floop:

/* Open up the ephemeris file.  */
if( defd <= 0 )
	{
	if( defd < 0 )
		{
		printf( "Enter DE file name ? " );
		gets( &defile[0] );
		}
	defd = open( &defile[0], O_BINARY | O_RDONLY, S_IREAD );
	if( defd <= 0 )
		{
		printf( "Can't find DE file <%s>\n", &defile[0] );
		defd = -1;
		goto floop;
		}
	printf( "Opened %s\n", &defile[0] );

/* rec0 is assumed initialized properly in de245.h or de200.h or de102.h */
	bcnt = rec0;
	lseek( defd, bcnt, SEEK_SET );
	read( defd, &JDb, 8 );
/* The number of days covered by one record is the difference between
   the dates on two adjacent records.  */
	lseek( defd, bcnt+recsiz, SEEK_SET );
	read( defd, &JDi, 8 );
	JDi -= JDb;
/*#if DEBUG*/
#if 1
	printf( "First date in file = %.8E\n", JDb );
#endif
	}

nobj = objnum;
if( nobj == 3 )
	nobj = 10;	/* moon */
if( nobj == 0 )
	nobj = 3;	/* sun */
JD = tdb(J);

/* Sun */
#if DEBUG
printf( "psunb, vsunb\n" );
#endif
if( jpl( JD, 11, psunb, vsunb ) )
	{
	printf( "Error, DE file boundary violation\n" );
	exit(0);
	}

if( (e == &earth) || (nobj == 3) || (nobj == 10) )
	{
/* Moon, geocentric
 */
#if DEBUG
	printf( "pgmoon, vgmoon\n" );
#endif
	jpl( JD, 10, pm, vm );
	if( e != &earth )
		{
		for( i=0; i<3; i++ )
			{
			vobjb[i] = vm[i];
			pobjb[i] = pm[i];
			vobjh[i] = vm[i];
			pobjh[i] = pm[i];
			}
		goto output;
		}


/* Earth-Moon barycenter
 */
#if DEBUG
	printf( "pemb, vemb\n" );
#endif
	jpl( JD, 3, pobjb, vobjb );
/* Heliocentric EMB */
	for( i=0; i<3; i++ )
		{
		pobjh[i] = pobjb[i] - psunb[i];
		vobjh[i] = vobjb[i] - vsunb[i];
		}
/* Heliocentric Earth
 */
	ratio = 1.0/(emrat+1.0);
	for( i=0; i<3; i++ )
		{
		pobjh[i] = pobjh[i] - ratio * pm[i];
		vobjh[i] = vobjh[i] - ratio * vm[i];
		}

/* Earth position and velocity re solar system barycenter
 */
#if DEBUG
	printf( "barycentric earth = pemb - pgmoon/(1+emratio)\n" );
#endif
	for( i=0; i<3; i++ )
		{
		pearthb[i] = pobjh[i] + psunb[i];
		vearth[i] = vobjh[i] + vsunb[i];
		}
	jvearth = JD;
	}
else
	{
/* Selected object, not Earth or Moon
 */
#if DEBUG
	printf( "pobjb, vobjb\n" );
#endif
	jpl( JD, nobj, pobjb, vobjb );
	for( i=0; i<3; i++ )
		{
		pobjh[i] = pobjb[i] - psunb[i];
		vobjh[i] = vobjb[i] - vsunb[i];
		}
#if DEBUG
	printf( "pobjb-psunb\n" );
	prvec(pobjh);
	prvec(vobjh); /* heliocentric velocity */
#endif
	}

output:

/* Write out the heliocentric object coordinates */
for( i=0; i<3; i++ )
	rect[i] = pobjh[i];
epsiln(J2000);
x = pobjh[0];
y = pobjh[1];
z = pobjh[2];
/* radius distance */
ratio = x*x + y*y + z*z;
polar[2] = sqrt(ratio);
/* Convert from equatorial to ecliptic coordinates */
ratio  =  coseps * y  +  sineps * z;
z  = -sineps * y  +  coseps * z;
y = ratio;
/* convert from rectangular to polar */
polar[0] = zatan2( x, y );
polar[1] = asin( z/polar[2] );
return(0);
}



#if DEBUG
void prvec( vec )
double vec[];
{
int k;

for( k=0; k<3; k++ )
	printf( "%17.9E ", vec[k] );
printf( "\n" );
}
#endif



#define DOUBLE double
void divdif();
DOUBLE difpol();

int jpl( JD, iobj, p, v )
double JD;
int iobj;
double p[], v[];
{
int i, k;
long recno;
double JD0, x, t;
double rec[73];
double pcofs[NINTERP+2], vcofs[NINTERP+2];
double pdiffs[NINTERP+2], vdiffs[NINTERP+2];

/*iobj -= 1;*/
/* First record number later than requested date.  */
x = ((JD - JDb) / JDi) + 1.0;
recno = (long) x;
JD0 = JDb + (double )recno * JDi;
bcnt = (recno - NINTERP) * recsiz + rec0;

#if DEBUG
printf( "recno = %ld, bcnt = %ld\n", recno, bcnt );
#endif

t = (JD - JD0)/JDi; /* time scaled to fraction of subrecord */

for( k=0; k<3; k++ )
	{
	  lseek( defd, bcnt, SEEK_SET );
	  for( i=0; i<=NINTERP; i++ )
	    {
	      if( read( defd, rec, (int) recsiz ) != (int) recsiz )
		return(-1);
	      vcofs[i] = rec[6*iobj + 2*k + 1];
	      pcofs[i] = rec[6*iobj + 2*k + 2];
	    }
	  divdif (vcofs, NINTERP, vdiffs);
	  divdif (pcofs, NINTERP, pdiffs);
	  p[k] = difpol (pdiffs, NINTERP, t);
	  v[k] = difpol (vdiffs, NINTERP, t);
	}
/* The last record read should have the date JD0.  */
if( rec[0] != JD0 )
  printf("Record has wrong date %.16e.\n", rec[0]);

#if DEBUG
prvec(p);
prvec(v);
#endif
return(0);
}

/* Interpolation routines taken from de118i.  */
#define DOUBLE double
#define One 1.0

/* Compute zeroth through kth backward differences
 * of the data in the input array
 */
void divdif(vec , k, diffn)
DOUBLE vec[]; /* input array of k+1 data items */
DOUBLE *diffn; /* output array of ith differences */
int k;
{
DOUBLE diftbl[NINTERP+1];
DOUBLE *p, *q;
DOUBLE y;
int i, o;

/* Copy the given data (zeroth difference) into temp array
 */
p = diftbl;
q = vec;
for( i=0; i<=k; i++ )
	*p++ = *q++;

/* On the first outer loop, k-1 first differences are calculated.
 * These overwrite the original data in the temp array.
 */
o = k;
for( o=k; o>0; o-- )
	{
	p = diftbl;
	q = p;
	for( i=0; i<o; i++ )
		{
		y = *p++;
		*q++ = *p - y;
		}
	*diffn++ = *p; /* copy out the last (undifferenced) item */
#if DEBUG
	printf( "%.5e ", *p );
#endif
	}
#if DEBUG
	printf( "%.5e\n", *(q-1) );
#endif
*diffn++ = *(q-1);
}


/* Update array of differences, given new data value.
 * diffn is an array of k+1 differences, starting with the
 * zeroth difference (the previous original data value).
 */
void dupdate( diffn, k, f )
register DOUBLE *diffn;  /* input and output array of differences */
int k; /* max order of differences */
DOUBLE f; /* new data point (zeroth difference) */
{
DOUBLE new, old;
int i;

new = f;
for( i=0; i<k; i++ )
	{
	old = *diffn;
	*diffn++ = new;
#if DEBUG
	printf( "%.5e ", new );
#endif
	new = new - old;
	}
#if DEBUG
	printf( "%.5e\n", new );
#endif
*diffn++ = new;
}




/* Evaluate the interpolating polynomial
 *
 *              (x - x )
 *                    n    1
 * P(x) = f  +  --------  D f   +  ...
 *         n       h         n
 *
 *     (x - x )(x - x   )...(x - x     )
 *           n       n-1          n+2-k    k-1
 *  +  ---------------------------------  D    f
 *                   k-1                        n
 *                  h     (k-1)!
 *
 *
 *         j
 *  where D denotes the jth backward difference, see dupdate(), and
 *
 *  f   =   f( x , y(x ) )  is the interpolated derivative y'(x ) .
 *   n          n     n                                        n
 *
 * The subroutine argument t is linearly scaled so that t = 1.0
 * will evaluate the polynomial at x = x_n + h,
 * t = 0.0 corresponds to x = x_n, etc.
 */
DOUBLE difpol( diffn, k, t )
DOUBLE *diffn;
int k; /* differences go up to order k-1 */
DOUBLE t; /* scaled argument */
{
DOUBLE f, fac, s, u;
int i;

f = *diffn++; /* the zeroth difference = nth data point */
u = One;
/*s = x/h - n;*/
/*s = One; */	/* to evaluate the polynomial at x = xn + h */
s = t;
fac = One;
for( i=1; i<k; i++ )
	{
	if( s == 0 )
		break;
	u *= s / fac;
	f += u  *  (*diffn++);
	fac += One;
	s += One;
	}
return( f );
}

#endif  /* SSYSTEM */
