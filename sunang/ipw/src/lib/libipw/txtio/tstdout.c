/*
** NAME
**	tstdout -- pre-defined text file descriptor for standard output
**
** SYNOPSIS
**	#include "txtio.h"
**
**	TEXT_FD_T  tstdout;
**
** DESCRIPTION
**	tstdout is a pre-defined TEXT_FD_T variable that refers to standard
**	output as a text file.
**
** RETURN VALUE
**
** EXAMPLES
**	To print a floating point value to standard output:
**
**	    float data;
**
**	    /* the variable "data" is assigned some value */
**
**	    tprintf(tstdout, "data = %f\n", data);
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
**	tstdin, tstderr, tprintf, topen
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
