# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

my $debug = 0;
###########################################################
# parameters
my $request_router_latency = 3;
my $request_injection_latency = 3;
my $request_taken_off_latency = 2;
my $response_router_latency = 4;
my $response_injection_latency = 3;
my $response_taken_off_latency = 2;
my $local_mem_process_latency = 2;
my $localize_const_gv = 1;

(defined $ARGV[0]) or die "Expect the *.emul.src file as argument 0.\n";
(defined $ARGV[1]) or die "Expect the *.sym_table.src file (symbol table dumped by readelf) as argument 1.\n";
(defined $ARGV[2]) or die "Expect the filename of gxemul trace (*.raw.trace) as argument 2.\n";

my @ld_ops = ("lb", "lbu", "lh", "lhu", "lw", "lwcz", "lwl", "lwr");
my @sw_ops = ("sb", "sh", "sw", "swcz", "swl", "swr");

###########################################################
# find function names and addresses from *.emul.src
# the function name can be wrong from gxemul's output, but address must be right
my $f_addr_to_name = {};
print "Functions and its text addresses:\n" if $debug >= 1;
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
		print "\tAddress: ".$faddr."\tFunction Name: ".$fname."\n" if $debug >= 1;
	}
	elsif ($_ =~ "Disassembly of section" and $_ !~ "Disassembly of section .text") {
		last;
	}
}
close SRC;

###########################################################
# parse global variable SECTIONS from gv_table.src
# the 3 hashes below are using starting address as key
my $gv_name = {};	#name
my $gv_size = {};	#size
my $gv_const = {};	#whether the gv is constant
print "Global variables and read only info:\n" if $debug >= 1;
open SRC, $ARGV[1] or die $_;
while (<SRC>) {
	# table format:
	#   [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
	#   [ 2] .rodata           PROGBITS        8003461c 00561c 00579c 00   A  0   0  4
	chop;
	$_ =~ s/^\s*\[.*\]\s*//;
	my @tmp = split(/\s+/);
	next if scalar(@tmp) != 10;
	my $saddr = 0;
	if ($tmp[0] =~ /\.rodata/ or
		$tmp[0] =~ /\.data/ or
		$tmp[0] =~ /\.bss/ or
		$tmp[0] =~ /\.sbss/ ) {
		$saddr = hex($tmp[2]);
		$gv_name->{$saddr} = $tmp[0];
		$gv_size->{$saddr} = hex($tmp[4]);
		$gv_const->{$saddr} = ($tmp[0]=~/\.rodata/) ? 1 : 0;
	} else {
		next;
	}
	# debug info
	print "\tAddress: ".sprintf("%08x", $saddr)
		."\tSize: ".sprintf("%04x", $gv_size->{$saddr})
		."\tRead-Only: ".$gv_const->{$saddr}
		."\tVar Name: ".$gv_name->{$saddr}
		."\n" if $debug >= 1;
}
close SRC;

###########################################################
# extracting

# parsing variables
my $l = "";
my $init_sp = hex("a0007f00");
my $sp = $init_sp;
my $op = "";
my $addr = 0;
my $depth = 0;
my $func_depth = 1;
my $cycle = 0;
my $gv = 1;	# 1 means gv
my $ld = 1; # 0 means sw

# stacks
my @sp_stack = ();	# to keep track of the upper bounds of functions' stack
push @sp_stack, $init_sp;	# the upper bound of main's stack is the initial sp
my @func_stack = ();
push @func_stack, "main";
# stats
my @stack_ld_each_depth = (0);
my @stack_sw_each_depth = (0);
my @gv_ld_each_depth = (0);
my @gv_sw_each_depth = (0);
my $stack_ld_cycles = 0;
my $stack_sw_cycles = 0;
my $gv_ld_cycles = 0;
my $gv_sw_cycles = 0;
my $max_func_depth = $func_depth;

