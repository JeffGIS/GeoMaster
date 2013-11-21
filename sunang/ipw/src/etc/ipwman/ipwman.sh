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

: {$IPW?}

## NAME
##	ipwman -- generate IPW manual pages from source file comments
##
## SYNOPSIS
##	ipwman [-p] [-f] [command | function] ...
##	ipwman [-k] keyword ...
##
## DESCRIPTION
##	ipwman simulates the UNIX "man" command for IPW, by extracting the
##	header comments from the IPW source files corresponding to the
##	specified commands or library functions, and writing them to the
##	standard output.
##
##	Leading "*"s or "#"s are stripped from the header comments.  Each
##	group of header comments associated with a particular command or
##	function has the source file's last modifed date and a form feed
##	character appended.
##
##	If the -k option is used, no manual pages will be shown.  Instead,
##	a command-summary file is searched for for all commands matching
##	the keyword given.  If multiple keywords are given, they are
##	combined by an "or" operation, so commands matching any of the
##	keywords will be printed.  The command summary file consists of
##	the text in the NAME section of the manual for all IPW programs
##	installed.
##
##	The -p and -f options restrict the search to only programs or
##	functions, respectively.
##
## OPTIONS
##	-k	The summary description of all commands with the text
##		{keyword} in their summary will be output.
##
##	-p	Do not search for functions, only programs.
##
##	-f	Do not search for programs, only functions.
##
## EXAMPLES
##	To print the manual pages for the "gradient" and "shade" commands,
##	using pr to paginate and date the pages:
##
##		ipwman gradient shade | pr | lpr
##
##	To find programs relating to solar radiation:
##
##		ipwman -k solar radiation
##
## FILES
##	$IPW/src/(bin,etc)/{command}/main.c
##	$IPW/src/(bin,etc)/{command}/{command}.sh
##	$IPW/src/exp/*/(bin,etc)/{command}/main.c
##	$IPW/src/exp/*/(bin,etc)/{command}/{command}.sh
##
##		These files are searched for command header comments.
##
##	$IPW/src/lib/*/{function}.c
##	$IPW/src/exp/*/lib/*/{function}.c
##
##		These files are searched for function header comments.
##
##	$IPW/command-summary
##
##		This file contains the command summaries.  The Perl script
##		mk.comsum is used to generate this file.
##
## DIAGNOSTICS
##	No information for:
##	  {command}
##	  ...
##
##		The specified {command}s either do not exist or have no
##		appropriate source file comments.
##
## RESTRICTIONS
##	ipwman is currently implemented as a shell script.
##
## FUTURE DIRECTIONS
##
## HISTORY
##	7/1/90	 Written by James Frew, UCSB.
##	8/1/92	 Added exp directory searches, use $PAGER instead of more,
##		 don't strip all spaces from front.  Dana Jacobsen, ERL-C.
##	4/1/93	 Remove last ^L from output.  Dana Jacobsen, ERL-C.
##	4/15/93	 Added searching through exp libraries.  Dana Jacobsen, ERL-C.
##	7/19/93	 Added source listing at bottom.  Dana Jacobsen, ERL-C.
##	7/29/93	 Added the -k (apropos) option.  Dana Jacobsen, ERL-C.
##	8/19/93	 Added the -p and -f options.  Dana Jacobsen, ERL-C.
##	10/20/93 Minor fix to mank.awk.  Dana Jacobsen, ERL-C.
##	10/19/94 Added paths to the grass & las converters. Rusty Dodson, ERL-C
##	5/10/95	 Change to sed line in -k option.  Dana Jacobsen, ERL-C.
##
## BUGS
##
## SEE ALSO
##	IPW:   ipw, imtoum
##	UNIX:  lpr, man, pr

PATH=$PATH:$IPW/lib
. ipwenv

pgm=`basename $0`
optstring='kpf'
synopsis='[-k] [-p] [-f] [program | function]'
description='extract header comments from IPW source files'
 
case $* in
''|-H)	usage $pgm "$synopsis" "$description"
	exit 1
	;;
esac

set -- `getopt "$optstring" $* 2>/dev/null` || {
        usage $pgm "$synopsis" "$description"
        exit 1
}

progonly=''
funconly=''
while :; do
	case $1 in
	--)	shift
		break
		;;
	-k)	shift
		shift
		for word do
			word=`echo $word | tr A-Z a-z`
			kw="$kw|$word"
		done
		kw=`echo $kw | sed 's/^|//'`
		cat $IPW/command-summary | tr A-Z a-z | \
		$AWK -f $IPW/pub/mank.awk  keyword="$kw"
		exit 0
		;;
	-p)	shift
		progonly="TRUE"
		break
		;;
	-f)	shift
		funconly="TRUE"
		break
		;;
	*)	sherror $pgm '"getopt" failed'
		exit 1
		;;
	esac
	shift
done

cd $IPW/src || exit 1

# find files containing header comments

pdirs='bin etc lib exp/*/bin exp/*/etc exp/*/lib exp/*/misc/*'
ldirs='lib/lib* exp/*/lib'

files=
undoc=

for item do
	found=no

	if test -z "$funconly"; then
		for dir in $pdirs; do
			test -r $dir/$item/main.c && {
				files="$files $dir/$item/main.c"
				found=yes
				continue
			}

			test -r $dir/$item/$item.sh && {
				files="$files $dir/$item/$item.sh"
				found=yes
			}

			test -r $dir/$item/$item.c && {
				files="$files $dir/$item/$item.c"
				found=yes
			}
		done
	fi

	if test -z "$progonly"; then

		for dir in $ldirs; do
			test -r $dir/$item.c && {
				files="$files $dir/$item.c"
				found=yes
			}

			for subdir in $dir/*; do
				test -d $subdir -a -r $subdir/$item.c && {
					files="$files $subdir/$item.c"
					found=yes
				}
			done
		done
	fi

	case $found in
	no)	undoc="$undoc $item"
		;;
	esac
done

# print header comments

if test -t; then
	test -z "$PAGER" && PAGER=more
else
	PAGER=cat
fi

{
	case $undoc in
	'')	;;
	*)	echo 'No information for:'
		for item in $undoc; do
			echo "	$item"
		done
		echo ''
		;;
	esac

	for file in $files; do
		ftype=`echo $file | sed -n '
			y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/
			/^ETC/s/.*/IPW Base Support/p
			/^BIN/s/.*/IPW Base Program/p
			/^LIB/s/.*/IPW Base Function/p
			/^EXP.[^/]*.BIN/s/^EXP.\([^/]*\).*/IPW \1 Extension Program/p
			/^EXP.[^/]*.ETC/s/^EXP.\([^/]*\).*/IPW \1 Extension Support/p
			/^EXP.[^/]*.LIB/s/^EXP.\([^/]*\).*/IPW \1 Extension Function/p
		'`
		fdate=`sed -n '/\$Header/s/^.* \(..\/..\/..\) .*$/\1/p' $file`
		fauth=`sed -n '/Written by/s/^.*Written by \([^,]*\).*$/\1/p' $file | head -1`
		sed -n '
			/^[*#][*#]/s/.. \{0,1\}//p
			/\$Header/d
		' $file
		echo $ftype:$fdate:$fauth |
			$AWK -F: '{ printf "\n%-35s %8s %35s\n", $1, $2, $3 }'
		echo ''
	done
} |
	sed '$d' | $PAGER

# $Header: /local/share/pkg/ipw/src/etc/ipwman/RCS/ipwman.sh,v 1.12 90/11/16 16:52:04 frew Exp $
