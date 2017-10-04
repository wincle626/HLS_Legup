# Lanny: June-29-2012
# This script is used to generate detailed instruction profile for a program
# This script take trace file from gxemul as input - make detail_instr_profiling
#!/usr/bin/perl
use warnings;
use strict;
use integer;

my $debug_msg = 0;
my $tmp = "";
my $ins = "";
my $group;
my @isa = ();
my %isa_group = (
	"add_g"    => ["add", "addi", "addu", "addiu"],
	"and_g"    => ["and", "andi"],
	"div_g"    => ["div", "divu"],
	"mult_g"   => ["mult", "multu"],
	"or_g"     => ["nor", "or", "ori"],
	"shift_g"  => ["sll", "sllv", "sra", "srav", "srl", "srlv"],
	"sub_g"    => ["sub", "subu"],
	"xor_g"    => ["xor", "xori"],
	"lui_g"    => ["lui"],
	"slt_g"    => ["slt", "slti", "sltu", "sltiu"],
	"branch_g" => ["b", "bczt", "bczf", "beq", "bgez", "bgezal", "bgtz", "blez", "bgezal", "bltzal", "bltz", "bne"],
	"jump_g"   => ["j", "jal", "jalr", "jr"],
	"load_g"   => ["lb", "lbu", "lh", "lhu", "lw", "lwcz", "lwl", "lwr"],
	"store_g"  => ["sb", "sh", "sw", "swcz", "swl", "swr"],
	"miscel_g" => ["mfhi", "mflo", "mthi", "mtlo", "mfcz"],
	"nop_g"    => ["nop"]
);

foreach $group (sort keys %isa_group) {
	foreach $ins (@{ $isa_group{$group} } ) {
		push @isa, $ins;
	}
}

defined $ARGV[0] or die "Error: expect name of trace file from gxemul as Argument 0\n";
defined $ARGV[1] or die "Error: expect name of src file as Argument 1\n";
defined $ARGV[2] or die "Error: expect name of output profile report as Argument 2\n";

# Parse function name and address from *.emul.src
my @names = ();
my @addrs = ();	# addr is used to represent function in following profiling of trace file
my $longest_func_name = 0;
open SRC, $ARGV[1] or die $!;
while (<SRC>)	{
	chomp ($_);
	if ($_ =~ />:/) {
		my $name = $_;
		my $addr = $_;
		$name =~ s/:.*//;
		$name =~ s/.*</</;
		$addr =~ s/ .*//;
		push @names, $name;
		push @addrs, $addr;
		
		$longest_func_name = ($longest_func_name <= length($name) ) ? length($name) : $longest_func_name;
	}
	elsif ($_ =~ "Disassembly of section" and $_ !~ "Disassembly of section .text") {
		last;
	}
}
close (SRC);

my $line = "";
my @info = ();
my $known_ins = 0;
my $func = "";
my %storage = ();

my @func_stack = ();
my $sp = 0;
my @cntr_stack = ();
my $curr_func = "";
my $curr_cntr = {};
my $popd_func = "";
my $popd_cntr = {};

# initialization
$curr_func = "should_never_get_stored";
foreach $ins (@isa) {
	foreach $func (@addrs) {
		$storage {$func}{$ins} = 0;
	}
	$curr_cntr->{$ins} = 0;
	$popd_cntr->{$ins} = 0;
}
# cntr_stack should be initialized, push and pop won't work since it only pushs hashref onto array.
# actual data copy (deep copy) is required to perform push/pop a hash onto/from array
# let's say maximum stack size is 32
for ($sp =0; $sp <32; $sp+=1) {
	$tmp = {};
	foreach $ins (@isa) {
		$tmp->{$ins} = 0;
	}
	push @cntr_stack, $tmp;
}
$sp = 0;

my $delay = 1;	# used to wait for an actual function jump after seeing a function jump instruction
my $jtype = 0;	# jtype represents the last function jumping type: 0-call and 1-retn

