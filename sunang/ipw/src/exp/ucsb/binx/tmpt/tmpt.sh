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
##      tmpt -- read Thematic Mapper (TM) image from tape
##
## SYNOPSIS
##	tmpt [-b band] 
##
## DESCRIPTION
##	tmpt reads a TM image off of tape and writes an IPW file.
##
## OPTIONS
##	-b	Which band to read (default: 1).
##
## EXAMPLES
##
## FILES
##	The output file will be 2983 lines by 4220 samples.  It will have
##	"lq" (linear quantization), "sun", and "sat" headers.
##
## DIAGNOSTICS
##
## RESTRICTIONS
##	The tape device read is /dev/nrmt0.
##
## FUTURE DIRECTIONS
##
## HISTORY
##	11/1/90	Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	UNIX:  dd

PATH="$PATH:$IPW/lib"
. ipwenv

optstring='b:'
synopsis='[-b band]'
description='read tm image from tape'

# get command-line arguments

band=1

set -- `getopt "$optstring" $* 2>/dev/null` ||
	exec usage $0 "$synopsis" "$description"

	if test -t 1; then
		sherror $0 "can't write image to a terminal"
		exec usage $0 "$synopsis" "$description"
	fi

while :; do
	case $1 in
	--)     shift
		break
	  	;;
	-b)	band=$2
		break
		;;
	esac
	shift
done

mt rew

offset=`expr $band - 1`
offset=`expr $offset \* 3`
offset=`expr $offset + 1`

mt fsf $offset

hdr=$TMPDIR/`basename $0`$$
trap 'rm -f $hdr' 0
trap 'exit 0' 1 2 3 15

	# read image header ("leader file") from tape

	dd bs=4320 < /dev/nrmt0 2>/dev/null > $hdr
	
	# get info for satellite header from
	# "scene header record"

	sat=`
	dd bs=4320 skip=1 count=1 < $hdr 2>/dev/null | \
	dd bs=116 skip=1 count=1 2>/dev/null | \
	$AWK '{
		landsat = substr($0, 49, 1)
		path = substr($0, 50, 3)
		row = substr($0, 53, 3)
		year = substr($1, 1, 4)
		month = substr($1, 5, 2)
		day = substr($1, 7, 2)
		hour = substr($1, 9, 2)
		min = substr($1, 11, 2)
		printf("%d ", landsat)
		printf("%s%s ", path, row)
		printf("%s%s%s ", year, month, day)
		printf("%s%s\n", hour, min)
	}'`
#	echo $sat > /dev/tty
	set $sat
	landsat=$1
	loc=$2
	date=$3
	time=$4
	
	# get info for lq header from
	# "radiometric calibration ancillary record"

	lq=`
	dd bs=4320 skip=3 < $hdr 2>/dev/null | \
	dd bs=17 skip=1 count=1 2>/dev/null | \
	$AWK '{print $1, $2}'`
	set $lq
	min=`expr 10 \* $1`
	max=`expr 10 \* $2'`
#	echo $min
#	echo $max
	
	# get info for sun header from
	# "map projection ancillary record"

	sun=`
	dd bs=4320 skip=2 count=1 < $hdr 2>/dev/null | \
	dd bs=605 skip=1 count=1 2>/dev/null | \
	$AWK '{
		pi = atan2(0,-1)
		dtr = pi / 180
		zen = (90 - $1) * dtr
		azm = (180 - $2) * dtr
		printf("%f %f\n", cos(zen), azm)
	}'`
	set $sun
	cos_zen=$1
	azm=$2
#	echo $cos_zen
#	echo $azm
	
	# make ipw headers

	{
	mkbih -s 4320 -l 2984 -f  | \
	mklqh -m 0,$min,255,$max -f | \
	mksunh -a $azm -z $cos_zen -f | \
	mksath -p "landsat $landsat" -s "tm $band" -l $loc -d $date -t $time -f
	
	# read image from tape
	# window actual image
	# (1st line, 1st 33 pixels, last 52 pixels of each line are header info)

	dd bs=4320 < /dev/nrmt0 2>/dev/null
	} | \
	window -b 1,33 -n 2983,4220

# $Header: /local/share/pkg/ipw/src/bin/tmpt/RCS/tmpt.sh,v 1.5 90/11/16 16:51:52 frew Exp $
