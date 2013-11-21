#!/usr/local/bin/perl
#
# This converts an IPW manual page as printed by ipwman into a UNIX manual
# page (nroff -man format).  It is a 90% solution and will not do a perfect
# job, but it should get you most of the way.
#
# This is ugly, and not a good example of perl programming.  Sorry.
#
# Dana Jacobsen, ERL-C.
#

$final = "";

$_ = <> until /^NAME/;


$final .= ".SH NAME\n";
until (($_ = <>) =~ /^SYNOPSIS/) {
  next if /^\s*$/;
  s/^\t//;
  s/--/\\-/;
  ($progname) = /^\s*(\S+)/;
  $final .= $_;
}


$final .= ".SH SYNOPSIS\n";
$times = 0;
until (($_ = <>) =~ /^DESCRIPTION/) {
  next if /^\s*$/;
  s/^\t//;
  if (/^$progname/) {
    $times++;
    $final .= "\n.br\n" if $times > 1;
  }
#  print "isnow::::\n$_\n";

  s/^\s*($progname)\s*/\n.B "$1\\0"\n/;

  push(@options, /[^a-zA-Z0-9]-[a-zA-Z]/g);

  s/\s*\[-(.)\]\s*/\n\[\n.B \\-$1\n\]\\0/g;
  s/\s*\[-(.) (\S+)\]\s*/\n\[\n.B \\-$1\n$2 \]\\0/g;
  s/\s*-(.) ([^-]\S*)\s*/\n.B \\-$1\n$2\\0/g;
  $final .= $_;
}
$final .= "\n";
$reploptions = "";
foreach $opt (@options) {
  $opt =~ s/^(.|\n)-//;
  $reploptions .=
           "s/(^|[^a-zA-Z0-9])-$opt([^a-zA-Z0-9])/\$1-\\\\f3$opt\\\\fP\$2/g;\n";
}


$final .= ".SH DESCRIPTION\n";
until (($_ = <>) =~ /^OPTIONS/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  $final .= "\n.br\n" if /^\s/;
  s/\b($progname)\b/\\f2$1\\fP/g;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions;
  $final .= $_;
}


$final .= ".SH OPTIONS\n";
$inoption = 0;
$intext = 0;
until (($_ = <>) =~ /^EXAMPLES/) {
  if (/^\s*$/) {
    $inoption = 0;
    $intext = 0;
    next;
  }
  s/^\t//;
  if ($inoption == 0) {
    if (/^-.(\s{2,}|\t)/) {
      $inoption = 1;
      $intext = 0;
      ($opt) = /^-(.)\s+/;
      s/^-(.)\s+//;
      $final .= "\n.TP 0.5i\n.B \\-$opt\n";
    } else {
      $final .= "\n.LP\n" if ($intext == 0);
      $intext = 1;
    }
  }
  s/\b($progname)\b/\\f2$1\\fP/g;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions;
  s/^\s+//;
  $final .= $_;
}


$final .= ".SH EXAMPLES\n";
$indent = 0;
until (($_ = <>) =~ /^FILES/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  if ( ($indent == 1) && (/^\S/) ) {
    $final .= "\n.sp -.1v\n.fi\n\\fR\n.RE\n";
    $indent = 0;
  }
  if ( ($indent == 0) && (/^\s/) ) {
    $final .= "\n.RS\n\\f(CW\n.nf\n.sp -.5v\n";
    $indent = 1;
  }
  s/\b($progname)\b/\\f2$1\\fP/g unless $indent;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  s/^\s+//;
  $final .= $_;
}
$final .= "\n.fi\n.sp -1v\n\\fR\n" if ($indent == 1);


$final .= ".SH FILES\n";
$indent = -1;
until (($_ = <>) =~ /^DIAGNOSTICS/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  if (/^\S/) {
    if ($indent == 1) {
      $final .= "\n.sp -.1v\n.nf\n\\f(CW\n.RE\n";
    } elsif ($indent == -1) {
      $final .= "\n.nf\n\\f(CW\n";
    }
    $indent = 0;
  } else {
    if ($indent == 0) {
      $final .= "\n.RS\n\\fR\n.fi\n.sp -.7v\n";
    } elsif ($indent == -1) {
      $final .= "\n.RS\n";
    }
    $indent = 1;
  }
  s/\b($progname)\b/\\f2$1\\fP/g if $indent;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  s/^\s+//;
  $final .= $_;
}
$final .= "\n.fi\n.sp -1v\n\\fR\n" if ($indent == 0);


