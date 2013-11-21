
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   _crh.c   1.4   10/19/90";
#endif

/*
** NAME
**	_crh -- array of class range headers
**
** SYNOPSIS
**	#include "_crh.h"
**
**	extern CRH_T	**_crh[];
**
** DESCRIPTION
**	_crh is an array of double-indirect pointers to class range headers,
**	indexed by the corresponding UNIX file descriptor.
**
** RESTRICTIONS
**	The number of (CRH_T **) pointers in _crh is set at compile time to
**	{OPEN_LIMIT}.  This imposes a hard limit on the number of image files a
**	process may have open at once.
**
** WARNINGS
**
** APPLICATION USAGE
**	_crh should only be accessed by other UIO routines.
**
** FUTURE DIRECTIONS
**	Routines accessing _crh will replace the use of (CRH_T **) pointers
**	at the application program level.
**
** BUGS
*/

#include "ipw.h"
#include "crh.h"

CRH_T          **_crh[OPEN_LIMIT] = {NULL};
