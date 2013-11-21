#include  <varargs.h>
#include  <stdio.h>

extern int      errno;
extern int      sys_nerr;
extern char    *sys_errlist[];

/*
 *	error( funcname, format, arg1, arg2, .... )
 *	char *funcname;
 *	char *format;
 */

/* VARARGS1 */
error(funcname, va_alist)
char *funcname;
va_dcl
{
    va_list         args;
    char           *fmt;

    va_start(args);

    (void) fprintf(stderr, "%s: ", funcname);
    fmt = va_arg(args, char *);
    (void) vfprintf(stderr, fmt, args);
    putc( '\n', stderr );
    va_end(args);
    exit(1);
}


/*
 *	syserror( funcname, thiscall )
 *	char *funcname;
 *	char *thiscall;
 *
 *	Report on a system error.  Funcname should identify the function and
 *	thiscall should identify the failing system call within the function.
 */
syserror(funcname, thiscall)
char           *funcname;
char           *thiscall;
{
    (void) fprintf(stderr, "SYSTEM ERROR in %s: %s (%d",
		   funcname, thiscall, errno);
    if (errno > 0 && errno < sys_nerr)
	(void) fprintf(stderr, "; %s)\n", sys_errlist[errno]);
    else
	(void) fprintf(stderr, ")\n");
    exit(1);
}

#ifndef	lint
static char     rcsid[] = "$Header: /home/ipw/src/etc/rastool/RCS/error.c,v 1.1 90/01/31 13:31:40 frew Exp $";

#endif
