
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   copyimg.c   1.3   6/17/91";
#endif

/*
** NAME
** 	copyimg -- copy LAS binary image to IPW output image
** 
** SYNOPSIS
**	#include "las.h"
**
**	copyimg (lasimage, fdo, ddr, pixsiz, raw)
**	char *lasimage;
** 	int fdo;
**	struct DDR ddr;
**	int pixsiz;
**	bool_t raw;
** 
** DESCRIPTION
**	copyimg copies the binary data from the LAS image file to the
**	IPW image file.
** 
** RESTRICTIONS
** 
** RETURN VALUE
** 
** GLOBALS ACCESSED
** 
** ERRORS
** 
** WARNINGS
** 
** APPLICATION USAGE
** 
** FUTURE DIRECTIONS
** 
** BUGS
**
*/

#include "ipw.h"
#include "las.h"

void
copyimg (lasimage, fdo, ddr, pixsiz, raw)
	char	       *lasimage;	/* name of LAS image file	 */
	int             fdo;		/* output image file descriptor	 */
	struct DDR	ddr;		/* DDR of las image		 */
	int		pixsiz;		/* LAS pixel size in bytes	 */
	bool_t		raw;		/* flag for raw mode		 */
{
	int		fdi;		/* input image file descriptor	 */
	long		bands;		/* read all bands flag		 */
	long		sl;		/* start line			 */
	long		ss;		/* start sample			 */
	long		nlines;		/* # lines returned by eopenr()	 */
	long		nsamps;		/* # samps returned by eopenr()	 */
	long		nbands;		/* # bands returned by eopenr()	 */
	long		acc;		/* file access code		 */
	long		dtype;		/* data type			 */
	long		opt;		/* opt parms: not used		 */
	long		bufsiz;		/* size of input buffer		 */
	long		line;		/* line loop counter		 */
	long		band;		/* band loop counter		 */
	long		oneline;	/* one line in buffer flag	 */
	unsigned char  *ibuf;		/* input buffer			 */
	pixel_t        *obuf;		/* output buffer		 */
	fpixel_t       *fobuf;		/* floating point output buffer	 */



   /* open LAS image file */

	sl = 1;
	ss = 1;
	nlines = 0;
	nsamps = 0;
	acc = IREAD;
	dtype = ddr.dtype;
	opt = 0;
	oneline = 1;
	nbands = 0;
	if (c_eopenr (&fdi, lasimage, &nbands, &sl, &ss, &nlines, &nsamps, &acc,
		    &dtype, &opt, &oneline) != E_SUCC) {
		error ("error opening %s", lasimage);
	}

   /* make sure DDR and image agree */

	if ((nlines != ddr.nl) || (nsamps != ddr.ns)) {
		error ("image and DDR do not agree on #lines and/or #samples");
	}
	if (nbands != ddr.nbands) {
		error ("image and DDR do not agree on #bands");
	}

   /* allocate input line buffer */

	ibuf = (unsigned char *) ecalloc (nbands * nsamps, pixsiz);
	if (ibuf == NULL) {
		error ("error allocating input buffer");
	}

   /* allocate output buffer */

	if (!raw) {
		fobuf = (fpixel_t *) ecalloc (nsamps * nbands, sizeof(fpixel_t));
		if (fobuf == NULL) {
			error ("error allocating output buffer");
		}
	} else {
		obuf = (pixel_t *) ecalloc (nsamps * nbands, sizeof(pixel_t));
		if (obuf == NULL) {
			error ("error allocating output buffer");
		}
	}

   /* loop on lines reading LAS file and writing to IPW image */

	for (line = 1; line <= nlines; line++) {

		/* read and copy all bands into output buffer */

		for (band = 1; band <= nbands; band++) {

			/* read next line of LAS image */

			if (c_eread (&fdi, &band, &line, ibuf, &oneline)
			    != nsamps) {
				error ("error reading LAS image, line %d", line);
			}

			/* copy to output buffer */

			switch (ddr.dtype) {
			   case EBYTE:
				copy_byte (ibuf, band-1, nbands, nsamps,
					   raw, fobuf, obuf);
				break;
		    	   case EWORD:
				copy_short (ibuf, band-1, nbands, nsamps,
					    raw, fobuf, obuf);
				break;
		    	   case ELONG:
				copy_long (ibuf, band-1, nbands, nsamps,
					   raw, fobuf, obuf);
				break;
		    	   case EREAL:
				copy_float (ibuf, band-1, nbands, nsamps,
					    raw, fobuf, obuf);
				break;
			}
		}

		/* write output buffer to IPW file */

		if (!raw) {
			if (fpvwrite (fdo, fobuf, nsamps) != nsamps) {
				error ("error writing output buffer, line %d",
				       line);
			}
		} else {
			if (pvwrite (fdo, obuf, nsamps) != nsamps) {
				error ("error writing output buffer, line %d",
				       line);
			}
		}
	}
}