open SRC, $ARGV[2] or die $!;
while (<SRC>) {
	chop;
	$l = $_;
	$l =~ s/[^\t]*\t//;  # remove pc and instruction
	
	if ($l =~ /\[.*\]/) {    # memory access, store or load
		$op = $l;
		$op =~ s/\s.*//;
		$addr = $l;
		$addr =~ s/.*\[0xffffffff//;
		$addr =~ s/\].*//;
		$addr =~ s/\s.*//;
		$addr = hex($addr);
		#####################################
		# check the scope of accessing memory address
		################
		if ($addr >= $sp and $addr < $init_sp) { # variables on stack
			$gv =0;
			$depth = 0;
			foreach my $residing_space (reverse @sp_stack) {
				last if ($addr < $residing_space);
				$depth = $depth + 1;
			}
		} else { # global variable
			$gv = 1;
			my $gv_saddr = -1;
			# search for the global variable to know if it is read only
			foreach my $saddr (keys $gv_name) {
				if ($addr >= $saddr and $addr < ($saddr + $gv_size->{$saddr}) ) {
					$gv_saddr = $saddr;
					last;
				}
			}
			die "Error: No global variable found that matches the accessing address: ".sprintf("%08x\n",$addr) if $gv_saddr == -1;
			if ($gv_const->{$gv_saddr} == 1 && $localize_const_gv == 1) { # read only global variable
				$depth = 0;
			} else { # main function will have depth=1 to access global variable
				$depth = scalar(@sp_stack);
				die "Error: Non-read-only global variable cannot be accessed with depth being 0.\n" if $depth == 0;
			}
		}
		#####################################
		# check the memory access type
		################
		if (grep {$_ eq $op} @ld_ops) {    # load operations
			$ld = 1;
		} elsif (grep {$_ eq $op} @sw_ops) {    # store operations
			$ld = 0;
		} else {
			print "\nWarning: unknown operation accessing memory: ".$op."\n";
		}
		#####################################
		# Calculate cycle latency
		################
		$cycle = $local_mem_process_latency; # the minimum latency when residing in local ram
		if ($depth >= 1) { # in the parent node
			# inject and remove request
			$cycle += $request_injection_latency + $request_taken_off_latency;
			# inject and remove response if it's a load operation
			$cycle += ($response_injection_latency + $response_taken_off_latency) if $ld == 1;
			# the latency spent in propagating thru intermediate routers
			$cycle += ($depth-1) * ( # when $depth == 1, meaning variable resides in direct parent, no extra propagation
				$request_router_latency + (($ld==1)?$response_router_latency:0)
				);
		}
		#####################################
		# update stats
		################
		print "Func:".$func_stack[-1]." " if ($debug >= 2);
		if ($ld == 1) { # ld access
			print " ld @ addr:".sprintf("0x%08x", $addr) if ($debug >= 2);
			print " depth:".sprintf("%2d", $depth) if ($debug >= 2);
			print " latency:",sprintf("%3d", $cycle) if ($debug >= 2);
			if ($gv == 1) { # global variable
				$gv_ld_each_depth[$depth] += 1;
				$gv_ld_cycles += $cycle;
				if ($depth == 0) {
					print " LOCAL-RO-GV" if ($debug >= 2);
				} else {
					print " GV" if ($debug >= 2);
				}
			} else { # stack variable
				$stack_ld_each_depth[$depth] += 1;
				$stack_ld_cycles += $cycle;
				if ($depth == 0) {
					print " LOCAL-STACK" if ($debug >= 2);
				} else {
					print " PARENT-STACK" if ($debug >= 2);
				}
			}
		} elsif ($ld == 0) {
			print " sw @ addr:".sprintf("0x%08x", $addr) if ($debug >= 2);
			print " depth:".sprintf("%2d", $depth) if ($debug >= 2);
			print " latency:",sprintf("%3d", $cycle) if ($debug >= 2);
			if ($gv == 1) { # global variable
				$gv_sw_each_depth[$depth] += 1;
				$gv_sw_cycles += $cycle;
				die "\nError: writing to read-only global variable.\n" if $depth==0;
				print " GV" if ($debug >= 2);
			} else { # stack variable
				$stack_sw_each_depth[$depth] += 1;
				$stack_sw_cycles += $cycle;
				if ($depth == 0) {
					print " LOCAL-STACK" if ($debug >= 2);
				} else {
					print " PARENT-STACK" if ($debug >= 2);
				}
			}
		} else {
			die "Error: \$ld must be either 0 or 1.\n";
		}
		print "\n" if ($debug >= 2);
	}
	elsif ($l =~ /jal\t[^\s]*\t\<.*\>/) {  # calling a function - "jal	0xffffffff8003007c	<eginfo>"
		$l =~ s/jal\t0xffffffff//;
		$l =~ s/\t<.*//; 
		print "<".$func_stack[-1]." calling ".$f_addr_to_name->{$l}.", current sp->".sprintf("%08x", $sp)."\n" if $debug >= 1;
		# depth increases
		$func_depth = $func_depth + 1;
		# update max_func_depth if necessary
		if ($func_depth > $max_func_depth) {
			$max_func_depth - $func_depth == -1 or die "Error: function call depth cannot increment by more than 1 at a time.\n";
			$max_func_depth = $func_depth;
			push @stack_ld_each_depth, 0;
			push @stack_sw_each_depth, 0;
			push @gv_ld_each_depth, 0;
			push @gv_sw_each_depth, 0;
			scalar(@stack_ld_each_depth) == $max_func_depth or die "Error: size of stats array does not match with max. function call depth.\n";
		}
		# push function 
		push @sp_stack, $sp;
		push @func_stack, $f_addr_to_name->{$l};
	}
	elsif ($l =~ /jr\tra\t\<.*\>/ or $l =~ /jr\tra$/) {  # returning
		if ($func_stack[-1] eq "main") {
			print "Main returns, Program execution finishes...\n" if $debug == 1;
			next;
		}
		print "<".$func_stack[-1]." returning to ".$func_stack[-2]." >\n" if $debug >= 1;
		pop @sp_stack;
		pop @func_stack;
		$func_depth = $func_depth - 1;
	}
	elsif ($l =~ /addiu\tsp,sp,/) {  # sp is changing
		$l =~ s/addiu\tsp,sp,//;
		$sp = $sp+int($l);
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

($sp eq $init_sp) or die "Error: sp is not properly traced - sp = $sp at end of execution\n";

###########################################################
# print result
print "################# Statistics ###################\n";
print "Depth".(" "x6);
for (my $i=0; $i<$max_func_depth; $i++) {
	printf ("%6d", $i);
}
print "     Count     Latency\n";

my $stack_ld_cnt = 0;
my $stack_sw_cnt = 0;
my $gv_ld_cnt = 0;
my $gv_sw_cnt = 0;
print "Stack ld".(" "x3);
for (my $i=0; $i<$max_func_depth; $i++) {
	$stack_ld_cnt += $stack_ld_each_depth[$i];
	printf ("%6d", $stack_ld_each_depth[$i]);
}
printf ("%10d%12d\n", $stack_ld_cnt, $stack_ld_cycles);

print "Stack sw".(" "x3);
for (my $i=0; $i<$max_func_depth; $i++) {
	$stack_sw_cnt += $stack_sw_each_depth[$i];
	printf ("%6d", $stack_sw_each_depth[$i]);
}
printf ("%10d%12d\n", $stack_sw_cnt, $stack_sw_cycles);

print "GV ld".(" "x6);
for (my $i=0; $i<$max_func_depth; $i++) {
	$gv_ld_cnt += $gv_ld_each_depth[$i];
	printf ("%6d", $gv_ld_each_depth[$i]);
}
printf ("%10d%12d\n", $gv_ld_cnt, $gv_ld_cycles);

print "GV sw".(" "x6);
for (my $i=0; $i<$max_func_depth; $i++) {
	$gv_sw_cnt += $gv_sw_each_depth[$i];
	printf ("%6d", $gv_sw_each_depth[$i]);
}
printf ("%10d%12d\n", $gv_sw_cnt, $gv_sw_cycles);

print "################################################\n";

print "Number of stack access: ".($stack_ld_cnt + $stack_sw_cnt)."\n";
print "Number of gv access: ".($gv_ld_cnt + $gv_sw_cnt)."\n";
print "Total memory access: ".($stack_ld_cnt + $stack_sw_cnt + $gv_ld_cnt + $gv_sw_cnt)."\n";
print "Total latency of ld: ".($stack_ld_cycles + $gv_ld_cycles)."\n";
print "Total latency of sw: ".($stack_sw_cycles + $gv_sw_cycles)."\n";
print "Total latency to access stack: ".($stack_ld_cycles + $stack_sw_cycles)."\n";
print "Total latency to access gv   : ".($gv_ld_cycles + $gv_sw_cycles)."\n";
print "Total latency: ".($stack_ld_cycles + $stack_sw_cycles + $gv_ld_cycles + $gv_sw_cycles)."\n";

print "################################################\n";
