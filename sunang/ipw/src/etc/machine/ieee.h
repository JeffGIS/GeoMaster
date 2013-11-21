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
**  Floating-point representations that also meet the requirements
**  for single-precision and double-precision normalized numbers in
**  the
**
**  IEEE Standard for Binary Floating-Point Arithmetic (ANSI/IEEE
**  Std 754-1985).
**
**  (The floating-point numbers listed here were rounded toward 1.0)
*/

#define FLT_RADIX	2
#define FLT_MANT_DIG	24
#define FLT_EPSILON	1.193e-07
#define FLT_DIG		6
#define FLT_MIN_EXP	-126
#define FLT_MIN		1.176e-38
#define FLT_MIN_10_EXP	-37
#define FLT_MAX_EXP	128
#define FLT_MAX		3.402e+38
#define FLT_MAX_10_EXP	38

#define DBL_MANT_DIG	53
#define DBL_EPSILON	1.220447e-16
#define DBL_DIG		15
#define DBL_MIN_EXP	-1022
#define DBL_MIN		2.225074e-308
#define DBL_MIN_10_EXP	-307
#define DBL_MAX_EXP	1024
#define DBL_MAX		1.797693e+308
#define DBL_MAX_10_EXP	308

/* $Header: /local/share/pkg/ipw/src/etc/machine/RCS/ieee.h,v 1.3 90/11/19 14:49:45 frew Exp $ */
