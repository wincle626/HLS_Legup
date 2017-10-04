
.. _MB_MAX_BACK_PASSES:

MB_MAX_BACK_PASSES
----------------

The bitwidth minimization pass repeatedly traverses the CDFG to reduce the 
number of bits used in each variable. Every forward traversal of the CDFG is 
followed by a backward traversal. This parameter specifies the maximum number of 
times the analysis should traverse backwards on the CDFG.

Setting this parameter to -1 instructs the analysis to continually traverse the 
CDFG until no further bitwidth reduction can be achieved.

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

-1, 0 to INT_MAX

Default Value
++++++++++++++

-1

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

Need to set :ref:`MB_MINIMIZE_HW` to 1 for this parameter to take effect on the 
generated Verilog.

Related parameters:
:ref:`MB_RANGE_FILE`, :ref: `MB_PRINT_STATS`

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

``set_parameter MB_MAX_BACK_PASSES -1``

--------------------------------------------------------------------------------

