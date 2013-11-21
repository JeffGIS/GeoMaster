
/* least squares polynomial fit */

/*
  apfit(double arx[], double ary[], long n, long d,
        double *x, const int savemem);

    Given observed values in arx and ary, with n elements in each,
return d elements in the array x, representing the value given to
the fitting function (in this case a polynomial fit, so:

  fity(n) = x[0] + x[1]*n + x[2]*n^2 + x[3]*n^3 + ... + x[d]*n^d

If savemem is true, the routine will take up a significantly
smaller chunk of memory, but also run significantly slower.
*/

/* Written by Dana Jacobsen, EPA ERL-C, 1992 */

#include "mvalloc.h"
#include "ansidecl.h"
#include "mat.h"

#if defined(__GNUC__) && defined(__STDC__) && !defined(__STRICT_ANSI__)
#define INLINE inline
#else
#define INLINE
#endif

static INLINE double EXFUN(fitf, (double a, int x));

void
DEFUN(apfit, (arx, ary, n, d, x, savemem),
           double    *arx
       AND double    *ary
       AND long       n
       AND long       d
       AND double    *x
       AND CONST int savemem)
{
  register int        k, j, i;
  register double     tv;
           double  ** A;
           double  ** m;
           double  ** tm;
           double   * b;

  d++;

  A  = (double **) MValloc(MV_DOUBLE, 2, d, d);     /* make arrays      */
  b  = (double *) MValloc(MV_DOUBLE, 1, d);
  if (!savemem) {
    m  = (double **) MValloc(MV_DOUBLE, 2, n, d);
    tm = (double **) MValloc(MV_DOUBLE, 2, d, n);
  } else {
    m  = (double **) NULL;    /* To make lint and gcc stop complaining  */
    tm = (double **) NULL;    /* about variables used but not set.      */
  }

  for (i = 0; i < d; i++) {             /* zero arrays      */
    b[i] = 0;
    for (j = 0; j < d; j++)
      A[i][j] = 0;
  }

  if (!savemem) {
    for (i = 0; i < n; i++)               /* initialize m     */
      for (j = 0; j < d; j++) {
              tv = fitf(arx[i], j);
         m[i][j] = tv;                    /* set m            */
        tm[j][i] = tv;                    /* set transpose m  */
      }
  }

  if (!savemem) {
    for (i=0; i < d; i++)                 /* b = tm * ary     */
        for (k = 0; k < n; k++)
          b[i] += tm[i][k] * ary[k];
  } else {
    for (i=0; i < d; i++)                 /* b = tm * ary     */
        for (k = 0; k < n; k++)
          b[i] += fitf(arx[k], i) * ary[k];
  }


  if (!savemem) {
    for (i=0; i < d; i++)                 /* A = tm * m       */
      for (j=0; j < d; j++)
        for (k = 0; k < n; k++)
          A[i][j] += tm[i][k] * m[k][j];
  } else {
    for (i=0; i < d; i++)                 /* A = tm * m       */
      for (j=0; j < d; j++)
        for (k = 0; k < n; k++)
          A[i][j] += fitf(arx[k], i) * fitf(arx[k], j);
  }


  msolve(A, x, b, d);                   /* solve   A x = b   */

  MVfree((void *) A );
  MVfree((void *) b );
  if (!savemem) {
    MVfree((void *) m );
    MVfree((void *) tm);
  }
}



/*  The fitting function:     a^x  */

static INLINE double
DEFUN(fitf, (a, x), double a AND int x)
{
  double v = 1;

  while (x-- > 0)
    v *= a;
  return(v);
}
