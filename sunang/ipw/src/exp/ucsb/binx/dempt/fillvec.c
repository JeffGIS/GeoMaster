/*
** fill fpixel vector with constants
*/

#include "ipw.h"

void
fillvec(x, fill, n)
	REG_1 fpixel_t *x;		/* vector to fill	 */
	FREG_1 fpixel_t fill;		/* fill value		 */
	REG_2 int       n;		/* # elements		 */
{
	while (--n >= 0) {
		*x++ = fill;
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: fillvec.c,v 1.1 89/01/07 15:08:51 dozier Exp $";

#endif
