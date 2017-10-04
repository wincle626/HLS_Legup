.. _FAQ:

Frequently Asked Questions 
==================================

**How is LegUp different from other high-level synthesis (HLS) tools?**

   The source code is available for research purposes and
   we include a test suite to verify circuit correctness using simulations



**Why should I use LegUp versus some other HLS tool?**
   If you may have a need to modify or experiment with the underlying
   HLS algorithms, then you need access to the HLS tool source code.
   The reason you may need to modify such algorithms is: 1) you are an HLS
   researcher, or 2) you are curious how they work, or 3) you wish
   to build HLS support for a hardware platform that is presently unsupported
   by any existing commercial HLS tool

**What is the latest version of the LegUp release?  When did the project start?**
   4.0 is the latest release.  We started the project at the University of Toronto in 2009 and
   made our first public release in 2011

**Is LegUp free to use for commercial purposes?**
   No, it is free only for research/academic purposes.  Please
   see the license agreement when you download LegUp

**Should I use the Virtual Machine or install LegUp from scratch?**
   We highly recommend using the VM, as it is a turn-key solution that
   requires no messy library installation.

**What are the goals of the LegUp project?**

 - To make FPGAs easier to program
 - To help researchers develop new high-level synthesis algorithms

**What is the input high-level lanagage?**

   ANSI C without recursive functions, or dynamic memory.
   Functions, arrays, structs, global variables, floating point, and pointers are supported

**What is the output?**

   Verilog that can be simulated with ModelSim and synthesized using Altera Quartus II
   The synthesized circuits have been verified in hardware using an Altera DE2 (Cyclone II FPGA),
   Altera DE4 (Stratix IV FPGA), Altera DE5 (Stratis V FPGA), or the Altera DE1-SoC (Cyclone V-SoC FPGA).

**Does LegUp support software/hardware partitioning?**

  Yes. We call this the LegUp hybrid flow. You can specify a list of functions
  to synthesize into hardware accelerators. The rest of the program is left
  running on a soft MIPS processor or ARM processor and hardware/software communication
  interfaces are generated automatically.  The MIPS soft processor can be used
  on any of the supported FPGAs; the ARM processor is only avaialble on the Cyclone V-SoC FPGA (DE1-SoC board).

**What high-level synthesis algorithms are supported?**

 - SDC scheduling with operator chaining and pipelined functional units
 - Binding using bipartite weighted matching
 - Pattern-based resource sharing
 - Loop pipelining
 - If-conversion (beta)

**How are the quality of results?**

 - Hardware metrics are given on our quality of results page.
 - We've found that the area-delay product over our benchmarks is compariable
   to eXCite, a commercial high-level synthesis tool.  
 - Quality of loop pipelining results are equal to (or better than)
   a state-of-the-art commercial tool (2014)

**Do you support VHDL output?**

    No. We only support Verilog.

**Do you support Xilinx FPGAs?**

    No, not officially.  However, it is possible to generate
    Verilog in the pure hardware flow (no processor) that 
    is generic and can be targeted to Xilinx FPGAs (at least for integer programs that require no floating point).  
    See the Constraints Manual for how to configure LegUp to use
    generic dividers.  However, bear in mind that LegUp uses delay estimation
    models during scheduling to make decisions about operator chaining.  Presently,
    the estimation models are only developed for Altera devices.

**Do you support having a NiosII or Microblaze processor?**

    No. Swapping the Tiger MIPS processor with a Microblaze/NiosII processor
    would be non-trivial.

**How can I see the CDFG from legup? Can you display a gantt chart?**

    Yes! There is a "bare bones" GUI that you can use to see the
    CDFG and schedule in a Gantt-style form.  To find out how
    to use the GUI, do Tutorial #1, which is found on the Tutorials
    section of the website.

**Does legup support scheduling constraints? e.g., the number of
operators, the time a certain operation should be used?**

    Yes, to some extent.  You can control the number of specific
    types of each hardware resource to be used.  See the constraints
    manual for how to use TCL constraints to do this.

**How often do you release?**

   Roughly every year.

**Why use the LLVM compiler infrastructure over GCC?**

    When we compared LLVM to GCC we found that the benefits outweighed the
    disadvantages.

    GCC Pros:

      * Mature and very popular
      * Supports auto-vectorization
      * Compiles faster code than LLVM (5-10%)
      * Support for adding new optimization passes using a shared library  (plug-in)

    GCC Cons:

      * Very little documentation
      * Large complex C codebase with heavy use of globals and macros.
      * Only have access to single static assignment form (GIMPLE) in the optimization phase

    LLVM Pros:

      * Great Documentation
      * Used by Apple, NVIDIA, Xilinx, Altera, and others
      * Very modular C++ design. Easy to add compiler passes and targets
      * Code is very easy to work with and understand
      * Access to SSA in every stage of the compiler
      * Permissive BSD license
      * Mature state-of-the-art compiler

**Why did you write a new high-level synthesis tool when there are so many out there?**

   None of the existing high-level synthesis tools have source code available
   for researchers.
   `GAUT <http://www-labsticc.univ-ubs.fr/www-gaut/>`_ claims to be open-source
   but the code is not available for download.
   `xPilot <http://cadlab.cs.ucla.edu/soc/>`_ from UCLA is an advanced research
   tool but only the binary is available and it hasn't been updated since 2007.
   `ROCCC <http://www.jacquardcomputing.com/roccc/>`_ provides an open source
   eclipse plugin based on SUIF and LLVM but only supports small C programs.
   Standard C code must be rewritten to work with ROCCC because all function
   parameters must be structs. 
   `Trident <http://trident.sourceforge.net/>`_ uses a very old version of LLVM
   to interface with an extensive amount of Java code, but unfortunately no
   longer compiles with the latest version of LLVM.

