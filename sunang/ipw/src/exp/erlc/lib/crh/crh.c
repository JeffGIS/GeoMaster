
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   crh.c   1.4   10/19/90";
#endif

/*
** NAME
**	crh -- get CR header associated with file descriptor
**
** SYNOPSIS
**	#include "crh.h"
**
**	CRH_T **crh(fd)
**	int fd;
**
** DESCRIPTION
**
** RESTRICTIONS
**
** RETURN VALUE
**	pointer to the CR header array associated with file descriptor fd; or
**	NULL if there are no CR headers associated with file descriptor fd.
**
** GLOBALS ACCESSED
**	_crh	array of CR pointers, indexed by file descriptor
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** BUGS
*/

#include "ipw.h"
#include "crh.h"
#include "_crh.h"

CRH_T **
crh(fd)
	int		fd;		/* image file descriptor	*/
{
	ASSERT_OK_FD(fd);
	return (_crh[fd]);
}
