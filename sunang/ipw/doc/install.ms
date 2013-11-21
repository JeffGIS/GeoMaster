.\" refer -e | tbl | *roff -ms
.\"
.\" NOTE: this document assumes that a constant-width font is mounted on
.\" position 7.  The "old" (C/A/T-specific) troff has only 4 font positions,
.\" so it will not be able to process this document.
.\" 
.so macs
.MD $Header: /home/ipw/doc/RCS/install.ms,v 1.7 90/02/02 10:32:54 frew Exp $
.TL
Installing \*i
.AU
J. Frew
.AI
Computer Systems Laboratory
University of California
Santa Barbara, CA  93016
.sp \n(PDu
(805) 961-8413
.sp \n(PDu
.ft 7
frew@crseo.ucsb.edu
.ft P
.\"---------------------------------------------------------------------------
.NH
Introduction
.LP
This document describes how to install \*i on a new host system.
\*i is currently known to operate on the following systems:
.TS
center ;
cfB | cfB
l | l .
hardware	operating system
=
Sun-3	SunOS 3.4, 3.5, 4.0.3
_
Sun-4	SunOS 4.0.3
_
IBM PS/2 Model 80	AIX 1.1
_
IBM RT Model 135	AOS 4.3
.TE
If your host is one of these,
then installing \*i should go pretty smoothly.
If not,
well,
it OUGHT to go smoothly,
but will require some thinking on your part.
In any event,
let me know if you encounter any major omissions or outright lies herein.
.\"---------------------------------------------------------------------------
.NH
A note on configuration files
.LP
In the process of installing \*i you will be modifying a few system-dependent
files.
In most (but not all) cases,
a directory containing such a file also contains a subdirectory named
.CW conf .
This subdirectory contains versions of the system-dependent file that have
been used on existing \*i host systems.
You can often copy (or link to) the appropriate version of the file
in
.CW conf ,
instead of creating a new file.
.\"---------------------------------------------------------------------------
.NH
Create \*i account
.LP
Your \*i host system should have a unique
.CW ipw
user and a unique
.CW ipw
group.
Create them now if they don't already exist (you must be the super-user to do
this).
Here is the relevant entry from our
.CW /etc/passwd
file:
.CS
ipw:*:22:21:(IPW subsystem):/usr/home/ipw:/bin/csh
.CE
The
.CW *
in the password field will keep the account disabled until we explicitly set a
password.
The user- and group-IDs,
and the home directory,
will have to be customized for your system.
.LP
Here is the relevant line from our
.CW /etc/group
file:
.CS
ipw::21:dozier,frew,judy,venema
.CE
You should edit the user list to include only those users (besides
.CW ipw )
to whom you wish to grant write access to \*i source files,
libraries,
etc.
.LP
Next,
make a home directory for
.CW ipw :
.CS
# mkdir ~ipw
# chown ipw ~ipw
# chgrp ipw ~ipw
# chmod 775 ~ipw
.CE
Finally,
set a password for the
.CW ipw
account:
.CS
# passwd ipw
.CE
All the remaining installation instructions assume you are logged in as
(or
.CW su 'd
to)
.CW ipw .
.\"---------------------------------------------------------------------------
.NH
Load the distribution tape
.LP
\*i is distributed on magnetic tape as a single \*(UX
.CW tar
archive.
To extract the tape,
type:
.CS
% cd ~ipw
% tar xvpbf \f2blocking-factor\fP \f2tape-device\fP
.CE
where
.I blocking-factor
is 20 for 1/2-inch tapes,
or 200 for 1/4-inch cartridge tapes.
.I tape-device
is the appropriate \*(UX tape device name (e.g.,
.CW /dev/rmt8 ,
.CW /dev/rst0 ,
etc.).
The
.CW p
flag is important;
it guarantees that the original file permissions will be restored.
.\"---------------------------------------------------------------------------
.NH 2
Possible ownership problems
.LP
Do an 
.CW ls
.CW -lg
(or
.CW ls
.CW -l
for USG-derived \*(UXes) on
.CW ~ipw .
If the extracted directories are owned by user
.CW ipw
and group
.CW ipw,
then skip to the next step
.B Configuring
.B the
.B shell(s) .
.LP
If your \*(UX system allows non-privileged users to execute the chown system
call,
then the
.CW tar
command shown above may set the user- and group-IDs of the \*i files to
whatever they were on the system where the distribution tape was created.
These will almost certainly NOT be the correct user- and group-IDs for your
system.
If this occurs,
you will have to become root and run
.CW chown
and
.CW chgrp
on the \*i hierarchy.
.LP
If your system has the
.CW xargs
command,
you can type:
.CS
# find ~ipw -print | xargs chown ipw
# find ~ipw -print | xargs chgrp ipw
.CE
Otherwise,
type
.CS
# find ~ipw -exec chown ipw {} \e; -exec chgrp ipw {} \e;
.CE
.\"---------------------------------------------------------------------------
.NH
Configuring the shell(s)
.LP
From here on,
we make 2 important assumptions about the \*i environment:
.IP \(bu
An \*i user's interactive command interpreter is
.CW csh
(the Berkeley \*QC shell\*U)
.IP \(bu
command files (\*Qshell scripts\*U) whose first character is a colon (
.CW : )
will be executed by
.CW sh
(the \*QBourne shell\*U).
Your sh must understand the sharp-sign (
.CW # )
comment convention;
no other extensions to the original Version 7
.[ [
%T \*(UX Programmer's Manual, vol. 1
%Q Bell Telephone Laboratories, Inc.
%C New York
%I Holt, Rinehart, and Winston
%D 1983
%X original Version 7 \*(UX manual
.]]
.CW sh
are used.
.LP
See
.B pub/cshrc
below for suggestions on dealing with departures from these assumptions.
.LP
Also,
all path names given below are relative to
.CW ~ipw .
.\"---------------------------------------------------------------------------
.NH 2
pub/cshrc
.LP
The file
.CW pub/cshrc
should be sourced by all \*i users before they issue any \*i commands.
Usually,
a line like the following is placed in an \*i user's
.CW .cshrc
file:
.CS
source ~ipw/pub/cshrc
.CE
This file should not need to be changed,
but take a look at it now.
The environment variable
.CW IPW
must be set to the name of the \*i root directory.
The environment variable
.CW TMPDIR
must be set to the name of a directory on which scratch images may be written.
Finally,
the directories
.CW ~ipw/bin
and
.CW ~ipw/etc
must be in an \*i user's search path.
.LP
If shell scripts beginning with a colon aren't automatically run by
.CW sh ,
then you may need to add the following line to
.CW pub/cshrc :
.CS
setenv SHELL /bin/sh	# or wherever sh lives
.CE
(This is necessary on some XENIX systems,
for instance.)
.LP
If your interactive command interpreter isn't
.CW csh ,
then you'll need to prepare an equivalent of
.CW pub/cshrc .
For example,
a
.CW pub/profile
for
.CW sh
might look like this:
.CS
IPW=/usr/home/ipw	# or whatever corresponds to ~ipw
TMPDIR=/usr/tmp
export IPW TMPDIR
PATH=$PATH:$IPW/bin:$IPW/etc
.CE
and then
.CW sh
users would put the following in their
.CW .profile
file:
.CS
\&. /usr/home/ipw/pub/profile
.CE
.\"---------------------------------------------------------------------------
.NH 2
lib/ipwenv
.LP
The file
.CW lib/ipwenv
contains local definitions for nonportable \*(UX commands used by \*i shell
scripts.
Check the definitions in this file and fix any that are inappropriate for your
host:
.IP "\f7AWK\fP"
An
.CW awk
command that conforms to the 1985 definition
.[ [
%T The AWK Programming Language
%A A. V. Aho
%A B. W. Kernighan
%A P. J. Weinberger
%C Reading, MA
%I Addison-Wesley
%D 1988
.]]
of the
.CW awk
language.
As of this writing,
all \*i scripts will work with older versions of
.CW awk ,
but this may change with future releases of \*i.
.IP "\f7LC_WD\fP"
Produce a multi-column listing of the current directory.
On some \*(UX systems,
the
.CW ls
command does this by default.
.IP "\f7LLG\fP"
Produce a \*Qlong\*U directory listing with both the owner and the group of
each file.
Some \*(UX systems use the command
.CW ls
.CW -lg ,
while others use
.CW ls
.CW -l .
.\"---------------------------------------------------------------------------
.NH
Configuring make
.LP
There are a gazillion variations of the
.CW make
command out there.
Instead of hoping you've got a competent one,
we just assume the lowest common denominator (Version 7
.CW make )
and provide an
.CW ipwmake
command to automatically generate an appropriate
.CW makefile
for each command,
library,
etc. (actually,
the rules and dependencies are piped directly to
.CW make ).
.CW ipwmake
uses some nonportable \*Qboilerplate\*U that you will probably have to twiddle.
.\"---------------------------------------------------------------------------
.NH 2
lib/make/local
.LP
The
.CW lib/make
directory contains the boilerplate used by the
.CW ipwmake
command (the specific stuff is kept in a
.CW Makedefs
file in each source directory).
You can take a look at
.CW etc/ipwmake
to see how it all fits together.
The only file that should need changing should be
.CW lib/make/local .
A detailed description of the contents of
.CW lib/make/local
is beyond the scope of this document;
see the comments therein.
However,
here are some things to watch out for:
.IP "\f7AR_XOPTS\fP"
.CW ipwmake 's
library-update procedure relies on the ability to extract files from an
archive with their original modification times.
BSD-derived
.CW ar 's
use the 
.CW o
qualifier to do this,
while (some) USG
.CW ar 's
do this by default.
.IP
IF YOUR
.CW ar
CAN'T DO THIS SOMEHOW, YOU'RE S.O.L. !!
.LP
The directory
.CW lib/make/local/conf
contains versions of
.CW lib/make/local
for various systems.
.\"---------------------------------------------------------------------------
.NH
Configuring the C compiler
.LP
Now we'll twiddle the header files used by \*i C code.
This basically involves wiring in the specific ways in which your system
differs from the \*Qideal\*U combination of ANSI C
.[ [
%T Draft Proposed American National Standard for Information Systems \- Programming Language C
%Q X3J11 Technical Committee on the C Programming Language
%C New York
%I X3 Secretariat, Computer and Business Equipment Manufacturers Association
%D 7 December 1988
%R X3.159-1989
%X C language standard
.]]
and POSIX
.[ [
%T IEEE Standard Portable Operating System for Computer Environments
%Q Technical Committee on Operating Systems of the IEEE Computer Society
%C New York
%I IEEE
%D 1988
%R Std 1003.1-1988
%X POSIX operating system standard
.]].
.\"---------------------------------------------------------------------------
.NH 2
h/config.h
.LP
The directory
.CW h/conf
contains various versions of
.CW h/config.h
with reasonably mnemonic names (e.g.
.CW sun+sysv
for SunOS with System V
.[ [
%T System V Interface Definition, issue 2 (3 vols.)
%I AT&T
%D 1986
%R Select Code 307-131
%X \*(UX System V operating system standard
.]]
enhancements).
.CW h/config.h
contains a whole bunch of
.CW #define 's that customize your C environment.
Unless your system EXACTLY matches one of the \*Qcanned\*U configurations,
you should check each
.CW #define
against its description in
.CW h/conf/README .
Here are some things to pay special attention to:
.IP "\f7CC_NOBCOPY\fP, \f7CC_NOBZERO\fP"
Note that these options read as a double-negative;
i.e.
.CW CC_NOBCOPY
should be 1 if
.CW bcopy()
ISN'T provided in your host's C library.
.IP "\f7CC_POSIX\fP"
If this option is 1,
then POSIX include files will be fetched from
.CW h/posix
instead of from
.CW #include
.CW <...>
.IP "\f7CC_SAMESIZE\fP"
This option is obsolete;
it should always be 0 (zero)
.IP "\f7OPT_BUF\fP"
This is the buffer size \*i uses with
.CW read()
and
.CW write() .
It
.B
MUST NOT
.R
be larger than the maximum number of bytes that may be written to a pipe in a
single
.CW write() .
4096 is the largest portable value.
On memory-limited systems,
you may want to use a smaller value,
in which case 1024 is suggested.
.IP "\f7FREG_\fP?"
These options are only appropriate for machines with separate floating-point
registers (e.g. MC68881) that are known to the C compiler.
Do
.B NOT
use for machines that store floating-point values in general registers
(e.g. VAX).
.\"---------------------------------------------------------------------------
.NH 2
h/ansi
.LP
If your system has ANSI-conforming C header files,
then you must copy (or link)
.CW <float.h>
and
.CW <limits.h>
to the directory
.CW h/ansi .
Otherwise,
you must create substitutes for these header files,
as described below.
.\"---------------------------------------------------------------------------
.NH 3
h/ansi/float.h
.LP
This is a substitute for the ANSI header file
.CW <float.h> .
If you're lucky,
you'll be able to use one of the existing versions in
.CW h/ansi/conf.float .
If you're a bit less lucky,
i.e. if your particular machine isn't mentioned explicitly BUT it supports
IEEE 754 floating point,
you can use the generic version
.CW h/ansi/conf.float/ieee754 .
.LP
If,
however,
you're not so lucky,
you'll have to generate these values yourself.
First,
try going to the directory
.CW src/etc/machine
and typing
.CW make .
If the resulting program
.CW machine
compiles and runs successfully,
it will generate a
.CW float.h
case for your machine \- you just have to catch the output of
.CW "machine -float"
and stuff it into
.CW h/ansi/float.h .
.LP
Finally,
if
.CW machine
doesn't compile or run successfully,
you'll have to do some detective work.
Your system may have a file
.CW <values.h>
that contains some of the stuff you'll need;
otherwise,
dig into a hardware manual or find a guru.
.\"---------------------------------------------------------------------------
.NH 3
h/ansi/limits.h
.LP
This is a substitute for the ANSI header file
.CW <limits.h> .
As for
.CW h/ansi/float.h,
you may get lucky and be able to use an existing version in
.CW h/ansi/conf.float .
Otherwise,
you can use
.CW "machine -limits" .
.\"---------------------------------------------------------------------------
.NH 2
h/posix
.LP
If your system conforms to the IEEE 1003 (\*QPOSIX\*U) \*(UX specification,
then you must copy (or link)
.CW <limits.h>
to the directory
.CW h/posix .
Otherwise,
you must create a substitute for this header file,
as described below.
.\"---------------------------------------------------------------------------
.NH 3
h/posix/limits.h
.LP
This is a substitute for the POSIX header file
.CW <limits.h> .
Actually,
this file only contains the stuff NOT present in the ANSI
.CW <limits.h>
(the real POSIX file also contains all the ANSI stuff \- aren't standards
wonderful?).
.LP
The various possible entries are documented in the file.
Only a few are currently used by \*i.
As with the ANSI stuff,
you may get lucky and be able to use one of the existing versions in
.CW h/posix/conf ,
otherwise,
see
.CW h/posix/conf/README
for guidance in building your own
.CW h/posix/limits.h .
.\"---------------------------------------------------------------------------
.NH
src/lib/libunix
.LP
\*i provides substitutes for some ANSI library functions that may not be
available on all systems.
The sources for these functions are in the directory
.CW src/lib/libunix/src .
Any of these functions that your host does not already provide must be copied
(or linked) to the directory
.CW src/lib/libunix .
The
.CW Makedefs
file in
.CW src/lib/libunix
must then be edited to select those functions.
.\"---------------------------------------------------------------------------
.NH
Building \*i
.LP
At this point,
you should be able to run
.CW ipwmake
in the
.CW src
directory and build the whole wretched mess:
.CS
% cd ~ipw/src
% ipwmake install >& make.out &
.CE
Any errors in
.CW make.out
(other than those explicitly marked
.CW ignored )
indicate a problem,
and should be tracked down before using \*i.
.\"---------------------------------------------------------------------------
.NH
Post-installation issues
.LP
Certain \*i commands cannot (yet) be installed automatically,
since they require non-portable system header files and/or function libraries.
At this writing,
the following commands must be installed by explicitly running
.CW ipwmake
in the appropriate source directory:
.TS
center ;
cfB | cfB
lf7 | lf7 .
command	requires
=
etc/mc	-ltermcap
_
etc/rastool	T{
<suntool/canvas.h>
<suntool/scrollbar.h>
<suntool/sunview.h>
-lsuntool
-lsunwindow
-lpixrect
T}
_
etc/xim	T{
<X11/Xos.h>
<X11/Xlib.h>
<X11/Xutil.h>
<X11/cursorfont.h>
-lX11
T}
.TE
Note that the distributed version of
.CW lib/ipwenv
assumes that the
.CW mc
command exists.
If you cannot compile
.CW mc ,
then you should change the definition of
.CW LC_WD
in
.CW lib/ipwenv .
.LP
If you will be developing your own \*i C programs,
then you will probably want to compile profiled and debugging versions
of the \*i libraries,
and
.CW lint
libraries for use with
.CW "ipwmake lint" .
Run
.CW "ipwmake lintlib" ,
.CW "ipwmake -D" ,
and
.CW "ipwmake -P"
in the appropriate library directories.
.\"---------------------------------------------------------------------------
.NH
If You Have Problems ...
.LP
I can be reached at the various phone/email/USmail coordinates given at the
beginning of this document.
Email is DEFINITELY preferred.
Just to get you going,
how about letting me know how effective this document was?
.LP
Anybody who gets \*i directly from me (i.e. legitimately) will be placed on
the list for email updates and general chatter.
.LP
Good Luck!
.DS
-Frew
.DE
.[
$LIST$
.]
