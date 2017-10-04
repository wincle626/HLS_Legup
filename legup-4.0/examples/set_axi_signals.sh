#!/bin/bash

#
# Author:	Bain Syrowik
# Email:	bain.syrowik@mail.utoronto.ca
# Date:		June 1, 2015
# File:		set_axi_signals.sh
#

#
# This script sets the AMBA AXI sideband signals in legup_system.v so that ACP
# accesses are coherent.
# It should be run immediately after qsys-generate, and should only be run once.
#
# Usage:
# ./set_axi_signals.sh <file>
# where <file> is the top-level verilog file for the QSys system
#

#
# Notes:
#
# See page 9-30 of the Altera Cyclone V TRM:
#
# * Match the ARCACHE attributes to the configured MMU page table attributes.
# * ACP ARCACHE[1] must be set to 0x1
# * ACP ARUSER[0] must be set to 0x1
#
# * Match the AWCACHE attributes to the configured MMU page table attributes.
# * ACP AWCACHE[1] must be set to 0x1
# * ACP AWUSER[0] must be set to 0x1
#
# Page 9-34 of the Altera Cyclone V TRM states:
# "For coherent, cacheable reads or writes, the user field of the vid*rd and
# vid*wr registers must be set to 5'b11111 (coherent write back, write allocate
# inner cache attribute). This configuration ensures that the inner cache policy
# matches the policy used for cacheable data written by the processor."
# So we will set AxUSER to 5'h1f.
#
# As stated in section A4.4 of the AMBA AXI and ACE Protocol Specification,
# AxCACHE should be set to 4'hf for write-back, read- and write-allocate memory,
# which is how the ARM MMU is currently set up in the LegUp ARM startup code.
#
# Section A4.7 of the AMBA AXI and ACE Protocol Specification gives the
# following table:
# AxPROT	Value	Function
# [0]		0		Unprivileged access
#			1		Privileged access
# [1]		0		Secure access
#			1		Non-secure access
# [2]		0		Data access
#			1		Instruction access
#
# Accordingly, AxPROT is set to 3'h1 for privileged, secure, data accesses.
#


# Ensure the input file is passed as an argument
if [[ $# -ne 1 ]] ; then
	echo "$0 $@"
	echo "Incorrect usage."
	echo "Expecting: $0 <file>"
	echo "Where <file> is the top-level verilog file for the QSys system."
	echo "For example:"
	echo "$0 \$(OUTPUT_PATH)/legup_system/synthesis/legup_system.v"
	exit 1
fi

# Ensure input file exists
if [ ! -e $1 ] ; then
	echo "Error: file '$1' does not exist."
	exit 1
fi

# Do replacements in input file
# We want to replace this:
# .f2h_AWCACHE (mm_interconnect_3_arm_a9_hps_f2h_axi_slave_awcache), // .awcache
# with this:
# .f2h_AWCACHE (4'hf), // .awcache
#
# Match 3 patterns:
#	f2h_A[RW]CACHE.*(	the first part of the string, to the opening parenthesis
#	.*					the text inside the parentheses
#	)					the last part of the string from the closing parenthesis
# The first and last search pattern are inside parentheses, allowing them to be
# refernced using \1 and \2, respectively, in the replacement pattern.
sed -i "s/\(f2h_A[RW]CACHE.*(\).*\()\)/\14\'hf\2/" $1
sed -i "s/\(f2h_A[RW]PROT.*(\).*\()\)/\13\'h1\2/" $1
sed -i "s/\(f2h_A[RW]USER.*(\).*\()\)/\15\'h1f\2/" $1

