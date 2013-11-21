
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   projname.c   1.1   6/28/91";
#endif

/*
** NAME
** 	projname -- return name of projection for given ID
** 
** SYNOPSIS
**	char *projname (proj_id);
**	int proj_id;
** 
** DESCRIPTION
** 	projname returns a pointer to a character string containing the
**	name of the geographic projection that corresponds to the given
**	projection ID.
** 
** RESTRICTIONS
**	The returned character string is in a static area and is rewritten
**	with each call.
** 
** RETURN VALUE
**	pointer to character string containing projection name
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
#include "constants.h"

static char name[40];

char *projname (proj_id)
	int		proj_id;	/* projection ID		 */
{
	if (proj_id < 0 || proj_id >= NPROJ)
		return (NULL);

	strcpy (name, projnames[proj_id]);
	return (name);
}
