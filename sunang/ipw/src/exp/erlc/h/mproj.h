
/*	SCCS version: @(#)   mproj.h   1.2   9/26/91	*/

/*
** NAME
**	mproj.h -- header file for map projection codes
**
** SYNOPSIS
**	#include "mproj.h"
**
** DESCRIPTION
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#ifndef	MPROJ_H
#define	MPROJ_H

#define NPARMS	15		/* # projection parameters	         */
#define NPROJ	22		/* # projection ids		         */
#define NUNITS	 6		/* # valid unit ids		         */
#define NDATUM	20		/* # datum codes		         */
#define NUTMZ	60		/* # UTM zones (negative for S. Hem.)    */
#define MIN_SPZ	101		/* minimum State Plane zone code         */
#define MAX_SPZ	5400		/* maximum State Plane zone code         */
#define MIN_PER 45              /* arbitrary minimum orbit time          */

#define RAD_EARTH   6370997	/* default Earth radius in meters*/
#define UNDEF_ZONE	62	/* zone if undefined		 */
#define MAX_LAT         90      /* maximum latitude 90 degrees   */
#define MAX_LONG       180      /* maximum longitude 180 degrees */
#define MIN_AZIM      -180      /* minimum azimuth angle         */
#define MAX_AZIM       360      /* maximum azimuth angle         */

/* Projection IDs */

#define UNKNOWN -1      /* -1 = unknown projection			 */
#define GEO	 0	/*  0 = geographic				 */
#define UTM	 1	/*  1 = Universal Transverse Mercator (UTM)	 */
#define SPCS	 2	/*  2 = State Plane Coordinates			 */
#define ALBERS	 3	/*  3 = Albers Conical Equal Area		 */
#define LAMCC	 4	/*  4 = Lambert Conformal Conic			 */
#define MERCAT	 5	/*  5 = Mercator			 	 */
#define PS	 6	/*  6 = Polar Stereographic			 */
#define POLYC	 7	/*  7 = Polyconic				 */
#define EQUIDC	 8	/*  8 = Equidistant Conic			 */
#define TM	 9	/*  9 = Transverse Mercator			 */
#define STEREO	10	/* 10 = Stereographic				 */
#define LAMAZ	11	/* 11 = Lambert Azimuthal Equal Area		 */
#define AZMEQD	12	/* 12 = Azimuthal Equidistant			 */
#define GNOMON	13	/* 13 = Gnomonic				 */
#define ORTHO	14	/* 14 = Orthographic				 */
#define GVNSP	15	/* 15 = General Vertical Near-Side Perspective	 */
#define SNSOID	16	/* 16 = Sinusiodal				 */
#define EQRECT	17	/* 17 = Equidistant Cylindrical Rectangular	 */
#define MILLER	18	/* 18 = Miller Cylindrical			 */
#define VGRINT	19	/* 19 = Van der Grinten				 */
#define HOM	20	/* 20 = (Hotine) Oblique Mercator 		 */
#define SOM	21	/* 21 = Space Oblique Mercator (SOM)		 */

/* Codes for unit_id */

#define RADIANS		0
#define FEET		1
#define METERS		2
#define SECONDS		3
#define DEGREES		4
#define DMS		5

/* Codes for datum */
/* Used by GCTP to determine semi-major axis and eccentricity squared */
/* See USGS General Cartographic Transformation Package for more details */

#define CLARKE_1866	 0		/* Clarke 1866 (default)	 */
#define CLARKE_1880	 1		/* Clarke 1880			 */
#define BESSEL		 2		/* Bessel			 */
#define INT_1967	 3		/* International 1967		 */
#define INT_1909	 4		/* International 1909		 */
#define WGS_72		 5		/* WGS 72			 */
#define EVEREST		 6		/* Everest			 */
#define WGS_66		 7		/* WGS 66			 */
#define GRS_1980	 8		/* GRS 1980			 */
#define AIRY		 9		/* Airy				 */
#define MOD_EVRST	10		/* Modified Everest		 */
#define MOD_AIRY	11		/* Modified Airy		 */
#define WALBECK		12		/* Walbeck			 */
#define SE_ASIA		13		/* Southeast Asia		 */
#define AUS_NAT		14		/* Australian National		 */
#define KRASS		15		/* Krassovsky			 */
#define HOUGH		16		/* Hough			 */
#define MERC_1960	17		/* Mercury 1960			 */
#define MOD_MERC_68	18		/* Modified Mercury 1968	 */
#define SPHERE		19		/* Sphere of radius 6370997 mtrs */

/* Projection definition structure */
struct projdef {
        int     id;             /* projection id         */
	int	uid;		/* units id		 */
	int	zone;		/* projection zone	 */
	int	datum;		/* projection datum	 */
	double	parms[NPARMS];	/* projection parameters */
};

#ifdef MPROJ_STATICS
char *PROJECT_STR [NPROJ] [2] = {
  {"geo",   "Geographic"},
  {"utm",    "Universal Transverse Mercator"},
  {"spcs",   "State Plane Coordinates"},
  {"albers", "Albers Conical Equal Area"},
  {"lamcc",  "Lambert Conformal Conic"},
  {"mercat", "Mercator"},
  {"ps",     "Polar Stereographic"},
  {"polyc",  "Polyconic"},
  {"equidc", "Equidistant Conic"},
  {"tm",     "Transverse Mercator"},
  {"stereo", "Stereographic"},
  {"lamaz",  "Lambert Azimuthal Equal Area"},
  {"azmeqd", "Azimuthal Equidistant"},
  {"gnomon", "Gnomonic"},
  {"ortho",  "Orthographic"},
  {"gvnsp",  "General Vert. Near-Side Persp."},
  {"snsoid", "Sinusoidal"},
  {"eqrect", "Equirectangular"},
  {"miller", "Miller Cylindrical"},
  {"vgrint", "Van der Grinten"},
  {"hom",    "Oblique Mercator (Hotine)"},
  {"som",    "Space Oblique Mercator (SOM)"}
} ;

char *UNIT_STR [NUNITS] = {
  "radians",
  "feet",
  "meters (default)",
  "seconds",
  "degrees",
  "packed DMS (DDDMMMSSS.SS)"
} ;

char *DATUM_STR [NDATUM] = {
  "Clarke 1866 (default)",
  "Clarke 1880",
  "Berssel",
  "International 1967",
  "International 1909",
  "WGS 72",
  "Everest",
  "WGS 66",
  "GRS 1980",
  "Airy",
  "Modified Everest",
  "Modified Airy",
  "Walbeck",
  "Southeast Asia",
  "Australian National",
  "Krassovsky",
  "Hough",
  "Mercury 1960",
  "Modified Mercury 1968",
  "Sphere, rad.= 6370997 meters"
} ;
#endif

/* some programs include this just for the definitions, so check
 * if we're IPW before using EXFUN
 */

#ifdef IPW_H
extern char		* EXFUN(projname, (int proj_id));
extern char		* EXFUN(projunits, (int units_id));
extern struct projdef	* EXFUN(readproj, (char *name));
extern int		  EXFUN(writeproj, (struct projdef *proj));
extern double		  EXFUN(dms, (double value));
#else
extern char * projname();
extern char * projunits();
extern struct projdef * readproj();
extern int writeproj();
extern double dms();
#endif

#endif
