.. _set_combine_basicblock:

set_combine_basicblock
----------------------

This parameter allows for basic block merging within the LLVM IR which 
potentially reduces the number of cycles of execution. There are two modes of
operation: merge patterns throughout program, merge patterns only within loops.
Currently, only 2 patterns are supported:

Pattern A:

.. image:: images/PatternA.PNG

A1, A2, A3 are basicblocks.

Pattern B:

.. image:: images/PatternB.PNG

B1, B2, B3, B4 are basicblocks.

Category
+++++++++

HLS Constraint

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1, 2

Default Value
++++++++++++++

unset (off by default)

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Note:
May require :ref:`LOCAL_RAMS` and :ref:`GLOBAL_RAMS` to be turned off.

Applicable Flows
+++++++++++++++++

All devices for pure hardware flow

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

``set_combine_basicblock 1``

--------------------------------------------------------------------------------

