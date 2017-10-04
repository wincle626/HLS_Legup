
.. _MULTI_CYCLE_REMOVE_REG_DIVIDERS:

MULTI_CYCLE_REMOVE_REG_DIVIDERS
----------------

Enabling this parameter will multi-cycle dividers. This requires 
:ref:`MULTI_CYCLE_REMOVE_REG` to be set to 1.

Multi-cycle paths are only enabled for the pure hardware flow with no 
multipumping and no pipelined resources.


Category
+++++++++

Constraints

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
:ref:`MB_RANGE_FILE`, :ref:`MB_MAX_BACK_PASSES`, :ref:`MB_PRINT_STATS`

Applicable Flows
+++++++++++++++++

Pure hardware flow with no multipumping and no pipelined resources.

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter MULTI_CYCLE_REMOVE_REG_DIVIDERS 1``

--------------------------------------------------------------------------------

