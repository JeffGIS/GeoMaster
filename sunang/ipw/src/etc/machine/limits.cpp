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
**  calcuates number of bits in a character, minimum signed, maximum
**  signed, and maximum unsigned values of whatever integer_t and uint_t
**  are typedef'ed to.
*/

extern int      char_bit();

void
LIMITS(min_signed, max_signed, max_unsigned)
    integer_t      *min_signed;
    integer_t      *max_signed;
    uint_t         *max_unsigned;
{
    static int      already = 0;
    static integer_t jmax;     /* largest signed integer_t	 */
    static integer_t jmin;     /* smallest signed integer_t	 */
    static uint_t   kmax;      /* largest unsigned uint_t	 */
    int             bits;      /* bits in byte			 */
    integer_t       j;	       /* generic signed integer	 */
    integer_t       last;
    uint_t          k;	       /* generic unsigned integer	 */

    if (!already) {
	already = 1;
		    /*
		     * Number of bits per character
		     */
	bits = char_bit();
		    /*
		     * Maxiumum positive value of signed
		     * integer: turn on the left-most bit, then
		     * take complement.
		     */
	j = ~0 << (sizeof(integer_t) * bits - 1);
	jmax = ~j;
		    /*
		     * Minimum (maxiumum negative) value of
		     * signed integer: start with the negative
		     * of the maximum positive number and
		     * subtract until wrap-around occurs.
		     */
	j = -jmax;
	if (j >= 0) {
	    j = jmax - 100;
	    j = -j;
	}
	else {
	    while (j < 0) {
		last = j--;
	    }
	    jmin = last;
	}
		    /*
		     * Largest unsigned integer: beware that
		     * unsigned longs may not be supported.
		     */
	kmax = (sizeof(k) == sizeof(integer_t)) ?
	    (uint_t) ~ 0 : (unsigned) ~0;
    }
		/*
		 * copy answers into arguments
		 */
    *min_signed = jmin;
    *max_signed = jmax;
    *max_unsigned = kmax;
}

/* $Header: /local/share/pkg/ipw/src/etc/machine/RCS/limits.cpp,v 1.3 90/11/19 14:49:53 frew Exp $ */
