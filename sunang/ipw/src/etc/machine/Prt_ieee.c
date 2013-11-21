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
**  If floating-point arithmetic meets IEEE standards, as outlined in
**
**	IEEE Standard for Binary Floating-Point Arithmetic (ANSI/IEEE
**	Std 754-1985).
**
**  then
**	CC_IEEE_754 is #define'd
**  otherwise
**	nothing is printed.
*/

extern int      is_ieee();
extern void	Cprint();

void
Prt_ieee()
{
    if (is_ieee()) {
	Cprint("CC_IEEE_754", "");
    }
}

#ifndef lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/etc/machine/RCS/Prt_ieee.c,v 1.3 90/11/19 14:49:32 frew Exp $";

#endif
