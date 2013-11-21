/*
 * NB: This is a skeleton file.  You must do the following substitutions:
 *	XX	header name, upper-case (e.g. WIN)
 *	xx	header name, lower case (e.g. win)
 *	YY	header field name, upper-case (e.g. BLINE)
 *	yy	header field name, upper-case (e.g. bline)
 *     You must also add/delete code as indicated by "### ... %%%" comments
 */
#ifndef	XX_H
#define	XX_H

/*
** NAME
**	xxh.h -- header file for XXH header
**
** SYNOPSIS
**	#include "xx.h"
**
** DESCRIPTION
**
** FUTURE DIRECTIONS
**
** BUGS
*/

typedef struct {
} XXH_T;

#define	XXH_HNAME	"xx"		/* header name within IPW	 */
#define	XXH_VERSION	"$Revision: 1.5 $"	/* RCS revsion #	 */

/* field keywords */
#define	XXH_YY		"yy"
/*### etc. %%%*/

/* field access macros */
#define	xxh_yy(p)	( (p)->yy )
/*### etc. %%%*/

/* definitions */
#define	foo		bar
/*### etc. %%%*/

/* function declarations */
extern bool_t   xxhcheck();
extern XXH_T  **xxhdup();
extern XXH_T   *xxhmake();
extern XXH_T  **xxhread();
extern int      xxhwrite();

/* $Header: xxh.h,v 1.5 88/03/12 15:54:23 frew Exp $ */
#endif
