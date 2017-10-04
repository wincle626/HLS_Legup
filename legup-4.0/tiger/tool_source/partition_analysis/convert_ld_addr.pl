# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0] and (-e $ARGV[0])) or die "Expect the *.hw_accessed_gv.rpt as argument 0.";
(defined $ARGV[1] and (-e $ARGV[1])) or die "Expect the file *.gv_table.src as argument 1.";
(defined $ARGV[2] and (-e $ARGV[2])) or die "Expect the load addr trace (*.filtered.load.trace as argument 2.";
(defined $ARGV[3]) or die "Expect the filename of converted load addr trace as argument 3.";

# parse all the global variables that are directed accessed by HW. (NOT including those accessed by pointer argument!!!)
my @hw_gv=();
open HW_GV, $ARGV[0] or die $!;
while (<HW_GV>) {
	chop;
	push @hw_gv, $_;
}
close HW_GV;

# starting value of sp in tiger and gxeml
my $starting_sp_tiger = hex("0x01000000");
my $starting_sp_gxeml = hex("0xa0007f00");
my $sp_diff = $starting_sp_gxeml - $starting_sp_tiger;
# any address greater than boundary should be in STACK space (gxemul)
my $boundary = hex("0x90000000");

# global variable information in actual hybrid binary src
my $gv_saddr = {};	#starting address
my $gv_const = {};	#whether the gv is constant

# parse global variable from gv_table.src
open SRC, $ARGV[1] or die $_;
while (<SRC>) {
	# lines in gv_table.src: <0080245c g     O .bss	00000004 evalue>
	chop;
	$_ =~ s/\s\.hidden\s/ /;
	my @tmp = split(/\s+/);
	scalar(@tmp) == 6 or die "Error: Only ".scalar(@tmp)." columns - $_.\n\tGlobal Variable Table is expected to have 6 columns per line.\n";
	$gv_saddr->{$tmp[5]} = hex($tmp[0]);
	$gv_const->{$tmp[5]} = ( ($tmp[3] eq ".rodata") and (grep {$_ eq $tmp[5]} @hw_gv) )? 1 : 0;
}
close SRC;

# convert load addresses
open TRACE, $ARGV[2] or die $_;
open CONVERT, ">".$ARGV[3] or die $_;
while (<TRACE>){
	chop;
	my $l = $_;
	if ($l =~ /^\<.*\>/){
		if ($l =~ /^\<calling/) {
			my $sp = $l;
			$sp =~ s/.*\> sp-\>//;
			$sp = sprintf ("%08x", hex($sp) - $sp_diff);
			$l =~ s/\> sp-\>.*/\> sp-\>$sp/;
		}
		print CONVERT $l."\n";
		next;
	}

	if ($l =~ / = /) { # Global Variable
		my $gv_name = $l;
		$gv_name =~ s/.* = //;
		$gv_name =~ s/\+.*//;

		my $addr_plus = 0;
		if ($l =~ / = .*\+0x/) {
			$addr_plus = $l;
			$addr_plus =~ s/.*\+0x//;
			$addr_plus = hex($addr_plus);
		}
		
		if (! exists $gv_saddr->{$gv_name} ) {
			print "Warning: Global Variable $gv_name not found in gv_table. Eliminating...\n";
			next;
		}

		$l = sprintf ("%08x", $gv_saddr->{$gv_name} + $addr_plus);
		$l = ($gv_const->{$gv_name} == 1) ? $l." <CONST>" : $l;
		print CONVERT $l."\n";
	}
	else {
		my $addr = hex($l);
		print "Warning: Address ($l) is lower than boundary ($boundary), but converted as stack.\n" if $addr <= $boundary;
		$l = sprintf("%08x", $addr - $sp_diff);
		print CONVERT $l."\n";
	}
}
close TRACE;
close CONVERT;

