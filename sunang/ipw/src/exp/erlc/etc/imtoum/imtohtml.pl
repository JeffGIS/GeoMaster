#!/usr/local/bin/perl
#
# This converts an IPW manual page as printed by ipwman into an HTML page
# that is viewable by such browsers as Netscape, Mosaic, Arena, and Lynx.
#
# This program is a modification of imtoum.  No cleanup or code rewriting
# was attempted, so the code is as muddled as imtoum.
#
# Written by Dana Jacobsen, ERL-C, March 1995.
#

$final = "";

$_ = <> until /^NAME/;

$progname = "!";
$final .= "<H3>NAME</H3>\n";
until (($_ = <>) =~ /^SYNOPSIS/) {
  next if /^\s*$/;
  s/^\t//;
  ($progname) = /^\s*(\S+)/ unless $progname ne "!";
  $final .= $_;
}


$final .= "<H3>SYNOPSIS</H3>\n";
$times = 0;
until (($_ = <>) =~ /^DESCRIPTION/) {
  next if /^\s*$/;
  s/^\t//;
  s/\b($progname)\b/<b>$1<\/b>/;

  push(@options, /[^a-zA-Z0-9]-[a-zA-Z]/g);

  s/\s*\[-(.)\]\s*/ \[<b>-$1<\/b>\] /g;
  s/\s*\[-(.) (\S+)\]\s*/ \[<b>-$1<\/b> $2 \] /g;
  s/\s*-(.) ([^-]\S*)\s*/ <b>-$1<\/b> $2 /g;
  $final .= $_;
}
$final .= "\n";
$reploptions = "";
foreach $opt (@options) {
  $opt =~ s/^(.|\n)-//;
  $reploptions .=
           "s/(^|[^a-zA-Z0-9])-$opt([^a-zA-Z0-9])/\$1<b>-$opt<\/b>\$2/g;\n";
}


$final .= "<H3>DESCRIPTION</H3>\n";
until (($_ = <>) =~ /^OPTIONS/) {
  if (/^\s*$/) {
    $final .= "\n<P>\n";
    next;
  }
  s/^\t//;
  if (/^\s/) {
    $final .= "<PRE>\n";
    $final .= $_;
    $final .= "</PRE>\n";
    next;
  }
  s/\b($progname)\b/<i>$1<\/i>/g;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions;
  $final .= $_;
}


$final .= "<H3>OPTIONS</H3>\n";
$final .= "<DL>\n";
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
      $final .= "\n<DT>-$opt\n";
      $final .= "<DD>";
    } else {
      $final .= "\n<P>\n" if ($intext == 0);
      $intext = 1;
    }
  }
  s/\b($progname)\b/<i>$1<\/i>/g;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions;
  s/^\s+//;
  $final .= $_;
}
$final .= "</DL>\n";


$final .= "<H3>EXAMPLES</H3>\n";
$indent = 0;
until (($_ = <>) =~ /^FILES/) {
  if (/^\s*$/) {
    $final .= "\n<P>\n";
    next;
  }
  s/^\t//;
  if ( ($indent == 1) && (/^\S/) ) {
    $final .= "\n</PRE>\n";
    $indent = 0;
  }
  if ( ($indent == 0) && (/^\s/) ) {
    $final .= "\n<PRE>\n";
    $indent = 1;
  }
  s/\b($progname)\b/<i>$1<\/i>/g unless $indent;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  #s/^\s+//;
  $final .= $_;
}
$final .= "\n</PRE>\n" if ($indent == 1);


$final .= "<H3>FILES</H3>\n";
$final .= "<DL>\n";
$indent = -1;
until (($_ = <>) =~ /^DIAGNOSTICS/) {
  if (/^\s*$/) {
    #$final .= "\n<P>\n";
    next;
  }
  s/^\t//;
  if (/^\S/) {
    if ($indent == 1) {
      $final .= "\n<DD>";
    } elsif ($indent == -1) {
      $final .= "\n<DT>";
    }
    $indent = 0;
  } else {
    if ($indent == 0) {
      $final .= "\n<DD>";
    } elsif ($indent == -1) {
      $final .= "\n<DT>";
    }
    $indent = 1;
  }
  s/\b($progname)\b/<i>$1<\/i>/g if $indent;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  s/^\s+//;
  $final .= $_;
}
$final .= "</DL>\n";


$final .= "<H3>DIAGNOSTICS</H3>\n";
$indent = -1;
until (($_ = <>) =~ /^RESTRICTIONS/) {
  if (/^\s*$/) {
    #$final .= "\n<P>\n";
    next;
  }
  s/^\t {0,2}//;
  if (/^\S/) {
    if ($indent == 1) {
      $final .= "\n<PRE>\n\t";
    } elsif ($indent == -1) {
      $final .= "\n<PRE>\n\t";
    } elsif ($index == 0) {
      $final .= "\t";
    }
    $indent = 0;
  } else {
    if ($indent == 0) {
      $final .= "\n</PRE>\n";
    } elsif ($indent == -1) {
      $final .= "\n\n";
    } elsif ($indent == 1) {
      $final .= "\n\n" if (/^\t\t/);   # allow tables inside text
    }
    $indent = 1;
  }
  s/\b($progname)\b/<i>$1<\/i>/g if $indent;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions if $indent;
  #s/^\s*\./\\&./g;
  #if (/^\t\t/) {
  #  s/^\s+/\t/;    # indent tables inside text
  #} else {
  #  s/^\s+//;
  #}
  $final .= $_;
}
$final .= "\n</PRE>\n" if ($indent == 0);


