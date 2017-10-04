#!/usr/bin/perl
#
# USAGE: compare.pl benchmark.csv group/benchmark.csv localrams/benchmark.csv
#
# Creates a nice table comparing multiple runs so you can copy into an open office spreadsheet
#
# For example:
# 1. Create baseline results
#       Backup the chstone benchmarks into the 'baseline' folder:
#             ./benchmark.pl -b baseline
#             cd baseline
#       Run modelsim and tests:
#             runtest
#       Run quartus:
#             ../benchmark.pl -q
#       Parse results
#             ../benchmark.pl
#
# 2. Create new results (in this case local RAMs ON)
#       Backup the chstone benchmarks into the 'localrams' folder:
#             ./benchmark.pl -b localrams
#             cd localrams
#       Run modelsim and tests:
#             LEGUP_LOCAL_RAMS=1 runtest
#       Run quartus:
#             ../benchmark.pl -q
#       Parse results
#             ../benchmark.pl
# 
# 3. Compare results:
#       compare.pl baseline/benchmark.csv localrams/benchmark.csv

use warnings;
use strict;
use Cwd;
use Data::Dumper;
use Getopt::Long;

my @benchmarks = qw|chstone/adpcm  chstone/aes  chstone/blowfish  chstone/dfadd
chstone/dfdiv  chstone/dfmul  chstone/dfsin  chstone/gsm  chstone/jpeg
chstone/mips  chstone/motion  chstone/sha dhrystone|;

# use ignore to skip some benchmarks if they aren't working
my %ignore = (
    'chstone/jpeg' => 1,
    #'chstone/motion' => 1,
    #'dhrystone' => 1
);

my %tables = (
    # leave out time -- let excel calculate automatically
    #'performance' => ['cycles', 'Fmax', 'time'],
    'performance' => ['cycles', 'Fmax'],
    'area' => ['membits', 'M9K', 'M144K','LEs', 'regs'],
);




my @files = @ARGV;

# datastructure stats holds
# { 'group/benchmark.csv' =>
#       {'chstone/adpcm' =>
#          {'time' => '222',
#           'cycles' => '24809',
#           ... 
#       {'chstone/aes' =>
#           ...
# { 'localrams/benchmark.csv' =>
#       ...
my %stats = ();
foreach my $file (@files) {
    die "Not a valid file: '$file'\n" unless (-f $file);

    open (FILE, "<$file") or die("Couldn't open '$file': $!\n");

    $stats{$file} = ();
    my $header_line = <FILE>;
    chomp $header_line;
    my @headers = split / /, $header_line;
    die unless ($headers[0] eq 'name');

    #print Dumper(\@headers);

    while(my $line = <FILE>) {
        chomp $line;
        #print "line $line\n";
        my @values = split / /, $line;
        my $benchmark = $values[0];
        #print "benchmark $benchmark\n";

        $stats{$file}{$benchmark} = ();
        foreach my $i (0..scalar(@values)-1) {
            my $metric = $headers[$i];
            my $value = $values[$i];
            #print "$i $metric $value \n";
            $stats{$file}{$benchmark}{$metric} = $value;
        }
    }

    close(FILE);
}

#print Dumper(\%stats);
#exit;

foreach my $table (keys %tables) {
    my @metrics = @{$tables{$table}};
    print "Table: $table\n";
    print " ";
    foreach my $metric (@metrics) {
        print "$metric ";
    }
    print "\n";

    foreach my $benchmark ('Benchmark', @benchmarks) {
        print "$benchmark ";

        if ($benchmark eq 'Benchmark') {
            # header
            foreach my $file (@files) {
                print "$file ";
            }
        } else {
            foreach my $metric (@metrics) {
                foreach my $file (@files) {
                    if (exists $ignore{$benchmark}) {
                        print "  ";
                    } else {
                        #print "$file $benchmark $metric\n";
                        print $stats{$file}{$benchmark}{$metric}." ";
                    }
                }
            }
        }
        print "\n";
    }
}
#printf "%-20s", "geomean";
#foreach my $key (@metric_keys) {
#	printf " & %-10.2f", $geomean{$key};
#}
#print " \\\\\n";
#print "\n";

