

.. _debug:

Debugging
=================

LegUp includes a source-level debugger.  This allows for debugging of the produced RTL design using a software-like debugger that supports single-stepping, breakpoints, and variable inspection.  

The debugger supports the following modes of execution:

    #. **In-system interactive**: The design is implemented and running on the FPGA, and the debugger application is connected to the FPGA board.  As the user single-steps through execution, the design on the FPGA is executed in lock-step by enabling and disabling the clock to the circuit.  

    #. **In-system record and replay**.  In this mode, the user can run the design at full speed, and the circuit execution is captured into on-chip memories.  Once the circuit is stopped, the execution history can be retrieved and debugging can be performed on a replay of the execution.  (This is similar to an embedded logic analyzer, except that debugging is performed using a software-like debugger, rather than a waveform.)  Compression techniques are used, tailored to each design, to obtain long execution traces.  (See [:ref:`1 <debug-reference-goeders1>`] and [:ref:`2 <debug-reference-goeders2>`])

    #. **Simulation**.  This mode supports the same software-like debug experience as above, but instead of being connected to an FPGA, execution is performed using Modelsim.

    #. **C/RTL Verification**.  In this mode the C code is executed using gdb, and the RTL is simulated in Modelsim.  The tool automatically detects discrepancies, and reports them to the user.  (See [:ref:`3 <debug-reference-calagar>`])

More information can be found in included papers [:ref:`debug-references`].

Presently, the debugger only works with the hardware-only flow;  it does not support the hybrid flow where a processor is present.

Installation
++++++++++++

Install the necessary packages in Ubuntu::

    sudo apt-get install python3-pyqt5 \
    mysql-server libmysqlclient-dev python3-mysql.connector
        
During MySQL installation you may be prompted for a database username/password.  Set these to: ::

    username: root
    password: letmein

If you are using a different username/password, you will need to update the ``DEBUG_DB_USER`` and ``DEBUG_DB_PASSWORD`` options in ``examples/legup.tcl``.


Next you need to install pyserial.  You can try the ``python3-serial`` package in Ubuntu, but at one point it was broken. To install the package manually:
    #. Download pyserial-2.7.tar.gz from https://pypi.python.org/pypi/pyserial
    #. Extract
    #. sudo python3 setup.py install


Configuring your Design
+++++++++++++++++++++++
There are example designs already configured for debugging.  These are located in the examples/debug directory.  The following describes how to configure a new design for debugging.


Add the following to your Makefile: ::

    DEBUG_G_FLAG = 1

This will ensure the debug flag (``-g``) is enabled during Clang compilation.


If you want to perform in-system debugging, you need to add the following option: ::

    DEBUGGER = 1

This will ensure that when you create a Quartus project (``make p``), the necessary debugger RTL files are also included in the project.


If you want to perform debugging without any optimizations, as presented in [:ref:`1 <debug-reference-goeders1>`], include the following in your Makefile.  If you are debugging with optimizations, as described in [:ref:`2 <debug-reference-goeders2>`], skip this step. ::

        NO_OPT = 1
        NO_INLINE = 1
        DEBUG_KEEP_VARS_IN_MEM = 1
        

If you do not already have a ``config.tcl`` file for the design, you will need to create one, and add the following to your Makefile: ::

    LOCAL_CONFIG = -legup-config=config.tcl

Edit your ``config.tcl``, and add the following to the top of the file (if not already included): ::
    
    source ../legup.tcl
    set_project CycloneII DE2 Tiger_SDRAM    

This will configure the project for the DE2 board.  Next, add the following to your ``config.tcl`` to enable debugging.  This is required for any mode of debugging (simultion or in-system)::

    set_parameter DEBUG_FILL_DATABASE 1
    set_parameter DEBUG_DB_SCRIPT_FILE <your_legup_dir>/examples/createDebugDB.sql

If you want to debug in-system, you will also need to add the following option, which will instruct LegUp to automatically instrument the RTL file with necessary debug logic. ::

    set_parameter DEBUG_INSERT_DEBUG_RTL 1        
    

If you want to perform in-system debug, with visiblility into variables located in datapath registers, as explained in [:ref:`2 <debug-reference-goeders2>`], add the following option: ::

    set_parameter DEBUG_CORE_TRACE_REGS 1

