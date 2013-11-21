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

#include "convolve.h"

/*
 * getkernel -- read convolution kernel
 */

fpixel_t      **
DEFUN(getkernel, (fd, nrowsp, ncolsp),
	int             fd		/* kernel file descriptor	 */
   AND  int            *nrowsp		/* -> # rows in kernel		 */
   AND  int            *ncolsp)		/* -> # columns in kernel	 */
{
	REG_2 int       col;		/* current kernel column	 */
	FILE           *fp;		/* stdio -> fd			 */
	REG_1 fpixel_t **kernel;	/* -> kernel array		 */
	FREG_1 fpixel_t ksum;		/* sum of coefficients		 */
	REG_4 int       nrows;		/* # rows in kernel		 */
	REG_5 int       ncols;		/* # columns in kernel		 */
	REG_3 int       row;		/* current kernel row		 */

 /*
  * connect stdio stream to file
  */
	fp = fdopen(fd, "r");
	if (fp == NULL) {
		syserr();
		uferr(fd);
		error("can't open file for stdio input");
	}
 /*
  * read kernel size
  */
	if (fscanf(fp, "%d %d", nrowsp, ncolsp) != 2) {
		error("can't read kernel size");
	}

	nrows = *nrowsp;
	ncols = *ncolsp;

	if (nrows < 1 || ncols < 1) {
		error("bad kernel size: %dx%d", nrows, ncols);
	}

	if (!(nrows & 1 && ncols & 1)) {
		error("both kernel dimensions must be odd");
	}
 /*
  * allocate kernel array
  */
 /* NOSTRICT */
	kernel = (fpixel_t **) allocnd(sizeof(fpixel_t), 2, nrows, ncols);
	if (kernel == NULL) {
		error("can't allocate kernel array");
	}
 /*
  * read kernel
  */
	ksum = 0.0;

	for (row = 0; row < nrows; ++row) {
		bool_t          zero;	/* ? all cols in this row == 0	 */

		zero = TRUE;

		for (col = 0; col < ncols; ++col) {
			double          k;	/* current kernel value	 */

			if (fscanf(fp, "%lf", &k) != 1) {
				error("can't read kernel[%d][%d]", row, col);
			}

			if (k != 0.0) {
				kernel[row][col] = k;

				ksum += k;
				zero = FALSE;
			}
		}
 /*
  * if this row is all 0's then toss it
  */
		if (zero) {
			kernel[row] = NULL;
		}
	}

	(void) fclose(fp);
 /*
  * normalize kernel so coefficients sum to 1 (unless they already sum to 0)
  */
	if (ksum != 0) {
		for (row = 0; row < nrows; ++row) {
			if (kernel[row] != NULL) {
				for (col = 0; col < ncols; ++col) {
					kernel[row][col] /= ksum;
				}
			}
		}
	}
 /*
  * done
  */
	return (kernel);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/convolve/RCS/getkernel.c,v 1.2 90/11/11 17:01:00 frew Exp $";

#endif
