
/* LINTLIBRARY */

#include "ipw.h"

/*
** NAME
**	emalloc -- interface to UNIX malloc, calloc
**
** SYNOPSIS
**	addr_t emalloc(nelem, elsize)
**	int nelem, elsize;
**
** DESCRIPTION
**	emalloc is the IPW interface to the UNIX function malloc.  emalloc
**	checks that its arguments are positive nonzero values and sets an
**	IPW error condition if the allocation fails.  This function is
**	almost identical to ecalloc, but does not initialize the returned
**	area.
**
** RESTRICTIONS
**	Since malloc does not initialize the returned area to 0 like calloc
**	does, this function should only be used when time is critical and
**	it is guaranteed to be initialized first.
**
** RETURN VALUE
**	pointer to allocated memory; else NULL if allocation failed
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	Use emalloc instead of malloc or ecalloc when time is critical, and
**	the returned space does not need to be initialized.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

addr_t
emalloc(nelem, elsize)
	int		nelem;
	int		elsize;
{
	addr_t		rtn;
	xsize_t		nbytes;

	nbytes = nelem * elsize;
	assert(nbytes > 0);

	rtn = malloc( nbytes );
	if (rtn == NULL) {
		syserr();
	}

	return (rtn);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/util/RCS/ecalloc.c,v 1.0 93/07/28 17:19:24 jacobsd Exp $";
#endif
