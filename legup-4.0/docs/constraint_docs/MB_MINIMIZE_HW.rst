
.. _MB_MINIMIZE_HW:

MB_MINIMIZE_HW
----------------

This parameter toggles whether the reduced bitwidths analyzed by the bitwidth 
minimization pass will be used in generating the Verilog design.

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

Related parameters:
:ref:`MB_RANGE_FILE`, :ref:`MB_MAX_BACK_PASSES`, :ref:`MB_PRINT_STATS`

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

``set_parameter MB_MINIMIZE_HW 1``

--------------------------------------------------------------------------------

