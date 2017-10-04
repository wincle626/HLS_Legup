# Lanny: June-27-2012
# This file is used to change hash parameters in verilog files for simulation purpose.
# Note:
#	Using -gParameter=<...> in vsim command is the other way to pass in parameter.
#	However, this approach(-gParam) is limited when the parameter is not present at the top level.
#	i.e., In order to simulate the whole sopc system, hash registers are initialized
#	by forcing the output interface of jtag_master. The hash parameters are coded within that
#	file. In this case, -gParameter cannot pass down the parameters

#!/usr/bin/perl
use warnings;
use strict;
use POSIX qw/ceil/;
# example hash file
#	tab[] = {0,12,13,0,0,0,0,8,14,5,4,0,15,11,0,0,}
#	V1 = 0x9e3779b9
#	A1 = 32
#	A2 = 28
#	B1 = 0
#	B2 = 0xf

( scalar(@ARGV)==2 or scalar(@ARGV)==4 ) or die "Expect either 2 or 4 arguments.";
defined $ARGV[0] or die "Expect *.v (verilog file that declares the parameters) filename as argument 2.";
defined $ARGV[1] or die "Expect *.hash filename as argument 0.";

my $N   =0;
my $tab ="";
my $V1  ="";
my $A1  ="";
my $A2  ="";
my $B1  ="";
my $B2  ="";
my $TRACE_SIZE  = undef;
my $STARTING_PC = undef;

open HASH, $ARGV[1] or die $!;
while (<HASH>) {
	chomp($_);
	my $line = $_;
	if ($line =~ "tab") {
		$line =~ s/.*{//;
		$line =~ s/,}//;
		
		foreach my $element (split /,/, $line) {
			my $element_in_hex ="";
			$element_in_hex = sprintf ("%02x", int($element));
			$tab = $element_in_hex.$tab;
			$N += 1;
		}
		my $head = ($N*8)."\\'h";
		$tab = $head.$tab;
	}
	elsif ($line =~ "V1") {
		$line =~ s/.*0x/32\\'h/;
		$V1 = $line;
	}
	elsif ($line =~ "A1") {
		$line =~ s/.*= /8\\'d/;
		$A1 = $line;
	}
	elsif ($line =~ "A2") {
		$line =~ s/.*= /8\\'d/;
		$A2 = $line;
	}
	elsif ($line =~ "B1") {
		$line =~ s/.*= /8\\'d/;
		$B1 = $line;
	}
	elsif ($line =~ "B2") {
		$line =~ s/.*0x/8\\'h/;
		$B2 = $line;
	}
}
close (HASH);

system ("sed -i \"s/parameter ACTUAL_FUNC_NUM = .*/parameter ACTUAL_FUNC_NUM = $N,/\" $ARGV[0]");
system ("sed -i \"s/parameter tab = .*/parameter tab = $tab,/\" $ARGV[0]");
system ("sed -i \"s/parameter V1 = .*/parameter V1 = $V1,/\" $ARGV[0]");
system ("sed -i \"s/parameter A1 = .*/parameter A1 = $A1,/\" $ARGV[0]");
system ("sed -i \"s/parameter A2 = .*/parameter A2 = $A2,/\" $ARGV[0]");
system ("sed -i \"s/parameter B1 = .*/parameter B1 = $B1,/\" $ARGV[0]");
system ("sed -i \"s/parameter B2 = .*/parameter B2 = $B2/\" $ARGV[0]");

# TRACE SIZE & STARTING_PC need to be provided for leap_tb (the testbench only simulates leap profiler)
if (scalar(@ARGV)==4 ) {
	$TRACE_SIZE =`cat $ARGV[3] | wc -l`;
	chomp ($TRACE_SIZE);

	# Find the starting pc of the program
	$STARTING_PC =`grep \\<main\\> $ARGV[4] | sed 's/ .*//'`;
	defined $STARTING_PC or die "Error: function <main> not found in $ARGV[4]\n.";
	chomp ($STARTING_PC);
	$STARTING_PC = "32\\'h".$STARTING_PC;
	
	system ("sed -i \"s/parameter TRACE_SIZE = .*/parameter TRACE_SIZE = $TRACE_SIZE,/\" $ARGV[0]");
	system ("sed -i \"s/parameter STARTING_PC = .*/parameter STARTING_PC = $STARTING_PC,/\" $ARGV[0]");
}

