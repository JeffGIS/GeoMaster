/*
**	Calculate radiation values for this input line.
**	(See toporad.h for the order of the values in the input line.)
*/

#include "ipw.h"
#include "fpio.h"
#include "toporad.h"

int
radcalc(in_buf, n, net, mu0, out_buf)
	fpixel_t       *in_buf;		/* -> input buffer		 */
	int             n;		/* number of pixels		 */
	bool_t          net;		/* ? net radiation		 */
	fpixel_t        mu0;		/* cosine of solar zenith angle	 */
	fpixel_t       *out_buf;	/* -> output buffer		 */
{
	fpixel_t        brad;		/* beam radiation		 */
	fpixel_t        rad;		/* total radiation		 */
	fpixel_t        drad;		/* diffuse radiation		 */

 /*
  * check for errors in input
  */
	if (in_buf == NULL) {
		usrerr("radcalc: input buffer NULL");
		return (ERROR);
	}
	if (out_buf == NULL) {
		usrerr("radcalc: output buffer NULL");
		return (ERROR);
	}
	if (n <= 0) {
		usrerr("radcalc: length of buffers = %d", n);
		return (ERROR);
	}
	if (mu0 < 0 || mu0 > 1) {
		usrerr("cosine of solar angle = %g", mu0);
		return (ERROR);
	}

	while (--n >= 0) {
 /*
  * beam radiation in band BEAM_BAND
  */
		brad = rad = in_buf[BEAM_BAND];
 /*
  * multiply by cosine of local illumination angle, which is in MU_BAND
  */
		rad *= in_buf[MU_BAND];
 /*
  * diffuse radiation in DIFFUSE_BAND
  */
		drad = in_buf[DIFFUSE_BAND];
 /*
  * add diffuse radiation, accounting for sky view factor
  */
		rad += drad * in_buf[VF_BAND];
#if 0 /* >>>>>replaces #ifdef 0 that wouldn't compile<<<<<< */
 /*
  * add reflection from adjacent terrain
  */
		rad += in_buf[CT_BAND] * in_buf[ALB_BAND] *
			(drad * (1 - in_buf[VF_BAND]) + brad * mu0);
#endif
 /*
  * net radiation, if desired
  */
		if (net) {
			rad *= 1 - in_buf[ALB_BAND];
		}
 /*
  * increment pointers for next pixel
  */
		*out_buf++ = rad;
		in_buf += NBANDS;
	}
	return (OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/dozier/ipw/src/bin/toporad/RCS/radcalc.c,v 1.1 89/07/05 13:25:25 dozier Exp $";

#endif
