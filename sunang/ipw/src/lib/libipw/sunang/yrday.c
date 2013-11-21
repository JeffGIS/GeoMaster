/* LINTLIBRARY */

#include "ipw.h"
#include "sunang.h"

/*
** NAME
**	yrday -- returns day-of-year corresponding to (year, month, day)
**
** SYNOPSIS
**	int yrday(year, month, day)
**	int year, month, day;
**
** DESCRIPTION
**	Yrday returns the day of the year, which begins at noon of the date
**	specified by year, month, and day.  This is sometimes incorrectly
**	called the Julian day.  Positive year signifies A.D.; negative year
**	signifies B.C (remember that the year after 1 B.C. is 1 A.D).
**
** RESTRICTIONS
**
** RETURN VALUE
**	integer day of year; 0 if error
**
** GLOBALS ACCESSED
**
** ERRORS
**	yrday: there is no year zero; zero returned
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
DEFUN(yrday, (year, month, day),
	int             year
   AND  int             month
   AND  int             day)
{
	int		jnow;		/* Julian day			*/
	int		jfirst;		/* Julian day of yr/01/01	*/

	if ((jnow = julday(year, month, day)) == 0)
		return (0);

	jfirst = julday(year, 1, 1);

	return (1 + jnow - jfirst);
}

#ifndef	lint
static char     rcsid[] = "$Header: yrday.c,v 1.4 88/03/13 22:01:15 dozier Exp $";

#endif
