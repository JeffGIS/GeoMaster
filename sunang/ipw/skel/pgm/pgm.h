#ifndef	PGM_H
#define	PGM_H

/*
 * header file for "@PGM@" program
 */

typedef struct {
	int             i_fd;		/* input image file descriptor	 */
	int             o_fd;		/* output image file descriptor	 */
	@parameter declaration ...@
} PARM_T;

extern PARM_T   parm;

extern void     headers();
extern void     @PGM@();
@extern ...@

/* $Header: pgm.h,v 1.1 88/09/08 20:22:46 frew Exp $ */
#endif
