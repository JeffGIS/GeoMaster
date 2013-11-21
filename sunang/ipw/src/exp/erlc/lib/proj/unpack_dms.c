
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   unpack_dms.c   1.1   10/15/91";
#endif

/*
** NAME
** 	unpack_dms - convert packed DMS to decimal degrees
** 
** SYNOPSIS
**	double dms (value);
**	double value;
** 
** DESCRIPTION
**	unpack_dms converts the given value in packed DMS format
**	(degrees/minutes/seconds in the format +/-DDDMMSS.SSS where
**	DDD is the degrees, MM is minutes and SS.SSS is seconds.) to
**	decimal degrees.
** 
** RESTRICTIONS
** 
** RETURN VALUE
**	DMS value in double precision.
** 
** GLOBALS ACCESSED
** 
** ERRORS
**	no error check is done on the given value, so garbage in/garbage out.
** 
** WARNINGS
** 
** APPLICATION USAGE
** 
** FUTURE DIRECTIONS
** 
** BUGS
**
*/

#include <math.h>

double unpack_dms (value)
	double		value;		/* input value in packed DMS	 */
{
	int		sign;		/* sign of input value		 */
	int		deg;		/* degrees part			 */
	int		min;		/* minutes part			 */
	float		sec;		/* seconds part			 */
	double		uvalue;		/* absolute value of given value */
	double		ddeg;		/* decimal degrees return value	 */


	if (value >= 0) {
		sign = 1;
		uvalue = value;
	} else {
		sign = -1;
		uvalue = -value;
	}

	deg = uvalue / 10000.0;
	min = fmod (uvalue, 10000.0) / 100.0;
	sec = fmod (uvalue, 100.0);

	ddeg = sign * (deg + min / 60.0 + sec / 3600.0);

	return (ddeg);
}
