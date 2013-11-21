#include "ipw.h"

#include "bih.h"
#include "gethdrs.h"
@#include ...@

#include "pgm.h"

/*
 * headers -- process image headers
 */

void
headers()
{
	BIH_T         **i_bihpp;	/* -> input BIH array		 */
	BIH_T         **o_bihpp;	/* -> output BIH array		 */

 /*
  * read BIH
  */
	i_bihpp = bihread(parm.i_fd);
	if (i_bihpp == NULL) {
		error("can't read basic image header");
	}
 /*
  * create and write BIH
  */
	o_bihpp = bihdup(i_bihpp);
	if (o_bihpp == NULL) {
		error("can't create basic image header");
	}

	if (bihwrite(parm.o_fd, o_bihpp) == ERROR) {
		error("can't write basic image header");
	}
 /*
  * @process remaining headers@
  */
 /*
  * done
  */
	if (boimage(parm.o_fd) == ERROR) {
		error("can't terminate header output");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: headers.c,v 1.1 88/09/08 20:22:44 frew Exp $";

#endif
