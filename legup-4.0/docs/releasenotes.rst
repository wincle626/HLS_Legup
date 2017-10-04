.. _release:

Release Notes
===============

LegUp |version|
~~~~~~~~~~~~~~~~

Major new features
+++++++++++++++++++++++++++++++++++
 * Support for ARM processor in the hybrid flow (on the DE1-SoC board).
 * Generic Verilog in the pure hardware flow that can be targeted to ANY FPGA vendor (e.g. Xilinx) or even ASICs.  This is enabled through the use of generic dividers, RAM blocks and multipliers.  
 * Support for pthreads and OpenMP in the pure hardware flow.  A processor is no longer necessary in the system for the HLS of software threads.
 * Improved loop pipelining for loops with recurrences and resource constraints, as described in FPL 2014 paper (Canis et al.).
 * Local and grouped RAMs.  Pointer analysis is used to analyze when RAMs can be kept local to a Verilog module.  LegUp is also able to group multiple arrays into a single RAM in the hardware.
 * QSys compatibility so LegUp hybrid implementations can be compiled by the latest versions of Altera Quartus II.
 * LLVM 3.5: LegUp 4.0 is built within the most recent version of LLVM.
 * Device support: Altera Cyclone II, Cyclone IV, Cyclone V, Stratix IV, Stratix V.
 * Custom Verilog support which gives the user the ability to tell LegUp to NOT perform HLS for a given function, for which they intend to provide their own Verilog.

Minor new features
+++++++++++++++++++++++++++++++++++
 * Constraints manual explaining all TCL constraints and their function.
 * FSM implementation using case statement instead of if-else.

Beta features
+++++++++++++++++++++++++++++++++++
 * Bitwidth minimization, as described in ASP-DAC 2013 paper (Gort et al.).
 * Multi-cycling support, as described in DATE 2015 paper (Hadjis et al.).
 * HLS debugging support, as described in FPL 2014 paper (Calagar et al., Goeders et al.).
 * If-conversion that flattens the control-flow graph in certain cases, which may improve performance and enable loop pipelining opportunities.

Contributors to LegUp 4.0
+++++++++++++++++++++++++++++++++++
 * Andrew Canis
 * Jongsok (James) Choi
 * Blair Fort
 * Ruo Long (Lanny) Lian
 * Bain Syrowik
 * Nazanin Calagar
 * Jeffrey Goeders
 * Hsuan (Julie) Hsiao
 * Yu Ting (Joy) Chen
 * Mathew Hall
 * Stefan Hadjis
 * Marcel Gort
 * Tomasz Czajkowski
 * Stephen Brown
 * Jason Anderson

LegUp 3.0
~~~~~~~~~~~~~~~~

Major new features
+++++++++++++++++++++++++++++++++++
 * Loop pipelining for simple loops using iterative modulo scheduling
 * Floating point support
 * Multi-pumping of DSP-block multipliers tested on the DE4
 * Hardware Profiler of hybrid flow working and fully scripted
 * Multi-ported caches using LVT memories and multi-pumping memories
 * Support for dual-port memories
 * Support for resource constraints in SDC scheduler
 * New minimize bitwidth pass
 * Scheduling cycle prediction engine for hybrid and pure-hardware flow


Minor new features
+++++++++++++++++++++++++++++++++++
 * Improvements to the design of the internal memory controller
 * Alias analysis for load/store scheduling
 * Scripts to set multi-cycle timing constraints based on LegUp schedule
 * New benchmarks to test multi-pumped multipliers with loop unrolling
 * Tests for floating point operations
 * Allow "local" rams that don't use shared memory controller if no pointers alias
 * Support for independent  "local" rams to be accessed in parallel
 * Can set metadata in LLVM IR for loop pipelining
 * Multiplier resource sharing now only shares multipliers that infer DSPs
 * Can specify the multiplier pipeline stages
 * Support for C loop labels for loop pipelining
 * Support for DDR2 memory for the Tiger MIPS processor on the Altera DE4 board
 * Can use Edmonds matching algorithm to group multipliers into the same cycle
 * Latex gantt chart generated for schedule
 * Scripts to generate characterization for DSP-block multiplier
 * Script to find the path between two signals in LegUp generated Verilog
 * Code refactoring and removal of dead code

Beta features
+++++++++++++++++++++++++++++++++++
 * GUI for viewing scheduling report
 * PCIe support for DE4
 * Support for parallel accelerators using Pthreads and OpenMP

LegUp 2.0
~~~~~~~~~~~~~~~~

