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

#include "ipw.h"

#include "pixio.h"

#include "_pixio.h"

/*
** NAME
**	pvread -- read pixel vectors
**
** SYNOPSIS
**	#include "pixio.h"
**
**	int pvread(fd, buf, npixv)
**	int fd, npixv;
**	pixel_t *buf;
**
** DESCRIPTION
**	pvread reads npixv pixel vectors from file descriptor fd into buf.
**
** RESTRICTIONS
**
** RETURN VALUE
**	number of pixel vectors read; else ERROR for failure
**
** GLOBALS ACCESSED
**	_piocb[fd]	pixel I/O control block for file descriptor fd
**			(indirectly through _pioinit)
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	pvread is the lowest level of IPW input that represents pixel values
**	in a single format (unsigned integers).
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
pvread(fd, buf, npixv)
	int             fd;		/* input file descriptor	 */
	REG_4 pixel_t  *buf;		/* -> pixel buffer		 */
	int             npixv;		/* # pixel vectors to read	 */
{
	REG_3 int      *ip;		/* generic integer pointer	 */
	REG_6 int       n;		/* loop counter			 */
	REG_5 int       nbytes;		/* # bytes / raw input pixel	 */
	int             npixels;	/* total # pixels read		 */
	REG_2 char     *rawp;		/* -> raw input byte		 */
	PIXIO_T        *p;		/* -> pixel I/O control block	 */
	REG_1 char     *pixp;		/* -> byte within pixel_t	 */
	int             xnbytes;	/* saved nbytes (little-endians) */

	p = _pioinit(fd, PIXIO_READ, npixv);
	if (p == NULL) {
		return (ERROR);
	}
 /*
  * read raw pixels
  */
	nbytes = uread(fd, p->rawbuf, p->nbytes);
	if (nbytes <= 0) {
		return (nbytes);
	}

	if (nbytes == p->nbytes) {
		npixels = p->npixels;
	}
	else {
 /*
  * check that # raw pixels will make integral # converted pixel vectors
  */
		if (nbytes % p->pixvsiz != 0) {
			usrerr("partial pixel vector read");
			return (ERROR);
		}

		npixv = nbytes / p->pixvsiz;
		npixels = npixv * p->nbands;
	}
 /*
  * convert raw pixels -> pixel_t
  */
	ip = p->pixsiz;
	rawp = p->rawbuf;
 /* NOSTRICT */
	pixp = (char *) buf;

	n = npixels;

	if (p->flags & PIXIO_SWAP) {
		do {
			nbytes = *ip++;
			xnbytes = nbytes;
#if CC_BIGENDIAN
			pixp += sizeof(pixel_t);
#else
			pixp += xnbytes;
#endif
			do {
				*--pixp = *rawp++;
			} while (--nbytes > 0);
#if CC_BIGENDIAN
			pixp += xnbytes;
#else
			pixp += sizeof(pixel_t);
#endif
		} while (--n > 0);
	} else {
		do {
			nbytes = *ip++;
#if CC_BIGENDIAN
			pixp += sizeof(pixel_t);
			pixp -= nbytes;
#else
			xnbytes = nbytes;
#endif
			do {
				*pixp++ = *rawp++;
			} while (--nbytes > 0);
#if ! CC_BIGENDIAN
			pixp += sizeof(pixel_t);
			pixp -= xnbytes;
#endif
		} while (--n > 0);
	}

 /*
  * mask off unsignificant bits in buf
  */
	ip = p->pixmask;

	n = npixels;
	do {
		*buf++ &= *ip++;
	} while (--n > 0);
 /*
  * return # pixel vectors
  */
	return (npixv);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/pixio/RCS/pvread.c,v 1.9 90/11/11 17:17:21 frew Exp $";

#endif
