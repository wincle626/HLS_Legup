.. _EXPLICIT_LPM_MULTS:

EXPLICIT_LPM_MULTS
-------------------


This parameter explicitly instantiate all multipliers as Altera lpm_mult modules.
When this is off (default) multiplies are instantiated using the Verilog multiply operator.

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

    ``set_parameter EXPLICIT_LPM_MULTS 1``

--------------------------------------------------------------------------------

