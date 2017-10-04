.. _PS_MIN_SIZE

PS_MIN_SIZE
-------------

Minimum pattern size to share. This is used because sharing is more
beneficial for larger patterns (larger patterns have a smaller
mux:instruction ratio) and sometimes sharing is only beneficial for
patterns of a certain size or greater. For example, in Cyclone II
sharing small patterns (e.g. 2 adds) does not reduce area.

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

1

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

    ``set_parameter PS_MIN_SIZE 1''

--------------------------------------------------------------------------------

