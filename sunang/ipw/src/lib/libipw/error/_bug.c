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

#include "_error.h"

/*
** NAME
**	_bug -- print bug message
**
** SYNOPSIS
**	void _bug(const char *msg, const char *file, int line)
**
** DESCRIPTION
**	_bug prints the bug message "msg", along with filename "file" and line
**	number "line", on the standard error output.  Program execution is
**	then terminated.
**
**	_bug is called by the "assert" and "bug" macros, which obtain the file
**	name and line number from the predefined macros __FILE__ and __LINE__,
**	respectively.
**
** WARNINGS
**	never returns
**
** APPLICATION USAGE
**	_bug is not meant to be called by applications programs.
*/

void
DEFUN( _bug, (msg, file, line),
	CONST char     *msg
   AND  CONST char     *file
   AND  int             line)
{
	__bug("%s\n\t(file \"%s\", line %d)", msg, file, line);
}
