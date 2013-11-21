
#include "ipw.h"
#include "io.h"
#include "_uio.h"

/*
** NAME
**	ugetlevel -- get maximum I/O level used in UIO control block
**
** SYNOPSIS
**	int ugetlevel(int fd)
**
** DESCRIPTION
**	ugetlevel gets the maximum I/O level used in the UIO control block.
**	This is used to check which version of close should be called.
**
** RESTRICTIONS
**
** RETURN VALUE
**
** GLOBALS ACCESSED
**	_uiocb[fd]	UIO control block for file descriptor fd
**
** ERRORS
**
** WARNINGS
**
** APPLICATION USAGE
**	ugetlevel should not be called by any applications.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
DEFUN(ugetlevel, (fd),
	int	fd)
{
	ASSERT_OK_FD(fd);

	return(_uiocb[fd].level);
}
