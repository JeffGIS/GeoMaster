/*
** NAME
**	getok -- get decision from standard input
**
** DESCRIPTION
**	Ok prompts on the standard output.  If the response begins
**	with 'y', ok returns a 1; otherwise, it returns a 0.
**
**	If the standard input is not a terminal, no prompt is printed.
**
** HISTORY
**	spring 1981: 	written by J. Frew, CSL, UCSB
**	spring 1987: 	modified by D. Marks, SNARL, to correct bug
**			found under Sun Unix, V3.3
**	May 1995	Converted to IPW by J. Domingo, OSU, US EPA
*/

#include	<stdio.h>
#include	<ctype.h>
#include	"ipw.h"
#include	"txtio.h"
#include	"erlc.h"

int
DEFUN(getok, (prompt),
	char	*prompt)
{
	char	answer[4];

	if (is_a_tty(tstdin)) {
		tprintf(tstdout, "%s (y|n)? ", prompt);
	}

	if (tscanf(tstdin, "%s", answer) == EOF)
		ipwexit(EX_OK);

	if (answer[0] == 'y')
		return(TRUE);
	else
		return(FALSE);
}
