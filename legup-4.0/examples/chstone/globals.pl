#!/usr/bin/perl
use warnings;
use strict;

die unless (defined $ARGV[0]);
my $file = $ARGV[0];
open (In, "<$file");
my $flag = 0;

while (<In>) {
  my $line = $_;
  if ($line =~ /Disassembly of section/) {
    if ($line =~ /data/ || $line =~ /bss/ || $line =~ /scommon/ ) {
      $flag = 1;
    } else {
      $flag = 0;
    }
  } else {
    if ($flag) {
      if ($line =~ /(.+)<(.+)>:/) {
        print "`define TAG_g_", $2, "_a", " 32'h", $1, "\n";
      }
    }
  }
}

close (In);
