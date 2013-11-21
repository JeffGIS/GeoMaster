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
##	prhdr -- print IPW image headers
##
## SYNOPSIS
##	prhdr [image ...]
##
## DESCRIPTION
##	prhdr copies the IPW headers of the input {image}s (default:
##	standard input) to the standard output.  If more than one
##	{image} is specified, then each group of output headers will
##	be preceded by:
##
##		::::::::::::::
##		{image}
##		::::::::::::::
##
## OPTIONS
##
## EXAMPLES
##	The command:
##
##		prhdr image
##
##	might produce something like the following output:
##
##		!<header> basic_image_i -1 $Revision: 1.8 $
##		byteorder = 3210
##		nlines = 96
##		nsamps = 281
##		nbands = 1
##		!<header> basic_image 0 $Revision: 1.8 $
##		bytes = 1
##		bits = 8
##		annot = Calculated NDVI for Period 03/02 thru 03/15/1990
##		history = indvi
##		!<header> image -1 $Revision: 1.5 $
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##	IPW headers are printable text and are always separated from the
##	image data by a form feed (ASCII NP) character.  You can therefore
##	view the header of any IPW image directly with a pagination
##	command such as "more" that pauses when it encounters a form feed.
##
## FUTURE DIRECTIONS
##	prhdr is currently implemented as a shell script.
##	It may be desirable to reimplement prhdr as a program.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	IPW:  mk*h, rmhdr
##	UNIX: less, more, sed

PATH="${PATH}:${IPW}/lib"
. ipwenv

case $1 in
'')	set -- -
	;;
-)	;;
-*)	exec usage $0 '[file ...]' 'print IPW image headers'
	;;
esac

for file do
	case $# in
	0|1)	;;
	*)	echo "::::::::::::::
$file
::::::::::::::"
		;;
	esac

	case $file in
	-)	file=
		;;
	*)	test -r $file || exec sherror $0 "can't open file" $file
		;;
	esac

	sed '
		// {
			s///
			q
		}
	' $file
done

exit 0

# $Header: /local/share/pkg/ipw/src/bin/prhdr/RCS/prhdr.sh,v 1.15 90/11/16 16:51:50 frew Exp $
