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

/* LINTLIBRARY */

#include "ipw.h"

#include "_fpio.h"

/*
** NAME
**	_fpiocb -- floating-point I/O control block array
**
** SYNOPSIS
**	#include "_fpio.h"
**
**	extern FPIO_T *_fpiocb[];
**
** DESCRIPTION
**	_fpiocb is an array of pointers to floating-point I/O control blocks,
**	indexed by the corresponding UNIX file descriptor.
**
** RESTRICTIONS
**	The number of pointers to floating-point I/O control blocks is set at
**	compile time to {OPEN_LIMIT}.  This imposes a hard limit on the number
**	of floating-point I/O streams a process may access simultaneously.
**
** WARNINGS
**
** APPLICATION USAGE
**	_fpiocb is not meant to be accessed by IPW applications programs.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

FPIO_T         *_fpiocb[OPEN_LIMIT] = {NULL};

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/fpio/RCS/_fpiocb.c,v 1.2 90/11/11 17:14:50 frew Exp $";

#endif
