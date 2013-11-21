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

/*
** NAME
**	testbih -- test BIH I/O functions
**
** SYNOPSIS
**	testbih [in]
**
** DESCRIPTION
**	testbih exercises the BIH I/O functions.  A BIH is read from either
**	the specified file or the standard input, and is written to the
**	standard output.
**
** OPTIONS
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** BUGS
*/

main(argc, argv)
	int             argc;
	char          **argv;
{
	extern void	testbih();

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&operands,
		0
	};

	int             fdi;		/* input image file descriptor	 */
	int             fdo;		/* output image file descriptor	 */

 /*
  * begin
  */
	ipwenter(&argc, &argv, optv, "test BIH I/O functions");
 /*
  * access input file(s)
  */
	if (argc == 0) {
		fdi = ustdin();
	}
	else if (argc == 1) {
		fdi = uropen(argv[0]);
		if (fdi == ERROR) {
			error("can't open \"%s\"", argv[0]);
		}
	}
	else {
		usage();
	}

	no_tty(fdi);
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * do it
  */
	testbih(fdi, fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/bih/TEST/RCS/main.c,v 1.2 90/11/11 17:13:21 frew Exp $";

#endif
