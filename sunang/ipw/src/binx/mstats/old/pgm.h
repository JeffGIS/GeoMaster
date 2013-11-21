#ifndef	PGM_H
#define	PGM_H

/*
 * header file for "mstats" program
 */

typedef struct {
	int             i_fd;		/* input image file descriptor	 */
	int             i_nlines;	/* # input lines		 */
	int             i_nsamps;	/* # samples / input line	 */
	int             i_nbands;	/* # bands / input sample	 */
	int             i_npixels;	/* # pixels / input image	 */
	fpixel_t       *i_buf;		/* -> input line buffer		 */
	double         *sum_x;		/* sum[band]			 */
	double        **sum_xy;		/* sum[band][band]		 */
} PARM_T;

extern PARM_T   parm;

extern void     accum();
extern void     headers();
extern void     init();
extern void     mcov();
extern void     mstats();

/* $Header: pgm.h,v 1.1 89/05/09 11:56:11 frew Exp $ */
#endif
