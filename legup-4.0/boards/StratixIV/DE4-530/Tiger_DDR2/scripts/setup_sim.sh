#!/bin/bash

sed -i "s/MEM_VERBOSE\(\s*\)(1)/MEM_VERBOSE\1(0)/" legup_system/testbench/legup_system_tb/simulation/legup_system_tb.v
sed -i "s/MEM_INIT_EN\(\s*\)(0)/MEM_INIT_EN\1(1)/" legup_system/testbench/legup_system_tb/simulation/legup_system_tb.v
sed -i "s/MEM_INIT_FILE\(\s*\)(\"\")/MEM_INIT_FILE\1(\"legup_ddr2_memory_init.hex\")/" legup_system/testbench/legup_system_tb/simulation/legup_system_tb.v
cp legup_system/testbench/legup_system_tb/simulation/submodules/*.hex legup_system/testbench/legup_system_tb/simulation/.

