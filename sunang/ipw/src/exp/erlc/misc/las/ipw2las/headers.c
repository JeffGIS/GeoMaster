
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   headers.c   1.1   5/14/91";
#endif

/*
** NAME
** 	headers -- read headers of the IPW image file
** 
** SYNOPSIS
**	headers (fdi, fdm, raw, maxpv, got_geo, bline, bsamp, dline, dsamp)
**	int fdi, fdm;
**	bool_t raw;
**	float *maxpv;
**	bool_t *got_geo;
**	double *bline, *bsamp, *dline, *dsamp;
** 
** DESCRIPTION
** 	Headers reads the headers of the input IPW images and returns the
**	contents of the basic image and geo headers in the given variables.
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
#include "gethdrs.h"
#include "bih.h"
#include "lqh.h"
#include "geoh.h"
#include "fpio.h"
#include "las.h"

void
headers (fdi, fdm, raw, maxpv, got_geo, bline, bsamp, dline, dsamp)
	int             fdi;		/* input image file descriptor	 */
	int             fdm;		/* mask image file descriptor	 */
	bool_t		raw;		/* flag for raw mode		 */
	float	       *maxpv;		/* max pixel value of all bands  */
	bool_t	       *got_geo;	/* flag if geo header read	 */
	double	       *bline;		/* geo y-coord of UL of image	 */
	double	       *bsamp;		/* geo x-coord of UL of image	 */
	double	       *dline;		/* pixel size in y-direction	 */
	double	       *dsamp;		/* pixel size in x-direction	 */
{
	BIH_T         **i_bihpp;	/* -> input file BIH array	 */
	BIH_T         **m_bihpp;	/* -> mask file BIH array	 */
	GEOH_T        **i_geohpp;	/* -> input file GEOH array	 */
	LQH_T         **i_lqhpp;	/* -> input file LQH            */
	float          *fpmax;		/* F.P. maxs of all bands	 */
	int		nbands;		/* # bands in input image	 */
	int		band;		/* band loop counter		 */

	static GETHDR_T h_lqh = {LQH_HNAME, (ingest_t) lqhread};
	static GETHDR_T h_geoh = {GEOH_HNAME, (ingest_t) geohread};
	static GETHDR_T *request[] = {&h_geoh, &h_lqh, 0};


   /* read BIH from input file */

	i_bihpp = bihread (fdi);
	if (i_bihpp == NULL) {
		error ("can't read BIH of input image");
	}

   /* read and check BIH of mask file if provided */

	if (fdm != ERROR) {
		m_bihpp = bihread (fdm);
		if (m_bihpp == NULL) {
			error ("can't read BIH of mask image");
		}
		if (bih_nbands (m_bihpp[0]) > 1) {
			error ("mask image has > 1 bands");
		}
		if (bih_nlines (m_bihpp[0]) != bih_nlines (i_bihpp[0]) ||
		    bih_nsamps (m_bihpp[0]) != bih_nsamps (i_bihpp[0])) {
			error ("mask and input image have different dimensions");
		}
		skiphdrs (fdm);
	}

   /* read LQH and GEOH from input file */

	if (raw) {
		request[1] = 0;
	}
	gethdrs (fdi, request, NO_COPY, ERROR);

   /* set max pixel values of all bands from LQ header or raw pixel values */

	nbands = bih_nbands (i_bihpp[0]);
	if (!raw && (i_lqhpp = (LQH_T **) hdr_addr(h_lqh)) != NULL) {
		fpmax = fpfmax (fdi);
		*maxpv = fpmax[0];
		for (band = 1; band < nbands; band++) {
			*maxpv = MAX (*maxpv, fpmax[band]);
		}
	} else {
		*maxpv = ipow2 (bih_nbits (i_bihpp[0])) - 1;
		for (band = 1; band < nbands; band++) {
			*maxpv = MAX (*maxpv,
				      ipow2 (bih_nbits (i_bihpp[band])) - 1);
		}
	}

   /* set geographic parameters from GEO header */

	if ((i_geohpp = (GEOH_T **) hdr_addr(h_geoh)) == NULL) {
		error ("IPW image has no GEO header - LAS image will have no projection information");
		*got_geo = FALSE;
	} else {
		*bline = geoh_bline (i_geohpp[0]);
		*bsamp = geoh_bsamp (i_geohpp[0]);
		*dline = geoh_dline (i_geohpp[0]);
		*dsamp = geoh_dsamp (i_geohpp[0]);
		*got_geo = TRUE;
	}
}
