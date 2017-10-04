#!/usr/bin/perl
use warnings;
use strict;

my @benchmarks = (
"chstone/adpcm",
"chstone/aes",
"chstone/blowfish",
"chstone/dfadd",
"chstone/dfdiv",
"chstone/dfmul",
"chstone/dfsin",
"chstone/gsm",
"chstone/jpeg",
"chstone/mips",
"chstone/motion",
"chstone/sha",
"dhrystone"
);

my @metrics = (
"Fmax",
"Logic",
"Combinational",
"Registers",
"DSP"
);

foreach my $benchmark (@benchmarks) {
    unless (-e "$benchmark/*.v" || $ARGV[0] eq "q") {
        system("cd $benchmark && make");
    }
    unless (-e "Resource.log") {
        system("touch Resource.log");
    }
    open (Fout, ">>Resource.log");
    print Fout "Benchmark: $benchmark\n";
    foreach my $metric (@metrics) {
        my $line = qx/grep $metric $benchmark\/resources.summary/;
        print Fout $line;
    }
    print Fout "\n";
}
close (Fout);

