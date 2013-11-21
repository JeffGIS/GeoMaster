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
** number of decimal digits to left of decimal point
*/

int
COUNT(number)
    real_t          number;    /* abs value must be > 1 */
{
    int             digits;
    real_t          invbase;

    digits = 0;
    if (number < 0) {
	number = -number;
    }
    if (number > (double) 1) {
	invbase = 1;
	invbase /= 10;
		    /*
		     * keep dividing by 10 until number is less
		     * than 1.0
		     */
	do {
	    ++digits;
	    number *= invbase;
	} while (number > (double) 1);
    }

    return (digits);
}

/* $Header: /local/share/pkg/ipw/src/etc/machine/RCS/dcount.cpp,v 1.3 90/11/19 14:49:39 frew Exp $ */
