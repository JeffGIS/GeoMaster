#ifndef	SUNH_H
#define	SUNH_H

/*
** NAME
**	sunh -- sun geometry header
**
** SYNOPSIS
**	#include "sunh.h"
**
** DESCRIPTION
**
** FUTURE DIRECTIONS
**
** BUGS
*/

typedef struct {
	double          cos_sun;	/* cosine solar zenith ang	 */
	double          zen_sun;	/* solar zenith angle (radians)	 */
	double          azm_sun;	/* solar azimuth (rad from S)	 */
}               SUNH_T;

#define	SUNH_HNAME	"sun"		/* header name within IPW	 */
#define	SUNH_VERSION	"$Revision: 1.1 $"	/* RCS revsion #	 */

/* field keywords */
#define	SUNH_COS	"cos_sun"
#define	SUNH_ZEN	"zen_sun"
#define	SUNH_AZM	"azm_sun"

/* field access macros */
#define sunh_cos(p)	((p)->cos_sun)
#define sunh_zen(p)	((p)->zen_sun)
#define sunh_azm(p)	((p)->azm_sun)

/* function declarations */
extern bool_t    EXFUN(sunhcheck, (SUNH_T **sunhpp, int nbands));
extern SUNH_T  * EXFUN(sunhmake, (double cos_zen, double azm));
extern SUNH_T ** EXFUN(sunhread, (int fd));
extern int       EXFUN(sunhwrite, (int fd, SUNH_T **sunhpp));

/* $Header: sunh.h,v 1.1 88/03/11 17:05:12 dozier Exp $ */
#endif
