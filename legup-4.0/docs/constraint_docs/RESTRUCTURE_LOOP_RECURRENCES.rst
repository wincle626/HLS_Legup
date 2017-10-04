.. _RESTRUCTURE_LOOP_RECURRENCES:

RESTRUCTURE_LOOP_RECURRENCES
-----------------------------

This parameter restructures the loop body expression tree using associative
transformations to reduce recurrence cycles that limit initiation interval when
loop pipelining.

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

    ``set_parameter RESTRUCTURE_LOOP_RECURRENCES 1``

--------------------------------------------------------------------------------

