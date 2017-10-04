#!/usr/bin/perl
use warnings;
use strict;
use POSIX qw/ceil/;

# This script generates Verilog for and synthesizes patterns of size 1-3 with 
# multiplexing on the inputs, writing area and fmax data to an output.dat file.
#
# Patterns are of this form:
#
#            Inputs
#            \ / \ /
#   "OP1"-->  O   O  <--"OP2"
#              \ /
#               O  <--"OP3"
#               | 
#            Output
#
#
# The 3 operations, OP1 OP2 and OP3, are specified as command line args
# and are in the order shown above. "no_op" is used if an operation is
# not present (see example below). Operation names are similar to
# those used in "hwtest.pl" (see below in comments)
#
# Command line arguments:   ./Sharing.pl StratixIV muxwidth OP1 OP2 OP3 bitwidth
#
# e.g.:
#
#            ./Sharing.pl StratixIV 2 signed_add no_op bitwise_OR 16
#
# corresponds to:
#
#            \ / 
#             + 
#              \ /
#              OR
#               | 
#
# with 2-to-1 multiplexing on all 3 inputs.
#
##=======-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

my $devicefam = "CycloneII"; # Cyclone II  or Stratix IV
my $muxwidth = 2; # default 2
my $Vfile = "";

# Below are a list of all LLVM instructions:

#"signed_add",
#"signed_sub",
#"signed_mul",    # LLVM implements signed multiplication

#"signed_icmpeq"  # "signed" is used for consistency with hwtest (but unneeded)
#"signed_icmpne",

#"bitwise_OR",                      
#"bitwise_AND",                      
#"bitwise_XOR",                                                

#"signed_divide", # division, modulus and comparisons (except equal) can be signed or unsigned
#"unsigned_divide",                    
#"signed_modulus",                    
#"unsigned_rem",
#"unsigned_icmplt",                    
#"signed_icmplt",                    
#"unsigned_icmpgt",                    
#"signed_icmpgt",                    
#"unsigned_icmple",                    
#"signed_icmple",                    
#"unsigned_icmpge",                    
#"signed_icmpge",                    
#"shift_ll",      # Shift left, equivalent to LLVM shl                
#"shift_rl",      # Logical shift right, >>, pad with 0s, equivalent to LLVM lshr
#"shift_ra",      # Arithmetic logical right, >>> (signed), pad with 1s or 0s, equivalent to LLVM ashr                                                        

# Checking if device argument provided
if ($ARGV[0] eq "StratixIV") {
  $devicefam = $ARGV[0];
}
my $device = "AUTO"; # Can change to target device

# Checking is muxwidth is provided
if ($ARGV[1] ne "") {
  $muxwidth  = $ARGV[1];
}

my $count;
my $totalinputs = 0;
my $OP1 = $ARGV[2];
my $OP2 = $ARGV[3];
my $OP3 = $ARGV[4];

my $operator1 = "";
my $operator2 = "";
my $operator3 = "";

# Determine total number of inputs, it is either 2, 4, 6, or 8
if ($OP1 eq "no_op") {
  $totalinputs = $totalinputs + $muxwidth; # 2 (mux) inputs on this side
} else {
  $totalinputs = $totalinputs + $muxwidth + $muxwidth; # just 1 input on this side
}
if ($OP2 eq "no_op") {
  $totalinputs = $totalinputs + $muxwidth; # 2 (mux) inputs on this side
} else {
  $totalinputs = $totalinputs + $muxwidth + $muxwidth; # just 1 input on this side
}
my $nummuxes = $totalinputs / $muxwidth;

# Determine the operation width
my $width = $ARGV[5];

mkdir ("$devicefam");
chdir ("$devicefam"); 
open (Fout, ">>output.dat");
my $date = qx/date/;
print Fout "Data collected on $date",
     "Device Family: $devicefam\n",
     "Device: $device\n";
close (Fout);
chdir ("..");

my $quartuspath = qx/which quartus/;
$quartuspath =~ s/bin\/\/quartus//g;
$quartuspath =~ s/bin\/quartus//g;
chomp($quartuspath);

$Vfile = "${OP1}_${OP2}_${OP3}_${muxwidth}to1mux_${width}bit"; # File name characterized by inputs, type, and muxwidth
system ("mkdir -p $devicefam/$Vfile");
chdir ("$devicefam/$Vfile");

