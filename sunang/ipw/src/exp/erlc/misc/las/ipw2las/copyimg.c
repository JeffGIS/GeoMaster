
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   copyimg.c   1.1   5/14/91";
#endif

/*
** NAME
** 	copyimg -- copy IPW image to LAS output image
** 
** SYNOPSIS
**	void
**	copyimg (fdi, fdm, lasname, raw, float_dtype, maxpv)
** 	int fdi, fdm;
**	char *lasname;
**	bool_t raw, float_dtype;
**	float maxpv;
** 
** DESCRIPTION
**	copyimg copies the binary data from the IPW image file to the
**	LAS image file.
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
#include "bih.h"
#include "fpio.h"
#include "las.h"

void
copyimg (fdi, fdm, lasname, raw, float_dtype, maxpv)
	int             fdi;		/* input image file descriptor	 */
	int             fdm;		/* mask image file descriptor	 */
	char	       *lasname;	/* LAS image filename		 */
	bool_t		raw;		/* flag for raw pixel mode	 */
	bool_t		float_dtype;	/* flag for floating point pixels*/
	float		maxpv;		/* max pixel value of all bands	 */
{
	int		fdo;		/* input image file descriptor	 */
	int		nbands;		/* # bands in input image	 */
	int		nlines;		/* # lines in input image	 */
	int		nsamps;		/* # samples in input image	 */
	int		pixsiz;		/* pixel size in bytes		 */
	long		bands;		/* read all bands flag		 */
	long		sl;		/* start line			 */
	long		ss;		/* start sample			 */
	long		acc;		/* file access code		 */
	long		dtype;		/* data type			 */
	long		nbuf;		/* # lines to remain in buffer	 */
	long		opt;		/* opt parms: not used		 */
	long		bufsiz;		/* size of input buffer		 */
	long		nbytes;		/* # bytes in output buffer	 */
	long		nline;		/* # lines to write at a time	 */
	long		line;		/* line loop counter		 */
	fpixel_t       *ibuf;		/* input line buffer		 */
	pixel_t        *mbuf;		/* mask line buffer		 */
	unsigned char  *obuf;		/* output line buffer		 */


	nbands = hnbands (fdi);
	nlines = hnlines (fdi);
	nsamps = hnsamps (fdi);

   /* determine data type and pixel size for LAS image */

	if (float_dtype) {
		dtype = EREAL;
		pixsiz = sizeof (float);
	} else {
		if (maxpv <= UCHAR_MAX) {
			dtype = EBYTE;
			pixsiz = 1;
		} else if (maxpv <= SHRT_MAX) {
			dtype = EWORD;
			pixsiz = 2;
		} else if (maxpv <= LONG_MAX) {
			dtype = ELONG;
			pixsiz = 4;
		} else {
			error ("max value in input image is too large to integerize; use -f option");
		}
	}

   /* open LAS image file for random access */

	sl = 1;
	ss = 1;
	acc = IWRITE;
	nbuf = 0;
	opt = 0;
	nline = 1;
	if (c_eopenr (&fdo, lasname, &nbands, &sl, &ss, &nlines, &nsamps, &acc,
		    &dtype, &opt, &nline) != E_SUCC) {
		error ("error opening %s", lasname);
	}

   /* allocate input line buffer */

	ibuf = (fpixel_t *) ecalloc (nsamps * nbands, sizeof(fpixel_t));
	if (ibuf == NULL) {
		error ("error allocating input buffer");
	}

   /* allocate output line buffer */

	obuf = (unsigned char *) ecalloc (nsamps, pixsiz);
	if (obuf == NULL) {
		error ("error allocating output buffer");
	}

   /* allocate mask line buffer */

	if (fdm != ERROR) {
		mbuf = (pixel_t *) ecalloc (nsamps, sizeof(pixel_t));
		if (mbuf == NULL) {
			error ("error allocating mask buffer");
		}
	}

   /* loop on lines reading IPW image, unpacking bands and writing to LAS image */

	for (line = 0; line < nlines; line++) {

		/* read line of IPW image */

		if (fpvread (fdi, ibuf, nsamps) != nsamps) {
			error ("error reading input image, line %d", line);
		}

		/* read line of mask image */

		if (fdm != ERROR) {
			if (pvread (fdm, mbuf, nsamps) != nsamps) {
				error ("error reading mask image, line %d", line);
			}
		}

		/* unpack each band and write to LAS file */

		switch (dtype) {
		   case EBYTE:
			out_byte (&fdo, fdm, ibuf, mbuf, nsamps, nbands, line,
				  obuf);
			break;
		   case EWORD:
			out_short (&fdo, fdm, ibuf, mbuf, nsamps, nbands, line,
				   obuf);
			break;
		   case ELONG:
			out_long (&fdo, fdm, ibuf, mbuf, nsamps, nbands, line,
				  obuf);
			break;
		   case EREAL:
			out_float (&fdo, fdm, ibuf, mbuf, nsamps, nbands, line,
				   obuf);
			break;
		}
	}


   /* close LAS image */

	eclose (&fdo);
}
