/* Constants for the JPL DE200 Chebyshev Ephemeris, CD ROM version
 */

#define DE102F 0
#define DE200F 1

double JDb = 2305424.5; /* starting JED = Dec 9, 1599 */
double JDe = 2513392.5; /* ending JED = May 2, 2169 */
double JDi = 32.0; /* days per record */
#define DRSIZ 6608
long recsiz = (long )DRSIZ; /* bytes per record */

long rec0 = 13216L; /* offset to first record */
/*long rec0 = 0L;*/
/* or maybe it is 19824L */

int objs[12][3] = {
{  3, 12, 4,},
{147, 12, 1,},
{183, 15, 2,},
{273, 10, 1,},
{303, 9, 1,},
{330, 8, 1,},
{354, 8, 1,},
{378, 6, 1,},
{396, 6, 1,},
{414, 12, 8,},
{702, 15, 1,},
{747, 10, 4},
};

double au = 1.4959787066e8; /* Kilometers per au */
double emrat = 81.300587; /* Earth/Moon mass ratio */
double radearth = 6378.14;
double clight = 2.99792458e5;
char *dename = "DM1:DE200.DAT";
