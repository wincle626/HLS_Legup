.. highlight:: cpp

.. _progman:

Programmer's Manual
======================

This is a programmer's manual for the LegUp high-level synthesis framework.  The intent is to
give a reader a top-level view of how LegUp is implemented, and where key
pieces of the functionality reside in the codebase.   LegUp is
a target back-end pass to the `LLVM <http://llvm.org/>`_ compiler
infrastructure. If you haven't used LLVM before please familiarize yourself
with the `LLVM Documentation <http://llvm.org/docs/>`_. This manual assumes
that you understand basic LLVM concepts.
First we discuss the LegUp compiler backend pass, which receives the final optimized LLVM intermediate
representation (IR) as input and produces Verilog as output. 
Next we discuss the LegUp frontend compiler passes, which receive LLVM IR as
input and produce modified LLVM IR as output.

If you just want to dive in. Start by looking at ``runOnModule()`` in
`llvm/lib/Target/Verilog/LegupPass.cpp
<http://legup.eecg.utoronto.ca/git?p=legup.git;a=blob;f=llvm/lib/Target/Verilog/LegupPass.cpp;hb=HEAD>`_

.. You should also look at the generated LegUp 
.. `Doxygen <http://legup.eecg.utoronto.ca/doxygen/namespacelegup.html>`_.


LLVM Backend Pass
------------------

Most of the LegUp code is implemented as a target backend pass in the LLVM compiler
framework. The top-level class is called *LegupPass*. This class
gets run by the LLVM pass manager, which calls the method
*runOnModule()* passing in the LLVM IR for the entire program and expecting the 
final Verilog code as output.

The LegUp code is logically structured according to the flow chart:

.. figure:: /images/hls-flow.png
	:scale: 65%
	:align: center

	LegUp flow

There are five major logical steps performed in order: Allocation, Scheduling, Binding, RTL generation, and producing Verilog output.
First, we have an *Allocation* class that reads in a user Tcl configuration script
that specifies the target device, timing constraints, and HLS options. 
The class reads another Tcl script that contains the FPGA device specific operation delay and area characteristics.
These Tcl configuration settings are stored in a global *LegupConfig* object
accessable throughout the code.
We pass the Allocation object to all the later stages of LegUp. This object
also handles mapping LLVM instructions to unique signal names in Verilog and for ensuring these names do not
overlap with reserved Verilog keywords.
Global settings that should be readable from all other
stages of LegUp should be stored in the Allocation class.

The next step loops over each function in the program and performs HLS scheduling.
The default scheduler uses the SDC approach and is implemented in the *SDCScheduler* class.
The scheduler uses the *SchedulerDAG* class, which holds all the dependencies
between instructions for a function.
The final function schedule is stored in a *FiniteStateMachine* object that specifies the start and end
state of each LLVM instruction.

Next, we perform binding in the *BipartiteWeightedMatchingBinding* class,
which performs bipartite weighted matching.
We store the binding results in a datastructure that maps each LLVM instruction
to the name of the hardware functional unit that the instruction should be implemented on.

In the next step, the *GenerateRTL* class loops over every LLVM
instruction in the program and using the schedule and binding information,
creates an *RTLModule* object (`RTL Datastructure`_) that represents the final hardware circuit. 

Finally, the *VerilogWriter* class loops over each  *RTLModule* object
and prints out the corresponding Verilog for the hardware module.
We also print out any hard-coded testbenches and top-level modules.

LLVM Frontend Passes
---------------------

In this section, we discuss portions of the LegUp code that are implemented as frontend LLVM passes. 
These passes receive LLVM IR as input and return modified LLVM IR as output and are
run individually using  the LLVM **opt** command. In the class
implementing each pass, the LLVM pass manager will call the method
*runOnFunction()* and provide the LLVM IR for the function and will expect
the modified LLVM IR as output.

For the hybrid flow, we remove all functions from the IR that should be implemented in software
in the *HwOnly* class. We remove all functions that should be implemented in hardware
with the *SwOnly* class. We run these two passes on the original IR of the program to
generate two new versions of the IR. We pass the HwOnly IR to the LegUp HLS
backend and the SwOnly IR to the MIPS/ARM compiler backend.

Loop pipelining is performed by the *SDCModuloScheduler* class. This pass will 
determine the pipeline initiation interval and the scheduled start and end time of each
instruction in the pipeline. This data is stored in LLVM IR metadata which can be read
later by the LegUp backend.

If-conversion is performed by the class *LegUpCombineBB*, which removes
simple control flow and combines basic blocks.
In the class *PreLTO*, we detect LLVM built-in functions that can exist
in the IR (i.e. memset, memcpy). We replace these functions with equivalent LLVM IR
instructions that we can synthesize in the LegUp backend.

Execution Flow
---------------

The overall flow of execution in LegUp is as follows. First, the LLVM
front end :file:`clang-3.5` takes the .c files and compiles them into
LLVM intermediate representation, stored in a byte code file
(.bc). This byte code may contain LLVM intrinsic functions, which are
functions that LLVM assumes exist in the C library: memcpy, memset,
and memmove. These functions do not exist in hardware, so we replace
them with functions that we've hand written. This is done with ``opt
-legup-prelto`` and then linking in our versions with llvm-link
lib/liblegup.bc. Now we are left solely with LLVM IR and no
intrinsics.  We pass this code to llvm-ld to perform link time
optimizations. Finally, we pass the optimized bytecode to
``llc -march=v`` to run the Verilog backend (LegUp HLS).  You
can find such calls in the main LegUp Makefile: ``examples/Makefile.common``.

