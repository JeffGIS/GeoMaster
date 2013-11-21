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
**	random -- generate random values
**
** SYNOPSIS
**	random -n nlines -r min,max[,...] [-p precision] [-s seed]
**
** DESCRIPTION
**	Random prints columns of random values in the range specified as
**	ASCII text to the standard output.  One column is printed for each
**	min,max pair specified.
**
** OPTIONS
**	-n	Print {nlines} lines of output.
**
**	-r	Output values will range between {min} and {max} inclusive.
**		Each {min},{max} pair will control an output column.
**
**	Both -n and -r must be specified.
**
**	-p	Print output values using {precision} digits of precision
**		(default: 0, meaning round to nearest integer).
**
**	-s	Initialize the random number generator with {seed}
**		(default: obtain seed from system clock).  This option can
**		be used to obtain the same output from multiple invocations
**		of random.
**
** EXAMPLES
**	The command:
**
**		random -n 5 -r 0,10,0,10
**
**	yields something like:
**
**		7 1
**		5 8
**		7 2
**		1 8
**		0 2
**
**	To obtain the values of 100 randomly selected pixels from a
**	512 line by 512 sample "image":
**
**		random -n 100 -r 0,511,0,511 | sort -n | primg -i image
**
**	Note that the random coordinates must be sorted before being
**	passed to primg.
**
** FILES
**
** DIAGNOSTICS
**	# of values must be > 0
**
**		{nlines} must be a positive, non-zero integer.
**
**	range(s) must be specified by min,max pairs
**
**		-r must have an even number of arguments
**
**	output precision must be >= 0
**
**		{precision} must be a positive integer.
**
** RESTRICTIONS
**	random uses the random(3) functions from 4.3 BSD UNIX.  The source
**	for these functions is normally included with IPW.
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**
** BUGS
**
** SEE ALSO
**	IPW:  primg
**	UNIX: random, sort
*/

static int      prec = 0;		/* output precision		 */

static int
dprint(x)
	double          x;
{
	return (printf("%d ", (int) (x < 0 ? x - 0.5 : x + 0.5)));
}

static int
gprint(x)
	double          x;
{
	return (printf("%.*g ", prec, x));
}

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_r = {
		'r', "range of values: one min,max pair per column",
		REAL_OPTARGS, "m",
		REQUIRED, 2
	};

	static OPTION_T opt_n = {
		'n', "# of rows to generate",
		INT_OPTARGS, "n",
		REQUIRED, 1, 1
	};

	static OPTION_T opt_p = {
		'p', "output precision, 0 == integer (default)",
		INT_OPTARGS, "precision",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_s = {
		's',
		"random generator seed, default is to get seed from clock",
		INT_OPTARGS, "seed",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_n,
		&opt_r,
		&opt_p,
		&opt_s,
		0
	};

	REG_4 int       col;		/* current column		 */
	REG_1 double   *min;		/* -> min value [column]	 */
	REG_5 int       ncols2;		/* 2 * # columns to print	 */
	REG_6 int       nrows;		/* # rows to print		 */
	REG_2 double   *range;		/* -> range of values [column]	 */
	REG_3 int       (*rprint) ();	/* -> output function		 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "print random values");

	nrows = int_arg(opt_n, 0);
	if (nrows < 1) {
		error("# of values must be > 0");
	}

	ncols2 = n_args(opt_r);
	if (ncols2 & 1) {
		error("range(s) must be specified by min,max pairs");
	}

	if (got_opt(opt_p)) {
		prec = int_arg(opt_p, 0);
		if (prec < 0) {
			error("output precision must be >= 0");
		}
	}

	rprint = (prec > 0) ? gprint : dprint;
 /*
  * The output columns are numbered 0, 2, 4, ...  This allows us to set
  * 2 pointers into the min,max array, then use the column numbers as
  * indices on these pointers.
  */
	min = real_argp(opt_r);
	range = min + 1;
 /*
  * convert max to range
  */
	for (col = 0; col < ncols2; col += 2) {
		range[col] -= min[col];
	}
 /*
  * generate random numbers
  */
 /* seed from command line	 */
	if (got_opt(opt_s)) {
		srandom((unsigned) int_arg(opt_s, 0));
	}
 /* generate seed from time of day	 */
	else {
		frinit();
	}

	do {
		for (col = 0; col < ncols2; col += 2) {
			(void) (*rprint) (frand() * range[col] + min[col]);
		}

		putchar('\n');
	} while (--nrows > 0);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/random/RCS/main.c,v 1.10 90/11/11 17:08:24 frew Exp $";

#endif
