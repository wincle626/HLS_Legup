# Lanny: June-23-2012
#!/usr/bin/perl
use warnings;
use strict;

defined $ARGV[0] or die "Expect sdram.dat file as argument 0.";
(-e $ARGV[0]) or die "File $ARGV[0] does not exist.";

my $num_lines = 0;
# read sdram.dat into array
open SRC, $ARGV[0] or die $!;
my @src_array=<SRC>;
close (SRC);

# get the array size
my $ori_size = scalar(@src_array);

# go through the array from the end
my $last_line = $src_array [scalar(@src_array) -1];
chomp ($last_line);
my $second_last_line = $src_array [scalar(@src_array) -2];
chomp ($second_last_line);

# remove 8 zeros at a time
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

# print to a new file
my $count = 0;
open DST, ">program.dat" or die $!;
foreach my $half_word (@src_array) {
	chomp ($half_word);
	my $first_byte = $half_word;
	my $second_byte = $half_word;
	$first_byte =~ s/^..//;
	$second_byte =~ s/..$//;
	print DST "0x".$first_byte." "."0x".$second_byte." ";
	
	$count = $count + 2;
	if ( $count == 256) {
		print DST "\n";
		$count = 0;
	}
}
close (DST);
