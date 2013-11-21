
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   out_buf.cpp   1.1   5/14/91";
#endif

/*
** NAME
**	out_byte, out_short, out_long, out_float - unpack bands of input
**		line and write to output image
** 
** SYNOPSIS
**	out_byte  (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf);
**	out_short (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf);
**	out_long  (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf);
**	out_float (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf);
**
**	int *fdo, fdm;
**	fpixel_t *ibuf;
**	int nsamps, nbands;
**	int *line;
**	unsigned char *obuf;	(for out_byte)
**	short *obuf;		(for out_short)
**	long *obuf;		(for out_long)
**	float *obuf;		(for out_float)
** 
** DESCRIPTION
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

#ifdef TYPE_BYTE
out_byte (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf)
	unsigned char  *obuf;		/* output buffer		 */
#endif

#ifdef TYPE_SHORT
out_short (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf)
	short	       *obuf;		/* output buffer		 */
#endif

#ifdef TYPE_LONG
out_long (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf)
	long	       *obuf;		/* output buffer	 	 */
#endif

#ifdef TYPE_FLOAT
out_float (fdo, fdm, ibuf, mbuf, nsamps, nbands, line, obuf)
	float	       *obuf;		/* output buffer	 	 */
#endif

	int	       *fdo;		/* -> output file descriptor	 */
	int		fdm;		/* mask image file descriptor	 */
	fpixel_t       *ibuf;		/* input line buffer	 	 */
	pixel_t        *mbuf;		/* mask line buffer	 	 */
	int		nsamps;		/* # samps in input image	 */
	int		nbands;		/* # bands in input image	 */
	int		line;		/* current line index		 */
{
#ifdef TYPE_BYTE
	REG_1 u_char   *obufp;		/* -> in output buffer		 */
#endif

#ifdef TYPE_SHORT
	REG_1 short    *obufp;		/* -> in output buffer		 */
#endif

#ifdef TYPE_LONG
	REG_1 long     *obufp;		/* -> in output buffer		 */
#endif

#ifdef TYPE_FLOAT
	REG_1 float    *obufp;		/* -> in output buffer		 */
#endif

	REG_2 fpixel_t *ibufp;		/* -> in input buffer		 */
	REG_3 pixel_t  *mbufp;		/* -> in mask buffer		 */
	REG_4 int	band;		/* band loop counter		 */
	REG_5 int	samp;		/* sample loop counter		 */
	long		eline;		/* LAS line index (from 1)	 */
	long		eband;		/* LAS band number (from 1)	 */
	long		nl_blk;		/* # lines to write (1)		 */


	eline = line + 1;
	eband = 1;
	nl_blk = 1;

   /* loop on bands filling output buffer and writing line to output file */

	for (band = 0; band < nbands; band++) {

		ibufp = &ibuf[band];
		mbufp = mbuf;
		obufp = obuf;

		for (samp = 0; samp < nsamps; samp++) {

			if (fdm == ERROR || *mbufp++) {
				*obufp++ = *ibufp;
			} else {
				*obufp++ = 0;
			}
			ibufp += nbands;
		}

		if (ewrite (fdo, &eband, &eline, obuf, &nl_blk) == E_FAIL) {
			error ("error writing output image, line %d", line);
		}

		eband++;
	}
}
