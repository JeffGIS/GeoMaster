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
  * parse options
  */

#include "ipw.h"
#include "getargs.h"
#include "fpio.h"

void
options(opt_s, opt_a, opt_d, opt_i, s, a, nbits, spacing)
	OPTION_T       *opt_s;
	OPTION_T       *opt_a;
	OPTION_T       *opt_d;
	OPTION_T       *opt_i;
	bool_t         *s;		/* ? compute slopes	 */
	bool_t         *a;		/* ? compute aspects	 */
	int            *nbits;		/* # output bits	 */
	fpixel_t       *spacing;	/* grid spacing		 */
{
	*s = got_opt(*opt_s);
	*a = got_opt(*opt_a);

 /* default is to do both */
	if (!*s && !*a) {
		*s = *a = TRUE;
	}
 /*
  * # bits in output image
  */
	if (got_opt(*opt_i)) {
		nbits[0] = int_arg(*opt_i, 0);
		if (nbits[0] <= 0) {
			error("# bits must be >= 1");
		}
		if (n_args(*opt_i) > 1) {
			nbits[1] = int_arg(*opt_i, 1);
			if (nbits[1] <= 0) {
				error("# bits must be >= 1");
			}
		}
		else {
			nbits[1] = nbits[0];
		}
	}
	else {
		nbits[0] = nbits[1] = CHAR_BIT;
	}
 /*
  * grid spacing
  */
	if (got_opt(*opt_d)) {
		if (n_args(*opt_d) == 1) {
			spacing[0] = spacing[1] = real_arg(*opt_d, 0);
		}
		else {
			spacing[0] = real_arg(*opt_d, 0);
			spacing[1] = real_arg(*opt_d, 1);
		}
		if (spacing[0] <= 0 || spacing[1] <= 0)
			error("-d %g,%g : must be positive",
			      spacing[0], spacing[1]);
	}
	else {
		spacing[0] = spacing[1] = 0;
	}
}

#ifndef lint
static char	rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/gradient/RCS/options.c,v 1.6 90/11/11 17:03:34 frew Exp $";

#endif
