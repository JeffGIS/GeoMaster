/*
	macros from qdips macros.h that are useful or required for conversion
	of qdips pgms to ipw that have not been included in ~ipw/h/macro.h
 */
/*****************************************************************************/

/*
 *  absolute value of x
 */
#define	abs(x)				( (x) < 0 ? -(x) : (x) )
#ifndef ABS
#define	ABS(x)				( (x) < 0 ? -(x) : (x) )
#endif

/*
 *  set i'th bit
 */
#define	BIT(i)				( 1 << (i) )

/*
 *  print name and current value of variable (for debugging)
 */
#ifdef	DEBUG
#define	DBFLUSH()			(void)fflush(stdout)
#define	DBID()				printf("===>  %s(%d)  <===\n", \
						__FILE__, __LINE__)
#define	DBPR(var,fmt)			printf("***  %s(%d) var = %fmt\n", \
						__FILE__, __LINE__, var)
#define	debug(var,fmt)			printf("***  %s(%d) var = %fmt\n", \
						__FILE__, __LINE__, var)
#define	DBSLEEP(x)			sleep(x);
#define	DBSTR(str)			printf("***  %s(%d): %s :", \
						__FILE__, __LINE__, str)
#else
#define	DBFLUSH()
#define	DBID()
#define	DBPR(var, fmt)
#define	debug(var, fmt)
#define	DBSLEEP(x)
#define	DBSTR(str)
#endif

/*
 *  convert degress, minutes, seconds to decimal degrees
 */
#define	dmstd(d, m, s)			( (d) < 0 ?\
			(double)((d)  -  (m) / 60.  -  (s) / 3600.) :\
			(double)((d)  +  (m) / 60.  +  (s) / 3600.) )
#define	DMSTD(d, m, s)			( (d) < 0 ?\
			(double)((d)  -  (m) / 60.  -  (s) / 3600.) :\
			(double)((d)  +  (m) / 60.  +  (s) / 3600.) )

/*
 *  is i even?
 */
#define	even(i)				( !((i) & 01) )
#define	EVEN(i)				( !((i) & 01) )

/*
 *  does file f exist?
 */
#define	exists(f)			( access((f), 0) == 0 )
#define	EXISTS(f)			( access((f), 0) == 0 )

/*
 *  is file descriptor fd open?
 */
#define	ISOPEN(fd)			( close(dup(fd)) == 0 )

/*
 *  is year a leap year?
 */
#define LEAP(yr)			( yr && !(yr%4) &&\
						(yr%100 || !(yr%400)) )

/*
 *  mask low-order i bits
 */
#define	MASK(i)				( BIT(i) - 1 )

/*
 *  'typeless' min and max functions
 */
#define	max(x, y)			( (x) > (y) ? (x) : (y) )
#ifndef MAX
#define	MAX(x, y)			( (x) > (y) ? (x) : (y) )
#endif

#define	min(x, y)			( (x) < (y) ? (x) : (y) )
#ifndef MIN
#define	MIN(x, y)			( (x) < (y) ? (x) : (y) )
#endif

/*
 *  is i odd?
 */
#define	odd(i)				( (i) & 01 )
#define	ODD(i)				( (i) & 01 )

/*
 *  is i between j and k?
 */
#define isrange(i,j,k)			( (i) == (j)  ||  (i) == (k)  ||\
						(((i) > (j)) ^ ((i) > (k))) )
#define ISRANGE(i,j,k)			( (i) == (j)  ||  (i) == (k)  ||\
						(((i) > (j)) ^ ((i) > (k))) )

/*
 *  radians to decimal degrees
 */
#define	rtd(r)				( (r) * 57.29577951308232088 )
#define	RTD(r)				( (r) * 57.29577951308232088 )

/*
 *  # of bytes in string s
 */
#define	strsiz(s)			( strlen(s) + 1 )
#define	STRSIZ(s)			( strlen(s) + 1 )

/*
 *  vector position for matrix in symmetric storage mode
 */
#define	symadd(i,j)			( (i) <= (j) ?\
						((j) + 1) * (j) / 2 + (i) :\
						((i) + 1) * (i) / 2 + (j) )
#define	SYMADD(i,j)			( (i) <= (j) ?\
						((j) + 1) * (j) / 2 + (i) :\
						((i) + 1) * (i) / 2 + (j) )
