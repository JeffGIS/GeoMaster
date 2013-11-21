/* Read "Type 66" data records from JPL tape.
   This program extracts sampled time series data from the ephemeris. */


#include "kep.h"

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

/* Define or not in the makefile.  Use kepi.c when SSYSTEM defined.  */
#if ! SSYSTEM

/* Results wanted in TDT, convert to TDB while reading ephemeris.  */
#define TDB_TIME 0

/* Include constants for appropriate ephemeris */
#if DE400
#include "de400.h"
#endif

/* Don't include both lib403.h and de403.h  */
#if LIB403
#include "lib403.h"
#else
#if DE403
#include "de403.h"
#endif
#endif

#if DE404
#include "de404.h"
#endif

#if DE405
#include "de405.h"
#endif

#if DE406CD
#include "de406.h"
#endif

#if DE406
#include "de406.h"
static int ifile;
static int lfile = -1;
static char *pc;
#endif


#if DE245
#include "de245.h"
#endif

#if DE200
#include "de200.h"
#endif

#if DE200CD
#include "de200cd.h"
#endif

#if DE102
#include "de102.h"
/* If it is the DE102, define 1 to apply Set III corrections.  */
#define SETTHREE 1
#else
#define SETTHREE 0
#endif


/* System header files required for I/O.
   Be sure your system is represented here.  */

#ifdef _MSC_VER
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>
#endif

#ifdef __BORLANDC__
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#endif

#if UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

extern double pearthb[];
extern struct orbit earth;
extern double coseps, sineps, J2000;
extern char defile[];

int defd = 0;
static long bcnt = 0;

/* heliocentric velocity of given object (geocentric, if moon) */
double pobjb[3];
double pobjh[3];
double vobjb[3];
double vobjh[3];
double psunb[3];
double vsunb[3];

#if SETTHREE
static struct orbit xorb;
int set3();
#endif
#if ENDIANNESS
#if __STDC__
void swapend(double *);
#else
void swapend();
#endif
#endif

/* Number of coordinates ndim = 2 for nutations, else 3.  */
static int ndim;

int kepjpl(J, e, rect, polar)
double J, rect[], polar[];
struct orbit *e;
{
double JD, ratio, x, y, z;
double pm[3], vm[3];
int nobj, i;

nobj = objnum;
if( nobj == 3 )
	nobj = 10;	/* moon */
if( nobj == 0 )
	nobj = 3;	/* sun */

#if TDB_TIME
JD = tdb(J);
#else
JD = J;
#endif

/* Nutations or librations.  */
#if LIB403
ndim = 2;
nobj = 0;
#else
ndim = 3;
if( nobj == 12 || nobj == 13)
#endif
  {
    if (nobj == 12)
      ndim = 2;
    if( jpl( JD, nobj, pobjb, vobjb ) )
	{
	printf( "Error, DE file boundary violation\n" );
	exit(0);
	}
    for( i=0; i<ndim; i++)
      {
	rect[i] = pobjb[i];
	polar[i] = vobjb[i];
      }
    return 0;
  }

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
#if DE102
#if SETTHREE
		set3( pm, vm, JD, 10, &xorb );
#else
		rotvec( pm, JD );
		rotvec( vm, JD );
#endif
#endif
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
#if DE102
#if SETTHREE
		set3( pobjh, vobjh, JD, 3, &xorb );
#else
		rotvec( pobjh, JD );
		rotvec( vobjh, JD );
#endif
#endif

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
#if DE102
#if SETTHREE
	set3( pobjh, vobjh, JD, nobj, &xorb );
#else
	rotvec( pobjh, JD );
	rotvec( vobjh, JD );
#endif
#endif
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
if( polar[2] != 0.0 )
  polar[1] = asin( z/polar[2] );
else
  polar[1] = 0.0;
return(0);
}



#if DEBUG
void prvec( vec )
double vec[];
{
int k;

for( k=0; k<ndim; k++ )
	printf( "%24.16E ", vec[k] );
printf( "\n" );
}
#endif


