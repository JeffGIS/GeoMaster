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
##	ipw -- list IPW commands
##
## SYNOPSIS
##	ipw
##
## DESCRIPTION
##	ipw prints a nicely-formatted listing of the currently-available
##	IPW commands on the standard output.
##
## OPTIONS
##
## EXAMPLES
##
## FILES
##	$IPW/lib/bins
##
##		This file contains a list of directories, relative to $IPW,
##		that contain executable IPW commands and scripts.  The format
##		of this file is:
##
##			name	description
##
##		The default version of this file is:
##
##			bin	general-purpose
##			etc	maintenance and support
##
## DIAGNOSTICS
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	ipw is currently implemented as a shell script.
##	It may be desirable to reimplement ipw as a program.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO

PATH=$PATH:$IPW/lib
. ipwenv

case $# in
0)	;;
*)	exec usage $0 '' 'list IPW command directories'
	;;
esac

ipw_bins=$IPW/lib/bins

grep -v '#' $ipw_bins |
	while read bin descrip; do
		bin=`eval echo $bin`
		case $bin in
		/*)	;;
		*)	bin=$IPW/$bin
			;;
		esac
		if test -d $bin  -a  -r $bin; then
			echo "
IPW $descrip commands:
"
			(cd $bin && eval $LC_WD)
		fi
	done

echo '
----------------------------------------------
Type "command -H" for a synopsis of "command".'

# $Header: /local/share/pkg/ipw/src/bin/ipw/RCS/ipw.sh,v 1.12 90/11/16 16:51:45 frew Exp $
