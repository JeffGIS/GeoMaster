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

#include <stdio.h>

extern char    *optarg;

extern char    *malloc();

main(argc, argv)
	int             argc;
	char           *argv[];

{
	register char  *buf;
	register int    bufsiz;
	register int    nbytes;
	int             opt;

	while ((opt = getopt(argc, argv, "b:")) != EOF) {
		switch (opt) {

		case 'b':
			bufsiz = atoi(optarg);
			if (bufsiz < 1) {
				fprintf(stderr, "%s: bad bufsiz]\n", optarg);
				exit(1);
			}

			break;

		default:
			fprintf(stderr, "Usage: %s [-b bufsiz]\n");
			exit(1);
		}
	}

	buf = malloc((unsigned) bufsiz);
	if (buf == NULL) {
		perror("can't allocate I/O buffer");
		exit(1);
	}

	while ((nbytes = fread(buf, 1, bufsiz, stdin)) > 0) {
		(void) fwrite(buf, 1, nbytes, stdout);
	}

	exit(0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/TEST/Stdio/RCS/rw.c,v 1.2 90/11/11 17:18:20 frew Exp $";

#endif
