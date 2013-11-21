#ifndef	FPIO_H
#define	FPIO_H

extern int         EXFUN(fpclose, (int fd));
extern fpixel_t  * EXFUN(fpfmax, (int fd));
extern fpixel_t  * EXFUN(fpfmin, (int fd));
extern void        EXFUN(fphdrs, (int fdi, int nbands, int fdo));
extern fpixel_t ** EXFUN(fpmap, (int fd));
extern int       * EXFUN(fpmaplen, (int fd));
extern int         EXFUN(fpvread, (int fd, fpixel_t *buf, int npixv));
extern int         EXFUN(fpvwrite, (int fd, fpixel_t *buf, int npixv));
extern int         EXFUN(mnxfp, (fpixel_t *x, int npixv, int nbands, fpixel_t *mmval));

/* $Header: fpio.h,v 1.5 93/08/18 23:19:00 jacobsd Exp $ */
#endif
