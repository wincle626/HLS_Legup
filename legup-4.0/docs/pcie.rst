.. highlight:: c

LegUp over PCIe
===============

The pcie/ directory contains working code for a new hybrid flow that uses a hard processor (x86) connected to a DE4 FPGA. The communication is done via the PCIe interface protocol. Similar to LegUp's MIPS hybrid flow, the PCIe flow allows the user to specify functions to compile into hardware accelerators.

User information
----------------

We recommend that you complete the Altera DE4 tutorial at http://www.altera.com/education/univ/materials/boards/de4/unv-de4-board.html with a DE4 to gain a better understanding on how the PCIe interface works and to physically set up the PCIe connection. You may use the code in driver/, as it is a compatible driver. The driver and initial pcie_tutorial/ design were modified from this tutorial.

Note: the following is still ongoing work, so it may not fully work as advertised. 'make pcie' requires pre-written C wrapper functions and a QSYS script to describe the system. We try to automatically generate the C wrapper and the QSYS script, but currently this is only functional for single-threaded functions that only pass by value. To compile with the generated wrappers, use 'make pcieAuto'. The examples directories contains some examples of C wrapers and QSYS scripts that allow pass by reference and multi-threaded functions, but automating these examples is still ongoing.

Once you have completed the tutorial, take a look at the vector_add example, which adds two arrays into a third array. In the vector_add example, config.tcl is used to specify that vector_add will get accelerated and synthesized onto the FPGA. Our flow supports passing in basic data types such as int len, used to pass in the array length, as well as pointers, which are used to pass the 3 arrays. The FPGA and hard processor have physically separate memory spaces, so the program must synchronize the memory to allow the accelerator to operate on a chunk of memory. We have provided the following API for memory synchronization:

.. code-block:: c

	#include "legup_mem.h"

	// Allocate a block of memory on the FPGA and return a pointer to the FPGA memory
	void * malloc_shared(size_t size, void *default_ptr); // ignore default_ptr for now
	// Free an allocated pointer to FPGA memory
	void * free_shared(void *ptr);

	// Copy data to the FPGA from the host processor
	memcpy_to_shared(void *dst_fpga_ptr, void *src_host_ptr, size_t size);
	// Copy data from the FPGA to the host processor
	memcpy_from_shared(void *dst_host_ptr, void *src_fpga_ptr, size_t size);

Note: the return value of malloc_shared() and the fpga_ptr arguments to the memcpy() functions are all pointers to FPGA memory, and do not make any sense to use on the host processor. We recommend making this explicit in the variable name by preceding with ``SHARED_MEM_`` or a similar prefix.

Once you understand what the code does, you can compile it. Type "make pcieSWAuto", which will create software executables. This produces three executables: vector_add.elf, vector_add_sw, and vector_add_memtest. vector_add.elf is the final program that will communicate with hardware accelerators. If you run this executable now, it should fail because it "Could not open device". This is because the DE4 is not yet programmed. vector_add_sw_memtest is a software-only implementation of the benchmark. It simulates the memory copying of the "legup_mem.h" functions, and its execution should mirror that of the hybrid executable. vector_add_sw is software-only simulation that does not do any copying. malloc_shared simply returns the default_ptr and the other functions do nothing. This executable is also compiled with "#define SW_ONLY" if pointer offsets need to be corrected for this version. For our work, the sw_memtest executable was very helpful to make sure our shared memory synchronization was correct and the sw executable was used as a fair software-only benchmark for performance comparison.

To generate the entire system, type "make pcie", which will also compile the software executables (ie calling "make pcieSWAuto"). This flow will:
  * Compile the software executables (make pcieSWAuto)
  * Generate Verilog for the hardware accelerators
  * Create the necessary software and hardware wrappers
  * Link the software executables
  * Generate the hardware system of accelerators, memory, and PCIe interface (make pcieVerilogbackend)
  * Compile the Quartus project (make pcieQuartus)
  * Program the Quartus project (make pcieProgram)

Once this is complete, you will need to restart your computer (ensure the DE4 remains powered on). Then, install the PCIe device driver (make pcieDriver) and then you should be able to run vector_add.elf and view the output.

Future work
-----------

We plan to support multiple threads by allowing each hardware accelerator to be duplicated, and each accessed by a single thread on the host processor program. We will add an extra configuration to specify the # of threads to match the number of generated hardware accelerators, and a method to specify a thread_id from the host processor so that it may be mapped to a unique hardware accelerator.
