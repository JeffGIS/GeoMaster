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
##	sherror -- standard IPW error message for shell scripts
##
## SYNOPSIS
##	sherror pgm message [file]
##
## DESCRIPTION
##	sherror is an error message generator that may be called by IPW shell
##	scripts.  The error messages are printed in a similar format to the
##	IPW error message standard for C programs.
##
## OPTIONS
##
## EXAMPLES
##	sherror $0 "$1: unsupported option"
##	sherror $0 "can't access file" $filename
##
## FILES
##
## DIAGNOSTICS
##	always exits with nonzero status
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	sherror is currently implemented as a shell script.
##	It may be desirable to reimplement sherror as a program.
##
## BUGS

PATH="$PATH:$IPW/lib"
. ipwenv

exec 1>&2

case $0 in
*bug)	severity=BUG
	;;
*)	severity=ERROR
	;;
esac

case $# in
2)	echo "
`basename $1`: $severity:
	$2
"
	;;
3)	echo "
`basename $1`: $severity:
	file "$3": $2
"
	;;
*)	echo 'ERROR: wrong # args!'
	;;
esac

exit 1

# $Header: /local/share/pkg/ipw/src/lib/sherror/RCS/sherror.sh,v 1.5 90/11/16 16:52:22 frew Exp $
