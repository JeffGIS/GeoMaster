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
**  This C routine is intended to determine the parameters of the
**  floating-point arithmetic system specified below.  The
**  determination of the first three uses an extension of an
**  algorithm due to M. Malcolm, CACM 15 (1972), pp. 949-951,
**  incorporating some, but not all, of the improvements suggested
**  by M. Gentleman and S. Marovich, CACM 17 (1974), pp. 276-277.
**  This version is a C translation of the Fortran routine
**  documented in:
**
**  W. J. Cody, "MACHAR: A subroutine to dynamically determine
**       machine parameters," ACM Trans. Math. Software, vol. 14,
**       pp. 303-309, 1988.
**
**  Parameter values reported are as follows:
**
**      radix   - the radix for the floating-point representation
**      signif  - the number of base "radix" digits in the floating-
**                point significand
**      rounds  - 0 if floating-point addition chops
**                1 if floating-point addition rounds, but not in the
**                  ieee style
**                2 if floating-point addition rounds in the ieee style
**                3 if floating-point addition chops, and there is
**                  partial underflow
**                4 if floating-point addition rounds, but not in the
**                  ieee style, and there is partial underflow
**                5 if floating-point addition rounds in the ieee style,
**                  and there is partial underflow
**      nguard  - the number of guard digits for multiplication with
**                truncating arithmetic.  It is
**                0 if floating-point arithmetic rounds, or if it
**                  truncates and only "signif" base "radix" digits
**                  participate in the post-normalization shift of the
**                  floating-point significand in multiplication;
**                1 if floating-point arithmetic truncates and more
**                  than "signif" base "radix" digits participate in the
**                  post-normalization shift of the floating-point
**                  significand in multiplication.
**      macheps - the largest negative integer such that
**                1.0+radix^macheps != 1.0, except that macheps is
**                bounded below by -(signif+3)
**      negeps  - the largest negative integer such that
**                1.0-radix^negeps != 1.0, except that negeps is
**                bounded below by -(signif+3)
**      expbits - the number of bits (decimal places if radix = 10)
**                reserved for the representation of the exponent
**                (including the bias or sign) of a floating-point
**                number
**      minexp  - the largest in magnitude negative integer such that
**                radix^minexp is positive and normalized
**      maxexp  - the smallest positive power of "radix" that overflows
**      epsilon - the smallest positive floating-point number such
**                that 1.0+epsilon != 1.0. In particular, if either
**                radix == 2 or rounds == 0, epsilon = radix^macheps.
**                otherwise, epsilon = radix^macheps/2
**      epsneg  - a small positive floating-point number such that
**                1.0-epsneg != 1.0. in particular, if radix = 2
**                or rounds = 0, epsneg = radix^negeps.
**                otherwise, epsneg = radix^negeps/2.  Because
**                negeps is bounded below by -(signif+3), epsneg may not
**                be the smallest number that can alter 1.0 by
**                subtraction.
**      smallest- the smallest non-vanishing normalized floating-point
**                power of the radix, i.e., smallest = radix^minexp
**      biggest - the largest finite floating-point number.  In
**                particular biggest = (1.0-epsneg)*radix^maxexp
**                note - on some machines biggest will be only the
**                second, or perhaps third, largest number, being
**                too small by 1 or 2 units in the last digit of
**                the significand.
*/

#define abs(x) ((x)>0? (x) : -(x))

