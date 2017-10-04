.. _GROUP_RAMS:

GROUP_RAMS
-------------

This parameter group all arrays in the global memory controller into four RAMs (one for each
bitwidth: 8, 16, 32, 64). This saves M9K blocks by avoiding having a small array taking
up an entire M9K block.


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

    ``set_parameter GROUP_RAMS 1``

--------------------------------------------------------------------------------

