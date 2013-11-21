/*
 * NB: This is a skeleton file.  You must do the following substitutions:
 *	XX	header name, upper-case (e.g. WIN)
 *	xx	header name, lower case (e.g. win)
 *	YY	header field name, upper-case (e.g. BLINE)
 *	yy	header field name, upper-case (e.g. bline)
 *     You must also add/delete code as indicated by "### ... %%%" comments
 */
/* LINTLIBRARY */

#include "ipw.h"

#include "hdrio.h"
#include "xxh.h"

/*
** NAME
**	xxhmake -- make an IPW XXH header
**
** SYNOPSIS
**	#include "xxh.h"
**
**	XXH_T *xxhmake(### initializers %%%)
**	### initializer declaration(s) %%%
**
** DESCRIPTION
**	xxhmake allocates an IPW XX header.  The header is initialized with
**	the remaining arguments.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to new XX header; NULL if error
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

XXH_T          *
xxhmake( /* ### initializers %%% */ )
 /* ### initializer declaration(s) %%% */
{
	XXH_T          *xxhp;		/* -> XX header			 */

 /* ### initializer assertions %%% */

 /*
  * allocate header
  */
 /* NOSTRICT */
	xxhp = (XXH_T *) hdralloc(1, sizeof(XXH_T), ERROR, XXH_HNAME);
	if (xxhp == NULL) {
		return (NULL);
	}
 /*
  * initialize header
  */
 /* ### initialize %%% */
	xxhp->yy = yy;
 /* ### etc. %%% */
#if 0
	if (yy != NULL) {
		xxhp->yy = hstrdup(yy, XXH_HNAME, NO_BAND);
		if (xxhp->yy == NULL) {
			return (NULL);
		}
	}
#endif
 /* ### etc. %%% */

	return (xxhp);
}

#ifndef	lint
static char     rcsid[] = "$Header: xxhmake.c,v 1.7 88/03/12 15:53:30 frew Exp $";

#endif
