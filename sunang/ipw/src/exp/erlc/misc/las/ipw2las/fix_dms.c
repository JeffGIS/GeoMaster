
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   fix_dms.c   1.1   10/1/91";
#endif

/*
** NAME
** 	fix_dms - convert DMS format to old GCTP version for LAS
** 
** SYNOPSIS
**	void
**	fix_dms (proj, inparms, outparms)
**	int proj;
**	double *inparms, *outparms;
** 
** DESCRIPTION
**	fix_dms copies the 15 word projection parameter array and
**	converts the packed DMS (degree-minute-second) from the new
**	GCTP II version in the projdefs file (+/-DDDMMSS.SSS) to the
**	old GCTP format that LAS uses (+/-DDDMMMSSS.SSS).
**	Note: this routine is a hack.  If more projections are added or
**	the format of the GCTP projection definition parameters is 
**	changed, this routine will have to be rewritten.
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

#include "mproj.h"

fix_dms (proj, inparms, outparms)
	int		proj;		/* projection identifier	 */
	double	       *inparms;	/* input projection parameters	 */
	double	       *outparms;	/* output projection parameters	 */
{
	int		i;		/* loop counter			 */

	double		las_dms();

   /* Copy projection parameters */

	for (i = 0; i < NPARMS; i++)
		outparms[i] = inparms[i];


   /* Use projection ID to determine which parameters are packed DMS */
   /* Convert packed DMS to decimal degress, then packed DMS old format */

	switch (proj) {

		case UTM:
			outparms[0] = las_dms (unpack_dms (inparms[0]));
			outparms[1] = las_dms (unpack_dms (inparms[1]));
			break;

		case ALBERS:
		case LAMCC:
		case EQUIDC:
			outparms[2] = las_dms (unpack_dms (inparms[2]));
			outparms[3] = las_dms (unpack_dms (inparms[3]));
			outparms[4] = las_dms (unpack_dms (inparms[4]));
			outparms[5] = las_dms (unpack_dms (inparms[5]));
			break;

		case MERCAT:
		case PS:
		case POLYC:
		case TM:
		case STEREO:
		case LAMAZ:
		case AZMEQD:
		case GNOMON:
		case ORTHO:
		case GVNSP:
		case EQRECT:
		case VGRINT:
			outparms[4] = las_dms (unpack_dms (inparms[4]));
			outparms[5] = las_dms (unpack_dms (inparms[5]));
			break;

		case SNSOID:
		case MILLER:
		case SOM:
			outparms[4] = las_dms (unpack_dms (inparms[4]));
			break;

		case HOM:
			outparms[5] = las_dms (unpack_dms (inparms[5]));

			/* format A */
			if (inparms[3] == 0.0) {
				outparms[8] = las_dms (unpack_dms (inparms[8]));
				outparms[9] = las_dms (unpack_dms (inparms[9]));
				outparms[10] = las_dms(unpack_dms(inparms[10]));
				outparms[11] = las_dms(unpack_dms(inparms[11]));
				break;

			/* format B */
			} else {
				outparms[4] = las_dms (unpack_dms (inparms[4]));
			}
	}
}
