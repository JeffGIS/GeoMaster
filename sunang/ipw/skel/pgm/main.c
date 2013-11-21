#include "ipw.h"

#include "getargs.h"
@#include ...@

#include "pgm.h"

/*
** NAME
**	@PGM@ -- @program description@
**
** SYNOPSIS
**	@PGM@ @options ..@ @operands ...@
**
** DESCRIPTION
**
** OPTIONS
**	@letter@	@option description@
**
** EXAMPLES
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** BUGS
*/

@#define ...@

@static ...@

main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_@letter@ = {
		'@letter@', "@option description@",
 /*
  * defaults: no optargs (i.e. boolean option)
  */
		@STR|INT|REAL@_OPTARGS, "@argument description word@",
 /*
  * defaults: OPTIONAL, 1, (unlimited)
  */
		@OPTIONAL|REQUIRED@, @min_nargs@, @max_nargs@
	};

	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&opt_@letter@,
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "@program description@");
 /*
  * access input file(s)
  */
	if (!got_opt(operands)) {
		parm.i_fd = ustdin();
	}
	else {
		parm.i_fd = uropen(str_arg(operands, 0));
		if (parm.i_fd == ERROR) {
			error("can't open \"%s\"", str_arg(operands, 0));
		}
	}

	no_tty(parm.i_fd);
 /*
  * access output file
  */
	parm.o_fd = ustdout();
	no_tty(parm.o_fd);
 /*
  * do it
  */
	headers();
	@PGM@();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.1 88/09/08 20:22:44 frew Exp $";

#endif
