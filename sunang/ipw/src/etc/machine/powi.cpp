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
**	Compute x raised to the power n.
**
**      Uses doubling method, adapted from R. G. Dromey, How to Solve
**	It By Computer, Prentice-Hall, 1982, p. 130.
**
**	no check for overflow
*/

real_t
POWERI(x, n)
    real_t          x;
    register        n;
{
    real_t          p = 1;

    if (x == 0) {
	p = 0;
    }

    else {
		    /*
		     * if n negative, reverse problem
		     */
	if (n < 0) {
	    n = -n;
	    x = 1 / x;
	}

		    /*
		     * loop
		     */
	for (;;) {
	    if (n & 01) {
		p *= x;
	    }
			/*
			 * Check for loop termination; putting
			 * check here protects against
			 * unnecessary overflow.
			 */
	    if ((n >>= 1) == 0) {
		break;
	    }
	    x *= x;
	}
    }

    return (p);
}

/* $Header: /local/share/pkg/ipw/src/etc/machine/RCS/powi.cpp,v 1.3 90/11/19 14:49:59 frew Exp $ */
