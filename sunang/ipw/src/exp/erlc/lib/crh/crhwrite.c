
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   crhwrite.c   1.6   10/19/90";
#endif

/*
** NAME
**	crhwrite -- write an IPW CRH header
**
** SYNOPSIS
**	#include "file.h"
**
**	int crhwrite (fd, crhpp)
**	int fd;
**	CRH_T **crhpp;
**
** DESCRIPTION
**      crhwrite writes the array of CRH headers pointed to by crhpp
**	to file descriptor fd.
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
**	crhwrite is called by IPW application programs to write
**	CRH headers.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	Written by Kelly Longley, ERL-C.
**
** BUGS
**	crhsort expects a *CRH_T, while we pass in a *CLASS.  This is wrong,
**	but I'm not sure I want to fix it.  It should blow up, but it seems
**	to work, so I won't muck with it now.  Dana Jacobsen, ERL-C.
*/

#include "ipw.h"
#include "bih.h"
#include "hdrio.h"
#include "crh.h"
#include "_crh.h"

int
crhwrite (fd, crhpp)
	int             fd;		/* output file descriptor	 */
	CRH_T         **crhpp;		/* -> array of CRH pointers	 */
{
	REG_2 int       band;		/* current header band #	 */
	int             nbands;		/* # bands in output image	 */
	CLASS          *classcpy;	/* copy of CLASS array to sort   */

	nbands = hnbands(fd);

	assert (crhcheck(crhpp, nbands));

   /* loop through possible output bands */

	for (band = 0; band < nbands; ++band) {
		int             i;	/* loop counter			 */
		REG_1 CRH_T    *crhp;	/* -> current CRH		 */
		char            s[HREC_SIZ];	/* output value		 */

		crhp = crhpp[band];
		if (crhp == NULL) {
			continue;
		}

		if (hwprmb(fd, CRH_HNAME, band, CRH_VERSION) == ERROR) {
			return (ERROR);
		}

   	/* write fields */

		(void) sprintf (s, "%d", crhp->nclass);
		if (hputrec(fd, (char *) NULL, CRH_NCLASS, s) == ERROR) { 
			return (ERROR);
		}

		(void) sprintf (s, "%d", crhp->floor);
		if (hputrec(fd, (char *) NULL, CRH_FLOOR, s) == ERROR) { 
			return (ERROR);
		}

		(void) sprintf (s, "%d", crhp->ceil);
		if (hputrec(fd, (char *) NULL, CRH_CEIL, s) == ERROR) { 
			return (ERROR);
		}

		/* copy CLASS array and sort by class numbers */

		classcpy = (CLASS *) ecalloc (crhp->nclass, sizeof(CLASS));
		if (classcpy == NULL) 
			error ("Unable to allocate class structure copy");

		memcpy ((char *) classcpy, crhp->class,
			(xsize_t) crhp->nclass * sizeof(CLASS));

		crhsort (classcpy, SORT_BY_CLASS);

		for (i = 0; i < crhp->nclass; i++) {
			(void) sprintf(s, "%.10g %.10g %.10g",
				       classcpy[i].lo,
				       classcpy[i].hi,
				       classcpy[i].rep);

			if (hputrec(fd, (char *) NULL, CRH_CLASS, s)
			    == ERROR) {
				return (ERROR);
			}
		}

		if (crhp->annot != NULL) {
			if (hputrec(fd, (char *) NULL, CRH_ANNOT, crhp->annot)
			    == ERROR) {
				return (ERROR);
			}
		}

		if (crhp->units != NULL) {
			if (hputrec(fd, (char *) NULL, CRH_UNITS, crhp->units)
			    == ERROR) {
				return (ERROR);
			}
		}


	}

	_crh[fd] = crhpp;
	return (OK);
}
