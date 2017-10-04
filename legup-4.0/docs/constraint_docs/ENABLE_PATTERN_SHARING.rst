.. _ENABLE_PATTERN_SHARING

ENABLE_PATTERN_SHARING
-------------

This parameter enables resource sharing for patterns of computational
operators in a program's dataflow graph.  The idea is that, in a given
program, there may be commonly occurring patterns of operators that
could be shared in the hardware, by putting multiplexers on the inputs
and steering the right data in at the right time (based on the FSM
state).  This may save area in certain cases.  The approach is
described in this paper:

Stefan Hadjis, Andrew Canis, Jason Helge Anderson, Jongsok Choi, Kevin
Nam, Stephen Dean Brown, Tomasz S. Czajkowski, "Impact of FPGA
architecture on resource sharing in high-level synthesis," FPGA 2012:
111-114


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

Actively in-use

Examples
+++++++++

    ``set_parameter ENABLE_PATTERN_SHARING 1``

--------------------------------------------------------------------------------

