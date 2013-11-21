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
##	ipwmake -- IPW "make" command
##
## SYNOPSIS
##	ipwmake [-DP] [make_options] target ...
##
## DESCRIPTION
##	ipwmake encapsulates the UNIX "make" command for IPW shell scripts,
##	programs, and libraries.  ipwmake constructs a makefile out of lots of
##	shared boiler-plate in $IPW/lib/make, plus a minimum of specific
##	information in ./Makedefs.
##
##	If the environment variable MAKEDEFS is set, then the file it names
##	used in place of the default "Makedefs" file.  In particular, setting
##	MAKEDEFS to "-" causes ipwmake to read control information from its
##	standard input.
##
## OPTIONS
##	-D	compile target for debugging
##
##	-P	compile target for profiling
##
## EXAMPLES
##
## FILES
##	Makedefs		information about specific objects to be made
##	$IPW/lib/make/local	locally-tuned macros
##	$IPW/lib/make/std	standard macros
##	$IPW/lib/make/debug	debugging macros
##	$IPW/lib/make/profile	profiling macros
##	$IPW/lib/make/rules	rules
##	$IPW/skel/makedefs.sh	skeleton "Makedefs" for shell scripts
##	$IPW/skel/makedefs.pgm	skeleton "Makedefs" for programs
##	$IPW/skel/makedefs.lib	skeleton "Makedefs" for object libraries
##
## DIAGNOSTICS
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##
## BUGS

PATH=$PATH:$IPW/lib
. ipwenv

make_lib=$IPW/lib/make

macros_local=$make_lib/local
macros_std=$make_lib/std
rules=$make_lib/rules
makedefs=${MAKEDEFS-Makedefs}

case $1 in
-H)	usage $0 '[-DP] [make_options] target ...' 'IPW "make" command'
	exit 1
	;;
-D)	macros_option=$make_lib/debug
	opts=-D
	shift
	;;
-P)	macros_option=$make_lib/profile
	opts=-P
	shift
	;;
*)	macros_option=
	opts=
	;;
esac

# If Makedefs doesn't exist, try getting it out of SCCS
 
if test $makedefs != '-' -a ! -s $makedefs; then
	if test -d "SCCS" ; then 
		sccs get -s Makedefs -GMakedefs
	fi
fi

if test $makedefs != '-' -a ! -s $makedefs; then
	sherror $0 "no $makedefs file"
	exit 1
fi

cat $macros_local $macros_std $macros_option $makedefs $rules |
	make -f - ${*-default} IPW=$IPW IPWMAKE_OPTS="$opts" IPWMAKE_ARGS="$*"

exit 0

# $Header: /local/share/pkg/ipw/src/etc/ipwmake/RCS/ipwmake.sh,v 1.10 90/11/16 16:52:02 frew Exp $
