.. _SDC_BACKTRACKING_PRIORITY:

SDC_BACKTRACKING_PRIORITY
--------------------------

The same as SDC_PRIORITY but for backtracking SDC modulo scheduling.  A
priority function isn't strictly necessary due to backtracking but a decent
scheduling order can really speed up scheduling by reducing the amount of
backtracking. Without this on you might have to increase the
SDC_BACKTRACKING_BUDGET_RATIO parameter to allow more backtracking.


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

1

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

    ``set_parameter SDC_BACKTRACKING_PRIORITY 1``

--------------------------------------------------------------------------------

