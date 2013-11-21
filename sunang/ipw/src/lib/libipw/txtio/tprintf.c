/*
** NAME
**	tprintf -- print formmated output to a text file
**
** SYNOPSIS
**	#include "txtio.h"
**
**	int  tprintf(file, format, ...)
**	TEXT_FD_T  file;
**	char *  format;
**
** DESCRIPTION
**	tprintf performs formatted output to a text file in the same
**	way the C library routine "fprintf" does.  The format string
**	specifies the types of the arguments that follow the format string
**	(see documentation, e.g., man pages, on fprintf for more details
**	about the format string).
**
** RETURN VALUE
**	tprintf returns the number of characters actually printed, or
**	a negative number if an error occurs.
**	
** EXAMPLES
**	If the variable "fd" refers to a text file that's open for
**	writing, then the following code prints a temperature to the
**	file:
**
**	    tprintf(fd, "temperature = %lf %c\n", temperature,
**		    (is_Farenheit ? 'F' : 'C'));
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
**	topen, tstdout, tstderr, tscanf
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
