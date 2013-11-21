/* Constants for the JPL DE200 Chebyshev Ephemeris
 */

#define DE102F 0
#define DE200F 0
#define DE245F 0
#define DE400F 1

double JDb = 2444208.5; /* starting JED 1979 Dec 1, for de400s.unx */
double JDe = 2455216.5; /* ending JED 2010 Jan 20 */
double JDi = 32.0; /* days per record */
#define DRSIZ 8144
long recsiz = (long )DRSIZ; /* bytes per record */
long rec0 = 16288L; /* offset to first record */

int objs[13][3] = {
  3, 14, 4,
171, 10, 2,
231, 13, 2,
309, 11, 1,
342, 8, 1,
366, 7, 1,
387, 6, 1,
405, 6, 1,
423, 6, 1,
441, 13, 8,
753, 11, 2,
819, 10, 4,
899, 10, 4
};

double au = 1.49597870693e8; /* Kilometers per au */
double emrat = 81.300588; /* Earth/Moon mass ratio */
double radearth = 6378.137;
double clight = 2.99792458e5;
char *dename = "de400.unx";
