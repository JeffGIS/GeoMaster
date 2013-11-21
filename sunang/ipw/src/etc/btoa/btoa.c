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
#include <stdlib.h> /* maybe sys/types.h? */
extern void     exit(int status);
extern size_t   fread(void *ptr, size_t size, size_t n, FILE *stream);
extern void     perror(const char *s);
extern int      getopt(int argc, char * const *argv, const char *optstring);
extern int      strcmp(const char *s1, const char *s2);
#else
extern void     exit();
#ifndef _IBMR2
extern unsigned fread();
#endif
extern void     perror();
#endif

/*
** NAME
**	btoa -- convert binary integers to ASCII
**
** SYNOPSIS
**	btoa [-1] [-2] [-4] [file ...]
**
** DESCRIPTION
**	btoa reads binary integers from {file} (default: standard input) and
**	writes either text equivalents to the standard output.  If more
**	than one {file} are specified, then they are read in sequence.
**
** OPTIONS
**	-1	Convert 1-byte (char) integers.
**
**	-2	Convert 2-byte (short) integers.
**
**	-4	Convert 4-byte (long) integers.
**
**	Exactly one of -1, -2, or -4 must be specified.
**
** EXAMPLES
**	To print the first 100 quantized pixel values from an IPW image
**	with 2-byte pixels:
**
**		rmhdr | btoa -2 | head -100
**
**	Using ipwfile, the number of bytes can be determined automatically:
**
**		rmhdr image | btoa -`ipwfile -my image` | head -100
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**	btoa is not an IPW program, but is provided with IPW for use in
**	shell scripts.
**
**	Input values are assumed to be unsigned.
**
** FUTURE DIRECTIONS
**	btoa should be renamed bin2text.
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  atob
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

void
main(argc, argv)
	int             argc;
	char           *argv[];

{
	void            i1toa();
	void            i2toa();
	void            i4toa();

	void            (*cvt) ();	/* -> atoi? function		 */
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

			cvt = i1toa;
			break;

		case '2':
			if (cvt != NULL) {
				usage(argv[0]);
			}

			cvt = i2toa;
			break;

		case '4':
			if (cvt != NULL) {
				usage(argv[0]);
			}

			cvt = i4toa;
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
		(*cvt) (stdin);
	}
	else {
		do {
			if (strcmp(argv[optind], "-") == 0) {
				(*cvt) (stdin);
			}
			else {
				FILE           *fpi;

				fpi = fopen(argv[optind], "r");
				if (fpi == NULL) {
					(void) fflush(stdout);
					perror(argv[optind]);
				}
				else {
					(*cvt) (fpi);
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

void
i1toa(fp)
	register FILE  *fp;		/* input file pointer		 */
{
	register int    i;		/* current input integer	 */

	while ((i = getc(fp)) != EOF) {
		(void) printf("%u\n", i);
	}
}

void
i2toa(fp)
	register FILE  *fp;		/* input file pointer		 */
{
	short           i;		/* current input integer	 */

 /* NOSTRICT */
	while (fread((char *) &i, sizeof(i), 1, fp) == 1) {
		(void) printf("%u\n", i & 0xFFFF);
	}
}

void
i4toa(fp)
	register FILE  *fp;		/* input file pointer		 */
{
	long            i;		/* current input integer	 */

 /* NOSTRICT */
	while (fread((char *) &i, sizeof(i), 1, fp) == 1) {
		(void) printf("%lu\n", i);
	}
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/btoa/RCS/btoa.c,v 1.5 90/11/11 17:11:20 frew Exp $";

#endif
