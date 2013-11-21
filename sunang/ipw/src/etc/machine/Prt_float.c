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
**  Output values for float.h.  For more documentation of the
**  variables, see:
**	"Draft Proposed American National Standard for Information
**	 Systems -- Programming Language C," Doc. No. X3J11/88-001.
*/

#include "ldbl.h"

#include <stdio.h>
extern double   powi();
extern int      dig_count();
extern int      ilog10();
extern void     Cprint();
extern void     Gprint();
extern void     Iprint();
extern void     describe();
extern void     dmachar();
extern void     fmachar();

#ifdef HAS_LDBL
extern void     ldmachar();
extern void     LGprint();
#endif

void
Prt_float()
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

#ifdef HAS_LDBL
    int             ldbl_dig;  /* no. decimal digits		 */
    int             ldmacheps; /* expon of dbl_epsilon		 */
    int             ldmax_e10; /* maximum base-10 exponent	 */
    int             ldmaxexp;  /* smallest overflow expon	 */
    int             ldmin_e10; /* minimum base-10 exponent	 */
    int             ldminexp;  /* largest non-zero base-10 exp	 */
    int             ldnegeps;  /* expon of dbl_epsneg		 */
    int             ldsignif;  /* no. mantissa digits		 */
    long double     ldbl_epsilon;	/* machine epsilon	 */
    long double     ldbl_epsneg;	/* machine neg epsilon 	 */
    long double     ldbl_max;  /* largest finite		 */
    long double     ldbl_min;  /* smallest non-zero		 */

#endif

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
#ifdef HAS_LDBL
    ldmachar(&mradix, &ldsignif, &rounds, &nguard, &ldmacheps,
	&ldnegeps, &iexp, &ldminexp, &ldmaxexp, &ldbl_epsilon,
	     &ldbl_epsneg, &ldbl_min, &ldbl_max);
#endif
		/*
		 * needed constants: no. of decimal digits.
		 */
    if (mradix != 10) {
	dbl_dig = dig_count(powi((double) mradix, dsignif - 1));
	dmax_e10 = ilog10(dbl_max);
	dmin_e10 = ilog10(dbl_min);
	flt_dig = dig_count(powi((double) mradix, fsignif - 1));
	fmax_e10 = ilog10((double) flt_max);
	fmin_e10 = ilog10((double) flt_min);
#ifdef HAS_LDBL
	ldbl_dig = ldig_count(lpowi((long double) mradix, ldsignif - 1));
	ldmax_e10 = ldlog10(ldbl_max);
	ldmin_e10 = ldlog10(ldbl_min);
#endif
    }
		/*
		 * now print out all the values
		 */
    switch (rounds) {
      case 0:
	describe("rounds toward zero (i.e. chops)");
	break;
      case 1:
	describe("rounds but not in IEEE style");
	break;
      case 2:
	describe("rounds in IEEE style");
	break;
      case 3:
	describe("chops, graceful underflow");
	break;
      case 4:
	describe("rounds, but not in IEEE style, graceful underflow");
	break;
      case 5:
	describe("rounds in IEEE style, graceful underflow");
	break;
      default:
	describe("rounding mode indeterminate");
    }
    Iprint("FLT_ROUNDS", rounds);
    describe("radix of floating-point representation");
    Iprint("FLT_RADIX", mradix);
    putchar('\n');
    describe("number of base-FLT_RADIX digits in floating-point mantissa");
    Iprint("FLT_MANT_DIG", fsignif);
    Iprint("DBL_MANT_DIG", dsignif);
#ifdef HAS_LDBL
    if (ldsignif == dsignif)
	Cprint("LDBL_MANT_DIG", "DBL_MANT_DIG");
    else
	Iprint("LDBL_MANT_DIG", ldsignif);
#endif
    putchar('\n');
    describe("number of decimal digits of precision");
    if (mradix != 10) {
	Iprint("FLT_DIG", flt_dig);
	Iprint("DBL_DIG", dbl_dig);
    }
    else {
	Cprint("FLT_DIG", "FLT_MANT_DIG");
	Cprint("DBL_DIG", "DBL_MANT_DIG");
    }
#ifdef HAS_LDBL
    if (ldbl_dig == dbl_dig)
	Cprint("LDBL_DIG", "DBL_DIG");
    else
	Iprint("LDBL_DIG", ldbl_dig);
