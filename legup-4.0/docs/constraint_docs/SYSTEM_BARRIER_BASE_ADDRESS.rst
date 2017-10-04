.. _SYSTEM_BARRIER_BASE_ADDRESS:

SYSTEM_BARRIER_BASE_ADDRESS
---------------------------

This parameter specifies the base address of the barrier in the QSys system.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

Any valid base address.

Default Value
++++++++++++++

N/A

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``boards/<device_family>/<board>/<system>/legup.tcl``

e.g. ``boards/CycloneV/DE1-SoC/Tiger_SDRAM/legup.tcl``

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Not used

Examples
+++++++++

``set_parameter SYSTEM_BARRIER_BASE_ADDRESS 0x00800000``

--------------------------------------------------------------------------------