void
MACHAR(radix, signif, rounds, nguard, macheps, negeps, expbits, minexp,
       maxexp, epsilon, epsneg, smallest, biggest)
    int            *radix;
    int            *signif;
    int            *rounds;
    int            *nguard;
    int            *macheps;
    int            *negeps;
    int            *expbits;
    int            *minexp;
    int            *maxexp;
    real_t         *epsilon;
    real_t         *epsneg;
    real_t         *smallest;
    real_t         *biggest;
{
    int             i;
    int             itemp;
    int             iz;
    int             j;
    int             k;
    int             mx;
    int             nxres;
    real_t          a;
    real_t          b;
    real_t          beta;
    real_t          betah;
    real_t          betain;
    real_t          one;
    real_t          t;
    real_t          temp1;
    real_t          temp;
    real_t          tempa;
    real_t          tryz;
    real_t          two;
    real_t          y;
    real_t          z;
    real_t          zero;

    one = 1;
    two = one + one;
    zero = one - one;
		/*
		 * Determine radix ala Malcolm.
		 */
    a = one;
    do {
	a += a;
	temp = a + one;
	temp1 = temp - a;
	z = temp1 - one;
    } while (z == zero);
    b = one;
    do {
	b += b;
	temp = a + b;
	itemp = (int) (temp - a);
    } while (itemp == 0);
    *radix = itemp;
    beta = *radix;
		/*
		 * Determine signif & rounds.
		 */
    *signif = 0;
    b = one;
    do {
	++(*signif);
	b *= beta;
	temp = b + one;
	temp1 = temp - b;
	z = temp1 - one;
    } while (z == zero);
    *rounds = 0;
    betah = beta / two;
    temp = a + betah;
    z = temp - a;
    if (z != zero)
	*rounds = 1;
    tempa = a + beta;
    temp = tempa + betah;
    z = temp - tempa;
    if (*rounds == 0 && z != zero)
	*rounds = 2;
		/*
		 * Determine negeps, epsneg.
		 */
    *negeps = *signif + 3;
    betain = one / beta;
    a = one;
    for (i = 1; i <= *negeps; ++i)
	a *= betain;
    b = a;
    for (;;) {
	temp = one - a;
	z = temp - one;
	if (z != zero)
	    break;
	a *= beta;
	--(*negeps);
    }
    *negeps = -(*negeps);
    *epsneg = a;
		/*
		 * Determine macheps, epsilon.
		 */
    *macheps = -(*signif) - 3;
    a = b;
    for (;;) {
	temp = one + a;
	z = temp - one;
	if (z != zero)
	    break;
	a *= beta;
	++(*macheps);
    }
    *epsilon = a;
		/*
		 * Determine nguard.
		 */
    *nguard = 0;
    temp = one + *epsilon;
    z = temp * one;
    z -= one;
    if (*rounds == 0 && z != zero)
	*nguard = 1;
		/*
		 * Determine expbits, minexp, smallest.
		 * 
		 * Loop to determine largest i and k = 2^i such
		 * that (1/beta) ^ (2^i) does not underflow.
		 * 
		 * Exit from loop is signaled by an underflow.
		 */
    i = 0;
    k = 1;
    z = betain;
    t = one + *epsilon;
    nxres = 0;
    for (;;) {
	y = z;
	z = y * y;
		    /*
		     * Check for underflow here.
		     */
	a = z * one;
	temp = z * t;
	tryz = a + a;
	if (tryz == zero || abs(z) >= y)
	    break;
	temp1 = temp * betain;
	temp1 *= beta;
	if (temp1 == z)
	    break;
	++i;
	k += k;
    }
    if (*radix != 10) {
	*expbits = i + 1;
	mx = k + k;
    }
    else {
		    /*
		     * This segment is for decimal machines
		     * only.
		     */
	*expbits = 2;
	iz = *radix;
	while (k >= iz) {
	    iz *= *radix;
	    ++(*expbits);
	}
	mx = iz + iz - 1;
    }
    for (;;) {
		    /*
		     * Loop to determine minexp, smallest. Exit
		     * from loop is signaled by an underflow.
		     */
	*smallest = y;
	y *= betain;
		    /*
		     * Check for underflow here.
		     */
	a = y * one;
	temp = y * t;
	tryz = a + a;
	if (tryz == zero || abs(y) >= *smallest)
	    break;
	++k;
	temp1 = temp * betain;
	temp1 *= beta;
	if (temp1 != y || temp == y)
	    continue;
	else {
	    nxres = 3;
	    *smallest = y;
	    break;
	}
    }
    *minexp = -k;
		/*
		 * Determine maxexp, biggest.
		 */
    if (mx <= k + k - 3 && *radix != 10) {
	mx += mx;
	++(*expbits);
    }
    *maxexp = mx + *minexp;
		/*
		 * Adjust rounds to reflect partial underflow.
		 */
    *rounds += nxres;
		/*
		 * Adjust for ieee-style machines.
		 */
    if (*rounds >= 2)
	*maxexp -= 2;
		/*
		 * Adjust for machines with implicit leading
		 * bit in binary significand, and machines with
		 * radix point at extreme right of significand.
		 */
    i = *maxexp + *minexp;
    if (*radix == 2 && i == 0)
	--(*maxexp);
    if (i > 20)
	--(*maxexp);
    if (a != y)
	*maxexp -= 2;
    *biggest = one - *epsneg;
    if (*biggest * one != *biggest)
	*biggest = one - beta * *epsneg;
    *biggest /= beta * beta * beta * *smallest;
    i = *maxexp + *minexp + 3;
    for (j = 1; j <= i; ++j)
	if (*radix == 2)
	    *biggest += *biggest;
	else
	    *biggest *= beta;
}

/* $Header: /local/share/pkg/ipw/src/etc/machine/RCS/machar.cpp,v 1.3 90/11/19 14:49:57 frew Exp $ */
