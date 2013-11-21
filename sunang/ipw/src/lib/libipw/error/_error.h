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

#ifndef	_ERROR_H
#define	_ERROR_H

#define	MSG_MAX		256		/* max #chars in usrerr message	 */

extern char   **_argv;
//extern int      _errno;
extern int      _fderr;
extern char     _usrerr[];

#if defined(va_dcl) || defined(_VA_LIST) || defined(_sys_varargs_h) || defined(_SYS_VARARGS_H) || defined(_VARARGS_INCLUDED) || defined(_STDARG_INCLUDED) || defined(_STDARG_H)
#ifndef _HIDDEN_VA_LIST
extern void     EXFUN( _error, (CONST char *severity, CONST char *fmt,
                                va_list ap));
#else /* AIX's HIDDEN VA_LIST */
extern void	EXFUN( _error, (CONST char *severity, CONST char *fmt,
				__va_list ap));
#endif
#endif

//#include "extern.h"

//extern void __NORETURNB EXFUN( __bug, (CONST char *format DOTS)) __NORETURNE;

/* $Header: /local/share/pkg/ipw/src/lib/libipw/error/RCS/_error.h,v 1.9 90/11/11 17:14:24 frew Exp $ */
#endif
