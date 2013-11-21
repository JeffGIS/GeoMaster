#!/usr/local/bin/perl

$/ = "\nNAME\n";

while (<>) {
  next unless /SYNOPSIS/;
  s/^\n*NAME\n//;
  s/\n?\nSYNOPSIS(.|\n)*//;
  s/(^|\n)\s+/$1/g;
  $dpos = index($_, " -- ");
  $rep = 12 - $dpos;
  $dpos = 0 if $dpos < 0;
  $spaces = ' ' x $rep;
  $spaces2 = ' ' x ($rep+$dpos+4);
  s/^/$1$spaces/;
  s/\n/\n$spaces2/g;

  print $_, "\n";
}
