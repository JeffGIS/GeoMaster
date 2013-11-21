/*
** NAME
**	tflush -- flush any pending input/output for a text file
**
** SYNOPSIS
**	#include "txtio.h"
**
**	int  tflush(file)
**	TEXT_FD_T  file;
**
** DESCRIPTION
**	tflush flushes any buffered data for a text file.  If the file is
**	opened for writing, any buffered data is first written to the file.
**	If the file is opened for reading, any buffered input data is cleared.
**
** RETURN VALUE
**	tflush returns 0 on success, or a non-zero value (EOF) if an
**	error occurs.
**
** EXAMPLES
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
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
