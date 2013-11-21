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

#include "xxh.h"

/*
** NAME
**	xxhcheck -- validate components of IPW XXH header
**
** SYNOPSIS
**	#include "xxh.h"
**
**	bool_t xxhcheck(xxhpp, nbands)
**	XXH_T **xxhpp;
**	int nbands;
**
** DESCRIPTION
**	xxhcheck checks that xxhpp points to an array of nbands pointers
**	to valid XXH headers.
**
** RESTRICTIONS
**
** RETURN VALUE
**	TRUE if xxhpp checks OK, else FALSE.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	xxhcheck is not meant to be called by IPW application programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

bool_t
xxhcheck(xxhpp, nbands)
	XXH_T         **xxhpp;		/* -> array of -> XXH header	 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	bool_t          found;		/* ? found at least 1 header	 */

	assert(xxhpp != NULL);
 /*
  * loop through possible bands
  */
	found = FALSE;

	for (band = 0; band < nbands; ++band) {
		XXH_T          *xxhp;	/* -> XXH for current band	 */

		xxhp = xxhpp[band];
		if (xxhp == NULL) {
			continue;
		}

		found = TRUE;
#if 0
		if (xxhp->yy == NULL) {
			usrerr("\"%s\" header, band %d: missing yy",
			       XXH_HNAME, band);
			return (FALSE);
		}
#endif
		if ( /* ### test xxhp->yy %%% */ ) {
			usrerr("\"%s\" header, band %d: %FMT: bad yy",
			       XXH_HNAME, band, xxhp->yy);
			return (FALSE);
		}

 /* ### etc. %%% */
	}

	assert(found);

	return (TRUE);
}

#ifndef	lint
static char     rcsid[] = "$Header: xxhcheck.c,v 1.3 87/10/15 15:08:31 frew Exp $";

#endif
