/* From P.J. Plauger's "The Standard C Library" */
/* With some modifications by Dana Jacobsen */

/* test math functions -- part 1 */
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

#if 1
#include "../ceil.c"
#include "../floor.c"
#endif

static double eps;

static int approx(d1, d2)
double d1;
double d2;
{
  if (d2 != 0)
    return (fabs((d2 - d1) / d2) < eps);
  else
    return (fabs(d1) < eps);
}

int
main()
{
  double huge_val, x;
  int xexp;

  eps = DBL_EPSILON * 4.0;

  assert(ceil(-5.0-eps) == -5.0);
  assert(ceil(-5.0+eps) == -4.0);
  assert(ceil(-5.1) == -5.0);
  assert(ceil(-5.0) == -5.0);
  assert(ceil(-4.9) == -4.0);
  assert(ceil(-eps) == 0.0);
  assert(ceil(0.0) == 0.0);
  assert(ceil(eps) == 1.0);
  assert(ceil(4.9) == 5.0);
  assert(ceil(5.0) == 5.0);
  assert(ceil(5.1) == 6.0);
  assert(ceil(5.0-eps) == 5.0);
  assert(ceil(5.0+eps) == 6.0);

  assert(fabs(-5.0) == 5.0);
  assert(fabs(0.0) == 0.0);
  assert(fabs(5.0) == 5.0);

  assert(floor(-5.0-eps) == -6.0);
  assert(floor(-5.0+eps) == -5.0);
  assert(floor(-5.1) == -6.0);
  assert(floor(-5.0) == -5.0);
  assert(floor(-4.9) == -5.0);
  assert(floor(-eps) == -1.0);
  assert(floor(0.0) == 0.0);
  assert(floor(eps) == 0.0);
  assert(floor(4.9) == 4.0);
  assert(floor(5.1) == 5.0);
  assert(floor(5.0-eps) == 4.0);
  assert(floor(5.0+eps) == 5.0);

  /* other stuff */

  puts("SUCCESS testing <math.h>, part 1");
  return(0);
}
