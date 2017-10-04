#!/usr/bin/python


################################################################################
#
# Look at profiling data (jtag_uart.txt file) and use the data to choose a fn
# to accelerate.  Create a 'set_accelerator_function' line in config.tcl so
# that the function will be accelerated when you recompile with 'make hybrid'.
#
################################################################################


################################################################################
# imports
################################################################################

import os
import sys
from collections import namedtuple


################################################################################
# Data
################################################################################

# a dictionary that correlates event numbers to human-readable event names
event_dict = {
	0x00 : "Software increment",
	0x01 : "Instruction cache miss",
	0x02 : "Instruction micro TLB miss",
	0x03 : "Data cache miss",
	0x04 : "Data cache access",
	0x05 : "Data micro TLB miss",
	0x06 : "Data read",
	0x07 : "Data write",
	0x09 : "Exception taken",
	0x0A : "Exception return",
	0x0B : "Write context ID",
	0x0C : "Software change of the PC",
	0x0D : "Immediate branch",
	0x0F : "Unaligned load or store",
	0x10 : "Branch mispredicted or not predicted",
	0x11 : "Cycle count",
	0x12 : "Predictable branches",
	0x40 : "Java bytecode execute",
	0x41 : "Software Java bytecode executed",
	0x42 : "Jazelle backward branches executed",
	0x50 : "Coherent linefill miss",
	0x51 : "Coherent linefill hit",
	0x60 : "Instruction cache dependent stall cycles",
	0x61 : "Data cache dependent stall cycles",
	0x62 : "Main TLB miss stall cycles",
	0x63 : "STREX passed",
	0x64 : "STREX failed",
	0x65 : "Data eviction",
	0x66 : "Issue does not dispatch any instruction",
	0x67 : "Issue is empty",
	0x68 : "Instructions coming out of the core renaming stage",
	0x69 : "Number of data linefills",
	0x6A : "Number of prefetcher linefills",
	0x6B : "Number of hits in prefetched cache lines",
	0x6E : "Predictable function returns",
	0x70 : "Main execution unit instructions",
	0x71 : "Second execution unit instructions",
	0x72 : "Load/Store Instructions",
	0x73 : "Floating-point instructions",
	0x74 : "NEON instructions",
	0x80 : "Processor stalls because of PLDs",
	0x81 : "Processor stalled because of a write to memory",
	0x82 : "Processor stalled because of instruction side main TLB miss",
	0x83 : "Processor stalled because of data side main TLB miss",
	0x84 : "Processor stalled because of instruction micro TLB miss",
	0x85 : "Processor stalled because of data micro TLB miss",
	0x86 : "Processor stalled because of DMB",
	0x8A : "Integer clock enabled",
	0x8B : "Data Engine clock enabled",
	0x8C : "NEON SIMD clock enabled",
	0x8D : "Instruction TLB allocation",
	0x8E : "Data TLB allocation",
	0x90 : "ISB instructions",
	0x91 : "DSB instructions",
	0x92 : "DMB instructions",
	0x93 : "External interrupts",
	0xA0 : "PLE cache line request completed",
	0xA1 : "PLE cache line request skipped",
	0xA2 : "PLE FIFO flush",
	0xA3 : "PLE request completed",
	0xA4 : "PLE FIFO overflow",
	0xA5 : "PLE request programmed"
}

################################################################################
# open files
################################################################################

config_file_path = os.getcwd() + "/config.tcl"
prof_file_path = os.getcwd() + "/" + sys.argv[1]

config  = open(config_file_path, 'r').readlines()
prof_file = open(prof_file_path, 'r').readlines()


################################################################################
# comment out all 'set_accelerator_function' lines in config
################################################################################

for ndx, line in enumerate(config):
	if line.startswith("set_accelerator_function"):
		config[ndx] = "#" + line



################################################################################
# read in the profiling data
################################################################################

print 

do_print = False

FuncVals = namedtuple("FuncVals", "index name parent calls self_c hier_c ev1 ev2 ev3 ev4 ev5 ev6")

func_list = []

for line in prof_file:
	if line.startswith("index\tfunction"):
		events = line.split()[-11::2]
		print "\033[32mEvents:\033[0m"
		for ev in events:
			print ev + "\t" + event_dict[int(ev, 0)]
		print
	if line.startswith("DONE"):
		break
	if do_print:
		words = line.split()
		vals = FuncVals._make(words)
		func_list.append(vals)
	if line.startswith("============================"):
		do_print = True


################################################################################
# print each function horizontally in a table
################################################################################

print "\033[32mProfiling Data:\033[0m"
# print header
for f in FuncVals._fields:
	print f + "\t",
print

# print each function
for f in func_list:
	for g in f:
		print g, "\t",
	print
print


################################################################################
# print each function vertically
################################################################################

""" for f in func_list:
	for field in FuncVals._fields:
		print field, "\t", getattr(f, field), "\n\t",
	print """


################################################################################
# populate funcs_to_accelerate 
################################################################################

# a list of fn's to accelerate
funcs_to_accelerate = []

# for now: naieve -> accelerate all fn's != main
for fn in func_list:
	if fn.name != "main":
		funcs_to_accelerate.append(str(fn.name))


################################################################################
# create the new config.tcl file
################################################################################

# add appropriate lines in conf_out
for fn in funcs_to_accelerate:
	new_line = "set_accelerator_function \"" + fn + "\"\n"
	for ndx, line in enumerate(config):
		if new_line in line:
			config[ndx] = new_line
			break
	else:
		config.append(new_line)
	 	continue

# write conf_out to file
config_file_out = open(config_file_path, 'w')
config_file_out.write("".join(config))


################################################################################
# display the contents of config.tcl
################################################################################

print "\033[32mconfig.tcl:\033[0m"
for line in config:
	print line,
print


################################################################################
# EOF
################################################################################

