#ifndef	SUNANG_H
#define	SUNANG_H

#include <time.h>

extern int          EXFUN(ephemeris, (struct tm *gmt, double *r, double *declin,
			double *omega));
extern struct tm  * EXFUN(gmt, (struct tm *local, int zone));
extern struct tm  * EXFUN(loct, (struct tm *grnwch, int zone, bool_t isdst));
extern int          EXFUN(julday, (int year, int month, int day));
extern int          EXFUN(rotate, (double mu, double azm, double mu_r, double lam_r,
			double *muPrime, double *azmPrime));
extern int          EXFUN(sunpath, (double lat, double lon, double declin,
			double omega, double *cosZ, double *azm));
extern struct tm  * EXFUN(tmconv, (long clockt, bool_t isdst));
extern struct tm  * EXFUN(unixtime, (int year, int month, int day, int hour,
                            int minute, int second, bool_t isdst));
extern int          EXFUN(weekday, (int year, int month, int day));
extern int          EXFUN(yrday, (int year, int month, int day));
extern char       * EXFUN(zonetime, (int zone, int dst));

#endif
