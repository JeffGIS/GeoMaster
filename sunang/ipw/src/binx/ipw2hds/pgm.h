#ifndef	PGM_H
#define	PGM_H

/*
 * header file for "hdspic" program
 */

typedef struct {
	int             i_fd;		/* input image file descriptor	 */
	int             i_nlines;	/* # input image lines		 */
	int             i_nsamps;	/* # samples / input image line	 */
} PARM_T;

extern PARM_T   parm;

extern void     headers();
extern void     hdspic();

/* $Header: pgm.h,v 1.1 89/04/16 17:35:02 frew Exp $ */
#endif
