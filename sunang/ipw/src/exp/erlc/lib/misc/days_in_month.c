
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   days_in_month.c   1.2   9/12/91";
#endif

/*
** NAME
** 	days_in_month - return number of days in given month of year
** 
** SYNOPSIS
**	day_count = days_in_month (year, month)
**	int year, month;
** 
** DESCRIPTION
**	days_in_month retuns the number of days in the given month of
**	the given year.
** 
** RESTRICTIONS
** 
** RETURN VALUE
**	number of days in month
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

extern int dysize();

int days_in_month (year, month)
	int		year;
	int		month;
{
	static int NDAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int ndays;

   /* Check for leap year for February */

	ndays = NDAYS[month-1];

	if (month == 2) {
		if (dysize(year) == 366) {
			ndays = 29;
		}
	}

	return (ndays);
}
