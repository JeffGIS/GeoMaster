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

/*
** NAME
**	machine -- machine-dependent constants
**
** SYNOPSIS
**	machine [-limits] [-float] [-ieee] [-H]
**
** DESCRIPTION
**	Machine prints out machine-dependent constants for integer and
**	floating-point arithmetic.  The variables are documented in
**	"Draft Proposed American National Standard for Information
**       Systems -- Programming Language C," Doc. No. X3J11/88-001.
**
**	The algorithm used for the floating-point quantities is:
**      W. J. Cody, 1988, "Algorithm 665, MACHAR: A subroutine to
**         dynamically determine machine parameters," ACM Transactions
**	   on Mathematical Software, v. 14, 303-309.
**
**	Values determined and printed are as follows:
**
**	If -limits option is specified:
**
**      CHAR_BIT        maximum number of bits for smallest object that
**				is not a bit field (byte)
**
**	(If signed characters are supported)
**	SCHAR_MIN	minimum value for an object of type "signed char"
**	SCHAR_MAX	maximum value for an object of type "signed char"
**
**	UCHAR_MAX	maximum value for an object of type "unsigned char"
**
**	CHAR_MIN	minimum value for an object of type "char"
**	CHAR_MAX	maximum value for an object of type "char"
**
**	SHRT_MIN	minimum value for an object of type "short int"
**	SHRT_MAX	maximum value for an object of type "short int"
**
**	USHRT_MAX	maximum value for an object of type
**				"unsigned short int"
**
**	INT_MIN		minimum value for an object of type "int"
**	INT_MAX		maximum value for an object of type "int"
**
**	UINT_MAX	maximum value for an object of type "unsigned int"
**
**	LONG_MIN	minimum value for an object of type "long int"
**	LONG_MAX	maximum value for an object of type "long int"
**
**	ULONG_MAX	maximum value for an object of type
**				"unsigned long int"
**
**	If -float option is specified (prefix FLT_ is for single-
**	precision, DBL_ for double precision):
**
**	FLT_RADIX	("b") radix of exponent representation
**	FLT_ROUNDS	values from 0 to 5 (see the Cody paper)
**
**	FLT_MANT_DIG	("p") number of base-b digits in the floating-point
**	DBL_MANT_DIG		mantissa
**	LDBL_MANT_DIG
**
**	FLT_DIG		number of decimal digits of precision:
**	DBL_DIG			if b == 10 then	p
**	LDBL_DIG		else (p - 1) * log10(b)
**
**	FLT_EPSILON	minimum positive floating-point number x such that
**	DBL_EPSILON		1.0 + x != 1.0
**	LDBL_EPSILON
**
**	FLT_MIN_EXP	("emin") minimum negative integer such that
**	DBL_MIN_EXP		b**(emin - 1) is a normalized floating-point
**	LDBL_MIN_EXP		number
**
**	FLT_MIN_10_EXP	minimum negative integer such that 10 raised to that
**	DBL_MIN_10_EXP		power is in the range of normalized
**	LDBL_MIN_10_EXP		floating-point numbers
**
**	FLT_MAX_EXP	("emax") maximum integer such that b**(emax - 1) is a
**	DBL_MAX_EXP		finite floating-point number
**	LDBL_MAX_EXP
**
**	FLT_MAX_10_EXP	maximum integer such that 10 raised to that power is
**	DBL_MAX_10_EXP		in the range of finite floating-point numbers
**	LDBL_MAX_10_EXP
**
**	FLT_MIN		minimum normalized positive floating-point number:
**	DBL_MIN			b**(emin - 1)
**
**	FLT_MAX		maximum finite floating-point number:
**	DBL_MAX			(1 - b**-p) * b**emax
**
** OPTIONS
**	limits	constants necessary for ANSI C <limits.h>
**	float	constants necessary for ANSI C <float.h>
**	ieee	#defines CC_IEEE_754 appropriately
**	H	produces a help message
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

#include <stdio.h>

extern int      exit();
extern void     usage();
extern void     Prt_float();
extern void     Prt_limits();
extern void     Prt_ieee();

main(argc, argv)
    int             argc;
    char          **argv;
{
    int             floating;
    int             limits;
    int             ieee_query;
    int             i;

		/*
		 * check arguments
		 */
    ieee_query = limits = floating = 0;
    for (i = 1; i < argc; i++) {

		    /*
		     * help message
		     */
	if (!strcmp("-H", argv[i])) {
	    usage(argv[0]);
	}

		    /*
		     * ANSI C <limits.h> values
		     */
	else if (!strcmp("-limits", argv[i])) {
	    limits = 1;
	}

		    /*
		     * ANSI C <float.h> values
		     */
	else if (!strcmp("-float", argv[i])) {
	    floating = 1;
	}

	else if (!strcmp("-ieee", argv[i])) {
	    ieee_query = 1;
	}

	else {
	    fprintf(stderr,
	     "Program %s: '%s' is an unrecognized argument\n",
		    argv[0], argv[i]);
	    exit(1);
	}
    }

		/*
		 * if no option specified, print usage message
		 */
    if (limits == 0 && floating == 0 && ieee_query == 0) {
	usage(argv[0]);
    }
		/*
		 * if more than one option specified, error
		 * exit
		 */
    if ((limits && floating) ||
	(limits && ieee_query) ||
	(floating && ieee_query)) {
	fprintf(stderr,
	  "Program %s: Cannot specify more than one option\n",
		argv[0]);
	exit(1);
    }

		/*
		 * print out values
		 */
    if (floating) {
	Prt_float();
    }
    else if (limits) {
	Prt_limits();
    }
    else if (ieee_query) {
	Prt_ieee();
    }

    (void) exit(0);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/Main.c,v 1.3 90/11/19 14:49:30 frew Exp $";

#endif
