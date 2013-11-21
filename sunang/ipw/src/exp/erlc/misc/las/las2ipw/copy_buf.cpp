
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   copy_buf.cpp   1.2   7/1/91";
#endif

/*
** NAME
** 	copy_byte, copy_short, copy_long, copy_real -- copy the data from the
**		given band into the output buffer
** 
** SYNOPSIS
**	copy_byte  (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
**	unsigned char *obuf;
**
**	copy_short (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
**	unsigned short *obuf;
**
**	copy_long  (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
**	unsigned long *obuf;
**
**	copy_float (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
**	float *obuf;
**
**	unsigned char *ibuf;
**	long band, nbands;
**	long nsamps;
**	bool_t raw;
**	fpixel_t *fobuf;
**	pixel_t *obuf;
** 
** DESCRIPTION
**	The routines copy_(*) copy one line of the given band from the
**	LAS input buffer to the IPW output buffer.
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

#ifdef TYPE_BYTE
void copy_byte (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
	unsigned char  *ibuf;		/* input buffer			 */
#endif

#ifdef TYPE_SHORT
void copy_short (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
	unsigned short *ibuf;		/* input buffer			 */
#endif

#ifdef TYPE_LONG
void copy_long (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
	unsigned long  *ibuf;		/* input buffer			 */
#endif

#ifdef TYPE_FLOAT
void copy_float (ibuf, band, nbands, nsamps, raw, fobuf, obuf)
	float	       *ibuf;		/* input buffer			 */
#endif

	long		band;		/* band #			 */
	long		nbands;		/* # bands in image		 */
	long		nsamps;		/* # samples per line		 */
	bool_t		raw;		/* flag for raw mode		 */
	fpixel_t       *fobuf;		/* output buffer		 */
	pixel_t        *obuf;		/* raw output buffer		 */
{
	long		samp;		/* sample loop index		 */

#ifdef TYPE_BYTE
	register unsigned char  *ibufp;	/* -> in output buffer		 */
#endif

#ifdef TYPE_SHORT
	register unsigned short *ibufp;	/* -> in output buffer		 */
#endif

#ifdef TYPE_LONG
	register unsigned long  *ibufp;	/* -> in output buffer		 */
#endif

#ifdef TYPE_FLOAT
	register float          *ibufp;	/* -> in output buffer		 */
#endif

	register pixel_t        *obufp;	/* -> in output buffer		 */
	register fpixel_t       *fobufp;/* -> in output buffer		 */


   /* loop on samples copying data */

	if (!raw) {
		fobufp = &fobuf[band];
		ibufp = ibuf;
		for (samp = 0; samp < nsamps; samp++) {
			*fobufp = *ibufp++;
			fobufp += nbands;
		}
	} else {
		obufp = &obuf[band];
		ibufp = ibuf;
		for (samp = 0; samp < nsamps; samp++) {
			*obufp = *ibufp++;
			obufp += nbands;
		}
	}
}
