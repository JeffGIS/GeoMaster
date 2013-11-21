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

#ifdef	lint

#include "ipw.h"

/*
**
*/

#undef	mode_t
#define	mode_t		int

#undef	off_t
#define	off_t		long

#undef	_exit
void	_exit(status) int status; { return; }

#undef	sleep
unsigned	sleep(seconds) unsigned seconds; { return 0; }

#undef	isatty
int	isatty(fildes) int fildes; { return 0; }

#undef	open
int	open(path, oflag) char *path; int oflag; { return 0; }

#undef	creat
int	creat(path, mode) char *path; mode_t mode; { return 0; }

#undef	unlink
int	unlink(path) char *path; { return 0; }

#undef	close
int	close(fildes) int fildes; { return 0; }

#undef	read
int	read(fildes, buf, nbyte) int fildes; char *buf; unsigned nbyte;
	{ return 0; }

#undef	write
int	write(fildes, buf, nbyte) int fildes; char *buf; unsigned nbyte;
	{ return 0; }

#undef	lseek
off_t	lseek(fildes, offset, whence) int fildes; off_t offset; int whence;
	{ return 0; }

#undef	fileno
int	fileno(stream) FILE *stream; { return 0; }

#undef	fdopen
FILE	*fdopen(fildes, type) int fildes; char *type; { return 0; }

/* lint */
#endif

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/syslint/RCS/posix.c,v 1.2 90/11/11 17:20:51 frew Exp $";

#endif
