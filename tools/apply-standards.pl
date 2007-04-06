#!/usr/bin/perl -w

use strict;
use File::Compare;

my @cfiles = glob("src/*.c src/*/*.c");

foreach my $file (@cfiles) {
  check_c_file($file);
}

sub compare_files {
  my ($filename, $newfile) = @_;
  if (compare($newfile, $filename) == 1) {
    my $backup = "$filename.bak";
    rename $filename, $backup or die "Cannot rename file: $!";
    rename $newfile, $filename or die "Cannot rename file: $!";
    print "$filename was modified.\n";
    print "Original file backed up as $backup\n";
  } else {
    unlink $newfile;
  }
}

sub check_c_file {
  my ($filename) = @_;
  my $newfile = "$filename.new";
  my $indent_opts = "-kr -i2 -nut -ncs -cp0 -l79 -T dstr -T strpool ".
                    "-T fpos_t -T FILE -T mbool -T hid_t -T hsize_t ".
                    "-T herr_t -T H5E_error_t -T GString -T gboolean ".
                    "-T GError -T gchar -T GArray -T GScanner";
  if (system("indent $indent_opts < $filename > $newfile") != 0) {
    die "Cannot fork: $?";
  }
  compare_files($filename, $newfile);
}
