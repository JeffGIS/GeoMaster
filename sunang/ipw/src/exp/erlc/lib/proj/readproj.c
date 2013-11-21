
#ifndef lint
static char *SCCSid=	"SCCS version: @(#)   readproj.c   1.1   6/28/91";
#endif

/*
** NAME
** 	readproj -- read projection definition file
** 
** SYNOPSIS
**	#include "mproj.h"
**
**	struct projdef *readproj (name)
**	char *name;
** 
** DESCRIPTION
** 	readproj reads the given projection definition file, writes the
**	contents into a projdef struct, and returns a pointer to the
**	struct.  The projection definition files are ASCII files with the
**	following format:
**		projection ID
**		units code
**		zone code
**		datum code
**		projection parameters (15 values)
** 
** RESTRICTIONS
** 
** RETURN VALUE
**	pointer to allocated/initialized projdef struct
**	NULL if error
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

struct projdef 	*
readproj (name)
	char	       *name;		/* name of projection def file	 */
{
	int		i;		/* loop counter			 */
	FILE	       *fp;		/* file pointer			 */
	struct projdef *proj;		/* -> struct with file contents	 */


   /* open projection file */

	fp = fopen (name, "r");
	if (fp == NULL) {
		usrerr ("error opening file %s", name);
		return (NULL);
	}

   /* allocate projdef struct */

	proj = (struct projdef *) ecalloc (1, sizeof(struct projdef));
	if (proj == NULL) {
		usrerr ("error allocation proj struct");
		return (NULL);
	}

   /* read file, initializing struct */
   /* check validity of data read */

	if (fscanf (fp, "%d", &proj->id) != 1) {
		usrerr ("error reading projection ID");
		return (NULL);
	}
	if (proj->id < 0 || proj->id >= NPROJ) {
		usrerr ("illegal projection ID");
		return (NULL);
	}

	if (fscanf (fp, "%d", &proj->uid) != 1) {
		usrerr ("error reading units ID");
		return (NULL);
	}
	if (proj->uid < 0 || proj->uid >= NUNITS) {
		usrerr ("illegal units ID");
		return (NULL);
	}

	if (fscanf (fp, "%d", &proj->zone) != 1) {
		usrerr ("error reading zone");
		return (NULL);
	}

	if (fscanf (fp, "%d", &proj->datum) != 1) {
		usrerr ("error reading datum code");
		return (NULL);
	}
	if (proj->datum < 0 || proj->datum >= NUNITS) {
		usrerr ("illegal datum code");
		return (NULL);
	}


	for (i = 0; i < NPARMS; i++) {
		if (fscanf (fp, "%lf", &proj->parms[i]) != 1) {
			usrerr ("error reading projection parameter");
			return (NULL);
		}
	}

	return (proj);
}
