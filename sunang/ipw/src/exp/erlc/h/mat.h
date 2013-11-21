
#ifndef MAT_H
#define MAT_H

/* function declarations */

extern void    EXFUN(msolve, (double **A, double *x, double *b, long n));
/* extern double  EXFUN(fitf, (double a, int x)); */
extern void    EXFUN(apfit, (double *arx, double *ary, long n, long d,
			double *x, CONST int ts));
extern void    EXFUN(gelim, (double **A, double *b, long n));
extern void    EXFUN(backsub, (double **A, double *x, double *b, long n));
extern void    EXFUN(msolve, (double **A, double *x, double *b, long n));

#endif
