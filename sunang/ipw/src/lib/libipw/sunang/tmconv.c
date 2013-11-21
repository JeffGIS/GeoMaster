/*
** NAME
**	tmconv -- convert clock time to time structure
**
** SYNOPSIS
**	#include <time.h>
**
**	struct tm *tmconv(clockt, isdst)
**	long clockt;
**	bool_t isdst;
**
** DESCRIPTION
**	Tmconv converts UNIX clock time (seconds since midnight
**	1970/1/1) to a time structure.  The flag isdst is non-zero is
**	the time is daylight time.
**
** RESTRICTIONS
**
** RETURN VALUE
**
** GLOBALS ACCESSED
**
** ERRORS
**	Dies with error message if something wrong.
**
** WARNINGS
**
** APPLICATION USAGE
**	Tmconv() should be used instead of the UNIX routine gmtime().
**	Usually the IPW routine unixtime() should be used, unless the
**	program requires that 'clock' be computed.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

/* LINTLIBRARY */

#include <time.h>
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include "ipw.h"
#include "sunang.h"

struct tm      *
DEFUN(tmconv, (clockt, isdst),
	long            clockt		/* seconds since 1970/1/1	 */
   AND  bool_t          isdst)		/* ? daylight time		 */
{
	time_t	t;
	struct tm      *tp;		/* -> output structure		 */

 /*
  * Convert to time structure. Because the UNIX routine gmtime
  * over-writes a static buffer area with each call, we create a new
  * local buffer area. Therefore tmconv can be called repeatedly with
  * different arguments to produce different time structures.
  */
 /* NOSTRICT */
  tp = (struct tm *) calloc(1, sizeof(struct tm));
  t = clockt;
	(void) memcpy((char *) tp, (char *) gmtime(&t), sizeof(struct tm));
 /*
  * turn on daylight savings flag
  */
	tp->tm_isdst = (isdst) ? 1 : 0;
 /*
  * the UNIX routines do not compute the day of the week for years
  * before 1970
  */
	if (tp->tm_year < 70) {
		tp->tm_wday = weekday(1900 + tp->tm_year,
				      1 + tp->tm_mon, tp->tm_mday);
	}
 /*
  * all done
  */
	return (tp);
}

#ifndef	lint
static char     rcsid[] = "$Header: tmconv.c,v 1.1 88/04/02 14:47:55 dozier Exp $";

#endif
