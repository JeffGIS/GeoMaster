/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the Computer Systems Laboratory, University of
 * California, Santa Barbara and its contributors'' in the documentation
 * or other materials provided with the distribution and in all
 * advertising materials mentioning features or use of this software.
 *
 * Neither the name of the University nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* LINTLIBRARY */

#include "ipw.h"
#include "bih.h"
#include "getargs.h"

/*
** NAME
**	ipwenter -- initialize an IPW main program
**
** SYNOPSIS
**	#include "getargs.h"
**
**	void ipwenter(argc, argv, optv, descrip)
**	int argc;
**	char **argv, *descrip;
**	OPTION_T *optv[];
**
** DESCRIPTION
**	Ipwenter is called to initialize an IPW main().  Its chief function is
**	to parse the command-line arguments in argv according to the option
**	descriptions supplied in optv (see getargs.c for more info on option
**	descriptor formats).
**
** RETURN VALUE
**
** GLOBALS ACCESSED
**
** ERRORS
**	If the command line typed by the user is incorrect, ipwenter prints a
**	usage message on the standard error output, then exits.
**
** APPLICATION USAGE
**	A call to ipwenter should be the first executable statement in an IPW
**	main().
**
** FUTURE DIRECTIONS
**	The functionality of ipwenter() should be incorporated into a
**	standard IPW main().
**
** BUGS
*/

void
DEFUN(ipwenter, (argc, argv, optv, descrip),
	int             argc		/* argc (from main())	 	 */
   AND  char          **argv		/* argv (from main())	 	 */
   AND  OPTION_T      **optv		/* -> option descriptors	 */
   AND  CONST char     *descrip)	/* program description string	 */
{
	if (getargs(argc, argv, optv, descrip) == NULL) {
		usage();
	}
 /*
  * initialize o_byteorder to hostorder() - may be changed after
  * ipwenter if a routine plans to bypass the pixio byte swapping
  * layer (e.g., transpose, flip, and window)
  */
	o_byteorder = hostorder();
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/lib/libipw/main/RCS/ipwenter.c,v 1.6 90/11/11 17:16:49 frew Exp $";

#endif
