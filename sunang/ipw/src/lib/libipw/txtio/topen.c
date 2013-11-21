/*
** NAME
**	topen -- open a text file for reading or writing
**
** SYNOPSIS
**	#include "txtio.h"
**
**	TEXT_FD_T  topen(name, mode)
**	char *	name;
**	char *	mode;			(IPW_READ or IPW_WRITE)
**
** DESCRIPTION
**	topen(name,mode) opens a text file for either reading or writing.
**	{name} specifies the name of the file to open.  {mode} is one of
**	these constants: IPW_READ or IPW_WRITE.
**
** RETURN VALUE
**	topen returns a text file descriptor (i.e., TEXT_FD_T) if the
**	open is successful.  If topen fails, it returns the special
**	value NO_TEXT_FD.
**
** EXAMPLES
**	To open a text file called "foo.dat" for reading:
**
**	    TEXT_FD_T fd;
**	
**	    if ((fd = topen("foo.dat", IPW_READ)) == NO_TEXT_FD)
**		    error("Can't open foo.dat");
**	
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
**	tstdin, tstdout, tstderr, tclose
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
