.. _VSIM_NO_ASSERT:

VSIM_NO_ASSERT
-------------

When set to 1, this constrain cause assertions to be disabled in the Verilog 
output used to debug LegUp. This is useful to speed-up long simulation.

Category
+++++++++

Simulation

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1

Default Value
++++++++++++++

unset (0)

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

Untested

Examples
+++++++++

    ``set_parameter VSIM_NO_ASSERT 1``

--------------------------------------------------------------------------------

