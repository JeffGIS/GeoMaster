#ifndef	HOR_H
#define	HOR_H

/*
 * horizon header
 */

typedef struct {
	double          azimuth;	/* direction toward horizons	 */
} HORH_T;

#define	HORH_HNAME	"hor"		/* header name within IPW	 */
#define	HORH_VERSION	"$Revision: 1.3 $"	/* RCS revsion #	 */

/* field keywords */
#define	HORH_AZM	"azimuth"

/* field access macros */
#define	horh_azm(p)	( (p)->azimuth )

/* function declarations */
extern bool_t    EXFUN(horhcheck, (HORH_T **horhpp, int nbands));
extern HORH_T  * EXFUN(horhmake, (double azm));
extern HORH_T ** EXFUN(horhread, (int fd));
extern int       EXFUN(horhwrite, (int fd, HORH_T **horhpp));

/* $Header: horh.h,v 1.2 88/03/06 21:43:37 dozier Exp $ */
#endif
