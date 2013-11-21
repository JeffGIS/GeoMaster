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
** Returns 1 if floating-point arithmetic meets IEEE standard, 0 otherwise
*/

#include "ieee.h"

extern void     fmachar();
extern void     dmachar();
extern int      ilog10();
extern int      dig_count();
extern double   powi();

int
is_ieee()
{
    double          dbl_epsilon;	/* machine epsilon	 */
    double          dbl_epsneg;/* machine neg epsilon	 	 */
    double          dbl_max;   /* largest finite		 */
    double          dbl_min;   /* smallest non-zero		 */
    float           flt_epsilon;	/* machine epsilon	 */
    float           flt_epsneg;/* machine neg epsilon	 	 */
    float           flt_max;   /* largest finite		 */
    float           flt_min;   /* smallest non-zero		 */
    int             dbl_dig;   /* no. decimal digits		 */
    int             dmacheps;  /* expon of dbl_epsilon		 */
    int             dmax_e10;  /* maximum base-10 exponent	 */
    int             dmaxexp;   /* smallest overflow expon	 */
    int             dmin_e10;  /* minimum base-10 exponent	 */
    int             dminexp;   /* largest non-zero base-10 exp	 */
    int             dnegeps;   /* expon of dbl_epsneg		 */
    int             dsignif;   /* no. mantissa digits		 */
    int             flt_dig;   /* no. decimal digits		 */
    int             fmacheps;  /* expon of flt_epsilon		 */
    int             fmax_e10;  /* maximum base-10 exponent	 */
    int             fmaxexp;   /* smallest overflow expon	 */
    int             fmin_e10;  /* minimum base-10 exponent	 */
    int             fminexp;   /* largest non-zero base-10 exp	 */
    int             fnegeps;   /* expon of dbl_epsneg		 */
    int             fsignif;   /* no. mantissa digits		 */
    int             iexp;      /* no. digits in exponent	 */
    int             mradix;    /* radix			 	 */
    int             nguard;    /* no. guard digits		 */
    int             rounds;    /* rounding code		 	 */

		/*
		 * call the double and single-precision version
		 * of Cody's MACHAR routine
		 */
    dmachar(&mradix, &dsignif, &rounds, &nguard, &dmacheps,
	    &dnegeps, &iexp, &dminexp, &dmaxexp, &dbl_epsilon,
	    &dbl_epsneg, &dbl_min, &dbl_max);
    fmachar(&mradix, &fsignif, &rounds, &nguard, &fmacheps,
	    &fnegeps, &iexp, &fminexp, &fmaxexp, &flt_epsilon,
	    &flt_epsneg, &flt_min, &flt_max);
		/*
		 * needed constants: no. of decimal digits.
		 */
    if (mradix != 10) {
	flt_dig = dig_count(powi((double) mradix, fsignif - 1));
	dbl_dig = dig_count(powi((double) mradix, dsignif - 1));
	fmin_e10 = ilog10((double) flt_min);
	dmin_e10 = ilog10(dbl_min);
	fmax_e10 = ilog10((double) flt_max);
	dmax_e10 = ilog10(dbl_max);
    }
		/*
		 * now compare against the values that also
		 * meet the IEEE standard
		 */
    if (mradix != FLT_RADIX) {
	return (0);
    }
		/*
		 * single-precision values; we compare only against the
		 exponents as this should be enough and prevents
		 over/under-flow
		 */
    if (fsignif < FLT_MANT_DIG) {
	return (0);
    }
    if (flt_dig < FLT_DIG) {
	return (0);
    }
    if (fminexp > FLT_MIN_EXP) {
	return (0);
    }
    if (fmin_e10 > FLT_MIN_10_EXP) {
	return (0);
    }
    if (fmaxexp < FLT_MAX_EXP) {
	return (0);
    }
    if (fmax_e10 < FLT_MAX_10_EXP) {
	return (0);
    }
		/*
		 * double-precision values
		 */
    if (dsignif < DBL_MANT_DIG) {
	return (0);
    }
    if (dbl_dig < DBL_DIG) {
	return (0);
    }
    if (dminexp > DBL_MIN_EXP) {
	return (0);
    }
    if (dmin_e10 > DBL_MIN_10_EXP) {
	return (0);
    }
    if (dmaxexp < DBL_MAX_EXP) {
	return (0);
    }
    if (dmax_e10 < DBL_MAX_10_EXP) {
	return (0);
    }
		/*
		 * passed all the tests
		 */
    return (1);
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/is_ieee.c,v 1.3 90/11/19 14:49:49 frew Exp $";

#endif
