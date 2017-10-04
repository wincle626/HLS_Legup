.. _CASE_FSM:

CASE_FSM
-------------

This parameter controls whether the finite state machine (FSM) in the Verilog
output by LegUp is implemented with a ``case`` statement or ``if-else``
statements.
Although both options are functionally equivalent; some back-end RTL synthesis
tools may be sensitive to the RTL coding style.

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

    ``set_parameter CASE_FSM 1``

--------------------------------------------------------------------------------

