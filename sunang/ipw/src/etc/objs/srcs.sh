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
##	objs,srcs -- list of .o,.[cfF] files for use in Makefile
##
## SYNOPSIS
##	objs
##	srcs
##
## DESCRIPTION
##	These scripts return a nicely formatted list of the either the
##	source files ({srcs}) or the object files ({objs}), suitable for
##	bringing into a Makefile.
##
## OPTIONS
##
## EXAMPLES
##	In vi, the following is often used to generate the SRCS line:
##
##		Enter "SRCS=\", hit escape, then type ":r !srcs"
##
##	This sets the 'SRCS' variable to a list of all the source files.
##	Remember to include the backslash after the '=' sign -- it is
##	necessary to continue the line.
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	objs is currently implemented as a shell script.
##	It may be desirable to reimplement objs as a program.
##
## HISTORY
##	11/16/90 Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO

PATH="$PATH:$IPW/lib"
. ipwenv

optstring=
synopsis='[dir ...]'

case `basename $0` in
objs)	sed_cmd='s/\..*/.o/'
	description='list of .o files for use in Makedefs'
	;;
srcs)	sed_cmd='s/,v$//'
	description='list of .[cfF] files for use in Makedefs'
	;;
*)	sherror $0 'My name is either "objs" or "srcs"!'
	exit 1
	;;
esac

set -- `getopt "$optstring" $* 2>/dev/null` || {
	usage $0 "$synopsis" "$description"
	exit 1
}
shift				# get rid of '--' from getopt

case $# in
0)	ls *.[cfF] *.[cfF],v 2>/dev/null
	;;
*)	for dir do
		ls $dir/*.[cfF] $dir/*.[cfF],v 2>/dev/null
	done
	;;
esac |
	sed -e "$sed_cmd" |
	{
		echo .ll 69	# (79 cols) - (leading tab) - (trailing ' \')
		echo .na
		echo .nh
		echo .pl 1
		sort -u
	} |
	nroff |
	sed '
		/^$/d
		s/^/	/
		s/$/ \\/
		$s/ \\$//
	'

exit 0

# $Header: /local/share/pkg/ipw/src/etc/objs/RCS/objs.sh,v 1.2 90/11/16 16:52:15 frew Exp $
