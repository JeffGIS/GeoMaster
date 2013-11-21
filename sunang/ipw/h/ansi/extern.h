#ifndef	EXTERN_H
#define	EXTERN_H

/*
 * UNIX extern
 *
 *  corrected ANSI version of IPW's extern.h
 *
 *  also supports GNU extensions if using GNU CC
 *
 */


#ifndef __NORETURNB
   /*
    * functions that do not return should have a __NORETURNB in front, and
    * a __NORETURNE on the end.  That allows us to define them in however
    * various compilers want to define them.
    * What this means is that a function is going to call exit or abort, so
    * it will not return.  This prevents uninitialized variable warnings, and
    * perhaps better optimization.
    */
#if (__GNUC__ > 1) && (__GNUC_MINOR__ > 4)
#  define __NORETURNB
#  define __NORETURNE	__attribute__ ((noreturn))
#else
   /* Note that ANSI has decided that volatile means something for functions,
    * so we can't use it to declare functions that do not return.
    */
#  define __NORETURNB
#  define __NORETURNE
#endif

#endif /* defined __NORETURNB */

extern int      errno;
extern char    *ipwoptarg;
extern int      ipwopterr;
extern int      ipwoptind;
extern int      ipwoptopt;
extern char    *sys_errlist[];
extern int      sys_nerr;


#if CC_EXTRA_STDIO                 /* If you have a stdio that is missing lots
                                      of things, as is very typical.          */
#ifndef CC_NO_EXTRA_STDIO          /* Same system, different compiler.  Put
                                      this is C_LOCAL or C_SPECIAL            */
#if CC_NEED_FLSBUF                 /* Need a definition for flush_buffer      */
    /* This is really annoying -- Sun acc calls this int, gcc < 2.4 doesn't
       care, gcc 2.5 calls it an unsigned int, and stdio puts unsigned chars
       in it.  On a Sun, they're all the same, but.. */
/* extern int      _flsbuf(int, FILE *); */
extern int      _flsbuf();
#endif
extern int      puts(CONST char *__s);
extern int      fputs(CONST char *__s, FILE *__stream);
extern int      printf(CONST char *__format, ...);
extern int      fprintf(FILE *__stream, CONST char *__format, ...);
extern int      scanf(CONST char *__format, ...);
extern int      fscanf(FILE *__stream, CONST char *__format, ...);
extern int      sscanf(CONST char *__s, CONST char *__format, ...);
extern size_t   fread(void * __ptr, size_t __size,
                      size_t __n, FILE *__stream);
extern size_t   fwrite(CONST void * __ptr, size_t __size, size_t __n, FILE *__s);
extern int      fclose(FILE *__stream);
extern int      fflush(FILE *__stream);
extern FILE   * fopen(CONST char *__filename, CONST char *__modes);
extern FILE   * fdopen(int fd, CONST char *type);
extern int      fseek(FILE *__stream, long int __off, int __whence);
extern long int ftell(FILE *__stream);
extern int      getw(FILE *__stream);
extern int      putw(int __w, FILE *__stream);
extern int      fgetc(FILE *__stream);
extern int      pclose(FILE *__stream);
extern void     setbuf(FILE *__stream, char *__buf);
extern void     setbuffer(FILE *__stream, char *__buf, int __size);
extern void     setlinebuf(FILE *__stream);
extern int      ungetc(int __c, FILE *__stream);
extern void     perror(CONST char *__s);
extern char   * tempnam(CONST char *__dir, CONST char *__pxf);
#if CC_NEED_VPRINT
extern int      vfprintf(FILE *stream, CONST char *format, va_list arg);
extern int      vprintf(CONST char *format, va_list arg);
/*
 * SunOS 4.x stupidly screwed this up, making it return a char *.  Unless
 * we make a special case just for SunOS 4.x, we can't define this.  Argh!
 *
 * extern int      vsprintf(char *s, CONST char *format, PTR arg);
 */
#endif   /* vprint stuff */
#ifndef isascii
extern int      isascii(int c);
#endif
#endif  /* CC_NO_EXTRA_STDIO not defined */
#endif  /* CC_EXTRA_STDIO defined */

