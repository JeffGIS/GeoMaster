/* LINTLIBRARY */

#include "ipw.h"

#include "rasterfile.h"
#include "sunras.h"

/*
 * sunhdr -- write Sun raster image file header
 */

int
sunhdr(nlines, nsamps, nbits, o_fd)
	int             nlines;		/* # image lines		 */
	int             nsamps;		/* # samples / line	 	 */
	int             nbits;		/* # bits / pixel		 */
	int             o_fd;		/* output image file descriptor	 */
{
	int             nbits_line;	/* # bits / line		 */
	int             nbytes_line;	/* # bytes / line		 */

	struct rasterfile ras;		/* Sun rasterfile header	 */

 /* NOSTRICT */
	ras.ras_magic = RAS_MAGIC;

	ras.ras_width = nsamps;
	ras.ras_height = nlines;
	ras.ras_depth = nbits;

	nbits_line = nsamps * nbits;
 /*
  * Sun bitmap lines are rounded up to a multiple of NBITS_ROUND bits
  */
	nbits_line += NBITS_ROUND - 1;
	nbits_line /= NBITS_ROUND;
	nbits_line *= NBITS_ROUND;

	nbytes_line = nbits_line / CHAR_BIT;

	ras.ras_length = nlines * nbytes_line;
	ras.ras_type = RT_STANDARD;

	if (nbits == 1) {
		ras.ras_maptype = RMT_NONE;
		ras.ras_maplength = 0;
	}
	else {
		ras.ras_maptype = RMT_EQUAL_RGB;
		ras.ras_maplength = 3 * 256;
	}
 /*
  * write rasterfile header
  */
 /* NOSTRICT */
	if (uwrite(o_fd, (addr_t) (&ras), sizeof(ras)) != sizeof(ras)) {
		error("can't write rasterfile header");
	}
 /*
  * if 8-bit pixels then write fake (1:1) colormap
  */
	if (nbits == 8) {
		int		i;	/* loop counter			*/

		char		map[256];	/* colormap array	*/

		for (i = 0; i < sizeof(map); ++i) {
			map[i] = i;
		}

		for (i = 0; i < 3; ++i) {
			if (uwrite(o_fd, map, sizeof(map)) != sizeof(map)) {
				error("can't write fake colormap");
			}
		}
	}
 /*
  * return rasterfile line size
  */
	return (nbytes_line);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/ipw/src/bin/sunras/RCS/sunhdr.c,v 1.2 89/10/22 18:50:33 frew Exp $";

#endif
