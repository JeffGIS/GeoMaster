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
##	histeq -- make histogram-equalization look-up table
##
## SYNOPSIS
##	histeq [-n size] [-o min,max] [-i min,max] [-f floor] [-c ceil]
##
## DESCRIPTION
##	histeq reads an IPW histogram from the standard input and writes
##	an IPW lookup table to the standard output.
##
##	The lookup table may be applied to the original image (with the
##	IPW lutx command) to produce a "histogram equalized" image
##	(i.e. an image with a nearly-flat histogram).
##
## OPTIONS
##	-n 	number of elements in the histogram (default: 256)
##
##	-i	minimum and maximum values to use from the input histogram
##		(default: 0, {size}-1)
##
##	-o	minimum and maximum values to appear in the output look-up
##		table (default: 0, {size}-1)
##
##	-f	Have the output lookup table map input values less than the
##		specified minimum to {floor} (default: output minimum)
##
##	-c	Have the output lookup table map input values greater than the
##		specified maximum to {ceil} (default: output maximum)
##
## EXAMPLES
##	To produce a histogram-equalized version of "image":
##
##		hist image | histeq | lutx -i image
##
##	If "image" has 12-bit pixels, and the output image should have 8-bit
##	pixels, then replace the "histeq" command above with:
##
##		histeq -n 4096 -o 0,255
##
##	To avoid having extreme values overwhelm the output mapping, they
##	could be excluded by specifying an input range:
##
##		histeq -n 4096 -o 0,255 -i 1,4094
##
## FILES
##
## DIAGNOSTICS
##	can't read histogram from a terminal
##	can't write lut to a terminal
##
##		histeq will not read from or write to a terminal.
##	
##	bad option causes 0-bit output values
##	bad option causes {nbytes}-byte output values
##
##		One or more of the options would have resulted in a lookup
##		table with pixels outside the permissable size range of
##		1 .. 32 bits.
##
## RESTRICTIONS
##	The histogram is always read from the standard input.  No histogram
##	file operand is accepted.
##
##	The -n option exists only because histeq is implemented as a
##	shell script and does not read the histogram's header.
##
## FUTURE DIRECTIONS
##	It may be desirable to reimplement histeq as a program.
##
##	Use ipwfile to determine the size of the histogram if the -n option
##	is not specified.
##
## HISTORY
##	3/1/90	Written by James Frew, UCSB.
##	11/1/90	Changed input syntax, James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	IPW:   cnhist, hist, lutx
##	UNIX:  awk
##	Image: ppmhist, xv
##
##	William K. Pratt, "Digital Image Processing", 2nd edition, John Wiley,
##		1991, pp. 275-284.   (1978 edition, pp. 311-318)

PATH="$PATH:$IPW/lib"
. ipwenv

synopsis='[-n size] [-o min,max] [-i min,max] [-f floor] [-c ceil]'
descrip='make histogram-equalization look-up table'

# get command-line arguments

set -- `getopt 'n:o:i:f:c:' $* 2>/dev/null` ||
	exec usage $0 "$synopsis" "$descrip"

# avoid binary I/O on tty

test -t 0 && exec sherror $0 "can't read histogram from a terminal"
test -t 1 && exec sherror $0 "can't write lut to a terminal"

# parse options

size=256
o_args=
i_args=
floor=
ceil=

while :; do
	case $1 in
	--)	shift
		break
		;;
	-n)	size=$2
		shift
		;;
	-o)	o_args=$2
		shift
		;;
	-i)	i_args=$2
		shift
		;;
	-f)	floor=$2
		shift
		;;
	-c)	ceil=$2
		shift
		;;
	*)	exec shbug $0 "'getopt' failed"
		;;
	esac
	shift
done

# [[ There should be some code here to ensure that size is a power of 2. ]]

case $o_args in
'')	out_lo=0
	out_hi=`expr $size - 1`
	;;
*)	IFS_save="$IFS"
	IFS=,
	set,$o_args
	out_lo=$1
	out_hi=$2
	IFS="$IFS_save"
	;;
esac

case $i_args in
'')	in_lo=0
	in_hi=`expr $size - 1`
	;;
*)	IFS_save="$IFS"
	IFS=,
	set,$i_args
	in_lo=$1
	in_hi=$2
	IFS="$IFS_save"
	;;
esac

case $floor in
'')	floor=$out_lo
	;;
esac

case $ceil in
'')	ceil=$out_hi
	;;
esac

# derived parameters

nbits=`echo $ceil |
	$AWK '{
		nbits = 0;
		
		for (ceil = $1; ceil >= 1; ceil /= 2) {
			++nbits;
		}

		print nbits
	}'`

case $nbits in
0)	exec sherror $0 "bad option causes 0-bit output values"
	;;
esac

nbytes=`echo $nbits |
	$AWK '{
		nbytes = 1

		for (nbits = 8; $1 > nbits; nbits *= 2) {
			nbytes *= 2
		}

		print nbytes
	}'`

case $nbytes in
1|2|4)	;;
*)	exec sherror $0 "bad option causes ${nbytes}-byte output values"
	;;
esac

# write lookup table header to stdout

mkbih -l 1 -s $size  -i $nbits -f

# { equalization parameters;
#   normalized cumulative histogram < IPW histogram on stdin } |
# awk script generates LUT |
# convert to binary

{
	echo $in_lo $in_hi $out_lo $out_hi $floor $ceil
	cnhist $in_lo $in_hi
} | 
	$AWK '
		NR == 1 {
			in_lo = $1
			in_hi = $2
			out_lo = $3
			out_hi = $4
			floor = $5
			ceil = $6

			scale = out_hi - out_lo
			next
		}

		{ in_val = NR - 2 }

		in_val < in_lo {
			print floor
			next
		}

		in_val <= in_hi {
			print int(scale * $1 + out_lo) 
			next
		}

		in_val > in_hi {
			print ceil
		}
	' | 
	atob -$nbytes

exit 0

# $Header: /local/share/pkg/ipw/src/bin/histeq/RCS/histeq.sh,v 1.9 90/11/16 16:51:33 frew Exp $
