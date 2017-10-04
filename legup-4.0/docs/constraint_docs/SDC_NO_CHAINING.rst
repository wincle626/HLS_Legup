.. _SDC_NO_CHAINING

SDC_NO_CHAINING
-------------

This is an HLS scheduling constraint that can have a significant
impact on speed performance.  Chaining is a concept in HLS that allows
operators to be stitched together combinationally in a single clock
cycle, provided a target clock period constraint is met.  When this
parameter is set to 1, chaining is disabled and every operator will be
scheduled in its own FSM state.  This will generally increase the
number of cycles in the overall schedule (bad), but it may help the
circuit Fmax (good).

When this parameter is set to 1, then the CLOCK_PERIOD constraint is
irrelevant.  The reason for this is that the CLOCK_PERIOD constraint
aims to allow chaining to the extent possible such that the specified
period is met.

In general, this parameter should remain 0; however, there may be
some utility in setting it to 1 for research purposes.

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0 or 1

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

    ``set_parameter SDC_NO_CHAINING 1``

--------------------------------------------------------------------------------

