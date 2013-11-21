
/* LINTLIBRARY */

#include "ipw.h"
#include "geoh.h"

/*
** NAME
**      geohfree -- free all left over parts of a GEO header
**
** SYNOPSIS
**      #include "geoh.h"
**
**      int geohfree(geohpp, nbands)
**      GEOH_T **geohpp;
**	int nbands
**
** DESCRIPTION
**	geohfree frees an entire geohpp structure.
**
** RESTRICTIONS
**
** RETURN VALUE
**      OK for success, ERROR for failure
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
*/

int
DEFUN(geohfree, (geohpp, nbands),
     GEOH_T ** geohpp
 AND int       nbands)
{
  int band;

  if (geohpp == NULL)
    return (ERROR);

  for (band = 0; band < nbands; band++) {
    GEOH_T  *geohp;

    geohp = geohpp[band];
    if (geohp == NULL)
      continue;

    SAFE_FREE (geohp->units);
    SAFE_FREE (geohp->csys);
    SAFE_FREE (geohp);
  }

  SAFE_FREE (geohpp);

  return (OK);
}
