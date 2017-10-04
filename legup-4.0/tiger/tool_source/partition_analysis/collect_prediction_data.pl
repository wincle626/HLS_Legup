# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0]) or die "Expect program name as argument 0.";
(defined $ARGV[1]) or die "Expect accelerator name as argument 1.";
(defined $ARGV[2]) or die "Expect the filename of detail report as argument 2.\n";

my $acel_func_name = $ARGV[1];

(-e $ARGV[1].".acel_cycle.rpt" )  or die "$ARGV[1].acel_cycle.rpt not found.\n";
(-e $ARGV[1].".data_cache.rpt" )  or die "$ARGV[1].data_cache.rpt not found.\n";
(-e $ARGV[1].".data_store.rpt" )  or die "$ARGV[1].data_store.rpt not found.\n";
(-e $ARGV[1].".instr_cache.rpt")  or die "$ARGV[1].instr_cache.rpt not found.\n";
(-e $ARGV[1].".num_params.rpt" )  or die "$ARGV[1].num_params.rpt not found.\n";

# Coefficient in prediction equartion
my $cp_run = 2;             # addition cycles per run
my $cp_argument = 2;        # addition cycles per argument
my $cp_dload = 31;          # addition cycles per dcache miss
my $cp_dstore = 1.85;       # addition cycles per data store
my $cp_iload = 13;          # addition cycles per icache miss

my $tmp = "";
my $cmd = "";

# parse *.acel_cycle.rpt
	my $HW_states = "";
	my $Runs = "";
	$cmd = "grep '^ *$acel_func_name \| ' ".$ARGV[1].".acel_cycle.rpt\n";
	$tmp = `$cmd`;
	chomp ($tmp);

	$HW_states = $tmp;
	$HW_states =~ s/^ *$acel_func_name \| *//;
	$HW_states =~ s/ .*//;

	$Runs = $tmp;
	$Runs =~ s/ *\|$//;
	$Runs =~ s/.* //;

# parse *.data_cache.rpt
	my $d_hits   = "";
	my $d_misses = "";
	$cmd = "grep '^Accelerator - Cache Accesses: ' ".$ARGV[1].".data_cache.rpt\n";
	$tmp = `$cmd`;
	chomp ($tmp);

	$d_hits = $tmp;
	$d_hits =~ s/^Accelerator - Cache Accesses: *//;
	$d_hits =~ s/ .*//;

	$d_misses = $tmp;
	$d_misses =~ s/.* HITS, //;
	$d_misses =~ s/ .*//;
	
# parse *.data_store.rpt
	my $d_stores = "";
	$cmd = "grep ' stores to global memory space.' ".$ARGV[1].".data_store.rpt\n";
	$tmp =`$cmd`;
	chomp ($tmp);
	$d_stores = $tmp;
	$d_stores =~ s/ .*//;
	
# parse *.instr_cache.rpt
	my $i_hits   = "";
	my $i_misses = "";
	$cmd = "grep '^Accelerator - Cache Accesses: ' ".$ARGV[1].".instr_cache.rpt\n";
	$tmp = `$cmd`;
	chomp ($tmp);

	$i_hits = $tmp;
	$i_hits =~ s/^Accelerator - Cache Accesses: *//;
	$i_hits =~ s/ .*//;

	$i_misses = $tmp;
	$i_misses =~ s/.* HITS, //;
	$i_misses =~ s/ .*//;
	
# parse *.num_params.rpt
	my $num_params = "";
	$cmd = "grep '^".$ARGV[1].": ' ".$ARGV[1].".num_params.rpt\n";
	$tmp =`$cmd`;
	chomp ($tmp);
	$num_params = $tmp;
	$num_params =~ s/.*: //;
	$num_params =~ s/ .*//;


# Calculate predicted cycle
my $predicted_cycle = int($HW_states) + $cp_run*int($Runs) + int($d_hits) + $cp_dload*int($d_misses) + $cp_dstore*int($d_stores) + int($i_hits) + $cp_iload*($i_misses) + $cp_argument*int($Runs)*int($num_params);
$predicted_cycle = int($predicted_cycle);
my $pc_to_print = sprintf ("   %8d   |", $predicted_cycle);

# parse SW cycle if sw profiling report exists
my $sw_cycle = "";
my $sc_to_print = "";
if (-e $ARGV[0].".profiling.rpt") {
	my $rpt = $ARGV[0].".profiling.rpt";
	$sw_cycle = `grep $acel_func_name $rpt`;
	chomp $sw_cycle;
	my @tmp_data = split(/\s*\|\s*/, $sw_cycle);
	$sw_cycle = $tmp_data[4];
	$sc_to_print = sprintf (" %8d ", int($sw_cycle) );
}
elsif (-e $ARGV[0].".sim.profiling.rpt") {
	my $rpt = $ARGV[0].".sim.profiling.rpt";
	my $sw_cycle = `grep $acel_func_name $rpt`;
	chomp $sw_cycle;
	$sw_cycle =~ s/.*, Count *//;
	$sw_cycle =~ s/, Percentage.*//;
	$sw_cycle = int($sw_cycle);
	$sc_to_print = sprintf (" %8d ", int($sw_cycle) );
}
else {
	$sc_to_print = "   n\\a    ";
}

# print prediction details to spread sheet
my $nl;
if (-e $ARGV[2]) {
	$cmd = "cat ".$ARGV[2]." \| wc -l \n";
	$nl = `$cmd`;
	chomp ($nl);
	$nl = int($nl) + 1;
	
	open DST, ">>".$ARGV[2] or die $!;
}
else {
	$nl = 2;
	open DST, ">".$ARGV[2] or die $!;
	print DST "Program;Function;HW states;Runs;SW/HW transitions;d hits;d misses;d load cycle;d stores; d store cycle;i hits;i misses;i cycle;# of arguments;argument overhead;predict cycle;sw cycle;\n";
}

print DST $ARGV[0].";";			#A
print DST $ARGV[1].";";			#B
print DST $HW_states.";";		#C
print DST $Runs.";";			#D
print DST "=$cp_run*D$nl;";		#E	2*Runs
print DST $d_hits.";";			#F
print DST $d_misses.";";		#G
print DST "=F$nl+$cp_dload*G$nl;";	#H	data_hits + 31*data_misses
print DST $d_stores.";";		#I
print DST "=$cp_dstore*I$nl;";		#J	1.85 * data_stores
print DST $i_hits.";";			#K
print DST $i_misses.";";		#L
print DST "=K$nl+$cp_iload*L$nl;";	#M	instr_hits + 13*instr_misses
print DST $num_params.";";		#N
print DST "=$cp_argument*N$nl*D$nl;";	#O	2* num_params* Runs
print DST "=ROUND(C$nl+E$nl+H$nl+J$nl+M$nl+O$nl);";	#P
print DST $sw_cycle.";\n";		#Q

close DST;

# print predicted hybrid cycle to $(NAME).hybrid_prediction.rpt
if (-e $ARGV[0].".hybrid_prediction.rpt") {
	open DST, ">>".$ARGV[0].".hybrid_prediction.rpt" or die $!;
}
else {
	open DST,  ">".$ARGV[0].".hybrid_prediction.rpt" or die $!;
	print DST '-'x25 ."+".'-'x14 ."+".'-'x11 ."\n";
	print DST ' 'x16 ."Function | Hybrid Cycle | SW cycle \n";
	print DST '-'x25 ."+".'-'x14 ."+".'-'x11 ."\n";
}

print DST ' 'x(25-1-length($ARGV[1])) . $ARGV[1]. " |".$pc_to_print.$sc_to_print."\n";

close DST;