The flow of the Verilog backend is as follows: First, the LLVM pass
manager calls ``LegupPass:runOnModule()``, which is a top-level
governing function of the LegUp HLS flow.  In that method, you will
see many references to LEGUP_CONFIG, which is an object that holds the constraints on the
HLS.  For instance, if you wanted to know the number of DSPs available
on this device use LEGUP_CONFIG->getMaxDPSs().  In the ``runOnModule``
function, you will also see references to the Allocation object, which
contains global information about the circuit, such as its RTL
modules, list of RAMs, and the GlobalNames object.  The GlobalNames
class is used to make sure each LLVM instruction has a unique name
that doesn't overlap with a reserved Verilog keyword (reg, wire,
etc.).  Reading further in ``LegupPass::runOnModule()``, you will see
the method creating an RTL generator (``GenerateRTL``) instance for
each function in the LLVM module being synthesized, and further down
you will see the lines that invoke the HLS scheduler for each
function:

.. code-block:: c

	GenerateRTL *HW = *i;
	HW->scheduleOperations();

The purpose of scheduling is to schedule LLVM instructions into clock
cycles. There are some helper classes to aid in this task: The data
flow graph of LLVM instructions is represented in the ``SchedulerDAG``
class. Given an instruction, you can get the successors and predecessor
instructions. For instance:

.. code-block:: llvm

    %1 = add %2, %3
    %4 = add %1, %5

The predecessors of %1 are its operands: instructions %2, and %3. %1's successor is the instruction %4.
There are also dependencies between load, store, and call instructions that can access memory.
These dependencies can be detected using alias analysis performed by LLVM.
Scheduling works on a function-by-function basis. There are two important parameters
for each instruction: 

1. The latency, how many clock cycles you must wait until the output is
   available, loads have latency of 2 (by default, but controllable through a tcl parameter).
2. The delay of an instruction is the other parameter, retrieved from
   LEGUP_CONFIG by accessing the ``Operation`` object for a given type
   of instruction (e.g. a 32-bit signed addition).  Such delays are stored
   in a tcl characterization file for each supported device.  These are
   found in the ``boards`` subdirectory of the LegUp distribution.
   For example, see ``boards/CycloneV/CycloneV.tcl``.

The delay approximations allow the algorithm to determine how many instructions
can be *chained* together in the same cycle. During scheduling, each instruction
is assigned a state object that represents a state in a finite state machine
stored in the FSM object.  Branches, jump, and switch instructions are used to
determine next state variable assignments. Each state has 3 possibilities
analogous to the LLVM branch, jump, and switch instructions. First, a
defaultTransition can be specified. Or a single transition variable can be set,
then one or more transition conditions can be specified.  If the transition
variable is equal to the condition, then the associated state is the next state.
In essence, the transitions are the edges in a state diagram. 
After we have scheduled each function, we can 
call generateRTL to create the RTLModule object
representing the final hardware circuit for the scheduled function.

An RTLModule has a list of inputs, outputs, parameters, and RTLSignals. To
understand RTLSignal, it's useful to
look at the structure of the Verilog code.  Each RTLSignal represents a wire or
register that can be driven by other RTLSignals under different conditions.
Each condition is listed as an if statement in the **always@** block devoted to that
signal.  The most common condition is if we are in a particular state. It is so
common that there is a function to simplify this process:

.. code-block:: c

    connectSignalToDriverInState(signal, driverSignal, state, instruction, ...)

Here we say that during **state** we want **driverSignal** to drive **signal**.
The optional **instruction** argument adds a comment above this Verilog
assignment indicating the instruction that **driverSignal** was
dervied from. 
Another option is to unconditionally drive a signal.  In order to do so, use:

.. code-block:: c

    signal->connect(driver, instruction) 

Note that this will clear away prior conditional drivers.  To manually specify a
conditional driver use:

.. code-block:: c

    signal->addCondition(conditionSignal, driverSignal)

If the **conditionSignal** is 1 then **driverSignal** drives **signal**. 

To create a register or wire RTLSignal use these functions:

.. code-block:: c

    rtl->addReg(...) 
    rtl->addWire(...) 

Where **rtl** is an RTLModule object, **rtl** must keep track of all signals
used in order to print the variable declarations.  To create a signal you must
specify a name. Normally the ``verilogName(instruction)`` function is used, which
creates a unique name for the instruction using the GlobalNames object in the
allocation object discussed above. For all instructions, we follow the
convention that there are 2 signals created, one wire, to represent the
instruction during the state it is assigned, and one register, which the wire
feeds in the assigned state only.  The register is used if the instruction is
used in another state. The name of the wire is ``verilogName(instruction)``,
the name of the register is ``verilogName(instruction) + "_reg"``.

.. index:: test suite
.. _testsuite:

Test Suite
-----------

The test suite is built using DejaGNU (also used by GCC and LLVM). 
The DejaGNU test framework is launched by the ``runtest`` command in the ``examples`` subdirectory,
which recursively searches all the directories in the current
working directory for ``dg.exp`` tcl files.  
Every directory in ``examples`` that is part of the test suite
has a ``dg.exp`` tcl file, for instance ``examples/array/dg.exp``.
These tcl files all load the library ``examples/lib/legup.exp``
and call functions like **run-test** or **run-test-gx** to run
various tests.

To run the default test suite use the command:

.. code-block:: bash

    cd examples
    runtest

You should see the following output after a few minutes:

.. code-block:: none

                ===  Summary ===

    # of expected passes		476

Note: The number of passes you observe may differ, depending on the LegUp version your are using.  The default test suite essentially takes every example and runs:

