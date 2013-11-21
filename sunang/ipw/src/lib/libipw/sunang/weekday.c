/* LINTLIBRARY */
#include <math.h>
#include "ipw.h"
#include "sunang.h"

int
dysize(int year);
/*
** NAME
**	weekday -- day of week
**
** SYNOPSIS
**	int weekday(year, month, day)
**	int year, month, day;
**
** DESCRIPTION
**	Weekday generates the day of the week (Sunday = 0) given year,
**	month, and day.
**
**	year: fully specified, i.e. 1988, not 88
**	month: 1 - 12
**	day: 1 - 31
**
** RESTRICTIONS
**
** RETURN VALUE
**	Day of week (Sunday = 0).
**	NULL is returned on EOF or error.
**
** GLOBALS ACCESSED
**
** ERRORS
**	No diagnostics.
**
** WARNINGS
**
** APPLICATION USAGE
**	Normally weekday is not called by IPW applications programs,
**	except indirectly through unixtime().  The main reason for its
**	existence is that the UNIX gmtime() and localtime() routines do
**	not correctly calculate the day of the week for dates prior to
**	1970.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
DEFUN(weekday, (year, month, day),
	int             year
   AND  int             month
   AND  int             day)
{
	int             days = 0;
	int             y;

 /*
  * The addend is 4 mod 7 (January 1, 1970 was Thursday).
  */
	if (year >= 1970) {
		for (y = 1970; y < year; y++)
			days += dysize(y - 1900);
	}
	else {
		for (y = year; y < 1970; y++)
			days -= dysize(y - 1900);
	}

	days += 4 + yrday(year, month, day);
	if (days < 0) {
		days += ((abs(days) / 7) + 1) * 7;
	}

	return ((days - 1) % 7);
}

#ifndef	lint
static char     rcsid[] = "$Header: weekday.c,v 1.1 88/03/30 15:42:44 dozier Exp $";

#endif
