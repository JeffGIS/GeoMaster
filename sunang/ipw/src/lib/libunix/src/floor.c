
/* LINTLIBRARY */

/*
** NAME
**	floor -- computes the largest integer not greater than _x_
**
** SYNOPSIS
**	double floor(double x)
**
** DESCRIPTION
**	Truncates the argument to an integer.
**
** RESTRICTIONS
**
** RETURN VALUE
**
** APPLICATION USAGE
**	floor is a version of the POSIX standard function floor.
**
**	You should only use this version of floor if your system doesn't
**	provide one in its standard C or math libraries.  The version
**	supplied there will no doubt be much more efficient as it can
**	utilize properties of the internal FP format.
**
** BUGS
*/


double
floor(x)
double x;
{
  long ip;

  ip = (long) x;

  if (  ( x >= 0.0 )  ||  ( (double) ip == x )  ) {
    return ip;
  } else {
    return ip - 1;
  }

}
