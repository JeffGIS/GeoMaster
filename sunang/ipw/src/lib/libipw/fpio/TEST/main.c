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
**	fpio -- test fpio package
**
** SYNOPSIS
**	fpio [image]
**
** DESCRIPTION
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

	void		fpio();

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "test fpio package");
 /*
  * access input file(s)
  */
	if (!got_opt(operands)) {
		fdi = ustdin();
	}
	else {
		fdi = uropen(str_arg(operands, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(operands, 0));
		}
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
	fpio(fdi, fdo);
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/fpio/TEST/RCS/main.c,v 1.2 90/11/11 17:14:46 frew Exp $";

#endif
