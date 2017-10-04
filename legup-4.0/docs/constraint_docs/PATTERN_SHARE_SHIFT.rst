.. _PATTERN_SHARE_SHIFT

PATTERN_SHARE_SHIFT
-------------

Select which instructions to share in pattern sharing. Choices are:
  Adders / Subtractors
  Bitwise operations (AND, OR, XOR)
  Shifts (logical shift Left/Right and arithmetic shift Right)

If set, these instructions will be included in patterns and shared with 2-1
muxing. Note that multipliers, dividers and remainders are not shared in 
patterns because they should be shared with more than 2-1 muxing (if at all). 
The bipartite binding algorithm is used for those instructions while pattern 
sharing is used for the smaller instructions above.

This particular parameter pertains to sharing shifts.

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

0 or 1

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

    ``set_parameter PATTERN_SHARE_SHIFT 1''

--------------------------------------------------------------------------------

