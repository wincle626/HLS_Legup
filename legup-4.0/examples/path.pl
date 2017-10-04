#!/usr/bin/perl 

# Find the path between two signals in a LegUp verilog file
# Useful for analysing the critical path
# Usage:
#       ./path.pl <start_signal> <end_signal> <verilog_file>

use strict;
use Data::Dumper;

my $start = $ARGV[0];
my $end = $ARGV[1];
my $file = $ARGV[2];

print "Finding all fanout of signal: $start\n";
print "In file: $file\n";

# build up a graph of signal fanout / fanin
my %lines = ();
my %fanin = ();
my %fanout = ();
open (FILE, "<$file") or die "Can't open file! $!\n";
my $comment;
while(my $line = <FILE>) {
    if ($line =~ /^always/) {
        $comment = "\n";
    }
    if ($line =~ /^\/\*/) {
        $comment = $line;
    }
    if ($line =~ /^(\S+)\s*<?=\s*(.*)$/) {
        my $sink = $1;
        my $rest = $2;
        #print $line;
        chomp $line;
        $lines{$sink} = $line."\t".$comment;
        #print "sink: $sink\n";
        while ($rest =~ /(\w+)/g) {
            my $fanin = $1;
            next if ($fanin =~ /^d?\d+$/);
            push @{$fanin{$sink}}, $fanin;
            push @{$fanout{$fanin}}, $sink;
            #print "fanin: $1\n";
        }
    }
}
close(FILE);
#print Dumper(\%lines);
#print Dumper(\%fanin);
#print Dumper(\%fanout);

# recursively search for end signal
my @path = ();
#find_path_forward($start, $end, \@path);
#@path = reverse @path;
find_path_backward($start, $end, \@path);
print Dumper(\@path);

foreach my $signal (@path) {
    print $lines{$signal};
    #print "$signal\n"
}

sub find_path_backward {
    my $start = shift;
    my $end = shift;
    my $path = shift;
    print "$start -> $end\n";

    foreach my $fi (@{$fanin{$end}}) {
        # cut at registers
        if ($fi eq $start) {
            print "Found!!\n";
            push @$path, $fi;
            return 1;
        }
        next if ($fi =~ /_reg$/);
        if (find_path_backward($start, $fi, $path)) {
            push @$path, $fi;
            return 1;
        }
    }
    return 0;
}


sub find_path_forward {
    my $start = shift;
    my $end = shift;
    my $path = shift;
    print "$start -> $end\n";

    foreach my $fo (@{$fanout{$start}}) {
        # cut at registers
        if ($fo eq $end) {
            print "Found!!\n";
            push @$path, $fo;
            return 1;
        }
        next if ($fo =~ /_reg$/);
        if (find_path_forward($fo, $end, $path)) {
            push @$path, $fo;
            return 1;
        }
    }
    return 0;
}
