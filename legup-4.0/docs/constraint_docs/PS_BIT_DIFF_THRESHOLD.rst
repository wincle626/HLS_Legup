.. _PS_BIT_DIFF_THRESHOLD

PS_BIT_DIFF_THRESHOLD
-------------

Two operations will only be shared if the difference of their true bit
widths is below this threshold: e.g. an 8-bit adder will not be shared
with a 32-bit adder unless BIT_DIFF_THRESHOLD >= 24

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

10

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

    ``set_parameter PS_BIT_DIFF_THRESHOLD 10''

--------------------------------------------------------------------------------

