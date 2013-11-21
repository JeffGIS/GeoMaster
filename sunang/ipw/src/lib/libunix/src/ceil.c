
/* LINTLIBRARY */

/*
** NAME
**	ceil -- computes the largest integer not less than _x_
**
** SYNOPSIS
**	double ceil(double x)
**
** DESCRIPTION
**	Rounds the integral value up to the next integer value.
**
** RESTRICTIONS
**
** RETURN VALUE
**
** APPLICATION USAGE
**	ceil is a version of the POSIX standard function ceil.
**
**	You should only use this version of ceil if your system doesn't
**	provide one in its standard C or math libraries.  The version
**	supplied there will no doubt be much more efficient as it can
**	utilize properties of the internal FP format.
**
** BUGS
*/


double
ceil(x)
double x;
{
  long ip;

  ip = (long) x;

  if (  ( x <= 0.0 )  ||  ( (double) ip == x )  ) {
    return ip;
  } else {
    return ip + 1;
  }

}
