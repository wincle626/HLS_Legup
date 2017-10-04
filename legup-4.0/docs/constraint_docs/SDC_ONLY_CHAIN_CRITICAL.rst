.. _SDC_ONLY_CHAIN_CRITICAL:

SDC_ONLY_CHAIN_CRITICAL
------------------------

By default during modulo scheduling we allow chaining to occur anywhere. This
parameter turns off chaining for any operation not on a loop recurrence for
maximum pipelining while still not impacting II. This parameter only works when
SDC_BACKTRACKING is on.

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

set_parameter MODULO_SCHEDULER SDC_BACKTRACKING

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter SDC_ONLY_CHAIN_CRITICAL 1``

--------------------------------------------------------------------------------

