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
##	cnhist -- convert IPW histogram to cumulative normalized ASCII
##
## SYNOPSIS
##	cnhist [begin [end]]
##
## DESCRIPTION
##	cnhist reads an IPW histogram image from the standard input and writes
##	a cumulative normalized histogram in ASCII to the standard output.
##	If specified, only pixel values greater than or equal to begin, and
##	less than or equal to end, will be considered.
##
## OPTIONS
##	begin	Begin processing at the input value with this index number
##		(default: 0).  The first {begin} output values will be 0.
##
##	end	Cease processing after the input value with this index
##		number (default: last input value).  The last {last - end}
##		output values will be 1.
##
## EXAMPLES
##	To view a cumulative histogram of "image":
##
##		hist image | cnhist | graph -a | plot
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##	cnhist is implemented as a shell script that invokes the $AWK command
##	to perform the normalization calculations
##
## FUTURE DIRECTIONS
##	It may be desirable to reimplement cnhist as a program.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB
##
## BUGS
##
## SEE ALSO
##	IPW:  btoa, hist, histeq, rmhdr
##	UNIX: awk, graph, plot
##	GNU:  graph, xplot

PATH="$PATH:$IPW/lib"
. ipwenv

case $# in
0)	lo=0
	hi=-1
	;;
1)	lo=$1
	hi=-1
	;;
2)	lo=$1
	hi=$2
	;;
*)	lo=-H
	;;
esac

case $lo in
-H)	exec usage $0 '[begin [end]]' \
		'convert IPW histogram to cumulative normalized ASCII'
	;;
esac

{
	echo $lo $hi 
	primg -r -a
} |
	$AWK '
		# NR == DN + 2

		NR == 1 {
			lo = $1 + 2

			hi = $2
			if (hi >= 0) {
				hi += 2
			}

			next
		}

		NR < lo {
			hist[NR] = 0
			next
		}

		hi < 0 || NR <= hi {
			sum += $1
			hist[NR] = sum 
			next
		}

		hi >= 0 && NR > hi {
			hist[NR] = sum
		}

		END {
			for (i = 2; i <= NR; ++i) {
				print hist[i] / sum
			}
		}
	' 

# $Header: /local/share/pkg/ipw/src/bin/cnhist/RCS/cnhist.sh,v 1.7 90/11/16 16:51:26 frew Exp $
