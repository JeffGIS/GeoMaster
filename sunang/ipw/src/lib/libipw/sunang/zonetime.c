
/*
** NAME
**	zonetime -- return time zone name given offset from GMT
**
** SYNOPSIS
**	char *zonetime(int zone, int dst)
**
** DESCRIPTION
**	zonetime returns the short time zone name given an offset from GMT
**	(now known as UTC) time and a flag as to whether Daylight Savings
**	time is in effect.  DST can affect the string returned.
**
** RESTRICTIONS
**	Since a single longitude can run through multiple countries, the
**	string returned is not correct for all countries.  Many times do
**	not have timezone's associated with them, so are returned as a
**	string of the form "GMT+0:00" where "+0:00" is the offset given.
**
** RETURN VALUE
**	A string represtening the time zone name.
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
** HISTORY
**	5/3/93	Written by Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	UNIX: timezone (Sun 3C)
*/

#include <time.h>
#include "ipw.h"
#include "sunang.h"

char *
DEFUN( zonetime, (zone, dst),
    int zone
AND int dst)
{
  char *z;
  char *zp;
  char tempz[255];
  int   hr, min;

  z = NULL;

  switch (zone) {
		/* New Zealand */
    case -720:	z = strdup("NZ%T");	break;
		/* Australia/NSW (Tasmania,Queensland) */
    case -600:	z = strdup("EST");	break;
		/* Australia/North */
    case -570:	z = strdup("CST");	break;
		/* Japan if not dst, Korean otherwise */
    case -540:  z = strdup( (dst) ? "K%T" : "JST");	break;
		/* Australia/West if not dst, PRChina otherwise (could also be
		   CST:ROC, HKT:HongKong, SST:Singapore) */
    case -480:	z = strdup( (dst) ? "C%T" : "WST");	break;
		/* Iran */
    case -210:	z = strdup("I%T");	break;
		/* Turkey (West Soviet Union) */
    case -180:	z = strdup("EET%");	break;
		/* East Europe */
    case -120:	z = strdup("EET%");	break;
		/* Middle Europe (Poland) */
		/* Also known as CET for Central European Time */
    case  -60:	z = strdup("MET%");	break;
		/* UTC time if not dst, otherwise British Summer Time */
		/* Ought to be WET% for Western Europe & Iceland */
    case    0:	z = strdup( (dst) ? "BST" : "GMT");	break;
		/* Brazil/DeNoranha */
    case  120:	z = strdup("F%T");	break;
		/* Brazil/East */
    case  180:	z = strdup("E%T");	break;
		/* Canada/NewFoundland */
    case  210:	z = strdup("N%T");	break;
		/* Canada/Atlantic (W%T: Brazil/West, C%T: Chile/Continental) */
    case  240:	z = strdup("A%T");	break;
		/* US/Eastern (Canada/Eastern, Cuba, A%T: Brazil/Acre */
    case  300:	z = strdup("E%T");	break;
		/* US/Central (Canada/Central, Mexico/Central, E%T:Chile/EasterIsland */
    case  360:	z = strdup("C%T");	break;
		/* US/Mountain (Canada/Mountain, Mexico/BajaSur) */
    case  420:	z = strdup("M%T");	break;
		/* US/Pacific (Canada/Pacific, Mexico/BajaNorte) */
    case  480:	z = strdup("P%T");	break;
		/* US/Alaska (Y%T: Canada/Yukon) */
    case  540:	z = strdup("AK%T");	break;
		/* US/Hawaii if not dst, otherwise US/Aleutian */
    case  600:	z = strdup( (dst) ? "HA%T" : "HST");	break;
		/* US/Samoa */
    case  660:	z = strdup("SST");	break;
    default:
			hr = zone / 60;
			min = ABS(zone - (hr * 60));
			hr = -hr;
			(void) sprintf(tempz, "GMT%+d:%02d", hr, min);
			z = strdup(tempz);
  }
  if (z == NULL)
    return(strdup("No data"));
  zp = strchr(z, '%');
  if (zp != NULL)
    *zp = (dst) ? 'D' : 'S';

  return(z);
}


#ifdef TEST_MAIN

char *timezone();

void main() {
  int i;

  for (i=-13*60; i < (14*60); i+= 30) {
    printf("%+4d, %d, Zone %20s : %20s\n", i, 0, timezone(i,0), zonetime(i,0) );
    printf("%+4d, %d, Zone %20s : %20s\n", i, 1, timezone(i,1), zonetime(i,1) );
  }
}
#endif
