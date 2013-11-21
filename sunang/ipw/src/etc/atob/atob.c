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
extern int      optind;

#ifdef __STDC__
#include <stdlib.h>
extern void     exit(int status);
extern size_t   fwrite(const void *ptr, size_t size, size_t n, FILE *s);
extern void     perror(const char *s);
extern int      getopt(int argc, char * const *argv, const char *optstring);
extern int      strcmp(const char *s1, const char *s2);
#else
extern void     exit();
#ifndef _IBMR2
extern unsigned fwrite();
#endif
extern void     perror();
#endif

/*
** NAME
**	atob -- convert ASCII integers to binary
**
** SYNOPSIS
**	atob [-1] [-2] [-4] [file ...]
**
** DESCRIPTION
**	atob reads text integers from {file} (default: standard input) and
**	writes thier binary equivalents to the standard output.  If more
**	than one {file} are specified, then they are read in sequence.
**
** OPTIONS
**	-1	Convert input to 1-byte (char) integers.
**
**	-2	Convert input to 2-byte (short) integers.
**
**	-4	Convert input to 4-byte (long) integers.
**
**	Exactly one of -1, -2, or -4 must be specified.
**
** EXAMPLES
**	To convert 256 text integers to an IPW lookup table:
**
**		atob -1 | mkbih -l 1 -s 256 -y 1
**
** FILES
**
** DIAGNOSTICS
**	{file}: bad input
**
**		An input value could not be converted to an integer.
**
** RESTRICTIONS
**	atob is not an IPW program, but is provided with IPW for use in
**	shell scripts.
**
**	Negative input values, or input values too large to be represented
**	in the specified number of output bytes, will be converted to
**	binary in a machine-dependent fashion.
**
** FUTURE DIRECTIONS
**	atob should be renamed text2bin.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  btoa
*/

#define	OPTSTRING	"124"
#define	SYNOPSIS	"-{1,2,4} [file ...}"

static void
usage(pgm)
	char           *pgm;		/* program name			 */
{
	(void) fflush(stdout);
	(void) fprintf(stderr, "Usage: %s %s\n", pgm, SYNOPSIS);
	exit(1);
}

static void
error(fp, file)
	FILE           *fp;		/* input stdio stream		 */
	char           *file;		/* input file name		 */
{
	if (ferror(fp)) {
		perror(file);
	}
	else {
		(void) fprintf(stderr, "%s: bad input\n", file);
	}

	exit(1);
}

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             atoi1();
	int             atoi2();
	int             atoi4();

	int             (*cvt) ();	/* -> atoi? function		 */
	int             opt;		/* current option		 */

 /*
  * initialize
  */
	cvt = NULL;
 /*
  * options
  */
	while ((opt = getopt(argc, argv, OPTSTRING)) != EOF) {
		switch (opt) {

		case '1':
			if (cvt != NULL) {
				usage(argv[0]);
			}

			cvt = atoi1;
			break;

		case '2':
			if (cvt != NULL) {
				usage(argv[0]);
			}

			cvt = atoi2;
			break;

		case '4':
			if (cvt != NULL) {
				usage(argv[0]);
			}

			cvt = atoi4;
			break;

		default:
			usage(argv[0]);
		}
	}

	if (cvt == NULL) {
		usage(argv[0]);
	}
 /*
  * operands
  */
	if (optind >= argc) {
		if (!(*cvt) (stdin)) {
			error(stdin, "(stdin)");
		}
	}
	else {
		do {
			if (strcmp(argv[optind], "-") == 0) {
				if (!(*cvt) (stdin)) {
					error(stdin, "(stdin)");
				}
			}
			else {
				FILE           *fpi;

				fpi = fopen(argv[optind], "r");
				if (fpi == NULL) {
					(void) fflush(stdout);
					perror(argv[optind]);
				}
				else {
					if (!(*cvt) (fpi)) {
						error(fpi, argv[optind]);
					}

					(void) fclose(fpi);
				}
			}
		}
		while (++optind < argc);
	}
 /*
  * done
  */
	exit(0);
}

int
atoi1(fp)
	register FILE  *fp;		/* input file pointer		 */
{
	int             i;		/* current input integer	 */

	while (fscanf(fp, "%d", &i) == 1) {
		putchar(i & 0377);
	}

	return (feof(fp));
}

int
atoi2(fp)
	register FILE  *fp;		/* input file pointer		 */
{
	short           i;		/* current input integer	 */

	while (fscanf(fp, "%hd", &i) == 1) {
 /* NOSTRICT */
		(void) fwrite((char *) &i, sizeof(i), 1, stdout);
	}

	return (feof(fp));
}

int
atoi4(fp)
	register FILE  *fp;		/* input file pointer		 */
{
	long            i;		/* current input integer	 */

	while (fscanf(fp, "%ld", &i) == 1) {
 /* NOSTRICT */
		(void) fwrite((char *) &i, sizeof(i), 1, stdout);
	}

	return (feof(fp));
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/atob/RCS/atob.c,v 1.5 90/11/11 17:11:17 frew Exp $";

#endif