.. code-block:: bash

    make
    make v

The first command generates the Verilog for the testcase.  The second command simulates that Verilog with ModelSim.  
The output is then parsed to ensure the *return_val* is correct and there are
no Modelsim warnings or errors.

You should run the LegUp test suite regularly during development to ensure your
hardware is correct. We have found that it is much easier to track down bugs
this way than debugging the RTL simulations. In fact, we run our regression tests after
every commit using `buildbot <http://www.legup.org:9100/waterfall>`_.


Other useful variants of the ``runtest`` DejaGNU command are:

.. code-block:: bash

    # for verbose output:
    runtest -v
    # only run the mips test:
    runtest chstone/mips/dg.exp


LLVM Passes
------------

LLVM is structured as a series of compiler passes that run in sequence on the
underlying intermediate representation. The main LegUp pass is a target backend
called LegupPass. Passes are normally classes inheriting from FunctionPass,
which have an entry function called:

.. code-block:: c

    bool runOnFunction(Function &F); 

When runOnFunction() is called, LLVM has already constructed the intermediate
representation (IR) for the input C file. By traversing over the IR we
perform the steps to generate valid Verilog RTL code.
LegupPass inherits from ModulePass, which has an entry function called:

.. code-block:: c

    bool runOnModule(Module &M); 

In LLVM, a Module has a list of Functions.  A Function has a list of
BasicBlocks.  A BasicBlock has a list of instructions.  The definition
of a basic block is a straightline sequence of code with a single entry
point (at the beginning) and a single exit point (at the end).

Source Files
-------------

LegUp files inside the LLVM source tree:
  * The core of LegUp is in:
     * :file:`llvm/lib/Target/Verilog/`
  * Other LegUp passes that are run with opt:
     * :file:`llvm/lib/Transforms/LegUp/`
  * llc calls the LegupPass and has been slightly modified:
     * :file:`llvm/tools/llc/llc.cpp`
  * Other files with minor changes:
     * :file:`llvm/tools/opt/opt.cpp` (can use Tcl)
     * :file:`llvm/autoconf/configure.ac` (add Verilog target)
     * :file:`llvm/configure` (add Verilog target)

Important Classes
------------------

RTL Datastructure
++++++++++++++++++

The data structure that we use to represent an arbitrary circuit uses the
following classes:

  * ``RTLModule`` - a hardware module.
  * ``RTLSignal`` - a register or wire signal in the circuit. The
    signal can be driven by multiple RTLSignals each predicated on a RTLSignal
    to form a multiplexer.
  * ``RTLConst`` - a constant value.
  * ``RTLOp`` - a functional unit with one, two or three operands.
  * ``RTLWidth`` - the bit width of an RTLSignal (i.e. [31:0])

As an example let's implement the following Verilog using the RTL data structure:
                                                                     
.. code-block:: v

    module bitwise_AND_no_op_bitwise_OR_2to1mux_32bit
    #(parameter WIDTH=32)
    (
        input signed [WIDTH-1:0] data1,
        input signed [WIDTH-1:0] data2,
        input signed [WIDTH-1:0] data3,
        input signed [WIDTH-1:0] data4,
        input signed [WIDTH-1:0] data5,
        input signed [WIDTH-1:0] data6,
        input select,
        input clk,
        output reg [WIDTH-1:0] dataout
    );
        reg signed [WIDTH-1:0] data1_reg;
        reg signed [WIDTH-1:0] data2_reg;
        reg signed [WIDTH-1:0] data3_reg;
        reg signed [WIDTH-1:0] data4_reg;
        reg signed [WIDTH-1:0] data5_reg;
        reg signed [WIDTH-1:0] data6_reg;
        reg signed [WIDTH-1:0] w1;
        reg signed [WIDTH-1:0] w2;
        reg signed [WIDTH-1:0] w3;

        always @ (posedge clk)
        begin
            data1_reg <= data1;
            data2_reg <= data2;
            data3_reg <= data3;
            data4_reg <= data4;
            data5_reg <= data5;
            data6_reg <= data6;

            dataout <= (w1 & w2) | w3;
        end

        always @ (*)
        begin
            if (select==0)
            begin
                w1 <= data1_reg;
                w2 <= data2_reg;
                w3 <= data3_reg;
            end
            else
            begin
                w1 <= data4_reg;
                w2 <= data5_reg;
                w3 <= data6_reg;
            end
        end

    endmodule

The RTL data structure for the above Verilog looks like::

    RTLModule *rtl = new
        RTLModule("bitwise_AND_no_op_bitwise_OR_2to1mux_32bit");
    rtl->addIn("clk");

    RTLSignal *select = rtl->addIn("select");

    rtl->addParam("WIDTH", "32");
    RTLWidth *width = new RTLWidth("WIDTH-1");
    std::map<int, RTLSignal*> inputs;
    for (int i = 1; i <=6; i++) {
        std::string name = "data" + utostr(i);
        RTLSignal *in = rtl->addIn(name, width);

        RTLSignal *reg = rtl->addReg(name + "_reg", width);
        reg->connect(in);
        inputs[i] = reg;
    }
    RTLSignal *dataout = rtl->addOutReg("dataout", width);

    RTLOp *cond_zero = new RTLOp(RTLOp::EQ);
    cond_zero->setOperand(0, select);
    cond_zero->setOperand(1, new RTLConst("0"));

    RTLOp *cond_one = new RTLOp(RTLOp::EQ);
    cond_one->setOperand(0, select);
    cond_one->setOperand(1, new RTLConst("1"));

    RTLSignal *w1 = rtl->addWire("w1", width);
    w1->addCondition(cond_zero, inputs[1]);
    w1->addCondition(cond_one, inputs[4]);

    RTLSignal *w2 = rtl->addWire("w2", width);
    w2->addCondition(cond_zero, inputs[2]);
    w2->addCondition(cond_one, inputs[5]);

    RTLSignal *w3 = rtl->addWire("w3", width);
    w3->addCondition(cond_zero, inputs[3]);
    w3->addCondition(cond_one, inputs[6]);

    // Note: you can pass an instruction to RTLOp's constructor
    RTLOp *op_and = new RTLOp(RTLOp::And);
    op_and->setOperand(0, w1);
    op_and->setOperand(1, w2);

    RTLOp *op_or = new RTLOp(RTLOp::Or);
    op_or->setOperand(0, op_and);
    op_or->setOperand(1, w3);

    dataout->connect(op_or);

    // to print out verilog
    Allocation *allocation = new Allocation(&M);
    allocation->addRTL(rtl);
    VerilogWriter *writer = new VerilogWriter(Out, allocation);
    writer->printRTL(rtl);

