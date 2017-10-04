.. _LOCAL_RAMS:

LOCAL_RAMS
-------------

This parameter turns on alias analysis to determine when an array 
is only used in one function.  These arrays can be placed in a block ram
inside that hardware module instead of in global memory.
This increases performance because local rams can be accessed in parallel
while global memory is limited to two ports.

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

    ``set_parameter LOCAL_RAMS 1``

--------------------------------------------------------------------------------

