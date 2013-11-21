
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   projunits.c   1.1   6/28/91";
#endif

/*
** NAME
** 	projunits -- return units of projection for given units ID
** 
** SYNOPSIS
**	char *projunits (units_id);
**	int units_id;
** 
** DESCRIPTION
** 	projunits returns a pointer to a character string containing the
**	units of the geographic projection that corresponds to the given
**	units ID.
** 
** RESTRICTIONS
**	The returned character string is in a static area and is rewritten
**	with each call.
** 
** RETURN VALUE
**	pointer to character string containing projection units
**	or NULL if illegal ID
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

static char *punits[] = {
	"radians",
	"feet",
	"meters",
	"seconds",
	"degrees",
	"dms"
};

static char units[40];

char *projunits (units_id)
	int		units_id;	/* projection ID		 */
{
	if (units_id < 0 || units_id >= NUNITS)
		return (NULL);

	strcpy (units, punits[units_id]);
	return (units);
}
