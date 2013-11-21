/*
** NAME
**	tstdin -- pre-defined text file descriptor for standard input
**
** SYNOPSIS
**	#include "txtio.h"
**
**	TEXT_FD_T  tstdin;
**
** DESCRIPTION
**	tstdin is a pre-defined TEXT_FD_T variable that refers to standard
**	input as a text file.
**
** RETURN VALUE
**
** EXAMPLES
**	To read an integer value from standard input:
**
**	    int data;
**
**	    if (tscanf(tstdin, "%d", &data) == 1)
**		/* process data */
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
**	tstdout, tstderr, tscanf, topen
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
