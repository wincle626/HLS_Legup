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

my @verilogfiles = (
"adpcm",
"aes",
"bf",
"dfadd",
"dfdiv",
"dfmul",
"dfsin",
"gsm",
"main",
"mips",
"mpeg2",
"sha_driver",
"dhry"
);

my $i = 0;
foreach my $benchmark (@benchmarks) {
    unless (-e "$benchmark/*.v" || $ARGV[0] eq "q") {
        system("cd $benchmark && make");
    }
    my $verilogfile = $verilogfiles[$i];
    my $filename = $verilogfile . "NOOP.v";
    $verilogfile = $verilogfile . ".v";
    system("cp nooptimization.pl $benchmark/.");
    chdir("$benchmark");
    system("./nooptimization.pl $verilogfile");
    system("rm $verilogfile");
    system("mv $filename $verilogfile");
    system("rm nooptimization.pl");
    $i ++;
    chdir("../..");
}

