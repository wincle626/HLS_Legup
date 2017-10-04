#!/usr/bin/perl
use warnings;
use strict;

die unless ($ARGV[0] ne "");
my $Vfile = $ARGV[0];
$Vfile =~ /(.+)\.v/;
my $outputfile = $1 . "NOOP.v";
my @directives = (
"/* synthesis keep */",
"/* synthesis noprune */"
);
open (Out, ">$outputfile");
open (In, "<$Vfile");
while (<In>) {
	my $line = $_;
	my $directive = "";
	$line =~ s/\/\/.*//;
	$line =~ s/\/\*.*\*\///;
	if ($line =~ /\s*(\w+)/) {
		if ($1 eq "wire") {
			$directive = $directives[0];
		} elsif ($1 eq "reg") {
			$directive = $directives[1];
		} else {
			$directive = "";
		}
		if ($directive ne "") {
			$line =~ s\;\ $directive;\;
		}
	}
	print Out $line;	
}
close (In);
close (Out);
