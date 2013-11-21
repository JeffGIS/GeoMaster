/* Constants for the JPL DE200 Chebyshev Ephemeris
 */

#define DE102F 0
#define DE200F 1

double JDb = 2378640.5; /* starting JED = May 5, 1800 */
double JDe = 2469808.5; /* ending JED = Jan 2, 2050 */
double JDi = 32.0; /* days per record */
#define DRSIZ 6612
long recsiz = (long )DRSIZ; /* bytes per record */
long rec0 = 18132L; /* offset to first record */
/*long rec0 = 0L;*/

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
