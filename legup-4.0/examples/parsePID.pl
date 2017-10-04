#!/usr/bin/perl
#
# USAGE: perl parsePID.pl in.rpt -- produces out.in.rpt
#
# This script is meant to be used after performing 'make memProf' and
# executing the resultant executable to produce a log of memory accesses.
# It takes in the .log file and creates an output log which converts the
# seemingly random tid's from the pthread_self() call into contiguous
# numbers i.e. 0, 1, 2, 3, ...
#

use warnings;
use strict;

use List::MoreUtils qw{any};

# expected input file <name>.log
my $in = $ARGV[0];
open(INFILE, "<$in") || die "Error opening file $in for reading: $!\n";

# get name of out file
my @inName = split(/\./, $in);
my $out = "out.";
my $i = 0;
for ($i = 0; $i < $#inName; $i++) {
    $out .= $inName[$i] . ".";
}
$out .= $inName[$i];
open(OUTFILE, ">$out") || die "Error opening file $out for writing: $!\n";

print "Processing file $in to produce $out\n";

# get number of threads found
my @TIDArray = ();

foreach my $line (<INFILE>) {
    if (($line =~ /{tid: (\d+)}$/) && not any {$_ eq $1} @TIDArray) {
    #if (($line =~ /{tid: (\d+)}$/) && not ($1 ~~ @TIDArray)) {
        push(@TIDArray, $1);
    }
}

print("Found " . ($#TIDArray + 1) . " distinct thread ids\n");

# sort array by ascending order
@TIDArray = sort{$a <=> $b} @TIDArray;

# create a hash table out of array
my %TIDHash = ();
my $newTID = 0; # omp thread ids begin from 0
foreach my $key (@TIDArray) {
    $TIDHash{ $key } = $newTID;
    $newTID++;
}

# reread file
seek INFILE, 0, 0;

foreach my $line (<INFILE>) {
    if (($line =~ /{tid: (\d+)}$/) && any {$_ eq $1} @TIDArray) {
        my $newLine = $line;
        $newLine =~ s/{tid: (\d+)}$/{tid: $TIDHash{$1}}/;
        print OUTFILE $newLine;
    } else {
        print OUTFILE $line;
    }
}

close (INFILE);
close (OUTFILE);