The following trace buffer optimizations are also available, as described in [:ref:`2 <debug-reference-goeders2>`]: ::

    set_parameter DEBUG_CORE_TRACE_REGS_DELAY_WORST 1
    set_parameter DEBUG_CORE_TRACE_REGS_DELAY_ALL 1
    set_parameter DEBUG_CORE_TRACE_REGS_DUAL_PORT 1

Compiling your Design
+++++++++++++++++++++
Once configured for debugging your design can be compiled as usual: ::

    make

If you want to perform in-system debug, you will also need to generate a bitstream: ::

    make p
    make f


Using the Debugger
++++++++++++++++++

    #. *(In-system only)* Connect the DE2 board to the computer using the RS232 port.

    #. Launch the debugger from <legup-3.0>/dbg/debugger/src: ::

        python3 main.py

    #. Once launched, open the folder containing your design.

    #. *(In-system only)* On the FPGA tab, choose the /dev/ttyUSB0 port (or whatever port your serial is) and click 'Connect'.  If you have RS232 permission issues, see `here <http://askubuntu.com/questions/133235/how-do-i-allow-non-root-access-to-ttyusb0-on-12-04>`__.


.. image:: /images/debugger.*

=======  ============  ======================================================================================================================================================================================================================================================
Item #   Name          Description
=======  ============  ======================================================================================================================================================================================================================================================
1        Mode          - **FPGA Live**: In-system Interactive debugger.  Single stepping will execute the design on the FPGA in lock-step by enabling/disabling clock.
                       - **FPGA Replay**: In-system Replay.  Switching to this mode will retrieve the saved execution history from *FPGA Live* mode.  Due to on-chip memory constraints, this may not contain the entire execution history.
                       - **Simulation**: Debugging is performed without a connected FPGA. Modelsim is used to simulate circuit execution.
2        Open Design   Opens a new design.  Make sure you compile (make) before opening.
3        Refresh       Refreshes the current state.  This can be useful after reprogramming the bitstream.
4        Reset         Resets the design by asserting the reset signal.  *Note: If the design relies upon memory initialization values, resetting will not be sufficient.*
5        Play          Run the design indefinitely (until completion or a breakpoint).  *Note: Once a design completes execution it returns to its initial state.  So if you hit run without a breakpoint the design will completely execute, but you won't see any visible change to the debugger*.
6        Pause         Pauses execution. (Mostly useful during simulation)
7        Step Back     Steps back one clock cycle. This is available during *FPGA Replay* or *Simulation*.
8        Step Forward  Steps forward one clock cycle.  
9        Slider        This slider quickly moves through execution history.  This is available during *FPGA Replay* or *Simulation*.
10       Breakpoints   Double click in this area to add a breakpoint.  Breakpoints must be on lines with corresponding IR instructions.  Breakpoints have a 1 cycle delay. *Note: The breakpoint is associated with the first FSM state of all IR instructions at that line of C code.  If a line of C code has multiple IR instructions, and only a later instruction is executed, the breakpoint will not be triggered*
11       Source code   Source code.  All lines of source code which have an underlying IR instruction being executed will be highlighted in green.
12       Gantt         Gantt chart of HLS scheduling.  Each rectangle corresponds to an IR instruction.  You can click on the boxes to view the assocated IR instruction.
13       Gantt line    This red line shows the current state in the Gantt chart.
14       IR Insns      This displays the underlying IR instructions for the executing (green) instructions.  If an IR instruction in the Gantt charge is selected (blue), the selected IR instruction(s) will be shown instead.
=======  ============  ======================================================================================================================================================================================================================================================

Reading Variables
+++++++++++++++++

.. image:: /images/debugger_variables.*
    :align: center

This pane provides information on the source-code variables.  The availability of variable values depends on the mode of operation, and how the variable is mapped to FPGA resources (memory vs. register).

If all compiler optimizations are disabled, all variables will be located in on-chip memory.  

Once compiler optimizations are enabled, some variables are optimized to datapath registers, some are replaced with constants, some remain in memory, and others are completely optimized away.  (See [:ref:`2 <debug-reference-goeders2>`]).  The value of variables optimized away can never be obtained, while those that are optimized to constants are always available to the user.  Variables in on-chip memory and datapath registers are available under certain circumstances (see table below).


