.. _INCREMENTAL_SDC:

INCREMENTAL_SDC
----------------

This parameter solve the SDC problem during loop pipelining incrementally by
detecting negative cycles in the constraint graph. This is marginally faster
than the default method of rerunning the LP solver.

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

    ``set_parameter INCREMENTAL_SDC 1``

--------------------------------------------------------------------------------

