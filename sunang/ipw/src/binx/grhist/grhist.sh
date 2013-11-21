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
##	grhist -- graph an IPW histogram
##
## SYNOPSIS
##	grhist [graph-options]
##
## DESCRIPTION
##	grhist reads an IPW histogram from the standard input, and writes a
##	graphic rendition of the histogram (using graph(1)) to the standard
##	output.
##
## OPTIONS
##	Any command-line arguments are passed to graph(1).
##
## EXAMPLES
##	To compute and plot a histogram on the LaserWriter:
##		hist image | grhist | lpr -g
##
##	To plot a (precomputed) histogram on a Tektronix 4014:
##		grhist <histogram | plot -T4014
##
##	To compute a histogram and display on an X Windows display:
##		hist image | grhist | xplot
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##	The output of grhist is constrained by the output options of the
##	graph(1) program.
##
## FUTURE DIRECTIONS
##	grhist is implemented as the pipeline
##
##		primg | awk | graph
##
##	It may be desirable to reimplement grhist as a single program.
##
##	An implementation of grhist in Perl has been written by Dana
##	Jacobsen, ERL-C which is faster and smaller.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB.
##	4/5/93	Removed non standard empty quotes passed to graph. This
##		broke some graph programs.  Dana Jacobsen, CSC, ERL-C.
##
## BUGS
##
## SEE ALSO
##	IPW:  btoa, hist, rmhdr
##	UNIX: awk, graph, plot
##	GNU:  xplot, graph, graphics, gnuplot
##	Image: pgmhist, ppmhist

PATH="$PATH:$IPW/lib"
. ipwenv

case $* in
-H)	exec usage $0 '[graph-options]' 'plot a histogram'
	;;
esac

if test -t 0; then
	exec usage $0 '[graph-options]' 'plot a histogram'
fi

primg -r -a |
	$AWK '$1 != 0 {
		print NR - 1, 0
		print NR - 1, $1
	}' |
	graph -b $*

# Used to be quotes on the end of the second print statement:
#		print NR - 1, $1, "\" \""
# but that doesn't do anything with normal graph and breaks GNU graph
# which is necessary on DG/UX.

exit 0

# $Id: grhist.sh,v 1.5 90/11/16 16:51:31 frew Exp $
