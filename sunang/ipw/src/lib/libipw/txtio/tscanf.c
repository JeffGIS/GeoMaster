/*
** NAME
**	tscanf -- scan a text file for formatted input
**
** SYNOPSIS
**	#include "txtio.h"
**
**	int  tscanf(file, format, ...)
**	TEXT_FD_T  file;
**	char *  format;
**
** DESCRIPTION
**	tscanf scans a text file for formatted input in the same way the
**	C library routine "fscanf" does.  The format string specifies
**	what type of values to scan for (see documentation, e.g., man pages,
**	on fscanf for more details about the format string).  The format
**	string indicates the number of arguments that follow the format
**	string.
**
** RETURN VALUE
**	tscanf returns the number of arguments that were actually
**	assigned values, or it returns EOF if failure occurred before
**	the first assignment.
**
** EXAMPLES
**	Suppose the file "locations.dat" contains a list of locations
**	where each location is specified by a pair of floating values that
**	represent its longitude and latitude.  The following loop reads
**	in the locations one at a time for processing:
**
**	    TEXT_FD_T  fd;
**	    float      longitude;
**	    float      latitude;
**
**	    if ((fd = topen("locations.dat", IPW_READ)) == NO_TEXT_FD)
**		error("Can't open 'locations.dat'");
**
**	    while (tscanf(fd, "%f %f", &longitude, &latitude) == 2)
**		{
**		/* process location */
**		}
**
**	    tclose(fd);
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
**	topen, tstdin, tprintf
*/

/*
 *  This function is implemented as a macro in the include file: txtio.h
 */
