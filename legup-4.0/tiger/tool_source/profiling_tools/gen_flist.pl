# Lanny: May-26-2012
# This script is to generate a list of function addresses and store in file *.flist 
# This script is to replace original make_flist.cpp

# !!! In original make_flist.cpp, Mark said the hash generation will fail (perfect returns
# segfault) when # of functions is 17. However, I have test the case but it actually works
# well. So I did not consider 17 keys as special case in my script.

#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0] and $ARGV[0]=~/\.src$/) or die "Expect *.src filename as argument 0.";

my @funcs = (); 
open SRC, $ARGV[0] or die $!;
while (<SRC>)	{
	chomp ($_);
	if ($_ =~ />:/) {
		my $tmp_l = $_;
		$tmp_l =~ s/://;
		$tmp_l =~ s/ .*//;
		$tmp_l =~ s/^8/0/;
		$tmp_l =~ s/$/\n/;
		push(@funcs, $tmp_l);
	}
	elsif ($_ =~ "Disassembly of section" and $_ !~ "Disassembly of section .text") {
		last;
	}
}
close SRC;

my $flist = $ARGV[0];
$flist =~ s/\.src$/\.flist/;

open DST, ">$flist" or die $!;
print DST @funcs;
close DST;

