: 'DO NOT DELETE THIS LINE: it keeps this script from being run by csh'
#-----------------------------------------------------------------------
# Copyright (c) 1990 The Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that: (1) source distributions retain this entire copyright
# notice and comment, and (2) distributions including binaries display
# the following acknowledgement:  ``This product includes software
# developed by the Computer Systems Laboratory, University of
# California, Santa Barbara and its contributors'' in the documentation
# or other materials provided with the distribution and in all
# advertising materials mentioning features or use of this software.
#
# Neither the name of the University nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#-----------------------------------------------------------------------

: ${IPW?}

## NAME
##	interp -- interpolate between breakpoints
##
## SYNOPSIS
##	interp
##
## DESCRIPTION
##	interp copies ASCII X-Y pairs of integers from the standard input
##	to the standard output. If successive Xs differ by more than +- 1,
##	then the intervening X values are also printed, along with linearly
##	interpolated integer Ys.
##
## OPTIONS
##
## EXAMPLES
##	The following input:
##
##		0 0
##		8 4
##
##	produces the following output:
##
##		0 0
##		1 1
##		2 1
##		3 2
##		4 2
##		5 3
##		6 3
##		7 4
##		8 4
##
## FILES
##
## DIAGNOSTICS
##	input line must have exactly 2 fields
##
## RESTRICTIONS
##	all Xs and Ys must be integers.
##
## FUTURE DIRECTIONS
##	interp is currently implemented as a shell script.
##	It may be desirable to reimplement interp as a program.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	IPW:  interp, mklut
##	UNIX: awk

PATH="$PATH:$IPW/lib"
. ipwenv

optstring=
synopsis=''
description='interpolate between breakpoints'

set -- `getopt "$optstring" $* 2>/dev/null` || {
	exec usage $0 "$synopsis" "$description"
}

while :; do
	case $1 in
	--)	shift
		break
		;;
	*)	exec sherror $0 '"getopt" failed'
		;;
	esac

	shift
done

$AWK '
	BEGIN {
		stderr = "cat 1>&2"
	}

	NF != 2 {
		print "BAD INPUT:", $0 | stderr
		exit 1
	}

	NR == 1 {
		xold = $1
		yold = $2
	}

	{
		xnew = $1
		ynew = $2
	}

	xnew != xold {
		dx = xnew - xold
		dydx = (ynew - yold) / dx

		if (dx < 0) {
			dx = -1
		}
		else {
			dx = 1
		}

		y = yold
		for (x = xold; x != xnew; x += dx) {
			print x, int(y + 0.5)
			y += dydx
		}

		xold = xnew
		yold = ynew
	}

	END {
		print x, int(yold + 0.5)
	}
'

case $? in
0)	exit 0
	;;
*)	exec sherror $0 'input line must have exactly 2 fields'
	;;
esac

# $Header: /local/share/pkg/ipw/src/bin/interp/RCS/interp.sh,v 1.3 90/11/16 16:51:42 frew Exp $
