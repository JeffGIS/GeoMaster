
/* LINTLIBRARY */

#include <time.h>

/*
** NAME
**	difftime -- compute the different between two calendar times
**
** SYNOPSIS
**	double difftime(time_t time1, time_t time0)
**
** RETURN VALUE
**	The difftime function returns the difference expressed in seconds
**	as a double.
**
** WARNING
**	This function is correct ONLY if time_t is expressed in seconds,
**	which happens to be true for most machines.  If this is not true,
**	this function will return garbage.  Scream at your vendor to
**	implement difftime (if they're POSIX or "Standard C", they should
**	have it), especially if they're going to have bizzare time_t
**	formats (after all, that's the point of difftime and such -- to
**	allow vendors to make time_t be anything they want).
*/

double
difftime(time1, time0)
	time_t	time1;
	time_t	time0;
{
  return (double) (time1 - time0);
}