#endif
    putchar('\n');
    describe("minimum negative integer such that FLT_RADIX raised to that power minus 1 is a normalized floating-point number");
    Iprint("FLT_MIN_EXP", fminexp);
    Iprint("DBL_MIN_EXP", dminexp);
#ifdef HAS_LDBL
    if (ldminexp == dminexp)
	Cprint("LDBL_MIN_EXP", "DBL_MIN_EXP");
    else
	Iprint("LDBL_MIN_EXP", ldminexp);
#endif
    putchar('\n');
    describe("minimum negative integer such that 10 raised to that power is in the range of normalized floating-point numbers");
    if (mradix != 10) {
	Iprint("FLT_MIN_10_EXP", fmin_e10);
	Iprint("DBL_MIN_10_EXP", dmin_e10);
#ifdef HAS_LDBL
	if (ldmin_e10 == dmin_e10)
	    Cprint("LDBL_MIN_10_EXP", "DBL_MIN_10_EXP");
	else
	    Iprint("LDBL_MIN_10_EXP", ldmin_e10);
#endif
    }
    else {
	Cprint("FLT_MIN_10_EXP", "FLT_MIN_EXP");
	Cprint("DBL_MIN_10_EXP", "DBL_MIN_EXP");
#ifdef HAS_LDBL
	Cprint("LDBL_MIN_10_EXP", "LDBL_MIN_EXP");
#endif
    }
    putchar('\n');
    describe("maximum integer such that FLT_RADIX raised to that power minus 1 is a representable finite floating-point number");
    Iprint("FLT_MAX_EXP", fmaxexp);
    Iprint("DBL_MAX_EXP", dmaxexp);
#ifdef HAS_LDBL
    if (ldmaxexp == dmaxexp)
	Cprint("LDBL_MAX_EXP", "DBL_MAX_EXP");
    else
	Iprint("LDBL_MAX_EXP", ldmaxexp);
#endif
    putchar('\n');
    describe("maximum integer such that 10 raised to that power is in the range of representable finite floating-point numbers");
    if (mradix != 10) {
	Iprint("FLT_MAX_10_EXP", fmax_e10);
	Iprint("DBL_MAX_10_EXP", dmax_e10);
#ifdef HAS_LDBL
	if (ldmax_e10 == dmax_e10)
	    Cprint("LDBL_MAX_10_EXP", "DBL_MAX_10_EXP");
	else
	    Iprint("LDBL_MAX_10_EXP", ldmax_e10);
#endif
    }
    else {
	Cprint("FLT_MAX_10_EXP", "FLT_MAX_EXP");
	Cprint("DBL_MAX_10_EXP", "DBL_MAX_EXP");
#ifdef HAS_LDBL
	Cprint("LDBL_MAX_10_EXP", "LDBL_MAX_EXP");
#endif
    }
    putchar('\n');
    describe("maximum representable finite floating-point number");
    Gprint("FLT_MAX", (double) flt_max, flt_dig - 1);
    Gprint("DBL_MAX", dbl_max, dbl_dig - 1);
#ifdef HAS_LDBL
    if (dbl_max == ldbl_max)
	Cprint("LDBL_MAX", "DBL_MAX");
    else
	LGprint("LDBL_MAX", ldbl_max, ldbl_dig - 1);
#endif
    putchar('\n');
    describe("minimum positive floating-point number x such that 1.0 + x != 1.0");
    Gprint("FLT_EPSILON", (double) flt_epsilon, flt_dig - 1);
    Gprint("DBL_EPSILON", dbl_epsilon, dbl_dig - 1);
#ifdef HAS_LDBL
    if (dbl_epsilon == ldbl_epsilon)
	Cprint("LDBL_EPSILON", "DBL_EPSILON");
    else
	LGprint("LDBL_EPSILON", ldbl_epsilon, ldbl_dig - 1);
#endif
    putchar('\n');
    describe("minimum normalized positive floating-point number");
    Gprint("FLT_MIN", (double) flt_min, flt_dig - 1);
    Gprint("DBL_MIN", dbl_min, dbl_dig - 1);
#ifdef HAS_LDBL
    if (dbl_min == ldbl_min)
	Cprint("LDBL_MIN", "DBL_MIN");
    else
	LGprint("LDBL_MIN", ldbl_min, ldbl_dig - 1);
#endif
    putchar('\n');
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/Prt_float.c,v 1.3 90/11/19 14:49:31 frew Exp $";

#endif
