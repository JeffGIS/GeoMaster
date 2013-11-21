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
**	__bug -- print bug message
**
** SYNOPSIS
**	void __bug(const char *format, ...)
**
** DESCRIPTION
**
** APPLICATION USAGE
**	__bug should never be called by application programs.
*/

#ifdef __STDC__

void __bug(const char *fmt, ...)
{
  va_list  ap;

  va_start(ap, fmt);
  _error("BUG", fmt, ap);
  /* NOTREACHED */
  va_end(ap);
  ipwexit(EX_ERROR);
}

#else  /* non ANSI */

void __bug(va_alist)
va_dcl
{
  char     *fmt;
  va_list   ap;

  va_start(ap);

  fmt = va_arg(ap, char *);
  _error("BUG", fmt, ap);
  /* NOTREACHED */
  va_end(ap);
  ipwexit(EX_ERROR);
}

#endif
