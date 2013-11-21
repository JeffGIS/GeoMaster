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
##	cmpimg -- compare two images
##
## SYNOPSIS
##	cmpimg image1 image2
##
## DESCRIPTION
##	cmpimg compares the pixel data from {image1} and {image2}, ignoring
##	any headers.  A "-" in place of an image represents the standard
##	input.  The output of cmpimg is a message printed to the standard
##	output indicating whether the two input images differ or are
##	identical.
##
## OPTIONS
##
## EXAMPLES
##	The following sequence might be used to test a command, such as
##	transpose, that should be its own inverse:
##
##		transpose image | transpose | cmpimg - image
##
## FILES
##	$TMPDIR/cmpimg{NNNNN}
##
##		Temporary storage for a headerless copy of {image1}.
##
##	$TMPDIR/cmpimg{NNNNN}2
##
##		Temporary storage for a headerless copy of {image2}.
##
## DIAGNOSTICS
##	Images "{image1}", "{image2}" are identical
##
##	Images "{image1}", "{image2}" differ
##
## RESTRICTIONS
##	cmpimg is implemented as a shell script that invokes the UNIX cmp
##	command.  This has the following consequences:
##
##		Since cmp has no notion of image structure, cmpimg gives no
##		indication of the logical image locations (line, sample, band)
##		where differences occur.
##
##		Some implementations of cmp do not allow "-" (standard input)
##		to be specified for {image2}.
##
##	The exit status of cmpimg does NOT indicate the result.
##
## FUTURE DIRECTIONS
##	It may be desirable to reimplement cmpimg as a program.
##
## HISTORY
##	7/1/90  Written by James Frew, UCSB
##	4/1/93	Use second temporary file to avoid a spurious newline.
##		Dana Jacobsen, ERL-C.
##
## BUGS
##
## SEE ALSO
##	IPW:  rmhdr
##	UNIX: cmp

PATH="$PATH:$IPW/lib"
. ipwenv

pgm=`basename $0`
optstring=
synopsis='image1 image2'
description='compare two images'

set -- `getopt "$optstring" $* 2>/dev/null` ||
	exec usage $pgm "$synopsis" "$description"

while :; do
	case $1 in
	--)	shift
		break
		;;
	*)	exec sherror $pgm '"getopt" failed'
		;;
	esac

	shift
done

case $# in
2)	;;
*)	exec usage $pgm "$synopsis" "$description"
	;;
esac

case $2 in
-)	exec usage $pgm "$synopsis" "$description"
	;;
esac

tmp=$TMPDIR/`basename $0`$$
tmp2=$TMPDIR/`basename $0`$$-2
trap 'rm -f $tmp $tmp2' 0
trap 'exit 0' 1 2 3 15

case $1 in
-)	rmhdr >$tmp || exit 1
	;;
*)	rmhdr $1 >$tmp || exit 1
	;;
esac

rmhdr $2 > $tmp2 || exit 1
cmp -s $tmp2 $tmp

case $? in
0)	echo "Images \"$1\", \"$2\" are identical"
	;;
*)	echo "Images \"$1\", \"$2\" differ"
	;;
esac

exit 0

# $Header: /local/share/pkg/ipw/src/bin/cmpimg/RCS/cmpimg.sh,v 1.2 90/11/16 16:51:23 frew Exp $
