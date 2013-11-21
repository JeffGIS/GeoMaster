#include <math.h>
#include "ipw.h"
#include "getargs.h"

/*
** NAME
**	seq -- generate sequence of numbers
**
** SYNOPSIS
**	seq -r lo,hi [ -n num ] [ -d inc ] [ -s func ] [ -f format ]
**
** DESCRIPTION
**	seq generates a sequence of numbers between {lo} and {hi}
**	inclusive.  The number of points and spacing can be set with
**	other options (default: 100 points with linear spacing).
**
** OPTIONS
**	-r	The numbers will range from {lo} to {hi}, inclusive.
**
**	-n	{num} (default: 100) numbers will be generated.
**
**	-d	The spacing between points will be {inc} (default: use
**		the spacing function and {num} to determine).
**
**	At most one of -n or -d may be specified.
**
**	-s	The numbers will be spaced using a {func} (default: linear)
**		function.  Acceptable values are "lin", "log", "exp", and
**		"sqrt".
**
**	-f	The numbers will be printed using the C format {format}
**		(default: "%g").
**
** EXAMPLES
**	To generate 4 equally spaced points between 200 and 600:
**
**		seq -r 200,600 -n 4
**
**	To generate numbers between 200 and 600, increasing by 100 each
**	interval:
**
**		seq -r 200,600 -d 100
**
**	To generate 30 numbers between 0 and 10, with exponential spacing:
**
**		seq -r 0,10 -n 30 -s exp
**
** FILES
**
** DIAGNOSTICS
**	-d specified, -n ignored
**
**		If -n (number of points to generate) is specified as well as
**		-d (the spacing between points), the -n option is ignored.
**
**	-n too small, must be >= 3
**
**		At least 3 points must be requested.
**
**	[ -f ..%..g.. or ..%..f.. or ..%..e.. ]
**
**		The format specification must be a valid C printf-style
**		format.
**
**	range error: lo equals hi
**
**		The low and high values are equal.
**
**	no negative numbers with {func} spacing
**
**		The "log" and "sqrt" spacing functions do not allow negative
**		numbers.
**
**	no non-positive numbers with {func} spacing
**
**		The "log" spacing function does not allow zero.
**
**	increment negative or zero
**
**		The spacing between points (the increment) must be positive
**		if {lo} is less than {hi}.
**
**	increment positive or zero
**
**		The spacing between points (the increment) must be negative
**		if {lo} is greater than {hi}.
**
**	math function error
**
**		A value given to the spacing function was invalid.
**
**	range error: increment out of range
**	range error: start out of range
**	range error: fin out of range
**
**		The function being ued has caused one of the variables
**		to go out of range.  Lower the range values.
**
** RESTRICTIONS
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/5/89	Written by Jeff Dozier, UCSB.
**	4/27/93	Error messages modifed.  Added range chacking.
**		Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
*/


void   EXFUN( seqloop, (double start, double fun, double inc, double (*inv)(),
                        CONST char *fmt));
double EXFUN( square, (double x));

#define DEFAULT	100

void
main(argc, argv)
	int             argc;
	char           *argv[];

