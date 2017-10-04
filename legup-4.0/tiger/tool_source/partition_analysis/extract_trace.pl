# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0]) or die "Expect the *.emul.src file as argument 0.\n";
(defined $ARGV[1]) or die "Expect the filename of gxemul trace (*.raw.trace) as argument 1.\n";
(defined $ARGV[2]) or die "Expect the filename of extracted load trace as argument 2.\n";
(defined $ARGV[3]) or die "Expect the filename of extracted store trace as argument 3.\n";

my @ld_ops = ("lb", "lbu", "lh", "lhu", "lw", "lwcz", "lwl", "lwr");
my @sw_ops = ("sb", "sh", "sw", "swcz", "swl", "swr");

# find function names and addresses from *.emul.src
# the function name can be wrong from gxemul's output, but address must be right
my $f_addr_to_name = {};
open SRC, $ARGV[0] or die $!;
while (<SRC>) {
	chop;
	if ($_ =~ />:/) { # i.e., "80030000 <main>:"
		my $faddr = $_;
		$faddr =~ s/ .*//;
		my $fname = $_;
		$fname =~ s/.* <//;
		$fname =~ s/>:.*//;
		$f_addr_to_name->{$faddr} = $fname;
	}
	elsif ($_ =~ "Disassembly of section" and $_ !~ "Disassembly of section .text") {
		last;
	}
}
close SRC;

# extracting
open SRC, $ARGV[1] or die $!;
open DST_ld, ">".$ARGV[2] or die $!;
open DST_sw, ">".$ARGV[3] or die $!;

my $l = "";
my $init_sp = "a0007f00";
my $sp = $init_sp;
my $op = "";
while (<SRC>) {
	chop;
	$l = $_;
	$l =~ s/[^\t]*\t//;  # remove pc and instruction
	
	if ($l =~ /\[.*\]/) {    # memory access, store or load
		$op = $l;
		$op =~ s/\s.*//;
		if (grep {$_ eq $op} @ld_ops) {    # load operations
			$l =~ s/.*\[0xffffffff//;
			$l =~ s/].*//;
			print DST_ld $l."\n";
		}
		elsif (grep {$_ eq $op} @sw_ops) {    # store operations
			$l =~ s/.*\[0xffffffff//;
			$l =~ s/].*//;
			$l =~ s/\s.*//;
			print DST_sw $l."\n";
		}
	}
	elsif ($l =~ /jal\t[^\s]*\t\<.*\>/) {  # calling a function - "jal	0xffffffff8003007c	<eginfo>"
		$l =~ s/jal\t0xffffffff//;
		$l =~ s/\t<.*//; 
		print DST_ld "<calling ".$f_addr_to_name->{$l}."> sp->".$sp."\n";
		print DST_sw "<calling ".$f_addr_to_name->{$l}."> sp->".$sp."\n";
	}
	elsif ($l =~ /jr\tra\t\<.*\>/ or $l =~ /jr\tra$/) {  # returning
		print DST_ld "<returning>\n";
		print DST_sw "<returning>\n";
	}
	elsif ($l =~ /addiu\tsp,sp,/) {  # sp is changing
		$l =~ s/addiu\tsp,sp,//;
		$sp = sprintf ("%08x", hex($sp)+int($l));
	}
	else {  # check if there is other instructions that change sp
		if ($l =~ /[a-z]*\tsp/) {
			if ($l !~ ".*\tsp,fp,zr") {	# this instruction is ok to ignore
				print "Warning: unknown instruction is changing sp - $l \n";
			}
		}
	}
}

close SRC;
close DST_ld;
close DST_sw;

($sp eq $init_sp) or die "Error: sp is not properly traced - sp = $sp at end of execution\n";

