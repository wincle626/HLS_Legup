#!/usr/bin/perl

use warnings;

# open input command list
my $num_args = $#ARGV + 1;
if ($num_args != 1) {
  print "Missing input command file. Usage: pcie_tb.pl command_list\n";
  exit;
}
open COMMANDS, "<$ARGV[0]" or die $!;
my @commands = <COMMANDS>;

# assume the programmer has defined the function to accelerate already
system("make cleanpcie; make pcieConfig; make pcieSWAuto; make hybridHwOnlyPass; make pcieHWloweringLinking");

# open the generated qsys tcl file and modify it to determine number of threads
open QSYS, "<legup_qsys_pcie_generated.tcl" or die "cannot open > legup_qsys_pcie_generated.tcl";
my @qsys_file = <QSYS>;
close(QSYS);

my $accel_name;
if ($commands[0] =~ m/function\s*=\s*(\w+)/)
{
    $accel_name = $1;
}
else
{
    print "No function name specified in command list, Exiting!\n";
    exit;
}

my $num_threads;
my $accel_offset;
my $extracted_accel_name;
my $found_accel_name = 0;

#print "accel_name from command list is $accel_name\n";
foreach $line (@qsys_file)
{
    if ($line =~ m/set num_threads (\d+)/)
    {
        $num_threads = $1;
    }
    elsif ($line =~ m/add_instance $accel_name\_\$i/)
    {
        $found_accel_name = 1;
    }
    elsif ($line =~ m/\$baseAddress \+ (\d+)/)
    {
        $accel_offset = $1;
        last;
    }
}
if ($found_accel_name == 0)
{
    # check that the programmer correctly picked the same function in config.tcl 
    print "Accel name in command list does not match accel name in qsys file. Exiting!\n";
    exit;
}
#print "num_threads: $num_threads, accel_name: $accel_name, accel_offset: $accel_offset\n";

# remove the function definition from the internal array representation
splice(@commands, 0, 1);

#generate testbench file according to number of threads and input commands
open TB_MASTER, ">tb_master.v" or die $!;

print TB_MASTER "module tb_master (\n";
print TB_MASTER "\toutput reg clk,\n";
print TB_MASTER "\toutput reg reset,\n";

print TB_MASTER "\t//mem\n";
print TB_MASTER "\toutput reg [31:0] avm_mem_address,\n";
print TB_MASTER "\toutput reg avm_mem_read,\n";
print TB_MASTER "\toutput reg avm_mem_write,\n";
print TB_MASTER "\toutput reg [63:0] avm_mem_writedata,\n";
print TB_MASTER "\tinput [63:0] avm_mem_readdata,\n\n";

for(my $i = 0; $i < $num_threads; $i++)
{
    print TB_MASTER "\t//accel $i\n";
    print TB_MASTER "\toutput reg [31:0] avm_a$i\_address,\n";
    print TB_MASTER "\toutput reg avm_a$i\_read,\n";
    print TB_MASTER "\toutput reg avm_a$i\_write,\n";
    print TB_MASTER "\toutput reg [31:0] avm_a$i\_writedata,\n";
    print TB_MASTER "\tinput [31:0] avm_a$i\_readdata";
    if ($i != $num_threads - 1) {
        print TB_MASTER ",";
    }
    print TB_MASTER "\n\n";
}

print TB_MASTER ");\n\n";

print TB_MASTER "initial\n";
print TB_MASTER "    clk <= 0;\n";
print TB_MASTER "always @ (clk)\n";
print TB_MASTER "    clk <= #10 ~clk;\n\n";

print TB_MASTER "initial begin\n";
print TB_MASTER "@(negedge clk);\n";
print TB_MASTER "reset <= 1;\n";
print TB_MASTER "@(negedge clk);\n";
print TB_MASTER "reset <= 0;\n";
print TB_MASTER "// command list begin\n\n";

