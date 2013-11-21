
#ifdef CC_USE_UNISTD
#include <unistd.h>
#else

#ifndef	UNISTD_H
#define	UNISTD_H

/*
 * IEEE P1003 <unistd.h>
 *
 * symbolic constants for "access" function:
 *
 *	R_OK		test for "read" permission
 *	W_OK		test for "write" permission
 *	X_OK		test for "execute" (search) permission
 *	F_OK		test for existence of file
 *
 * symbolic constants for "lseek" function:
 *
 *	SEEK_SET	seek from beginning of file
 *	SEEK_CUR	seek from current position in file
 *	SEEK_END	seek from end of file
 */

#define	SEEK_CUR	1
#define	SEEK_END	2

/* $Header: /usr/home/ipw/h/posix/RCS/unistd.h,v 1.6 89/10/27 17:47:59 frew Exp $ */
#endif

#endif
