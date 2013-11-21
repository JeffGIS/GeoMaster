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
 * do_kmap -- initialize kernel (weight*fpixel)[pixel] maps
 */

fpixel_t     ***
DEFUN(do_kmap, (kernel, nrows, ncols, map, maplen),
	fpixel_t      **kernel		/* -> kernel array		 */
   AND  int             nrows		/* # kernel rows		 */
   AND  int             ncols		/* # kernel columns		 */
   AND  fpixel_t       *map		/* -> fpixel[pixel] map		 */
   AND  int             maplen)		/* # map elements		 */
{
	REG_1 fpixel_t ***kmap;		/* -> kernel map array		 */
	int             row;		/* current kernel row counter	 */

	assert(map != NULL);
	assert(maplen > 0);
 /*
  * allocate kernel map pointer array
  */
 /* NOSTRICT */
	kmap = (fpixel_t ***) allocnd(sizeof(fpixel_t *), 2, nrows, ncols);
	if (kmap == NULL) {
		error("can't allocate kernel map pointer array");
	}
 /*
  * loop through kernel rows
  */
	for (row = 0; row < nrows; ++row) {
		REG_6 int       col;	/* current kernel column	 */

		if (kernel[row] == NULL) {
			continue;
		}
 /*
  * loop through kernel columns
  */
		for (col = 0; col < ncols; ++col) {
			REG_2 int       c;	/* column index		 */
			REG_5 int       i;	/* map index		 */
			REG_4 fpixel_t *mp;	/* -> weight*fpixel map	 */
			REG_3 int       r;	/* row index		 */
			FREG_1 fpixel_t val;	/* kernel[row][col]	 */

			val = kernel[row][col];
			if (val == 0) {
				continue;
			}
 /*
  * compare kernel[row][col] against kernel value already processed -- if a
  * match is found, use the previously computed map
  */
			for (r = 0; r < row; ++r) {
				if (kernel[r] == NULL) {
					continue;
				}

				for (c = 0; c < ncols; ++c) {
					if (val == kernel[r][c]) {
						mp = kmap[r][c];
						assert(mp != NULL);

						goto found;
					}
				}
			}

			for (c = 0; c < col; ++c) {
				if (val == kernel[row][c]) {
					mp = kmap[row][c];
					assert(mp != NULL);

					goto found;
				}
			}
 /*
  * no match -- compute a new map
  */
 /* NOSTRICT */
			mp = (fpixel_t *) ecalloc(maplen, sizeof(fpixel_t));
			if (mp == NULL) {
				error("can't allocate kernel map array");
			}

			for (i = 0; i < maplen; ++i) {
				mp[i] = map[i] * val;
			}
	found:
			kmap[row][col] = mp;
		}
	}

	return (kmap);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/convolve/RCS/do_kmap.c,v 1.2 90/11/11 17:00:57 frew Exp $";

#endif
