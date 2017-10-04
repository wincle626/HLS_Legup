#!/usr/bin/perl
#
# USAGE: mark_labels.pl in.c > out.c
#
# This script allows LegUp to recognize loop labels by parsing in.c:
#   loop: for (i = 0; i < N; i++) {
#       c[i] = a[i] + b[i];
#   }
#
# And printing out:
#   loop: for (i = 0; i < N; i++) {
#       __legup_label("loop");
#       c[i] = a[i] + b[i];
#   }

use warnings;
use strict;

my $in = $ARGV[0];

open(FILE, "<$in") || die "Error opening $in: $!\n";

my $found = 0;
my $endBracket = 0;
my $label = '';
my $comment = 0;
my $functionName = "__legup_label";
#print "void $functionName(char* label) {}\n";
print "extern void $functionName(char* label);\n";
while (my $line = <FILE>) {
    print $line;
    if ($line =~ /\/\*/) {
        $comment = 1;
    }
    if ($line =~ /\*\//) {
        $comment = 0;
    }

    next if ($comment);

    if ($line =~ /^\s*(\w+):/) {
        $found = 1;
        $label = $1;
    }
    if ($found && $line =~ /.*{/) {
        $endBracket = 1;
    }
    if ($found && $endBracket) {
        print "$functionName(\"$label\");\n";
        $found = 0;
        $endBracket = 0;
        $label = '';
    }
}
