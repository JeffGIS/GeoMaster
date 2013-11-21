/*
** units for elevations and datum
*/

#include "ipw.h"

static char    *xun[4] = {
	"unknown",
	"feet",
	"meters",
	"seconds"
};

char           *
units(refunits)
	int             refunits;
{
	if (refunits < 1 || refunits > 3) {
		warn("units code = %d, unknown", refunits);
		refunits = 0;
	}
	return (xun[refunits]);
}

#ifndef	lint
static char     rcsid[] = "$Header: units.c,v 1.1 89/01/07 15:09:00 dozier Exp $";

#endif
