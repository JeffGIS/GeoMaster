
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   crhdup.c   1.4   10/19/90";
#endif

/*
** NAME
**	crhdup -- duplicate an IPW CRH header
**
** SYNOPSIS
**	#include "crh.h"
**
**	CRH_T **crhdup(old, nbands)
**	CRH_T **old;
**	int nbands;
**
** DESCRIPTION
**	crhdup creates a duplicate of the IPW CRH header pointed to by
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
**	crhdup is an easy way for applications programs to create a new CRH
**	(e.g., for an output image, after having read the CRH for an input
**	image).
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include "ipw.h"
#include "crh.h"
#include "_crh.h"

CRH_T         **
crhdup(oldhpp, nbands)
	CRH_T         **oldhpp;		/* -> old CRH array		 */
	int             nbands;		/* # header bands		 */
{
	int             band;		/* loop counter			 */
	CRH_T         **newhpp;		/* -> new CRH array		 */


   /* source CRH must be valid */

	assert (crhcheck(oldhpp, nbands));

   /* allocate new pointer array */

	newhpp = (CRH_T **) hdralloc (nbands, sizeof(CRH_T *), ERROR,
			CRH_HNAME);
	if (newhpp == NULL) {
		return (NULL);
	}

   /* duplicate headers */

	for (band = 0; band < nbands; ++band) {
		CRH_T          *newhp;	/* -> new CRH			 */
		CRH_T          *oldhp;	/* -> new CRH			 */

		oldhp = oldhpp[band];
		newhp = (CRH_T *) hdralloc(1, sizeof(CRH_T), ERROR,
					   CRH_HNAME);
		if (newhp == NULL) {
			return (NULL);
		}

   /* duplicate scalar fields */

		bcopy ((char *) oldhp, (char *) newhp, sizeof(CRH_T));

   /* duplicate string fields */

		if (oldhp->annot != NULL && (newhp->annot =
		    hstrdup(oldhp->annot, CRH_HNAME, band)) == NULL) {
			return (NULL);
		}

		if (oldhp->units != NULL && (newhp->units =
		    hstrdup(oldhp->units, CRH_HNAME, band)) == NULL) {
			return (NULL);
		}
 
   /* duplicate arrays */

		if (_crharrays(newhp) == ERROR) {
			return (NULL);
		}

		(void) memcpy((char *) newhp->class, (char *) oldhp->class,
			      (xsize_t) newhp->nclass * sizeof(CLASS));
 
   /* done with this band's header */

		newhpp[band] = newhp;
	}

	return (newhpp);
}