/*
 * stdlib.h stuff
 */

#ifndef CC_NO_EXTRA_STDLIB
extern int        abs(int i);
extern double     atof(CONST char *__nptr);
extern int        atoi(CONST char *__nptr);
extern long int   atol(CONST char *__nptr);
#if defined(sparc) && defined(__svr4__)
extern void     * bsearch(CONST void *key, CONST void *base, size_t nmemb,
                      size_t size, int (*compar)(CONST void *a, CONST void *b));
#else
extern char     * bsearch(CONST void *key, CONST void *base, size_t nmemb,
                      size_t size, int (*compar)(CONST void *a, CONST void *b));
#endif
extern void     * calloc(xsize_t __nmemb, xsize_t __size);
extern void __NORETURNB exit(int __status) __NORETURNE; 
extern void       free(void * __ptr);
extern char     * getenv(CONST char *__name);
extern void     * malloc(xsize_t __size);
typedef int __cmpfunc(CONST PTR a, CONST PTR b);
#if CC_QSORT_VOID
extern void       qsort(CONST PTR base, size_t nelements, size_t size,
                        int (*compar)(CONST PTR a, CONST PTR b));
/*
extern void       qsort(CONST PTR base, size_t nelements, size_t size,
                        __cmpfunc *compar);
*/
#else
extern int        qsort(CONST PTR base, size_t nelements, size_t size,
                        __cmpfunc *compar);
#endif
extern int        _quicksort(CONST PTR base, size_t nmemb, size_t size,
                        __cmpfunc *compar);
extern int        rand(void);
extern long int   random(void);
extern void     * realloc(void * __ptr, xsize_t __size);
#if defined(sparc) && defined(__svr4__)
extern void       srand(unsigned int __seed);
#else
extern int        srand(unsigned int __seed);
#endif
extern void       srandom(unsigned int __seed);
/*
 * This isn't part of POSIX, so it goes wherever the vendor feels like
 * putting it.  Too bad, because it just makes our job that much harder.
 */
extern double     hypot(double x, double y);
#endif

/*
 * string.h stuff
 */

extern char     * strcat(char *__src, CONST char *__dest);
extern char     * strchr(CONST char *__s, int __c);
extern int        strcmp(CONST char *__s1, CONST char *__s2);
extern char     * strcpy(char *__dest, CONST char *__src);
extern xsize_t    strcspn(CONST char *__s, CONST char *__reject);
extern xsize_t    strlen(CONST char *__s);
extern char     * strncat(char *__s1, CONST char *__s2, xsize_t __n);
extern int        strncmp(CONST char *__s1, CONST char *__s2, xsize_t __n);
extern char     * strrchr(CONST char *__s, int __c);
extern xsize_t    strspn(CONST char *__s, CONST char *__accept);
extern char     * strstr(CONST char *__s1, CONST char *__s2);

/*
 * memory part of string.h (Sun puts this in memory.h)
 */

extern void     * memcpy(PTR dest, CONST PTR src, xsize_t n);
extern void     * memset(PTR s, int c, xsize_t n);

#if !CC_NOBCOPY
extern void     * bcopy(CONST PTR source, PTR dest, xsize_t n);
#endif
#if !CC_NOBZERO
extern void     * bzero(PTR s, xsize_t n);
#endif

/*
 * If we're BSD, index and rindex are in strings.h
 */

#if CC_INDEX
extern char     * index(CONST char *__s, int __c);
#endif
#if CC_RINDEX
extern char     * rindex(CONST char *__s, int __c);
#endif

/*
 * unistd.h stuff
 */

#if CC_USE_UNISTD
#include <unistd.h>
#else

extern int        close(int __fd);
extern int        isatty(int __fd);
extern off_t      lseek(int __fd, off_t __offset, int __whence);
extern int        unlink(CONST char *__path);
   /*
    * POSIX says nbyte is an unsigned int.
    * K & R ANSI C standard says it is an int.
    * SunOS 4.1 manual says int, but header files say unsigned.
    * Solaris 2.1 manual says size_t, but header files say unsigned.
    * DG manual says unsigned, but header files say size_t.
    * HP manual says size_t, but header files say unsigned.
    *
    * Hey, at least size_t == unsigned int on these systems.
    * AIX says size_t == unsigned long.
    */
