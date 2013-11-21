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
 * generalize reverse function
 */

void
func(vec, n)
	register mtype	*vec;		/* -> beginning of vector	*/
	register int	n;		/* length of vector		*/
{
	register mtype	*end;		/* -> last element		*/
	register mtype	temp;

	end = &vec[n-1];
	while (end > vec) {
		temp = *end;
		*end-- = *vec;
		*vec++ = temp;
	}
}

/* $Header: /local/share/pkg/ipw/src/bin/flip/RCS/reverse.cpp,v 1.5 90/11/11 17:02:53 frew Exp $ */