Signal Truncation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To get the lower 32 bits of a 64 bit signal::

    RTLOp *lower = rtl->addOp(RTLOp::Trunc);
    lower->setCastWidth(RTLWidth("32"));
    lower->setOperand(0, signal_64);

To get the upper 32 bits of a 64 bit signal use a shift followed by
the truncation above::

    RTLOp *shift = rtl->addOp(RTLOp::Shr);
    shift->setOperand(0, signal_64);
    shift->setOperand(1, new RTLConst("32"));
    RTLOp *upper = rtl->addOp(RTLOp::Trunc);
    upper->setCastWidth(RTLWidth("32"));
    upper->setOperand(0, shift);

Alternatively you can use the truncation operator directly::

    RTLOp *upper = rtl->addOp(RTLOp::Trunc);
    upper->setCastWidth(RTLWidth("63", "32"));
    upper->setOperand(0, signal_64);

GenerateRTL 
+++++++++++++

``GenerateRTL`` uses the scheduling and binding algorithms to generate the
final RTL data structure for the synthesized circuit.

VerilogWriter
++++++++++++++

``VerilogWriter`` prints an ``RTLModule`` as Verilog, the memory controller,
testbench, and required avalon signals.


SDC-Based Scheduling
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The scheduler returns a ``FiniteStateMachine`` object for each LLVM function.

``FiniteStateMachine`` stores ``State`` objects in a doubly-linked list.  The
``State`` class stores a sequential list of instructions and the next state
transitions.

The ``SchedulerDAG`` class creates an InstructionNode for each instruction and
computes memory and data dependencies. ``InstructionNodes`` also store the
propogation delay of the instruction.  The ``SchedulerMapping`` class maps
``InstructionNodes`` to control steps.

The SDC scheduler is based on the formulation described in [Cong06]_.
Scheduling is formulated mathematically, as a system of equations to be solved.
The formulation is a linear program (LP) that can be solved in polynomial time.
SDC stands for System of Difference Constraints.  All of the constraints in
the LP have the form::

    x1 - x2 REL y 

where REL is a relational operator: EQUALS, LESS THAN OR EQUAL TO, GREATER THAN OR EQUAL TO.  
Constraints, in this form, are "difference constraints", hence the name SDC.
We use the lpsolve open source linear system solver. See `lpsolve <http://lpsolve.sourceforge.net/>`_.

The advantage of SDC is its flexibility: different styles of scheduling,
with different types of constraints, can all be elegantly rolled into the
same mathematical formulation.  By using SDC-based scheduling within LegUp,
we bring its scheduler closer to state-of-the-art.

Each of the supported devices (e.g. Cyclone II, Stratix III, etc) has a default
clock period constraint, which we have determined as reasonable to
produce good wall-clock times for a basket of programs.
Chaining of operators in a
cycle is permitted, within the clock period constraint limits.  

A highly relevant constraint i found in the relevant device-specific file, such as 
``boards\CycloneII\CycloneII.tcl``.  This sets the target clock period for LegUp HLS, used in SDC scheduling:

  * **CLOCK_PERIOD**: Setting this parameter to a particular integer
    value in ns will set the clock period constraint.

The ``examples/legup.tcl`` file sets the following parameters which control
the SDC scheduler:

  * **SDC_NO_CHAINING**: Disable chaining of operations in a clock cycle.
    This will achieve the maximum amount of pipelining.  The **CLOCK_PERIOD**
    parameter is useless when this is set.
  * **SDC_ALAP**: Perform as-late-as-possible (ALAP) scheduling instead of
    as-soon-as-possible (ASAP).
  * **SDC_DEBUG**: Cause debugging information to be printed from the
    scheduler.

Relevant source files for SDC scheduling: SDCScheduler.h and
SDCScheduler.cpp.  In the .cpp file, start by looking at the createMapping()
method, which is the top-level method that implements the flow of SDC
scheduling.


Known Issues with SDC Scheduler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 * Doesn't support global scheduling across basic block boundaries
 * Instructions from different basic blocks can never be in the same state

Binding 
---------

