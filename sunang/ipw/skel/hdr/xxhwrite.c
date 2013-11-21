/*
 * NB: This is a skeleton file.  You must do the following substitutions:
 *	XX	header name, upper-case (e.g. WIN)
 *	xx	header name, lower case (e.g. win)
 *	YY	header field name, upper-case (e.g. BLINE)
 *	yy	header field name, upper-case (e.g. bline)
 *	CVT	header field output function (e.g. itoa)
 *     You must also add/delete code as indicated by "### ... %%%" comments
 */
/* LINTLIBRARY */

#include "ipw.h"

#include "hdrio.h"
#include "xxh.h"

/*
** NAME
**	xxhwrite -- write an IPW XXH header
**
** SYNOPSIS
**	#include "file.h"
**
**	int xxhwrite(fd, xxhpp)
**	int fd;
**	XXH_T **xxhpp;
**
** DESCRIPTION
**      xxhwrite writes the array of XXH headers pointed to by xxhpp
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
**	xxhwrite is called by IPW application programs to write
**	XXH headers.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
xxhwrite(fd, xxhpp)
	int             fd;		/* output file descriptor	 */
	XXH_T         **xxhpp;		/* -> array of XXH pointers	 */
{
	REG_2 int       band;		/* current header band #	 */
	int             nbands;		/* # bands in output image	 */

	nbands = hnbands(fd);

	assert(xxhcheck(xxhpp, nbands));
 /*
  * loop through possible output bands
  */
	for (band = 0; band < nbands; ++band) {
		REG_1 XXH_T    *xxhp;	/* -> current XXH		 */
		char            s[HREC_SIZ];	/* output value		 */

		xxhp = xxhpp[band];
		if (xxhp == NULL) {
			continue;
		}
 /*
  * write preamble
  */
		if (hwprmb(fd, XXH_HNAME, band, XXH_VERSION) == ERROR) {
			return (ERROR);
		}
 /*
  * write fields
  */
		if (hputrec(fd, (char *) NULL, XXH_YY,
			    CVT(s, xxhp->yy))
		    == ERROR) {
			return (ERROR);
		}
 /* ### etc. %%% */
	}

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: xxhwrite.c,v 1.4 87/10/15 15:08:42 frew Exp $";

#endif
