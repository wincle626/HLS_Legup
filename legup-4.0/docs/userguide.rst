.. highlight:: c

.. _userguide:

User Guide
=================

LegUp accepts a vanilla ANSI C file as input,
that is, no pragmas or special keywords are required, 
and produces a Verilog hardware description file as output that
can be synthesized onto an Altera FPGA.
Any C **printf** statements are converted to Verilog **$display** statements that
are printed during a modelsim simulation, making it possible
to compile the same C file with gcc and check its output against the
simulation.

LegUp has two different synthesis *flows*: 
 * Pure hardware: Synthesizes the whole C file into hardware with no soft processor
 * Hybrid: Execute a portion of the C file on the TigerMIPS soft processor and
   synthesize the rest into hardware

The LegUp synthesis flow is driven by TCL scripts and Makefiles mainly in the 
``examples`` directory. The ``examples`` directory contains sample C benchmark 
programs that make up the LegUp test suite.  There is on TCL script and are four
global Makefiles:

* ``legup.tcl``: Defines all of the default user defined settings that can be 
  used to guide LegUp's hardware/software generation. 
  Users may have to edit this Makefile to change the following variables:

    * **FAMILY**: specify the target FPGA device family, either: CycloneII (default) or StratixIV
    * **LEGUP_SDC_PERIOD**: specify the target clock period constraint (ns). 
      Defaults to 15ns for CycloneII and 5ns for StratixIV.

* ``Makefile.config``: This Makefile defines all the Makefile global variables.
  Most of these global variables are read from the settings with the TCL files 
  and should not be modified in this file.

* ``Makefile.common``: This is the central Makefile that is included by all other 
  Makefiles in the ``examples`` directory. It includes all of the primary make
  targets that users would wish to invoke.
  This file includes all the other Makefiles in the example directory.

* ``Makefile.ancillary``: This Makefile contains secondary make targets that are 
  called by the primary targets in ``Makefile.common``. Users should not call 
  these target directly.

* ``Makefile.aux``: This Makefile contains old or infrequently used targets. 
  This target should not be used, as they may no longer work and are not tested
  on a regular basis. 


If we look inside the ``examples/array`` directory there are three files:

 * ``array.c``: This is the C file we wish to synthesize into hardware.
 * ``dg.exp``: Test suite file. See :ref:`testsuite`
 * ``Makefile``

Note: That a fourth file, ``config.tcl``, can be included to overwrite any of the 
default settings in ``legup.tcl``, such as the target board or whether loop 
pipeline is enabled. An example of enabling loop pipelining in the **Loop 
Pipelining** section below.

The contents of ``Makefile`` are:

.. code-block:: make

    NAME=array
    ifeq ($(NO_OPT),)
        NO_OPT=1
    endif
    ifeq ($(NO_INLINE),)
        NO_INLINE=1
    endif
    LEVEL = ..
    include $(LEVEL)/Makefile.common

There are two important environment variables defined here:

 * **NO_OPT**: Disable all compiler optimizations. This passes the flag ``-O0`` to ``clang``.
 * **NO_INLINE**: Disable all function inlining

The reason we turn off all optimizations and disable inlining for this simple benchmark
is for testing purposes. We want to avoid the LLVM compiler optimizing away the whole program.
For most complex programs (like CHStone) you will want to remove these
Makefile variables to enable full LLVM optimizations.

Note that this Makefile includes ``examples/Makefile.common``, which uses the **NAME** and
**LEVEL** variables to customize the LegUp synthesis flow for this specific benchmark.

The central ``examples/Makefile.common`` defines the LegUp synthesis flow. To run
LegUp use the following commands:

 * **make**: run the pure hardware flow
 * **make hybrid**: run the hybrid flow. 
 * **make sw**: run the software only flow. 

A few other useful commands for the pure hardware flow are:

 * **make v**: simulate the output Verilog file with Modelsim in textual mode
 * **make w**: simulate the output Verilog file with Modelsim and show waveforms
 * **make p**: create a Quartus project in the current directory
 * **make q**: run the Quartus mapper on the Verilog file
 * **make f**: run a full Quartus compile Verilog file
 * **make watch**: debug the hardware implementation by comparing a Modelsim simulation trace to a pure software trace. See :ref:`watch`.
 * **make dot**: compile all .dot graph files in the current directory into .ps files
   
A few other useful commands for the hybrid and software only flows are:

 * **make hybridsim**: run the hybrid flow and simulate the output Verilog with Modelsim
 * **make swsim**: run the software only flow and simulate the MIPS processor executing the software with Modelsim
 * **make hybridquartus**: run a full Quartus compile on the hybrid system created with make hybrid
 * **make emul**: simulate MIPS assembly on GXemul MIPS emulator
   
