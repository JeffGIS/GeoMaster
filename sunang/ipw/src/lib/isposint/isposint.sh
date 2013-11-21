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
##	isposint -- test whether argument is a positive nonzero integer
##
## SYNOPSIS
##	isposint argument
##
## DESCRIPTION
##	isposint tests whether its single argument is a postive nonzero
##	decimal integer.  
##
##	isposint is used mainly as an argument checker by IPW shell scripts.
##
## OPTIONS
##
## EXAMPLES
##
## FILES
##
## DIAGNOSTICS
##	exit status {,non-}0 if argument is{, not} a positive nonzero integer
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	isposint is currently implemented as a shell script.
##	It may be desirable to reimplement isposint as a program.
##
## BUGS
##	The treatment of 0 may offend mathematical purists.

PATH="$PATH:$IPW/lib"
. ipwenv

case $# in
1)	;;
*)	exec usage $0 'arg' \
		'test whether argument is a positive nonzero integer'
	;;
esac

test "$1" || exit 1

i=`expr "$1" : '\([1-9][0-9]*\)' 2>/dev/null`
test "$i" = "$1"

exit $?

# $Header: /local/share/pkg/ipw/src/lib/isposint/RCS/isposint.sh,v 1.2 90/11/16 16:52:20 frew Exp $