open TRACE, $ARGV[0] or die $!;
while (<TRACE>)
{
	chop;
	if ($_ !~ /^ffffffff/) {
		next;
	}
	
	$line = $_;
	$line =~ s/^ffffffff//;
	$line =~ s/://;
	$line =~ s/\s*\(d\)\s*/\t/;
	@info = split /\s+/, $line;
	# $info[1] - address
	# $info[2] - op code
	# $info[3] - register name if ($info[2] eq "jr") ; i.e., jr ra

	# Check if the instruction is known.
	$known_ins = 1;
	unless (grep {$_ eq $info[2]} @isa) {
		print "Warning: unknown instruction - $info[2] at adddress - $info[0]\n";
		$known_ins = 0;
	}
	
	# Let's start profiling
	
	##########################
	## Handle call and retn ##
	##########################
	$delay += 1; #resets when (jr ra) | jal | jalr is detected
	if ($delay == 2) {	# this means current line contains the first instruction of the target function of the previous function jump
		if ($jtype == 0) {
			# on call, push func and curr_cntr+popd_cntr onto stack
			push @func_stack, $curr_func;
			if ($debug_msg) {	print "c-".$curr_cntr->{"add"}." p-".$popd_cntr->{"add"}."\n";	}
			foreach $ins (@isa) { # for ascendent
				$curr_cntr->{$ins} += $popd_cntr->{$ins};
			}
			$sp += 1; # push @cntr_stack, $curr_cntr;
			foreach $ins (@isa) {	# deep copy to perform push of a hash
				$cntr_stack[$sp]->{$ins} = $curr_cntr -> {$ins};
			}
			if ($debug_msg) {	print "push-".$curr_cntr->{"add"}."\n";	}
			# Check if the function address is known.
			$known_ins = 1;
			(grep {$_ eq $info[0]} @addrs) or die "Error: unknow function address - $info[0]\n";
			$curr_func = $info[0];
			foreach $ins (@isa) { # for ascendent
				$curr_cntr->{$ins} = 0;	#resets current cntr
				$popd_cntr->{$ins} = 0;	#popd_cntr should be 0 for a newly called function
			}			
		}
		elsif ($jtype == 1) {
			# on retn, add curr_cntr and popd_cntr to storage
			# pop popd_func and popd_cntr from stack for ASCENDENT
			if ($debug_msg) {	print "s-".$storage{$curr_func}{"add"}." p-".$popd_cntr->{"add"}." c-".$curr_cntr->{"add"}."\n";	}
			foreach $ins (@isa) {
				$storage {$curr_func}{$ins} += $popd_cntr->{$ins} + $curr_cntr->{$ins};
			}
			if ($debug_msg) {	print "store-".$storage {$curr_func}{"add"}."\n\n";	}
			# Check if the function address is known.
			$popd_func = pop @func_stack;
			foreach $ins (@isa) { # for ascendent
				$popd_cntr->{$ins} += $curr_cntr->{$ins} + $cntr_stack[$sp]->{$ins};;
				$curr_cntr->{$ins} = 0;	#resets current cntr
			}
			if ($debug_msg) {	print "new_pop-".$popd_cntr->{"add"}."\n";	}

			$sp -= 1; #$popd_cntr = pop @cntr_stack;
			if ($debug_msg) {	print "pop-".$popd_cntr->{"add"}."\n";	}
			(grep {$_ eq $popd_func} @addrs) or die "Error: unknown function address - $popd_func\n";
			$curr_func = $popd_func;
		}
		else {
			die "Error: really weird~~~\n";
		}
	}

	if ($known_ins) {	
#		$curr_cntr->{"add"} += 1;  # uncomment this and comment the next line to debug
		$curr_cntr->{$info[2]} += 1;
	}
		
	if ($info[2] eq "jal" or $info[2] eq "jalr") {
		$delay = 0;
		$jtype = 0;
	}
	elsif ($info[2] eq "jr" and $info[3] eq "ra") {
		$delay = 0;
		$jtype = 1;
	}
}
close TRACE;

# $delay stays 1 at end of TRACE, the last return (end of main) needs an additional store

if ($debug_msg) {	print "s-".$storage{$curr_func}{"add"}." p-".$popd_cntr->{"add"}." c-".$curr_cntr->{"add"}."\n";	}
foreach $ins (@isa) {
	$storage {$curr_func}{$ins} += $popd_cntr->{$ins} + $curr_cntr->{$ins};
}
$popd_func = pop @func_stack;
$sp -= 1; #$popd_cntr = pop @cntr_stack;
($popd_func eq "should_never_get_stored" and $sp==0) or die "Error: something wrong with the stack!!!\n";

# output with format~~~
open OUT, ">$ARGV[2]";
$tmp = 0;
while ($longest_func_name > $tmp) {
	print OUT " ";
	$tmp += 1;
}
print OUT " |";

foreach $group (sort keys %isa_group) {
	$tmp = 0;
	while (8 >= $tmp+length($group) ) {
		print OUT " ";
		$tmp += 1;
	}
	print OUT $group." |";
}
print OUT "\n";

#foreach $ins (@isa) {
#	$tmp = 0;
#	while (8 >= $tmp+length($ins) ) {
#		print OUT " ";
#		$tmp += 1;
#	}
#	print OUT $ins." |";
#}
#print OUT "\n";


my @formatted_names=();
foreach my $name (@names) {
	while ($longest_func_name > length($name)) {
		$name = " ".$name;
	}
	push @formatted_names, $name;
}

my $i;
my $group_cnt;
for ($i=0; $i<scalar(@addrs); $i+=1) {
	print OUT $formatted_names[$i]." |";
	foreach $group (sort keys %isa_group) {
		$group_cnt = 0;
		foreach $ins (@{ $isa_group{$group} } ) {
			$group_cnt += $storage{$addrs[$i]}{$ins};
		}
		$tmp = sprintf (" %8d |", $group_cnt);
		print OUT $tmp;
	}
	print OUT "\n";
}
#for ($i=0; $i<scalar(@addrs); $i+=1) {
#	print OUT $formatted_names[$i]." |";
#	foreach $ins (@isa ) {
#		$tmp = sprintf (" %8d |", $storage{$addrs[$i]}{$ins});
#		print OUT $tmp;
#	}
#	print OUT "\n";
#}
close OUT;

