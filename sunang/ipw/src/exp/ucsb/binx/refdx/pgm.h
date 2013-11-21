
#ifndef PGM_H
#define PGM_H

#include "complex.h"

extern int         EXFUN( akcoef, (double *x, double *y, int nx, double *c));
extern void        EXFUN( main, (int argc, char **argv));

extern COMPLEX_T * EXFUN( refice, (COMPLEX_T *result, double lambda,
                                   double temp));
extern COMPLEX_T * EXFUN( trefice, (COMPLEX_T *result, double lambda,
                                    double temp));
extern COMPLEX_T * EXFUN( refh2o, (COMPLEX_T *result, double lambda,
                                   double temp));
extern COMPLEX_T * EXFUN( trefh2o, (COMPLEX_T *refwat, double lambda,
                                    double tc));
extern int         EXFUN( modprod, (int a, int b, int i));
extern double EXFUN( spval1, (double u, double x[], double y[], int n,
                              double c[]));
extern int    EXFUN( vdcps, (int n, int *ipf, int *iexp, int *ipwr));
extern void   EXFUN( vtdbl, (double *zot, int m, int n));
extern void   EXFUN( sqdbl, (double *zot, int n));
extern void   EXFUN( wavechoice, (char *units, double (**xform)(double)));
extern double EXFUN( multiply, (double x));
extern double EXFUN( divide, (double x));

#endif
