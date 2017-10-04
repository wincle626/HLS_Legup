.. _MODULO_DEBUG:

MODULO_DEBUG
-------------

This parameter show some high level debugging information from the loop pipelining (modulo) scheduler. 
For instance, the final initiation interval (II) and how many backtracking attempts were made.

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

    ``set_parameter MODULO_DEBUG 1``

--------------------------------------------------------------------------------

