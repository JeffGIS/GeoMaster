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

#else

#include <stdio.h>

extern void     exit();

extern char    *optarg;
extern int      optind;

#endif

/*
** NAME
**	getopt -- command-line option cracker for shell scripts
**
** SYNOPSIS
**	set -- `getopt legal-options $*`
**
** DESCRIPTION
**	Getopt implements, for shell scripts, the C-language getopt()
**	command-line option syntax.  The arguments are an "optstring"-style
**	list of acceptable option letters (suffixed by ":" if they require an
**	option-argument), followed by the actual arguments to the shell
**	script.  Getopt echoes these arguments in a standard form (each option
**	letter with its own leading "-"; options and option-arguments
**	delimited by spaces; options separated from operands by a "--"
**	argument) on the standard output.  Illegal options or missing
**	option-arguments yield a message on the standard error output and a
**	nonzero exit status.
**
** DIAGNOSTICS
**	"getopt: illegal option -- x"
**		The option letter <x> was present on the command line but not
**		in the "legal-options" string.
**
**	"getopt: option requires an argument -- x"
**		The option letter <x> requires an argument, but none was
**		supplied.
**
** SEE ALSO
**	getopt(3)
**
** EXAMPLES
**		set -- `getopt ab: $*` || {
**			echo "Usage: $0 [-a] [-b foo] [file ...]" 1>&2
**			exit 1
**		}
**		...
**		a_flag=
**		b_arg=
**		while :; do
**			case $1 in
**			--)	shift
**				break
**				;;
**			-a)	a_flag=yes
**				;;
**			-b)	shift
**				b_arg=$1
**				;;
**			*)	# "can't happen ..."
**				echo "$0: getopt error" 1>&2
**				exit 1
**			esac
**			shift
**		done
**		...
**		for file do
**			...
**		done
**
** NOTICE
**	Dedicated to the public by J. Frew (frew@ucsb.edu) 24 Feb 1987.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

main(argc, argv)
	int             argc;
	char          **argv;
{
	int             i;		/* loop counter			 */
	int             opt;		/* current option letter	 */
	char           *optstring;	/* legal option letters		 */

	if (argc < 2) {
		(void) fprintf(stderr, "Usage: %s legal-args $*\n", argv[0]);
		exit(1);
	}

	optstring = argv[1];
 /*
  * remove optstring from argv before calling getopt()
  */
	for (i = 1; i < argc; ++i) {
		argv[i] = argv[i + 1];
	}

	--argc;

	while ((opt = getopt(argc, argv, optstring)) != EOF) {
		if (opt == '?') {
			exit(1);
		}

		(void) printf("-%c ", opt);

		if (optarg != NULL) {
			(void) printf("%s ", optarg);
		}
	}

	(void) printf("-- ");

	for (; optind < argc; ++optind) {
		(void) printf("%s ", argv[optind]);
	}

	(void) printf("\n");
	exit(0);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/getopt/RCS/main.c,v 1.2 90/11/11 17:11:22 frew Exp $";

#endif
