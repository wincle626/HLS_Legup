.. _PIPELINE_ALL:

PIPELINE_ALL
-------------

This parameter tells LegUp to try to pipeline every loop in the program
regardless of the loop label. Loops will only be pipelined if they are only one
basic block.

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

    ``set_parameter PIPELINE_ALL 1``

--------------------------------------------------------------------------------