Major new features
+++++++++++++++++++++++++++++++++++
 * SDC scheduler (see [Cong06]_)
 * Pattern-based resource sharing (paper to appear in FPGA 2012)
 * Added an online demo version of LegUp to the website
 * LLVM version updated to 2.9
 * Compiler front-end updated to clang (llvm-gcc is deprecated)
 * Support for Stratix IV (DE4 board) with device characterization
 * Added: Polly, CLooG, isl. These libraries support polyhedral loop dependency analysis
 * New documentation with pdf version
 * Cache simulator for TigerMIPS
 * Memory access profiler for extracting parallel functions
 * Added bit width minimization analysis used for pattern sharing
 * Added live variable analysis pass used for binding/pattern sharing
 * Significant code refactoring, both for clarity, modifiability, and also removal of dead code
 * Tcl interface to control LegUp parameters: see examples/legup.tcl
 * Supported Quartus version is now 10.1sp1

Minor new features
+++++++++++++++++++++++++++++++++++
 * New datastructure to represent the output circuit as Cell, Pin, and Net objects
 * Register sharing for mul/div/rem functional units
 * Binding restricts multiplier usage to DSPs available on FPGA
 * Test suite examples now return non-zero values and print "RESULT: PASS" when successful
 * Two new C example benchmarks: 1) 16-bit FFT, 2) 32-bit 16 tap FIR filter
 * Connected signals are now verified to have equal bit width
 * All signals that don't drive primary outputs are removed
 * New log file: scheduling.legup.rpt, which lists the LLVM instructions assigned to each state
 * New log file: binding.legup.rpt, which lists the patterns found and shared during binding
 * Divider functional units now use clock enable instead of a counter
 * Most classes no longer inherit from LLVM's FunctionPass to avoid LLVM PassManager issues
 * LegupTcl and LegupConfig files moved into llvm/lib/Target/Verilog directory
 * Combinational loops are detected and avoided in binding
 * Verilog variable names now include the LLVM register name, basic block name, and C function name
 * State names are now appended with the actual state number
 * Makefile now supports parallel make ie. make -j4
 * benchmark.pl parser now supports StratixIV and TimeQuest
 * Makefile can support linking multiple C files

Bug Fixes
+++++++++++++++++++++++++++++++++++
 * Altsyncrams now have correct intended_device_family
 * Fixed memory leaks using valgrind
 * No more Quartus warnings when synthesizing Verilog output
 * Fix to Verilog output "if" statements that reduced ALM count
 * Added warning for uninitialized variables
 * Fixed varXXXX variable postfix changing between runs
 * Combinational always blocks now use blocking assignment
 * Removed inferred latches

Improvements to the hybrid flow
++++++++++++++++++++++++++++++++++
 * Designed a new memory controller for hardware accelerators to control memory
   accesses between global and local memory
 * Added a test suite to accelerate each function in all benchmarks. All
   functions return the correct result.
 * Added burst capability and pipeline bridges to the processor
 * Combined 3 avalon ports from accelerator into 1 port for stability and
   reduced area
 * One new C example benchmark, memory_access_test, to test different memory
   access patterns
 * Fixed simulation path and SOPC generation issues which were causing problems
   for certain users
 * Fixed minor bugs in data cache

Beta features
+++++++++++++++++++++++++++++++++++
 * Hardware profiler for TigerMIPS soft processor
 * Loop pipelining using Iterative Modulo Scheduling

LegUp 1.0
~~~~~~~~~~~~~~~~

Features
+++++++++++++++++++++++++++++++++++

 - C to Verilog high-level synthesis tool. Tested on Linux 32/64-bit.
 - Supports CHStone benchmark suite and dhrystone benchmarks
 - Tiger MIPS processor from the University of Cambridge
 - ASAP/ALAP scheduling with operator chaining and pipelined functional units
 - Binding for multipliers and dividers using bipartite weighted matching
 - Quality of results for Cyclone II are given in [Canis11]_.
   We've found that the area-delay product over our benchmarks is compariable
   to eXCite, a commercial high-level synthesis tool.

.. [Canis11] 
    A. Canis, J. Choi, M. Aldham, V. Zhang, A. Kammoona, J.H. Anderson, S. Brown,
    T. Czajkowski, "LegUp: High-level synthesis for FPGA-based
    processor/accelerator systems," ACM/SIGDA International Symposium on Field
    Programmable Gate Arrays (FPGA), pp. 33-36, Monterey, CA, February 2011.
