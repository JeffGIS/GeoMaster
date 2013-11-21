
/* LINTLIBRARY */

#include "ipw.h"
#include "lqh.h"

/*
** NAME
**      lqhfree -- free all left over parts of a LQH header
**
** SYNOPSIS
**      #include "lqh.h"
**
**      int lqhfree(lqhpp, nbands)
**      LQH_T **lqhpp;
**	int nbands
**
** DESCRIPTION
**      lqhfree frees any LQH structures that might be left over after a
**	file is closed.
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
**	Applications should not call lqhfree.  fpclose will call this routine.
**
**	Do not attempt to use lqhpp or any floating point functions on
**	the file after this is called.
**
** FUTURE DIRECTIONS
**
** BUGS
*/

int
DEFUN(lqhfree, (lqhpp, nbands),
     LQH_T ** lqhpp
 AND int      nbands)
{
  int band;

  if (lqhpp == NULL)
    return (ERROR);

  for (band = 0; band < nbands; band++) {
    LQH_T  *lqhp;

    lqhp = lqhpp[band];
    if (lqhp == NULL)
      continue;

    SAFE_FREE (lqhp->units);
    SAFE_FREE (lqhp->interp);
    SAFE_FREE (lqhp->bkpt);
    SAFE_FREE (lqhp);
    /* lininv and map are freed by fpclose */
  }

  SAFE_FREE (lqhpp);

  return (OK);
}
