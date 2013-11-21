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
#include <windows.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "ipw.h"

#include "_error.h"

/*
** NAME
**	usrerr -- log user's error message string
**
** SYNOPSIS
**	void usrerr(const char *format, ...)
**
** DESCRIPTION
**	Usrerr saves the error message described by the printf-style "format"
**	and "args" in a place accessible to the _error() error handler.
**
** GLOBALS ACCESSED
**	_usrerr
**		pointer to user error message storage area
**
** APPLICATION USAGE
**	Usrerr should be called whenever it is desired to log the occurence of
**	an error, while postponing the decision as to how to deal with the
**	error.
*/

#ifdef __STDC__
 
void usrerr(const char *fmt, ...)
{
  va_list  ap;
  char	str[1024];
  
  va_start(ap, fmt);
  (void) sprintf(str, fmt, ap);
  MessageBox (0,str,0,MB_ICONEXCLAMATION);
  va_end(ap);
}
 
#else  /* non ANSI */
 
void
usrerr(va_alist)
va_dcl
{
  char     *fmt;
  va_list   ap;
  char	str[1024];
  
  va_start(ap, fmt);
  (void) sprintf(str, fmt, ap);
  MessageBox (0,str,0,MB_ICONEXCLAMATION);
   va_end(ap);
}

#endif
