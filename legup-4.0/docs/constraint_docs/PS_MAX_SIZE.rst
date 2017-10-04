.. _PS_MAX_SIZE

PS_MAX_SIZE
-------------

Maximum pattern size to share. Setting to 0 will also disable pattern
sharing.  Setting this to a high value may incur significant runtime.

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

    ``set_parameter PS_MAX_SIZE 10''

--------------------------------------------------------------------------------

