
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   crhmake.c   1.4   10/19/90";
#endif

/*
** NAME
**	crhmake -- make an IPW CRH header
**
** SYNOPSIS
**	#include "crh.h"
**
**	CRH_T *crhmake (nclass, lo, hi, rep, floor, ceil, annot, units)
**	int nclass;
**	fpixel_t *lo, *hi, *rep;
**	pixel_t floor, ceil;
**	char *annot;
**	char *units;
**
** DESCRIPTION
**	crhmake allocates a single IPW CR header.  The header is initialized
**	with the arguments.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to new CR header; NULL if error
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

#include "ipw.h"
#include "hdrio.h"
#include "crh.h"
#include "_crh.h"

CRH_T          *
crhmake (nclass, lo, hi, rep, floor, ceil, annot, units)
	int             nclass;		/* #classes     		 */
	fpixel_t       *lo;		/* array of lower range values   */
	fpixel_t       *hi;		/* array of higher range values  */
	fpixel_t       *rep;		/* array of representative vals  */
	pixel_t		floor;		/* class floor value		 */
	pixel_t		ceil;		/* class ceiling value		 */
	char           *annot;		/* annotation                    */
	char           *units;		/* units of data                 */
{
	CRH_T          *crhp;		/* -> CR header			 */
	int             cls;		/* current class #     		 */

	assert (nclass > 0);
	assert (lo != NULL);
	assert (hi != NULL);
	assert (rep != NULL);

   /* allocate header */

	crhp = (CRH_T *) hdralloc (1, sizeof(CRH_T), ERROR, CRH_HNAME);
	if (crhp == NULL) {
		return (NULL);
	}

   /* initialize scalars */

	crhp->nclass = nclass;
	crhp->floor = floor;
	crhp->ceil = ceil;

   /* initialize strings */

	if (annot != NULL && (crhp->annot = hstrdup(annot,
				       CRH_HNAME, NO_BAND)) == NULL) {
		return (NULL);
	}

	if (units != NULL && (crhp->units = hstrdup(units,
				       CRH_HNAME, NO_BAND)) == NULL) {
		return (NULL);
	}


   /* initialize arrays */

	if (_crharrays(crhp) == ERROR) {
		return (NULL);
	}

	for (cls = 0; cls < nclass; cls++) {

		crhp->class[cls].cls = cls + 1;
		crhp->class[cls].lo = lo[cls];
		crhp->class[cls].hi = hi[cls];
		crhp->class[cls].rep = rep[cls];
	}

	return (crhcheck(&crhp, 1) ? crhp : NULL);
}
