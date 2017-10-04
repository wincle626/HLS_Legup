.. _RESTRICT_TO_MAXDSP:

RESTRICT_TO_MAXDSP
-------------------

This parameter turns on multiplier sharing (binding) when the number of
multipliers required by the design is going to exceed the maximum DSP blocks
available on the target FPGA.  Turning this on prevents the final circuit from
implementing multiplication with soft logic (high area cost) after all DSP
blocks are used, instead sharing the available DSP blocks using multiplexing.

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1

Default Value
++++++++++++++

0

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

Untested recently

Examples
+++++++++

    ``set_parameter RESTRICT_TO_MAXDSP 1``

--------------------------------------------------------------------------------

