/* x200.c
   Read data records from JPL ephemeris file.

   Configure macros ALLOBJS, HEADER, HELIOCENTRIC, POLAR, OFDATE,
   SPECOUT, VELOCITY to taste.  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif

/* All objects, or not.  */
/* If > 0, indicates number of objects wanted.  */
#define ALLOBJS 0

/* Put header at start of file.  */
#define HEADER 1
/* Use this for barycentric sun.
#define HELIOCENTRIC 0
#define POLAR 0
*/

#define HELIOCENTRIC 1
#define POLAR 1


/* Write out the velocity following the position.  */
#define VELOCITY 0

/* Create 3 files containing longitude, latitude, radius. */
#define SPECOUT 0
#if SPECOUT
#undef ALLOBJS
#undef HEADER
#undef HELIOCENTRIC
#undef POLAR
#define ALLOBJS 0
#define HEADER 1
#define HELIOCENTRIC 1
#define POLAR 1
#endif

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

/* Conversion factors between degrees and radians */
double DTR = 1.7453292519943295769e-2;
double RTD = 5.7295779513082320877e1;
double RTS = 2.0626480624709635516e5; /* arc seconds per radian */
double STR = 4.8481368110953599359e-6; /* radians per arc second */
double PI = 3.14159265358979323846;

/* Standard epochs.  Note Julian epochs (J) are measured in
 * years of 365.25 days.
 */
double J2000 = 2451545.0;	/* 2000 January 1.5 */
double B1950 = 2433282.42345905; /* 1950 January 0.923 Besselian epoch */
double J1900 = 2415020.0;	/* 1900 January 0, 12h UT */

/* Include constants for appropriate ephemeris
   by defining one of DE102, DE200, DE403, DE404 in the compilation
   command line. */

#include "kep.h"

/* Default file name for the JPL ephemeris.
   Replace these by your own names.  */
#if LIB403
char *defile0 = "/d/astro/lib403.unx";
#else /* not LIB403 */
#if DE403
char *defile0 = "/dos2/astro/de403.unx";
#endif
#endif /* not LIB403 */
#if DE404
char *defile0 = "/d/astro/de/de404.unx";
#endif
#if DE405
char *defile0 = "/d/astro/de/de405.unx";
#endif
#if DE406CD
char *defile0 = "/cdrom/unix.406";
#else
#if DE406
char *defile0 = "/d/astro/de/de406.unx";
#endif
#endif
#if DE200 | DE200CD
char *defile0 = "/dos2/astro/de200.unx";
#endif
#if DE102
char *defile0 = "/dos2/astro/de102.unx";
#endif
extern int defd; /* ephemeris file descriptor; see kepjpl.c */

#ifdef OFDATE
#undef OFDATE
#endif
/* Precess to mean ecliptic and equinox of date.  */
#define OFDATE 0

/* Items referenced by kepjpl.c.  */
struct orbit earth;
struct orbit orb;
/* Ephemeris file name.  */
char defile[128] = {0};
double jvearth;
double pearthb[3];
double vearth[3];
int objnum ;
/* Referenced by lonlat.c.  */
int prtflg = 0;
/* Referenced by dms.c.  */
double TDT, UT;
int ephprint = 0;
FILE *ephfile = NULL;

#if SPECOUT
int fds, fdl, fdd;
static double ref[3];
#endif
int fd = 0;
int fo = 0;
int nsamps;
double dJD;

double ps[3];
double vs[3];
double pobj[3];
double vobj[3];

static double p[3];

double floor();
#define mods3600(x) ((x) - 1296000.0 * floor((x)/1296000.0))
char str[80];