Binding uses the libhungarian-v0.1.2 library to solve bipartite weighted matching.
This is the problem of finding the optimal assignment (assigning a set of jobs
to a set of machines) in O(n^3), where n=max{#jobs, #machines}.
Bipartite weighted matching is used to minimize the number of operations that
share a functional unit.

Pattern Sharing Introduction
+++++++++++++++++++++++++++++

In the Legup 1.0 release, which targeted Cyclone II, Binding shared
only dividers and remainders. 

Binding has been modified to share other types of operations, as well 
as larger computational patterns. This was shown to reduce area on Stratix IV.

Enabling and Disabling Pattern Sharing
++++++++++++++++++++++++++++++++++++++++

The ``examples/legup.tcl`` file sets the following parameters which control
pattern sharing:

.. code-block:: tcl

    # Maximum chain size to consider. Setting to 0 uses Legup 1.0 original
    # binding
    # SET TO 0 TO DISABLE PATTERN SHARING
    set_parameter PS_MAX_SIZE 10

    # The settings below should all be nonzero, but can be disabled when
    # debugging
    # if set, these will be included in patterns and shared with 2-to-1 muxing
    set_parameter PATTERN_SHARE_ADD 1
    set_parameter PATTERN_SHARE_SUB 1
    set_parameter PATTERN_SHARE_BITOPS 1
    set_parameter PATTERN_SHARE_SHIFT 1

Setting PATTERN_SHARE_ADD, PATTERN_SHARE_SUB, PATTERN_SHARE_BITOPS and PATTERN_SHARE_SHIFT will
share these operations when constructing computational patterns.
Note that all 4 should be set when sharing for best results, but 
the parameters provide a means for debugging. Setting these four parameters
all to 0 also results in the original LegUp Binding (equivalent to setting
PS_MAX_SIZE to 0). However PS_MAX_SIZE takes precedence, so for example 
even if PATTERN_SHARE_ADD is set to 1, if PS_MAX_SIZE = 0 then LegUp
original Binding will be active. i.e. both these examples will bind as in LegUp 1.0:

.. code-block:: tcl

    set_parameter PS_MAX_SIZE 0

    set_parameter PATTERN_SHARE_ADD 1
    set_parameter PATTERN_SHARE_SUB 1
    set_parameter PATTERN_SHARE_BITOPS 1
    set_parameter PATTERN_SHARE_SHIFT 1


.. code-block:: tcl

    set_parameter PS_MAX_SIZE 1

    set_parameter PATTERN_SHARE_ADD 0
    set_parameter PATTERN_SHARE_SUB 0
    set_parameter PATTERN_SHARE_BITOPS 0
    set_parameter PATTERN_SHARE_SHIFT 0

Writing Patterns to DOT and Verilog Files
++++++++++++++++++++++++++++++++++++++++++

Patterns found can also be written to .dot and .v files. 

Setting the PS_WRITE_TO_DOT parameter to be nonzero will save all patterns of size > 1
to .dot files, and then convert these to .pdf files so that patterns may be
visualized.  The file name includes the pattern size and the frequency of
occurrence. 

The Graphviz graph visualization software can be downloaded from:
`<http://www.graphviz.org/Download.php>`_

Similarly, for experimental purposes, it is possible to
create a verilog module for each pattern, by setting the
PS_WRITE_TO_VERILOG parameter nonzero. This creates a .v file
for that specific pattern with the same filename as the
.dot and .pdf files.

To avoid writing patterns of any frequency to these files, 
the parameter FREQ_THRESHOLD lets only patterns shared with 
frequency greater than or equal to this threshold to
be written to dot, pdf or verilog files. 

The dot, pdf and verilog, files will be created in folders
created for each function (given the function name).

.. _loop_pipelining:

Loop Pipelining
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To turn on loop pipelining use the tcl command :ref:`loop_pipeline`.
For working examples that use loop pipelining, look in ``legup/examples/pipeline/``.

Loop pipelining supports simple array array based dependencies like::
    
    loop: for (i = 1; i < N; i++) {
        a[i] = a[i-1] + 2
    }

Loop pipelining also supports resource constraints (for instance dual port
memories).  To avoid being constrained by global memory ports we highly recommend
turning on :ref:`LOCAL_RAMS` when loop pipelining to increase memory bandwidth.

LegUp can only pipeline loops with a single basic block.  If there is control
flow inside the loop then please turn on if-conversion using the
:ref:`set_combine_basicblock` tcl command. Or you can manually convert the if statements
into sequential code using the C ternary operator "?:".

Modulo scheduling rearranges the operations from one iteration of the loop into a schedule that
can be repeated at a fixed interval without violating any data dependencies or
resource constraints.  This fixed interval between starting successive
iterations of the loop is called the *initiation interval* (II) of the
loop pipeline.  
The best pipeline performance and hardware utilization is achieved with an II
of one, meaning that successive iterations of the loop begin every cycle,
analogous to a MIPS processor pipeline. 

If we are pipelining a loop that contains neither resource constraints
or cross-iteration dependencies then the initiation interval will be one.
Furthermore, in this case we can use a standard scheduling approach, which will correctly schedule
the loop into a feed-forward pipeline. However, when the loop does contain
constraints then the initiation interval may have to be greater than one.
For instance, if two memory operations are required in the loop body but only a single memory
port is available then the initiation interval must be two. In this case,
modulo scheduling will be required because standard scheduling has no concept
of an initiation interval. Standard scheduling assumes that operations
from separate control steps do not execute in parallel when satisfying resource
constraints, which is no longer true in a loop pipeline.
For instance, the standard approach may schedule the first memory operation in
the first time step and the second memory operation in the third time step,
but if new data is entering the pipeline every two cycles then these memory
operations will occur in parallel and conflict with the single memory port.

The LegUp loop pipelining implementation is in the following source files:
     * :file:`llvm/lib/Transforms/LegUp/LoopPipeline.cpp`
     * :file:`llvm/lib/Transforms/LegUp/ModuloScheduler.cpp`
     * :file:`llvm/lib/Transforms/LegUp/SDCModuloScheduler.cpp`
     * :file:`llvm/lib/Transforms/LegUp/ElementaryCycles.cpp`
     * :file:`llvm/lib/Transforms/LegUp/SDCSolver.cpp`

Please see our paper for more details on the loop pipelining algorithm:
    * A. Canis, J.H. Anderson, S.D. Brown, "Modulo SDC Scheduling with
      Recurrence Minimization in High-Level Synthesis," IEEE International
      Conference on Field-Programmable Logic and Applications (FPL), Munich,
      Germany, September 2014.
    * http://legup.eecg.utoronto.ca/pipelining_fpl_2014.pdf

After running loop pipelining please see the end of report file ``pipelining.legup.rpt`` for
details on the final initiation interval (II) of the pipeline and the schedule.
The loop pipelining pass annotates the LLVM IR with metadata.

For example, given the loop in ``legup/examples/pipeline/simple/simple.c``::

    #define N 4
    loop: for (i = 0; i < N; i++) {
        printf("Loop body\n");
        printf("a[%d] = %d\n", i, a[i]);
        printf("b[%d] = %d\n", i, b[i]);
        c[i] = a[i] + b[i];
        printf("c[%d] = %d\n", i, c[i]);
    }

After running "make", we can look at a snippet from the bottom of ``pipelining.legup.rpt``::

    II = 1
    Final Modulo Reservation Table:
    FuName: main_0_a_local_mem_dual_port
    time slot: 0
       issue slot: 0 instr:   %15 = load volatile i32* %scevgep4, align 4, !tbaa !3
       issue slot: 1 instr:   %11 = load volatile i32* %scevgep4, align 4, !tbaa !3
    FuName: main_0_b_local_mem_dual_port
    time slot: 0
       issue slot: 0 instr:   %16 = load volatile i32* %scevgep3, align 4, !tbaa !3
       issue slot: 1 instr:   %13 = load volatile i32* %scevgep3, align 4, !tbaa !3
    FuName: main_0_c_local_mem_dual_port
    time slot: 0
       issue slot: 0 instr:   store volatile i32 %17, i32* %scevgep2, align 4, !tbaa !3
       issue slot: 1 instr:   %18 = load volatile i32* %scevgep2, align 4, !tbaa !3

We see that the initiation interval is 1, which means we can start a new iteration of this loop
every clock cycle. There are resource constraints caused by each dual-ported
(two issue slots) local memory: a, b, c.

We can also see the final pipeline schedule:: 

    Pipeline Table:
    Total pipeline stages: 4
    Stage:        0              1              2              3
       II:        0     |        0     |        0     |        0
     Time:        0     |        1     |        2     |        3
                 %9             %9             %9             %9
          %scevgep2      %scevgep2      %scevgep2      %scevgep2
          %scevgep3      %scevgep3      %scevgep3      %scevgep3
          %scevgep4      %scevgep4      %scevgep4      %scevgep4
           <badref>       <badref>       <badref>       <badref>
                %10            %10            %10            %10
                %11            %11            %11            %11
                  -            %12            %12            %12
                %13            %13            %13            %13
                  -            %14            %14            %14
                %15            %15            %15            %15
                %16            %16            %16            %16
                  -            %17            %17            %17
                  -       <badref>       <badref>       <badref>
                  -              -            %18            %18
                  -              -              -            %19
                %20            %20            %20            %20
          %exitcond      %exitcond      %exitcond      %exitcond
           <badref>       <badref>       <badref>       <badref>

In this table, the time (clock cycle) is increasing left to right. The first column (time=0 cycles)
contains every instruction from the loop body that will occur in the first clock cycle after starting the pipeline.
We see that every instruction is repeated after every clock cycle (since II=1) and
each successive instruction corresponds to the next iteration of the loop.
However, some instructions (%19) don't start their first loop iteration until the 4th cycle of the pipeline.
In this case, after 4 cycles the loop pipeline is in *steady state* and the
instructions in the last column of this table will repeat for each successive iteration until the loop pipeline is done.
Some instructions don't have a short label (store or calls) and are marked by <badref> in the table.

We can also look at a snippet of LLVM IR in the loop body in ``simple.ll``:

.. code-block:: llvm

    %18 = load volatile i32* %scevgep2, align 4, !tbaa !4,
          !legup.pipeline.start_time !8, !legup.pipeline.avail_time !9,
          !legup.pipeline.stage !8

    !9 = metadata !{metadata !"3"}
    !8 = metadata !{metadata !"2"}

This load instruction is starts at cycle 2 and finishes at cycle 3 and is in pipeline stage 2 of this pipeline.
This means that assuming the loop pipeline starts right away, this load will start loading after 2 cycles
and will keep loading every cycle after that until the loop pipeline is finished. 

The terminator instruction holds more useful information regarding the pipeline:

.. code-block:: llvm

  br i1 %exitcond, label %.preheader.preheader, label %legup_memset_4.exit,
      !legup.pipelined !2, !legup.II !2, !legup.totalTime !10, !legup.maxStage !9,
      !legup.tripCount !10, !legup.label !11, !legup.pipeline.start_time !3,
      !legup.pipeline.avail_time !3, !legup.pipeline.stage !3

    !2 = metadata !{metadata !"1"}
    !3 = metadata !{metadata !"0"}
    !9 = metadata !{metadata !"3"}
    !10 = metadata !{metadata !"4"}
    !11 = metadata !{metadata !"loop"}

This basic block is pipelined (``legup.pipelined`` is 1). The initiation
interval (II) is 1. The total number of timesteps in the pipeline is 4. The
maximum stage is 3, so there are 4 stages (stages are indexed from 0).  The
tripcount (number of iterations) of the loop is 4. The label of the loop is
"loop". The pipeline time step that the branch has been scheduled to is 0 and
its pipeline stage is 0 (this can be ignored).

The ``GenerateRTL.cpp`` file handles the construction of the loop pipeline in
the function ``generateAllLoopPipelines()``. The FSM of the original function is modified
so that there is a state waiting for the loop pipeline hardware to complete,
looking at a snippet of the ``scheduling.legup.rpt`` file:

.. code-block:: none

    state: LEGUP_loop_pipeline_wait_loop_1_16
       %9 = phi i32 [ %20, %legup_memset_4.exit ], [ 0, %legup_memset_4.exit.preheader ], !legup.canonical_induction !2, !legup.pipeline.start_time !3, !legup.pipeline.avail_time !3, !legup.pipeline.stage !3 (endState: LEGUP_loop_pipeline_wait_loop_1_16)
       br i1 %exitcond, label %.preheader.preheader, label %legup_memset_4.exit, !legup.pipelined !2, !legup.II !2, !legup.totalTime !10, !legup.maxStage !9, !legup.tripCount !10, !legup.label !11, !legup.pipeline.start_time !3, !legup.pipeline.avail_time !3, !legup.pipeline.stage !3
       Transition: if (loop_1_pipeline_finish): LEGUP_F_main_BB_preheaderpreheader_17 default: LEGUP_loop_pipeline_wait_loop_1_16

The state ``LEGUP_loop_pipeline_wait_loop_1_16`` waits until the signal
``loop_1_pipeline_finish`` is asserted before continuing to the next state.

The pipeline has the following control signals (all active high):
  * loop_1_pipeline_start: starts the pipeline. Should not be asserted if the pipeline
    is running
  * loop_1_pipeline_finish: the pipeline is finished and all computation is complete
  * loop_1_valid_bit_*: the valid_bit signals form a shift register:
    ``loop_1_valid_bit_1 <= loop_1_valid_bit_0`` etc.
    Every time new data enters the pipeline a 1 is shifted into loop_1_valid_bit_0.
    If the valid bit is high for a time step then the input data is valid and that
    pipeline step can be performed
  * loop_1_ii_state_*: this is a counter from 0 to II-1. This counter is only
    needed for pipelines with an initiation interval greater than 1
  * loop_1_epilogue: no new data, the pipeline is being flushed
  * loop_1_i_stage*: the value of the induction variable (i) at each pipeline stage

Most operations in the pipeline will use both the loop_1_ii_state and
loop_1_valid_bit to determine when to execute. For example:

.. code-block:: v

    always @(posedge clk) begin
	if ((~(memory_controller_waitrequest) & ((loop_1_ii_state == 1'd0) & loop_1_valid_bit_3))) begin
		main_legup_memset_4exit_18_reg <= main_legup_memset_4exit_18;
	end
    
If a value calculated in one pipeline stage is used in a later pipeline stage, then LegUp will
insert extra registers to store the value. We must create a register
for every pipeline stage that must be crossed. These registers will be named
_reg_stage* in the output Verilog.
For example, if another operation needs to use
``main_legup_memset_4_exit_scevgep6`` in stage 2 then the register
``main_legup_memset_4_exit_scevgep6_reg_stage2`` would be used.

LegupConfig 
------------

``LegupConfig`` is an Immutable LLVM pass that can read LegUp .tcl files.
For instance, to read the functions that should be accelerated.


PreLTO 
--------

``PreLTO`` pass computes the new size for memset and memcpy when applied to structs.
The pass is needed because struct lengths may be different.


LLVM  
------

Alias Analysis 
+++++++++++++++

*Alias analysis*, or memory disambiguation, is the problem of 
determining when two pointers refer to overlapping memory locations.
An *alias* occurs during program execution when two or more pointers
refer to the same memory location.
*Points-to analysis*, a closely related problem, determines which memory locations a pointer can reference.
Solving the alias and points-to analysis problems require us to know the values of all
pointers at any state in the program, which makes this an undecidable
problem in general. 

Points-to analysis algorithms are categorized by flow-sensitivity and context-sensitivity.
An approach is flow-sensitive if the control flow within the given procedure is
used during analysis while a flow-insensitive approach ignores instruction execution order.
Context-sensitive analysis considers the possible calling contexts of a
procedure during analysis.
Points-to analysis can either be confined to a single function, called intraprocedural,
or applied to the whole program, called interprocedural.
Points-to analysis algorithms have varying levels of accuracy and may be overly conservative,
but for programs without dynamic memory, recursion, and function pointers, most pointers are resolvable at compile-time.

The compiler community has developed fast interprocedural flow-insensitive and
context-insensitive algorithms. 
Andersen described the most accurate of these approaches, which
formulates the points-to analysis problem as a set of inclusion constraints for
each program variable that are then solved iteratively.
Steensgaard presented a less accurate points-to analysis, which used
a set of type constraints modeling program memory locations that can be solved in linear-time.
In LegUp, we use the points-to analysis described by Hardekopf,
which speeds up Andersen's approach by detecting and removing cycles that can occur in the
inclusion constraints graph.
We used the code from:
    * https://code.google.com/p/addr-leaks/wiki/HowToUseThePointerAnalysis
The algorithm is described in the paper:
    * Ben Hardekopf and Calvin Lin. The ant and the grasshopper: fast
      and accurate pointer analysis for millions of lines of code. In
      Proceedings of the 2007 ACM SIGPLAN conference on Programming language
      design and implementation (PLDI '07). New York, NY, USA, 290-299.


To aid pointer analysis, the C language now includes a pointer type qualifier keyword,
*restrict*, allowing the user to assert that memory accesses by the pointer do
not alias with any memory accesses by other pointers.

In LegUp, the alias analysis implementation occurs in two stages run by *Allocation::runPointsToAnalysis()*.
First, we analyse the LLVM IR and set up a constraints problem in the
*PADriver* class. Next we solve that constraints problem in the class *PointerAnalysis*.
If there are bugs in the alias analysis they are likely within the *PADriver* class. These bugs occur
when LegUp doesn't add a constraint caused by a particular LLVM instruction.

LegUp uses this alias analysis information to determine which memories a particular load/store instruction
can access. If a memory is only accessed within a single function, we call that a local memory and create
a block RAM inside the corresponding hardware module for that function.
For alias analysis debugging information please see the *memory.legup.rpt* report file.
The end of this file will contain information about which memory is local to a
particular function or in global shared memory:

.. code-block:: none

    Final memory allocation:
    Global Memories:
            RAM: float_exception_flags
            ROM: countLeadingZeros32_countLeadingZerosHigh
    Local Memories:
            ROM: test_in Function: main
            ROM: test_out Function: main

You can force a memory to be global or local using the tcl commands
:ref:`set_memory_global` or :ref:`set_memory_local`.

Alias analysis is also required for determining dependencies between load/store/call instructions.
These instruction are not connected by a use-def chain like other LLVM instructions.
In the worst case, without alias analysis, we must perform these memory
instructions sequentially to avoid memory hazards.

For example:

.. code-block:: llvm

    store %a, 10
    %b = load %a

This store and load have a read after write data dependency. 
The store must occur before the load, which means these instructions cannot be
performed in parallel 
The LLVM ``MemDep`` analysis pass gives the dependencies between load/store/call
instructions.  Mod/Ref means modify/refer.


LLVM Intrinsics 
++++++++++++++++

The CHStone gsm benchmark requires the LLVM intrinsic function memset.i64().
By using the lowerIntrinsics function from CBackend we can turn this call into
a memset() but we can't lower that.
Even with -ffreestanding gcc requires: memcpy, memmove, memset, memcpy.

To handle this we create a custom intrinsic C functions defined in: 
  * ``examples/lib/llvm/liblegup/`` - source files
  * ``examples/lib/include/legup/intrinsics.h`` - header file

These functions are compiled into an .a archive, which is linked with every
Legup example.


Tips/Tricks 
-------------

Compiling 
++++++++++

To quickly compile only ``llc`` after modifying a file in ``llvm/lib/Target/Verilog/``::

    # llvm/utils must be on your path
    makellvm llc 

Debugging Segfaults 
++++++++++++++++++++

To debug segfaults in ``llc`` first make sure you have compiled a debug build.
Do this by uncommenting the following line in ``Makefile`` and rerunning ``make``::

    #DEBUG_MODE = --disable-optimized

Then either update LLVM_BUILD in either ``examples/Makefile.config`` or your environment::

    LLVM_BUILD=Debug+Asserts

Then use gdb::

    > gdb llc
    (gdb) run -march=v array.bc

To see DEBUG() print statements use the -debug flag::

    llc -march=v -debug array.bc


Debugging RTL generated by LegUp 
++++++++++++++++++++++++++++++++++

Printf 
~~~~~~~
The easiest way to debug in Legup is to use C printf statements
which will translate to Verilog **$display** statements which will
print to the terminal when simulating the circuit in Modelsim.

.. _watch:

make watch 
~~~~~~~~~~~
To try make watch run::

    cd array
    make watch

If your hardware is correct 'make watch' will give a diff that returns nothing::

    diff -q lli.txt sim.txt

``make watch`` does the following:
  #. Creates an annotated LLVM IR by adding a printf instruction at the end of
     every basic block that prints the current value of all registers modified
     in that basic block
  #. Runs Legup on this annotated LLVM code to generate Verilog with $display
     statements at the end of each basic block
  #. Simulates the Verilog with Modelsim, which will print out the state of registers
     as the program executes
  #. Runs the annotated LLVM with the LLVM interpreter (``lli``) 
  #. diffs the two outputs to verify that the values of the registers are the
     same between software and hardware


Presently, the order the basic blocks are executed is identical when
running in software or hardware, in the future this will change as
basic blocks start to run in parallel. This will break this debugging
method as the order of the basic block execution will be non-deterministic.

One caveat, registers that contain addresses to memory are not compared,
because the software version of the code will have different addresses than the
hardware.  In some cases LLVM will cast a pointer to an integer, making it hard
to identify that the register actually stores an address and this will lead to
a false mismatch.


LegUp Quality of Results 
--------------------------

To determine the LegUp quality of results we use the `CHStone benchmark suite
<http://www.ertl.jp/chstone/>`_ and Dhrystone.  These are tracked on our
`quality of results page
<http://www.legup.org:9100/perf/dashboard/overview.html>`_.

The horizontal axis shows the git revision, the rightmost being the latest
revision.  Click on a graph to zoom in, and click on a particular revision to
view the git log message for that revision.  Latency metrics are from a
functional simulation using Modelsim. Area and fmax is provided from Quartus
after place and route.


.. [Cong06] 
    J. Cong and Z. Zhang, “An Efficient and Versatile Scheduling Algorithm
    Based On SDC Formulation,” Proceedings of the 2006 Design Automation
    Conference, San Francisco, CA, pp. 433-438, July 2006.

