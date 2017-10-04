.. _DFG_SHOW_DUMMY_CALLS:

DFG_SHOW_DUMMY_CALLS
--------------------

Show dummy calls such as printf in the dataflow dot graphs.

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

unset (0, dummy calls ignored)

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

:ref:`NO_DFG_DOT_FILES` should be disabled

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter DFG_SHOW_DUMMY_CALLS 1``

--------------------------------------------------------------------------------

