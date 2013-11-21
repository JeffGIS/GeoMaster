#ifndef	SKEWH_H
#define	SKEWH_H

/*
** NAME
**	skewh.h -- header file for SKEWH header
**
** SYNOPSIS
**	#include "skew.h"
**
** DESCRIPTION
**	The presence of as skew header indicates that an image has been skewed
**	horizontally by the skew program.  The skew angle in the header is
**	used by skew to undo the skewing.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

typedef struct {
	double          angle;		/* skew angle (degrees)		 */
} SKEWH_T;

#define	SKEWH_HNAME	"skew"		/* header name within IPW	 */
#define	SKEWH_VERSION	"$Revision: 1.3 $"	/* RCS revsion #	 */

/* field keywords */
#define	SKEWH_ANGLE		"angle"

/* field access macros */
#define	skewh_angle(p)	( (p)->angle )

/* misc. */
#define	SKEW_MAX	45.0		/* maximum skew angle		 */
#define	SKEW_MIN	(-45.0)		/* minimum skew angle		 */

/* function declarations */
extern bool_t     EXFUN(skewhcheck, (SKEWH_T **skewhpp, int nbands));
extern SKEWH_T  * EXFUN(skewhmake, (double angle));
extern SKEWH_T ** EXFUN(skewhread, (int fd));
extern int        EXFUN(skewhwrite, (int fd, SKEWH_T **skewhpp));

/* $Header: skewh.h,v 1.3 90/11/11 17:00:16 frew Exp $ */
#endif