int jpl( JD, iobj, p, v )
double JD;
int iobj;
double p[], v[];
{
int i, subis, nc, k, nchar;
long recno;
double JD0, JDs, x, t;
double cofs[18];
long double ps, vs;
long double pcofs[18];
long double vcofs[18];
#if DEBUG
double a[3];
long double acofs[18];
long double as;
#endif

/* Open up the ephemeris file.  */
#if DE406CD
/* The whole ephemeris is in one file.  */
#endif
#if DE406
      if (JD < de406dates[0])
	{
	  printf ("? date too early\n");
	  return -1;
	}
      for (ifile = 0; ifile < 20; ifile++)
        {
	  if (JD < de406dates[ifile])
	    break;
	}
      if (ifile >= 20)
	{
	  printf ("? date too large\n");
	  return -1;
	}
      ifile -= 1;
      if (ifile != lfile)
	{
	  if (defd > 0)
	    close (defd);
	  defd = 0;
	  pc = defile;
	  /* Find end of current file name string.  */
	  while (*pc != '\0')
	    ++pc;
	  /* Find last separator slash or colon in file name path.  */
	  while ((*pc != '/') && (*pc != ':') && (*pc != '\\'))
	    --pc;
	  ++pc;
	  /* Append file name to directory.  */
	  strcpy (pc, de406names[ifile]);
	  lfile = ifile;
	}
#endif  /* DE406 */
if( defd <= 0 )
	{
floop:
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

#if DE245 | DE400 | DE403 | DE404 | DE405 | DE406 | DE406CD | LIB403
	bcnt = rec0;
#else
#if DE200CD
/* This is for DE200 on CD rom. */
	bcnt = rec0;
#else
/* This is for DE200, DE102 magtapes. */
	bcnt = rec0 + 4L;
#endif
#endif
	lseek( defd, bcnt, SEEK_SET );
	read( defd, &JDb, 8 );
#if DECDATA
	cvtdec((unsigned short *)&JDb);
#endif
#if ENDIANNESS
	swapend( &JDb );
#endif

#if 1
	printf( "First date in file = %.8E\n", JDb );
#endif
	}

if (JD < JDb)
  return (-1);

/* Nutations have only two coordinates.  */
if( iobj == 12 )
  ndim = 2;
else
  ndim = 3;

iobj -= 1;
x = (JD - JDb) / JDi;	/* record number */
recno = (long) x;
JD0 = JDb + (double )recno * JDi;
bcnt = recno * recsiz + rec0;
i = sizeof( double ) * ( objs[iobj][0] - 1 );
#if DE245 | DE400 | DE403 | DE404 | DE405 | DE406 | DE406CD | LIB403
bcnt = bcnt + (long )i;
#else
#if DE200CD
bcnt = bcnt + (long )i;
#else
/* This is for DE200, DE102 on magtape. */
bcnt = bcnt + 4 + (long )i;
#endif
#endif
subis = objs[iobj][2];
nc = objs[iobj][1];
JDs = JDi/( (double )subis ); /* days per subrecord */

#if DEBUG
printf( "recno %ld\n", recno );
printf( "nc %d  subis %d\n", nc, subis );
#endif

if( subis > 1 )
	{
	recno = (long) ((JD - JD0) / JDs);
	JD0 += (double )recno * JDs; /* start time of subrecord */
	recno *= ndim * sizeof( double ) * nc;
	bcnt += recno;
	}

#if DEBUG
printf( "bcnt = %ld\n", bcnt );
#endif
lseek( defd, bcnt, SEEK_SET );
t = (JD - JD0)/JDs; /* time scaled to fraction of subrecord */

/* Chebyshev polynomials
 */
t = 2.0 * t - 1.0;
#if DEBUG
printf( "t %17.9E\n", t );
#endif
pcofs[0] = 1.0;
pcofs[1] = t;
vcofs[0] = 0.0;
vcofs[1] = 1.0;
#if DEBUG
acofs[0] = 0.0;
acofs[1] = 0.0;
#endif
t *= 2.0;
for( i=2; i<nc; i++ )
	{
	pcofs[i] = t * pcofs[i-1]  -  pcofs[i-2];
	vcofs[i] = 2.0 * pcofs[i-1] + t * vcofs[i-1] - vcofs[i-2];
#if DEBUG
	acofs[i] = 4.0 * vcofs[i-1] + t * acofs[i-1] - acofs[i-2];
#endif
	}

for( k=0; k<ndim; k++ )
	{
	nchar = sizeof( double ) * nc;
	if( read( defd, &cofs[0], nchar ) != nchar )
		return(-1);
#if DECDATA
	for( i=0; i<nc; i++ )
		cvtdec( (unsigned short *) &cofs[i] );
#endif
#if ENDIANNESS
	for( i=0; i<nc; i++ )
		swapend( &cofs[i] );
#endif
	ps = 0.0;
	vs = 0.0;
#if DEBUG
	as = 0.0;
#endif
	for( i=nc-1; i>=0; i-- )
		{
		ps += pcofs[i] * cofs[i];
		vs += vcofs[i] * cofs[i];
#if DEBUG
		as += acofs[i] * cofs[i];
#endif
		}
	t = 0.5 * JDs;
	/* Don't scale nutations or librations.  */
#if LIB403
	if (1)
#else
	if (iobj == 11 || iobj == 12)
#endif
	  {
	    p[k] = ps;
	    v[k] = vs / t;
#if DEBUG
	    a[k] = as / (t * t);
#endif
	  }
	else
	  {
	    p[k] = ps/au;
	    v[k] = vs / (t * au);
#if DEBUG
	    a[k] = as / (t * t * au);
#endif
	  }
#if DEBUG
printf (
  "k= %d: pcofs                vcofs              acofs            cofs\n",
  k);
for(i=0; i<nc; i++)
  {
    printf("%19.12Le%19.12Le%19.12Le%19.12e\n",
	   pcofs[i], vcofs[i], acofs[i], cofs[i]);
  }
#endif
      }
#if DEBUG
prvec(p);
prvec(v);
prvec(a);
#endif
return(0);
}

