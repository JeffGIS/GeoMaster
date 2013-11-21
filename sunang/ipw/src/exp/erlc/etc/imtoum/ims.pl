#!/usr/local/bin/perl

# Script to interactively display IPW manual pages.
#
# Why Perl?  Because with sh, the PATH variable can't be set reliably,
# and csh has no signal handling capabilities, so we end up leaving
# droppings in the temp area.  This is far more complicated than it
# should be though.
#
# Dana Jacobsen, ERL-C, July 1993

eval "exec /usr/local/bin/perl -S $0 $*" if $running_under_some_shell;

($program = $0) =~ s,.*/,,;

$dvi = 0;
$postscript = 0;

while (@ARGV) {
  $_ = shift @ARGV;
  /^--$/  && do { push(@pages, @ARGV); undef @ARGV; next; };
  /^-d/ && do { $dvi = 1; next; };
  /^-p/ && do { $postscript = 1; next; };
  /^-g/ && do { $ghostview = 1; next; };
  push (@pages, $_);
}
$pages = join(' ', @pages);
if ($pages =~ /^\s*$/) {
  die "Usage:  $program [-dvi] [-ps]  <manual pages>\n";
}

if ( ($dvi == 0) && ($postscript == 0) ) {
  $postscript = 1;
}
if ( ($dvi == 1) && ($postscript == 1) ) {
  die "Can't display both postscript and dvi.\n";
}

umask 022;

$SIG{'INT'}  = 'CLEANUP';
$SIG{'TERM'} = 'CLEANUP';
$SIG{'HUP'}  = 'IGNORE';

chop($cwd = `pwd`);
$root = $cwd;
$TMPDIR = ($ENV{'TMPDIR'}) ? $ENV{'TMPDIR'} : '/tmp';
$TMPDIR =~ s#^(~([a-z0-9]+))(/.*)?$#((getpwnam($2))[7]||$1).$3#e; # expand ~user
$TMPDIR =~ s#^(~)(/.*)?$#((getpwnam(getlogin))[7]||$1).$2#e;     # expand ~/file
$IPW = ($ENV{'IPW'}) ? $ENV{'IPW'} : '~ipw';

$tmpimg = $TMPDIR . "/$program.$$";

if ($postscript) {
  $tmpimg .= ".ps";
  $gopt = "-Tps -man -rC1 -rD1";
  if ($ghostview) {
    $viewer = "/usr/local/gnu/bin/ghostview";
  } else {
    $viewer = "/usr/openwin/bin/pageview";
  }
}
if ($dvi) {
  $tmpimg .= ".dvi";
  $gopt = "-Tdvi -man -rC1 -rD1";
  $viewer = "/usr/local/bin/xdvi";
}

chdir "/usr/local/gnu/bin";
system("$IPW/etc/ipwman $pages | $IPW/etc/imtoum | groff $gopt > $tmpimg");
if (!-w "$tmpimg") {
  &mdie("Can't seem to write to temporary file.  Check TMPDIR setting.\n");
}
if (!-r "$tmpimg") {
  &mdie("Can't view manual page.\n");
}
if (!-s "$tmpimg") {
  &mdie("No manual page for $pages.\n");
}
system("$viewer $tmpimg");
chdir $root;
unlink $tmpimg;


sub CLEANUP {
  chdir $root;
  unlink $tmpimg;
  exit 1;
}

sub mdie {
  chdir $root;
  unlink $tmpimg;
  die @_;
}
