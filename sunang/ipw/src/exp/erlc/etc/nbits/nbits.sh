: ${IPW?}

## NAME
##	nbits -- print # bits required to represent integer arg
##
## SYNOPSIS
##	nbits number
##
## DESCRIPTION
##
## OPTIONS
##
## EXAMPLES
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	nbits is currently implemented as a shell script.
##	It may be desirable to reimplement nbits as a program.
##
## BUGS

PATH=$PATH:$IPW/lib
. ipwenv

case $# in
1)	echo $1 |
		$AWK '{
			print int(log($1) / log(2)  +  1)
		}'
	;;
*)	usage $0 'number' 'print # bits required to represent "number"'
	exit 1
	;;
esac

# $Header: /usr/home/ipw/src/lib/nbits/RCS/nbits.sh,v 1.5 89/10/25 18:05:15 frew Exp $