# TODO: populate the testbench with the input commands
foreach $command (@commands)
{
    chop ($command);
    if ($command =~ m/#(.*)/)
    {
        # insert the comments into the testbench
        print TB_MASTER "// $1\n";
    }
    # memory read command (length is given in 64-bit WORDS, not bytes)
    elsif ($command =~ m/read 0x([0-9a-fA-F]+) (\d+)/)
    {
        # read from a specific address
        my $address = hex "$1";
        my $length = $2; # in bytes
        if ($length < 0)
        {
            print "Come on, you need to specify a longer length ;)\n";
            exit;
        }
        print TB_MASTER "// $command\n";
        if ($address >= 0x40000000) 
        {
            # read from memory
            for ($i = 0; $i < $length; $i++) {
                #length of 8 because of 64 bit memory
                my $new_addr = $address + $i * 8;
                print TB_MASTER "\t@(negedge clk);\n";
                printf TB_MASTER "\tavm_mem_address <= 32'h%x;\n", $new_addr;
                print TB_MASTER "\tavm_mem_read <= 1;\n";
                print TB_MASTER "\t@(negedge clk)\n";
                print TB_MASTER "\t\$display (\"avm_mem_readdata: %x\", avm_mem_readdata);\n";
            }
            print TB_MASTER "\tavm_mem_read <= 0;\n";
        }
        else
        {
            print "Invalid address specified for read (must read on-chip memory, which is at 0x40000000!\n";
            exit;
        }
    }
    # memory write command. address always given in hex, values can be either.
    elsif ($command =~ m/write 0x([0-9a-fA-F]+) (.*)/)
    {
        # read from a specific address
        my $address = hex "$1";
        my $data = $2;
        if ($data =~ m/0x([[:xdigit:]]+)/)
        {
            $data = $1;
        }
        print TB_MASTER "// $command\n";
        if ($address >= 0x40000000) 
        {
            print TB_MASTER "\t@(negedge clk);\n";
            printf TB_MASTER "\tavm_mem_address <= 32'h%x;\n", $address;
            printf TB_MASTER "\tavm_mem_writedata <= 64'h%s;\n", $data;
            print TB_MASTER "\tavm_mem_write <= 1;\n";
            print TB_MASTER "\t@(negedge clk)\n";
            print TB_MASTER "\tavm_mem_write <= 0;\n";
        }
        else
        {
            print "Invalid address specified for write (must write to on-chip memory, which is at 0x40000000!\n";
            exit;
        }
    }
    # accelerator write command format: accel_name accel_number argument
    # this command implicitly starts the accelerator
    # user must split 64 bit arguments across two write commands
    elsif ($command =~ m/$accel_name (\d+) (.*)/) 
    {
        my $accel_number = $1;
        #check to see if this is a valid accel number
        if ($accel_number >= $num_threads || $accel_number < 0)
        {
            print "Invalid accelerator ID specified. Must be between 0 and $num_threads\n";
            exit;
        }
        my $argument = $2;
        my @args = split(' ', $argument);
        foreach $arg (@args)
        {
            #check to see if they are written in hex. if they are, convert to decimal.
            if ($arg =~ m/0x([[:xdigit:]]+)/)
            {
                $arg = hex("$1");
            }
            #print "$arg\n";
        }
        my $num_args = @args;
        #print "num_args = $num_args\n";
        my $i = 0;
        foreach $arg (@args)
        {
            my $address = (3 + $i) * 4 + $accel_offset * $accel_number;
            print TB_MASTER "// accel $accel_number argument $i: $arg\n";
            print TB_MASTER "\t@(negedge clk);\n";
            printf TB_MASTER "\tavm_a$accel_number\_address <= 32'h%x;\n", $address;
            print TB_MASTER "\tavm_a$accel_number\_write <= 1;\n";
            printf TB_MASTER "\tavm_a$accel_number\_writedata <= 32'h%x;\n", $arg;
            $i++;
        }
        print TB_MASTER "// accel $accel_number START\n";
        print TB_MASTER "\t@(negedge clk);\n";
        printf TB_MASTER "\tavm_a$accel_number\_address <= 32'h%x;\n", 2 * 4 + $accel_offset * $accel_number;
        print TB_MASTER "\tavm_a$accel_number\_write <= 1;\n";
        printf TB_MASTER "\tavm_a$accel_number\_writedata <= 32'h%x;\n", 1;
        print TB_MASTER "\t@(negedge clk);\n";
        print TB_MASTER "\tavm_a$accel_number\_write <= 0;\n";
    }
    elsif ($command =~ m/poll_complete (\d+)/)
    {
        #poll accel_number done bit 
        my $accel_number = $1;
        print TB_MASTER "// accel $accel_number POLL DONE BIT\n";
        print TB_MASTER "\t@(negedge clk);\n";
        printf TB_MASTER "\tavm_a$accel_number\_address <= 32'h%x;\n", 2 * 4 + $accel_offset * $accel_number;
        print TB_MASTER "\tavm_a$accel_number\_read <= 1;\n";
        print TB_MASTER "\t@(negedge clk);\n";
        print TB_MASTER "\twhile (avm_a$accel_number\_readdata[0] == 0)\n";
        print TB_MASTER "\t\t@(negedge clk);\n";
        print TB_MASTER "\tavm_a$accel_number\_read <= 0;\n";
    }
    else
    {
        print "Unrecognized command!\n";
        exit;
    }
}
print TB_MASTER "// command list end\n\n";
print TB_MASTER "\$finish;\n";
print TB_MASTER "end\n";

print TB_MASTER "endmodule\n";

close TB_MASTER;

#generate qsys hw tcl file
open TCL, ">tb_master_hw.tcl" or die $!;

print TCL "#TCL file generated by pcie_tb.pl\n\n";
print TCL "package require -exact qsys 12.0\n\n";
print TCL "# module tb_master\n\n";
print TCL "set_module_property NAME tb_master\n";
print TCL "set_module_property VERSION 1.0\n";
print TCL "set_module_property INTERNAL false\n";
print TCL "set_module_property OPAQUE_ADDRESS_MAP true\n";
print TCL "set_module_property DISPLAY_NAME tb_master\n";
print TCL "set_module_property INSTANTIATE_IN_SYSTEM_MODULE true\n";
print TCL "set_module_property EDITABLE true\n";
print TCL "set_module_property ANALYZE_HDL AUTO\n";
print TCL "set_module_property REPORT_TO_TALKBACK false\n";
print TCL "set_module_property ALLOW_GREYBOX_GENERATION false\n";
print TCL "\n";
print TCL "\n";
print TCL "# \n";
print TCL "# file sets\n";
print TCL "# \n";
print TCL "add_fileset QUARTUS_SYNTH QUARTUS_SYNTH \"\" \"\"\n";
print TCL "set_fileset_property QUARTUS_SYNTH TOP_LEVEL tb_master\n";
print TCL "set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false\n";
print TCL "add_fileset_file tb_master.v VERILOG PATH tb_master.v\n";
print TCL "# \n";
print TCL "# connection point reset_source\n";
print TCL "# \n";
print TCL "add_interface reset_source reset start\n";
print TCL "set_interface_property reset_source associatedClock clock_source\n";
print TCL "set_interface_property reset_source associatedDirectReset \"\"\n";
print TCL "set_interface_property reset_source associatedResetSinks \"\"\n";
print TCL "set_interface_property reset_source synchronousEdges DEASSERT\n";
print TCL "set_interface_property reset_source ENABLED true\n";
print TCL "\n";
print TCL "add_interface_port reset_source reset reset Output 1\n";
print TCL "\n";
print TCL "\n";
print TCL "# \n";
print TCL "# connection point clock_source\n";
print TCL "# \n";
print TCL "add_interface clock_source clock start\n";
print TCL "set_interface_property clock_source associatedDirectClock \"\"\n";
print TCL "set_interface_property clock_source clockRate 0\n";
print TCL "set_interface_property clock_source clockRateKnown false\n";
print TCL "set_interface_property clock_source ENABLED true\n";
print TCL "\n";
print TCL "add_interface_port clock_source clk clk Output 1\n";
print TCL "# \n";
print TCL "# connection point mem\n";
print TCL "# \n";
print TCL "add_interface mem avalon start\n";
print TCL "set_interface_property mem addressUnits SYMBOLS\n";
print TCL "set_interface_property mem associatedClock clock_source\n";
print TCL "set_interface_property mem associatedReset reset_source\n";
print TCL "set_interface_property mem bitsPerSymbol 8\n";
print TCL "set_interface_property mem burstOnBurstBoundariesOnly false\n";
print TCL "set_interface_property mem burstcountUnits WORDS\n";
print TCL "set_interface_property mem doStreamReads false\n";
print TCL "set_interface_property mem doStreamWrites false\n";
print TCL "set_interface_property mem holdTime 0\n";
print TCL "set_interface_property mem linewrapBursts false\n";
print TCL "set_interface_property mem maximumPendingReadTransactions 0\n";
print TCL "set_interface_property mem readLatency 0\n";
print TCL "set_interface_property mem readWaitTime 0\n";
print TCL "set_interface_property mem setupTime 0\n";
print TCL "set_interface_property mem timingUnits Cycles\n";
print TCL "set_interface_property mem writeWaitTime 0\n";
print TCL "set_interface_property mem ENABLED true\n";
print TCL "\n";
print TCL "add_interface_port mem avm_mem_address address Output 32\n";
print TCL "add_interface_port mem avm_mem_read read Output 1\n";
print TCL "add_interface_port mem avm_mem_write write Output 1\n";
print TCL "add_interface_port mem avm_mem_writedata writedata Output 64\n";
print TCL "add_interface_port mem avm_mem_readdata readdata Input 64\n";
for ($i = 0; $i < $num_threads; $i++)
{
    print TCL "# \n";
    print TCL "# connection point a$i\n";
    print TCL "# \n";
    print TCL "add_interface a$i\ avalon start\n";
    print TCL "set_interface_property a$i\ addressUnits SYMBOLS\n";
    print TCL "set_interface_property a$i\ associatedClock clock_source\n";
    print TCL "set_interface_property a$i\ associatedReset reset_source\n";
    print TCL "set_interface_property a$i\ bitsPerSymbol 8\n";
    print TCL "set_interface_property a$i\ burstOnBurstBoundariesOnly false\n";
    print TCL "set_interface_property a$i\ burstcountUnits WORDS\n";
    print TCL "set_interface_property a$i\ doStreamReads false\n";
    print TCL "set_interface_property a$i\ doStreamWrites false\n";
    print TCL "set_interface_property a$i\ holdTime 0\n";
    print TCL "set_interface_property a$i\ linewrapBursts false\n";
    print TCL "set_interface_property a$i\ maximumPendingReadTransactions 0\n";
    print TCL "set_interface_property a$i\ readLatency 0\n";
    print TCL "set_interface_property a$i\ readWaitTime 1\n";
    print TCL "set_interface_property a$i\ setupTime 0\n";
    print TCL "set_interface_property a$i\ timingUnits Cycles\n";
    print TCL "set_interface_property a$i\ writeWaitTime 0\n";
    print TCL "set_interface_property a$i\ ENABLED true\n";
    print TCL "\n";
    print TCL "add_interface_port a$i\ avm_a$i\_address address Output 32\n";
    print TCL "add_interface_port a$i\ avm_a$i\_read read Output 1\n";
    print TCL "add_interface_port a$i\ avm_a$i\_write write Output 1\n";
    print TCL "add_interface_port a$i\ avm_a$i\_writedata writedata Output 32\n";
    print TCL "add_interface_port a$i\ avm_a$i\_readdata readdata Input 32\n";
}

close TCL;

#remove the generated legup tcl file
system("rm legup_qsys_pcie_generated.tcl");

#create custom simulation legup tcl file
open QSYS, ">legup_qsys_pcie_generated.tcl" or die $!;

print QSYS "load_system pcie_tutorial/qsys_system.qsys\n";
print QSYS "remove_instance pcie_ip\n";
print QSYS "remove_instance sgdma\n";
print QSYS "add_instance onchip_memory altera_avalon_onchip_memory2\n";
print QSYS "add_instance tb_master tb_master\n";
print QSYS "set_instance_parameter_value onchip_memory dualPort \"true\"\n";
print QSYS "set_instance_parameter_value onchip_memory dataWidth \"64\" \n";
print QSYS "set_instance_parameter_value onchip_memory memorySize \"786432\"\n";
print QSYS "add_connection tb_master.mem onchip_memory.s1\n";
print QSYS "set_connection_parameter_value tb_master.mem/onchip_memory.s1 baseAddress \"0x40000000\"\n";
print QSYS "add_connection tb_master.clock_source onchip_memory.clk1\n";
print QSYS "add_connection tb_master.clock_source onchip_memory.clk2\n";
print QSYS "add_connection tb_master.reset_source onchip_memory.reset1 \n";
print QSYS "add_connection tb_master.reset_source onchip_memory.reset2 \n";
print QSYS "\n";
print QSYS "set num_threads $num_threads\n";
print QSYS "set baseAddress 0x0\n";
print QSYS "\n";
#loop portion
print QSYS "for {set i 0} {\$i < \$num_threads} {incr i} {\n";
print QSYS "\tadd_instance $accel_name\_\$i $accel_name\n";
print QSYS "\tadd_instance accel_mem_bridge_\$i accel_to_mem_bridge\n";
print QSYS "\t# Connect clock to accelerators\n";
print QSYS "\tadd_connection tb_master.clock_source $accel_name\_\$i.clockreset\n";
print QSYS "\tadd_connection tb_master.reset_source $accel_name\_\$i.clockreset_reset\n";
print QSYS "\t# Connect clock to bridge\n";
print QSYS "\tadd_connection tb_master.clock_source accel_mem_bridge_\$i.clock\n";
print QSYS "\tadd_connection tb_master.reset_source accel_mem_bridge_\$i.reset\n";
print QSYS "\t# Connect PCIe to each accelerator\n";
print QSYS "\tadd_connection tb_master.a\$i $accel_name\_\$i.s1\n";
print QSYS "\tset_connection_parameter_value tb_master.a\$i/$accel_name\_\$i.s1 baseAddress \"[expr \$baseAddress + 64*\$i]\"\n";
print QSYS "\t# Connect each accelerator to the bridge\n";
print QSYS "\tadd_connection $accel_name\_\$i.ACCEL accel_mem_bridge_\$i.accel\n";
print QSYS "\tset_connection_parameter_value $accel_name\_\$i.ACCEL/accel_mem_bridge_\$i.accel baseAddress \"0x0\"\n";
print QSYS "\t# Connect the bridge to mem\n";
print QSYS "\tadd_connection accel_mem_bridge_\$i.mem onchip_memory.s2\n";
print QSYS "\tset_connection_parameter_value accel_mem_bridge_\$i.mem/onchip_memory.s2 baseAddress \"0x0\"\n";
print QSYS "}\n";
print QSYS "save_system\n";

close QSYS;

#stitch together everything in the qsys file
system("make pcieVerilogbackendAuto");

#generate the qsys system!
system("mkdir simulation_tb; ip-generate --project-directory=pcie_tutorial --output-directory=simulation_tb --file-set=QUARTUS_SYNTH --component-file=pcie_tutorial/qsys_system.qsys --system-info=DEVICE_FAMILY=\"Stratix IV\"");
