/* LINTLIBRARY */

#include <time.h>
#include "ipw.h"
#include "sunang.h"

#include "sunlight.h"

/*
** NAME
**	krontime -- times and weights for Kronrod quadrature
**
** SYNOPSIS
**	#include "sunlight.h"
**
**	struct qt **krontime(nkpts, t1, t2, lat, lon)
**	int nkpts;
**	struct tm *t1, *t2;
**	double lat, lon;
**
** DESCRIPTION
**	Creates time structures for Kronrod quadrature between t1 and
**	t2.  nkpts is the number of Kronrod points, currently either 15
**      or 21.  lat and lon are in radians.  Returned is an array of
**	pointers to time structures.
**
** RESTRICTIONS
**	nkpts must be either 15 or 21.
**
** RETURN VALUE
**	NULL is returned on EOF or error.
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
*/

struct qt     **
DEFUN( krontime, (nkpts, t1, t2, lat, lon),
	int             nkpts		/* # Kronrod quadruature pts	 */
   AND  struct tm      *t1		/* time at beginning		 */
   AND  struct tm      *t2		/* time at end			 */
   AND  double          lat		/* latitude (radians)		 */
   AND  double          lon)		/* longitude (radians)		 */
{
	struct qt     **qp;		/* -> return value		 */
	long            clock;
	double          clock1;		/* # sec since 70/1/1 for t1	 */
	double          clock2;		/* # sec since 70/1/1 for t2	 */
	double         *xgk;		/* -> abscissae			 */
	double         *wgk;		/* -> weights			 */
	int             j;
	double          rv;		/* radius vector (not used)	 */
	double          declin;		/* declination			 */
	double          omega;		/* solar longitude		 */

 /*
  * array of pointers to results
  */
 /* NOSTRICT */
	qp = (struct qt **) ecalloc(nkpts, sizeof(struct qt *));
	if (qp == NULL) {
		return (NULL);
	}
	for (j = 0; j < nkpts; ++j) {
 /* NOSTRICT */
		qp[j] = (struct qt *) ecalloc(nkpts, sizeof(struct qt));
		if (qp[j] == NULL) {
			return (NULL);
		}
	}
 /*
  * seconds since midnight 1970/1/1 for t1 and t2
  */
	clock1 = SEC_DAY *
		(julday(1900 + t1->tm_year, 1 + t1->tm_mon, t1->tm_mday)
		 - julday(1970, 1, 1))
		+ SEC_HR * t1->tm_hour
		+ SEC_MIN * t1->tm_min
		+ t1->tm_sec;

	clock2 = SEC_DAY *
		(julday(1900 + t2->tm_year, 1 + t2->tm_mon, t2->tm_mday)
		 - julday(1970, 1, 1))
		+ SEC_HR * t2->tm_hour
		+ SEC_MIN * t2->tm_min
		+ t2->tm_sec;
 /*
  * Kronrod abscissae and weights
  */
 /* NOSTRICT */
	xgk = (double *) ecalloc(nkpts, sizeof(double));
	if (xgk == NULL)
		return (NULL);
 /* NOSTRICT */
	wgk = (double *) ecalloc(nkpts, sizeof(double));
	if (wgk == NULL)
		return (NULL);
	switch (nkpts) {
	case 15:
		vqk15((double) SEC_DAY, clock1, clock2, xgk, wgk);
		break;
	case 21:
		vqk21((double) SEC_DAY, clock1, clock2, xgk, wgk);
		break;
	default:
		usrerr("nkpts = %d");
		return (NULL);
	}
 /*
  * transfer the weights to the output structures, compute time
  * structures, solar zenith angles and azimuths
  */
	for (j = 0; j < nkpts; ++j) {
		(qp[j])->q_x = xgk[j];
		(qp[j])->q_wt = wgk[j];
		clock = xgk[j] + 0.5;
		(qp[j])->q_tp = tmconv(clock, 0);
		if (ephemeris((qp[j])->q_tp, &rv, &declin, &omega) == ERROR)
			return (NULL);
		switch (sunpath(lat, lon, declin, omega,
			      &((qp[j])->q_cos), &((qp[j])->q_azm))) {
		case 1:		/* sun below horizon */
			(qp[j])->q_cos = 0;
			break;
		case ERROR:
			return (NULL);
		default:
			;
		}
	}

	return (qp);
}

#ifndef	lint
static char     rcsid[] = "$Header: krontime.c,v 1.2 88/04/03 11:06:52 dozier Exp $";

#endif
