/*
** projection system
*/

#include "ipw.h"

static char    *xproj[4] = {
	"geographic",
	"UTM",
	"State Plane",
	"unknown"
};

char           *
proj(refsys)
	int             refsys;
{
	if (refsys < 0 || refsys > 2) {
		warn("projection system = %d, unknown", refsys);
		refsys = 3;
	}
	return (xproj[refsys]);
}

#ifndef	lint
static char     rcsid[] = "$Header: proj.c,v 1.1 89/01/07 15:08:58 dozier Exp $";

#endif
