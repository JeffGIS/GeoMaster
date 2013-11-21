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
**	xxhdup -- duplicate an IPW XXH header
**
** SYNOPSIS
**	#include "xxh.h"
**
**	XXH_T **name(old)
**	XXH_T **old;
**
** DESCRIPTION
**	xxhdup creates a duplicate of the IPW XXH header pointed to by
**	"old".
**
** RESTRICTIONS
**
** RETURN VALUE
**	A pointer to the duplicate header is returned.  NULL is returned if
**	an error occurs.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	xxhdup is an easy way for applications programs to create a new XXH
**	(e.g., for an output image, after having read the XXH for an input
**	image).
**
** FUTURE DIRECTIONS
**
** BUGS
*/

XXH_T         **
xxhdup(oldhpp, nbands)
	XXH_T         **oldhpp;		/* -> old XXH array		 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	XXH_T         **newhpp;		/* -> new XXH array		 */

 /*
  * source XXH must be valid
  */
	assert(xxhcheck(oldhpp, nbands));
 /*
  * allocate new pointer array
  */
 /* NOSTRICT */
	newhpp = (XXH_T **) hdralloc(nbands, sizeof(XXH_T *), ERROR,
				     XXH_HNAME);
	if (newhpp == NULL) {
		return (NULL);
	}
 /*
  * duplicate headers
  */
	for (band = 0; band < nbands; ++band) {
		XXH_T          *newhp;	/* -> new XXH			 */
		XXH_T          *oldhp;	/* -> new XXH			 */

		oldhp = oldhpp[band];
 /* NOSTRICT */
		newhp = (XXH_T *) hdralloc(1, sizeof(XXH_T), ERROR,
					   XXH_HNAME);
		if (newhp == NULL) {
			return (NULL);
		}
#if 0
 /*
  * duplicate scalar fields
  */
 /* NOSTRICT */
		bcopy((char *) oldhp, (char *) newhp, sizeof(XXH_T));
#endif
#if 0
 /*
  * duplicate string fields
  */
		newhp->yy = hstrdup(oldhp->yy, XXH_HNAME, band);
		if (newhp->yy == NULL) {
			return (NULL);
		}

 /* ### etc. %%% */
#endif
		newhpp[band] = newhp;
	}

	return (newhpp);
}

#ifndef	lint
static char     rcsid[] = "$Header: xxhdup.c,v 1.3 87/10/15 15:08:35 frew Exp $";

#endif