$final .= ".SH DIAGNOSTICS\n";
$indent = -1;
until (($_ = <>) =~ /^RESTRICTIONS/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t {0,2}//;
  if (/^\S/) {
    if ($indent == 1) {
      $final .= "\n.sp -.1v\n.nf\n\\f(CW\n.RE\n";
    } elsif ($indent == -1) {
      $final .= "\n.nf\n\\f(CW\n";
    }
    $indent = 0;
  } else {
    if ($indent == 0) {
      $final .= "\n.RS\n\\fR\n.fi\n.sp -.5v\n";
    } elsif ($indent == -1) {
      $final .= "\n.RS\n";
    } elsif ($indent == 1) {
      $final .= "\n.br\n" if (/^\t\t/);   # allow tables inside text
    }
    $indent = 1;
  }
  s/\b($progname)\b/\\f2$1\\fP/g if $indent;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions if $indent;
  s/^\s*\./\\&./g;
  if (/^\t\t/) {
    s/^\s+/\t/;    # indent tables inside text
  } else {
    s/^\s+//;
  }
  $final .= $_;
}
$final .= "\n.fi\n.sp -1v\n\\fR\n" if ($indent == 0);


$final .= ".SH RESTRICTIONS\n";
until (($_ = <>) =~ /^FUTURE DIRECTIONS/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  $final .= "\n.br\n" if /^\s/;
  s/\b($progname)\b/\\f2$1\\fP/g;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions;
  $final .= $_;
}


$final .= ".SH FUTURE DIRECTIONS\n";
until (($_ = <>) =~ /^HISTORY/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  $final .= "\n.br\n" if /^\s/;
  s/\b($progname)\b/\\f2$1\\fP/g;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions;
  $final .= $_;
}


$final .= ".SH HISTORY\n";
until (($_ = <>) =~ /^BUGS/) {
  if (/^\s*$/) {
    next;
  }
  s/^\t {0,2}//;
  if (/^\S/) {
    ($date) = /^([\d\/]+)\s+/;
    s/^([\d\/]+)\s+//;
    $final .= "\n.TP 0.8i\n.B $date\n";
  }
  s/\b($progname)\b/\\f2$1\\fP/g;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions;
  s/^\s+//;
  $final .= $_;
}


$final .= ".SH BUGS\n";
until (($_ = <>) =~ /^SEE ALSO/) {
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  $final .= "\n.br\n" if /^\s/;
  s/\b($progname)\b/\\f2$1\\fP/g;
  s/\{([^\s}]+)\}/\\f2$1\\fP/g;
  eval $reploptions;
  $final .= $_;
}


$final .= ".SH SEE ALSO\n";
while (<>) {
  if (eof()) {
    chop;
    $last = $_;
    next;
  }
  if (/^\s*$/) {
    $final .= "\n.LP\n";
    next;
  }
  s/^\t//;
  s/^(\w+):\s+/\n.TP 0.6i\n.I $1\n/;
#  $final .= "\n.br\n" if (/^IPW/ || /^UNIX/ || /^Image/);
  s/^\s*//;
  $final .= $_;
}





$final =~ s/\n\n/\n/g;
1 while $final =~ s/(\.SH [A-Z ]+)\n(\.LP\n)*\.SH /$1\n.rs\n.sp .5v\n.SH /g;
$final =~ s/\.LP\n\.RS/.RS/g;

($ftype, $fdate, $fauth) = split(/  +/, $last);
if ( ($fdate !~ /..\/..\/../) && ($fauth =~ /^\s*$/) ) {
  $fauth = $fdate;
  $fdate = '';
}
print ".de an-header\n.an-init\n.ev 1\n.sp .5i\n",
      ".tl '\\\\*[an-title]'\\\\*[an-extra3]'\\\\*[an-title]'\n",
      ".sp |1i\n.ev\n.ns\n..\n";
$progname =~ tr/a-z/A-Z/;
print ".TH \"\\f3\\s+0$progname\\s0\\fP\" ",
      "\"\" \"$fdate\" \"$fauth\" \"$ftype\"\n";
print $final;
