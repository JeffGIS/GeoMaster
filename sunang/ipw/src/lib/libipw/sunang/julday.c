/* LINTLIBRARY */

#include "ipw.h"
#include "sunang.h"

/*
** NAME
**	julday -- returns Julian date corresponding to (year, month, day)
**
** SYNOPSIS
**	int julday(year, month, day)
**	int year, month, day;
**
** DESCRIPTION
**	Julday returns the Julian date, which begins at noon of the
**	date specified by year, month, and day.  Positive year
**	signifies A.D.; negative year signifies B.C (remember that the
**	year after 1 B.C. is 1 A.D).
**
**	This implementation of julday was translated from Fortran code in
**	Press et al., Numerical Recipes, ch. 1.
**
** RESTRICTIONS
**
** RETURN VALUE
**	integer Julian date; 0 if error
**
** GLOBALS ACCESSED
**
** ERRORS
**	julday: there is no year zero; zero returned
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

/*
 * Gregorian calendar begins 1582/10/15
 */
#define IGREG		( (long) (15 + 31 * (long) (10 + 12 * 1582)) )

#define	DAYS_YR		365.25
#define	DAYS_MO		30.6001

#define	MAGIC		1720995L	/* where did this come from?	 */

int
DEFUN(julday, (year, month, day),
	int             year
   AND  int             month
   AND  int             day)
{
	int             ja;
	int             jm;
	long            yd;
	int             jy;

	if (year == 0) {
		usrerr("julday: there is no year zero; zero returned");
		return (0);
	}

	if (year < 0) {
		++year;
	}

	if (month > 2) {
		jy = year;
		jm = month + 1;
	}
	else {
		jy = year - 1;
		jm = month + 13;
	}

	yd = (long) (DAYS_YR * jy) + (long) (DAYS_MO * jm) + day + MAGIC;

	if ((long) (day + 31 * (long) (month + 12 * year)) >= IGREG) {
		ja = 0.01 * jy;
		yd += 2 - ja + (int) (0.25 * ja);
	}
 /* NOSTRICT */
	return ((int) yd);
}



#ifndef	lint
static char     rcsid[] = "$Header: julday.c,v 1.1 88/02/22 17:37:31 dozier Exp $";

#endif
