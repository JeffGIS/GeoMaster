/* LINTLIBRARY */

#include "ipw.h"

#include "_strvec.h"

/*
** NAME
**	freesv -- free an entire strvec
**
** SYNOPSIS
**	int freesv(p);
**	STRVEC_T *p;
**
** DESCRIPTION
**	freesv frees the entire strvec structure.
**
** RESTRICTIONS
**
** RETURN VALUE
**	OK is the free was successful, ERROR otherwise.
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

int
DEFUN(freesv, (p), STRVEC_T *p)
{
	int i;

	if ( p == NULL)
		return (OK);

	assert(ok_sv(p));

	for (i = 0; i < p->n; i++) {
		SAFE_FREE(p->v[i]);
	}
	SAFE_FREE(p->v);
	SAFE_FREE(p);

	return (OK);
}
