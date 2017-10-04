# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0]) or die "Expect accelerator name as argument 0.";
(defined $ARGV[1]) or die "Expect the file *.emul.func_table.src as argument 1.";
(defined $ARGV[2]) or die "Expect the file *.func_table.src as argument 2.";
(defined $ARGV[3]) or die "Expect the filename of gxemul trace (*.raw.trace) as argument 3.\n";
(defined $ARGV[4]) or die "Expect the output file of instruction address as argument 4.";

my $acel_func_name = $ARGV[0];

# Note: the function name can be wrong from gxemul's output (*.raw.trace), but address must be right
# find function names, address ranges from *.emul.func_table.src
# function information is stored in hash structure with function ADDRESSes as keys
my $gxemul_func_name = {};
my $gxemul_func_size = {};
open SRC, $ARGV[1] or die $_;
while (<SRC>) {
	# lines in func_table.src: <00800e18 g     F .text	00000028 legup_sequential_accel_function>
	chop;
	my @tmp = split(/\s+/);
	if ( scalar(@tmp) != 6 ) {
		print "Warning: Only ".scalar(@tmp)." columns.\n\tFunction Table is expected to have 6 columns per line.\n\tEliminating $_\n";
		next;
	}
	$gxemul_func_name->{hex($tmp[0])} = $tmp[5];
	$gxemul_func_size->{hex($tmp[0])} = hex($tmp[4]);
}
close SRC;

# find function names, address ranges from *.func_table.src
# function information is stored in hash structure with function NAMEs as keys
my $hybrid_func_addr = {};
my $hybrid_func_size = {};
open SRC, $ARGV[2] or die $_;
while (<SRC>) {
	# lines in func_table.src: <00800e18 g     F .text	00000028 legup_sequential_accel_function>
	chop;
	my @tmp = split(/\s+/);
	if ( scalar(@tmp) != 6 ) {
		print "Warning: Only ".scalar(@tmp)." columns.\n\tFunction Table is expected to have 6 columns per line.\n\tEliminating $_\n";
		next;
	}
	$hybrid_func_addr->{$tmp[5]} = hex($tmp[0]);
	$hybrid_func_size->{$tmp[5]} = hex($tmp[4]);
}
close SRC;

exists $hybrid_func_addr->{"legup_sequential_$acel_func_name"} or die "Error: Hyprid Wrapper legup_sequential_$acel_func_name not found.\n";

# generate instruction trace
open SRC, $ARGV[3] or die $!;
open DST, ">".$ARGV[4] or die $!;

my $curr_func_addr = hex("80030000");  # starting address of current function

my $skip_branch_slot = 0;
my @func_stack = (0); # stack of starting address of functions
my $depth = 0;  # ($depth > 0 and $branch_slot==0) means current function is accelerated in hybrid

while (<SRC>) {
	chop;
	my $l = $_;
	next if ( $l !~ /^ffffffff/);
	
	# current instruction is the branch slot after func call/retn
	if ($skip_branch_slot == 1) {
		$skip_branch_slot = 0;
		next;
	}
	
	# instruction addr
	my $addr = $_;
	$addr =~ s/^ffffffff//;
	$addr =~ s/: .*//;
	$addr = hex($addr);
	
	my $tmp = "";
	if ( $depth ==0 ) { # non-accelerated functions
		exists $gxemul_func_name->{$curr_func_addr} or die "Error: Unknow function address - $curr_func_addr\n";
		exists $hybrid_func_addr->{$gxemul_func_name->{$curr_func_addr}} or die "Error: Unknow function name - ".$gxemul_func_name->{$curr_func_addr}."\n";
		die "Function address is out of range.\n" if ( $addr - $curr_func_addr > $gxemul_func_size->{$curr_func_addr} - 4);
		die "Function address is out of range.\n" if ( $addr - $curr_func_addr > $hybrid_func_size->{$gxemul_func_name->{$curr_func_addr}} - 4);

		$tmp = sprintf ("%08x", $addr - $curr_func_addr + $hybrid_func_addr->{$gxemul_func_name->{$curr_func_addr}});
		print DST $tmp."\n";
	}
	
	if ($l =~ /\sjal\t[^\s]*\t\<.*\>/) {  # calling a function - "jal	0xffffffff8003007c	<foo>"
#		print "  "x scalar(@func_stack) ."On Call: ".$gxemul_func_name->{$curr_func_addr};	printf (" - %08x\n", $addr);

		$skip_branch_slot = 1; # skip next instruction - branch slot
		# print instr address of branch slot (pc+4)
		if ($depth ==0 ) {
			$tmp = sprintf ("%08x", $addr + 4 - $curr_func_addr + $hybrid_func_addr->{$gxemul_func_name->{$curr_func_addr}});
			print DST $tmp."\n";
		}

		# push starting address of current function onto stack
		push @func_stack, $curr_func_addr;
		
		# find next function address
		$curr_func_addr = $l;
		$curr_func_addr =~ s/.*\sjal\t0xffffffff//;
		$curr_func_addr =~ s/\t<.*//;
		$curr_func_addr = hex($curr_func_addr);

		$depth += 1 if ( ($gxemul_func_name->{$curr_func_addr} eq $acel_func_name) or ($depth > 0) );
		if ($depth == 1) { # Just entering accelerating function
			print DST "<Accelerator Started>\n";
			for (my $i = 0;	$i< $hybrid_func_size->{"legup_sequential_$acel_func_name"}; $i+=4) {
				$tmp = sprintf ("%08x", $hybrid_func_addr->{"legup_sequential_$acel_func_name"} + $i);
				print DST $tmp."\n";
			}
			print DST "<Accelerator Finished>\n";		
		}
	}
	elsif ($l =~ /jr\tra\t\<.*\>/ or $l =~ /jr\tra$/) {  # returning
#		print "  "x scalar(@func_stack) ."On Retn: ".$gxemul_func_name->{$curr_func_addr};	printf (" - %08x\n", $addr);
		$skip_branch_slot = 1; # skip next instruction - branch slot
		# print instr address of branch slot (pc+4)
		if ($depth ==0 ) {
			$tmp = sprintf ("%08x", $addr + 4 - $curr_func_addr + $hybrid_func_addr->{$gxemul_func_name->{$curr_func_addr}});
			print DST $tmp."\n";
		}

		# pop starting address of ascendent function from stack
		$curr_func_addr = pop @func_stack;

		$depth -= 1 if ($depth > 0);
	}
}
close SRC;
close DST;

die "Error: The program is not properly traced - Function Stack is not empty - ".scalar(@func_stack).".\n" if scalar(@func_stack) != 0;
die "Error: The program is not properly traced - Accelerating Function Depth is not 0 - $depth.\n" if $depth != 0;

