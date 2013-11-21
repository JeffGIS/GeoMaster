
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   crhsort.c   1.3   10/19/90";
#endif

/*
** NAME
**	crhsort -- return a copy of classes sorted by class number
**
** SYNOPSIS
**	#include "file.h"
**
**	int crhsort (crhp, sortkey)
**	CRH_T *crhp;
**      int sortkey;
**
** DESCRIPTION
**      crhsort sorts the given CRH header according to the given sort key:
**	SORT_BY_CLASS or SORT_BY_RANGE.
**
** RESTRICTIONS
**
** RETURN VALUE
**	OK for success, ERROR for failure
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	crhsort is called by IPW application programs to sort the CRH
**	header by class number or by range.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include "ipw.h"
#include "crh.h"

static int EXFUN(compar_cls, (CLASS *class1, CLASS *class2));
static int EXFUN(compar_rng, (CLASS *class1, CLASS *class2));

int
crhsort (crhp, sortkey)
	CRH_T         *crhp;		/* -> CRH for a specific band	 */
	int	       sortkey;		/* sort key			 */
{

	switch (sortkey) {

		case SORT_BY_CLASS:
			(void) SORT_ALG (crhp->class, crhp->nclass,
				sizeof(CLASS), compar_cls);
			break;

		case SORT_BY_RANGE:
			(void) SORT_ALG (crhp->class, crhp->nclass,
				sizeof(CLASS), compar_rng);
			break;
		default:
			usrerr ("illegal key for crhsort");
			return (ERROR);
	}

	return (OK);

}

static int
compar_cls (class1, class2)
CLASS *class1, *class2;
{
	if (class1->cls >= class2->cls)
		return (1);
	else
		return (-1);
}

static int
compar_rng (class1, class2)
CLASS *class1, *class2;
{
	if (class1->rep >= class2->rep)
		return (1);
	else
		return (-1);
}
