.. _PS_MIN_WIDTH

PS_MIN_WIDTH
-------------

The minimum bit width of an instruction to consider (e.g. don't bother
sharing 1 bit adders)

This parameter does NOTHING unless ENABLE_PATTERN_SHARING is 1.

Paper: Stefan Hadjis, Andrew Canis, Jason Helge Anderson, Jongsok
Choi, Kevin Nam, Stephen Dean Brown, Tomasz S. Czajkowski, "Impact of
FPGA architecture on resource sharing in high-level synthesis," FPGA
2012: 111-114


Category
+++++++++

HLS Constraints

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

Any integer

Default Value
++++++++++++++

2

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

ENABLE_PATTERN_SHARING

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter PS_MIN_WIDTH 2''

--------------------------------------------------------------------------------

