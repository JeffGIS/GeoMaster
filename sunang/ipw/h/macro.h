#ifndef	MACRO_H
#define	MACRO_H

/*
 * constants
 */

#define	BLANK		' '
#define	CREAT_MODE	0666		/* default mode for creat()	 */
#define	DEC_DIGITS	"0123456789"	/* legal decimal digits		 */
#define	EOS		'\0'
#define	ERROR		(-2)		/* integer function error return */
#define	EX_ERROR	1		/* error exit() code		 */
#define	EX_OK		0		/* normal exit() code		 */
#define	FALSE		0
#define	FD_STDERR	2		/* stderr file desriptor	 */
#define	FD_STDIN	0		/* stdin file desriptor		 */
#define	FD_STDOUT	1		/* stdout file desriptor	 */
#define	HDR_VERSIZ	64		/* #chars in img hdr version string */
#define	OK		1		/* integer function normal return */
#ifndef	PI
#define	PI		3.141592653589793	/* pi			 */
#endif
#define	PORT_NAMELEN	14		/* #chars in a "portable" filename */
#define	SYS_EOF		0		/* system call EOF return	 */
#define	SYS_ERROR	(-1)		/* system call error return	 */
#define	SYS_OK		0		/* system call normal return	 */
#define	TRUE		1
#define	WHITE_SPACE	" \t\n"



/*
 * Defaults for config.h
 */

#ifndef SORT_ALG
/*
 * By default, use qsort.  On systems where qsort is slow, add _quicksort
 * to the libunix Makefile, and make install.   Then change SORT_ALG to
 * _quicksort in config.h.
 */
#define SORT_ALG	qsort
#endif



/*
 * functions
 */

#define	DTR(d)			( (d) * (PI / 180.0) )
#ifndef MAX
#define	MAX(a, b)		( (a) > (b) ? (a) : (b) )
#endif
#ifndef MIN
#define	MIN(a, b)		( (a) < (b) ? (a) : (b) )
#endif
#ifndef ABS
#define ABS(x)			( (x) < 0 ? -(x) : (x) )
#endif
#ifndef ROUND
#define	ROUND(x)		( (x) < 0 ? (x) - 0.5 : (x) + 0.5 )
#endif
#define OVER_MAX_FD(fd)		( ((fd) >= OPEN_MAX) && ((fd) >= maxfiles()) )
#define OK_FD(fd)		( ((fd) >= 0) && ( ((fd) < OPEN_MAX) || \
                                                   ((fd) < maxfiles())    ) )
#define ASSERT_OK_FD(fd)	assert( (fd) >= 0 ); \
				assert( (fd) < OPEN_LIMIT); \
				if (OVER_MAX_FD((fd))) { \
				  error ("Too many open files.  Use 'limit' to raise this number."); \
				}
#define	SAFE_FREE(p)		if ((p) != NULL) free(p)
#define	SKIP_DIGITS(p)		( (p) += strspn(p, DEC_DIGITS) )
#define	SKIP_SPACE(p)		( (p) += strspn(p, WHITE_SPACE) )
#define	STRDIFF(s1, s2)		( (s1)[0] != (s2)[0] && strdiff(s1, s2) )
#define	STRDIFFN(s1, s2, n)	( (s1)[0] != (s2)[0] && strdiffn(s1, s2, n) )
#define	STREQ(s1, s2)		( (s1)[0] == (s2)[0] && streq(s1, s2) )
#define	STREQN(s1, s2, n)	( (s1)[0] == (s2)[0] && streqn(s1, s2, n) )

/* While __STDC__ is supposed to be defined if you meet the ANSI standard,
 * non-ANSI compilers can do anything they want.  Some vendors
 * have defined __STDC__ to 0 to mean that they are not-completely ANSI
 * complient.  What a mess.  What we have assumed is that __STDC__ defined
 * at all means prototypes are allowed, but __STDC__ must equal 1 to allow
 * ANSI preprocessor extensions (notably use of # and ##).
 */

#if __STDC__ == 1
#define	assert(expr)		if (! (expr)) \
					bug("assertion \""#expr"\" failed")
#else
#define	assert(expr)		if (! (expr)) bug("assertion \"expr\" failed")
#endif
#if CC_NOBCOPY
#define	bcopy(from, to, n)	( (void) memcpy(to, from, n) )
#endif
#define	bfext(i, o, w)		( ((i) >> (o)) & mask(w) )
#define	bfins(i, o, w)		( ((i) & mask(w)) << (o) )
#define	bit(i)			( 1 << (i) )
#define	bug(s)			_bug(s, __FILE__, __LINE__)
#if CC_NOBZERO
#define	bzero(p, n)		( (void) memset(p, 0, n) )
#endif
#define	kstrlen(s)		( sizeof(s) - 1 )
/* this was replaced by the two lines below:
 * #define	mask(n)			( ~(~0 << (n)) )
 */
#define	_mask(n)		( ~(~0 << (n)) )
#define	mask(n)			( (_mask((n) - 1) << 1) | 1 )
#define	pixidx(n, s, b)		( (s) * (n) + (b) )
#if CC_INDEX
#define	strchr			index
#endif
#define	strdiff(s1, s2)		( strcmp(s1, s2) != 0 )
#define	strdiffn(s1, s2, n)	( strncmp(s1, s2, n) != 0 )
#define	streq(s1, s2)		( strcmp(s1, s2) == 0 )
#define	streqn(s1, s2, n)	( strncmp(s1, s2, n) == 0 )
#if CC_RINDEX
#define	strrchr			rindex
#endif
#if CC_UCHAR
#define	uchar(i)		(i)
#else
#define	uchar(i)		( (i) & mask(CHAR_BIT) )
#endif
#if CC_ULONG
#define	ulong(i)		(i)
#else
#define	ulong(i)		( (i) & mask(CHAR_BIT * sizeof(ulong)) )
#endif
#if CC_USHORT
#define	ushort(i)		(i)
#else
#define	ushort(i)		( (i) & mask(CHAR_BIT * sizeof(ushort_t)) )
#endif

/* $Header: macro.h,v 1.13 89/05/18 18:50:43 frew Exp $ */
#endif
