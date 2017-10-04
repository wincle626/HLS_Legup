.. _SDC_MULTIPUMP:

SDC_MULTIPUMP
-------------


This parameter schedules more multipliers into the same state for multi-pumping.
If two multipliers are scheduled into the same state then we can replace them
with a multi-pumped DSP and save DSP blocks (with multi-pumping on)

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

MULTIPUMPING

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter SDC_MULTIPUMP 1``

--------------------------------------------------------------------------------

