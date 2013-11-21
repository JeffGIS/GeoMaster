: ${IPW?}

## NAME
##	randomize -- shuffle lines
##
## SYNOPSIS
##	randomize [-s seed] [image ...]
##
## DESCRIPTION
##	randomize copies lines from {image}s (default: standard input) and
##	writes to the standard output, arranging the lines in random order.
##
##	An image name of '-' means the standard input.
##
## OPTIONS
##	-s	A seed value of {seed} will be given to random, producing
##		repeatable output (default: no seed).
##
## EXAMPLES
##	To produce a scrambled version of {file}:
##
##		randomize file
##
## FILES
##	$TMPDIR/rmize{NNNNN}
##
##		Contains a copy of all the input.
##
##	$TMPDIR/rnd{NNNNN}
##
##		Contains random numbers used to sort the input.
##
## DIAGNOSTICS
##	# of values must be > 0
##
##		There must be at least one line of input.
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##
## HISTORY
##	5/27/88	Written by James Frew, UCSB.
##	4/27/93	Added -s option.  Allowed to read images on the command line
##		as well as from standard input.  Dana Jacobsen, ERL-C.
##
## BUGS
##
## SEE ALSO
##	IPW:  random
##	UNIX: wc, paste, sort, sed

PATH="$PATH:$IPW/lib"
. ipwenv

pgm=`basename $0`
optstring="s:i:"
synopsis='[-s seed] [image ...]'
description="shuffle lines"

set -- `getopt "$optstring" $* 2>/dev/null` || {
	exec usage $pgm "$synopsis" "$description"
	exit 1
}

seed=
image=

while :; do
	case $1 in
	--)	shift
		break
		;;
	-H)	usage $pgm "$synopsis" "$description"
		exit 1
		;;
	-s)	seed="-s $2"
		shift
		;;
	*)	shbug $pgm "'getopt' failed"
		exit 1
		;;
	esac
	shift
done

trap 'rm -f $TMPDIR/*$$' 0
trap 'exit 0' 1 2 3 15

case $# in
0)	cat > $TMPDIR/rmize$$
	image=$TMPDIR/rmize$$
	;;
1)	image=$1
	case $image in
	'-')	cat > $TMPDIR/rmize$$
		image=$TMPDIR/rmize$$
		;;
	*)	test -r $image || {
			sherror $pgm "can't open file", $image
			exit 1
		}
		;;
	esac
	;;
*)	rm -f $TMPDIR/rmize$$
	for image in $*
	do
		case $image in
		-)	cat >> $TMPDIR/rmize$$
			;;
		*)	test -r $image || {
				sherror $pgm "can't open file", $image
				exit 1
			}
			cat $image >> $TMPDIR/rmize$$
			;;
		esac
	done
	image=$TMPDIR/rmize$$
	;;
esac


random -r 0,1000000 $seed -n `cat $image | wc -l` > $TMPDIR/rnd$$

paste $TMPDIR/rnd$$ $image | sort -n | sed 's/^[0-9]*[ 	]*//'

rm -f $TMPDIR/*$$

exit 0

# $Header: sh,v 1.9 88/05/27 13:26:42 frew Exp $
