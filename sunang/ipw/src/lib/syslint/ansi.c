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

#include <time.h>
#include <varargs.h>

#include "ipw.h"

/*
**
*/

#undef	clock_t
#define	clock_t		long

#undef	fpos_t
#define	fpos_t		long

#undef	size_t
#define	size_t		xsize_t

#undef	time_t
#define	time_t		xtime_t

#undef	tm_t
#define	tm_t		struct tm

#undef	voidp_t
#define	voidp_t		addr_t

/*----------------------------------------------------------------------------
 * <assert.h> NOT USED
 */

/*----------------------------------------------------------------------------
 * <ctype.h>
 */

#undef	_ctype_
char    _ctype_[1];

#undef	isalnum
int	isalnum(c) int c; { return 0; }

#undef	isalpha
int	isalpha(c) int c; { return 0; }

#undef	iscntrl
int	iscntrl(c) int c; { return 0; }

#undef	isdigit
int	isdigit(c) int c; { return 0; }

#undef	isgraph
int	isgraph(c) int c; { return 0; }

#undef	islower
int	islower(c) int c; { return 0; }

#undef	isprint
int	isprint(c) int c; { return 0; }

#undef	ispunct
int	ispunct(c) int c; { return 0; }

#undef	isspace
int	isspace(c) int c; { return 0; }

#undef	isupper
int	isupper(c) int c; { return 0; }

#undef	isxdigit
int	isxdigit(c) int c; { return 0; }

#undef	tolower
int	tolower(c) int c; { return 0; }

#undef	toupper
int	toupper(c) int c; { return 0; }

/*----------------------------------------------------------------------------
 * <errno.h>
 */

#undef	errno
int	errno;

/*----------------------------------------------------------------------------
 * <float.h> NO DECLARATIONS
 */

/*----------------------------------------------------------------------------
 * <limits.h> NO DECLARATIONS
 */

/*----------------------------------------------------------------------------
 * <locale.h> NOT USED
 */

/*----------------------------------------------------------------------------
 * <math.h>
 */

#undef	acos
double	acos(x) double x; { return 0; }

#undef	asin
double	asin(x) double x; { return 0; }

#undef	atan
double	atan(x) double x; { return 0; }

#undef	atan2
double	atan2(y, x) double y; double x; { return 0; }

#undef	ceil
double	ceil(x) double x; { return 0; }

#undef	cos
double	cos(x) double x; { return 0; }

#undef	cosh
double	cosh(x) double x; { return 0; }

#undef	exp
double	exp(x) double x; { return 0; }

#undef	fabs
double	fabs(x) double x; { return 0; }

#undef	floor
double	floor(x) double x; { return 0; }

#undef	fmod
double	fmod(x, y) double x; double y; { return 0; }

#undef	frexp
double	frexp(value, eptr) double value; int *eptr; { return 0; }

#undef	ldexp
double	ldexp(x, iexp) double x; int iexp; { return 0; }

#undef	log
double	log(x) double x; { return 0; }

#undef	log10
double	log10(x) double x; { return 0; }

#undef	modf
double	modf(value, iptr) double value; double *iptr; { return 0; }

#undef	pow
double	pow(x, y) double x; double y; { return 0; }

#undef	sin
double	sin(x) double x; { return 0; }

#undef	sinh
double	sinh(x) double x; { return 0; }

#undef	sqrt
double	sqrt(x) double x; { return 0; }

#undef	tan
double	tan(x) double x; { return 0; }

#undef	tanh
double	tanh(x) double x; { return 0; }

/*----------------------------------------------------------------------------
 * <setjmp.h> NOT USED
 */

/*----------------------------------------------------------------------------
 * <signal.h> NOT USED
 */

/*----------------------------------------------------------------------------
 * <stdarg.h> NOT USED
 */

/*----------------------------------------------------------------------------
 * <stddef.h> NO DECLARATIONS
 */

/*----------------------------------------------------------------------------
 * <stdio.h>
 */

#undef	_iob
FILE	_iob[3];

#undef	clearerr
void	clearerr(stream) FILE *stream; { return; }

#undef	fclose
int	fclose(stream) FILE *stream; { return 0; }

#undef	feof
int	feof(stream) FILE *stream; { return 0; }

#undef	ferror
int	ferror(stream) FILE *stream; { return 0; }

#undef	fflush
int	fflush(stream) FILE *stream; { return 0; }

#undef	fgetc
int	fgetc(stream) FILE *stream; { return 0; }

