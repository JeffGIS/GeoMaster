#include "ipw.h"

#include "bih.h"
#include "gethdrs.h"
#include "lqh.h"

#include "pgm.h"

/*
 * headers -- process image headers
 */

void
headers()
{
 /*
  * need to read the LQ headers to initialize fpio
  */
 /* NOSTRICT */
	static GETHDR_T h_lqh = {LQH_HNAME, (ingest_t) lqhread};
	static GETHDR_T *request[] = {&h_lqh, NULL};

	BIH_T         **i_bihpp;	/* -> input BIH array		 */

 /*
  * read BIH
  */
	i_bihpp = bihread(parm.i_fd);
	if (i_bihpp == NULL) {
		error("can't read basic image header");
	}

	parm.i_nlines = bih_nlines(i_bihpp[0]);
	parm.i_nsamps = bih_nsamps(i_bihpp[0]);
	parm.i_nbands = bih_nbands(i_bihpp[0]);
 /*
  * process remaining headers
  */
	gethdrs(parm.i_fd, request, NO_COPY, ERROR);
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/bin/mstats/RCS/headers.c,v 1.2 89/11/17 16:47:36 frew Exp $";

#endif
