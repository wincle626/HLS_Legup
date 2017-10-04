# Lanny: June-24-2012
# This file is used to generate *.dat files in pre_on_board_profiling
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

defined $ARGV[0] or die "Expect *.hash filename as argument 0.";

my $tab ="";
my $V1  ="";
my $A1  ="";
my $A2  ="";
my $B1  ="";
my $B2  ="";

open HASH, $ARGV[0] or die $!;
while (<HASH>) {
	chomp($_);
	my $line = $_;
	if ($line =~ "tab") {
		$line =~ s/.*{//;
		$line =~ s/}//;
		$line =~ s/,/ /g;
		$tab = $line;	
	}
	elsif ($line =~ "V1") {
		$line =~ s/.*= //;
		$V1 = sprintf ("%08x", hex($line));
	}
	elsif ($line =~ "A1") {
		$line =~ s/.*= //;
		$A1 = sprintf ("%02x", int($line));
	}
	elsif ($line =~ "A2") {
		$line =~ s/.*= //;
		$A2 = sprintf ("%02x", int($line));
	}
	elsif ($line =~ "B1") {
		$line =~ s/.*= //;
		$B1 = sprintf ("%02x", int($line));
	}
	elsif ($line =~ "B2") {
		$line =~ s/.*= //;
		$B2 = sprintf ("%02x", hex($line));
	}
}
close (HASH);

system ("echo $tab > AH_tab.dat");
system ("echo 0x$V1 > AH_reg.dat");
system ("echo 0x$A1$A2$B1$B2 >> AH_reg.dat");
