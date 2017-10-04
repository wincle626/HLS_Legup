.. _DONT_CHAIN_GET_ELEM_PTR:

DONT_CHAIN_GET_ELEM_PTR
-------------

By default, LegUp assumes that get element pointer instructions take zero time 
in hardware and will therefore chain consecutive get element pointer 
instructions into a single cycle. This default behaviour can be disabled by 
setting the DONT_CHAIN_GET_ELEM_PTR constraint to zero. This will split 
dependent get element pointer instructions into seperate clock cycles.

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

    ``set_parameter DONT_CHAIN_GET_ELEM_PTR 0``

--------------------------------------------------------------------------------

