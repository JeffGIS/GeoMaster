
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   writeproj.c   1.1   6/28/91";
#endif

/*
** NAME
** 	writeproj -- write projection definition file
** 
** SYNOPSIS
**	#include "mproj.h"
**
**	int writeproj (proj)
**	struct projdef *proj;
** 
** DESCRIPTION
** 	writeproj writes the given projection parameters to the standard
**	output in the format of a projection definition file.  The projection
**	definition files are ASCII files with the following format:
**
**		projection ID
**		units code
**		zone code
**		datum code
**		projection parameters (15 values)
**
**	See the documentation for the USGS General Cartographic Transormation
**	Package for details on the parameters.
** 
** RESTRICTIONS
** 
** RETURN VALUE
**	ERROR if error occurred (usrerr() will have error message),
**	OK otherwise.
** 
** GLOBALS ACCESSED
** 
** ERRORS
** 
** WARNINGS
** 
** APPLICATION USAGE
** 
** FUTURE DIRECTIONS
** 
** BUGS
**
*/

#include "ipw.h"
#include "mproj.h"

int
writeproj (proj)
	struct projdef *proj;		/* -> struct with file contents	 */
{
	int		i;		/* loop counter			 */


   /* check validity of projection parameters */

	if (proj->id < 0 || proj->id >= NPROJ) {
		usrerr ("illegal projection ID");
		return (ERROR);
	}
	if (proj->uid < 0 || proj->uid >= NUNITS) {
		usrerr ("illegal units ID");
		return (ERROR);
	}
	if (proj->datum < 0 || proj->datum >= NUNITS) {
		usrerr ("illegal datum code");
		return (ERROR);
	}

   /* write parameters to file */

	printf ("%d\n", proj->id);
	printf ("%d\n", proj->uid);
	printf ("%d\n", proj->zone);
	printf ("%d\n", proj->datum);

	for (i = 0; i < NPARMS; i++) {
		printf ("%.*g\n", DBL_DIG, proj->parms[i]);
	}

	return (OK);
}
