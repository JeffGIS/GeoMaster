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
**  number of bits per byte
*/

typedef unsigned char integer_t;

int
char_bit()
{
    integer_t       j;	       /* contains bits to count  */
    static int      bits;      /* bit counter		  */
    static int      already = 0;	/* ? already called	 */

    if (!already) {
	bits = 0;
	j = 1;
		    /*
		     * Shift j left until bit is shifted off
		     * the end. (Left shift is guaranteed to be
		     * zero-fill.)
		     */
	while (j != 0) {
	    j <<= 1;
	    bits++;
	}
		    /*
		     * bits contains number of bits in j
		     */
    }
    return (bits);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/char_bit.c,v 1.3 90/11/19 14:49:36 frew Exp $";

#endif
