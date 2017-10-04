.. _GROUP_RAMS_SIMPLE_OFFSET:

GROUP_RAMS_SIMPLE_OFFSET
-------------------------

When GROUP_RAMS is on, this option simplifies the address offset calculation.
Calculate the offset for each array into the shared RAM to minimize addition.
The offset must be a multiple of the size of the array in bytes (to allow an OR
instead of an ADD): 
      before: addr = baseaddr + offset
      after:  addr = baseaddr OR offset
the idea is that none of the lower bits of baseaddr should overlap with any
bits of offset. This improves area and fmax (less adders) but at the cost of
wasted empty memory inside the shared RAM

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

    ``set_parameter GROUP_RAMS_SIMPLE_OFFSET 1``

--------------------------------------------------------------------------------

