/* LINTLIBRARY */

/*
**  NAME
**	gmt - convert local time to GMT
**	loct - convert GMT time to local
**
**  SYNOPSIS
**	#include	<time.h>
**
**	struct tm *gmt(local, zone)
**	struct tm *local;
**	int zone;
**
**	struct tm *loct(grnwch, zone, isdst)
**	struct tm *grnwch;
**	int zone;
**	bool_t isdst;
**
**  DESCRIPTION
**	Gmt converts local time into GMT; the input time structure
**	"local" specifies the local time (see <time.h>).  Loct converts
**	GMT into local time; the input time structure "grnwch"
**	specifies GMT and the "isdst" flag specifies that the output
**	time structure is daylight time.  The "zone" variable specifies
**	the time zone, in minutes of time west of Greenwich.  The
**	output structure will contain the GMT or local time, with all
**	fields filled in.  The routines allocate new output time
**	structures with each call, so that they may be called
**	repeatedly with different arguments without over-writing static
**	data.
**
**  DIAGNOSTICS
**	returns NULL on error and writes message via usrerr()
**
*/

#include <time.h>
#include "ipw.h"
#include "sunang.h"

#define MAXZ	(180 * 4)		/* max/min zone		 */
#define SEC_DAY	86400			/* sec / day		 */
#define SEC_MIN	60			/* sec / min		 */
#define SEC_HR	3600			/* sec / hr		 */

static struct tm *
DEFUN(tconv, (tp, zone, locflag, isdst),
	struct tm      *tp		/* input - local/GMT time	 */
   AND  int             zone		/* minutes W of Greenwich	 */
   AND  bool_t          locflag		/* ? input time is local	 */
   AND  bool_t          isdst)		/* ? output local time is dst	 */
{
	long            clockt;		/* sec since midnight 1970/1/1	 */

	assert(tp != NULL);
 /*
  * check zone
  */
	if (abs(zone) > MAXZ) {
		usrerr("gmt: zone = %d", zone);
		return (NULL);
	}
 /*
  * convert time to seconds since midnight 1970/1/1
  */
	clockt = SEC_DAY *
		(julday(1900 + tp->tm_year, 1 + tp->tm_mon, tp->tm_mday)
		 - julday(1970, 1, 1))
		+ tp->tm_hour * SEC_HR
		+ tp->tm_min * SEC_MIN
		+ tp->tm_sec;
 /*
  * convert local time to GMT
  */
	if (locflag) {
 /*
  * convert daylight to standard
  */
		if (tp->tm_isdst) {
			clockt -= SEC_HR;
		}
 /*
  * convert to Greenwich
  */
		clockt += zone * SEC_MIN;
	}
 /*
  * else convert GMT to local
  */
	else {
		clockt -= zone * SEC_MIN;
		if (isdst) {
			clockt += SEC_HR;
		}
	}
 /*
  * Convert to output time structure.
  */
	return(tmconv(clockt, locflag && isdst));
}



struct tm      *
DEFUN(gmt, (local, zone),
	struct tm      *local 
   AND  int             zone)
{
	return (tconv(local, zone, 1, 0));
}

struct tm      *
DEFUN(loct, (grnwch, zone, isdst),
	struct tm      *grnwch
   AND  int             zone
   AND  bool_t          isdst)
{
	return (tconv(grnwch, zone, 0, isdst));
}


#ifndef	lint
static char     rcsid[] = "$Header: gmt.c,v 1.4 89/04/18 15:30:46 frew Exp $";

#endif
