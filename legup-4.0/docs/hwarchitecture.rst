.. highlight:: c

.. _hwarch:

Hardware Architecture
======================

This section will explain the architecture of the hardware produced by LegUp when
synthesizing C into Verilog.

Modules
--------

Each C function corresponds to a Verilog module. For instance, the following C
prototype::

    int function(int a, int* b);

Would generate a module with the following interface:

.. code-block:: v

    module function
        input clk;
        input reset;
        input start;
        output reg finish;

        output reg [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] memory_controller_address;
        output reg  memory_controller_enable;
        output reg  memory_controller_write_enable;
        input memory_controller_waitrequest;
        output reg [`MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_in;
        input [`MEMORY_CONTROLLER_DATA_SIZE-1:0] memory_controller_out;

        input [31:0] a;
        input [`MEMORY_CONTROLLER_ADDR_SIZE-1:0] b;

        output reg [31:0] return_val;
    endmodule

    
The start/reset signals are used by the first state of the state machine:

.. image:: /images/first_state.*

The finish signal is kept low until the last state of the state machine, where
finish is set to 1 when waitrequest is low.
Memory signals to the memory controller are shown below. These memory ports
are only created if the memory controller exists in the circuit. This is 
described in more detail later in this document.

==============================  =============================
Memory signal                   Description
==============================  =============================
memory_controller_address       32-bit address of memory                          
memory_controller_enable        enable reading/writing this cycle                 
memory_controller_write_enable  if 1 then write, else read                        
memory_controller_waitrequest   if this is 1 then hold the state machine constant 
memory_controller_in            data being read from memory                       
memory_controller_out           data to write into memory                         
==============================  =============================


Function parameters are provided by ports a (integer), and b (pointer). The
return_val port passes back the function return value.

The module instantiation hierarchy is dependent on the call graph of the C
code. For instance, with the function call graph shown by:

.. image:: /images/call_graph.*

Module instantiation hierarchy is shown by:

.. image:: /images/call_hier.*

Note that module **c** is instantiated twice.

Memory architecture
--------------------

LLVM has three types of memory: stack, globals, and the heap. We can ignore the
heap because Legup does not support dynamically allocated memory. The stack is
used for local variables but there is no equivalent of a stack in hardware.

In Legup, there exists 4 hierachies of memories: 1) Local memory, 2) global memory,
3) cache memory, and 4) off-chip memory. 1 and 2 are implemented inside a 
hardware accelerator for data local to the accelerator, 
3 and 4 are shared between all hardware accelerators and the 
processor. In the pure hardware case, where the
processor doesn't exist in the system, only the first two 
levels of memories exist. 

LegUp uses points-to analysis to determine which arrays are used by which 
functions. If an array is determined to be used by a single function, that array
is implemented as a local array, which is created and connected directly  
inside the module that accesses it. If an array is accessed by multiple functions
or if the points-to analysis cannot determine the exact array for a pointer, the
array is implemented as a global array, which is created inside the memory
controller. The memory controller, which is described below, allows memory 
accesses to be steered to the correct array at runtime. 

By default, each local/global memories are stored in a separate block RAM. 
This allows all local memories to be accessed in parallel. For global memories
inside the memory controller, they are limited to 2 memory accesses per cycle, 
due to RAMs being dual-ported memories. However, storing them in separate RAMs
can help for debugging, as it is easier for the hardware designer to debug 
if he/she sees individual arrays from his C program in separate rams rather than 
buried in a large ram. 
There is an option to merge memories in the memory controller to single RAMs,
in order to reduce the RAM usage. This can be done by turning on the
GROUP_RAMS tcl parameter. 

Local Memories
------------------

Any memory that is used by a single function is designated as a local memory, 
and is created inside the module being used. There can be many local memories 
inside a module and they can all be accessed in parallel, since they are 
implemented in separate block RAMs. Since local memories are connected directly
in a module, they do not require expensive multiplexing, as in the case of 
global memories. This helps to reduce area and improve Fmax. Also they do not
connect through the memory controller ports shown earlier but connect directly 
through wires inside the module. 

All local memories are implemented in dual-ported RAMs and have a 
latency of 1 clock cycle. 


Global Memories
------------------

Any memories which are used by multiple functions, or if the points-to analysis
fails to determine the exact array for a pointer, that array is designated as a 
global array, and is created inside the memory controller.
Each global memory is identified by a unique number called a tag. 
We first describe below the address format for global memories below. 

32-bit Address Format:

.. image:: /images/memory_architecture.*


The upper 8 bits of memory addresses are reserved for tag bits, allowing
255 memory locations. The tag bits are used at circuit runtime to steer each 
memory access into the memory controller to the correct RAM, or to the processor
memory. 
Tag 0x0 is reserved for null pointers. Tag 0x1 is
reserved for processor memory. The 24 bit address allows a 16MB
byte-addressable address space. Because the lower bits are used for the
pointer address, this scheme allows pointer arithmetic, incrementing the
address won't affect the tag bits. 

For instance:

Inside the top level module the tag bits are used to steer the memory accesses
to either the memory controller or the processor memory. 
The following figure shows memory
accesses from top-level module:

.. image:: /images/top_level.*

All global memories are implemented in dual-ported memoires and have a 
2 cycle latency. 

Memory Controller
------------------

We describe the memory controller architecture below. 

.. image:: /images/mem_ctrl.*

The memory controller is a ram composed of smaller rams. We need a memory
controller to share memory between modules and to handle pointer aliasing
within the same module. We need the tag bits because at compile time you may 
not be able to calculate exactly which pointers point to the memory and 
that no other pointer ever points to that piece of memory. 
So the memory controller is a central place to handle aliasing. The memory 
controller is only created if there are memories shared between functions, 
or if the points-to analysis fails to determine the exact memory for 
all pointers in the program.
In the figure, mem_data_out width is the max data width of
all RAMs in the memory controller. The size of pointers is currently fixed at
32 bits. 

The latency of reading from a RAM
is one cycle, so we must use the previous tag to determine which
RAM is outputting the data requested in the previous cycle. We
registered the output of the memory controller to improve Fmax as the steering
mux can become large. Note that for tags 0 and 1, mem_dat_out keeps its old
value.

The mem_waitrequest signal is not shown here and for the pure hardware case 
it is always given a 0 value. 
If mem_waitrequest equals 1 then the memory controller is
indicating it will take longer to retrieve the memory. As long as
mem_waitrequest is high the memory is not ready. After mem_waitrequest goes low
then the data will be available on the next cycle. This is important for the
processor memory which can take many cycles if there is a cache miss. In every
state machine that legup generates the state will not change if mem_waitrequest
is high.

LegUp also handles structs. In a struct, the
individual elements can have non-uniform size. Also structs must be byte
addressable. To handle this we need an additional 2-bit input **mem_size** which
indicates the size of the struct element we are accessing. mem_size is 0 for
byte, 1 for short, 2 for integer, 3 for long. For each struct a 64-bit wide ram
is instantiated. Using the mem_addr and mem_size we can use the byte enable of
the ram to only write the correct section of the ram. When reading data, we
must steer the correct bits of the 64-bit word to the lowermost bits of
mem_data_out.

If an array is initialized in the C code, we create a MIF, memory initialization 
file, for that RAM. 

Currently, only one memory controller module may be created for a program. 
Each module must communicate through its parent
module to get to the memory controller. Hence, there are muxes at each level of
the hierarchy as shown in the figure:

.. image:: /images/call_graph_mux.*

For instance, in the main module we are either in the body of the main
function, in 'a', or in 'b', so we need a 3-1 mux. Since we do not allow
recursion, the call graph will always be a tree. Note that the further down on
the call graph there is more delay to the memory controller.


Function Calls
---------------

Every function call requires two states. An initial state to set start=1 for
the called function, then a second state that loops until receiving a finish=1
from the called function. Function calls are not allowed in the same state as a
memory load/store.

Signed/Unsigned
---------------

In LLVM, all integers are assumed to be unsigned unless passed to a signed
instruction (sdiv, srem). Since integers are unsigned, before being passed to
an add operation they must be appropriately sign or zero extended. To deal with
sign extension LLVM has two instructions: sign extend (sext) and zero extend
(zext), which both result in an unsigned integer. However, Verilog operations
such as +/- depend on the type of the operands, which can be 'signed' or
'unsigned'.

In LegUp, we declare every Verilog variable as unsigned and use the $signed()
Verilog command when required by an instruction such as sdiv, srem, or sext.

Mult-dimensional Arrays
------------------------

Multi-dimensional arrays are stored in row-major order, the same convention
used by C. For instance given an array::

    int array[2][2][2] = {{{0, 1}, {2, 3}}, {{4, 5}, {7, 8}}}

If we assign variables for the size of each dimension of the array[A][B][C]
where A=2, B=2, C=2. Then to access the element array[a][b][c] the memory
offset is given by::

     offset = c + C*b + C*B*a = c + C*(b + B*a)

This supports storing an array of arbitrary dimension in a ram the same width
as an element with A*B*C rows.

Functional Units
-----------------

To keep Fmax high, we pipelined dividers/remainders and multipliers. 
The pipeline depth of Dividers/Remainders are equal the bit width of the 
operation. Multipliers have a pipeline depth of 2. 

We only share dividers/modulus functional units to save area.
The divider clock enable is set to 0 when the memory controller's wait_request
signal is high or when we're calling a function.

Structs
--------

Structs are supported by LegUp including pointers, arrays, structs and
primitives as elements. Pointers to structs are also supported, for example
linked lists can be synthesized.

LLVM's TargetData is used to specify alignment for structs. For instance for a
32-bit machine, pointers are 32-bits and 32-bit aligned. LLVM integers of type
i64 are 64-bit aligned. Structs are 64-bit aligned.


Avalon Signals
---------------

Each hardware accelerator contains the following Avalon signals.

.. tabularcolumns:: |p{5cm}|p{10cm}|

==============================  =============================
Avalon signal                   Description
==============================  =============================
csi_clockreset_clk              hardware accelerator clock 
csi_clockreset_reset            hardware accelerator reset
==============================  =============================

Avalon slave signals (prefixed with **avs_s1**) are used by the processor to
communicate with the hardware accelerator

.. tabularcolumns:: |p{5cm}|p{10cm}|

==============================  =========================================================================================================================================================================
Avalon signal                   Description
==============================  =========================================================================================================================================================================
avs_s1_address                  address sent from processor to hardware accelerator. Determines which accelerator argument is being written or whether the processor is giving the start signal
avs_s1_read                     processor sets high to read return value from hardware accelerator
avs_s1_write                    processor sets high to write an argument or start the processor.
avs_s1_readdata                 accelerator sets this to the return data to send back to the processor
avs_s1_writedata                processor sets this to the value of the argument being written to the accelerator
==============================  =========================================================================================================================================================================

Avalon master signals (prefixed with **avm**) which talk to the on-chip data
cache.  These signals correspond to the memory-mapped address of the data
cache. 


.. tabularcolumns:: |p{5cm}|p{10cm}|

==============================  =========================================================================================================================================================================
Avalon signal                   Description
==============================  =========================================================================================================================================================================
avm_ACCEL_address               points to the memory-mapped address of the data cache
avm_ACCEL_read                  set high when accelerator is reading from memory
avm_ACCEL_write                 set high when accelerator is writing to memory
avm_ACCEL_readdata              data returned from memory when accelerator issues a read
avm_ACCEL_writedata             on a write, it sends the data to be written to memory, as well as the memory address and the size of the data (8bit, 16bit, 32bit, 64bit)
								on a read, it sends the memory address and the size of the data (8bit, 16bit, 32bit, 64bit)
avm_ACCEL_waitrequest           asserted until the read data is received
==============================  =========================================================================================================================================================================

The on-chip data cache is a write-through cache, hence when an accelerator or
the processor writes to the cache, the cache controller also sends the data to
the off-chip main memory. 

If a memory read results in a cache miss, the cache controller will access off-chip main
memory to get the data, which will be written to the cache and also returned to
the accelerator.

For parallel execution which uses either a mutex or a barrier, the following Avalon signals are also created. This Avalon master is used to communicate with the mutex
to either lock (pthread_mutex_lock) or unlock (pthread_mutex_unlock), and also used communicate with the barrier to initialize (pthread_barrier_init) 
and poll on the barrier (pthread_barrier_wait) until all threads have reached the barrier. 

.. tabularcolumns:: |p{5cm}|p{10cm}|

==============================  =============================
Avalon signal                   Description
==============================  =============================
avm_API_address                 points to the memory-mapped address of mutex, barrier
avm_API_read                    set high when accelerator is reading from mutex, barrier
avm_API_write                   set high when accelerator is writing to mutex, barrier
avm_API_readdata                data returned from mutex, barrier when accelerator issues a read
avm_API_writedata               data written to mutex, barrier when accelerator issues a read
avm_API_waitrequest             asserted until the read data is received
==============================  =============================

