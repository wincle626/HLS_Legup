.. _SDC_PRIORITY:

SDC_PRIORITY
-------------

In greedy SDC modulo scheduling the order of scheduling instructions uses a
priority function based on perturbation. Perturbation is calculated for each
instruction by:
   1) adding a GE constraint to the SDC formulation for that
      instruction
   2) resolving the SDC schedule
   3) counting how many many other instructions are displaced from
      their prior schedule
Instruction with higher perturbation are scheduled with higher
priority.  If this is off, instructions are scheduled randomly.

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

1

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

    ``set_parameter SDC_PRIORITY 1``

--------------------------------------------------------------------------------

