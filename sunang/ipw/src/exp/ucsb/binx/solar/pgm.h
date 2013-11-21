extern int    EXFUN( akcoef, (double *x, double *y, int nx, double *c));
extern void   EXFUN( main, (int argc, char **argv));
extern int    EXFUN( modprod, (int a, int b, int i));
extern void   EXFUN( solar, (int n, double *range, int *date, bool_t abbrev));
extern double EXFUN( solint, (double a, double b));
extern double EXFUN( solwnt, (int a, int b));
extern double EXFUN( solval, (double lambda));
extern double EXFUN( solwn, (int eta));
extern double EXFUN( splint, (double *x, double *y, int nx, double *c,
                              double a, double b));
extern double EXFUN( spval1, (double u, double x[], double y[], int n,
                              double c[]));
extern int    EXFUN( updbl, (double *x, double *y));
extern int    EXFUN( downdbl, (double *x, double *y));
extern int    EXFUN( vdcps, (int n, int *ipf, int *iexp, int *ipwr));
extern void   EXFUN( vtdbl, (double *zot, int m, int n));
extern void   EXFUN( sqdbl, (double *zot, int n));
