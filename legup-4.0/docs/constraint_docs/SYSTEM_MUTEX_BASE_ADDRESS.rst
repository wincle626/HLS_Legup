.. _SYSTEM_MUTEX_BASE_ADDRESS:

SYSTEM_MUTEX_BASE_ADDRESS
-------------------------

This parameter specifies the base address of the mutex module in the QSys
system.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

The base address of the Mutex module in QSys.

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

Not used.

Examples
+++++++++

``set_parameter SYSTEM_MUTEX_BASE_ADDRESS 0x00800000``

--------------------------------------------------------------------------------
