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
##	install -- install commands, libraries, etc.
##
## SYNOPSIS
##	install [-c] [-m mode] [-o owner] [-g group] [-s] file destination
##
## DESCRIPTION
##	install moves or copies (default: move) {file} to {destination},
##	optionally changing its owner, group, or permission mode.  If
##	{destination} is a directory, {file} will be placed in that
##	directory.
##
## OPTIONS
##	-c	{file} will be copied to {destination} rather than moved.
##
##	-m	The permission mode for the installed file will be set to
##		{mode}, which can be any mode recognized by chmod.
##
##	-o	The ownership of the installed file will be set to {owner}.
##		This may require root privileges (see the UNIX man page for
##		chown(1)).
##
##	-g	The group ownership of the installed file will be set to
##		{group}.  This may require either that you are a member of
##		the specified group, or root privileges (see the UNIX man
##		page for chgrp(1)).
##
##	-s	After installation, run the program strip to remove the
##		symbol tables.  This will make executables smaller, but
##		will remove any debugging information.  It only makes
##		sense for executable binary files.
##
## EXAMPLES
##	Within IPW, install is almost always invoked by make or ipwmake.
##
## FILES
##
## DIAGNOSTICS
##
## RESTRICTIONS
##
## FUTURE DIRECTIONS
##	install is currently implemented as a shell script.
##	It may be desirable to reimplement install as a program.
##
##	Perhaps GNU install, part of the fileutils package, should be used.
##
## HISTORY
##	11/16/90 Written by James Frew, UCSB.
##
## BUGS
##
## SEE ALSO
##	UNIX:  install, chmod, chgrp, chown, strip

PATH="$PATH:$IPW/lib:$IPW/etc"
. ipwenv

synopsis='[-c] [-m mode] [-o owner] [-g group] [-s] file destination'
descrip='install commands, libraries, etc.'

pgm=$0
set -- `getopt 'cg:m:o:s' $* 2>/dev/null` || {
	usage $pgm "$synopsis" "$descrip"
	exit 1
}

rm='rm -f'
create=mv

chmod=:
mode=
chown=:
owner=
chgrp=:
group=
strip=:

while :; do
	case $1 in
	--)	shift
		break
		;;
	-c)	create=cp
		;;
	-m)	chmod=chmod
		mode=$2
		shift
		;;
	-o)	chown=chown
		owner=$2
		shift
		;;
	-g)	chgrp=chgrp
		group=$2
		shift
		;;
	-s)	strip=strip
		;;
	*)	shbug $pgm "'getopt' failed"
		exit 1
		;;
	esac
	shift
done

case $# in
2)	src=$1
	dest=$2
	;;
*)	usage $pgm "$synopsis" "$descrip"
	exit 1
	;;
esac

test -f $src || {
	sherror $pgm "no such file" $src
	exit 1
}

case $dest in
'.'|$src)
	create=:
	rm=:
	;;
*)	test -d $dest && dest=$dest/$src
	;;
esac

$LLG $src
$rm $dest
$create $src $dest
$strip $dest
$chmod $mode $dest
$chgrp $group $dest
$chown $owner $dest
$LLG $dest

# RCS $Header: /local/share/pkg/ipw/src/etc/install/RCS/install.sh,v 1.13 90/11/16 16:51:57 frew Exp $
