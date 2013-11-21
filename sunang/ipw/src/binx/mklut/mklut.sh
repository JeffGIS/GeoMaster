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
##	mklut -- make look-up table
##
## SYNOPSIS
##	mklut [-i in_nbits] [-o out_nbits] [-k bkgd]
##
## DESCRIPTION
##	mklut creates an IPW lookup table and writes it to the standard
##	output.  The lookup table is filled in with text values read from
##	the standard input in the form:
##
##		in	out
##
##	mklut sets the {in}th element (relative to 0) of the lookup table
##	to {out}.  The input lines must be sorted so that the {in} values
##	are in numerically ascending order.
##
##	Unreferenced LUT locations are set to a default value.
##
## OPTIONS
##	-i	number of bits per input image pixel (default: 8).  The
##		output lookup table will contain 2^{in_nbits} entries.
##
##	-o	number of bits per output image pixel (default: 8).  The
##		output lookup table will contain {out_nbits}-bit pixels.
##
##	-k	default output value (default: 0).
##
##	-i, -o, and -k must all have positive non-zero integer arguments.
##
## EXAMPLES
##	To convert "image" with 12-bit pixels to 8-bit pixels with linear
##	scaling:
##
##		interp | mklut -i 12 | lutx -i image
##		0 0
##		4095 255
##
## FILES
##
## DIAGNOSTICS
##	can't write lookup table to a terminal
##
##		mklut will not write a lookup table to a terminal device.
##
##	{arg}: not a positive integer
##
##		All arguments must be positive non-zero integers.
##
##	{in_nbits}: too many bits per input pixel
##
##		A 2^{in_nbits}-entry lookup table won't fit in memory.
##
##	mkbih: WARNING: input image larger than header indicates
##
##		mklut was given an image with more values than would fit
##		in the input number of bits.
##
## RESTRICTIONS
##	Only single-band LUTs may be created.
##
## FUTURE DIRECTIONS
##	mklut is currently implemented as a shell script.
##	It may be desirable to reimplement mklut as a program.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	IPW:  interp, lutx
##	UNIX: sort

PATH="$PATH:$IPW/lib"
. ipwenv

optstring='i:o:k:'
synopsis='[-i in_nbits] [-o out_nbits] [-k bkgd]'
description='make look-up table'

# get command-line arguments

set -- `getopt "$optstring" $* 2>/dev/null` ||
	exec usage $0 "$synopsis" "$description"

if test -t 1; then
	sherror $0 "can't write lookup table to a terminal"
	exec usage $0 "$synopsis" "$description"
fi

ibits=8
obits=8
const=0

while :; do
	case $1 in
	--)	shift
		break
		;;
	-i)	ibits=$2
		isposint $ibits ||
			exec sherror $0 "$ibits: not a positive integer"
		shift
		;;
	-o)	obits=$2
		isposint $obits ||
			exec sherror $0 "$obits: not a positive integer"
		shift
		;;
	-k)	const=$2
		isposint $const ||
			exec sherror $0 "$const: not a positive integer"
		shift
		;;
	*)	exec sherror $pgm '"getopt" failed'
		;;
	esac
	shift
done
#DEBUG echo "ibits=$ibits"
#DEBUG echo "obits=$obits"
#DEBUG echo "const=$const"

obytes=`echo $obits |
        $AWK '{
                nbytes = 1

                for (nbits = 8; $1 > nbits; nbits *= 2) {
                        nbytes *= 2
                }

                print nbytes
        }'`

nsamps=`echo "2 ${ibits}^p" |
	dc`

#isposint $nsamps || exec sherror $0 "$ibits: too many bits per input pixel"
#DEBUG echo "nsamps=$nsamps"

{
	echo $nsamps $const
	cat
} |
	$AWK '
		NR == 1 {
			max = $1
			k = $2
			next
		}
		{
			for (; i < $1; ++i)
				print k

			print $2
			++i
		}
		END {
			for (; i < max; ++i)
				print k
		}
	' |
	atob -$obytes |
	mkbih -l 1 -s $nsamps  -i $obits 

exit $?

# $Header: /local/share/pkg/ipw/src/bin/mklut/RCS/mklut.sh,v 1.5 90/11/16 16:51:47 frew Exp $