.. NOTE::

    For examples that use the hybrid flow look in ``examples/chstone_hybrid/``

Pure Hardware Flow
------------------------------

The pure hardware flow synthesizes the entire C file into hardware with no soft
processor. To run this flow use:

.. code-block:: bash

    make

This is similar to other high-level synthesis tools. To look at an example,
change into the ``legup/examples/array`` directory and type ``make``. This will
run the following commands:

.. code-block:: bash

     ../mark_labels.pl array.c > array_labeled.c

``mark_labels.pl`` annotates loop that have labels as required for loop pipelining.

.. code-block:: bash

     clang-3.5 array_labeled.c -emit-llvm -c -fno-builtin -I ../lib/include/ 
     -m32 -I /usr/include/i386-linux-gnu -O0 -mllvm -inline-threshold=-100 
     -fno-inline -fno-vectorize -fno-slp-vectorize -o array.prelto.1.bc

``clang`` compiles the ``array.c`` file into LLVM byte code file:
``array.prelto.1.bc``. Note that inlining is off (``-mllvm
-inline-threshold=-100``) and optimizations are off (``-O0``).
The next command:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/opt -mem2reg -loops -loop-simplify < 
     array.prelto.cv.bc > array.prelto.2.bc

This uses the LLVM ``opt`` command to run a LegUp LLVM passes called ``-mem2reg``,
``-loops`` and ``-loop-simplify``, which performs promotes memory references to 
be register references and general loop optimization. The command produces 
``array.prelto.2.bc``.
The next command:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/opt 
     -load=../../llvm/Release+Asserts/lib/LLVMLegUp.so 
     -legup-config=../legup.tcl  -disable-inlining -disable-opt 
     -legup-prelto < array.prelto.linked.1.bc > array.prelto.6.bc

This uses the LLVM ``opt`` command to run a LegUp LLVM pass called ``-legup-prelto``, which
performs LLVM intrinsic function lowering and produces ``array.prelto.6.bc``.
The next command:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/opt 
     -load=../../llvm/Release+Asserts/lib/LLVMLegUp.so 
     -legup-config=../legup.tcl  -disable-inlining -disable-opt 
     -std-link-opts < array.prelto.6.bc -o array.prelto.bc

This uses the LLVM ``opt`` command to run a LLVM pass called ``-std-link-opts``, which
performs standard LLVM link-time optimizations and produces ``array.prelto.bc``.
The next command:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/llvm-link  array.prelto.bc 
     ../lib/llvm/liblegup.bc ../lib/llvm/libm.bc -o array.postlto.6.bc

This uses the LLVM ``llvm-link`` command to link in one of llvm's libraries, 
``libm.bc`` and produces ``array.postlto.6.bc``.
The next command:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/opt -internalize-public-api-list=main 
     -internalize -globaldce array.postlto.6.bc -o array.postlto.8.bc

This uses the LLVM ``opt`` command to run a LLVM pass called ``-globaldce``, which
performs standard LLVM dead-code elimination (DCE) to remove all unused functions 
and produces ``array.postlto.8.bc``.
The next command:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/opt 
     -load=../../llvm/Release+Asserts/lib/LLVMLegUp.so 
     -legup-config=../legup.tcl  -disable-inlining -disable-opt 
     -instcombine -std-link-opts < array.postlto.8.bc -o array.postlto.bc

This uses the LLVM ``opt`` command to run a LLVM pass called ``-std-link-opts``, which
performs standard LLVM link-time optimizations and produces ``array.postlto.bc``.
The next command:

.. code-block:: bash

     # iterative modulo scheduling
     ../../llvm/Release+Asserts/bin/opt 
     -load=../../llvm/Release+Asserts/lib/LLVMLegUp.so 
     -legup-config=../legup.tcl  -disable-inlining -disable-opt -basicaa 
     -loop-simplify -indvars2  -loop-pipeline array.postlto.bc -o array.1.bc
     ../../llvm/Release+Asserts/bin/opt 
     -load=../../llvm/Release+Asserts/lib/LLVMLegUp.so 
     -legup-config=../legup.tcl  -disable-inlining -disable-opt 
     -instcombine array.1.bc -o array.bc 

This uses the LLVM ``opt`` command to run a LegUp LLVM pass called ``-loop-pipeline``, which
pipeline loops if pipelining is enabled and produces ``array.bc``.
The following commands:

.. code-block:: bash

     ../../llvm/Release+Asserts/bin/llvm-dis array.prelto.linked.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.prelto.6.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.prelto.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.postlto.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.postlto.6.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.postlto.8.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.1.bc
     ../../llvm/Release+Asserts/bin/llvm-dis array.bc


