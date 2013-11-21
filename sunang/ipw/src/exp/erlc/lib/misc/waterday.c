
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   waterday.c   1.5   9/12/91";
#endif

/*
** NAME
** 	waterday -- convert from date (year, month, day) to water day, year
** 
** SYNOPSIS
**	int waterday (year, mon, day, wyear, wday)
** 	int year, mon, day;
**	int *wyear, *wday;
** 
** DESCRIPTION
**	waterday returns the day (0-365) in the water year (Oct 1 - Sept. 30)
**	corresponding to the given year, month, and day.
** 
** RESTRICTIONS
** 
** RETURN VALUE
** 
** GLOBALS ACCESSED
** 
** ERRORS
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

#include <time.h>
#include "ipw.h"

#define OCT_1	273.0	/* days before Oct. 1 (non-leap year)	 */
#define	END_YR	92.0	/* days from Oct. 1 to Dec. 31		 */

void
waterday (year, mon, day, wyear, wday)
	int		year;		/* year				 */
	int		mon;		/* month			 */
	int		day;		/* day				 */
	int	       *wyear;		/* water year			 */
	int	       *wday;		/* water day			 */
{
	int		jd;		/* Julian day			 */
	struct tm      *time;		/* tm struct for given date	 */
	bool_t		leap;		/* flag for leap year		 */
	int		Oct_1_days;	/* days before Oct 1		 */

	struct tm      *unixtime();

	if ((time = unixtime (year, mon, day, 0, 0, 0, 0)) == NULL)
		error ("Illegal time %d/%d/%d", mon, day, year);

	jd = time->tm_yday;

	if (dysize(year) == 365) {
		leap = FALSE;
		Oct_1_days = OCT_1;
	} else {
		leap = TRUE;
		Oct_1_days = OCT_1 + 1;
	}

	if (jd >= Oct_1_days) {
		*wday = jd - Oct_1_days;
		*wyear = year + 1;
	} else {
		*wday = jd + END_YR;
		*wyear = year;
	}
}