my @params1 = split(/_/, $OP1); # split operation 1 into its individual words 
my @params2 = split(/_/, $OP2); # split operation 2 into its individual words 
my @params3 = split(/_/, $OP3); # split operation 3 into its individual words 

my $unsigned = 0;
if ( ($params1[0] eq "unsigned") || ($params2[0] eq "unsigned") || ($params3[0] eq "unsigned") ) {
  $unsigned = 1;
}

# Creates project setup file
open (Fout, ">setup_proj.tcl"); # tcl file needed by quartus
print Fout "project_new $Vfile -overwrite\n",
     "set_global_assignment -name FAMILY $devicefam\n",
      "set_global_assignment -name SDC_FILE $Vfile.sdc\n",
     "set_global_assignment -name DEVICE $device\n",
     "set_global_assignment -name TOP_LEVEL_ENTITY $Vfile\n",
     "set_global_assignment -name SOURCE_FILE $Vfile.v\n";
print Fout "set_global_assignment -name NUMBER_OF_PATHS_TO_REPORT 100000\n",
     "create_base_clock -fmax 1500MHz -target clk clk\n",
     "project_close";
close (Fout);

# Creates Hardware Component's parametrized verilog file
open (Fout, ">$Vfile.v");

# Setting operator based on operation type
if ($params1[1] eq "add") {
  $operator1 = "+";
} elsif ($params1[1] eq "sub") {
  $operator1 = "-";
} elsif ($params1[1] eq "mul") {
  $operator1 = "*";
} elsif ($params1[1] eq "div") {
  $operator1 = "/";
} elsif ($params1[1] eq "rem") {
  $operator1 = "%";
} elsif ($params1[1] eq "AND") {
  $operator1 = "&";
} elsif ($params1[1] eq "OR") {
  $operator1 = "|";
} elsif ($params1[1] eq "XOR") {
  $operator1 = "^";
} elsif ($params1[1] eq "ll") {
  $operator1 = "<<";
} elsif ($params1[1] eq "rl") {
  $operator1 = ">>";
} elsif ($params1[1] eq "ra") {
  $operator1 = ">>>";
} elsif ($params1[1] eq "icmplt") {
  $operator1 = "<";
} elsif ($params1[1] eq "icmpgt") {
  $operator1 = ">";
} elsif ($params1[1] eq "icmpeq") {
  $operator1 = "==";
} elsif ($params1[1] eq "icmpne") {
  $operator1 = "!=";
} elsif ($params1[1] eq "icmple") {
  $operator1 = "<=";
} elsif ($params1[1] eq "icmpge") {
  $operator1 = ">=";
}

if ($params2[1] eq "add") {
  $operator2 = "+";
} elsif ($params2[1] eq "sub") {
  $operator2 = "-";
} elsif ($params2[1] eq "mul") {
  $operator2 = "*";
} elsif ($params2[1] eq "div") {
  $operator2 = "/";
} elsif ($params2[1] eq "rem") {
  $operator2 = "%";
} elsif ($params2[1] eq "AND") {
  $operator2 = "&";
} elsif ($params2[1] eq "OR") {
  $operator2 = "|";
} elsif ($params2[1] eq "XOR") {
  $operator2 = "^";
} elsif ($params2[1] eq "ll") {
  $operator2 = "<<";
} elsif ($params2[1] eq "rl") {
  $operator2 = ">>";
} elsif ($params2[1] eq "ra") {
  $operator2 = ">>>";
} elsif ($params2[1] eq "icmplt") {
  $operator2 = "<";
} elsif ($params2[1] eq "icmpgt") {
  $operator2 = ">";
} elsif ($params2[1] eq "icmpeq") {
  $operator2 = "==";
} elsif ($params2[1] eq "icmpne") {
  $operator2 = "!=";
} elsif ($params2[1] eq "icmple") {
  $operator2 = "<=";
} elsif ($params2[1] eq "icmpge") {
  $operator2 = ">=";
}

