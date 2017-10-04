# Lanny: June-18-2012
#!/usr/bin/perl
use warnings;
use strict;

defined $ARGV[0] or die "Expect sdram.dat file as argument 0.";
(-e $ARGV[0]) or die "File $ARGV[0] is not at CWD.";

system ("mv $ARGV[0] $ARGV[0].bak");

# Find out number of functions in .text
my $num_lines = 0;
open SRC, "$ARGV[0].bak" or die $!;
my @src_array=<SRC>;
close (SRC);

my $ori_size = scalar(@src_array);

my $last_line = $src_array [scalar(@src_array) -1];
chomp ($last_line);
my $second_last_line = $src_array [scalar(@src_array) -2];
chomp ($second_last_line);

while ( ($last_line eq "0000") and ($second_last_line eq "0000") ) {
	pop(@src_array);
	pop(@src_array);
	
	$last_line = $src_array [scalar(@src_array) -1];
	chomp ($last_line);
	$second_last_line = $src_array [scalar(@src_array) -2];
	chomp ($second_last_line);
}

# add 8 zeros at the end; in case that the last non-zero machine code is an instruction that
#	expects a delay slot at the next line
push @src_array, "0000";
push @src_array, "0000";

open DST, ">$ARGV[0]" or die $!;
print DST @src_array;
close (DST);

my $new_size = scalar(@src_array);

system ("rm $ARGV[0].bak");

printf ("%d lines removed from the end of original sdram.dat\n", $ori_size-$new_size);
