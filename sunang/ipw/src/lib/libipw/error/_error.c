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

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "ipw.h"

#include "_error.h"

/*
** NAME
**	_error -- print standard error message
**
** SYNOPSIS
**	#include <varargs.h>
**
**	void _error(const char *severity, const char *fmt, ...)
**
** DESCRIPTION
**	_error is a low-level error handler.  It is almost always called by
**	the functions error(), warn(), and _bug().  _error prints the message
**	described by the printf-style "fmt" and "args" on the standard error
**	output.
**
**	If syserr() has been called, then _error also prints a UNIX system
**	error message.  If usrerr() has been called, then _error also prints a
**	saved user error message.  If uferr() has been called, then the
**	corresponding saved UNIX filename is printed.
**
**	If called by error() (i.e., if "severity" is anything other than
**	"WARNING"), _error then exits.
**
** GLOBALS ACCESSED
**	_argv	pointer to program argument vector (pointer to program name is
**		fetched from _argv[0])
**
**	_errno	most recent UNIX "errno" value, set by syserr()
**
**	_fderr	file descriptor associated with error, set by uferr()
**
**	_usrerr
**		local error message string, set by usrerr()
**
** APPLICATION USAGE
**	_error is not meant to be called by applications programs.
*/


void
DEFUN( _error, (severity, fmt, ap),
        CONST char     *severity       /* error severity label          */
   AND  CONST char     *fmt            /* user msg printf format        */
   AND  va_list         ap)            /* -> user msg args              */
{
  char      *pgm;
  char      *iseverity;

 /*
  * We can't use assert() here, since assert() MAY call _error()! Therefore,
  * we supply default values for bogus args.
  */
	if (_argv == NULL || _argv[0] == NULL) {
		pgm = "(program?)";
	}
	else {
		pgm = _argv[0];
	}

	if (severity == NULL) {
		iseverity = strdup("BUG");
	} else {
		iseverity = strdup(severity);
	}
 /*
  * in case stdout is same file as stderr
  */
	(void) fflush(stdout);
/*
 * Output format is:
 *
 * <pgm>: <severity>:
 *	<message>
 *	(File: <filename>)
 *	(IPW error is: <usrerr>)
 *	(UNIX error is: <syserr>)
 *	(command line: <cmd arg ...>)	(only if severity == "BUG")
 */
	(void) fprintf(stderr, "\n%s: %s:\n", pgm, iseverity);

	if (fmt != NULL) {
		(void) fprintf(stderr, "\t");

		(void) vfprintf(stderr, fmt, ap);
		(void) fprintf(stderr, "\n");
		va_end(ap);
	}

	if (OK_FD(_fderr)) {
		char           *p;	/* scratch char pointer		 */

		p = ufilename(_fderr);
		if (p != NULL) {
			(void) fprintf(stderr, "\t(File: %s)\n", p);
		}
	}

	if (_usrerr[0] != EOS) {
		(void) fprintf(stderr, "\t(IPW error is: %s)\n", _usrerr);
	}

	if (_errno != 0) {
		(void) fprintf(stderr, "\t(UNIX error is: ");

		if (_errno > 0 && _errno < sys_nerr) {
			(void) fprintf(stderr, "%s", sys_errlist[_errno]);
		}
		else {
			(void) fprintf(stderr, "Error %d", _errno);
		}

		(void) fprintf(stderr, ")\n");
	}

	if (streq(iseverity, "BUG")) {
		int             i;	/* loop counter			 */

		(void) fprintf(stderr, "\t(command line:");

		for (i = 0; _argv[i] != NULL; ++i) {
			(void) fprintf(stderr, " %s", _argv[i]);
		}

		(void) fprintf(stderr, ")\n");
	}

	(void) fprintf(stderr, "\n");

	if (!streq(iseverity, "WARNING")) {
		ipwexit(EX_ERROR);
	}
	SAFE_FREE(iseverity);

	_fderr = ERROR;
	_usrerr[0] = EOS;
	_errno = 0;
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/error/RCS/_error.c,v 1.10 90/11/11 17:14:21 frew Exp $";

#endif
