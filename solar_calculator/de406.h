/* Constants for the JPL DE403 Chebyshev Ephemeris.  */

#define DE102F 0
#define DE200F 0
#define DE245F 0
#define DE400F 0
#define DE403F 0
#define DE404F 1

double JDb = 625360.5; /* starting JED -3000 Feb 23, for de406.unx */
double JDe = 2816848.5; /* ending JED 3000 March 3 */
/* Willmann-Bell CD ROM 
  starts  3001 B.C. Feb 24 = 625360.5
  ends at 3000 July 8 = 2816976.5      */
double JDi = 64.0; /* days per record */

/* 728 values per record times size of floating point format (8 bytes). */
#define DRSIZ 5824
long recsiz = (long )DRSIZ; /* bytes per record */

long rec0 = 11648L; /* offset to first record */
/* long rec0 = 0L; */  /* Use 0 for excerpted segments of the file.  */

int objs[13][3] = {
{  3, 14, 4},
{171, 12, 1},
{207, 9, 2},
{261, 10, 1},
{291, 6, 1},
{309, 6, 1},
{327, 6, 1},
{345, 6, 1},
{363, 6, 1},
{381, 13, 8},
{693, 12, 1},
{729, 0, 0},  /* No nutations */
{729, 0, 0},  /* No librations */
};

double au = 1.49597870691e8; /* Kilometers per au */
double emrat = 81.30056; /* Earth/Moon mass ratio */
double radearth = 6378.137;
double clight = 2.99792458e5;
char *dename = "de406.unx";
/*  Starting dates of posted de406 subfiles */
double de406dates[] = {
  625360.5,
  734864.5,
  844432.5,
  954000.5,
  1063568.5,
  1173136.5,
  1282704.5,
  1392272.5,
  1501904.5,
  1611472.5,
  1721040.5,
  1830608.5,
  1940176.5,
  2049744.5,
  2159312.5,
  2268880.5,
  2378448.5,
  2488016.5,
  2597584.5,
  2707152.5,
  };
/* Corresponding file names of posted subfiles */
char *de406names[] = {
"unxm3000.406",
"unxm2700.406",
"unxm2400.406",
"unxm2100.406",
"unxm1800.406",
"unxm1500.406",
"unxm1200.406",
"unxm0900.406",
"unxm0600.406",
"unxm0300.406",
"unxp0000.406",
"unxp0300.406",
"unxp0600.406",
"unxp0900.406",
"unxp1200.406",
"unxp1500.406",
"unxp1800.406",
"unxp2100.406",
"unxp2400.406",
"unxp2700.406",
};

