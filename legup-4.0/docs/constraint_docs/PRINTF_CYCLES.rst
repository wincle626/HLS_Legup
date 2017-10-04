.. _PRINTF_CYCLES:

PRINTF_CYCLES
-------------

Enabling this parameter will result in prepending each printf function with the
number of cycles elapsed to be displayed in ModelSim simulations.

Category
+++++++++

Debugging

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

Actively in-use

Examples
+++++++++

    ``set_parameter PRINTF_CYCLES 1``

--------------------------------------------------------------------------------

