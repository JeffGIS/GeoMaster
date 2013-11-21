: ${IPW?}

## NAME
##	topquad -- daily integrated radiation over topographic grid
##
## SYNOPSIS
##	topquad [-n] -z elev -t tau -w omega -g gfact -r R0 [-s S0]
##		[-x w1,w2] -d y,m,d -b d[,m,s] -l d[,m,s] [image]
##
## DESCRIPTION
##	topquad calculates daily integrated radiation over a topographic
##	grid, using a two-stream atmospheric radiation model and 21-point
##	Kronrod quadrature between sunrise and sunset.  The input image
##	has the following bands:
##
##		elevation
##		slope
##		aspect
##		sky view factor
##		terrain configuration factor
##		surface albedo
##
## OPTIONS
##	-n	Calculate net radiation (default: calculate incoming
##		radiation).
##
##	-z	The elevation of optical depth measurement is {elev} meters.
##
##	-t	The optical depth at {elev} is {tau}.
##
##	-w	The single-scattering albedo is {omega}.
##
##	-g	The scattering asymmetry factor is {gfact}.
##
##	-r	The mean surface albedo is {R0}.
##
##	-s	The exoatmospheric solar irradiance is {S0} (default: use
##		"solar" to calculate this from the -x range).
##
##	-x	The wavelength range, in micrometers, is {w1}-{w2} (default:
##		no wavelength range; the solar irradiance is specified).
##
##	At least one of -s and/or -x must be specified.
##
##	-d	The date is {year}, {month}, {day}.
##
##	-b	The latitude is {d} degrees, {m} minutes, {s} seconds.
##
##	-l	The longitude is {d} degrees, {m} minutes, {s} seconds.
##
## EXAMPLES
##	Running topquad over a section of the Columbia Basin on June 22, 1990,
##	with input files muxed together:
##
##		mux elev slope-aspect skyview-terrain albedo |  \
##		topquad -z 200 -t .2 -w .85 -g .55 -r .1244 -x .58,.68  \
##			-d 1990,6,22 -b 48 -l -119,30  > rad.incoming
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##	topquad is implemented as a shell script that calls many different
##	IPW routines to accomplish it's task.
##
## FUTURE DIRECTIONS
##	Should obtain the latitude & longitude from the input image.
##
## HISTORY
##	7/6/89	Written by Jeff Dozier, UCSB.
##
## BUGS
##
## SEE ALSO
##	IPW:  solar, sunlight, elevrad, toporad, horizon, shade

PATH="$PATH:$IPW/lib"
. ipwenv

pgm=`basename $0`
optstring='nz:t:w:g:r:s:x:d:b:l:'
synopsis='[-n] -z elev -t tau -w omega -g g -r R0 [-s S0] [-x w1,w2] \
	-d y,m,d -b d,m,s -l d,m,s [image]'
description='daily integrated radiation over topographic grid'

set -- `getopt "$optstring" $* 2>/dev/null` || {
	usage $pgm "$synopsis" "$description"
	exit 1
}

elev=
tau=
omega=
gfact=
R0=
S0=
wrange=
year=
month=
day=
date=
lat=
lon=
net=0

# parse options, if none print description

case $# in
0|1)	usage $pgm "$synopsis" "$description"
	exit 1
	;;
esac

while :; do
	case $1 in
	--)	shift
		break
		;;
	-H)	usage $pgm "$synopsis" "$description"
		exit 1
		;;
	-n)	net=1
		;;
	-z)	elev=$2
		shift
		;;
	-t)	tau=$2
		shift
		;;
	-w)	omega=$2
		shift
		;;
	-g)	gfact=$2
		shift
		;;
	-r)	R0=$2
		shift
		;;
	-s)	S0=$2
		shift
		;;
	-x)	wrange=$2
		shift
		;;
	-d)	date=$2
		shift
		;;
	-b)	lat=$2
		shift
		;;
	-l)	lon=$2
		shift
		;;
	*)	sherror $pgm '"getopt" failed'
		exit 1
		;;
	esac
	shift
done

tdir=$TMPDIR/$pgm.$$
mkdir $tdir

# list of temporary files used (i.. image; a.. ascii)
# directory for all is $TMPDIR/$pgm.$$
#
# iX - copy of input if stdin
# iZ - elevation image (1 band)
# iGR - gradient image (2 bands)
# iVF - view factor/albedo image (3 bands)
# aQ - quadrature times (sunlight output)
# aS - input for final awk script
# sh - sequence of commands to run
#
# the rest of the files are named within awk script
#
# rad.xx - interval radiation files to be summed by lincom
#
# the following files are re-created for each time step
#
# shade - cosine illumination angle, corrected for horizons
# hor - horizon mask
# erad - beam and diffuse radiation over elevation grid

# Can only have 1 input image.  If stdin must make duplicate copy
# because we need multiple access.

