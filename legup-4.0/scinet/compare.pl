#!/usr/bin/perl
#
# Compares two scinet runs by parsing their benchmark.csv files
# ./benchmark.pl must already be run on the directory to produce the benchmark.csv
# USAGE: ./compare.pl <dir1> <dir2>
#

use warnings;
use strict;
use Cwd;
use Data::Dumper;
use Getopt::Long;


# example benchmark.csv:
# name time cycles Fmax LEs regs comb mults membits logicUtil 
# chstone/adpcm 230 35178 152.77 8327 10492 9355 220 27072 14162 
# chstone/dfdiv 13 2407 178.64 6295 9741 5814 24 12416 10675 
# chstone/dfmul 1 348 253.74 1968 2371 2165 16 12032 3371 
# chstone/dfsin 666 71152 106.91 12043 16445 12130 44 12832 21032 
# chstone/gsm 37 6176 169.2 5015 6209 6285 58 10144 8430 
# chstone/jpeg 19671 1543216 78.45 17705 20766 22616 184 468784 31226 
# chstone/mips 31 6888 219.15 1745 2094 2099 8 4480 3002 
# dhrystone 40 7592 188.86 2775 3646 3482 4 2256 4920 

die "./compare.pl <dir1> <dir2>\n" unless scalar(@ARGV) == 2;

my $dir1 = $ARGV[0];
my $dir2 = $ARGV[1];
print "Comparing $dir1 to $dir2\n";

my @metric_keys = qw(time cycles Fmax LEs regs comb mults membits logicUtil);
my @benchmarks = qw|chstone/adpcm chstone/dfdiv chstone/dfmul chstone/dfsin chstone/gsm chstone/jpeg chstone/mips dhrystone|;

my %results = ();

my $pwd = &Cwd::cwd();
foreach my $dir ($dir1, $dir2) {
    chdir "$dir/examples" or die "$!";
    #system("./benchmark.pl");
    # parse benchmark.csv file
    open(CSVFILE, '<', 'benchmark.csv') || die "Error: $!\n";
    my $firstline = <CSVFILE>;
    while (my $line = <CSVFILE>) {
        my @metrics = split /\s+/, $line;
        my $name = shift @metrics;
        foreach my $key (@metric_keys) {
            $results{$dir}->{$name}->{$key} = shift @metrics;
        }
        #print Dumper(\%results);
    }
    close(CSVFILE);

    chdir $pwd or die "$!";
}

# calculate geomean
foreach my $key (@metric_keys) {
    $results{$dir1}->{'geomean'}->{$key} = 1;
    $results{$dir2}->{'geomean'}->{$key} = 1;
}
my $count = 0;
foreach my $name (@benchmarks) {
    $count++;
    foreach my $key (@metric_keys) {
        my $d1 = $results{$dir1}->{$name}->{$key};
        my $d2 = $results{$dir2}->{$name}->{$key};
        # for 0 entries just assume 1. otherwise geomean will just equal 0.
        if ($d1 != 0) {
            $results{$dir1}->{'geomean'}->{$key} *= $d1;
        }
        if ($d2 != 0) {
            $results{$dir2}->{'geomean'}->{$key} *= $d2;
        }
    }
}
$count = 1 if ($count == 0); # avoid division by zero
foreach my $key (@metric_keys) {
    $results{$dir1}->{'geomean'}->{$key} **= 1.0/$count;
    $results{$dir2}->{'geomean'}->{$key} **= 1.0/$count;
}




# print out metrics
foreach my $key (@metric_keys) {
    printf "%-20s", "$key";
    printf "%-40s", "$dir1";
    printf "%-40s", "$dir2";
    printf "%-10s", "Abs";
    printf "%-10s", "%";
    print "\n";
    foreach my $name (@benchmarks, 'geomean') {
        printf "%-20s", $name;
        my $d1 = $results{$dir1}->{$name}->{$key};
        my $d2 = $results{$dir2}->{$name}->{$key};
        printf "%-40s", $d1;
        printf "%-40s", $d2;
        printf "%-10.2f", $d2-$d1;
        printf "%-10.2f", ($d1 == 0) ? 0 : ($d2-$d1)/$d1*100;
        print "\n";
    }
    print "\n";
}
