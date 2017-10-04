
.. _MB_RANGE_FILE:

MB_RANGE_FILE
----------------

The bitwidth minimization pass in LegUp is capable of reading in data value 
ranges based on profiling results. This parameter specifies the filename from 
which to read the initial data ranges used in the analysis. If this parameter is 
not set, no initial ranges are assumed. 


Category
+++++++++

HLS Constraints

Value Type
+++++++++++

String

Valid Values
+++++++++++++

Alphanumeric file name

Default Value
++++++++++++++

N/A

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

Need to set :ref:`MB_MINIMIZE_HW` to 1 for this to take effect

Related parameters:
:ref:`MB_PRINT_STATS`, :ref: `MB_MAX_BACK_PASSES`

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

``set_parameter MB_RANGE_FILE "range.profile"``

--------------------------------------------------------------------------------

