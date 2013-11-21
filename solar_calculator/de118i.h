
/* Constants for de118i ephemerides, assuming default DE200 values used.  */

/* Interpolator degree */
#define NINTERP 12

long rec0 = 0L;
long recsiz = 584L; /* bytes per record */
double JDi, JDb;
double au = 1.4959787066e8; /* Kilometers per au */
double emrat = 81.300587; /* Earth/Moon mass ratio */
double radearth = 6378.14;
double clight = 2.99792458e5;
char *dename = "system.out";
