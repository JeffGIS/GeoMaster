/*
** NAME
**	tstderr -- pre-defined text file descriptor for standard error
**
** SYNOPSIS
**	#include "txtio.h"
**
**	TEXT_FD_T  tstderr;
**
** DESCRIPTION
**	tstderr is a pre-defined TEXT_FD_T variable that refers to standard
**	error as a text file.
**
** RETURN VALUE
**
** EXAMPLES
**	To print a message to standard error:
**
**	    if (data < MAXIMUM) 
**	        tprintf(tstderr, "data (= %d) is > maximum value (= %d)\n",
**			data, MAXIMUM);
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
**	tstdin, tstdout, tprintf
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
