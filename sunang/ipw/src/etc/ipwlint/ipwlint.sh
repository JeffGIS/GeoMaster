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
##	ipwlint -- run lint on IPW source files
##
## SYNOPSIS
##	ipwlint [ipwmake-options] source-file ...
##
## DESCRIPTION
##	ipwlint is a front-end for ipwmake that allows you to run lint on
##	specific IPW source files (as opposed to "ipwmake lint", which always
##	processes all the source files mentioned in ./Makedefs).
##
## OPTIONS
##	(same as ipwmake)
##
## EXAMPLES
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	ipwlint is currently implemented as a shell script.
##	It may be desirable to reimplement ipwlint as a program.
##
## BUGS

PATH="$PATH:$IPW/lib"
. ipwenv

synopsis='[ipwmake-options] source-file ...'
description='run lint on IPW source files'

while :; do
	case $1 in
	''|-H)	usage $0 "$synopsis" "$description"
		exit 1
		;;
	-*)	opts="$opts $1"
		;;
	*)	break
		;;
	esac
	shift
done

MAKEDEFS=-
export MAKEDEFS

echo "SRCS=$*" |
	ipwmake $opts lint

# $Header: /local/share/pkg/ipw/src/etc/ipwlint/RCS/ipwlint.sh,v 1.3 90/11/16 16:51:59 frew Exp $