#undef	fgetpos
int	fgetpos(stream, pos) FILE *stream; fpos_t *pos; { return 0; }

#undef	fgets
char *	fgets(s, n, stream) char *s; int n; FILE *stream; { return 0; }

#undef	fopen
FILE *	fopen(filename, mode) char *filename; char *mode; { return 0; }

#undef	fprintf
/* VARARGS2 */
int	fprintf(stream, format) FILE *stream; char *format; { return 0; }

#undef	fputc
int	fputc(c, stream) int c; FILE *stream; { return 0; }

#undef	fputs
int	fputs(s, stream) char *s; FILE *stream; { return 0; }

#undef	fread
size_t	fread(ptr, size, nmemb, stream)
	voidp_t ptr; size_t size; size_t nmemb; FILE *stream;
	{ return 0; }

#undef	freopen
FILE *	freopen(filename, mode, stream)
	char *filename; char *mode; FILE *stream;
	{ return 0; }

#undef	fscanf
/* VARARGS2 */
int	fscanf(stream, format) FILE *stream; char *format; { return 0; }

#undef	fseek
int	fseek(stream, offset, whence) FILE *stream; long offset; int whence;
	{ return 0; }

#undef	fsetpos
int	fsetpos(stream, pos) FILE *stream; fpos_t *pos; { return 0; }

#undef	ftell
long	ftell(stream) FILE *stream; { return 0; }

#undef	fwrite
size_t	fwrite(ptr, size, nmemb, stream)
	voidp_t ptr; size_t size; size_t nmemb; FILE *stream;
	{ return 0; }

#undef	getc
int	getc(stream) FILE *stream; { return 0; }

#undef	getchar
int	getchar() { return 0; }

#undef	gets
char *	gets(s) char *s; { return 0; }

#undef	perror
void	perror(s) char *s; { return; }

#undef	printf
/* VARARGS */
int	printf(format) char *format; { return 0; }

#undef	putc
int	putc(c, stream) int c; FILE *stream; { return 0; }

#undef	putchar
int	putchar(c) int c; { return 0; }

#undef	puts
int	puts(s) char *s; { return 0; }

#undef	remove
int	remove(filename) char *filename; { return 0; }

#undef	rename
int	rename(old, new) char *old; char *new; { return 0; }

#undef	rewind
void	rewind(stream) FILE *stream; { return; }

#undef	scanf
/* VARARGS */
int	scanf(format) char *format; { return 0; }

#undef	setbuf
void	setbuf(stream, buf) FILE *stream; char *buf; { return; }

#undef	setvbuf
int	setvbuf(stream, buf, mode, size)
	FILE *stream; char *buf; int mode; size_t size;
	{ return 0; }

#undef	sprintf
/* VARARGS2 */
int	sprintf(s, format) char *s; char *format; { return 0; }

#undef	sscanf
/* VARARGS2 */
int	sscanf(s, format) char *s; char *format; { return 0; }

#undef	tmpfile
FILE *	tmpfile() { return 0; }

#undef	tmpnam
char *	tmpnam(s) char *s; { return 0; }

#undef	ungetc
int	ungetc(c, stream) int c; FILE *stream; { return 0; }

#undef	vfprintf
int	vfprintf(stream, format, arg) FILE *stream; char *format; va_list arg;
	{ return 0; }

#undef	vprintf
int	vprintf(format, arg) char *format; va_list arg; { return 0; }

#undef	vsprintf
int	vsprintf(s, format, arg)
	char *s; char *format; va_list arg;
	{ return 0; }

/*----------------------------------------------------------------------------
 * <stdlib.h>
 *
 * UNUSED:
 *	div      ldiv     mblen    mbstowcs mbtowc   wcstombs wctomb
 */

#undef	abort
void	abort() { return; }

#undef	abs
int	abs(j) int j; { return 0; }

#undef	atexit
int	atexit(func) void (*func)(); { return 0; }

#undef	atof
double	atof(nptr) char *nptr; { return 0; }

#undef	atoi
int	atoi(nptr) char *nptr; { return 0; }

#undef	atol
long	atol(nptr) char *nptr; { return 0; }

#undef	bsearch
voidp_t	bsearch(key, base, nmemb, size, compar)
	voidp_t key; voidp_t base; size_t nmemb; size_t size; int (*compar)();
	{ return 0; }

#undef	calloc
voidp_t	calloc(nmemb, size) size_t nmemb; size_t size; { return 0; }

#undef	exit
void	exit(status) int status; { return; }

#undef	free
void	free(ptr) voidp_t ptr; { return; }

