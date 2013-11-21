/*
** NAME
**	head_final -- write output LQH
**
** DESCRIPTION
**	We had to wait until we processed the data before writing
**	output LQH.
**
** GLOBALS ACCESSED
**
** RETURN VALUE
**	OK for normal return, ERROR otherwise.
**
*/

#include <math.h>

#include "ipw.h"
#include "bih.h"
#include "hdrio.h"
#include "fpio.h"
#include "lqh.h"

extern LQH_T  **newlqh();

int
head_final(fdo, rval)
	int             fdo;		/* output file descriptor	 */
	fpixel_t       *rval;		/* output min/max values	 */
{
	LQH_T         **o_lqh;		/* output LQH			 */
	int             nbits;		/* # bits in output file	 */
	pixel_t         ival[2];	/* min,max pixel values		 */

 /*
  * output LQH, max and min from args
  */
 /* NOSTRICT */
	o_lqh = (LQH_T **) hdralloc(1, sizeof(LQH_T *), fdo, LQH_HNAME);
	if (o_lqh == NULL) {
		usrerr("Can't create new LQH");
		return (ERROR);
	}
	nbits = hnbits(fdo, 0);
	ival[0] = 0;
	ival[1] = ipow2(nbits) - 1;
	o_lqh[0] = lqhmake(nbits, 2, ival, rval, "W m^-2",
			   (char *) NULL);
	if (o_lqh[0] == NULL) {
		usrerr("Can't create new LQH");
		return (ERROR);
	}
 /*
  * write output LQH
  */
	if (lqhwrite(fdo, o_lqh) == ERROR) {
		usrerr("Can't write output LQH");
		return (ERROR);
	}
 /*
  * get ready for rest of output
  */
	if (boimage(fdo) == ERROR) {
		usrerr("Can't terminate header output");
		return (ERROR);
	}

	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/dozier/ipw/src/bin/toporad/RCS/head_final.c,v 1.1 89/07/05 13:25:23 dozier Exp $";

#endif
