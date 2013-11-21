#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   _crharrays.c   1.4   10/19/90";
#endif

/*
** NAME
**	_crharrays -- allocate arrays for CR header
**
** SYNOPSIS
**	int _crharrays (crhp)
**	CRH_T *crhp;
**
** DESCRIPTION
**	_crharrays allocates the arrays required by the CR header pointed to
**	by crhp.
**
** RESTRICTIONS
**	crhp->nclass must be set before _crharrays is called.
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
**	_crharrays is not meant to be called by IPW applications programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include "ipw.h"
#include "crh.h"

int
_crharrays (crhp)
	CRH_T          *crhp;		/* -> CR header			 */
{
	assert (crhp != NULL);
	assert (crhp->nclass > 0);

	crhp->class = (CLASS *) ecalloc(crhp->nclass, sizeof(CLASS));
	if (crhp->class == NULL) {
		return (ERROR);
	}

	return (OK);
}
