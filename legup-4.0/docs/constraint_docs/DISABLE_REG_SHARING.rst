.. _DISABLE_REG_SHARING:

DISABLE_REG_SHARING
-------------------

Disables register sharing based on live variable analysis.

Category
+++++++++

HLS Constraint

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1

Default Value
++++++++++++++

unset (0 - disabled)

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

    ``set_parameter DISABLE_REG_SHARING 1``

--------------------------------------------------------------------------------

