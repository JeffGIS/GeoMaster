
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   headers.c   1.4   6/17/91";
#endif

/*
** NAME
** 	headers -- read/write headers for las2ipw program
** 
** SYNOPSIS
**	#include "las.h"
**
**	headers (lasimage, fdo, fdh, ddr, override_geo, raw);
**	char *lasimage;
** 	int fdo;
**	int fdh;
**	struct DDR ddr;
**	bool_t override_geo;
**	bool_t raw;
** 
** DESCRIPTION
** 	Headers writes the output basic image header for the output image
**	of the las2ipw program.
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
#include "proj.h"
#include "bih.h"
#include "lqh.h"
#include "geoh.h"
#include "las.h"

void
headers (lasimage, fdo, fdh, ddr, override_geo, raw)
	char	       *lasimage;	/* name of LAS image		 */
	int             fdo;		/* output image file descriptor	 */
	int		fdh;		/* lq file descriptor		 */
	struct DDR	ddr;		/* DDR of input LAS image	 */
	bool_t		override_geo;	/* flag to use header geo area	 */
	bool_t		raw;		/* flag for raw mode		 */
{
	int		band;		/* band loop counter		 */
	BIH_T         **i_bihpp;	/* -> header file BIH array	 */
	BIH_T         **o_bihpp;	/* -> output BIH array		 */
	GEOH_T        **i_geohpp;	/* -> header file GEOH array	 */
	GEOH_T        **o_geohpp;	/* -> output GEOH array		 */
	LQH_T         **i_lqhpp;	/* -> header file LQH            */
	LQH_T         **o_lqhpp;	/* -> output LQH array		 */
	double          bline;		/* GEOH coords for first line	 */
	double          bsamp;		/* GEOH coords for first sample	 */
	double          dline;		/* spacing between lines	 */
	double          dsamp;		/* spacing between samples	 */
	double	       *minval;		/* min value of each band	 */
	double	       *maxval;		/* max value of each band	 */
	long		window[4];	/* window for mins/maxs		 */
	long	       *bands;		/* bands for min/max calculation */
	long		oneimg;		/* one image for minmax()	 */
	int	       *nbits;		/* # bits per band		 */
	pixel_t		ibkpt[2];	/* integer breakpoints for LQ	 */
	fpixel_t	fbkpt[2];	/* floating breakpoints for LQ	 */
	char		proj[80];	/* projection name string	 */
	char	       *punits;		/* -> name of projection units	 */
	char		zone_label[80]; /* buffer for UTM zone label	 */

	static GETHDR_T h_lqh = {LQH_HNAME, (ingest_t) lqhread};
	static GETHDR_T h_geoh = {GEOH_HNAME, (ingest_t) geohread};
	static GETHDR_T *request[] = {&h_lqh, &h_geoh, 0};


   /* allocate arrays for retrieval or calculation of mins/maxs */

	minval = (double *) ecalloc (ddr.nbands, sizeof(double));
	maxval = (double *) ecalloc (ddr.nbands, sizeof(double));
	bands = (long *) ecalloc (ddr.nbands, sizeof(long));
	for (band = 0; band < ddr.nbands; band++) {
		bands[band] = band + 1;
	}
	window[0] = 1;
	window[1] = 1;
	window[2] = ddr.nl;
	window[3] = ddr.ns;

   /* retrieve or calculate min and max of each band in LAS image */

	oneimg = 1;
	if (c_minmax (lasimage, &oneimg, window, bands, &ddr.nbands,
		      minval, maxval)
		!= E_SUCC) {
		error ("error retrieving/calculating LAS image min/max values");
	}

   /* no negative values if raw mode */

	if (raw) {
		for (band = 0; band < ddr.nbands; band++) {
			if (minval[band] < 0) {
				error ("band %d contains negative values; can't use raw mode",
					band);
			}
		}
	}

   /* allocate array to hold # bits/band */

	nbits = (int *) ecalloc (ddr.nbands, sizeof(int));

   /* read BIH, LQH and GEOH from supplied header file, if there is one */
   /* duplicate BIH for output image */

	if (fdh != ERROR) {
		i_bihpp = bihread (fdh);
		if (i_bihpp == NULL) {
			error ("can't read BIH of supplied LQH");
		}

		if (bih_nbands (i_bihpp[0]) != ddr.nbands)
			error ("input header has different # bands than LAS image");

		if ((bih_nlines (i_bihpp[0]) != ddr.nl) ||
		    (bih_nsamps (i_bihpp[0]) != ddr.ns))
			error ("header file has different dimensions than LAS image");

		/* copy # bits per band from BIH */
		/* if raw mode, make sure header has enought bits */

		for (band = 0; band < ddr.nbands; band++) {
			nbits[band] = bih_nbits(i_bihpp[band]);
			if (raw) {
				if (nbits[band] < hbit ((int) maxval[band])) {
					error ("not enough bits in band %d of input header",
					band);
				}
			}
		}

		gethdrs (fdh, request, ddr.nbands, fdo);
		i_lqhpp = (LQH_T **) hdr_addr(h_lqh);
		i_geohpp = (GEOH_T **) hdr_addr(h_geoh);
		o_bihpp = bihdup (i_bihpp);

   /* Create BIH for output image */

	} else {
		i_bihpp = (BIH_T **) hdralloc (ddr.nbands, sizeof(BIH_T *), fdo,
					BIH_HNAME);
		if (i_bihpp == NULL) {
			error ("Can't allocate new BIH");
		}

		for (band = 0; band < ddr.nbands; band++) {
			if (ddr.dtype == EREAL) {
				nbits[band] = CHAR_BIT;
			} else if (raw) {
				nbits[band] = hbit ((int) maxval[band]);
			} else {
				nbits[band] = hbit ((int) (maxval[band] - 
						          minval[band]));
			}
			i_bihpp[band] = bihmake (0, nbits[band],
					(STRVEC_T *) NULL, NULL,
			   	        i_bihpp[0], ddr.nl, ddr.ns, ddr.nbands);
			if (i_bihpp[band] == NULL) {
				error ("Can't make new BIH");
			}
		}
	}

   /* write BIH to output image */

	if (bihwrite (fdo, i_bihpp) == ERROR) {
		error ("cannot write BIH to output image");
	}

	
   /* create geodetic header or duplicate GEO header from header file */

	bline = ddr.upleft[0];
	bsamp = ddr.upleft[1];
	dline = -ddr.pdist_y;
	dsamp = ddr.pdist_x;
	if (!override_geo) {

		if (ddr.valid[0] != VALID || ddr.valid[4] != VALID ||
		    ddr.valid[5] != VALID || ddr.valid[6] != VALID)
			warn ("LAS geographic information is not valid");

		if (fdh != ERROR && i_geohpp != NULL) {
			if ((geoh_bline (i_geohpp[0]) != bline) ||
			    (geoh_bsamp (i_geohpp[0]) != bsamp) ||
			    (geoh_dline (i_geohpp[0]) != dline) ||
			    (geoh_dsamp (i_geohpp[0]) != dsamp)) {
				warn ("Geographic region and/or resolution of header file and LAS image\ndo not match - using LAS values");
			}
		}
		c_prostr (&ddr.proj_code, proj);
		punits = ddr.proj_units;

		if (ddr.proj_code == UTM) {
			sprintf (zone_label, "%s zone %d", proj, ddr.zone_code);
			strcpy (proj, zone_label);
		}

		o_geohpp = (GEOH_T **) hdralloc (ddr.nbands, sizeof(GEOH_T *), fdo,
						GEOH_HNAME);
		o_geohpp[0] = geohmake (bline, bsamp, dline, dsamp,
				punits, proj);
		for (band = 1; band < ddr.nbands; band++) {
			o_geohpp[band] = o_geohpp[0];
		}

	} else {
		if (i_geohpp == NULL) {
			error ("header file contains no geo header");
		}
		o_geohpp = geohdup (i_geohpp, ddr.nbands);
	}

   /* write GEO header to IPW file */

	if (geohwrite (fdo, o_geohpp) == ERROR) {
		error("can't write GEO header");
	}

   /* write input linear quantization header, if provided */

	if (fdh != ERROR && i_lqhpp != NULL) {
		if ((o_lqhpp = lqhdup(i_lqhpp, ddr.nbands)) == NULL) {
			error ("can't duplicate input LQH");
		}

   /* otherwise create LQH header */

	} else {

		o_lqhpp = (LQH_T **) hdralloc (ddr.nbands, sizeof(LQH_T *),
					     fdo, LQH_HNAME);
		if (o_lqhpp == NULL) {
			error ("can't allocate LQH header");
		}

   		/* create LQH for each band */

		ibkpt[0] = 0;
		for (band = 0; band < ddr.nbands; band++) {
			ibkpt[1] = ipow2 (nbits[band]) - 1;
			if (ddr.dtype == EREAL) {
				fbkpt[0] = minval[band];
				fbkpt[1] = maxval[band];
			} else if (raw) {
				fbkpt[0] = 0.0;
				fbkpt[1] = ipow2 (nbits[band]) - 1;
			} else {
				fbkpt[0] = minval[band];
				fbkpt[1] = ipow2 (nbits[band]) + fbkpt[0] - 1;
			}
			o_lqhpp[band] = lqhmake (nbits[band], 2,
				ibkpt, fbkpt, (char *) NULL, (char *) NULL);
			if (o_lqhpp[band] == NULL) {
				error ("can't make LQH for band %d", band);
			}
		}
	}

   /* write LQH */

	if (lqhwrite (fdo, o_lqhpp) == ERROR) {
		error ("can't write LQH header");
	}

   /* done - get ready for image output */

	if (boimage (fdo) == ERROR) {
		error ("can't terminate header output");
	}
}
