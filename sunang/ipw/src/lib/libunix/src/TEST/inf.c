#include "ipw.h"
#undef assert
#include <assert.h>
#include <stdio.h>
 
#if 1
#include "../isinf.c"
#endif
 
/*
 * We check to see that isinf returns 0 for normal numbers, and !0 for
 * infinite numbers.  If we use our isinf function, we should return
 * a -1 for negative infinity, but all that is required is to return
 * a non-0 value.
 */

int main()
{
 double v1;
 
 /* Normal numbers */
 v1 = 1;
 assert(isinf(v1) == 0);
 v1 = -1;
 assert(isinf(v1) == 0);

 /* The largest double */
 v1 = DBL_MAX;
 assert(isinf(v1) == 0);

 /* 2 times the largest double.  This should return 1 */
 v1 *= 2.0;
 assert(isinf(v1) != 0);

 /* The negative of the largest double */
 v1 = -DBL_MAX;
 assert(isinf(v1) == 0);

 /* 2 times the negative of the largest double.  This should return -1 */
 v1 *= 2.0;
 assert(isinf(v1) != 0);
 
 puts("SUCCESS testing isinf()");
 return(0);
}
