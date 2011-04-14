#!/usr/bin/perl -w

if ($#ARGV != 0) {
  print "Usage: $0 directory\n\n";
  exit 1;
}

my $dir = $ARGV[0];
open(PIPE, "svn st -v $dir |");
while (<PIPE>) {
  if (/($dir\/.*)$/) {
    if (! -d $1) {
      print "$1\n";
    }
  }
}
