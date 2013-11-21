
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   update_ddr.c   1.4   10/1/91";
#endif

/*
** NAME
** 	update_ddr -- update DDR with geographic information
** 
** SYNOPSIS
**	void
**	update_ddr (lasprefix, bline, bsamp, dline, dsamp, got_geo, got_proj, prj)
**	char *lasprefix;
**	double bline, bsamp, dline, dsamp;
**	int got_geo, got_proj;
**	struct projdef *prj;
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

#include "las.h"
#include "mproj.h"

void
update_ddr (lasprefix, bline, bsamp, dline, dsamp, got_geo, got_proj, prj)
	char	       *lasprefix;	/* LAS image name prefix	 */
	double		bline;		/* geo x-coord of UL corner	 */
	double		bsamp;		/* geo y-coord of UL corner	 */
	double		dline;		/* geo x-increment		 */
	double		dsamp;		/* geo y-increment		 */
	int		got_geo;	/* flag if geo header read	 */
	int		got_proj;	/* flag if proj parms given	 */
	struct projdef *prj;		/* projection parameters	 */
{
	int		i;		/* loop counter 		 */
	int		band;		/* band loop counter 		 */
	double		eline;		/* y-coord at LR corner		 */
	double		esamp;		/* x-coord at LR corner		 */
	struct DDR	ddr;		/* DDR struct of output image	 */
	struct BDDR    *bddr;		/* band-dep BDDR of output image */


   /* read DDR (previously written by eopen() */

	if (c_getddr (lasprefix, &ddr) != E_SUCC) {
		error ("error reading DDR file %s.ddr", lasprefix);
	}

   /* allocate space for band-dependent records of DDR */

	bddr = (struct BDDR *) ecalloc (ddr.nbands, sizeof(struct BDDR));

   /* read in band-dependent records of DDR */

	for (band = 1; band <= ddr.nbands; band++) {
		if (c_getbdr (lasprefix, &bddr[band-1], &band) != E_SUCC) {
			error ("error reading DDR for band %d", band);
		}
	}

   /* write projection info, if provided; set validity flags */

	if (got_proj) {
		ddr.proj_code = prj->id;
		ddr.zone_code = prj->zone;
		ddr.datum_code = prj->datum;
		switch (prj->uid) {
			case 0:
				strcpy (ddr.proj_units, "radians");
				break;
			case 1:
				strcpy (ddr.proj_units, "feet");
				break;
			case 2:
				strcpy (ddr.proj_units, "meters");
				break;
			case 3:
				strcpy (ddr.proj_units, "seconds");
				break;
			case 4:
				strcpy (ddr.proj_units, "degrees");
				break;
			case 6:
				strcpy (ddr.proj_units, "dms");
				break;
		}

		/* Copy proj parms, fixing incompatible DMS formats */

		fix_dms (prj->id, prj->parms, ddr.proj_coef);

		ddr.valid[0] = VALID;
		ddr.valid[1] = VALID;
		ddr.valid[2] = VALID;
		ddr.valid[3] = VALID;
		ddr.valid[4] = VALID;
		ddr.valid[5] = VALID;
	} else {
		ddr.valid[0] = INVAL;
		ddr.valid[1] = INVAL;
		ddr.valid[2] = INVAL;
		ddr.valid[3] = INVAL;
		ddr.valid[4] = INVAL;
		ddr.valid[5] = INVAL;
	}

   /* copy corners if provided */

	if (got_geo) {
		eline = bline + (ddr.nl - 1) * dline;
		esamp = bsamp + (ddr.ns - 1) * dsamp;
		ddr.pdist_y = -dline;
		ddr.pdist_x = dsamp;
		ddr.upleft[0] = bline;
		ddr.upleft[1] = bsamp;
		ddr.loleft[0] = eline;
		ddr.loleft[1] = bsamp;
		ddr.upright[0] = bline;
		ddr.upright[1] = esamp;
		ddr.loright[0] = eline;
		ddr.loright[1] = esamp;
		ddr.valid[6] = VALID;
		ddr.valid[7] = VALID;
	} else {
		ddr.valid[6] = INVAL;
		ddr.valid[7] = INVAL;
	}

   /* write updated DDR */

	if (c_putddr (lasprefix, &ddr) != E_SUCC) {
		error ("error writing DDR");
	}
	for (band = 0; band < ddr.nbands; band++) {
		if (c_putbdr (lasprefix, &bddr[band]) != E_SUCC) {
			error ("error writing BDDR for band %d", band+1);
		}
	}
}