#undef	getenv
char *	getenv(name) char *name; { return 0; }

#undef	labs
long	labs(j) long j; { return 0; }

#undef	malloc
voidp_t	malloc(size) size_t size; { return 0; }

#undef	qsort
void	qsort(base, nmemb, size, compar)
	voidp_t base; size_t nmemb; size_t size; int (*compar)();
	{ return; }

#undef	rand
int	rand() { return 0; }

#undef	realloc
voidp_t	realloc(ptr, size) voidp_t ptr; size_t size; { return 0; }

#undef	srand
void	srand(seed) unsigned seed; { return; }

#undef	strtod
double	strtod(nptr, endptr) char *nptr; char **endptr; { return 0; }

#undef	strtol
long	strtol(nptr, endptr, base) char *nptr; char **endptr; int base;
	{ return 0; }

#undef	strtoul
unsigned long	strtoul(nptr, endptr, base)
		char *nptr; char **endptr; int base;
		{ return 0; }

#undef	system
int	system(string) char *string; { return 0; }

/*----------------------------------------------------------------------------
 * <string.h>
 */

#undef	memchr
voidp_t	memchr(s, c, n) voidp_t s; int c; size_t n; { return 0; }

#undef	memcmp
voidp_t	memcmp(s1, s2, n) voidp_t s1; voidp_t s2; size_t n; { return 0; }

#undef	memcpy
voidp_t	memcpy(s1, s2, n) voidp_t s1; voidp_t s2; size_t n; { return 0; }

#undef	memmove
voidp_t	memmove(s1, s2, n) voidp_t s1; voidp_t s2; size_t n; { return 0; }

#undef	memset
voidp_t	memset(s, c, n) voidp_t s; int c; size_t n; { return 0; }

#undef	strcat
char *	strcat(s1, s2) char *s1; char *s2; { return 0; }

#undef	strchr
char *	strchr(s, c) char *s; int c; { return 0; }

#undef	strcmp
int	strcmp(s1, s2) char *s1; char *s2; { return 0; }

#undef	strcoll
int	strcoll(s1, s2) char *s1; char *s2; { return 0; }

#undef	strcpy
char *	strcpy(s1, s2) char *s1; char *s2; { return 0; }

#undef	strcspn
size_t	strcspn(s1, s2) char *s1; char *s2; { return 0; }

#undef	strerror
char *	strerror(errnum) int errnum; { return 0; }

#undef	strlen
size_t	strlen(s) char *s; { return 0; }

#undef	strncat
char *	strncat(s1, s2, n) char *s1; char *s2; size_t n; { return 0; }

#undef	strncmp
int	strncmp(s1, s2, n) char *s1; char *s2; size_t n; { return 0; }

#undef	strncpy
char *	strncpy(s1, s2, n) char *s1; char *s2; size_t n; { return 0; }

#undef	strpbrk
char *	strpbrk(s1, s2) char *s1; char *s2; { return 0; }

#undef	strrchr
char *	strrchr(s, c) char *s; int c; { return 0; }

#undef	strspn
size_t	strspn(s1, s2) char *s1; char *s2; { return 0; }

#undef	strstr
char *	strstr(s1, s2) char *s1; char *s2; { return 0; }

#undef	strtok
char *	strtok(s1, s2) char *s1; char *s2; { return 0; }

#undef	strxfrm
size_t	strxfrm(s1, s2, n) char *s1; char *s2; size_t n; { return 0; }

/*----------------------------------------------------------------------------
 * <time.h>
 */

#undef	asctime
char *	asctime(timeptr) tm_t *timeptr; { return 0; }

#undef	clock
clock_t	clock() { return 0; }

#undef	ctime
char *	ctime(timer) time_t *timer; { return 0; }

#undef	difftime
double	difftime(time1, time0) time_t time1; time_t time0; { return 0; }

#undef	gmtime
tm_t *	gmtime(timer) time_t *timer; { return 0; }

#undef	localtime
tm_t *	localtime(timer) time_t *timer; { return 0; }

#undef	mktime
time_t	mktime(timeptr) tm_t *timeptr; { return 0; }

#undef	strftime
size_t	strftime(s, maxsize, format, timeptr)
	char *s; size_t maxsize; char *format; tm_t *timeptr;
	{ return 0; }

#undef	time
time_t	time(timer) time_t *timer; { return 0; }

/* lint */
#endif

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/syslint/RCS/ansi.c,v 1.2 90/11/11 17:20:45 frew Exp $";

#endif
