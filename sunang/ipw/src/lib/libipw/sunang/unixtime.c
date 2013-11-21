/* LINTLIBRARY */

#include <time.h>
#include "ipw.h"
#include "sunang.h"

/*
** NAME
**	unixtime -- create UNIX time structure
**
** SYNOPSIS
**	#include <time.h>
**
**	struct tm *unixtime(year, month, day, hour, minute, second, isdst)
**	int year, month, day, hour, minute, second;
**	bool_t isdst;
**
** DESCRIPTION
**	Unixtime creates a UNIX time structure (see <time.h>) from these
**	input arguments.
**
**	year: fully specified, i.e. 1988, not 88
**	month: 1 - 12, if zero, then day is day of year
**	day: 1 - 31, or day of year if month is zero
**	hour: 0 - 23
**	minute: 0 - 59
**	second: 0 - 59
**	isdst: 0 if daylight time not in effect, non-zero otherwise
**
**	The time may be either local or GMT.  To convert between these,
**	use the routines gmt() and loct().
**
** RESTRICTIONS
**
** RETURN VALUE
**      NULL is returned on EOF or error and a message is saved via
**	usrerr().
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	Most IPW programs that use time as an argument use the UNIX
**	time structure.  Use of this routine is recommended for
**	creation of a time structure.
**
** FUTURE DIRECTIONS
**
** BUGS
**	The basic unit in UNIX time-keeping, the number of seconds
**	since Jan 01, 1970, is stored in a long.  On a 32-bit machine,
**	this restricts the available dates from 1902 to Jan 2038.
*/

#define SEC_DAY	86400
#define SEC_HR	3600
#define SEC_MIN	60

#define EARLIEST	1902
#define LATEST		2037

struct tm      *
DEFUN(unixtime, (year, month, day, hour, minute, second, isdst),
	int             year
   AND  int             month
   AND  int             day
   AND  int             hour
   AND  int             minute
   AND  int             second
   AND  bool_t          isdst)
{
	long            clockt;		/* seconds since 1970/1/1	 */

 /*
  * check arguments
  */
	if (year < EARLIEST) {
		if (year <= 0) {
			usrerr("year = %d invalid", year);
		}
		else if (year < 100) {
			usrerr("year = %d, do you mean 19%02d?", year, year);
		}
		else {
			usrerr("year = %d; UNIX time only supports year >= %d",
			       year, EARLIEST);
		}
		return (NULL);
	}
	if (year > LATEST) {
		usrerr("year = %d; UNIX time only supports year <= %d",
		       year, LATEST);
		return (NULL);
	}
	if (month == 0) {
		if (day < 1 || day > 366) {
			usrerr("day = %d invalid", day);
			return (NULL);
		}
		month = 1;
	}
	else {
		if (month < 1 || month > 12) {
			usrerr("month = %d invalid", month);
			return (NULL);
		}
		if (day < 1 || day > 31) {
			usrerr("day = %d invalid", day);
			return (NULL);
		}
	}
	if (hour < 0 || hour > 23) {
		usrerr("hour = %d invalid", hour);
		return (NULL);
	}
	if (minute < 0 || minute > 59) {
		usrerr("minute = %d invalid", minute);
		return (NULL);
	}
	if (second < 0 || second > 59) {
		usrerr("second = %d invalid", second);
		return (NULL);
	}
 /*
  * days since 1970/1/1
  */
	clockt = julday(year, month, day) - julday(1970, 1, 1);
 /*
  * convert to seconds
  */
	clockt *= SEC_DAY;
	clockt += SEC_HR * hour + SEC_MIN * minute + second;
 /*
  * check for overflow
  */
//	assert((clockt < 0 && year < 1970) || (clockt >= 0 && year >= 1970));
 /*
  * convert to time structure
  */
	return (tmconv(clockt, isdst));
}

#ifndef	lint
static char     rcsid[] = "$Header: unixtime.c,v 1.5 88/11/28 10:37:29 dozier Exp $";

#endif