if ($params3[1] eq "add") {
  $operator3 = "+";
} elsif ($params3[1] eq "sub") {
  $operator3 = "-";
} elsif ($params3[1] eq "mul") {
  $operator3 = "*";
} elsif ($params3[1] eq "div") {
  $operator3 = "/";
} elsif ($params3[1] eq "rem") {
  $operator3 = "%";
} elsif ($params3[1] eq "AND") {
  $operator3 = "&";
} elsif ($params3[1] eq "OR") {
  $operator3 = "|";
} elsif ($params3[1] eq "XOR") {
  $operator3 = "^";
} elsif ($params3[1] eq "ll") {
  $operator3 = "<<";
} elsif ($params3[1] eq "rl") {
  $operator3 = ">>";
} elsif ($params3[1] eq "ra") {
  $operator3 = ">>>";
} elsif ($params3[1] eq "icmplt") {
  $operator3 = "<";
} elsif ($params3[1] eq "icmpgt") {
  $operator3 = ">";
} elsif ($params3[1] eq "icmpeq") {
  $operator3 = "==";
} elsif ($params3[1] eq "icmpne") {
  $operator3 = "!=";
} elsif ($params3[1] eq "icmple") {
  $operator3 = "<=";
} elsif ($params3[1] eq "icmpge") {
  $operator3 = ">=";
}

# Start generating Verilog

# Naming module and setting width parameter  
print Fout "module $Vfile\n",
     "#(parameter WIDTH=$width)\n",
     "(\n";
# Inputs   
if ($unsigned == 1) {    
  for ($count = 1; $count <= $totalinputs; $count++) {
    print Fout "  input [WIDTH-1:0] data$count\,\n";
  }
} else {
  for ($count = 1; $count <= $totalinputs; $count++) {
    print Fout "  input signed [WIDTH-1:0] data$count\,\n";
  }
}
# mux select input
if ($muxwidth != 1) {
  my $selwidth = ceil(log($muxwidth)/log(2));
  my $selindex = $selwidth - 1;
  if ($selindex == 0) {
    print Fout "  input select,\n";
  } else {
    print Fout "  input [$selindex:0] select,\n";
  }
}
print Fout "  input clk,\n";
# Outputs
if($operator3 eq "*") { 
  print Fout "  output reg [2*WIDTH-1:0] dataout\n";
} elsif ( ($operator3 eq "==") || ($operator3 eq "!=") || ($operator3 eq ">=") || ($operator3 eq "<=") ||
      ($operator3 eq ">") || ($operator3 eq "<")  ) { # If final operation is a compare
  print Fout "  output reg dataout\n";
} else {
  print Fout "  output reg [WIDTH-1:0] dataout\n";
}
print Fout ");\n";
# Reg
if ($unsigned == 1) {    
  for ($count = 1; $count <= $totalinputs; $count++) { 
    print Fout "  reg [WIDTH-1:0] data$count\_reg;\n";
  }
} else {
  for ($count = 1; $count <= $totalinputs; $count++) { 
    print Fout "  reg signed [WIDTH-1:0] data$count\_reg;\n";
  }
}
# Now, we have to add the wires. 
# The number is equal to the number of muxes. 
if ($muxwidth != 1) {
  if ($unsigned == 1) {    
    for ($count = 1; $count <= $nummuxes; $count++) { 
      print Fout "  reg [WIDTH-1:0] w$count\;\n";
    }
  } else {
    for ($count = 1; $count <= $nummuxes; $count++) { 
      print Fout "  reg signed [WIDTH-1:0] w$count\;\n";
    }
  }
}
# always @ posedge (clk)
print Fout "\n  always @ (posedge clk)\n",
     "  begin\n";
for ($count = 1; $count <= $totalinputs; $count++) { 
  print Fout "    data$count\_reg <= data$count\;\n";
}
if ($muxwidth != 1) {
  if ( ($operator1 ne "") && ($operator2 ne "") ) {
    print Fout "\n    dataout <= (w1 $operator1\ w2) $operator3\ (w3 $operator2\ w4);\n";
  } elsif ($operator1 ne "") {
    print Fout "\n    dataout <= (w1 $operator1\ w2) $operator3\ w3;\n";
  } elsif ($operator2 ne "") {
    print Fout "\n    dataout <= w1 $operator3\ (w2 $operator2\ w3);\n";
  } else {
    print Fout "\n    dataout <= w1 $operator3\ w2;\n";
  }
} else {
  if ( ($operator1 ne "") && ($operator2 ne "") ) {
    print Fout "\n    dataout <= (data1_reg $operator1\ data2_reg) $operator3\ (data3_reg $operator2\ data4_reg);\n";
  } elsif ($operator1 ne "") {
    print Fout "\n    dataout <= (data1_reg $operator1\ data2_reg) $operator3\ data3_reg;\n";
  } elsif ($operator2 ne "") {
    print Fout "\n    dataout <= data1_reg $operator3\ (data2_reg $operator2\ data3_reg);\n";
  } else {
    print Fout "\n    dataout <= data1_reg $operator3\ data2_reg;\n";
  }
}
print Fout "  end\n\n";

