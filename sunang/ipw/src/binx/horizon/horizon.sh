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
##	horizon -- compute horizon in specified direction
##
## SYNOPSIS
##	horizon -a azimuth [-d delta] [-z zen] [-u cos] [image]
##
## DESCRIPTION
##	horizon computes the local horizon angles toward the direction
##	azimuth, where azimuth=0 is toward the south and positive
##	angles are counter-clockwise.
##
##	horizon reads elevations from {image} (default: standard input)
##	and writes to the standard output an image whose pixels encode
##	the local horizon angles in the direction {azimuth} degrees
##	(ranging from -180 to 180) from south (positive east).  The
##	value of each output pixel is the cosine of the angle from the
##	zenith to the pixel's horizon in the forward (increasing sample
##	coordinates) direction.  (Note than this value is also the sine
##	of the angle from true horizontal to the pixel's horizon.)
##
## OPTIONS
##	-a	The direction of forward azimuth (i.e. increasing samples
##		along a line) is {azimuth} degrees east of south (-180..180).
##
##	-d	grid spacing (default: get grid spacing from the "geo"
##		header or set to 1 if no "geo" header).  Must be greater
##		than 0.  The units should be the same as for the elevations.
##
##	The following options change the output from linearly quantized
##	cosines to a 1-bit mask in which 1's indicate horizon angles
##	greater than a specified threshold.  They are typically used to
##	specify a solar zenith angle, the output being a mask of pixels
##	where the sun is visible.
##
##	-z	mask horizon angles with greater than {zen} degrees (0..90).
##
##	-u	mask horizon angles with cosines greater than {cos}.
##
## EXAMPLES
##	To compute northwest horizons:
##
##		horizon -a -135
##
##	To produce a mask of all northwest horizon angles greater than
##	45 degrees:
##
##		horizon -a -135 -z 45
##
##	(i.e., any pixels that would be shadowed by adjacent terrain at
##	this solar zenith and azimuth would be masked as 0.)
##
## FILES
##	$TMPDIR/horizon{NNNNN}
##
##		temporary command file, removed when horizon exits
##
## DIAGNOSTICS
##	spacing in geodetic header ignored
##
##		The -d option overrides any pixel spacing information
##		in the input image.
##
##	both -u and -z specified, -z over-ridden
##
##		If both -u and -z are specified then -z is ignored.
##
##	input file has {nbands} bands
##
##		The input image must have only 1 band.
##
##	only 1 line in input image
##	only 1 samp in input image
##
##		The input image must have at least 2 lines and 2 samples.
##
##	input file has no LQH, raw values used
##	no geodetic header, spacing set to 1.0
##
##		These deficiencies in the input image will introduce
##		linear errors into the horizon calculations.
##
## RESTRICTIONS
##	horizon is a shell script than skews and/or transposes the input
##	image to orient its scan lines in the direction {azimuth}, then
##	calls hor1d to perform the actual horizon calculations.
##
## FUTURE DIRECTIONS
##	It may be desirable to reimplement horizon as a program.
##
## HISTORY
##	7/1/90	Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	IPW:  hor1d, skew, transpose
##
##	J. Dozier, J. Bruno, and P. Downey, "A faster solution to the
##		horizon problem", Computers and Geosciences, volume 7,
##		number 2, pp. 145-151, 1981.

PATH="$PATH:$IPW/lib"
. ipwenv

pgm=`basename $0`
optstring='a:d:z:u:'
synopsis='-a azimuth [-d delta] [-z zenith] [-u cos] [image]'
description='find horizon in direction "azimuth"'

set -- `getopt "$optstring" $* 2>/dev/null` || {
	usage $pgm "$synopsis" "$description"
	exit 1
}

phi=
delta=0
zen=0
cosZ=0

# parse options

while :; do
	case $1 in
	--)	shift
		break
		;;
	-H)	usage $pgm "$synopsis" "$description"
		exit 1
		;;
	-a)	phi=$2
		shift
		;;
	-d)	delta=$2
		shift
		;;
	-z)	zen=$2
		shift
		;;
	-u)	cosZ=$2
		shift
		;;
	*)	sherror $pgm '"getopt" failed'
		exit 1
		;;
	esac
	shift
done

case $phi in
'')	usage $pgm "$synopsis" "$description"
	exit 1
	;;
esac

# can only have 1 input image

image="-"
for image do
	case $# in
	0|1)	;;
	*)	usage $pgm "$synopsis" "$description"
		exit 1
		;;
	esac

	case $image in
	-)	;;
	*)	test -r $image || {
			sherror $pgm "can't open file", $image
			exit 1
		}
		;;
	esac
done

# can't write image data to terminal

test -t 1 && {
	sherror $pgm "can't write image data to a terminal"
	exit 1
}

# see if we can remove temp file if we get stopped early
trap 'rm -f $TMPDIR/$pgm$$' 0
trap 'exit 0' 1 2 3 15

# awk script creates output command string and puts in temporary file

echo $phi $delta $zen $cosZ $image | $AWK '
	{
		phi = $1
		delta = $2
		zen = $3
		cosZ = $4
		image = $5
		if (substr(image,1,1) == "#")
			image = sprintf("\\%s", $5);
		skew = "skew -a"
		unskew = "skew"
		if (delta == 0) {
			ds = ""
		}
		else {
			ds = sprintf(" -d %g", delta);
		}
		if (zen == 0) {
			dz = ""
		}
		else {
			dz = sprintf(" -z %g", zen);
		}
		if (cosZ == 0) {
			dc = ""
		}
		else {
			dc = sprintf(" -u %g", cosZ);
		}
		hf = "hor1d" ds dz dc " -a"
		hb = "hor1d -b" ds dz dc " -a"
		xp = "transpose"
		if (phi == 90)
			print hf, phi, image
		else if (phi == -90)
			print hb, -phi, image
		else if (phi == 0)
			print xp, image, "|", hf, phi, "|", xp
		else if (phi == -180)
			print xp, image, "|", hb, phi+180, "|", xp
		else if (phi == 180)
			print xp, image, "|", hb, phi-180, "|", xp
		else if (-45 <= phi && phi <= 45)
			print skew, phi, image, "|", xp, "|", hf, phi, "|", xp, "|", unskew
		else if (-135 >= phi && phi > -180)
			print skew, phi+180, image, "|", xp, "|", hb, phi+180, "|", xp, "|", unskew
		else if (180 > phi && phi >= 135)
			print skew, phi-180, image, "|", xp, "|", hb, phi-180, "|", xp, "|", unskew
		else if (45 < phi && phi < 135)
			print xp, image, "|", skew, 90-phi, "|", xp, "|", hf, phi, "|", xp, "|", unskew, "|", xp
		else if (-45 > phi && phi > -135)
			print xp, image, "|", skew, -90-phi, "|", xp, "|", hb, phi+180, "|", xp, "|", unskew, "|", xp
		else
			print "error: phi =", phi
	}' > $TMPDIR/$pgm$$

# run command string from temporary file

chmod +x $TMPDIR/$pgm$$
sh -c $TMPDIR/$pgm$$

# remove temporary file

rm -f $TMPDIR/$pgm$$

exit 0

# $Header: /local/share/pkg/ipw/src/bin/horizon/RCS/horizon.sh,v 1.5 90/11/16 16:51:36 frew Exp $
