.. _NO_DFG_DOT_FILES:

NO_DFG_DOT_FILES
-------------

By default, LegUp generates data flow graph dot files for every basic block. 
This constraint can disable the generation of those dot files.

Category
+++++++++

Debugging

Value Type
+++++++++++

String

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

    ``set_parameter NO_DFG_DOT_FILES 1``

--------------------------------------------------------------------------------

