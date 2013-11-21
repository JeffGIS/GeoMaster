/*
** NAME
**	is_a_tty -- determine if a text file descriptor refers to a terminal
**
** SYNOPSIS
**	#include "txtio.h"
**
**	int is_a_tty(fd)
**	TEXT_FD_T fd;
**
** DESCRIPTION
**	Is_a_tty determines if a text file descriptor {fd} refers to a
**	terminal.
**
** RETURN VALUE
**	Is_a_tty returns 1 if {fd} refers to a terminal; otherwise, it
**	returns 0.
**
** EXAMPLES
**	The primary use of is_a_tty is to determine if a program is being
**	used interactively.  In this example, if standard input is a
**	terminal (i.e., not a re-directed file), then print a prompt
**	for the user.
**
**		if (is_a_tty(tstdin))
**			tprintf(tstderr, "Input dew point temperature\n");
**
** GLOBALS ACCESSED
**
** DIAGNOSTICS
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	5/95	Written by J. Domingo, OSU, EPA NHEERL/WED
**
** BUGS
**
** SEE ALSO
**	topen, tstdin, tprintf, no_tty
*/

/* LINTLIBRARY */

#include "ipw.h"
#include "txtio.h"

int
DEFUN(is_a_tty, (fd),
	TEXT_FD_T           fd)		/* text file descriptor		 */
{
	return isatty(fileno((FILE *) fd));
}
