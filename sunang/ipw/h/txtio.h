#ifndef	TXTIO_H
#define	TXTIO_H

#include <stdio.h>

typedef FILE *  TEXT_FD_T;

#define NO_TEXT_FD NULL

/*
 * Modes for "topen" routine
 */
#define IPW_READ  "r"
#define IPW_WRITE "w"

/*
 *  Functions for text files.
 *
 *  (Note:  Currently almost all these functions are simply macros that
 *	    refer to standard C library routines for file streams.)
 */
#define topen	fopen
#define tclose	fclose
#define tscanf	fscanf
#define tprintf	fprintf
#define tflush	fflush

#define tstdin	stdin
#define tstdout	stdout
#define	tstderr	stderr

extern 	int	EXFUN(is_a_tty, (TEXT_FD_T fd));

#endif
