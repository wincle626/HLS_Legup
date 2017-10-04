.. _set_custom_verilog_function:

set_custom_verilog_function
-------------

This TCL command is to specify the name and module interface for a function with
custom Verilog. LegUp will not generate Verilog for the functions specified by
the command, but will still instantiate the corresponding module based
on the specified interface and connect it with the rest of HLS-generated
Verilog. This command requires multiple arguments, 1) function name, 2) memory
access requirement and 3) direction, name and bit-width for each module port.
The Verilog definition of the custom function should be provided via the
``set_custom_verilog_file`` command.

Category
+++++++++

HLS Constraints

Valid Values
+++++++++++++

| Function name: string
| Memory access requirement: one of the two strings, 'memory' or 'noMemory'
| Module port directory: one of the two strings, 'input' or 'output'
| Module port bit-width: a string in format, 'high_bit:low_bit', where high_bit and low_bit are integers
| Module port name: a string with no whitespace.

Dependencies
+++++++++++++

set_custom_verilog_file

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

    | ``set_custom_verilog_function "boardGetChar" noMemory``
    |     ``input 7:0 UART_BYTE_IN \``
    |     ``input 0:0 UART_START_RECEIVE \``
    |     ``input 1:0 UART_RESPONSE \``
    |     ``output 17:2 LEDR``


--------------------------------------------------------------------------------

