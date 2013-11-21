/* LINTLIBRARY */

#ifdef	IPW
#include "ipw.h"
#else
#include <stdio.h>
extern char    *strchr();
extern int      strcmp();

#define	REG_1	register
#define	REG_2	register
#define	REG_3	register
#endif

/*
** NAME
**	getopt -- get option letter from argument vector
**
** SYNOPSIS
**	#include <stdio.h>
**
**	int getopt(argc, argv, optstring)
**	int argc;
**	char *argv[], *optstring;
**
** DESCRIPTION
**	"Getopt" is a (crude) command-line parser.  It returns the next option
**	letter in "argv" that matches a letter in "optstring".
**
**	"Argc" and "argv" are the command-line argument count and vector,
**	respectively, obtained as arguments to main().
**
**	"Optstring" is a string of recognized option letters.  If a letter is
**	followed by a colon, then the option is expected to have an argument
**	that may or may not be separated from it by white space.
**
** RETURN VALUE
**	EOF is returned when all options have been processed.  '?' is returned
**	if an unrecognized (i.e., not present in "optstring") option letter is
**	encountered.
**
** GLOBALS ACCESSED
**	optind	initially 1; set to the "argv" index of the next argument to
**		be processed
**
**	optarg	initially NULL; set to point to the current option-argument
**
** NOTES
**	This "getopt" is derived from the version posted to USENET on 03
**	November 1985 by John Quarterman, which was asserted to be
**	public-domain code from AT&T.
*/

int             optind = 1;		/* argv index of next argument	 */
char           *optarg = NULL;		/* -> current option-argument	 */

/* LINT: opterr not used but included for SVID compatibility */
int             opterr = 0;		/* ? print error message	 */

#ifdef	OPTOPT

/*
 * optopt is not used by IPW nor defined by SVID, but some System V code uses
 * it.  Define OPTOPT if you need it.
 */

int             optopt = 0;		/* current option letter	 */

#endif

int
getopt(argc, argv, optstring)
	int             argc;		/* # of command-line arguments	 */
	REG_1 char    **argv;		/* -> command-line arguments	 */
	char           *optstring;	/* legal option letters		 */
{
 /*
  * argv[optind][sp] is current option letter
  */
	static int      sp = 1;


	REG_2 int       c;		/* current option letter	 */
	REG_3 char     *cp;		/* -> option letter in "optstring" */

	if (sp == 1) {
		if (optind >= argc
		    || argv[optind][0] != '-' || argv[optind][1] == '\0') {
			return (EOF);
		}

		if (strcmp(argv[optind], "--") == NULL) {
			++optind;
			return (EOF);
		}
	}

	c = argv[optind][sp];
#ifdef	OPTOPT
	optopt = c;
#endif

	if (c == ':' || (cp = strchr(optstring, c)) == (char *) NULL) {
		if (argv[optind][++sp] == '\0') {
			++optind;
			sp = 1;
		}

		return ('?');
	}

	if (*++cp == ':') {
		if (argv[optind][sp + 1] != '\0') {
			optarg = &argv[optind++][sp + 1];
		}
		else if (++optind >= argc) {
			sp = 1;
			return ('?');
		}
		else {
			optarg = argv[optind++];
		}

		sp = 1;
	}
	else {
		if (argv[optind][++sp] == '\0') {
			sp = 1;
			++optind;
		}

		optarg = NULL;
	}

	return (c);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr.MC68020/home/ipw/src/lib/libunix/src/RCS/getopt.c,v 1.3 89/10/25 21:46:44 frew Exp $";

#endif
