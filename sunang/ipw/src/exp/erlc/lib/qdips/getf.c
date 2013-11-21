/*
** NAME
**	getf -- get a floating-pt. number, aggressively
**
** DESCRIPTION
**	Getf writes prompt on the standard output, followed by a ?
**	The user must type a real number in the range minval <-> maxval,
**	or getf will prompt again.  If maxval < minval, the number
**	is only required to be >= minval.  The number is returned.
**
** HISTORY
**	spring 1981: 	written by J.Frew, CSL, UCSB
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
**
** BUGS
**	There should be some way to specify maxval by itself.
*/

#include	<stdio.h>
#include	<ctype.h>
#include	"ipw.h"
#include	"txtio.h"

double
DEFUN(getf, (prompt, minval, maxval),
	char	*prompt
   AND	double	minval
   AND	double	maxval)
{
	double	val;

	while (TRUE) {
		if (is_a_tty(tstdin)) {
			tprintf(tstdout, "%s? ", prompt);
			tflush(tstdout);
		}
		if (tscanf(tstdin, "%lf", &val) == EOF)
			ipwexit(EX_OK);

		if (maxval < minval) {	/*  just check lower bound  */
			if (val >= minval)
				break;

			tprintf(tstdout, "(%g) must be >= %g\n", val, minval);
			tflush(tstdout);
		}
		else {
			if (val >= minval  &&  val <= maxval)
				break;

			tprintf(tstdout, "(%g) must be >= %g and <= %g\n",
				val, minval, maxval);
			tflush(stdout);
		}
	}

	return (val);
}
