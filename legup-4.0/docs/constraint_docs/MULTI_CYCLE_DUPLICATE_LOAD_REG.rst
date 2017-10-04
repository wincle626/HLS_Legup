
.. _MULTI_CYCLE_DUPLICATE_LOAD_REG:

MULTI_CYCLE_DUPLICATE_LOAD_REG
----------------

Enabling this parameter will duplicate load registers. This is used to allow 
loads from global memory to be incorporated into multi-cycle paths.

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

:ref:`MULTI_CYCLE_REMOVE_REG`

Related parameters:
:ref:`MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS`, :ref:`MULTI_CYCLE_DEBUG`, i
:ref:`MULTI_CYCLE_REMOVE_REG_DIVIDERS`, :ref:`MULTI_CYCLE_REMOVE_CMP_REG`,
:ref:`MULTI_CYCLE_DISABLE_REG_MERGING`

Applicable Flows
+++++++++++++++++

Pure hardware flow with no multipumping and no loop pipelining.

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter MULTI_CYCLE_DUPLICATE_LOAD_REG 1``

--------------------------------------------------------------------------------

