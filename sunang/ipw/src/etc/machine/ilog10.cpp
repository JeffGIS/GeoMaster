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
**  Determine integer exponent of argument to base 10.  For
**  arguments less than 1.0, rounding is upward.  For arguments
**  greater than 1.0, rounding is downward.
*/

int
LOG10(value)
    real_t          value;
{
    static int      already = 0;
    static real_t   inv_base;
    static real_t   base;

    int             j = 0;

    if (!already) {
	already = 1;
	base = 10;
	inv_base = 1;
	inv_base /= base;
    }
		/*
		 * check for zero or machine infinity
		 */
    if (value != 0 && value != value / 2) {

	if (value < 0) {
	    value = -value;
	}

	if (value > 1) {
	    while (value > 1) {
		++j;
		value *= inv_base;
	    }
	    --j;
	}

	else {
	    while (value < inv_base) {
		--j;
		value *= base;
	    }
	}
    }

    return (j);
}

/* $Header: /local/share/pkg/ipw/src/etc/machine/RCS/ilog10.cpp,v 1.3 90/11/19 14:49:47 frew Exp $ */
