.. _PIPELINE_SAVE_REG:

PIPELINE_SAVE_REG
------------------

This parameter saves registers in the loop pipeline by only using registers on
the pipeline stage boundaries. Instead of using a register for every single
pipeline time step.

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

    ``set_parameter PIPELINE_SAVE_REG 1``

--------------------------------------------------------------------------------