int
main()
{
double JD, isamps;
int i, k;
#if ALLOBJS
int iiobj;
#endif

#if LIB403
printf( "Extract data from JPL LIB403 ephemeris of librations.\n");
#else /* not LIB403 */
#if DE403
printf( "Extract data from JPL DE403 ephemeris.\n");
#endif
#endif /* not LIB403 */
#if DE404
printf( "Extract data from JPL DE404 ephemeris.\n");
#endif
#if DE405
printf( "Extract data from JPL DE405 ephemeris.\n");
#endif
#if DE406
printf( "Extract data from JPL DE406 ephemeris.\n");
#endif
#if DE200 | DE200CD
printf( "Extract data from JPL DE200 ephemeris.\n");
#endif
#if DE102
printf( "Extract data from JPL DE102 ephemeris.\n");
#endif

/* Fill in the default input file name.  */
strcpy (defile, defile0);

#if SPECOUT
fds = open( "lspec.dat", O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
	   S_IREAD | S_IWRITE );
fdl = open( "lspecl.dat", O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
	   S_IREAD | S_IWRITE );
fdd = open( "lspecr.dat", O_BINARY | O_RDWR | O_CREAT | O_TRUNC,
	   S_IREAD | S_IWRITE );
#else /* not SPECOUT */
printf( "Output file ? " );
gets( str );
fo = open( str, O_BINARY | O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE );
#endif /* not SPECOUT */

printf( "Starting JED ? " );
gets (str);
sscanf( str, "%lf", &JD );
#if HEADER
if (fo)
  write( fo, &JD, sizeof(double) );
#endif
printf( "%.16e\n", JD);

printf( "Number of samples ? " );
gets(str);
sscanf( str, "%d", &nsamps );
isamps = nsamps;
#if HEADER
if (fo)
  write( fo, &isamps, sizeof(double) );
#endif
printf( "%.16e\n", isamps);

printf( "Days between samples ? " );
gets(str);
sscanf( str, "%lf", &dJD );
#if HEADER
if (fo)
  write( fo, &dJD, sizeof(double) );
#endif
printf( "%.16e\n", dJD);

#if SPECOUT
ref[0] = JD; /* starting date */
ref[1] = isamps;
ref[2] = dJD;
write( fds, ref, 24 );
write( fdl, ref, 24 );
write( fdd, ref, 24 );
#endif /* SPECOUT */

#if !ALLOBJS
printf( "Object number ? " );
gets(str);
sscanf( str, "%d", &objnum );
printf("%d\n", objnum );
#endif

isamps = 0.0;

nxtinput:
printf("Input file name ?");
gets (str);
if (str[0] != '\0')
  strcpy (defile, str);
printf("\n%s\n", defile);

loop:

#if ALLOBJS
/* Date at start of each record.  */
write( fo, &JD, sizeof(double) );
for(iiobj=0; iiobj<ALLOBJS; iiobj++)
  {
#if DE403
    if( iiobj == 0 )
      objnum = 13; /* librations */
    else
      objnum = iiobj;
#else
    objnum = iiobj + 1;
#endif
#endif /* ALLOBJS */

/* Barycentric coordinates of object.  */
for (i=0; i<3; i++)
  {
    vobj[i] = 0.0;
    pobj[i] = 0.0;
  }
k = jpl( JD, objnum, pobj, vobj );
if (k < 0)
  {
    printf("jpl(%d) returns %d\n", objnum, k);
    goto done;
  }
#if HELIOCENTRIC
if (objnum != 10)
  {
    /* Heliocentric coordinates.  */
    k = jpl( JD, 11, ps, vs ); /* Sun's barycentric coordinates.  */
    if (k < 0)
      {
	printf("jpl(11) returns %d\n", k);
	goto done;
      }
    for (i = 0; i < 3; i++)
      {
	vobj[i] -= vs[i];
	pobj[i] -= ps[i];
      }
  }
#endif /* HELIOCENTRIC */

#if POLAR
/* Set last arg = 1 to precess to date */
#if OFDATE
lonlat (pobj, JD, p, 1);
#else
lonlat (pobj, JD, p, 0);
#endif

/* Write out polar coordinates.  */
#if SPECOUT
write (fds, &p[0], sizeof (double)); /* longitude */
write (fdl, &p[1], sizeof (double)); /* latitude */
write (fdd, &p[2], sizeof (double)); /* radius */
#else /* not SPECOUT */
write (fo, &p[0], 3 * sizeof (double));
#endif /* not SPECOUT */

#else /* not POLAR */
/* Write out equatorial x, y, z position and maybe velocity.  */
write( fo, &pobj[0], 24 );
#if VELOCITY
write( fo, &vobj[0], 24 );
#endif
#endif /* not POLAR  */

#if ALLOBJS
  }
#endif

isamps += 1.0;
if( --nsamps > 0.0 )
  goto not_done_yet;

goto really_done;

/* End of file reached.  */
done:
close (defd);
defd = 0;
printf("looking for JD = %.16e\n", JD);
printf ("Another input file?");
gets (str);
printf("\n%s\n", str);
if (str[0] == 'y')
  goto nxtinput;

really_done:
#if SPECOUT
lseek (fds, (long) sizeof(double), SEEK_SET);
write (fds, &isamps, sizeof (double));
lseek (fdl, (long) sizeof(double), SEEK_SET);
write (fdl, &isamps, sizeof (double));
lseek (fdd, (long) sizeof(double), SEEK_SET);
write (fdd, &isamps, sizeof (double));
close( fds );
close( fdl );
close( fdd );
#else /* not SPECOUT */
#if HEADER
lseek (fo, (long) sizeof(double), SEEK_SET);
write (fo, &isamps, sizeof (double));
#endif
close( fo );
#endif /* not SPECOUT */
printf ("Actual number of samples = %.0f\n", isamps);
exit(0);

not_done_yet:
JD += dJD;
goto loop;
}
