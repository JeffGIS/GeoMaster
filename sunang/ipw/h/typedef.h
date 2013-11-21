#ifndef	TYPEDEF_H
#define	TYPEDEF_H

/*
 * basic types
 */

#ifndef __sys_types_h		/* SunOS and SysVR4 don't like this */
#ifndef __types_h
#if defined(__STDC__) && !defined(_IBMR2)
typedef void   *addr_t;                 /* generic address               */
#else
typedef char   *addr_t;                 /* generic address               */
#endif
#endif
#endif

typedef int     bool_t;			/* TRUE or FALSE		 */

#if (!defined(_POSIX_SOURCE) && !defined(DGUX)) || defined(linux)

#if !defined(__sys_types_h) && !defined(_SYS_TYPES_H)
#ifndef _OFF_T
typedef long    off_t;			/* file offset			 */
#endif
#endif

#if CC_UCHAR
typedef unsigned char uchar_t;		/* unsigned char		 */
#else
typedef char    uchar_t;		/* unsigned char (fake)		 */
#endif

#if CC_ULONG
typedef unsigned long ulong_t;		/* unsigned long		 */
#else
typedef long    ulong_t;		/* unsigned long (fake)		 */
#endif

#if CC_USHORT
typedef unsigned short ushort_t;	/* unsigned short		 */
#else
typedef short   ushort_t;		/* unsigned short (fake)	 */
#endif

#endif /* _POSIX_SOURCE */

//typedef PIXEL   pixel_t;		/* integer pixel		 */
//typedef FPIXEL  fpixel_t;		/* floating-point pixel		 */
typedef int   pixel_t;		/* integer pixel		 */
typedef float  fpixel_t;		/* floating-point pixel		 */

#if !defined(__sys_stdtypes_h) && !defined(_SIZE_T) && !defined(__SIZE_TYPE__)
typedef long   xsize_t;		/* sizeof(anything)		 */
#else
typedef long	size_t;
#endif

#if !defined(__sys_stdtypes_h) && !defined(_TIME_T)
typedef long   xtime_t;		/* returned by time()		 */
#else
typedef time_t	xtime_t;
#endif

typedef struct {			/* vector of strings:		 */
	int             n;		/* -- # element in v		 */
	int             curr;		/* -- v[curr] is current string	 */
	char          **v;		/* -- -> strings		 */
} STRVEC_T;

/* $Header: /home/ipw/h/RCS/typedef.h,v 1.6 89/12/12 18:14:43 frew Exp $ */
#endif
