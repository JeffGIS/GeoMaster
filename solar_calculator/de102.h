/* Constants for the JPL DE102
 * Chebyshev ephemeris
 */

#define DE102F 1
#define DE200F 0

double JDb = 1206160.5;
double JDe = 1996112.5;
double JDi = 64.0;
long recsiz = 6188L;

/* first data record when introductory records are present:
 * long rec0 = 18084L;
 */

long rec0 = 0L;

int objs[12][3] = {
{  3, 15, 2,},
{ 93, 15, 1,},
{138, 15, 2,},
{228, 10, 1,},
{258, 9, 1,},
{285, 8, 1,},
{309, 8, 1,},
{333, 6, 1,},
{351, 6, 1,},
{369, 15, 8,},
{729, 15, 1,},
{774, 0, 0},
};

double au = 1.495978706835178e8; /* Kilometers per au */
double emrat = 81.3007; /* Earth/Moon mass ratio */
double radearth = 6378.14;
double clight = 2.99792458e5;
char *dename = "DM3:DE102.DAT";
