/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the Computer Systems Laboratory, University of
 * California, Santa Barbara and its contributors'' in the documentation
 * or other materials provided with the distribution and in all
 * advertising materials mentioning features or use of this software.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* LINTLIBRARY */

#include "ipw.h"

#include "hdrio.h"
#include "bih.h"
#include "sath.h"

/*
** NAME
**	sathwrite -- write an IPW SATH header
**
** SYNOPSIS
**	#include "file.h"
**
**	int sathwrite(fd, sathpp)
**	int fd;
**	SATH_T **sathpp;
**
** DESCRIPTION
**      sathwrite writes the array of SATH headers pointed to by sathpp
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
**	sathwrite is called by IPW application programs to write
**	SATH headers.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
sathwrite(fd, sathpp)
	int             fd;		/* output file descriptor	 */
	SATH_T        **sathpp;		/* -> array of SATH pointers	 */
{
	REG_2 int       band;		/* current header band #	 */
	int             nbands;		/* # bands in output image	 */

	nbands = hnbands(fd);

	assert(sathcheck(sathpp, nbands));
 /*
  * loop through possible output bands
  */
	for (band = 0; band < nbands; ++band) {
		REG_1 SATH_T   *sathp;	/* -> current SATH		 */

		sathp = sathpp[band];
		if (sathp == NULL) {
			continue;
		}
 /*
  * write preamble
  */
		if (hwprmb(fd, SATH_HNAME, band, SATH_VERSION) == ERROR) {
			return (ERROR);
		}
 /*
  * write fields
  */
		if (sathp->platform != NULL) {
			if (hputrec(fd, (char *) NULL, SATH_PLATFORM,
				    sathp->platform) == ERROR) {
				return (ERROR);
			}
		}
		if (sathp->sensor != NULL) {
			if (hputrec(fd, (char *) NULL, SATH_SENSOR,
				    sathp->sensor) == ERROR) {
				return (ERROR);
			}
		}
		if (sathp->location != NULL) {
			if (hputrec(fd, (char *) NULL, SATH_LOCATION,
				    sathp->location) == ERROR) {
				return (ERROR);
			}
		}
		if (sathp->gmdate != NULL) {
			if (hputrec(fd, (char *) NULL, SATH_GMDATE,
				    sathp->gmdate) == ERROR) {
				return (ERROR);
			}
		}
		if (sathp->gmtime != NULL) {
			if (hputrec(fd, (char *) NULL, SATH_GMTIME,
				    sathp->gmtime) == ERROR) {
				return (ERROR);
			}
		}
	}

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/sath/RCS/sathwrite.c,v 1.4 90/11/11 17:17:38 frew Exp $";

#endif
