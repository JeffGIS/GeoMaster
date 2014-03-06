/* port/cpl_config.h.  Generated automatically by configure.  */
/* port/cpl_config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

#define HAVE_LIBDL 1 

#define HAVE_DLFCN_H 1
/* #undef HAVE_DBMALLOC_H */
/* #undef HAVE_LIBDBMALLOC */
/* #undef WORDS_BIGENDIAN */

/* Define if you have ftell64 and fseek64 */
#define UNIX_STDIO_64 1

/* What 64bit stdio api entry points to use */
#define VSI_FTELL64 ftello64
#define VSI_FSEEK64 fseeko64

/* Define if you have 64 bit long long type */
#define HAVE_LONG_LONG 1

/* Define if large file api should be enabled, normally with UNIX_STDIO_64 */
#define VSI_LARGE_API_SUPPORTED 1

/* Define if _LARGEFILE64_SOURCE needs to be defined. */
#define VSI_NEED_LARGEFILE64_SOURCE 1



