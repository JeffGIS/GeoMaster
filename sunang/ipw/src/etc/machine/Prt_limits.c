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
**  Output values for limits.h.  For more documentation of the
**  variables, see:
**	"Draft Proposed American National Standard for Information
**	 Systems -- Programming Language C," Doc. No. X3J11/88-001.
*/

#include "schar.h"

extern int      char_bit();
extern void     Cprint();
extern void     Iprint();
extern void     Lprint();
extern void     ULprint();
extern void     Uprint();
extern void     char_limits();
extern void     describe();
extern void     int_limits();
extern void     long_limits();
extern void     shrt_limits();

void
Prt_limits()
{
    char            char_max;
    char            char_min;
    int             int_max;
    int             int_min;
    long            long_max;
    long            long_min;
    short           shrt_max;
    short           shrt_min;
    unsigned        uint_max;
    unsigned char   uchar_max;
    unsigned long   ulong_max;
    unsigned short  ushrt_max;
    int             j;

#ifdef HAS_SCHAR
    signed char     schar_max;
    signed char     schar_min;

#endif

		/*
		 * compute the values
		 */
#ifdef HAS_SCHAR
    char_limits(&schar_min, &schar_max, &uchar_max);
    char_min = schar_min;
    char_max = schar_max;
#else
    char_limits(&char_min, &char_max, &uchar_max);
#endif
    shrt_limits(&shrt_min, &shrt_max, &ushrt_max);
    int_limits(&int_min, &int_max, &uint_max);
    long_limits(&long_min, &long_max, &ulong_max);
		/*
		 * now print
		 */
    describe("maximum number of bits for smallest object that is not a bit-field (byte)");
    Iprint("CHAR_BIT", char_bit());
    putchar('\n');
		/*
		 * signed characters, if supported
		 */
#ifdef HAS_SCHAR
    describe("minimum/maximum values for an object of type signed char");
    Iprint("SCHAR_MIN", (int) schar_min);
    Iprint("SCHAR_MAX", (int) schar_max);
    putchar('\n');
#endif
		/*
		 * unsigned characters
		 */
    describe("maximum value for an object of type unsigned char");
    Uprint("UCHAR_MAX", (unsigned) uchar_max);
    putchar('\n');
		/*
		 * characters: see whether characters sign
		 * extend
		 */
    describe("minimum/maximum values for an object of type char");
    j = char_min;
    if (j < 0) {
#ifdef HAS_SCHAR
	Cprint("CHAR_MIN", "SCHAR_MIN");
	Cprint("CHAR_MAX", "SCHAR_MAX");
#else
	Iprint("CHAR_MIN", (int) char_min);
	Iprint("CHAR_MAX", (int) char_max);
#endif
	putchar('\n');
    }
    else {
	Iprint("CHAR_MIN", (int) 0);
	Cprint("CHAR_MAX", "UCHAR_MAX");
	putchar('\n');
    }
		/*
		 * don't know how to determine this dynamically
		 */
    describe("maximum number of bytes in a multibyte character");
    Iprint("MB_LEN_MAX", (int) 1);
    putchar('\n');
		/*
		 * integers
		 */
    describe("minimum/maximum values for an object of type int");
    Iprint("INT_MIN", int_min);
    Iprint("INT_MAX", int_max);
    putchar('\n');
    describe("maximum value for an object of type unsigned int");
    Uprint("UINT_MAX", uint_max);
    putchar('\n');

		/*
		 * short integers
		 */
    describe("minimum/maximum values for an object of type short int");
    if (shrt_min == int_min)
	Cprint("SHRT_MIN", "INT_MIN");
    else
	Iprint("SHRT_MIN", (int) shrt_min);
    if (shrt_max == int_max)
	Cprint("SHRT_MAX", "INT_MAX");
    else
	Iprint("SHRT_MAX", (int) shrt_max);
    putchar('\n');
    describe("maximum value for an object of type unsigned short int");
    if (ushrt_max == uint_max)
	Cprint("USHRT_MAX", "UINT_MAX");
    else
	Uprint("USHRT_MAX", (unsigned) ushrt_max);
    putchar('\n');

		/*
		 * long integers
		 */
    describe("minimum/maximum values for an object of type long int");
    if (long_min == int_min)
	Cprint("LONG_MIN", "INT_MIN");
    else
	Lprint("LONG_MIN", long_min);
    if (long_max == int_max)
	Cprint("LONG_MAX", "INT_MAX");
    else
	Lprint("LONG_MAX", long_max);
    putchar('\n');
    describe("maximum value for an object of type unsigned long int");
    if (ulong_max == uint_max)
	Cprint("ULONG_MAX", "UINT_MAX");
    else
	ULprint("ULONG_MAX", ulong_max);
    putchar('\n');
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/Prt_limits.c,v 1.3 90/11/19 14:49:33 frew Exp $";

#endif
