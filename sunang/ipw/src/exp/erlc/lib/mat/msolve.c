
/*

      msolve(double A[][], double x[], double b[], long n);

  Solve a linear system Ax = b.  It solves by gaussian elimination
and then back-substitution.

      gelim(double A[][], double b[]. long n);

  Gaussian elimination, with partial pivoting.

      backsub(double A[][], double x[], double b[], long n);

  Back-substitution.

      Routines written by Dana Jacobsen, EPA ERL-C, January 1992

*/

#include "ansidecl.h"
#include "mat.h"

extern double EXFUN(fabs, (double x));

void
DEFUN(msolve, (A, x, b, n),
      double **A
  AND double  *x
  AND double  *b
  AND long     n)
{
  gelim(A, b, n);
  backsub(A, x, b, n);
}


void
DEFUN(gelim, (A, b, n),
      double **A
  AND double  *b
  AND long     n)
{
  register double   m;
  register int	    i, j, k, max;

  for (j = 0;  j < (n - 1);  j++) {
    /* partial pivot */
    max = j;
    for (k = j+1; k < n; k++) 
      if (fabs(A[k][j]) > fabs(A[max][j])) max = k;
    for (k = j; k < n; k++) {
               m = A[j][k];
      A [j]  [k] = A [max][k];
      A [max][k] = m;
    }
    m = b[j]; b[j] = b[max]; b[max] = m;
    /* end pivot */
    for (i = j + 1;  i < n;  i++) {
      m = A[i][j] / A[j][j];
      A[i][j] = 0;
      for (k = j + 1;  k < n;  k++) {
        A[i][k] -= m * A[j][k];
      }
      b[i] -= m * b[j];
    }
  }
}

void
DEFUN(backsub, (A, x, b, n),
      double **A
  AND double  *x
  AND double  *b
  AND long     n)
{
  register double   t;
  register int	    j, k;

  for (j = (n - 1); j >= 0; j--) {
    t = 0.0;
    for (k = j + 1; k < n; k++)
      t += A[j][k] * x[k];
    x[j] = ( b[j] - t) / A[j][j];
  }
}
