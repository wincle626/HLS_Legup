.. _DUAL_PORT_BINDING:

DUAL_PORT_BINDING
-----------------

Enabling this parameter results in use of the dual-ported on-chip memories, allowing up to two memory accesses per clock cycle.

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

1 (On)

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

Example
+++++++++

``set_parameter DUAL_PORT_BINDING 1``

-----------------------------------------------------------------------------------------------------

