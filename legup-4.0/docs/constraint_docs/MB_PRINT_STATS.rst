
.. _MB_PRINT_STATS:

MB_PRINT_STATS
----------------

This parameter toggles whether the bitiwdth minimization pass prints a report 
showing the number of bits reduced. The reported numbers are an over-estimation 
of the savings since Quartus performs some bitwidth optimization internally 
already.

Since the bitwidth minimization anlysis is always performed, this report 
reflects the actual number of bits saved only if :ref:`MB_MINIMIZE_HW` is turned 
on.

Category
+++++++++

Miscellaneous

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

Need to set :ref:`MB_MINIMIZE_HW` to 1 for the report to reflect the actual 
number of bits saved.

Related parameters:
:ref:`MB_RANGE_FILE`, :ref: `MB_MAX_BACK_PASSES`

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

``set_parameter MB_PRINT_STATS 1``

--------------------------------------------------------------------------------

