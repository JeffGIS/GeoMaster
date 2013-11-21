
/* LINTLIBRARY */

#include "ipw.h"
#include "winh.h"

/*
** NAME
**      winhfree -- free all left over parts of a WIN header
**
** SYNOPSIS
**      #include "winh.h"
**
**      int winhfree(winhpp, nbands)
**      WINH_T **winhpp;
**	int nbands
**
** DESCRIPTION
**	winhfree frees an entire winhpp structure.
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
DEFUN(winhfree, (winhpp, nbands),
     WINH_T **  winhpp
 AND int       nbands)
{
  int band;

  if (winhpp == NULL)
    return (ERROR);

  for (band = 0; band < nbands; band++) {
    WINH_T  *winhp;

    winhp = winhpp[band];
    if (winhp == NULL)
      continue;

    SAFE_FREE (winhp);
  }

  SAFE_FREE (winhpp);

  return (OK);
}
