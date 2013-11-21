/* LINTLIBRARY */

#include "ipw.h"

/*
** NAME
**	maxfiles -- return maximum number of files opened
**
** SYNOPSIS
**	int maxfiles()
**
** DESCRIPTION
**	returns the maximum number of files IPW will allow to be opened.
**	This is either OPEN_LIMIT or the O/S soft-limit as returned by
**	a call to either sysconf() or getrlimit().
**
** RESTRICTIONS
**
** RETURN VALUE
**
** GLOBALS ACCESSED
**
** ERRORS
**	If sysconf() or getrlimit() fails, the function returns OPEN_LIMIT,
**	which will probably be higher than the real maximum.
**
** WARNINGS
**
** APPLICATION USAGE
**
** FUTURE DIRECTIONS
**
** HISTORY
**	 9 Nov 92   Written by Dana Jacobsen, EPA ERL-C
**	16 Aug 93   Added sysconf() section, Dana Jacobsen, ERL-C.
**
** BUGS
*/


#if CC_HAVE_SYSCONF  /* Have POSIX sysconf() */

#include <unistd.h>

int
maxfiles()
{
  long openmax;

  openmax = sysconf(_SC_OPEN_MAX);

  if (openmax == -1) {
    return(OPEN_LIMIT);
  }

  return( (int) openmax );
}

#else  /* Don't have POSIX sysconf(), so use BSD getrlimit() */

#include <sys/time.h>
#include <sys/resource.h>

extern int EXFUN(getrlimit, (int resource, struct rlimit *rlp));

int
maxfiles()
{
  int success;
  struct rlimit rlp;

  success = getrlimit(RLIMIT_NOFILE, &rlp);

  if (success != 0) {
    /* getrlimit failed for some reason.  Give them the static limit. */
    return (OPEN_LIMIT);
  }

  /* return the soft limit.  Use rlim_max for the hard limit. */
  return(  MIN(OPEN_LIMIT, rlp.rlim_cur)  );
}

#endif  /* sysconf() vs. getrlimit() */

#ifdef TEST_MAIN
#include <stdio.h>

void
main(argc, argv)
	int             argc;
	char          **argv;
{
  int mfiles;

  mfiles = maxfiles();

  printf("\nmaxfiles: %d\n", maxfiles());

  printf("1\t\t");
  printf("OK_FD says: %d\n", OK_FD(mfiles));
  ASSERT_OK_FD(1);
  printf("16\t\t");
  printf("OK_FD says: %d\n", OK_FD(mfiles));
  ASSERT_OK_FD(16);
  printf("mfiles-1\t");
  printf("OK_FD says: %d\n", OK_FD(mfiles));
  ASSERT_OK_FD(mfiles-1);
  printf("mfiles\t\t");
  printf("OK_FD says: %d\n", OK_FD(mfiles));
  ASSERT_OK_FD(mfiles);
  printf("mfiles+1\t");
  printf("OK_FD says: %d\n", OK_FD(mfiles+1));
  ASSERT_OK_FD(mfiles+1);

  exit(0);
}

#endif
