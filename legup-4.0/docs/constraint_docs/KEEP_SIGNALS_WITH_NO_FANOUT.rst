.. _KEEP_SIGNALS_WITH_NO_FANOUT:

KEEP_SIGNALS_WITH_NO_FANOUT
---------------------------

If this parameter is enabled, all signals will be printed to the output Verilog
file, even if they don't drive any outputs.

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

    ``set_parameter KEEP_SIGNALS_WITH_NO_FANOUT 1``

--------------------------------------------------------------------------------

