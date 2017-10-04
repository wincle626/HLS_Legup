.. _MULTIPLIER_NO_CHAIN:

MULTIPLIER_NO_CHAIN
--------------------

This parameter tells the LegUp scheduler not to chain multipliers. If this parameter
is on then every multiplier will be scheduled into a separate clock cycle.

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

    ``set_parameter MULTIPLIER_NO_CHAIN 1``

--------------------------------------------------------------------------------

