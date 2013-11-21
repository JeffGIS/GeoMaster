#include "ipw.h"

#include "getargs.h"

#include "pgm.h"

/*
** NAME
**	mstats -- image multivariate statistics
**
** SYNOPSIS
**	mstats [image]
**
** DESCRIPTION
**	mstats computes the band-wise mean vector and variance-covariance
**	matrix for the input image.  The statistics are written in ASCII on
**	the standard output, in the following format:
**
**		item				description
**		-------------------------	--------------------------
**		#<stats>			mstats output identifier
**
**		number_of_bands			# bands in the input image
**
**		mean_0 mean_1 ...		mean vector
**
**		variance_0			variance-covariance matrix
**		covariance_1,0 variance_1
**		...
**
**		* 0				(reserved for future use)
**
**		number_of_pixels		(reserved for future use)
**
** FUTURE DIRECTIONS
**	Support collection of per-class statistics, with classes specified by
**	an additional input image.
**
** BUGS
**	Susceptible to rounding error.
*/

main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		OPTIONAL, 1, 1,
	};

	static OPTION_T *optv[] = {
		&operands,
		0
	};

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "image multivariate statistics");
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
  * do it
  */
	headers();
	mstats();
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.1 89/05/09 11:56:10 frew Exp $";

#endif