# always @ (*)
if ($muxwidth != 1) {
  print Fout "  always @ (*)\n",
       "  begin\n";

  my $count2;
  my $temp_index;
  for ($count = 0; $count < $muxwidth; $count++) {
    if ($count == 0) {
      print Fout "    if (select==${count})\n";
    } elsif ($count != ($muxwidth-1)) {
      print Fout "    else if (select==${count})\n";
    } else {
      print Fout "    else\n";
    }
    print Fout "    begin\n";
    for ($count2 = 1; $count2 <= $nummuxes; $count2++) {  # For each mux
      $temp_index = $count2 + ($nummuxes * $count);
      print Fout "      w$count2\ <= data$temp_index\_reg;\n";
    }
    print Fout "    end\n"; 
  }
  print Fout "  end\n\n";
}
print Fout "endmodule\n\n";
close (Fout);

# Done generating verilog, now compile in Quartus

open (Fout, ">$Vfile.sdc"); # overwrite
print Fout "create_clock -period 1 -name clk [get_ports clk]\n";
close (Fout);

# Executes commands to create/compile project
my @commands = ("quartus_sh -t setup_proj.tcl $Vfile.v",
    "quartus_sh --flow compile $Vfile",
    "quartus_sta $Vfile"
    );
foreach my $command (@commands) {
  system ($command);
}


# Gathers required data and writes it to "output.dat"
my $line = qx/grep -A6 "Fmax Summary" $Vfile.sta.rpt/;
my $Fmax = 0;
my $FmaxRestriction = 0;
if ($line =~ /([\.\d]+) MHz[\s]+;[\s]+([\.\d]+)/) {
  $Fmax = $1;
  $FmaxRestriction = $2;
} else {
  $line = qx/grep "Fmax is" $Vfile.tan.rpt/;
  if($line =~ /([\.\d]+) MHz/) {
    $Fmax = $FmaxRestriction = $1;
  }
}

$line = qx/grep "Combinational ALUTs" $Vfile.map.rpt/;
my $ALUTs = 0;
my $LEs = 0;
if ($line =~ /([,\d]+)/) {
  $ALUTs = $1;
  $ALUTs =~ s/,//g;
} else {
  $line = qx/grep "Total logic elements" $Vfile.map.summary/;
  if ($line =~ /:\s([,\d]+)/) {
    $LEs = $1;
    $LEs =~ s/,//g;
  } 
}
$line = qx/grep "DSP" $Vfile.map.summary/;
my $DSPblocks = 0;
if ($line =~ /:\s([,\d]+)/) {
  $DSPblocks = $1;
  $DSPblocks =~ s/,//g;
} else {
  $line = qx/grep "Embedded Multiplier" $Vfile.map.summary/;
  if ($line =~ /:\s([,\d]+)/) {
    $DSPblocks = $1;
    $DSPblocks =~ s/,//g;
  }
}
$line = qx/grep "Total registers" $Vfile.map.rpt/;
my $Regs = 0;
if ($line =~ /([,\d]+)/) {
  $Regs = $1;
  $Regs =~ s/,//g;
}
$line = qx/grep "Total combinational functions" $Vfile.map.rpt/;
my $LUTs = 0;
if ($line =~ /([,\d]+)/) {
  $LUTs = $1;
  $LUTs =~ s/,//g;
}

chdir ("..");
open (Fout, ">>output.dat");
print Fout "Hardware Unit: $Vfile \n",
           "Fmax: $Fmax MHz\n",
           "Fmax Restriction: $FmaxRestriction MHz\n",
           "Logic Elements: $LEs\n",
           "Combinational Functions: $LUTs\n",
           "ALUTs: $ALUTs\n",
           "DSP elements: $DSPblocks\n",
           "Total Registers: $Regs\n\n";
close (Fout);

