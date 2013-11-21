: ${IPW?}

## NAME
##	name -- what I do
##
## SYNOPSIS
##	cmd [-abc] [-d optarg] operand
##
## DESCRIPTION
##
## OPTIONS
##	letter	description
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
##	PGM is currently implemented as a shell script.
##	It may be desirable to reimplement PGM as a program.
##
## BUGS

PATH="$PATH:$IPW/lib"
. ipwenv

optstring=
synopsis=
description=

set - `getopt "$optstring" $* 2>/dev/null` ||
	exec usage $0 "$synopsis" "$description"

(initialize options here)

while :; do
	case $1 in
	--)	shift
		break
		;;
(process options here)
	*)	exec sherror $0 '"getopt" failed'
		;;
	esac

	shift
done

for operand do
(process operands here)
done

(do the real work here)

exit $?

# $Header: sh,v 1.9 88/05/27 13:26:42 frew Exp $
