.. _INFERRED_RAMS:

INFERRED_RAMS
-------------

Use Verilog to infer RAMs instead of instantiating an Altera altsyncram module.
Note: inferred RAMs don't support structs (no byte-enable) so having
structs in your program forces this parameter to turn off

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

    ``set_parameter INFERRED_RAMS 1``

--------------------------------------------------------------------------------

