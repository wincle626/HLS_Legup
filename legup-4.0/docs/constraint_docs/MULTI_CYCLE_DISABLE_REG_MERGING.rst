
.. _MULTI_CYCLE_DISABLE_REG_MERGING:

MULTI_CYCLE_DISABLE_REG_MERGING
----------------

Enabling this will print qsf constraints to instruct Quartus synthesis to not 
merge registers that have multi-cycle constraints.

Category
+++++++++

Quartus

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
:ref:`MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS`, :ref:`MULTI_CYCLE_DEBUG`, 
:ref:`MULTI_CYCLE_REMOVE_REG_DIVIDERS`, :ref:`MULTI_CYCLE_DUPLICATE_LOAD_REG`, 
:ref:`MULTI_CYCLE_REMOVE_CMP_REG`

Applicable Flows
+++++++++++++++++

Pure hardware flow with no multipumping and no loop pipelining.

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter MULTI_CYCLE_DISABLE_REG_MERGING 1``

--------------------------------------------------------------------------------

