#include "ipw.h"

#include "bih.h"
#include "pgm.h"

void
ipw2xim()
{
	ximhdr();

	if (ucopy(parm.i_fd, parm.o_fd, imgsize(parm.i_fd)) == ERROR) {
		error("can't copy image data");
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr.MC68020/home/ipw/adm/snoopy/src/bin/ipw2xim/RCS/ipw2xim.c,v 1.2 89/10/25 20:00:13 frew Exp $";

#endif