extern int        read(int fd, PTR buf, unsigned int nbyte);
extern int        write(int fd, CONST PTR buf, unsigned int nbyte);

#endif /* CC_USE_UNISTD */

extern time_t     time(time_t *__timer);

/*
 * libunix stuff
 */

extern int      dysize(int year);
extern int      remove(CONST char *__filename);
extern double   difftime(xtime_t time1, xtime_t time0);
extern int      isinf(double value);

/******************************************************************************
 *                                                                            *
 *          Begin IPW routines -- no O/S specific things below                *
 *                                                                            *
 ******************************************************************************/
     
/*
 * IPW main
 */

/* ipwenter is defined in getargs.h so OPTION_T could be there */
extern void __NORETURNB    ipwexit(int status) __NORETURNE;

/*
 * IPW error
 */

extern void __NORETURNB   _bug(CONST char *msg, CONST char *file, int line)
                          __NORETURNE;
extern void __NORETURNB   error(CONST char *format, ...) __NORETURNE;
extern void               usrerr(CONST char *format, ...);
extern void               warn(CONST char *format, ...);
extern void               syserr(void);
extern void               uferr(int fd);

/*
 * IPW uio
 */

extern bool_t   ubof(int fd);
extern int      uclose(int fd);
extern long     ucopy(int fdi, int fdo, long ncopy);
extern bool_t   ueof(int fd);
extern char    *ufilename(int fd);
extern char    *ugets(int fd, char *buf, int nbytes);
extern int      uputs(int fd, CONST char *buf);
extern int      uread(int fd, addr_t buf, int nbytes);
extern int      uremove(CONST char *filename);
extern int      uropen(CONST char *name);
extern long     urskip(int fd, long nbytes);
extern int      ustdin(void);
extern int      ustdout(void);
extern int      uwflush(int fd);
extern int      uwopen(CONST char *name);
extern int      uwrite(int fd, CONST addr_t buf, int nbytes);
extern void     usetlevel(int fd, int level);
extern int      ugetlevel(int fd);

/*
 * strvec package
 */

extern STRVEC_T *addsv(STRVEC_T *strvec, char *string);
extern STRVEC_T *delsv(STRVEC_T *strvec, int i);
extern STRVEC_T *dupsv(STRVEC_T *strvec);
extern bool_t    ok_sv(STRVEC_T *strvec);
extern int       freesv(STRVEC_T *strvec);
extern char     *walksv(STRVEC_T *strvec, bool_t reset);

/*
 * IPW util
 */

extern addr_t        allocnd(int elsize, int ndim, ...);
extern char         *dtoa(char *s, double d);
extern char         *ftoa(char *s, float f);
extern addr_t        ecalloc(int nelem, int elsize);
extern addr_t        emalloc(int nelem, int elsize);
extern float         frand(void);
extern void          frinit(void);
extern int           hbit(unsigned i);
extern addr_t        hdralloc(int n, int size, int fd, CONST char *name);
extern char         *hstrdup(CONST char *s, CONST char *name, int band);
extern int           imgcopy(int i_fd, int o_fd);
extern char         *itoa(char *s, int i);
extern char         *ltoa(char *s, long i);
extern double        ltof(long i);
extern int           ltoi(long i);
extern unsigned int  ltou(long i);
extern int     	     maxfiles(void);
extern int           ndig(int i);
extern void          no_tty(int fd);
extern int           ipow2(int expo);
extern char         *rmlead(char *s);
extern void          rmtrail(char *s);
extern char         *strdup(CONST char *s);
#if defined(_TIME_H) || defined(__time_h)
extern long          tdiff(struct tm *t2, struct tm *t1);
#endif

/*
 * other IPW routines
 */

extern int	ipwgetopt(int argc, char* CONST *argv, CONST char *optstring);

#endif
