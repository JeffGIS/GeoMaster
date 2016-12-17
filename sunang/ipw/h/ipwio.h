
/*
 * io.h
 *
 * Defines constants and definitions that are shared by all I/O routines.
 *
 * Currently only used to hold definitions of identifiers for the type of
 * file IPW has opened.
 *
 */


#ifndef IPWIO_H
#define IPWIO_H

#define FTYPE_MIN	0
#define FTYPE_TXTIO	0
#define FTYPE_UIO	1
#define FTYPE_PIXIO	2
#define FTYPE_FPIO	3
#define FTYPE_MAX	4

#define TEXT_FTYPE(x)	( ((x) == FTYPE_UIO)     ? "uio"   : \
			   ((x) == FTYPE_PIXIO)  ? "pixio" : \
			    ((x) == FTYPE_FPIO)  ? "fpio"  : "unknown" \
			)
#define ASSERT_IO_LEVEL(routine,file)	if ((routine) != (file)) { \
			error("Program called %s close instead of %s close.", \
				TEXT_FTYPE(routine), TEXT_FTYPE(file)); \
					}
#endif
