.. _SDC_BACKTRACKING_BUDGET_RATIO:

SDC_BACKTRACKING_BUDGET_RATIO
-----------------------------

This parameter specifies the budget ratio used during loop pipelining. The
budget ratio is a measure of backtracking effort in the SDC
modulo scheduler. A low ratio will give up backtracking early, saving
runtime but may not achieve the minimum II.

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

Positive Integer

Default Value
++++++++++++++

6

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

set_parameter MODULO_SCHEDULER SDC_BACKTRACKING

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter SDC_BACKTRACKING_BUDGET_RATIO 6``

--------------------------------------------------------------------------------