{
	static OPTION_T opt_r = {
		'r', "beginning and end of range of numbers",
		REAL_OPTARGS, "#",
		REQUIRED, 2, 2
	};

	static OPTION_T opt_n = {
		'n', "number of points (default 100)",
		INT_OPTARGS, "points",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_d = {
		'd', "spacing between points",
		REAL_OPTARGS, "delta",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_s = {
		's', "spacing function: lin(default) log exp sqrt",
		STR_OPTARGS, "func",
		OPTIONAL, 1, 1
	};

	static OPTION_T opt_f = {
		'f', "C-type format (default is %g)",
		STR_OPTARGS, "fmt",
		OPTIONAL, 1, 1
	};

	static OPTION_T *optv[] = {
		&opt_r,
		&opt_n,
		&opt_d,
		&opt_s,
		&opt_f,
		0
	};

	double          start;		/* start of range		 */
	double          fin;		/* end of range			 */
	double          inc;		/* increment			 */
	bool_t          okneg;		/* ? OK for # negative		 */
	bool_t          okzero;		/* ? OK for # zero		 */
	char           *fmt;		/* C-type format		 */
	char           *pct;		/* -> % in fmt			 */
	char           *sf;		/* spacing function		 */
	int             npts;		/* # points			 */

	double          (*inverse) ();	/* -> inverse function		 */
	double          (*xform) ();	/* -> transform function	 */

 /*
  * begin
  */

	ipwenter(argc, argv, optv, "generates sequence of numbers");

 /*
  * how many points
  */

	if (got_opt(opt_n) && got_opt(opt_d)) {
		warn("-d specified, -n ignored");
	}
	else if (got_opt(opt_d)) {
		inc = real_arg(opt_d, 0);
		npts = 0;
	}
	else if (got_opt(opt_n)) {
		npts = ltoi(int_arg(opt_n, 0));
		if (npts <= 2) {
			error("-n too small, must be >= 3");
		}
	}
	else {
		npts = DEFAULT;
	}

 /*
  * output format
  */

	if (got_opt(opt_f)) {
		fmt = str_arg(opt_f, 0);

 /* check fmt to make sure valid, ending with e, f, or g */
		pct = strchr(fmt, '%');
		if (pct == NULL) {
			error("[ -f ..%..g.. or ..%..f.. or ..%..e.. ]");
		}
		if (strchr(pct, 'e') == NULL && strchr(pct, 'f') == NULL &&
		    strchr(pct, 'g') == NULL) {
			error("[ -f ..%..g.. or ..%..f.. or ..%..e.. ]");
		}
	}
	else {
		fmt = "%g";
	}

 /*
  * make sure range reasonable
  */

	start = real_arg(opt_r, 0);
	fin = real_arg(opt_r, 1);
	if (start == fin) {
		error("range error: lo equals hi");
	}

 /*
  * process spacing function and set transform and inverse funcs
  */

	if (got_opt(opt_s)) {
		sf = str_arg(opt_s, 0);
		if (streq(sf, "lin")) {
			xform = inverse = NULL;
			okneg = okzero = TRUE;
		}
		else if (streq(sf, "log")) {
			inverse = exp;
			xform = log;
			okneg = okzero = FALSE;
		}
		else if (streq(sf, "exp")) {
			inverse = log;
			xform = exp;
			okneg = okzero = TRUE;
		}
		else if (streq(sf, "sqrt")) {
			inverse = square;
			xform = sqrt;
			okneg = FALSE;
			okzero = TRUE;
		}
		else {
			error("%s: unrecognized spacing function", sf);
		}
	}
	else {
		okneg = okzero = TRUE;
		xform = inverse = NULL;
	}

 /*
  * check for bad negative or zero in range
  */

	if (!okneg && (start < (double) 0 || fin < (double) 0)) {
		error("no negative numbers with %s spacing", sf);
	}
	if (!okzero && (start == (double) 0 || fin == (double) 0)) {
		error("no non-positive numbers with %s spacing", sf);
	}

 /*
  * transform beginning and end and set increment
  */

	if (xform != NULL) {
		fin = (*xform) (fin);
		start = (*xform) (start);
	}
	if (npts != 0) {
		inc = (fin - start) / (npts - 1);
	}
	fin += inc / 2;

	if ( (inc >= DBL_MAX) || (inc <= DBL_MIN) ) {
		error("range error: increment out of range");
	}
	if ( (start >= DBL_MAX) || (start <= DBL_MIN) ) {
		error("range error: start out of range");
	}
	if ( (fin >= DBL_MAX) || (fin <= DBL_MIN) ) {
		error("range error: fin out of range");
	}
 /*
  * main loop, either forward or backward
  */

	seqloop(start, fin, inc, inverse, fmt);

 /*
  * all done
  */

	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /usr/home/dozier/ipw/src/bin/seq/RCS/main.c,v 1.2 89/07/05 15:05:17 dozier Exp $";

#endif
