#include "ipw.h"
#include "getargs.h"

/*
** NAME
**	toporad -- topographic distribution of solar radiation
**
** SYNOPSIS
**	toporad
**
** DESCRIPTION
**	Toporad calculates the topographic distribution of solar
**	radiation at a single time, using input beam and diffuse
**	radiation calculations supplied by elevrad.  The following
**	quantities vary over the image:  beam and diffuse irradiance,
**	local illumination angle, sky view factor, terrain
**	configuration factor, and surface albedo.   They are entered
**	via a single input file in this specified order.  Solar zenith
**	angle is constant over the image and is entered from the
**	command line.
**
** OPTIONS
**
** OPERANDS
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

extern void toporad();

main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T opt_n = {
		'n', "net radiation instead of incoming",
	};

	static OPTION_T operands = {
		OPERAND, "beam & diffuse irrad, mu_s, Vf, Ct, albedo",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_n,
		&operands,
		0
	};

	int             fdi;		/* input file descriptor	 */
	int             fdo;		/* output file descriptor	 */

 /*
  * begin
  */

	ipwenter(argc, argv, optv,
		 "topographic distribution of solar radiation");

 /*
  * access input files
  */
	if (!got_opt(operands)) {
		fdi = ustdin();
	}
	else {
		fdi = uropen(str_arg(operands, 0));
		if (fdi == ERROR) {
			error("can't open \"%s\"", str_arg(operands, 0));
		}
	}
	no_tty(fdi);
 /*
  * access output file
  */
	fdo = ustdout();
	no_tty(fdo);
 /*
  * read input data, calculate, and write
  */

	toporad(fdi, fdo, got_opt(opt_n));

 /*
  * all done
  */

	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/dozier/ipw/src/bin/toporad/RCS/main.c,v 1.1 89/07/05 13:25:24 dozier Exp $";

#endif
