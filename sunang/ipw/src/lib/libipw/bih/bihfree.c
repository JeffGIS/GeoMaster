
/* LINTLIBRARY */

#include "ipw.h"
#include "bih.h"

/*
** NAME
**      bihfree -- free all left over parts of a BIH header
**
** SYNOPSIS
**      #include "bih.h"
**
**      int bihfree(bihpp)
**      BIH_T **bihpp;
**
** DESCRIPTION
**      bihfree frees any BIH structures that might be left over after a file
**	is closed.  bihpp will be freed as well.
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
**      bihfree is called by IPW application programs after the file is
**	closed if there is going to be processing afterwards.  If the
**	program will end immediately afterwards, there is no need to call
**	bihfree, but if other processing will take place, calling bihfree
**	can save memory.
**
** FUTURE DIRECTIONS
**	Have pxclose call bihfree directly.
**
** BUGS
*/


int
DEFUN(bihfree, (bihpp),
      BIH_T ** bihpp)
{
  int band, nbands;

  if (bihpp == NULL)
    return (ERROR);

  if ( ! bihcheck(bihpp) )
    return (ERROR);

  nbands = bih_nbands(bihpp[0]);

  SAFE_FREE (bihpp[0]->img->byteorder);
  SAFE_FREE (bihpp[0]->img);

  for (band = 0; band < nbands; band++) {
    while ( ok_sv(bihpp[band]->annot)          &&
            (bihpp[band]->annot->n != 0)       &&
            (bihpp[band]->annot->v[0] != NULL) )
      delsv (bihpp[band]->annot, 0);
    SAFE_FREE(bihpp[band]->annot);
    while ( ok_sv(bihpp[band]->history)          &&
            (bihpp[band]->history->n != 0)       &&
            (bihpp[band]->history->v[0] != NULL) )
      delsv (bihpp[band]->history, 0);
    SAFE_FREE(bihpp[band]->history);
    SAFE_FREE (bihpp[band]);
  }
  SAFE_FREE(bihpp);
  return (OK);
}
