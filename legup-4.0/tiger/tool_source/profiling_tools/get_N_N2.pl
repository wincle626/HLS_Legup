# Lanny: May-26-2012
#!/usr/bin/perl
use warnings;
use strict;
use POSIX qw/ceil/;

( (defined $ARGV[0]) and (($ARGV[0] eq "N") or ($ARGV[0] eq "N2")) )  or die "Expect N or N2 as argument 0.";
defined $ARGV[1] or die "Expect *.src filename as argument 1.";

my $num_funcs = 0;

open SRC, $ARGV[1] or die $!;
while (<SRC>)	{
	chomp ($_);
	if ($_ =~ />:/) {
		$num_funcs += 1;
	}
	elsif ($_ =~ "Disassembly of section" and $_ !~ "Disassembly of section .text") {
		last;
	}
}
close (SRC);

$num_funcs = ($num_funcs <= 16) ? 16 : $num_funcs;

my $N2 = ceil( log($num_funcs)/log(2) );
my $N = 2**$N2;

if ($ARGV[0] eq "N") { print $N;}
elsif ($ARGV[0] eq "N2") {print $N2;} 
