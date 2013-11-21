
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   las_dms.c   1.1   10/1/91";
#endif

/*
** NAME
** 	las_dms - convert decimal degrees to packed DMS
** 
** SYNOPSIS
**	double las_dms (value);
**	double value;
** 
** DESCRIPTION
**	las_dms converts the given value in decimal degrees to packed 
**	degrees/minutes/seconds in the format +/-DDDMMMSSS.SSS where
**	DDD is the degrees, MMM is minutes and SSS.SSS is seconds.
** 
** RESTRICTIONS
** 
** RETURN VALUE
**	packed DMS value in double precision.
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

double las_dms (value)
	double		value;		/* input value in decimal deg	 */
{
	int		sign;		/* sign of input value		 */
	int		deg;		/* degrees part			 */
	int		min;		/* minutes part			 */
	float		sec;		/* seconds part			 */
	double		uvalue;		/* absolute value of given value */
	double		dms;		/* DMS return value		 */


	sign = 1;
	if (value < 0)
		sign = -1;
	uvalue = fabs (value);

	deg = uvalue;
	min = (uvalue - deg) * 60.0;
	sec = (uvalue - deg - min / 60.0) * 3600;

	/* round seconds to nearest 1000th */

	sec = (int) (sec * 1000 + 0.5) / 1000.0;

	if (sec >= 60) {
		sec = 0.0;
		min++;
		if (min >= 60) {
			min = 0;
			deg++;
		}
	}

	dms = ((deg * 1000000) + (min * 1000) + sec) * sign;

	return (dms);
}
