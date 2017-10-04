# Lanny: May-26-2012
# This script can be used to find functions' hash number based on input *.src and *.hash
#	can also be used to parse profiling_result.log if the file exists at CWD
#!/usr/bin/perl
use warnings;
use strict;
use integer;
use POSIX qw/ceil/;

defined $ARGV[0] or die "Expect *.hash_table.log filename as argument 0.";
defined $ARGV[1] or die "Expect *.src filename as argument 1.";
defined $ARGV[2] or die "Expect profiling_result.log filename as argument 2.";

my $N   = undef;
my @tab = ();
my $V1  = undef;
my $A1  = undef;
my $A2  = undef;
my $B1  = undef;
my $B2  = undef;

#$N = scalar(split /-/, $string);

my @funcs = (); 
open SRC, $ARGV[1] or die $!;
while (<SRC>)	{
	chomp ($_);
	if ($_ =~ />:/) {
		my $tmp_l = $_;
		$tmp_l =~ s/://;
		#seems not necessary to check starting address
		#die "Error: parsing $tmp_l, expect <main> as the first function starting at 0x00800020.\n" if ($tmp_l ne "00800020 <main>" and scalar(@funcs) == 0);
		push(@funcs, $tmp_l);
	}
	elsif ($_ =~ "Disassembly of section" and $_ !~ "Disassembly of section .text") {
		last;
	}
}
close (SRC);

#my @funcs = `grep \\\>\\\: $ARGV[1] | sed 's/\\\://' `;
my $main_data = ""; # counter value of the entire program ( main function )

foreach my $func (@funcs) {
	chomp ($func);
	my $addr = $func; $addr =~ s/ .*//;
	# The perfect hash take the LS 26 bits as input~
	$addr =~ s/^4/0/;
	$addr =~ s/^8/0/;
	my $name = $func; $name =~ s/.* //;
	if ($name =~ /\./) {next;}
	my $cmd = sprintf("grep 0x%x $ARGV[0]", hex($addr) );
	my $rslt = `$cmd`;
	$rslt =~ s/^.*--->\s*//;
	$rslt =~ s/\s*\n//;

	if ( -e $ARGV[2]) {
		my @data_ = `grep '^FuncNum-$rslt  Data-'  $ARGV[2] | sed 's/.*Data-\\s*//'`;
		my $data = $data_[0];
		$data =~ s/\n.*//;
		chomp ($data);
		my $percentage = "";
		if (defined $ARGV[3] and $ARGV[3] eq "SHOW_PERCENTAGE") {
			$main_data = $main_data eq "" ? $data : $main_data;
			if (int($data)*100.0/int($main_data) >= 1) {  $percentage = sprintf ("%3d", int($data)*100.0/int($main_data)); }
			else                                       {  $percentage =          " <1"; }
			printf ("Address $addr, Hash-Func Number $rslt, Count %6d, Percentage $percentage%%, Function $name\n", int($data) );
		}
		else {
			printf ("Address $addr, Hash-Func Number $rslt, Count %6d, Function $name\n", int($data) );
		}
	} else {
		printf ("Address $addr, Hash-Func Number $rslt, Function $name\n");
	}
}



