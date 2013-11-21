#ifndef	PGM_H
#define	PGM_H

/*
 * header file for "ipw2xim" program
 */

typedef struct {
	int             i_fd;		/* input image file descriptor	 */
	int             o_fd;		/* output image file descriptor	 */
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line		 */
	int             nbands;		/* # bands / sample		 */
	int             nlevels;	/* # levels / pixel		 */
} PARM_T;

#define	OUT_NLEVELS	256

extern PARM_T   parm;

extern void     headers();
extern void     ipw2xim();
extern void     ximhdr();

/* $Header: /usr.MC68020/home/ipw/adm/snoopy/src/bin/ipw2xim/RCS/pgm.h,v 1.1 89/10/25 19:53:53 frew Exp $ */
#endif
