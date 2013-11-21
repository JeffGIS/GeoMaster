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

#include "ipw.h"

#include "bih.h"
#include "getargs.h"

#include "mux.h"

/*
** NAME
**	mux -- band-interleave images
**
** SYNOPSIS
**	mux image ...
**
** DESCRIPTION
**	Mux band-interleaves the operand images into a single output image
**	that contains all of the input bands and writes the result to the
**	standard output.  The operand "-" means the standard input.
**
** OPTIONS
**
** EXAMPLES
**	To combine "red", "green", and "blue" images into a single
**	"color" image:
**
**		mux red green blue  >color
**
**	To multiply "image1" by "image2":
**
**		mux image1 image2  |  mult
**
** FILES
**	$TMPDIR/mux{NNNNN}
**
**		Temporary copy off all input headers.
**
** DIAGNOSTICS
**	image size differs from 1st image
**
**		All input images must have the same number of lines and
**		samples.
**
** RESTRICTIONS
**	The input images must agree in every dimension except number of
**	bands.
**
**	The maximum number of input images is limited by the number of
**	files that a program may have open simultaneously.  This limit is
**	set in the C shell by the "limit" command.  Try a command like:
**
**		limit descriptors 256
**
**	to get more.  This limit can be worked around by piping one mux
**	into another:
**
**		mux image1 ... imageN | mux - imageN+1 ...
**
** FUTURE DIRECTIONS
**
** HISTORY
**	7/1/90	Written by James Frew, UCSB.
**	4/26/93	Ran through Purify.  Dana Jacobsen, ERL-C.
**
** BUGS
**
** SEE ALSO
**	IPW:  demux
**	UNIX: limit
*/

void
main(argc, argv)
	int             argc;
	char          **argv;
{
	static OPTION_T operands = {
		OPERAND, "input image file",
		STR_OPERANDS, "image",
		REQUIRED, 1,
	};

	static OPTION_T *optv[] = {
		&operands,
		0
	};

	int             i;		/* current input file #		 */
	int            *i_fdp;		/* -> input file descriptor
					 * array */
	int             i_n;		/* # input files		 */
	int             o_fd;		/* output file descriptor	 */

 /*
  * begin
  */
	ipwenter(argc, argv, optv, "multiplex (band-interleave) images");
 /*
  * access input files
  */
	i_n = n_args(operands);

 /* NOSTRICT */
	i_fdp = (int *) ecalloc(i_n, sizeof(int));
	if (i_fdp == NULL) {
		error("can't allocate input file descriptor array");
	}

	for (i = 0; i < i_n; ++i) {
		i_fdp[i] = uropen(str_arg(operands, i));
		if (i_fdp[i] == ERROR) {
			error("can't open \"%s\"", str_arg(operands, i));
		}

		no_tty(i_fdp[i]);
	}
 /*
  * access output file
  */
	o_fd = ustdout();
	no_tty(o_fd);
 /*
  * turn off history mechanism
  */
	no_history(o_fd);
 /*
  * do it
  */
	mux(i_n, i_fdp, o_fd);
 /*
  * all done
  */
	ipwexit(EX_OK);
}

#ifndef	lint
static char     rcsid[] = "$Header: /local/share/pkg/ipw/src/bin/mux/RCS/main.c,v 1.12 90/11/11 17:07:21 frew Exp $";

#endif
