.. _set_operation_latency:

set_operation_latency
---------------------

This parameter sets the latency of a given operation when compiled in LegUp.
Latency refers to the number of clock cycles required to complete the computation; an operation with latency one requires one cycle, while zero-latency operations are completely combinational, meaning multiple such operations can be chained together in a single clock cycle.

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

<operation> integer

Valid Values
+++++++++++++

See Default and Examples
Note: operator name should match the device family operation database file:
boards/StratixIV/StratixIV.tcl or boards/CycloneII/CycloneII.tcl

Default Values
++++++++++++++

::

    altfp_add 14
    altfp_subtract 14
    altfp_multiply 11
    altfp_divide_32 33
    altfp_divide_64 61
    altfp_truncate_64 3
    altfp_extend_32 2
    altfp_fptosi 6
    altfp_sitofp 6
    signed_comp_o 1
    signed_comp_u 1
    reg 2
    mem_dual_port 2
    local_mem_dual_port 1
    multiply 1

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_operation_latency altfp_add_32 18``

``set_operation_latency multiply 0``

--------------------------------------------------------------------------------

