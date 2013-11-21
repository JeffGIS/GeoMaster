/*
** NAME
**	tclose -- close a text file descriptor
**
** SYNOPSIS
**	#include "txtio.h"
**
**	int  tclose(file)
**	TEXT_FD_T  file;
**
** DESCRIPTION
**	Tclose closes a text file.  If the file is opened for writing,
**	any buffered data is first written to the file.
**
** RETURN VALUE
**	It returns 0 on success, or a non-zero value if an error occurs.
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
**	topen
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
