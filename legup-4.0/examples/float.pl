#!/usr/bin/perl 

# Convert a hex float value from modelsim into the floating point number
# Usage:
#       ./float.pl <hex>

use strict;
use Data::Dumper;

my $hex = $ARGV[0];
my $binary = '';

#print "Original hex: $hex\n";

for (my $i = 0; $i < length($hex); $i++) {
    my $hchar = substr($hex, $i, 1);
    #print "hex: $hchar\n";
    my $bin = sprintf('%04b', hex($hchar));
    #print "bin: $bin\n";
    $binary .= $bin;
}

#print "Binary: $binary\n";
my $sign = substr($binary, 0, 1);
#print "Sign bit: $sign\n";
if ($sign == 0) {
    #print "Positive\n";
} else {
    #print "Negative\n";
}

my $mantissa = substr($binary, 0, 1);
my $exp = substr($binary, 1, 8);
my $num = substr($binary, 9, 23);

#$hex = "358637bd";
#$hex = "403b8811bb1366e4";
my $type = '';
if (length($hex) == 8) {
    $type = 'f'; # float
} elsif (length($hex) == 16) {
    $type = 'd'; # double
} else {
    die;
}

my $float = unpack $type, reverse pack "H*", $hex;

print "Float: $float\n";
