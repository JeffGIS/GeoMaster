/* Constants for the JPL DE404 Chebyshev Ephemeris.  */

#define DE102F 0
#define DE200F 0
#define DE245F 0
#define DE400F 0
#define DE403F 0
#define DE404F 1

double JDb = 625296.5; /* starting JED -3001 Dec 21, for de404.unx */
double JDe = 2817104.5; /* ending JED 3000 Nov 14 */
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
double emrat = 81.300585; /* Earth/Moon mass ratio */
double radearth = 6378.137;
double clight = 2.99792458e5;
char *dename = "de404.unx";
