
.. _MULTI_CYCLE_REMOVE_REG:

MULTI_CYCLE_REMOVE_REG
----------------

This parameter instructs LegUp to multi-cycle combinational data paths in 
infrequently used basic blocks, in an attempt to increase the circuit's FMax. 
Doing this will remove registers on the multi-cycle paths and generate 
multi-cycle sdc constraints.

Multi-cycle paths are only enabled for the pure hardware flow with no 
multipumping and no loop pipelining.


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

Related parameters:
:ref:`MULTI_CYCLE_ADD_THROUGH_CONSTRAINTS`, :ref:`MULTI_CYCLE_DEBUG`,
:ref:`MULTI_CYCLE_REMOVE_REG_DIVIDERS`, :ref:`MULTI_CYCLE_DUPLICATE_LOAD_REG`, 
:ref:`MULTI_CYCLE_REMOVE_CMP_REG`, :ref:`MULTI_CYCLE_DISABLE_REG_MERGING`

Applicable Flows
+++++++++++++++++

Pure hardware flow with no multipumping and no loop pipelining.

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter MULTI_CYCLE_REMOVE_REG 1``

--------------------------------------------------------------------------------

