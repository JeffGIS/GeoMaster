#ifndef	PIXIO_H
#define	PIXIO_H

extern int      EXFUN(pvread, (int fd, pixel_t *buf, int npixv));
extern int      EXFUN(pvwrite, (int fd, pixel_t *buf, int npixv));
extern int      EXFUN(pxclose, (int fd));

/* $Header: /usr/home/ipw/h/RCS/pixio.h,v 1.3 89/10/26 11:05:26 frew Exp $ */
#endif