Disassemble the LLVM bytecode using ``llvm-dis`` and create text files holding 
the LLVM intermediate representation for all stages of the LegUp flow.
The final command:

.. code-block:: bash

    ../../llvm/Debug+Asserts/bin/llc 
    -legup-config=../legup.tcl -march=v array.bc -o array.v

This uses the LLVM ``llc`` compiler targeting architecture ``v`` (Verilog). ``llc`` reads 
the ``examples/legup.tcl`` file containing LegUp synthesis parameters, and including a
device database file for the selected family, which holds the delay and area 
information for hardware operations.
Finally, ``llc`` calls LegUp backend pass (see ``runOnModule()`` in
``llvm/lib/Target/Verilog/LegupPass.cpp``) to produce the Verilog file
``array.v`` from the LLVM bytecode ``array.bc``.

Loop Pipelining
------------------------------

Loop pipelining is a feature introduced in LegUp 3.0. To look at some examples
that utilize loop pipelining navigate to the ``legup/examples/pipeline/simple``
directory. Take a look in the ``Makefile``

.. code-block:: make

    NAME=simple
    LOCAL_CONFIG = -legup-config=config.tcl

    # don't unroll the loop
    CFLAG += -mllvm -unroll-threshold=0

    LEVEL = ../..
    include $(LEVEL)/Makefile.common

The **LOCAL_CONFIG** variable specifies a local configuration tcl file named
``config.tcl`` in the current directory. Also note that we've turned the LLVM loop
unroll threshold to 0 so that the four iteration loop in this example is not unrolled.
Open ``config.tcl``:

.. code-block:: tcl

    source ../config.tcl

    loop_pipeline "loop"

    set_parameter LOCAL_RAMS 1

The :ref:`loop_pipeline` tcl command specifies that we wish to pipeline the loop with
label "loop" in ``simple.c``.
The :ref:`LOCAL_RAMS` tcl command causes LegUp to use local memory when possible, 
instead of storaging arrays in a global memory.

Open ``../config.tcl``:

.. code-block:: tcl

    source ../../legup.tcl

    set_parameter PRINTF_CYCLES 1

    set_operation_latency multiply 0

    set_project StratixIV DE4-530 Tiger_DDR2

The :ref:`PRINTF_CYCLES` tcl command causes ModelSim to print the cycle count
each time ``printf`` is called.
The :ref:`set_operation_latency multiply` tcl command causes LegUp to use 
multiplier that have a latency of zero cycles.
The :ref:`set_project` tcl command tells LegUp what FPGA Family, Development Kit
and project to target.

