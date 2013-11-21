#include "ipw.h"
#include "getargs.h"
#include "pgm.h"

/*
** NAME
**	solar -- exoatmospheric solar irradiance
**
** SYNOPSIS
**	solar [-w um[,um2]] [-d year,month,day] [-a]
**
** DESCRIPTION
**	Solar calculates exoatmospheric direct solar irradiance.  If two
**	arguments to -w are given, the integral of solar irradiance over
**	the range will be calculated.  If one argument is given, the
**	spectral irradiance will be calculated.
**
**	If no wavelengths are specified on the command line, single
**	wavelengths in um will be read from the standard input and the
**	spectral irradiance calculated for each.
**
** OPTIONS
**	-w	If two arguments are given, the integral of solar irradiance
**		in the range {um} to {um2} will be calculated.  If one
**		argument is given, the spectral irradiance will be calculated.
**		If no arguments are given, single wavelengths will be read
**		from the standard input.
**
**	-d	The date is {year}, {month}, {day}.  This is used to
**		calculate the solar radius vector which divides the result.
**		If no date is given, the solar radius vector is taken as 1.
**
**	-a	No unit annotation will be printed, just the numbers.
**
** EXAMPLES
**	To calculate the solar irradance between .58 um and .68 um on
**	June 22, 1990:
**
**		solar -d 1990,6,22 -w .58,.68
**
** FILES
**
** DIAGNOSTICS
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	4/4/88	Written by Jeff Dozier, UCSB.
**	4/30/93	Cleaned up and converted to ANSI C.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
*/

void
DEFUN( main, (argc, argv),
	int             argc
   AND  char          **argv)
{
	static OPTION_T opt_w = {
		'w',
		"wavelength range in um (otherwise from stdin)",
		REAL_OPTARGS, "um",
		OPTIONAL, 1, 2
	};

	static OPTION_T opt_d = {
		'd',
		"date (year, month, day)",
		INT_OPTARGS, "d",
		OPTIONAL, 3, 3
	};

	static OPTION_T opt_a = {
		'a',
		"abbreviated output (no annotation, just numbers)"
	};

	static OPTION_T *optv[] = {
		&opt_w,
		&opt_d,
		&opt_a,
		0
	};

	double         *range;
	int            *date;

 /*
  * begin
  */
	ipwenter(argc, argv, optv,
		 "exoatmospheric direct solar irradiance");
 /*
  * wavelength range or individual wavelengths
  */
	range = (got_opt(opt_w)) ? range = real_argp(opt_w) : NULL;
 /*
  * if specific date calculate radius vector, otherwise use 1.0
  */
	date = (got_opt(opt_d)) ? int_argp(opt_d) : NULL;
 /*
  * do it
  */
	solar(n_args(opt_w), range, date, got_opt(opt_a));
 /*
  * end
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: main.c,v 1.2 88/04/04 16:57:40 dozier Exp $";

#endif
