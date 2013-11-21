
/* LINTLIBRARY */

/*
** NAME
**	isinf -- test for infinity
**
** SYNOPSIS
**	int isinf(double x)
**
** DESCRIPTION
**	isinf returns a positive integer if x is +INFINITY, or a negative
**	integer if x is -INFINITY.  Otherwise it returns zero.
**
** RESTRICTIONS
**
** RETURN VALUE
**	x:   +INFINITY :  1
**      x:   -INFINITY : -1
**      x:   other     :  0
**
** APPLICATION USAGE
**	isinf is a version of the standard function isinf.
**
**	You should only use this version of isinf if your system doesn't
**	provide one in its standard C libraries.
**
** BUGS
**	There is no standard way to do this.  Most vendors supply a
**	function called isinf that does it.  IEEE 754 recommends a
**	function called "class" that determines the class of a number.
*/


/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */

#if defined(sun) || defined(__sun) || defined(__sun__)

#include "ipw.h"
#include <ieeefp.h>

int
DEFUN(isinf, (value), double value)
{
  fpclass_t  fpc;

  fpc = fpclass(value);
  if (fpc == FP_PINF)
    return(1);
  if (fpc == FP_NINF)
    return(-1);

  return(0);
}

#else  /* not Solaris 2.x */

#include "ipw.h"

int
DEFUN(isinf, (value), double value)
{
  if (value > DBL_MAX)
    return(1);
  if (value < (-(DBL_MAX)))
    return(-1);

  return 0;
}

#endif
