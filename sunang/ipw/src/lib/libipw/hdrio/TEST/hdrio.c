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

#include "ipw.h"

#include "getargs.h"
#include "hdrio.h"

main(argc, argv)
	int             argc;
	char          **argv;
{
	char            comment[HREC_MAX + 1];
	int             fdi;
	char            key[HREC_MAX + 1];
	int             rec;
	char            value[HREC_MAX + 1];

	ipwenter(argc, argv, (OPTION_T **) NULL, "test hdrio");

	fdi = uropen("-");
	if (fdi == ERROR) {
		error("can't open standard input");
	}

	while ((rec = hgetrec(fdi, comment, key, value)) != ERROR) {
		if (rec == HGOT_PRMB) {
			int             band;
			char           *name;
			char           *vers;

			band = hrband(fdi);
			name = hrname(fdi);
			vers = hrvers(fdi);

			printf("PREAMBLE");

			if (name != NULL) {
				printf(", name=%s", name);
			}

			if (band != NO_BAND) {
				printf(", band=%d", band);
			}

			if (vers != NULL) {
				printf(", vers=%s", vers);
			}

			putchar('\n');

			if (streq(name, BOIMAGE)) {
				break;
			}
		}
		else {
			printf("DATA, key=%s, value=%s, comment=%s\n",
			       key, value, comment);
		}
	}

	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/hdrio/TEST/RCS/hdrio.c,v 1.2 90/11/11 17:15:34 frew Exp $";

#endif