+-------------+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|Mode         | Variable Resource   |    Variable Availability                                                                                                                                                                                                                                              |
+=============+=====================+=======================================================================================================================================================================================================================================================================+
| FPGA Live   | On-chip memory      | Always available                                                                                                                                                                                                                                                      |
+             +---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|             | Datapath register   | Not available                                                                                                                                                                                                                                                         |
+-------------+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|FPGA Replay  | On-chip memory      | If the variable is updated (written to) during the replay period, the value is available after the first write (obtained from execution trace).  If the variable is never updated, the value is always available (obtained by reading directly from FPGA memory).     |
+             +---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|             | Datapath register   | The variable is available after the first time it is updated.                                                                                                                                                                                                         |
+-------------+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|Simulation   | On-chip memory      | The variable is available after the first time it is read/written.                                                                                                                                                                                                    |
+             +---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|             | Datapath register   | Not available (has not been implemented yet)                                                                                                                                                                                                                          |
+-------------+---------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

When reading the variable value, one of the following will be displayed.

================    ===============================================
Message             Description
================    ===============================================
Numeric value       The variable value was obtained successfully, and the decimal representation is given.
<N/A>               The variable exists in the RTL, but is not available (See table above).
<Optimized Away>    The variable has been optimized out of the RTL, due to compilzer optimizations.
<Unknown>           The variable is tracked, but at this point in the replay the variable has not been updated yet (See table above).
<Undefined>         The variable value is undefined.  This occurs for uninitialized variables.
================    ===============================================

The GUI supports displaying simple integer variables (char, short, int, etc.), and provides an interface for viewing struct and array members.  Other types of variable inspection (pointer dereferencing, etc) are not yet supported.

By default, when connected to an FPGA performing interactive debugging, variables are not automatically updated.  This is because the serial connection to the board is relatively slow, and loading the variables each cycle can slow down the speed at which you can single-step.  The *Auto-Refresh* checkbox is provided to override this behaviour.  When working with simulation or replay data the serial connection speed is irrelevant, and variables will be automatically refreshed after each single-step.



SW/RTL Discrepancy Checking
+++++++++++++++++++++++++++
The debugger supports SW/RTL discrepancy checking, as described in [:ref:`3 <debug-reference-calagar>`].  Discrepancy checking can be performed by clicking the *Run* button on the *Tools* tab.  Discrepancy checking simulates the RTL in Modelsim executes the software using gdb, and then compares the execution traces for differences.  A report of any differences can be viewed by clicking the *View Report* button.

To use discrepancy checking, ensure the following conditions are met:
    #. The following options must be added to the config.tcl for the design: ::

        set_parameter INSPECT_DEBUG 1
        set_parameter NO_ROMS 1

    #. Compile a binary of the design using gcc, ensuring the name of the executable is <design_name>.out: ::

        gcc -g array.c -o array.out

    #. Ensure that RTL debug instrumentation is not enabled.  This interferes with the RTL simulation.  The DEBUG_INSERT_DEBUG_RTL option should not be enabled.

    #. The discrepancy checking uses a MySQL debug database that is separate from the MySQL database used by the rest of the debugging tools.  Unfortunately this database only holds one design at a time.  Each time you compile a design with the INSPECT_DEBUG option enabled the database is erased and repopulated.  Make sure you do not compile any other designs between compilation and discrepancy detection of the design of interest.

    #. The discrepancy detection only works with unoptimized designs.  Make sure these options are included in the Makefile of the design: ::

        NO_OPT = 1
        NO_INLINE = 1
        DEBUG_KEEP_VARS_IN_MEM = 1

.. _debug-references:

References
++++++++++

.. _debug-reference-goeders1:

[1] Jeffrey Goeders and Steven Wilton. Effective FPGA Debug for High-Level Synthesis Generated Circuits. *In International Conference on Field Programmable Logic and Applications*, September 2014. [ `http <http://dx.doi.org/10.1109/FPL.2014.6927498>`__ ]

.. _debug-reference-goeders2:

[2] Jeffrey Goeders and Steven Wilton. Using Dynamic Signal-Tracing to Debug Compiler-Optimized HLS Circuits on FPGAs. *In International Symposium on Field-Programmable Custom Computing Machines*, pages 127-134, May 2015.

.. _debug-reference-calagar:

[3] Nazanin Calagar, Stephen D. Brown, and Jason H. Anderson.  Source-Level Debugging for FPGA High-Level Synthesis. *In International Conference on Field Programmable Logic and Applications*, September 2014. [ `http <http://dx.doi.org/10.1109/FPL.2014.6927496>`__ ]