$endmode = "<P>";
$final .= "<H3>RESTRICTIONS</H3>\n";
until (($_ = <>) =~ /^FUTURE DIRECTIONS/) {
  if (/^\s*$/) {
    $final .= "\n$endmode\n";
    $endmode = "<P>";
    next;
  }
  s/^\t//;
  if (/^\s/) {
    $final .= "\n<PRE>\n";
    $endmode = "</PRE>";
  }
  s/\b($progname)\b/<i>$1<\/i>/g;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions;
  $final .= $_;
}

$endmode = "<P>";
$final .= "<H3>FUTURE DIRECTIONS</H3>\n";
until (($_ = <>) =~ /^HISTORY/) {
  if (/^\s*$/) {
    $final .= "\n$endmode\n";
    $endmode = "<P>";
    next;
  }
  s/^\t//;
  if (/^\s/) {
    $final .= "\n<PRE>\n";
    $endmode = "</PRE>";
  }
  s/\b($progname)\b/<i>$1<\/i>/g;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions;
  $final .= $_;
}


$final .= "<H3>HISTORY</H3>\n";
$final .= "<DL>\n";
until (($_ = <>) =~ /^BUGS/) {
  next if /^\s*$/;
  s/^\t {0,2}//;
  if (/^\S/) {
    ($date) = /^([\d\/\?]+)\s+/;
    s/^([\d\/\?]+)\s+//;
    $final .= "<DT>$date\n";
    $final .= "<DD>";
  }
  s/\b($progname)\b/<i>$1<\/i>/g;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions;
  s/^\s+//;
  $final .= $_;
}
$final .= "</DL>\n";


$final .= "<H3>BUGS</H3>\n";
until (($_ = <>) =~ /^SEE ALSO/) {
  if (/^\s*$/) {
    $final .= "\n<P>\n";
    next;
  }
  s/^\t//;
  s/\b($progname)\b/<i>$1<\/i>/g;
  s/\{([^\s}]+)\}/<i>$1<\/i>/g;
  eval $reploptions;
  $final .= $_;
}


$final .= "<H3>SEE ALSO</H3>\n";
$final .= "<DL>\n";
$doipw = 0;
while (<>) {
  if (eof()) {
    chop;
    $last = $_;
    next;
  }
  if (/^\s*$/) {
    $doipw = 0;
    $final .= "\n<P>\n";
    next;
  }
  s/^\t//;
  if (/^(\w+):/) {
    if ($1 eq 'IPW') {
      $doipw = 1;
    } else {
      $doipw = 0;
    }
    $final .= "\n<DT>$1\n";
    $final .= "<DD>";
    s/^(\w+):\s+//;
  }
  if ($doipw) {
    s/\b(\S+)\b/<A href="$1.html">$1<\/A>/g;
  }
  s/^\s*//;
  $final .= $_;
}
$final .= "\n</DL>\n";





$final =~ s/\n\n/\n/g;
#1 while $final =~ s/(\.SH [A-Z ]+)\n(\.LP\n)*\.SH /$1\n.rs\n.sp .5v\n.SH /g;
#$final =~ s/\.LP\n\.RS/.RS/g;

($ftype, $fdate, $fauth) = split(/  +/, $last);
if ( ($fdate !~ /..\/..\/../) && ($fauth =~ /^\s*$/) ) {
  $fauth = $fdate;
  $fdate = '';
}

#
# print header information
#
print "<HTML>\n";
print "<HEAD>\n";
print "<TITLE>$progname -- $ftype</TITLE>\n";
print '<LINK REV="made" HREF="mailto:dana@acm.org">';
print "\n</HEAD>\n";

print "<BODY><H2 align=center>$progname -- $ftype</H2>\n\n";
$progname =~ tr/a-z/A-Z/;
      "\"\" \"$fdate\" \"$fauth\" \"$ftype\"\n";

#
# The manual page
#
print $final;

#
# Any trailer information
#
$today = "6 March 1995";
print "\n\n";
print "<HR>\n";
print "This page describes <b>$progname</b>, written by $fauth, version $fdate.\n";
print "<ADDRESS>\n";
print "Generated by <A href=\"http:/www.ecst.csuchico.edu/~jacobsd/\">imtohtml</A>\n";
print "on $today by dana@acm.org.\n";
print "</ADDRESS>\n</BODY></HTML>\n";

