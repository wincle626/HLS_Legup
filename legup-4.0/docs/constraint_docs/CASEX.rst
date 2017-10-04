.. _CASEX:

CASEX
-------------

This parameter indicates whether ``casex(...)`` statement should be used instead
of ``case(...)`` statement in the ``case`` style FSM verilog. This parameter
is ignored when ``CASE_FSM`` is disabled.

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

CASE_FSM

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter CASEX 1``

--------------------------------------------------------------------------------