#if DE102
#if SETTHREE

#else
/* Convert the equinox of the DE102 ephemeris to J2000.
 * Note there are other, orbital, corrections required to make
 * the DE102 agree more accurately with DE200.
 */
rotvec( vec, J )
double vec[];
double J;
{
double y[3];
double yy, zz;
int k;

y[0] =	 0.999925712980 * vec[0]
	-0.011178735677 * vec[1]
	-0.004858435955 * vec[2];
y[1] =	 0.012188864294 * vec[0]
	+0.917414007867 * vec[1]
	+0.397747369277 * vec[2];
y[2] =	 0.000010884494 * vec[0]
	-0.397777040626 * vec[1]
	+0.917482111996 * vec[2];


epsiln(J2000);
yy = y[1];
zz = y[2];
y[1]  =  coseps * yy  +  -sineps * zz;
y[2]  =  sineps * yy  +  coseps * zz;


for( k=0; k<3; k++ )
	vec[k] = y[k];
}
#endif
#endif

/* Convert DEC VAX/PDP-11 double precision number format
 * to IBM PC IEEE double number format.
 */
#if DECDATA
#include "cvtdec.c"
#endif

#if ENDIANNESS
#include <string.h>

void swapend(x)
double *x;
{
union {
  double d;
  char ch[8];
}u;
char c;

#if 0
u.d = *x;
#else
memcpy ((void *) u.ch, (void *) x, 8);
#endif
c = u.ch[0];
u.ch[0] = u.ch[7];
u.ch[7] = c;
c = u.ch[1];
u.ch[1] = u.ch[6];
u.ch[6] = c;
c = u.ch[2];
u.ch[2] = u.ch[5];
u.ch[5] = c;
c = u.ch[3];
u.ch[3] = u.ch[4];
u.ch[4] = c;
*x = u.d;
}
#endif

#endif  /* not SSYSTEM  */
