/*
 *  Changed 3/13/92 -- Dana Jacobsen.  Use ANSI extern.h if we're ANSI
 */
 
#ifdef __STDC__
#include "ansi/extern.h"
#else

#ifndef	EXTERN_H
#define	EXTERN_H

/*
 * UNIX extern
 *
 * We assume that any UNIX C environment supplies the following header files,
 * and that they declare (or #define) the following symbols:
 *
 * <ctype.h>
 *	isalpha  islower  isxdigit ispunct  isprint  isascii  tolower
 *	isupper  isdigit  isspace  isalnum  iscntrl  toupper  toascii
 *
 * <math.h>
 *	acos  atan2 cos   fabs  gamma j1    log   sin   tan   y1
 *	asin  atof  cosh  floor hypot jn    log10 sinh  tanh  yn
 *	atan  ceil  exp   fmod  j0    ldexp pow   sqrt  y0
 *
 * (Note that hypot should be declared in each program that uses it, as some
 *  vendors leave it out, as it is not required by POSIX.)
 *
 * <stdio.h>
 *	BUFSIZ  NULL    ferror  fopen   getc    putchar stdout
 *	EOF     fdopen  fgets   freopen getchar stderr
 *	FILE    feof    fileno  ftell   putc    stdin
 *
 * <time.h>
 *	struct tm
 *
 * <varargs.h>
 *	va_arg   va_dcl   va_end   va_list  va_start
 *
 * <stdio.h> is automatically included by ipw.h.  <ctype.h>, <math.h>,
 * <time.h>, and <varargs.h> may be optionally included by specific IPW source
 * files.
 *
 * All other non-int-valued UNIX library functions used by IPW are declared
 * here.
 */

extern int      errno;
extern char    *ipwoptarg;
extern int      ipwopterr;
extern int      ipwoptind;
extern int      ipwoptopt;
extern char    *sys_errlist[];
extern int      sys_nerr;

extern double   atof();
extern long     atol();

extern addr_t   calloc();
extern addr_t   malloc();
extern void     exit();
extern xsize_t  fread();
extern void     free();
extern xsize_t  fwrite();
extern char    *getenv();
extern long     lseek();
extern addr_t   memcpy();
extern addr_t   memset();
extern void     perror();
extern void     qsort();
extern long	random();
extern addr_t   realloc();
extern void	srandom();
extern char    *strcat();
extern char    *strchr();
extern char    *strcpy();
extern xsize_t  strcspn();
extern xsize_t  strlen();
extern char    *strncat();
extern char    *strrchr();
extern xsize_t  strspn();
extern xtime_t  time();
extern char    *tempnam();

/*
 * IPW init
 */

extern void     ipwenter();
extern void     ipwexit();
extern void     opt_check();

/*
 * IPW error
 */

extern void     _bug();
extern void     syserr();
extern void     uferr();
extern void     usrerr();

/*
 * IPW uio
 */

extern bool_t   ubof();
extern long     ucopy();
extern bool_t   ueof();
extern char    *ufilename();
extern char    *ugets();
extern long     urskip();

/*
 * strvec package
 */

extern STRVEC_T *addsv();
extern STRVEC_T *delsv();
extern STRVEC_T *dupsv();
extern char    *walksv();
extern int	freesv();

/*
 * IPW misc
 */

extern addr_t   allocnd();
extern char    *dtoa();
extern addr_t   ecalloc();
extern addr_t   emalloc();
extern float    frand();
extern void     frinit();
extern char    *ftoa();
extern addr_t   hdralloc();
extern char    *hostorder();
extern char    *hstrdup();
extern char    *itoa();
extern char    *ltoa();
extern double   ltof();
extern unsigned ltou();
extern void     no_tty();
extern char    *rmlead();
extern void     rmtrail();
extern char    *strdup();
extern double   difftime();

/* other declarations from ANSI extern.h */

#ifndef __NORETURNB
#  define __NORETURNB
#  define __NORETURNE
#endif


#endif

#endif /* not-ANSI */
