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

#ifdef IPW
#include "ipw.h"
#include "getargs.h"
#else
#include "_uio.h"

extern char    *optarg;

#endif

main(argc, argv)
	int             argc;
	char           *argv[];

{
	register char  *buf;
	register int    bufsiz;
	register int    fdi;
	register int    fdo;
	register int    nbytes;

#ifdef IPW
	static OPTION_T opt_b = {
		'b', "I/O buffer size",
		INT_OPTARGS, "bufsiz",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_i = {
		'i', "input file name",
		STR_OPTARGS, "infile",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_o = {
		'o', "output file name",
		STR_OPTARGS, "outfile",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_b,
		&opt_i,
		&opt_o,
		0
	};

	ipwenter(&argc, &argv, optv, "test UIO package");

	if (got_opt(opt_b)) {
		bufsiz = int_arg(opt_b, 0);
	}
	else {
		bufsiz = OPT_BUF;
	}

	if (got_opt(opt_i)) {
		fdi = uropen(str_arg(opt_i, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(opt_i, 0));
		}
	}
	else {
		fdi = ustdin();
	}

	if (got_opt(opt_o)) {
		fdo = uwopen(str_arg(opt_o, 0));
		if (fdo == ERROR) {
			error("can't open \"%s\"", str_arg(opt_o, 0));
		}
	}
	else {
		fdo = ustdout();
	}
#else
	int             opt;

	if (uioenter() == ERROR) {
		fprintf(stderr, "ERROR: uioenter\n");
	}

	bufsiz = OPT_BUF;
	fdi = FD_STDIN;
	fdo = FD_STDOUT;

	while ((opt = getopt(argc, argv, "b:i:o:")) != EOF) {
		switch (opt) {

		case 'b':
			bufsiz = atoi(optarg);
			if (bufsiz < 1) {
				fprintf(stderr, "%s: bad bufsiz\n", optarg);
				exit(1);
			}

			break;

		case 'i':
			fdi = uropen(optarg);
			if (fdi == ERROR) {
				fprintf(stderr, "can't uropen(%s)\n", optarg);
				exit(1);
			}

			break;

		case 'o':
			fdo = uwopen(optarg);
			if (fdo == ERROR) {
				fprintf(stderr, "can't uwopen(%s)\n", optarg);
				exit(1);
			}

			break;

		default:
			fprintf(stderr,
			  "Usage: %s [-b bufsiz] [-i infile] [-o outfile]\n",
				argv[0]);
			exit(1);
		}
	}
#endif
	buf = malloc((unsigned) bufsiz);
	if (buf == NULL) {
#ifdef IPW
		syserr();
		error("can't allocate I/O buffer");
#else
		perror("malloc(I/O buffer)");
		exit(1);
#endif
	}

	while ((nbytes = uread(fdi, buf, bufsiz)) > 0) {
		if (uwrite(fdo, buf, nbytes) == ERROR) {
#ifdef IPW
			error("write error");
#else
			fprintf(stderr, "ERROR: uwrite\n");
			exit(1);
#endif
		}
	}

	if (nbytes == ERROR) {
#ifdef IPW
		error("read error");
#else
		fprintf(stderr, "ERROR: uread\n");
		exit(1);
#endif
	}

#ifdef IPW
	ipwexit(EX_OK);
#else
	if (uioexit() == ERROR) {
		fprintf(stderr, "ERROR: uioexit\n");
	}

	exit(0);
#endif
}

#ifndef IPW
void
uferr(fd)
{
	fprintf(stderr, "\tFile: %s\n", ufilename(fd));
}

void
usrerr(s)
	char           *s;
{
	fprintf(stderr, "\tUIO error is: %s\n", s);
}

void
syserr()
{
	perror("\tUNIX error is");
}

#endif

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/uio/TEST/RCS/rw.c,v 1.2 90/11/11 17:18:25 frew Exp $";

#endif
