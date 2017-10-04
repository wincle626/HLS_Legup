#!/usr/bin/perl
use strict;

# generate a random array input for idct

my $num = 200;
print "volatile int x[$num][64] = {\n";
foreach my $n (1..$num) {
    print "{";
    foreach my $i (1..64) {
        # range -500 to 500
        my $r = int(rand(1001)) - 500;

        print ", " if ($i > 1);
        print "$r";
    }
    print "},\n";
}
print "};\n";

foreach my $i (1..64) {
    print "volatile int x".$i."[$num] = {";
    foreach my $n (1..$num) {
        # range -500 to 500
        my $r = int(rand(1001)) - 500;

        print ", " if ($n > 1);
        print "$r";
    }
    print "};\n";
}