img=$1
case $# in
0)	test -t 0 && {
	sherror $pgm "can't read image data from terminal"
	exit 1
	}
	;;
1)	;;
*)	usage $pgm "$synopsis" "$description"
	exit 1
	;;
esac

case $img in
''|'-')
	cat $img > $tdir/iX
	image=$tdir/iX
	;;
*)	test -r $img || {
		sherror $pgm "can't open file", $image
		exit 1
	}
	image=$img
	;;
esac

# can't write image data to terminal

test -t 1 && {
	sherror $pgm "can't write image data to a terminal"
	exit 1
}

# parse date into year, month, day

case $date in
'')	usage $pgm "$synopsis" "$description"
	exit 1
	;;
*)	IFS_save="$IFS"
	IFS=,
	set,$date
	year=$1
	month=$2
	day=$3
	IFS="$IFS_save"
	;;
esac

# make sure all essential arguments in

case $elev in
'')	sherror $pgm "-z arg missing"
	exit 1
	;;
esac

case $tau in
'')	sherror $pgm "-t arg missing"
	exit 1
	;;
esac

case $omega in
'')	sherror $pgm "-w arg missing"
	exit 1
	;;
esac

case $gfact in
'')	sherror $pgm "-g arg missing"
	exit 1
	;;
esac

case $R0 in
'')	sherror $pgm "-r arg missing"
	exit 1
	;;
esac

case $S0 in
'')	case $wrange in
	'')	sherror $pgm "-s and -x args missing, one must be present"
		exit 1
		;;
	*)	S0=`solar -d $date -w $wrange -a`
		;;
	esac
esac

case $year in
'')	sherror $pgm "-d arg missing or incomplete"
	exit 1
	;;
esac

case $month in
'')	sherror $pgm "-d arg missing or incomplete"
	exit 1
	;;
esac

case $day in
'')	sherror $pgm "-d arg missing or incomplete"
	exit 1
	;;
esac

case $lat in
'')	sherror $pgm "-b arg missing or incomplete"
	exit 1
	;;
esac

case $lon in
'')	sherror $pgm "-l arg missing or incomplete"
	exit 1
	;;
esac


# elevation file
demux -b 0 $image > $tdir/iZ

# slope/aspect file
demux -b 1,2 $image > $tdir/iGR

# sky view, terrain view, and albedo
demux -b 3,4,5 $image > $tdir/iVF

# 1st awk script writes weights and solar angles to temporary file

echo $date $lat $lon | $AWK '
	{
		print "sunlight -d", $1, "-b", $2, "-l", $3, "-q 21 -a"
	}' | sh > $tdir/aQ

# 2nd awk script uses these values to create processing script for elevrad
# (all args except -u are constant)

echo $net $elev $tau $omega $gfact $R0 $S0 \
	$tdir/iZ $tdir/iGR $tdir/iVF $tdir/ \
	> $tdir/aS

cat $tdir/aS $tdir/aQ | $AWK '
BEGIN {
	already = 0
	j = 0
}
{
	if (already) {
		wt[j] = $1
		mu = $2
		azm = $3
		rfile = sprintf("%serad", froot);
		print ecmd, mu, "\\"
		print "	", efile, ">", rfile
		hfile = sprintf("%shor", froot);
		print "horizon -u", mu, "-a", azm, "\\"
		print "	", efile, ">", hfile
		sfile = sprintf("%sshade", froot);
		print "shade -u", mu, "-a", azm, grfile, "|"
		print "	mux", "-", hfile, "|"
		print "	bitcom -m -a >",sfile
		tfile[j] = sprintf("%srad.%02d", froot, j);
		print "mux", rfile, "\\"
		print "	", sfile, "\\"
		print "	", vfile, "|"
		print "	", toporad, ">", tfile[j]
		++j
		print ""
	}
	else {
		already = 1
		if ($1 == 0)
			toporad = "toporad"
		else
			toporad = "toporad -n"
		elev = " -z " $2
		tau = " -t " $3
		omega = " -w " $4
		gfact = " -g " $5
		R0 = " -r " $6
		S0 = " -s " $7
		efile = $8
		grfile = $9
		vfile = $10
		froot = $11
		ecmd = "elevrad -n 8 " elev tau omega gfact R0 S0 " -u"
	}
}
END {
	n = j
	print "mux", "\\"
	for (j = 0; j < n; ++j)
		print "	", tfile[j], "\\"
	ORS = " "
	print "| lincom -c "
	ORS = ","
	for (j = 0; j < n-1; ++j)
		print wt[j]
	ORS = " "
	print wt[n-1]
}' > $tdir/sh$$

# run command string from temporary file

chmod +x $tdir/sh$$
sh -c $tdir/sh$$

# remove temporary files

rm -rf $tdir

exit 0

# $Header: /usr/home/dozier/ipw/src/bin/topquad/RCS/topquad.sh,v 1.5 89/07/06 22:02:15 dozier Exp $
