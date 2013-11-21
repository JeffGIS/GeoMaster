
#include "ipw.h"
#include "io.h"
#include "_uio.h"

/*
** NAME
**	usetlevel -- set maximum I/O level used in UIO control block
**
** SYNOPSIS
**	void usetlevel(int fd, int level)
**
** DESCRIPTION
**	usetlevel sets the maximum I/O level used in the UIO control block.
**	This is used to check which version of close should be called.
**
**	The level will be strictly increasing unless level is set to a
**	negative value.
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
**	usetlevel should not be called by any applications.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

void
DEFUN(usetlevel, (fd, level),
	int	fd
  AND   int	level)
{
	ASSERT_OK_FD(fd);

	if (level < 0) {
		assert (-level > FTYPE_MIN);
		assert (-level < FTYPE_MAX);
		_uiocb[fd].level = -level;
	} else {
		assert (level > FTYPE_MIN);
		assert (level < FTYPE_MAX);
		_uiocb[fd].level = MAX( _uiocb[fd].level , level );
	}
}
