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

#include "fpio.h"
#include "gethdrs.h"
#include "lqh.h"
/*
 * NB: add #include's for alternate headers here
 */

/*
** NAME
**	fphdrs -- read, then write any floating-point headers
**
** SYNOPSIS
**	#include "fpio.h"
**
**	void fphdrs(fdi, nbands, fdo)
**	int fdi, nbands, fdo;
**
** DESCRIPTION
**	fphdrs copies all (remaining) image headers from filw descriptor fdi
**	to file descriptor fdo.  Any floating-point headers encountered that
**	are known to fphdrs (currently: LQ) are also ingested, and thus become
**	accessible to the fpio initialization routine.
**
** RESTRICTIONS
**	Only the LQ header is recognized by this version of fphdrs.
**
**	fphdrs calls gethdrs; therefore, no further headers may be ingested
**	after fphdrs is called.  See gethdrs() for more info.
**
** GLOBALS ACCESSED
**
** ERRORS
**
** WARNINGS
**	fphdrs terminates program execution if any errors are encountered.
**
** APPLICATION USAGE
**	Call fphdrs if your program doesn't need to ingest any more headers,
**	but you want the floating-point headers read in order to initialize
**	the fpio package.
**
** FUTURE DIRECTIONS
**	Accept additional floating-point headers as they are developed.
**
** BUGS
*/

void
fphdrs(fdi, nbands, fdo)
	int             fdi;		/* input image file descriptor	 */
	int		nbands;		/* # image bands		*/
	int             fdo;		/* output image file descriptor	 */
{
 /*
  * NB: add GETHDR_T structs for alternate headers here
  */
 /* NOSTRICT */
	static GETHDR_T h_lq = {LQH_HNAME, (ingest_t) lqhread};

 /*
  * NB: insert addresses of additional GETHDR_T structs in list below
  */
 /* NOSTRICT */
	static GETHDR_T *hv[] = {&h_lq, 0};

	gethdrs(fdi, hv, nbands, fdo);
 /*
  * return if no output image
  */
	if (fdo == ERROR) {
		return;
	}
 /*
  * write floating-point headers
  */
	if (got_hdr(h_lq)) {
 /* NOSTRICT */
		if (lqhwrite(fdo, (LQH_T **) hdr_addr(h_lq)) == ERROR) {
			error("can't write LQ header");
		}
	}
 /*
  * NB: write alternate headers here
  */
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/fpio/RCS/fphdrs.c,v 1.5 90/11/11 17:15:03 frew Exp $";

#endif
