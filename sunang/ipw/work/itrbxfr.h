
/*
** SCCS version: @(#)   itrbxfr.h   4.5   10/19/90
**
** trbxfr.h:  Header file for trbxfr model
**/

#ifndef TRBXFR_H

#define TRBXFR_H

#include "lqh.h"

#define IBANDS		5	/* # bands in input image	*/
#define OBANDS 		3	/* # bands in output image	*/

#define HRS_2_SEC	3600.0
#define NO_DATA		-9999999.0	/* no data value - lower than mins */

#define Z_DEFAULT	3.0		/* default (meters) for upper height */

#ifdef BIH_H
extern int	EXFUN(bihvalid, (BIH_T **bih, int nb));
#endif
extern int	EXFUN(get_sample, (int fdi, fpixel_t *inbuf, pixel_t *mbuf,
			int samp, int *buf_index, double *vector, int nbands));
extern void	EXFUN(headers, (int fdi, int fdm, int fdo, int ibands,
			int obands));
extern int	EXFUN(hle1, (double press, double ta, double ts, double za,
			double ea, double es, double zq, double u,
			double zu, double z0,
			double *h, double *le, double *e));
extern void	EXFUN(itrbxfr, (int fdi, int fdm, int fdo, double delta_t,
			double z));
extern void	EXFUN(newlqh, (int fdo, double h_min, double h_max,
			double le_min, double le_max, double mm_min,
			double mm_max));
extern void	EXFUN(output, (char *tempfile, int fdo));

#endif
