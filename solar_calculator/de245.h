/* Constants for the JPL DE200 Chebyshev Ephemeris
 */

#define DE102F 0
#define DE200F 0
#define DE245F 1

double JDb = 2360400.5; /* starting JED = June 16, 1750 */
double JDe = 2470384.5; /* ending JED = August 1, 2051 */
double JDi = 32.0; /* Days per record. */
#define DRSIZ 8144
long recsiz = (long )DRSIZ; /* Bytes per record. */
long rec0 = 16288L;  /* Offset to first real data record. */
/* long rec0 = 0; */ /* For versions with no preamble.  */

int objs[13][3] = {
{  3, 14, 4,},
{171, 10, 2,},
{231, 13, 2,},
{309, 11, 1,},
{342, 8, 1,},
{366, 7, 1,},
{387, 6, 1,},
{405, 6, 1,},
{423, 6, 1,},
{441, 13, 8,},
{753, 11, 2,},
{819, 10, 4,},
{899, 10, 4},
};

double au = 1.495978707e8; /* Kilometers per au */
double emrat = 81.30059; /* Earth/Moon mass ratio */
double radearth = 6378.137;
double clight = 2.99792458e5;
/* The file name is actually taken from aa.ini.  */
char *dename = "de245.unx";
