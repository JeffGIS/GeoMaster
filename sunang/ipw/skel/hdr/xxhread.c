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
**	xxhread -- read an IPW XXH header
**
** SYNOPSIS
**	#include "xxh.h"
**
**	XXH_T **xxhread(fd)
**	int fd;
**
** DESCRIPTION
**	xxhread reads a XXH image header from file descriptor fd.  An array
**	of XXH_T pointers is allocated, one per band.  If a band has a XXH
**	header, then an XXH_T header is allocated and its address is placed
**	in the corresponding array element; otherwise, the corresponding
**	array element is NULL.
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to array of XXH_T pointers; else NULL if EOF or error
**
** GLOBALS ACCESSED
**
** ERRORS
**	can't allocate array of "xx" header pointers
**	"xx" header: bad band "{band}"
**	can't allocate "xx" header
**	"xx" header: key "{key}" has no value
**	"xx" header: bad key "{key}"
**
** WARNINGS
**	Before calling xxhread, the caller must verify (by calling hrname())
**	that a XXH header is available for ingesting.
**
** APPLICATION USAGE
**	xxhread is called by application programs to ingest XXH headers.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

XXH_T         **
xxhread(fd)
	int             fd;		/* image file descriptor	 */
{
	char           *hname;		/* current header name		 */
	int             nbands;		/* # bands / pixel		 */
	XXH_T         **xxhpp;		/* -> array of XXH pointers	 */

 /*
  * allocate array of header pointers
  */
	nbands = hnbands(fd);
	assert(nbands > 0);

 /* NOSTRICT */
	xxhpp = (XXH_T **) hdralloc(nbands, sizeof(XXH_T *), fd,
				    XXH_HNAME);
	if (xxhpp == NULL) {
		return (NULL);
	}
 /*
  * loop through per-band headers
  */
	while ((hname = hrname(fd)) != NULL && streq(hname, XXH_HNAME)) {
		int             band;	/* current header band #	 */
		int             err;	/* hgetrec return value		 */
		XXH_T          *xxhp;	/* -> current XXH		 */

		char            key[HREC_MAX + 1];	/* keyword	 */
		char            value[HREC_MAX + 1];	/* value string	 */

 /*
  * get header band #
  */
		band = hrband(fd);
		if (band < 0 || band >= nbands) {
			uferr(fd);
			usrerr("\"%s\" header: bad band \"%d\"",
			       XXH_HNAME, band);
			return (NULL);
		}
 /*
  * allocate header
  */
 /* NOSTRICT */
		xxhp = (XXH_T *) hdralloc(1, sizeof(XXH_T), fd,
					  XXH_HNAME);
		if (xxhp == NULL) {
			return (NULL);
		}

		xxhpp[band] = xxhp;
 /*
  * ingest records
  */
		while ((err = hgetrec(fd, (char *) NULL, key, value))
		       == HGOT_DATA) {
 /*
  * ignore all-comment records
  */
			if (key[0] == EOS) {
				continue;
			}
 /*
  * barf if missing value
  */
			if (value[0] == EOS) {
				uferr(fd);
				usrerr("\"%s\" header, key \"%s\": no value",
				       XXH_HNAME, key);
				return (NULL);
			}
 /*
  * match key to header field, ingest value
  */
			if (streq(key, XXH_YY)) {
				xxhp->yy = atof(value);
				xxhp->yy = atoi(value);
				xxhp->yy = atol(value);
				xxhp->yy = hstrdup(value, XXH_HNAME,
						   band);
 /* ### etc. %%% */
			}
			else if (streq(key, XXH_YY)) {
 /* ### etc. %%% */
			}
 /* ### etc. %%% */
		}

		if (err == ERROR) {
			return (NULL);
		}
	}

	if (hname == NULL) {
		return (NULL);
	}
 /*
  * verify the header
  */
	if (!xxhcheck(xxhpp, nbands)) {
		uferr(fd);
		return (NULL);
	}

	return (xxhpp);
}

#ifndef	lint
static char     rcsid[] = "$Header: xxhread.c,v 1.7 87/11/04 17:40:01 frew Exp $";

#endif
