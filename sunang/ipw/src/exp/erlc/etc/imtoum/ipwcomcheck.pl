#!/usr/local/bin/perl

@files = <*/main.c */*.sh>;

foreach $file (sort @files) {
  open(FL, $file) || next;
  $st = "";
  while (<FL>) {
    next unless /^[*#][*#] [A-Z]/;
    s/^[*#][*#] //;
    $st .= $_;
  }
  close (FL);
#  $csumo = unpack("%32C*", $st);
  $csum = &positsum($st);
  if ($file =~ /.sh$/) {
    1 while $file =~ s/(\S*)\///;
    $direc = $1;
    $file =~ s/.sh$//;
    if ($file ne $direc) {
      $file = $direc . "/" . $file;
    }
  } else {
    $file =~ s/\/.*//;
  }
  if ($csum == 447771) {
    printf "%20s    DONE\n", $file;
  } else {
    printf "%20s           NOT DONE\n", $file;
  }
}

sub positsum {
  local($str) = @_;
  local($pos) = 1;
  local($sum) = 0;

  grep($sum += $pos++ * unpack("c", $_), split(//,$str));
  $sum;
}