Open ``simple.c`` and verify the for loop has a label::

    loop: for (i = 0; i < N; i++) {

Now run ``make`` and ``make v``. Your modelsim output should look
like:

.. code-block:: none

	# Cycle:           52 Time:        1090    Loop body
	# Cycle:           53 Time:        1110    Loop body
	# Cycle:           53 Time:        1110    a[          0] =   1
	# Cycle:           53 Time:        1110    b[          0] =   5
	# Cycle:           54 Time:        1130    Loop body
	# Cycle:           54 Time:        1130    a[          1] =   2
	# Cycle:           54 Time:        1130    b[          1] =   6
	# Cycle:           55 Time:        1150    Loop body
	# Cycle:           55 Time:        1150    a[          2] =   3
	# Cycle:           55 Time:        1150    b[          2] =   7
	# Cycle:           55 Time:        1150    c[          0] =   6
	# Cycle:           56 Time:        1170    a[          3] =   4
	# Cycle:           56 Time:        1170    b[          3] =   8
	# Cycle:           56 Time:        1170    c[          1] =   8
	# Cycle:           57 Time:        1190    c[          2] =  10
	# Cycle:           58 Time:        1210    c[          3] =  12
	# Cycle:           61 Time:        1270    c[          0] =   6
	# Cycle:           63 Time:        1310    c[          1] =   8
	# Cycle:           65 Time:        1350    c[          2] =  10
	# Cycle:           67 Time:        1390    c[          3] =  12
	# At t=     1410000 clk=1 finish=1 return_val=        36
	# Cycles:           68


Notice how the print statements are happening out-of-order? For instance
``a[2]`` is printing out before ``c[0]``. To get more information about the
iterative modulo schedule of the loop body open ``pipelining.legup.rpt`` and
scroll to the bottom. You will see that the initiation interval (II) of the
loop is 1. Each instruction is scheduled into a stage of the pipeline.  To get
a better look at the pipeline run ``make w``. When asked "Are you sure you want
to finish?" select No. Use the ``ctrl-s`` shortcut to search for a signal
called ``loop_1_pipeline_start``. Hit tab on this signal to get to scroll to when
it's asserted. Zoom out a bit and you will be able to see the
``loop_1_valid_bit_*`` signals for when each time step of the pipeline is valid.

Try commenting out the ``loop_pipeline`` tcl command in ``config.tcl``. Run
``make`` and ``make v``.  Notice that the circuit gets slower, with latency in
cycles increasing to 77. Also the print statements are now happening in order.

Look through the other benchmarks in ``legup/examples/pipeline/`` to get more
examples of using loop pipelining. For more details see :ref:`loop_pipelining`.




Parallel Flow
--------------

LegUp can also execute multiple accelerators in parallel. This is done using Pthreads and OpenMP. 
Each thread is compiled into an accelerator, and LegUp instantiates as many accelerators as the number of threads used in the C program. 
Using Pthreads, you can either execute the same function in parallel using multiple threads, or you can also execute different functions in parallel. 
OpenMP can be used to execute a loop in parallel.

LegUp currently supports the following Pthread and OpenMP functions/pragmas:

==========================  ==========================  ========================
Pthread Functions           OpenMP Pragmas              OpenMP Functions
==========================  ==========================  ========================
pthread_create              omp parallel                omp_get_num_threads
pthread_join                omp parallel for            omp_get_thread_num
pthread_exit                omp master
pthread_mutex_lock          omp critical
pthread_mutex_unlock        omp atomic
pthread_barrier_init        reduction(operation: var)
pthread_barrier_wait
==========================  ==========================  ========================

For working examples that use Pthreads and OpenMP, look in ``legup/examples/parallel/``


The following make targets are relevant for the parallel flow:
 * ``make``: compile Pthreads applications to pure hardware
 * ``make parallel``: compile OpenMP and Pthreads+OpenMP applications to pure hardware
 * ``make v``: simulate parallel hardware with ModelSim
 * ``make w``: simulate parallel hardware with ModelSim, showing waveforms


.. Comment out the multi-ported caches subsection for the 4.0 release.

	Multi-ported Caches
	~~~~~~~~~~~~~~~~~~~

	When multiple accelerators execute in parallel, the memory bandwidth easily becomes the bottleneck, especially since FPGAs have on-chip RAMs which only have up to two ports. 
	To mitigate this, we created multi-ported caches which allow multiple memory accesses at the same time. There are two types of multi-ported caches, one of which is based
	on the live-value table (LVT) memory, and the other which uses the multi-pumping (MP) memory. For more details on the hardware architecture, please refer to our paper, ``Impact of Cache Architecture and Interface on Performance and Area of FPGA-Based Processor/Parallel-Accelerator Systems``, on our publications page. 
	Both the LVT cache and the MP cache are configured to have 4 ports, which means that 4 accelerators can access memory at the same time. When there are more than 4 parallel accelerators, arbitration is created for ports with more than one accelerator, and when multiple accelerators try to access the same port concurrently, the arbitor services the memory accesses in a round-robin manner. To use these multi-ported caches, one has to put the type of the multi-ported cache as well as the number of ports of the cache in the ``config.tcl`` file, as shown below::

		set_dcache_ports 4
		set_dcache_type LVT

	This creates the LVT cache and automatically connects the accelerators to each port of the cache. To use the MP cache, put the following in the ``config.tcl`` file::

		set_dcache_ports 4
		set_dcache_type MP

	Currently, only 4 ports are supported for both the LVT and the MP cache and they can only be used on the Stratix IV FPGA, as the Cyclone II FPGA has limited resources. These multi-ported caches are only used for the data cache, since the instruction cache is only accessed by the single MIPS processor. However, both the data cache and the instruction cache can be configured in terms of their total cache sizes, line sizes, and associativity by putting the following in the ``config.tcl`` file::

		set_dcache_size 	size in KB
		set_dcache_linesize size in bytes
		set_dcache_way 		associativity
		set_icache_size 	size in KB
		set_icache_linesize size in bytes
		set_icache_way 		associativity

	Using these commands automatically configures the cache and generates the RAMs with the specified sizes. 
	The data cache and the instruction cache can be configured independently and if they are not configured, a direct-mapped cache is used with the default sizes. 

.. end Multi-ported Caches

.. end Parallel Flow


Hardware/Software Hybrid Flow
------------------------------

LegUp can automatically compile one or more selected C functions into hardware
accelerators while running the remaining program segments on the processor.
Communication between the processor and hardware accelerators is performed over
the Avalon Interconnection Fabric, which is automatically generated by Altera's
QSys System Integration Tool.

The hybrid flow can target either a soft Tiger MIPS processor, or a hard ARM
Cortex-A9 processor.

..
	The hybrid flow can be implemented on the Cyclone II FPGA with SDRAM off-chip
	memory on the Altera DE-2 board or on the Stratix IV FPGA with DDR2 off-chip
	memory on the Altera DE-4 board.


Hybrid Flow Overview
~~~~~~~~~~~~~~~~~~~~

The steps of the hybrid flow are as follows:
 1. The C source is compiled to LLVM IR
 2. The LLVM IR is partitioned into a hardware section, and a software section
 3. A wrapper function is generated to replace each accelerated function in the software section
 4. The software section is compiled to either a MIPS or ARM executable
 5. The hardware section is compiled to verilog, taking global variable addresses from the compiled software section
 6. QSys is used to generate a system that includes the appropriate processor, the generated accelerator(s), and any additional hardware including caches, profilers, etc.  QSys automatically generates any necessary interconnect.
 7. The system can now be simulated (MIPS only), or synthesized and run on the board (ARM only)

If the function designated for acceleration has descendants (other functions
which are called by the designated function), all of its descendants are also
moved to hardware.
Descendant functions which have been moved to hardware which are not called by
other software functions are removed from the software section to reduce the
program footprint.
All remaining functions are compiled to a MIPS or ARM executable that can be run
on the processor.


Wrapper Functions
~~~~~~~~~~~~~~~~~

LegUp generates a C wrapper function for every function to be accelerated.

For example, look at the example in ``legup/examples/matrixmultiply``.
There are three files in this directory:
 * ``matrixmultiply.c``: C source for the application
 * ``Makefile``: local makefile for the application
 * ``config.tcl``: local LegUp configuration for the application

Let's say we want to accelerate the multiply function, shown below::

	int multiply(int i, int j)
	{
		int k, sum = 0;
		for(k = 0; k < SIZE; k++)
		{
			sum += A1[i][k] * B1[k][j];
		}
		resultAB1[i][j] = sum;
		return sum;
	}

To accelerate this function, put the function name in the ``config.tcl`` file
as shown below::

    set_accelerator_function "multiply"

Run ``make hybrid``.
LegUp will generate a C wrapper function,
**legup_sequential_multiply**, to replace the **multiply** function.
The wrapper function can be seen in the LLVM IR file ``matrixmultiply.sw.ll``:

.. code-block:: llvm

	define internal fastcc i32 @legup_sequential_multiply(i32 %i, i32 %j) {
	  volatile store i32 %i, i32* inttoptr (i32 -268435444 to i32*)
	  volatile store i32 %j, i32* inttoptr (i32 -268435440 to i32*)
	  volatile store i32 1, i32* inttoptr (i32 -268435448 to i32*)
	  %1 = volatile load i32* inttoptr (i32 -268435456 to i32*)
	  ret i32 %1
	}

Equivalent C code for the wrapper is shown below::

    // memory mapped addresses
    #define add_DATA   (volatile int *)0xf00000000
    #define add_STATUS (volatile int *)0xf00000008
    #define add_ARG1   (volatile int *)0xf0000000C
    #define add_ARG2   (volatile int *)0xf00000010

    int legup_sequential_multiply(int i, int j)
    {
        // pass arguments to accelerator
        *add_ARG1 = i;
        *add_ARG2 = j;
        // give start signal
        *add_STATUS = 1;
        // get return data
        return = *add_DATA;
    }

The wrapper function sends its arguments to the hardware accelerator then
asserts the accelerator start signal, at which point the accelerator will stall
the processor by asserting the Avalon waitrequest signal.
When the accelerator finishes and sets waitrequest to 0, the processor resumes
and retrieves the return value from the accelerator.


MIPS Hybrid Flow
~~~~~~~~~~~~~~~~

The MIPS flow should work on any development board with a compatible FPGA.
The DE1-SoC, SoCKit, DE4, and DE5 boards are supported.

**Running the MIPS Hybrid Flow**

First, ensure that a Tiger MIPS project has been selected in
``legup/examples/legup.tcl``::

	set_project CycloneV DE1-SoC Tiger_SDRAM

Next, ensure the local ``config.tcl`` contains the function to be accelerated::

    set_accelerator_function "multiply"

The following make targets are relevant for the MIPS hybrid flow:
 * ``make hybrid``: generate the hybrid system
 * ``make hybridsim``: generate the hybrid system and simulate it in ModelSim
 * ``make simulation``: simulate the system in ModelSim (``make hybrid`` must have been run previously)
 * ``make simulation_with_wave``: simulate the system in ModelSim with waveforms (``make hybrid`` must have been run previously)
 * ``make hybrid_compile``: run a full Quartus compile on the hybrid system (``make hybrid`` must have been run previously)
.. * ``make program_board``: program the board with the generated .sof file (``make hybrid_compile`` must have been run previously)
.. * ``make run_on_board``: to connect to the processor, and download and run the program (``make program_board`` must have been run previously)


**Memory Coherency**

In order to keep memory coherent, all global variables which are not
constants are stored in main memory, which is shared between the processor
and accelerators. When a hardware accelerator tries to access global variables
it first checks the on-chip data cache, which is also shared between the
processor and all accelerators. If there is a cache hit, the data is retrieved from
the cache. If there is a cache miss, the off-chip main memory is accessed, which
takes many more cycles to return the data. All constant variables in the hardware
accelerator are stored in local block RAMs, since they will never be modified
and thus it does not make sense to store them in high latency off-chip memory.
All hardware accelerator local variables are also stored in local block RAMs.

ARM Hybrid Flow
~~~~~~~~~~~~~~~

Thy hybrid flow can target either a Tiger MIPS processor, or an ARM Cortex-A9.
To test the ARM flow, it is necessary to have a board with a CycloneV SoC.
The DE1-SoC and SoCKit boards are supported.

**Running the ARM Hybrid Flow**

First, ensure that a ARM project has been selected in
``legup/examples/legup.tcl``::

	set_project CycloneV DE1-SoC ARM_Simple_Hybrid_System

Next, ensure the local ``config.tcl`` contains the function to be accelerated::

    set_accelerator_function "multiply"

The following make targets are relevant for the ARM hybrid flow:
 * ``make hybrid``: generate the hybrid system
 * ``make hybrid_compile``: run a full Quartus compile on the hybrid system (``make hybrid`` must have been run previously)
 * ``make program_board``: program the board with the generated .sof file (``make hybrid_compile`` must have been run previously)
 * ``make run_on_board``: to connect to the processor, and download and run the program (``make program_board`` must have been run previously)

Altera does not provide a simulation model for the ARM core; therefore, it is
not possible to simulate an ARM hybrid system.

.. NOTE::

	Any accelerator generated with the ARM hybrid flow should be the same as
	one generated with the MIPS hybrid flow, with the exception of global
	variable addresses.
	Hence, if an accelerator simulates properly in the MIPS flow, it should
	also work properly in the ARM flow.


**Memory Coherency**

Unlike the MIPS system, it is not possible to connect the accelerator(s)
directly to the processor cache.
However, cache coherency can still be maintained.

In the ARM system, the processor caches are part of the hard processor system,
or HPS.
The following figure shows the architecture of the Cyclone V Soc device found
on the DE1-SoC and SoCKit boards:

.. figure:: /images/ARMArch.png
	:scale: 65%
	:align: center

	Cyclone V SoC Architecture

The top of the figure shows the FPGA side, and the bottom shows the hard
processor system, or HPS, side.
The FPGA bridge facilitates communication between the FPGA and the HPS.
The L3 interconnect connects the FPGA bridge, SDRAM controller, microprocessor
unit, L2 cache, and peripherals (not shown).
The microprocessor unit contains a Cortex-A9 MPCore processor with two CPUs.
Each CPU has separate 32 KB instruction and data caches.
There is a shared 512 KB L2 cache.
The microprocessor unit also contains a snoop control unit, or SCU, and the
accelerator coherency port, or ACP.
The SCU maintains cache coherency between the two CPUs.
The ACP allows masters on the L3 interconnect to perform memory accesses that
are cache coherent with the microprocessor unit.

Memory accesses made to the ACP get routed through the SCU.
The SCU subsequently routes the request to the L1 and L2 cache.
If the request misses in both caches, it is routed to the SDRAM controller.
In addition to coherency, ACP accesses have the added benefit, (in the case of a
cache hit), of being faster than going directly to the SDRAM controller.
In the case of a miss, access times are the same as going directly to the SDRAM
controller through the L3 interconnect.

The ACP can be accessed through the FPGA-to-HPS bridge.
The 4 GB of address space is divided as follows:
 * ``0x00000000`` to ``0x7FFFFFFF`` maps to the SDRAM controller
 * ``0x80000000`` to ``0xBFFFFFFF`` maps to the ACP
 * ``0xC0000000`` to ``0xFFFFFFFF`` maps to the FPGA slaves and peripherals

LegUp forces all global memory accesses from the accelerator to go through the
ACP, ensuring cache coherency.


Hybrid Parallel Flow
~~~~~~~~~~~~~~~~~~~~

It is also possible to use the parallel and hybrid flows together.
``make hybrid`` can be used to compile Pthreads applications
``make hybridparallel`` can be used to compile OpenMP and Pthreads+OpenMP applications

This parallel hybrid flow has all the same properties as the sequential
hybrid flow, except for a few minor differences.
Instead of stalling the processor after calling an accelerator, the processor
continues to call all of the accelerators that execute in parallel and then
polls on the accelerators to check if they are done.
The return value is retrieved after polling.
Hence LegUp generates a pair of C wrappers for each parallel function, which is
called a calling wrapper and a polling wrapper.
The calling wrapper sends all of the arguments to the accelerator and asserts
the start signal, and the polling wrapper polls on the accelerator to check if
it is done, then retrieves the return value if necessary.
For example, let's say we want to parallelized the following add function::

    int add (int * a, int * b, int N)
    {
        int sum=0;
        for (int i=0; i<N; i++)
            sum += a[i]+b[i];
        return sum;
    }

The add function needs to be re-written so that it can be used with Pthreads, as shown below::

    void *add (void *threadarg)
    {
        int sum=0;
        struct thread_data* arg = (struct thread_data*) threadarg;
        int *a = arg->a;
        int *b = arg->b;
        int N = arg->N;
        for (int i=0; i<N; i++)
        {
            sum += a[i]+b[i];
        }
        pthread_exit((void*)sum);
    }

This add function can execute in parallel using two threads with the following code::

	pthread_create(&threads[0], NULL, add, (void *)&data[0]);
	pthread_create(&threads[1], NULL, add, (void *)&data[1]);

	pthread_join(threads[0], (void**)&result[0]);
	pthread_join(threads[1], (void**)&result[1]);

LegUp automatically replaces the call to pthread_create with calls to LegUp calling wrappers and replaces the call to pthread_join with calls to LegUp polling wrappers. With two threads executing the add function, the following shows the C-code equivalent of the wrapper functions that are generated::

    #define add0_DATA	(volatile int * ) 0xf0000000
    #define add0_STATUS	(volatile int * ) 0xf0000008
    #define add0_ARG1	(volatile int * ) 0xf000000c
    #define add0_ARG2	(volatile int * ) 0xf0000010

    void legup_call_add0(char *threadarg)
    {
        *add0_ARG1 = (volatile int) threadarg;
        *add0_ARG2 = (volatile int) 1;
        *add0_STATUS = 1;
    }

    #define add1_DATA	(volatile int * ) 0xf0000020
    #define add1_STATUS	(volatile int * ) 0xf0000028
    #define add1_ARG1	(volatile int * ) 0xf000002c
    #define add1_ARG2	(volatile int * ) 0xf0000030

    void legup_call_add1(char *threadarg)
    {
        *add1_ARG1 = (volatile int) threadarg;
        *add1_ARG2 = (volatile int) 2;
        *add1_STATUS = 1;
    }

    char *legup_poll_add0()
    {
        while (*add0_STATUS == 0){}
        return (char*)*add0_DATA;
    }

    char *legup_poll_add1()
    {
        while (*add1_STATUS == 0){}
        return (char*)*add1_DATA;
    }

legup_call indicates a calling wrapper, and legup_poll indicates a polling wrapper.
The first argument pointer, ARG1, passes in the function argument, threadarg,
and the second argument pointer, ARG2, passes in the threadID.
This threadID is determined at compiled time by looking at the number of threads which are accelerated.


Hybrid Flow Limitations
~~~~~~~~~~~~~~~~~~~~~~~

LegUp's hybrid flow does not work with the following features:
 * local memories
 * shared local memories


.. end Hybrid Flow




Pure Software Flow
------------------

LegUp also has a pure software flow that can be used for testing your C code.
The pure software flow works for both MIPS and ARM processors.
To target a specific processor architecture, make sure an appropriate project
has been selected in ``legup/examples/legup.tcl``.

The following make targets are relevant to the pure software flow:
 * ``make sw``: generate an ELF file for the desired processor architecture
 * ``make swsim``: compile the application and simulate it with ModelSim (MIPS only)
 * ``make simulation``: simulate execution of the application on the processor using ModelSim (``make sw`` must have been run previously)(MIPS only)
 * ``make simulation_with_wave``: simulate execution of the application on the processor using ModelSim, with waveforms (``make sw`` must have been run previously)(MIPS only)
 * ``make run_on_board``: run the application on the board (``make sw`` must have been run previously)(ARM only)
 * ``make emul``: compile and run the application in an emulator: gxemul for MIPS and QEMU for ARM

.. end SW Flow




Custom Verilog & Propagating I/O
----------------------------------

You can tell LegUp to instantiate particular C functions in the generated
verilog but leave the hardware implementation of those function up to you.
LegUp will not attempt to compile the code inside of any functions you mark as
custom verilog, so you can include code that LegUp cannot normally compile.

Every time you run LegUp it overwrites the generated verilog files, so you need
to write your custom verilog modules in separate files and tell LegUp to include those files.  To tell legup to include a file add the following command to the TCL configuration file:

.. code-block:: tcl

                set_custom_verilog_file "file_name"

To mark functions as custom verilog you add commands to the TCL configuration
file for the project.  The syntax for this command is as follows:

.. code-block:: tcl

		set_custom_verilog_function "function_name" <memory or noMemory> \
		    <input or output> high_bit:low_bit signal_name

The ``function_name`` is the name of your function as it appears in your C code.
The next token can be either ``memory`` or ``noMemory`` to specify whether or
not your custom verilog requires access to the LegUp memory signals.  Following
the memory token are sets of tokens that describe input and output signals.  The
signals specified here will propagate up the call tree and exist in the top
level module.  You can specify as many inputs and outputs as you want, but every
input or output requires three tokens to describe it:
* ``input`` or ``output`` specifies whether the signal should be a verilog
input or output
* ``high_bit``:``low_bit`` specifies the bits to which your signal should
connect.  ``high_bit`` and ``low_bit`` must be integers greater than zero.
* ``signal_name`` specifies the name of your signal

A complete example:

.. code-block:: tcl

		set_custom_verilog_function "assignSwitchesToLEDs" noMemory \
		                                                   output 5:0 LEDR \
                                                                   input 5:0 SW \
                                                                   input 3:0 KEY

In addition to specifying your function as custom verilog your Verilog and C
code must meet some specifications that ensure that your code integrates into
the LegUp generated code.  These specifications are described in Specifications
for Custom Verilog C Code and Specifications for Custom Verilog Modules.

Specifications for Custom Verilog C Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LegUp uses compiler optimizations to improve the performance of your code.  As a
result, some functions will be inlined or removed.  For custom verilog functions
inlining is equivalent to removing, so you must tell the compiler not to remove
or inline your custom verilog functions.  To do this, add the ``noinline`` and
``used`` C attributes to your function definitions.  An example of this is
provided below:

.. code-block:: c

		void __attribute__((noinline)) __attribute__((used)) exampleFunction() {..}

Additionally, you should add a volatile memory call to your function if the C
implementation does not call any functions.  To do this, you can add the
following code snippet to the body of your function:

.. code-block:: c

		volatile int i = 0;


Specifications for Custom Verilog Modules
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LegUp converts custom verilog function calls from C to module instantiations in
Verilog.  The module instantiations that LegUp generates define the naming
scheme for all input and output signals in your custom verilog.  As a result,
the modules you write must have the following signals:

 * ``input clk``: The standard clock for the circuit
 * ``input clk2x``: A clock running twice as fast as the standard clock
 * ``input clk1x_follower``: The standard clock with a phase shift of 180
   degrees
 * ``input reset``: The reset signal for the circuit
 * ``input start``: A 1 cycle pulse signifying that the function should start
   executing
 * ``output finish``: Set to 1 to tell the LegUp state machine that it can move
   on to the next state (if you never set this to 1 in your custom verilog your
   program will stop executing after it starts your function)
 * ``return_val``: Only necessary if your C function is non-void.  Has the bit
   width of the return type specified in the C file.  Value should be held until
   the start signal is asserted.

Additionally, your custom modules must have signals for any propagating I/O that
you specify, any arguments and return values that your C function has, and all
of the memory controller signals if you specify in the config file that your
function requires access to memory.  The propagating I/O signals can be
specified as they were declared in the config file.  The arguments to your
function have the bit width of their C type (e.g. ``int`` is ``[31:0]``) and the same
name with the prefix ``arg_``. So an argument declared in C as ``int counter``
would be declared in your verilog module as ``input [31:0] arg_counter``.  The
return value is called ``return_val`` and is also the same size as the C type.
For information on the memory controller signals, please see the Memory
Controller section of Hardware Architecture.

Custom Top Level Modules
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LegUp allows you to specify a custom top level module.  The top level module you
specify will be set to the top level module of any projects made with the ``make p``
command.  To specify a custom top level module add the following command to the
config file for your project:

.. code-block:: tcl

		set_custom_top_level_module "topLevelModuleName"

For convenience, you can put your custom top level module in any of the files you
include with the ``set_custom_verilog_file`` command.

Custom Test Benches
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LegUp allows you to specify a custom test bench module.  The module you specify will
be used whenever you run the ``make v`` or ``make w`` commands.  To specify a test
bench module add the following command to the TCL configuration file:

.. code-block:: tcl

		set_custom_test_bench_module "testBenchModuleName"

Make sure that your custom test bench modules are in verilog files included with the
``set_custom_verilog_file`` command.

Known Limitations of Custom Verilog
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At the present time propagating I/O is not supported for the hybrid flow or
pthreads.

You cannot call a custom verilog function inside of a loop that you have
marked for loop pipelining.

There is currently no support for inout style arguments (pass by reference)
other than storing a value in memory and passing in a pointer to that value.

Propagating signals can only be ``input`` or ``output``.

Bit width specifiers for propagating I/O can only be integers.
